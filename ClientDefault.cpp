/*
    Copyright 2002, 2003 General Software Laboratories
    
    
    This file is part of Dirt Secure Chat.

    Dirt Secure Chat is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Dirt Secure Chat is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Dirt Secure Chat; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: ClientDefault.cpp,v 1.47 2005-01-08 02:25:48 jason Exp $)

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

bool ClientDefault::Connect(const URL &url, bool is_reconnect, bool restore_away_message)
{
	wxASSERT(!is_reconnect || url == GetLastURL());
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
	if (!restore_away_message)
	{
		m_is_away = false;
		m_away_message.Empty();
	}
	if (!is_reconnect)
	{
		m_last_auth = ByteBuffer();
	}
	m_server_name = wxEmptyString;
	m_url = url;
	wxString msg;
	msg << wxT("Looking up ") << m_url.GetHostname();
	m_event_handler->OnClientInformation(wxEmptyString, msg);
	m_dns->Lookup(m_url.GetHostname());
	return true;
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
	return m_sck->Ok();
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
		if (auth.Length())
		{
			m_last_auth = m_config.EncodePassword(auth);
		}
		else
		{
			m_last_auth = ByteBuffer();
		}
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
			ByteBuffer auth = m_config.DecodePassword(m_last_auth, true);
			if (auth.Length())
			{
				Authenticate(auth);
			}
			else
			{
				m_event_handler->OnClientAuthNeeded(data);
			}
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
