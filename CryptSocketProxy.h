#ifndef CryptSocketProxy_H_
#define CryptSocketProxy_H_

#include "util.h"
#include "ConfigFile.h"

enum CryptSocketProxyProtocol
{
	ppUnknown = -1,
	ppSOCKS4 = 0,
	ppSOCKS5,
	ppHTTP
};

enum CryptSocketProxyDestMode
{
	pdmAny,
	pdmAllowOnly,
	pdmExcludeOnly
};

enum CryptSocketProxyConnectionTypes
{
	pctServer,
	pctDCCConnect,
	pctDCCListen
};

class CryptSocketProxySettings
{

public:
	CryptSocketProxySettings(Config &config);

	void LoadDefaults();
	bool LoadSettings();
	bool SaveSettings();

	static const wxString* const GetProtocolNames();
	static size_t GetProtocolCount();
	static bool DoesProtocolSupportAuthentication(CryptSocketProxyProtocol protocol);
	static CryptSocketProxyProtocol ProtocolFromString(const wxString &protocol);
	static wxString ProtocolToString(CryptSocketProxyProtocol protocol);
	static bool DoesProtocolSupportConnectionType(CryptSocketProxyProtocol protocol, CryptSocketProxyConnectionTypes type);

	bool DoesDestDestIPMatch(const wxString &dest_ip) const;
	bool DoesDestPortMatch(wxUint16 port) const;

	bool GetEnabled() const;
	CryptSocketProxyProtocol GetProtocol() const;
	wxString GetHostname() const;
	wxUint16 GetPort() const;
	wxString GetUsername() const;
	wxString GetPassword(bool decrypt) const;
	bool GetConnectionType(CryptSocketProxyConnectionTypes type) const;
	CryptSocketProxyDestMode GetDestNetworkMode() const;
	wxString GetDestNetworkNetwork() const;
	wxString GetDestNetworkSubnet() const;
	CryptSocketProxyDestMode GetDestPortsMode() const;
	wxString GetDestPortRanges() const;

	bool SetEnabled(bool enabled);
	bool SetProtocol(CryptSocketProxyProtocol protocol);
	bool SetProtocol(const wxString &protocol);
	bool SetHostname(const wxString &hostname);
	bool SetPort(wxUint16 port);
	bool SetPort(const wxString &port);
	bool SetUsername(const wxString &username);
	bool SetPassword(const wxString &password);
	bool SetConnectionType(CryptSocketProxyConnectionTypes type, bool enabled);
	bool SetDestNetworkMode(CryptSocketProxyDestMode mode);
	bool SetDestNetworkNetwork(const wxString &network);
	bool SetDestNetworkSubnet(const wxString &subnet);
	bool SetDestPortsMode(CryptSocketProxyDestMode mode);
	bool SetDestPortRanges(const wxString &port_ranges);

protected:
	template <typename T>
	inline bool Read(const wxString &key, bool (CryptSocketProxySettings::*fn)(T))
	{
		T x;
		bool b = m_config.m_config->Read(m_config.m_path + wxT("/Proxy/") + key, &x);
		if (b)
		{
			return (this->*fn)(x);
		}
		return true;
	}

	template <typename T>
	inline bool Write(const wxString &key, T (CryptSocketProxySettings::*fn)() const)
	{
		return m_config.m_config->Write(m_config.m_path + wxT("/Proxy/") + key, (this->*fn)());
	}

protected:
	Config &m_config;

	bool m_enabled;
	CryptSocketProxyProtocol m_protocol;
	wxString m_hostname;
	wxUint16 m_port;
	wxString m_username;
	wxString m_encrypted_password;
	bool m_connection_types[3];
	CryptSocketProxyDestMode m_dest_network_mode;
	wxString m_dest_network;
	wxString m_dest_subnet;
	CryptSocketProxyDestMode m_dest_port_ranges_mode;
	Uint16Array m_dest_port_ranges_low;
	Uint16Array m_dest_port_ranges_high;
	
};

class CryptSocketProxy
{
};

/*
class CryptSocketProxyNull : public CryptSocketProxy
{
};

class CryptSocketProxySOCKS4 : public CryptSocketProxy
{
};

class CryptSocketProxySOCKS5 : public CryptSocketProxy
{
};

class CryptSocketProxyHTTP : public CryptSocketProxy
{
};
*/

#endif
