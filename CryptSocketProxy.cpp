#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: CryptSocketProxy.cpp,v 1.2 2003-05-31 07:36:56 jason Exp $)

#include "CryptSocketProxy.h"

static const wxString protocol_names[] =
	{ wxT("SOCKS 4"), wxT("SOCKS 5"), wxT("HTTP CONNECT") };

CryptSocketProxySettings::CryptSocketProxySettings(Config &m_config);

bool CryptSocketProxySettings::LoadSettings();
bool CryptSocketProxySettings::SaveSettings();

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

bool CryptSocketProxySettings::DoesDestDestIPMatch(const wxString &dest_ip) const;
bool CryptSocketProxySettings::DoesDestPortMatch(wxUint16 port) const;

bool CryptSocketProxySettings::GetEnabled() const;
CryptSocketProxyProtocol CryptSocketProxySettings::GetProtocol() const;
wxString CryptSocketProxySettings::GetHostname() const;
wxUint16 CryptSocketProxySettings::GetPort() const;
wxString CryptSocketProxySettings::GetUsername() const;
wxString CryptSocketProxySettings::GetPassword(bool decrypt) const;
bool CryptSocketProxySettings::GetConnectionType(CryptSocketProxyConnectionTypes type) const;
CryptSocketProxyDestMode CryptSocketProxySettings::GetDestNetworkMode() const;
wxString CryptSocketProxySettings::GetDestNetworkNetwork() const;
wxString CryptSocketProxySettings::GetDestNetworkSubnet() const;
CryptSocketProxyDestMode CryptSocketProxySettings::GetDestPortsMode() const;
wxString CryptSocketProxySettings::GetDestPortRanges() const;

bool CryptSocketProxySettings::SetEnabled(bool enabled);
bool CryptSocketProxySettings::SetProtocol(CryptSocketProxyProtocol protocol);
bool CryptSocketProxySettings::SetProtocol(const wxString &protocol);
bool CryptSocketProxySettings::SetHostname(const wxString &hostname);
bool CryptSocketProxySettings::SetPort(wxUint16 port);
bool CryptSocketProxySettings::SetUsername(const wxString &username);
bool CryptSocketProxySettings::SetPassword(const wxString &password);
bool CryptSocketProxySettings::SetConnectionType(CryptSocketProxyConnectionTypes type, bool enabled);
bool CryptSocketProxySettings::SetDestNetworkMode(CryptSocketProxyDestMode mode);
bool CryptSocketProxySettings::SetDestNetworkNetwork(const wxString &network);
bool CryptSocketProxySettings::SetDestNetworkSubnet(const wxString &subnet);
bool CryptSocketProxySettings::SetDestPortsMode(CryptSocketProxyDestMode mode);
bool CryptSocketProxySettings::SetDestPortRanges(const wxString &port_ranges);
