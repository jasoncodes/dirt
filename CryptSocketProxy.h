#ifndef CryptSocketProxy_H_
#define CryptSocketProxy_H_

#include "util.h"
#include "ConfigFile.h"

enum CryptSocketProxyProtocol
{
	ppHTTP,
	ppSOCKS4,
	ppSOCKS5
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
	CryptSocketProxySettings(Config &m_config);

	bool LoadSettings();
	bool SaveSettings();

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
	bool SetHostname(const wxString &hostname);
	bool SetPort(wxUint16 port);
	bool SetUsername(const wxString &username);
	bool SetPassword(const wxString &password);
	bool SetConnectionType(CryptSocketProxyConnectionTypes type, bool enabled);
	bool SetDestNetworkMode(CryptSocketProxyDestMode mode);
	bool SetDestNetworkNetwork(const wxString &network);
	bool SetDestNetworkSubnet(const wxString &subnet);
	bool SetDestPortsMode(CryptSocketProxyDestMode mode);
	bool SetDestPortRanges(const wxString &port_ranges);

	bool DoesDestDestIPMatch(const wxString &dest_ip) const;
	bool DoesDestPortMatch(wxUint16 port) const;

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
	Uint16Array m_port_ranges_low;
	Uint16Array m_port_ranges_high;
	
};

class CryptSocketProxy
{
};

/*class CryptSocketProxyNull : public CryptSocketProxy
{
};

class CryptSocketProxyHTTP : public CryptSocketProxy
{
};

class CryptSocketProxySOCKS4 : public CryptSocketProxy
{
};

class CryptSocketProxySOCKS5 : public CryptSocketProxy
{
};*/

#endif
