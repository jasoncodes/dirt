#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: ClientDefault.cpp,v 1.15 2003-02-17 14:10:11 jason Exp $)

#include "ClientDefault.h"
#include "Modifiers.h"
#include "CryptSocket.h"
#include "util.h"

#define ASSERT_CONNECTED() { if (!IsConnected()) { m_event_handler->OnClientWarning(wxEmptyString, wxT("Not connected")); return; } }

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

	Debug(wxEmptyString, wxT("ClientDefault Ready"));

	wxArrayString users(true);
	users.Add(wxT("First"));
	users.Add(wxT("Third"));
	users.Add(wxT("Second"));
	m_event_handler->OnClientUserList(users);
	m_event_handler->OnClientUserJoin(wxT("Fourth"), wxT("fourth@dev.null"));
	m_event_handler->OnClientUserPart(wxT("Fourth"), wxT("fourth@dev.null"), wxT("Quit message goes here"));

}

ClientDefault::~ClientDefault()
{
	delete m_sck;
}

void ClientDefault::SendMessage(const wxString &context, const wxString &nick, const wxString &message)
{
	ASSERT_CONNECTED();
	ByteBuffer msg;
	if (nick.Length() > 0)
	{
		msg = EncodeMessage(context, "PRIVMSG", Pack(nick, message));
	}
	else
	{
		msg = EncodeMessage(context, "PUBMSG", message);
	}
	m_sck->Send(msg);
}

wxString ClientDefault::GetNickname()
{
	return wxT("LOCALHOST");
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
}

bool ClientDefault::IsConnected()
{
	return m_sck->Ok();
}

void ClientDefault::OnSocket(CryptSocketEvent &event)
{

	switch (event.GetSocketEvent())
	{

		case CRYPTSOCKET_CONNECTION:
			m_event_handler->OnClientInformation(wxEmptyString, wxT("Connected"));
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
