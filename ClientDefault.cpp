#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif

#include "ClientDefault.h"

ClientDefault::ClientDefault(ClientEventHandler *event_handler)
	: Client(event_handler)
{
	Debug(wxEmptyString, "ClientDefault Ready");
}

ClientDefault::~ClientDefault()
{
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
