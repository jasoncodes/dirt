#if defined(__WXGTK__) || defined(__WXMOTIF__)
	#include "wx/wx.h"
#endif
#include "wx/wxprec.h"
#include "RCS.h"
RCS_ID($Id: Server.cpp,v 1.1 2003-02-14 03:57:00 jason Exp $)

#include "Server.h"
#include "Modifiers.h"

BEGIN_EVENT_TABLE(Server, wxEvtHandler)
END_EVENT_TABLE()

Server::Server(ServerEventHandler *event_handler)
	: wxEvtHandler(), m_event_handler(event_handler)
{
}

Server::~Server()
{
}

void Server::ProcessInput(const wxString &input)
{
	m_event_handler->OnServerLog(wxString() << "You typed: \"" << input << (char)OriginalModifier << "\"");
}
