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
	m_event_handler->OnClientDebug(nick, 
		wxString() << "Send \"" <<
		message << "\" to " <<
		(nick.Length() == 0 ? "[everyone]" : nick));
}
