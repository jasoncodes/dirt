#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: ClientDefault.cpp,v 1.8 2003-02-13 13:16:50 jason Exp $)

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

	Debug(wxEmptyString, "ClientDefault Ready");

	wxArrayString users(true);
	users.Add("First");
	users.Add("Third");
	users.Add("Second");
	m_event_handler->OnClientUserList(users);
	m_event_handler->OnClientUserJoin("Fourth", "fourth@dev.null");
	m_event_handler->OnClientUserPart("Fourth", "fourth@dev.null", "Quit message goes here");

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
		is_private?nick:"EVERYONE", 
		wxString() << "You sent me \"" << message << (char)OriginalModifier << "\"",
		is_private);
}

wxString ClientDefault::GetNickname()
{
	return "LOCALHOST";
}

void ClientDefault::OnTestTimer(wxTimerEvent &event)
{
	m_event_handler->OnClientDebug(wxEmptyString, "This is a test timer");
}
