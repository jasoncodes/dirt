#if defined(__WXGTK__) || defined(__WXMOTIF__)
	#include "wx/wx.h"
#endif
#include "wx/wxprec.h"
#include "RCS.h"
RCS_ID($Id: Server.cpp,v 1.3 2003-02-14 12:43:01 jason Exp $)

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

	if (cmd == "START")
	{
		if (IsRunning())
		{
			m_event_handler->OnServerWarning("Server is already running");
		}
		else
		{
			Start();
		}
	}
	else if (cmd == "STOP")
	{
		if (IsRunning())
		{
			Stop();
		}
		else
		{
			m_event_handler->OnServerWarning("Server is not running");
		}
	}
	else if (cmd == "HELP")
	{
		m_event_handler->OnServerInformation("Supported commands: HELP START STOP");
	}
	else
	{
		m_event_handler->OnServerWarning("Unrecognized command: " + cmd);
	}

}
