#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: ClientDefault.cpp,v 1.11 2003-02-16 11:29:39 jason Exp $)

#include "ClientDefault.h"
#include "Modifiers.h"
#include "CryptSocket.h"

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

void ClientDefault::SendMessage(const wxString &nick, const wxString &message)
{
	//m_event_handler->OnClientMessageOut(nick, message);
	//m_event_handler->OnClientMessageIn(
	//	is_private?nick:wxT("EVERYONE"), 
	//	wxString() << wxT("You sent me \"") << message << (wxChar)OriginalModifier << wxT("\""),
	//	is_private);
	ASSERT_CONNECTED();
	if (nick.Length() > 0)
	{
		m_sck->Send(wxString() << wxT("PRIVMSG \"") << nick << wxT("\" ") << message);
	}
	else
	{
		m_sck->Send(wxString() << wxT("PUBMSG ") << message);
	}
}

wxString ClientDefault::GetNickname()
{
	return wxT("LOCALHOST");
}

bool ClientDefault::Connect(const wxString &url)
{
	m_event_handler->OnClientDebug(wxEmptyString, wxT("Note: Connect does not support parameters yet, using localhost:11626"));
	wxIPV4address addr;
	addr.Hostname("127.0.0.1");
	addr.Service(11626);
	m_sck->Connect(addr);
	m_event_handler->OnClientInformation(wxEmptyString, wxT("Connecting to localhost:11626"));
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
			m_event_handler->OnClientDebug(wxEmptyString, wxT("CRYPTSOCKET_INPUT: ") + event.GetData());
			break;

		case CRYPTSOCKET_OUTPUT:
			m_event_handler->OnClientDebug(wxEmptyString, wxT("CRYPTSOCKET_OUTPUT"));
			break;

		default:
			wxFAIL_MSG(wxT("Unexpected message in ClientDefault::OnSocket"));

	}

}
