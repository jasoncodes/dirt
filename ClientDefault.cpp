#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: ClientDefault.cpp,v 1.40 2003-06-27 11:34:44 jason Exp $)

#include "ClientDefault.h"
#include "DNS.h"
#include "Modifiers.h"
#include "CryptSocket.h"
#include "util.h"
#include "IPInfo.h"
#include "CryptSocketProxy.h"

enum
{
	ID_SOCKET = 200,
	ID_DNS
};

BEGIN_EVENT_TABLE(ClientDefault, Client)
	EVT_CRYPTSOCKET(ID_SOCKET, ClientDefault::OnSocket)
	EVT_DNS(ID_DNS, ClientDefault::OnDNS)
END_EVENT_TABLE()

ClientDefault::ClientDefault(ClientEventHandler *event_handler)
	: Client(event_handler)
{
	m_sck = new CryptSocketClient;
	m_sck->SetEventHandler(this, ID_SOCKET);
	NewProxySettings();
	m_dns = new DNS;
	m_dns->SetEventHandler(this, ID_DNS);
}

ClientDefault::~ClientDefault()
{
	delete m_sck;
	delete m_dns;
}

void ClientDefault::NewProxySettings()
{
	CryptSocketProxySettings proxy_settings(m_config);
	if (proxy_settings.IsEnabledForConnectionType(pctServer))
	{
		m_sck->SetProxySettings(&proxy_settings);
	}
	else
	{
		m_sck->SetProxySettings(NULL);
	}
}

const CryptSocketProxySettings ClientDefault::GetProxySettings() const
{
	return CryptSocketProxySettings(const_cast<ClientConfig&>(m_config));
}

void ClientDefault::SendMessage(const wxString &context, const wxString &nick, const wxString &message, bool is_action)
{
	ASSERT_CONNECTED();
	ByteBuffer msg;
	wxString type = is_action?wxT("ACTION"):wxT("MSG");
	if (nick.Length() > 0)
	{
		msg = EncodeMessage(context, wxT("PRIV") + type, Pack(nick, message));
	}
	else
	{
		msg = EncodeMessage(context, wxT("PUB") + type, message);
	}
	m_sck->Send(msg);
}

void ClientDefault::SetNickname(const wxString &context, const wxString &nickname)
{
	m_sck->Send(EncodeMessage(context, wxT("NICK"), nickname));
}

bool ClientDefault::Connect(const URL &url)
{
	m_sck->Close();
	m_dns->Cancel();
	if (url.GetProtocol(wxT("dirt")) != wxT("dirt"))
	{
		return false;
	}
	if (url.GetHostname().Length() == 0)
	{
		return false;
	}
	m_server_name = wxEmptyString;
	m_url = url;
	wxIPV4address addr;
	if (m_dns->Lookup(m_url.GetHostname()))
	{
		wxString msg;
		msg << wxT("Looking up ") << m_url.GetHostname();
		m_event_handler->OnClientInformation(wxEmptyString, msg);
		return true;
	}
	return false;
}

wxString ClientDefault::GetLastURLString() const
{
	wxString str;
	str << m_url.GetHostname();
	if (m_url.GetPort(11626) != 11626)
	{
		str << wxT(':') << m_url.GetPort();
	}
	return str;
}

void ClientDefault::OnDNS(DNSEvent &event)
{
	bool ok = event.IsSuccess();
	if (ok)
	{
		m_last_server_hostname = GetIPV4AddressString(event.GetIP());
		wxString msg;
		msg << wxT("Connecting to ") << GetLastURLString();
		m_event_handler->OnClientInformation(wxEmptyString, msg);
		m_sck->Connect(m_last_server_hostname, m_url.GetPort(11626));
	}
	if (!ok)
	{
		m_event_handler->OnClientWarning(wxEmptyString, wxT("Error resolving ") + event.GetHostname());
		m_event_handler->OnClientStateChange();
	}
}

void ClientDefault::Disconnect(const wxString &msg)
{
	m_sck->Close();
	m_dns->Cancel();
	m_contact_self = NULL;
	EmptyContacts();
	m_event_handler->OnClientInformation(wxEmptyString, msg);
	m_event_handler->OnClientStateChange();
}

bool ClientDefault::IsConnected() const
{
	return m_sck->Ok() && m_tmrPing->IsRunning();
}

const URL& ClientDefault::GetLastURL() const
{
	return m_url;
}

wxString ClientDefault::GetLastHostname() const
{
	return m_last_server_hostname;
}

void ClientDefault::SendToServer(const ByteBuffer &msg)
{
	wxString context = Unpack(msg, 2).Item(0);
	ASSERT_CONNECTED()
	m_sck->Send(msg);
}

void ClientDefault::Authenticate(const ByteBuffer &auth)
{
	try
	{
		ByteBuffer digest = Crypt::MD5MACDigest(m_authkey, auth);
		m_sck->Send(EncodeMessage(wxEmptyString, wxT("AUTH"), digest));
	}
	catch (...)
	{
		m_event_handler->OnClientWarning(wxEmptyString, wxT("Error encoding authentication data"));
	}
}

void ClientDefault::Oper(const wxString &context, const wxString &pass)
{
	ASSERT_CONNECTED();
	try
	{
		ByteBuffer digest = Crypt::MD5MACDigest(m_authkey, pass);
		SendToServer(EncodeMessage(context, wxT("OPER"), digest));
	}
	catch (...)
	{
		m_event_handler->OnClientWarning(context, wxT("Error encoding authentication data"));
	}
}

void ClientDefault::OnSocket(CryptSocketEvent &event)
{

	switch (event.GetSocketEvent())
	{

		case CRYPTSOCKET_CONNECTION:
			OnConnect();
			break;

		case CRYPTSOCKET_LOST:
		case CRYPTSOCKET_ERROR:
			{
				wxString msg;
				if (event.GetSocketEvent() == CRYPTSOCKET_LOST)
				{
					msg << wxT("Connection lost");
				}
				else if (event.GetSocketEvent() == CRYPTSOCKET_ERROR)
				{
					msg << wxT("Error connecting to ") << GetLastURLString();
				}
				if (event.GetData().Length())
				{
					if (msg.Length())
					{
						msg << wxT(" (") << event.GetData() << wxT(")");
					}
					else
					{
						msg << event.GetData();
					}
				}
				Disconnect(msg);
			}
			break;

		case CRYPTSOCKET_INPUT:
			ProcessServerInput(event.GetData());
			break;

		case CRYPTSOCKET_OUTPUT:
			break;

		default:
			wxFAIL_MSG(wxT("Unexpected message in ClientDefault::OnSocket"));

	}

}

bool ClientDefault::ProcessServerInputExtra(bool preprocess, const wxString &context, const wxString &cmd, const ByteBuffer &data)
{
	if (preprocess)
	{
		if (cmd == wxT("AUTHSEED"))
		{
			m_authkey = data;
			return true;
		}
		else if (cmd == wxT("AUTH"))
		{
			m_event_handler->OnClientAuthNeeded(data);
			return true;
		}
		else if (cmd == wxT("AUTHOK"))
		{
			m_event_handler->OnClientAuthDone(data);
			return true;
		}
		else if (cmd == wxT("AUTHBAD"))
		{
			m_event_handler->OnClientAuthBad(data);
			return true;
		}
		else if (cmd == wxT("PING"))
		{
			if (IsConnected())
			{
				SendToServer(EncodeMessage(context, wxT("PONG"), data));
			}
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}

ByteBuffer ClientDefault::GetKeyLocalPublic() const
{
	if (m_sck)
	{
		return m_sck->GetKeyLocalPublic();
	}
	return ByteBuffer();
}

ByteBuffer ClientDefault::GetKeyLocalPrivate() const
{
	if (m_sck)
	{
		return m_sck->GetKeyLocalPrivate();
	}
	return ByteBuffer();
}
