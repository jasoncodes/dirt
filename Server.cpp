#if defined(__WXGTK__) || defined(__WXMOTIF__)
	#include "wx/wx.h"
#endif
#include "wx/wxprec.h"
#include "RCS.h"
RCS_ID($Id: Server.cpp,v 1.5 2003-02-16 05:09:03 jason Exp $)

#include "Server.h"
#include "Modifiers.h"
#include "util.h"

#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY(ServerConnectionArray);

ServerConnection::ServerConnection()
	: m_jointime(wxDateTime::Now())
{
	ResetIdleTime();
}

ServerConnection::~ServerConnection()
{
}

ServerConnection::operator wxString() const
{
	wxString retval;
	retval << (GetNickname().Length()?GetNickname():wxT("N/A"));
	retval << wxT("@") << (GetRemoteHost().Length()?GetRemoteHost():wxT("N/A"));
	retval << wxT("(") << (GetUserDetails().Length()?GetUserDetails():wxT("N/A"));
	retval << wxT(") (");
	if (GetAwayMessage().Length())
	{
		retval << wxT("Away: ") << GetAwayMessage() << wxT("; ");
	}
	retval << wxT("Idle: ") << GetIdleTimeString() << wxT("; ");
	retval << wxT("Lag: ") << GetLatencyString() << wxT("; ");
	if (GetUserAgent().Length())
	{
		retval << wxT("Agent: ") << GetUserAgent() << wxT("; ");
	}
	retval << wxT("Joined: ") << GetJoinTimeString();
	retval << wxT(")");
	return retval;
}

BEGIN_EVENT_TABLE(Server, wxEvtHandler)
END_EVENT_TABLE()

Server::Server(ServerEventHandler *event_handler)
	: wxEvtHandler(), m_event_handler(event_handler)
{
	m_connections.Alloc(10);
}

Server::~Server()
{
}

void Server::ProcessInput(const wxString &input)
{

	wxString cmd, params;
	
	SplitHeadTail(input, cmd, params);
	if (input[0] == wxT('/'))
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

	if (cmd == wxT("START"))
	{
		if (IsRunning())
		{
			m_event_handler->OnServerWarning(wxT("Server is already running"));
		}
		else
		{
			Start();
		}
	}
	else if (cmd == wxT("STOP"))
	{
		if (IsRunning())
		{
			Stop();
		}
		else
		{
			m_event_handler->OnServerWarning(wxT("Server is not running"));
		}
	}
	else if (cmd == wxT("USERS"))
	{
		if (IsRunning())
		{
			m_event_handler->OnServerInformation(wxString() << wxT("There are currently ") << GetConnectionCount() << wxT(" connections"));
			for (size_t i = 0; i < GetConnectionCount(); ++i)
			{
				m_event_handler->OnServerInformation(wxT("    ") + GetConnection(i));
			}
		}
		else
		{
			m_event_handler->OnServerWarning(wxT("Server is not running"));
		}
	}
	else if (cmd == wxT("HELP"))
	{
		m_event_handler->OnServerInformation(wxT("Supported commands: HELP START STOP USERS"));
	}
	else
	{
		m_event_handler->OnServerWarning(wxT("Unrecognized command: ") + cmd);
	}

}
