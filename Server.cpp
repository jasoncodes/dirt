#if defined(__WXGTK__) || defined(__WXMOTIF__)
	#include "wx/wx.h"
#endif
#include "wx/wxprec.h"
#include "RCS.h"
RCS_ID($Id: Server.cpp,v 1.2 2003-02-14 04:39:57 jason Exp $)

#include "Server.h"
#include "Modifiers.h"
#include "util.h"

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

	wxString cmd, params;
	
	SplitHeadTail(input, cmd, params);
	if (input[0] == '/')
	{
		cmd = cmd.Mid(1);
	}
	cmd.MakeUpper();
	cmd.Trim(true);
	cmd.Trim(false);
	params.Trim(true);
	params.Trim(false);

	if (m_event_handler->OnServerPreprocess(cmd, params))
	{
		return;
	}

	if (cmd == "HELP")
	{
		m_event_handler->OnServerInformation("Supported commands: HELP");
	}
	else
	{
		m_event_handler->OnServerWarning("Unrecognized command: " + cmd);
	}

}
