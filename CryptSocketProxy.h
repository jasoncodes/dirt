#ifndef CryptSocketProxy_H_
#define CryptSocketProxy_H_

#include "util.h"
#include "ConfigFile.h"

class CryptSocketBase;
class CryptSocketProxy;

enum CryptSocketProxyProtocol
{
	ppUnknown = -1,
	ppSOCKS4,
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
	static bool DoesProtocolSupportUsername(CryptSocketProxyProtocol protocol);
	static bool DoesProtocolSupportPassword(CryptSocketProxyProtocol protocol);
	static CryptSocketProxyProtocol ProtocolFromString(const wxString &protocol);
	static wxString ProtocolToString(CryptSocketProxyProtocol protocol);
	static bool DoesProtocolSupportConnectionType(CryptSocketProxyProtocol protocol, CryptSocketProxyConnectionTypes type);

	bool IsEnabledForConnectionType(CryptSocketProxyConnectionTypes type) const;
	bool DoesDestDestIPMatch(const wxString &dest_ip) const;
	bool DoesDestPortMatch(wxUint16 port) const;

	CryptSocketProxy* NewProxyConnect(CryptSocketBase *sck, const wxString &ip, const wxUint16 port) const;
	CryptSocketProxy* NewProxyListen(CryptSocketBase *sck) const;

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

	inline bool Read(const wxString &key, bool (CryptSocketProxySettings::*fn)(const wxString&), const wxString &dummy)
	{
		wxString x;
		bool b = m_config.m_config->Read(m_config.m_path + wxT("/Proxy/") + key, &x);
		if (b)
		{
			return (this->*fn)(x);
		}
		return true;
	}

	template <typename T>
	inline bool Read(const wxString &key, bool (CryptSocketProxySettings::*fn)(CryptSocketProxyConnectionTypes, T), CryptSocketProxyConnectionTypes type)
	{
		T x;
		bool b = m_config.m_config->Read(m_config.m_path + wxT("/Proxy/") + key, &x);
		if (b)
		{
			return (this->*fn)(type, x);
		}
		return true;
	}

	template <typename T>
	inline bool Read(const wxString &key, bool (CryptSocketProxySettings::*fn)(T), size_t num, const wxString *const values)
	{
		wxString str;
		bool b = m_config.m_config->Read(m_config.m_path + wxT("/Proxy/") + key, &str);
		if (b)
		{
			for (size_t i = 0; i < num; ++i)
			{
				if (values[i] == str)
				{
					return (this->*fn)((T)i);
				}
			}
			return false;
		}
		return true;
	}

	template <typename T>
	inline bool Write(const wxString &key, T (CryptSocketProxySettings::*fn)() const)
	{
		return m_config.m_config->Write(m_config.m_path + wxT("/Proxy/") + key, (this->*fn)());
	}

	template <typename T, typename U>
	inline bool Write(const wxString &key, T (CryptSocketProxySettings::*fn)(U) const, U x)
	{
		return m_config.m_config->Write(m_config.m_path + wxT("/Proxy/") + key, (this->*fn)(x));
	}

	template <typename T>
	inline bool Write(const wxString &key, T (CryptSocketProxySettings::*fn)() const, size_t num, const wxString *const values)
	{
		size_t i = (size_t)(this->*fn)();
		if (i >= 0 && i <= num)
		{
			return m_config.m_config->Write(m_config.m_path + wxT("/Proxy/") + key, values[i]);
		}
		return false;
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

	friend class CryptSocketProxySettings;

public:
	CryptSocketProxy(CryptSocketBase *sck);
	virtual ~CryptSocketProxy();

	virtual void OnConnect() = 0;
	virtual void OnInput(const ByteBuffer &data) = 0;
	virtual bool IsConnectedToRemote() const = 0;

protected:
	virtual void ForwardInputToClient(const ByteBuffer &data);
	virtual void SendData(const ByteBuffer &data);
	virtual void ConnectionError(const wxString &msg);

protected:
	CryptSocketBase *m_sck;
	const CryptSocketProxySettings &m_settings;
	wxString m_dest_ip;
	wxUint16 m_dest_port;

};

#endif
