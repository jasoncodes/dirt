#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: CryptSocketProxy.cpp,v 1.24 2003-08-06 14:45:33 jason Exp $)

#include "CryptSocketProxy.h"
#include "IPInfo.h"
#include "CryptSocket.h"
#include "Crypt.h"
#include "HTTP.h"

//////// CryptSocketProxyHTTP ////////

static const wxString CRLF = wxT("\r\n");

class CryptSocketProxyHTTP : public CryptSocketProxy
{

public:
	CryptSocketProxyHTTP(CryptSocketBase *sck)
		: CryptSocketProxy(sck)
	{
		m_connected_to_remote = false;
	}

	virtual void OnConnect()
	{
		
		wxString request;
		
		request
			<< wxT("CONNECT ")
			<< m_dest_ip << wxT(':') << (int)m_dest_port
			<< wxT(" HTTP/1.0") << CRLF;

		request << wxT("User-Agent: ") << HTTP::GetDefaultUserAgent() << CRLF;

		wxString username = m_settings.GetUsername();
		wxString password = m_settings.GetPassword(true);
		if (username.Length() || password.Length())
		{
			wxString auth;
			auth << username << wxT(':') << password;
			request
				<< wxT("Proxy-Authorization: Basic ")
				<< Crypt::Base64Encode(auth, false)
				<< CRLF;
		}

		request << CRLF;

		SendData(request);

	}

	virtual void OnInput(const ByteBuffer &data)
	{
		m_buff += data;
		wxString separator = CRLF+CRLF;
		int pos = m_buff.Find(separator);
		if (pos > -1)
		{
			HTTPHeader header(m_buff.Left(pos + separator.Length()));
			if (header.IsValid())
			{
				if (header.GetStatusCode() == 200)
				{
					m_connected_to_remote = true;
					ProxyConnected(m_buff.Mid(pos + separator.Length()));
					m_buff = ByteBuffer();
				}
				else
				{
					ConnectionError(header.GetStatusLine());
				}
			}
			else
			{
				ConnectionError(wxT("Invalid HTTP response"));
			}
		}
		else if (m_buff.Length() > 4096)
		{
			ConnectionError(wxT("No HTTP response in first 4 KB"));
		}
	}

	virtual bool IsConnectedToRemote() const
	{
		return m_connected_to_remote;
	}

protected:
	bool m_connected_to_remote;
	ByteBuffer m_buff;

};

//////// CryptSocketProxySOCKS4 ////////

class CryptSocketProxySOCKS4 : public CryptSocketProxy
{

public:
	CryptSocketProxySOCKS4(CryptSocketBase *sck, bool is_connect)
		: CryptSocketProxy(sck), m_is_connect(is_connect)
	{
		m_connected_to_remote = false;
	}

	virtual void OnConnect()
	{
		wxASSERT_MSG(m_is_connect, wxT("Listen not supported yet"));
		ByteBuffer request;
		request += ByteBuffer(1, 4); // SOCKS 4
		request += ByteBuffer(1, 1); // CONNECT
		request += Uint16ToBytes(m_dest_port); // PORT
		request += Uint32ToBytes(wxUINT32_SWAP_ON_LE(GetIPV4Address(m_dest_ip))); // IP
		request += m_settings.GetUsername(); // USERID
		request += ByteBuffer(1, 0); // NULL
		SendData(request);
	}

	virtual void OnInput(const ByteBuffer &data)
	{
		wxASSERT_MSG(m_is_connect, wxT("Listen not supported yet"));
		m_buff += data;
		if (m_buff.Length() >= 8)
		{
			if (m_buff[0] == 0)
			{
				switch (m_buff[1])
				{
					case 90:
						m_connected_to_remote = true;
						ProxyConnected(m_buff.Mid(8));
						m_buff = ByteBuffer();
						break;
					case 91: 
						ConnectionError(wxT("Request rejected or failed"));
						break;
					case 92:
						ConnectionError(wxT("SOCKS server cannot connect to identd on the client"));
						break;
					case 93:
						ConnectionError(wxT("Client and identd report different user-ids"));
						break;
					default:
						ConnectionError(wxString() << wxT("Unknown SOCKS4 reply code: ") << (int)m_buff[1]);
						break;
				}
			}
			else
			{
				ConnectionError(wxT("Invalid SOCKS4 response"));
			}
		}
	}

	virtual bool IsConnectedToRemote() const
	{
		return m_connected_to_remote;
	}

protected:
	bool m_is_connect;
	bool m_connected_to_remote;
	ByteBuffer m_buff;

};

//////// CryptSocketProxySOCKS5 ////////

class CryptSocketProxySOCKS5 : public CryptSocketProxy
{

public:
	CryptSocketProxySOCKS5(CryptSocketBase *sck, bool is_connect)
		: CryptSocketProxy(sck), m_is_connect(is_connect)
	{
		m_connected_to_remote = false;
		m_state = stateUnknown;
	}

	virtual void OnConnect()
	{
		wxASSERT_MSG(m_is_connect, wxT("Listen not supported yet"));
		bool has_pass =
			m_settings.GetUsername().Length() ||
			m_settings.GetPassword(true).Length();
		ByteBuffer request;
		request += ByteBuffer(1, 5); // SOCKS 5
		request += ByteBuffer(1, has_pass ? 2 : 1);
		if (has_pass)
		{
			request += ByteBuffer(1, 2); // user/pass
		}
		request += ByteBuffer(1, 0); // no auth
		m_state = stateAuthType;
		SendData(request);
	}

	virtual void OnInput(const ByteBuffer &data)
	{
		wxASSERT_MSG(m_is_connect, wxT("Listen not supported yet"));

		m_buff += data;

		switch (m_state)
		{

			case stateAuthType:
				if (m_buff.Length() >= 2)
				{

					if (m_buff[0] == 5)
					{

						switch (m_buff[1])
						{

							case 0: // no auth
								m_buff = m_buff.Mid(2);
								SendConnectRequest();
								break;

							case 2: // user/pass
								m_buff = m_buff.Mid(2);
								SendUserPass();
								break;

							default:
								ConnectionError(wxT("Unknown authentication requested"));
								break;

						}

					}
					else
					{
						ConnectionError(wxT("Unexpected method response version"));
					}

				}
				break;

			case stateUserPassResponse:
				if (m_buff.Length() >= 2)
				{
					if (m_buff[0] == 1)
					{
						if (m_buff[1] == 0)
						{
							m_buff = m_buff.Mid(2);
							SendConnectRequest();
						}
						else
						{
							ConnectionError(wxT("Invalid username/password"));
						}
					}
					else
					{
						ConnectionError(wxT("Unexpected user/pass response version"));
					}
				}
				break;

			case stateConnectResponse:
				if (m_buff.Length() >= 2)
				{
					if (m_buff[0] == 5)
					{
						switch (m_buff[1])
						{
							case 0:
								if (m_buff.Length() >= 10)
								{
									if (m_buff[3] == 1)
									{
										//ByteBuffer remote_ip_bytes = m_buff.Mid(4,4);
										//ByteBuffer remote_port_bytes = m_buff.Mid(8,2);
										m_connected_to_remote = true;
										ProxyConnected(m_buff.Mid(10));
										m_buff = ByteBuffer();
									}
									else
									{
										ConnectionError(wxT("Unexpected address type in connect response"));
									}
								}
								break;
							case 1:
								ConnectionError(wxT("General SOCKS server failure"));
								break;
							case 2:
								ConnectionError(wxT("Connection not allowed by ruleset"));
								break;
							case 3:
								ConnectionError(wxT("Network unreachable"));
								break;
							case 4:
								ConnectionError(wxT("Host unreachable"));
								break;
							case 5:
								ConnectionError(wxT("Connection refused"));
								break;
							case 6:
								ConnectionError(wxT("TTL expired"));
								break;
							case 7:
								ConnectionError(wxT("Command not supported"));
								break;
							case 8:
								ConnectionError(wxT("Address type not supported"));
								break;
							default:
								ConnectionError(wxT("Unknown connect response code"));
								break;
						}
					}
					else
					{
						ConnectionError(wxT("Unexpected connect response version"));
					}
				}
				break;

			default:
				ConnectionError(wxT("Unexpected data from proxy"));
				break;

		}

	}

	virtual bool IsConnectedToRemote() const
	{
		return m_connected_to_remote;
	}

protected:
	virtual void SendConnectRequest()
	{
		ByteBuffer request;
		request += ByteBuffer(1, 5); // SOCKS 5
		request += ByteBuffer(1, 1); // CONNECT
		request += ByteBuffer(1, 0); // RESERVED
		request += ByteBuffer(1, 1); // IPV4
		request += Uint32ToBytes(wxUINT32_SWAP_ON_LE(GetIPV4Address(m_dest_ip))); // IP
		request += Uint16ToBytes(m_dest_port); // PORT
		m_state = stateConnectResponse;
		SendData(request);
	}

	virtual void SendUserPass()
	{
		ByteBuffer request;
		wxString username = m_settings.GetUsername();
		wxString password = m_settings.GetPassword(true);
		username = username.Left(255);
		password = password.Left(255);
		request += ByteBuffer(1, 1); // VERSION 1 of USER/PASS
		request += ByteBuffer(1, username.Length()); // username length
		request += username; // username
		request += ByteBuffer(1, password.Length()); // username length
		request += password; // password
		m_state = stateUserPassResponse;
		SendData(request);
	}

protected:
	bool m_is_connect;
	bool m_connected_to_remote;
	ByteBuffer m_buff;
	enum
	{
		stateUnknown,
		stateAuthType,
		stateUserPassResponse,
		stateConnectResponse
	} m_state;

};

//////// CryptSocketProxySettings ////////

static const wxString protocol_names[] =
	{ wxT("SOCKS 4"), wxT("SOCKS 5"), wxT("HTTP CONNECT") };

static const wxString dest_modes[] =
	{ wxT("any"), wxT("allow"), wxT("deny") };

CryptSocketProxySettings::CryptSocketProxySettings(Config &config)
	: m_config(config)
{
	LoadSettings();
}

void CryptSocketProxySettings::LoadDefaults()
{
	m_enabled = false;
	m_protocol = ppSOCKS4;
	m_hostname.Empty();
	m_port = 1080;
	m_username = wxEmptyString;
	m_encrypted_password = wxEmptyString;
	m_connection_types[pctServer] = true;
	m_connection_types[pctDCCConnect] = true;
	m_connection_types[pctDCCListen] = true;
	m_dest_network_mode = pdmAny;
	m_dest_network = wxEmptyString;
	m_dest_subnet = wxEmptyString;
	m_dest_port_ranges_mode = pdmAny;
	m_dest_port_ranges_low.Empty();
	m_dest_port_ranges_high.Empty();
}

bool CryptSocketProxySettings::LoadSettings()
{

	LoadDefaults();
	
	int failcount = 0;

	failcount += !Read(wxT("Enabled"), &CryptSocketProxySettings::SetEnabled);
	failcount += !Read(wxT("Protocol"), &CryptSocketProxySettings::SetProtocol, wxEmptyString);
	failcount += !Read(wxT("Hostname"), &CryptSocketProxySettings::SetHostname, wxEmptyString);
	failcount += !Read(wxT("Port"), &CryptSocketProxySettings::SetPort, wxEmptyString);
	failcount += !Read(wxT("Username"), &CryptSocketProxySettings::SetUsername, wxEmptyString);
	failcount += !Read(wxT("Password"), &CryptSocketProxySettings::SetPassword, wxEmptyString);
	failcount += !Read(wxT("Conditions/Connection Types/Server"), &CryptSocketProxySettings::SetConnectionType, pctServer);
	failcount += !Read(wxT("Conditions/Connection Types/DCC Connect"), &CryptSocketProxySettings::SetConnectionType, pctDCCConnect);
	failcount += !Read(wxT("Conditions/Connection Types/DCC Listen"), &CryptSocketProxySettings::SetConnectionType, pctDCCListen);
	failcount += !Read(wxT("Conditions/Destination Network/Mode"), &CryptSocketProxySettings::SetDestNetworkMode, WXSIZEOF(dest_modes), dest_modes);
	failcount += !Read(wxT("Conditions/Destination Network/Network"), &CryptSocketProxySettings::SetDestNetworkNetwork, wxEmptyString);
	failcount += !Read(wxT("Conditions/Destination Network/Subnet"), &CryptSocketProxySettings::SetDestNetworkSubnet, wxEmptyString);
	failcount += !Read(wxT("Conditions/Destination Ports/Mode"), &CryptSocketProxySettings::SetDestPortsMode, WXSIZEOF(dest_modes), dest_modes);
	failcount += !Read(wxT("Conditions/Destination Ports/Ranges"), &CryptSocketProxySettings::SetDestPortRanges, wxEmptyString);

	return (failcount == 0);

}

bool CryptSocketProxySettings::SaveSettings()
{

	int failcount = 0;

	failcount += !Write(wxT("Enabled"), &CryptSocketProxySettings::GetEnabled);
	failcount += !Write(wxT("Protocol"), &CryptSocketProxySettings::GetProtocol, WXSIZEOF(protocol_names), protocol_names);
	failcount += !Write(wxT("Hostname"), &CryptSocketProxySettings::GetHostname);
	failcount += !Write(wxT("Port"), &CryptSocketProxySettings::GetPort);
	failcount += !Write(wxT("Username"), &CryptSocketProxySettings::GetUsername);
	failcount += !Write(wxT("Password"), &CryptSocketProxySettings::GetPassword, false);
	failcount += !Write(wxT("Conditions/Connection Types/Server"), &CryptSocketProxySettings::GetConnectionType, pctServer);
	failcount += !Write(wxT("Conditions/Connection Types/DCC Connect"), &CryptSocketProxySettings::GetConnectionType, pctDCCConnect);
	failcount += !Write(wxT("Conditions/Connection Types/DCC Listen"), &CryptSocketProxySettings::GetConnectionType, pctDCCListen);
	failcount += !Write(wxT("Conditions/Destination Network/Mode"), &CryptSocketProxySettings::GetDestNetworkMode, WXSIZEOF(dest_modes), dest_modes);
	failcount += !Write(wxT("Conditions/Destination Network/Network"), &CryptSocketProxySettings::GetDestNetworkNetwork);
	failcount += !Write(wxT("Conditions/Destination Network/Subnet"), &CryptSocketProxySettings::GetDestNetworkSubnet);
	failcount += !Write(wxT("Conditions/Destination Ports/Mode"), &CryptSocketProxySettings::GetDestPortsMode, WXSIZEOF(dest_modes), dest_modes);
	failcount += !Write(wxT("Conditions/Destination Ports/Ranges"), &CryptSocketProxySettings::GetDestPortRanges);

	return (failcount == 0);

}

const wxString* const CryptSocketProxySettings::GetProtocolNames()
{
	return protocol_names;
}

size_t CryptSocketProxySettings::GetProtocolCount()
{
	return WXSIZEOF(protocol_names);
}

bool CryptSocketProxySettings::DoesProtocolSupportUsername(CryptSocketProxyProtocol protocol)
{

	switch (protocol)
	{

		case ppSOCKS4:
			return true;

		case ppSOCKS5:
			return true;

		case ppHTTP:
			return true;

		default:
			wxFAIL_MSG(wxT("Unknown protocol in CryptSocketProxySettings::DoesProtocolSupportUsername"));
			return false;

	}

}

bool CryptSocketProxySettings::DoesProtocolSupportPassword(CryptSocketProxyProtocol protocol)
{

	switch (protocol)
	{

		case ppSOCKS4:
			return false;

		case ppSOCKS5:
			return true;

		case ppHTTP:
			return true;

		default:
			wxFAIL_MSG(wxT("Unknown protocol in CryptSocketProxySettings::DoesProtocolSupportPassword"));
			return false;

	}

}

CryptSocketProxyProtocol CryptSocketProxySettings::ProtocolFromString(const wxString &protocol)
{
	for (size_t i = 0; i < WXSIZEOF(protocol_names); ++i)
	{
		if (protocol_names[i] == protocol)
		{
			return CryptSocketProxyProtocol(i);
		}
	}
	return ppUnknown;
}

wxString CryptSocketProxySettings::ProtocolToString(CryptSocketProxyProtocol protocol)
{
	wxASSERT(protocol >= 0 && protocol < WXSIZEOF(protocol_names));
	return protocol_names[protocol];
}

bool CryptSocketProxySettings::DoesProtocolSupportConnectionType(CryptSocketProxyProtocol protocol, CryptSocketProxyConnectionTypes type)
{

	switch (protocol)
	{

		case ppSOCKS4:
			return (type != pctDCCListen); //true; // DCC listens not supported yet

		case ppSOCKS5:
			return (type != pctDCCListen); //true; // DCC listens not supported yet

		case ppHTTP:
			return (type == pctServer);

		default:
			wxFAIL_MSG(wxT("Unknown protocol in CryptSocketProxySettings::DoesProtocolSupportConnectionType"));
			return false;

	}

}

bool CryptSocketProxySettings::DoesDestDestIPMatch(const wxString &dest_ip) const
{

	bool is_allow_only;

	switch (m_dest_network_mode)
	{

		case pdmAny:
			return true;

		case pdmAllowOnly:
			is_allow_only = true;
			break;

		case pdmExcludeOnly:
			is_allow_only = false;
			break;

		default:
			wxFAIL;
			return false;

	}

	if (!m_dest_network.Length() && !m_dest_subnet.Length())
	{
		return true;
	}

	wxUint32 ip = GetIPV4Address(dest_ip);
	wxUint32 network = GetIPV4Address(m_dest_network);
	wxUint32 subnet = GetIPV4Address(m_dest_subnet);

	wxCHECK_MSG(IsValidIPV4Address(ip), false, wxT("Invalid IP address"));
	wxCHECK_MSG(IsValidIPV4Address(network), false, wxT("Invalid network address"));
	wxCHECK_MSG(IsValidIPV4Address(subnet), false, wxT("Invalid subnet address"));

	bool ip_matches = ((ip & subnet) == network);

	return (ip_matches ^ (!is_allow_only));

}

bool CryptSocketProxySettings::DoesDestPortMatch(wxUint16 port) const
{

	bool is_allow_only;

	switch (m_dest_port_ranges_mode)
	{

		case pdmAny:
			return true;

		case pdmAllowOnly:
			is_allow_only = true;
			break;

		case pdmExcludeOnly:
			is_allow_only = false;
			break;

		default:
			wxFAIL;
			return false;

	}

	wxASSERT(m_dest_port_ranges_low.GetCount() == m_dest_port_ranges_high.GetCount());
	
	for (size_t i = 0; i < m_dest_port_ranges_low.GetCount(); ++i)
	{
		if (m_dest_port_ranges_low[i] <= m_dest_port_ranges_high[i])
		{
			if (port >= m_dest_port_ranges_low[i] && port <= m_dest_port_ranges_high[i])
			{
				return is_allow_only;
			}
		}
		else
		{
			if (port >= m_dest_port_ranges_high[i] && port <= m_dest_port_ranges_low[i])
			{
				return is_allow_only;
			}
		}
	}

	return !is_allow_only;

}

bool CryptSocketProxySettings::IsEnabledForConnectionType(CryptSocketProxyConnectionTypes type) const
{
	return
		GetEnabled() &&
		GetConnectionType(type) &&
		DoesProtocolSupportConnectionType(GetProtocol(), type);
}

bool CryptSocketProxySettings::GetEnabled() const
{
	return m_enabled;
}

CryptSocketProxyProtocol CryptSocketProxySettings::GetProtocol() const
{
	return m_protocol;
}

wxString CryptSocketProxySettings::GetHostname() const
{
	return m_hostname;
}

wxUint16 CryptSocketProxySettings::GetPort() const
{
	return m_port;
}

wxString CryptSocketProxySettings::GetUsername() const
{
	return m_username;
}

wxString CryptSocketProxySettings::GetPassword(bool decrypt) const
{
	return m_config.DecodePassword(m_encrypted_password, decrypt);
}

bool CryptSocketProxySettings::GetConnectionType(CryptSocketProxyConnectionTypes type) const
{
	wxASSERT(type >= 0 && type < 3);
	return m_connection_types[type];
}

CryptSocketProxyDestMode CryptSocketProxySettings::GetDestNetworkMode() const
{
	return m_dest_network_mode;
}

wxString CryptSocketProxySettings::GetDestNetworkNetwork() const
{
	return m_dest_network;
}

wxString CryptSocketProxySettings::GetDestNetworkSubnet() const
{
	return m_dest_subnet;
}

CryptSocketProxyDestMode CryptSocketProxySettings::GetDestPortsMode() const
{
	return m_dest_port_ranges_mode;
}

wxString CryptSocketProxySettings::GetDestPortRanges() const
{
	wxASSERT(m_dest_port_ranges_low.GetCount() == m_dest_port_ranges_high.GetCount());
	wxString str;
	for (size_t i = 0; i < m_dest_port_ranges_low.GetCount(); ++i)
	{
		if (i > 0)
		{
			str << wxT(',');
		}
		if (m_dest_port_ranges_low[i] == m_dest_port_ranges_high[i])
		{
			str << (int)m_dest_port_ranges_low[i];
		}
		else if (m_dest_port_ranges_low[i] < m_dest_port_ranges_high[i])
		{
			str << (int)m_dest_port_ranges_low[i] << wxT('-') << (int)m_dest_port_ranges_high[i];
		}
		else
		{
			str << (int)m_dest_port_ranges_high[i] << wxT('-') << (int)m_dest_port_ranges_low[i];
		}
	}
	return str;
}

bool CryptSocketProxySettings::SetEnabled(bool enabled)
{
	m_enabled = enabled;
	return true;
}

bool CryptSocketProxySettings::SetProtocol(CryptSocketProxyProtocol protocol)
{
	if (protocol >= 0 && (size_t)protocol < WXSIZEOF(protocol_names))
	{
		m_protocol = protocol;
		return true;
	}
	return false;
}

bool CryptSocketProxySettings::SetProtocol(const wxString &protocol)
{
	return SetProtocol(ProtocolFromString(protocol));
}

bool CryptSocketProxySettings::SetHostname(const wxString &hostname)
{
	m_hostname = hostname;
	return true;
}

bool CryptSocketProxySettings::SetPort(wxUint16 port)
{
	m_port = port;
	return true;
}

static bool ToUint16(const wxString &str, wxUint16 &ui16)
{
	unsigned long ul;
	if (str.ToULong(&ul) && ul < 0xffff)
	{
		ui16 = ul;
		return true;
	}
	return false;
}

bool CryptSocketProxySettings::SetPort(const wxString &port)
{
	return ToUint16(port, m_port);
}

bool CryptSocketProxySettings::SetUsername(const wxString &username)
{
	m_username = username;
	return true;
}

bool CryptSocketProxySettings::SetPassword(const wxString &password)
{
	if (password.Length())
	{
		wxString enc = m_config.EncodePassword(password);
		if (enc.Length())
		{
			m_encrypted_password = enc;
			return true;
		}
	}
	else
	{
		m_encrypted_password = wxEmptyString;
		return true;
	}
	return false;
}

bool CryptSocketProxySettings::SetConnectionType(CryptSocketProxyConnectionTypes type, bool enabled)
{
	if (type >= 0 && type < 3)
	{
		m_connection_types[type] = enabled;
		return true;
	}
	return false;
}

bool CryptSocketProxySettings::SetDestNetworkMode(CryptSocketProxyDestMode mode)
{
	if (mode >= 0 && mode < 3)
	{
		m_dest_network_mode = mode;
		return true;
	}
	return false;
}

bool CryptSocketProxySettings::SetDestNetworkNetwork(const wxString &network)
{
	if (network.Length() == 0 || IsValidIPV4Address(GetIPV4Address(network)))
	{
		m_dest_network = network;
		return true;
	}
	return false;
}

bool CryptSocketProxySettings::SetDestNetworkSubnet(const wxString &subnet)
{
	if (subnet.Length() == 0 || IsValidIPV4Address(GetIPV4Address(subnet)))
	{
		m_dest_subnet = subnet;
		return true;
	}
	return false;
}

bool CryptSocketProxySettings::SetDestPortsMode(CryptSocketProxyDestMode mode)
{
	if (mode >= 0 && mode < 3)
	{
		m_dest_port_ranges_mode = mode;
		return true;
	}
	return false;
}

bool CryptSocketProxySettings::SetDestPortRanges(const wxString &port_ranges)
{
	Uint16Array low_values, high_values;
	wxArrayString ranges = SplitString(port_ranges, wxT(","));
	for (size_t i = 0; i < ranges.GetCount(); ++i)
	{
		wxArrayString low_high = SplitString(ranges[i], wxT("-"));
		if (low_high.GetCount() == 1)
		{
			wxUint16 x;
			if (!ToUint16(low_high[0], x))
			{
				return false;
			}
			low_values.Add(x);
			high_values.Add(x);
		}
		else if (low_high.GetCount() == 2)
		{
			wxUint16 x, y;
			if (!ToUint16(low_high[0], x) || !ToUint16(low_high[1], y))
			{
				return false;
			}
			if (x < y)
			{
				low_values.Add(x);
				high_values.Add(y);
			}
			else
			{
				low_values.Add(y);
				high_values.Add(x);
			}
		}
		else
		{
			return false;
		}
	}
	m_dest_port_ranges_low = low_values;
	m_dest_port_ranges_high = high_values;
	wxASSERT(m_dest_port_ranges_low.GetCount() == m_dest_port_ranges_high.GetCount());
	return true;
}

CryptSocketProxy* CryptSocketProxySettings::NewProxyConnect(CryptSocketBase *sck, const wxString &ip, const wxUint16 port) const
{
	
	wxASSERT(sck);

	CryptSocketProxy *proxy;

	switch (GetProtocol())
	{

		case ppSOCKS4:
			proxy = new CryptSocketProxySOCKS4(sck, true);
			break;

		case ppSOCKS5:
			proxy = new CryptSocketProxySOCKS5(sck, true);
			break;

		case ppHTTP:
			proxy = new CryptSocketProxyHTTP(sck);
			break;

		default:
			wxFAIL_MSG(wxT("Unsupported protocol in CryptSocketProxySettings::NewProxyConnect"));
			return NULL;

	}

	wxASSERT(ip.Length() && port > 0);
	proxy->m_dest_ip = ip;
	proxy->m_dest_port = port;

	return proxy;

}

CryptSocketProxy* CryptSocketProxySettings::NewProxyListen(CryptSocketBase *WXUNUSED_UNLESS_DEBUG(sck)) const
{

	wxASSERT(sck);

//	switch (GetProtocol())
//	{

//		case ppSOCKS4:
//			return new CryptSocketProxySOCKS4(sck, false);

//		case ppSOCKS5:
//			return new CryptSocketProxySOCKS5(sck, false);

//		default:
			wxFAIL_MSG(wxT("Unsupported protocol in CryptSocketProxySettings::NewProxyListen"));
			return NULL;

//	}

}

//////// CryptSocketProxy ////////

CryptSocketProxy::CryptSocketProxy(CryptSocketBase *sck)
	: m_sck(sck), m_settings(*sck->GetProxySettings())
{
	m_dest_ip = wxEmptyString;
	m_dest_port = 0u;
}

CryptSocketProxy::~CryptSocketProxy()
{
}

void CryptSocketProxy::ForwardInputToClient(const ByteBuffer &data)
{
	m_sck->OnProxyInput(data);
}

void CryptSocketProxy::SendData(const ByteBuffer &data)
{
	m_sck->ProxySendData(data);
}

void CryptSocketProxy::ConnectionError(const wxString &msg)
{
	m_sck->OnSocketConnectionError(wxT("Proxy error: ") + msg);
}

void CryptSocketProxy::ProxyConnected(const ByteBuffer &data_to_forward)
{
	m_sck->OnConnectedToRemote();
	ForwardInputToClient(data_to_forward);
}
