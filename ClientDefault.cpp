#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif

#include "ClientDefault.h"

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
}

ClientDefault::~ClientDefault()
{
	delete tmrTest;
}

void ClientDefault::SendMessage(const wxString &nick, const wxString &message)
{
	bool is_private = (nick.Length() > 0);
	m_event_handler->OnClientMessageOut(nick, message);
	m_event_handler->OnClientMessageIn(is_private?nick:"EVERYONE", 
		"You sent me \"" + message + "\"", is_private);
}

wxString ClientDefault::GetNickname()
{
	return "LOCALHOST";
}

void ClientDefault::OnTestTimer(wxTimerEvent &event)
{
	m_event_handler->OnClientDebug(wxEmptyString, "This is a test timer");
}
