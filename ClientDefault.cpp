#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: ClientDefault.cpp,v 1.21 2003-02-21 07:53:13 jason Exp $)

#include "ClientDefault.h"
#include "Modifiers.h"
#include "CryptSocket.h"
#include "util.h"

enum
{
	ID_SOCKET = 1,
};

BEGIN_EVENT_TABLE(ClientDefault, Client)
	EVT_CRYPTSOCKET(ID_SOCKET, ClientDefault::OnSocket)
END_EVENT_TABLE()

ClientDefault::ClientDefault(ClientEventHandler *event_handler)
	: Client(event_handler)
{

	m_sck = new CryptSocketClient;
	m_sck->SetEventHandler(this, ID_SOCKET);

	Debug(wxEmptyString, AppTitle());

}

ClientDefault::~ClientDefault()
{
	delete m_sck;
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
	m_sck->Send(EncodeMessage(context, "NICK", nickname));
}

bool ClientDefault::Connect(const URL &url)
{
	if (url.GetProtocol("dirt") != "dirt")
	{
		return false;
	}
	if (url.GetHostname().Length() == 0)
	{
		return false;
	}
	m_url = url;
	wxIPV4address addr;
	addr.Hostname(url.GetHostname());
	addr.Service(url.GetPort(11626));
	m_sck->Connect(addr);
	m_event_handler->OnClientInformation(wxEmptyString, wxString() << wxT("Connecting to ") << url.GetHostname() << wxT(':') << url.GetPort(11626));
	return true;
}

void ClientDefault::Disconnect()
{
	m_sck->Close();
	m_event_handler->OnClientInformation(wxEmptyString, wxT("Disconnected"));
	m_event_handler->OnClientStateChange();
}

bool ClientDefault::IsConnected()
{
	return m_sck->Ok();
}

const URL& ClientDefault::GetLastURL()
{
	return m_url;
}

void ClientDefault::SendToServer(const ByteBuffer &msg)
{
	ASSERT_CONNECTED()
	m_sck->Send(msg);
}

void ClientDefault::Authenticate(const ByteBuffer &auth)
{
	try
	{
		ByteBuffer digest = Crypt::MD5MACDigest(m_authkey, auth);
		m_sck->Send(EncodeMessage(wxEmptyString, "AUTH", digest));
	}
	catch (...)
	{
		m_event_handler->OnClientWarning(wxEmptyString, "Error encoding authentication data");
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
			Disconnect();
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
