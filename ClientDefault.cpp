#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: ClientDefault.cpp,v 1.10 2003-02-16 05:09:02 jason Exp $)

#include "ClientDefault.h"
#include "Modifiers.h"

enum
{
	ID_TIMER_TEST = 1
};

BEGIN_EVENT_TABLE(ClientDefault, Client)
	EVT_TIMER(ID_TIMER_TEST, ClientDefault::OnTestTimer)
END_EVENT_TABLE()

ClientDefault::ClientDefault(ClientEventHandler *event_handler)
	: Client(event_handler)
{

	tmrTest = new wxTimer(this, ID_TIMER_TEST);
	//tmrTest->Start(5000);

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
	delete tmrTest;
}

void ClientDefault::SendMessage(const wxString &nick, const wxString &message)
{
	bool is_private = (nick.Length() > 0);
	m_event_handler->OnClientMessageOut(nick, message);
	m_event_handler->OnClientMessageIn(
		is_private?nick:wxT("EVERYONE"), 
		wxString() << wxT("You sent me \"") << message << (wxChar)OriginalModifier << wxT("\""),
		is_private);
}

wxString ClientDefault::GetNickname()
{
	return wxT("LOCALHOST");
}

void ClientDefault::OnTestTimer(wxTimerEvent &event)
{
	m_event_handler->OnClientDebug(wxEmptyString, wxT("This is a test timer"));
}

void ClientDefault::Connect(const wxString &url)
{
	m_event_handler->OnClientDebug(wxEmptyString, wxT("Connect not implemented yet"));
}
