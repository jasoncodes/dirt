#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: CryptSocketProxy.cpp,v 1.7 2003-06-02 08:00:12 jason Exp $)

#include "CryptSocketProxy.h"
#include "IPInfo.h"

static const wxString protocol_names[] =
	{ wxT("SOCKS 4"), wxT("SOCKS 5"), wxT("HTTP CONNECT") };

CryptSocketProxySettings::CryptSocketProxySettings(Config &config)
	: m_config(config)
{
	LoadDefaults();
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
//	failcount += !Read(wxT("Protocol"), &CryptSocketProxySettings::SetProtocol);
//	failcount += !Read(wxT("Hostname"), &CryptSocketProxySettings::SetHostname);
//	failcount += !Read(wxT("Port"), &CryptSocketProxySettings::SetPort);
//	failcount += !Read(wxT("Username"), &CryptSocketProxySettings::SetUsername);
//	failcount += !Read(wxT("Password"), &CryptSocketProxySettings::SetPassword);
	failcount += !Read(wxT("Conditions/Connection Types/Server"), &CryptSocketProxySettings::SetConnectionType, pctServer);
	failcount += !Read(wxT("Conditions/Connection Types/DCC Connect"), &CryptSocketProxySettings::SetConnectionType, pctDCCConnect);
	failcount += !Read(wxT("Conditions/Connection Types/DCC Listen"), &CryptSocketProxySettings::SetConnectionType, pctDCCListen);
//	failcount += !Read(wxT("Conditions/Destination Network/Mode"), &CryptSocketProxySettings::SetDestNetworkMode);
//	failcount += !Read(wxT("Conditions/Destination Network/Network"), &CryptSocketProxySettings::SetDestNetworkNetwork);
//	failcount += !Read(wxT("Conditions/Destination Network/Subnet"), &CryptSocketProxySettings::SetDestNetworkSubnet);
//	failcount += !Read(wxT("Conditions/Destination Ports/Mode"), &CryptSocketProxySettings::SetDestPortsMode);
//	failcount += !Read(wxT("Conditions/Destination Ports/Ranges"), &CryptSocketProxySettings::SetDestPortRanges);

	return (failcount == 0);

}

bool CryptSocketProxySettings::SaveSettings()
{

	int failcount = 0;

	failcount += !Write(wxT("Enabled"), &CryptSocketProxySettings::GetEnabled);
	failcount += !Write(wxT("Protocol"), &CryptSocketProxySettings::GetProtocol);
	failcount += !Write(wxT("Hostname"), &CryptSocketProxySettings::GetHostname);
	failcount += !Write(wxT("Port"), &CryptSocketProxySettings::GetPort);
	failcount += !Write(wxT("Username"), &CryptSocketProxySettings::GetUsername);
	failcount += !Write(wxT("Password"), &CryptSocketProxySettings::GetPassword, false);
	failcount += !Write(wxT("Conditions/Connection Types/Server"), &CryptSocketProxySettings::GetConnectionType, pctServer);
	failcount += !Write(wxT("Conditions/Connection Types/DCC Connect"), &CryptSocketProxySettings::GetConnectionType, pctDCCConnect);
	failcount += !Write(wxT("Conditions/Connection Types/DCC Listen"), &CryptSocketProxySettings::GetConnectionType, pctDCCListen);
	failcount += !Write(wxT("Conditions/Destination Network/Mode"), &CryptSocketProxySettings::GetDestNetworkMode);
	failcount += !Write(wxT("Conditions/Destination Network/Network"), &CryptSocketProxySettings::GetDestNetworkNetwork);
	failcount += !Write(wxT("Conditions/Destination Network/Subnet"), &CryptSocketProxySettings::GetDestNetworkSubnet);
	failcount += !Write(wxT("Conditions/Destination Ports/Mode"), &CryptSocketProxySettings::GetDestPortsMode);
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

bool CryptSocketProxySettings::DoesProtocolSupportAuthentication(CryptSocketProxyProtocol protocol)
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
			wxFAIL_MSG(wxT("Unknown protocol in CryptSocketProxySettings::DoesProtocolSupportAuthentication"));
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
		case ppSOCKS5:
			return true;

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
	if (protocol >= 0 && protocol < WXSIZEOF(protocol_names))
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
