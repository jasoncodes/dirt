#if defined(__WXGTK__) || defined(__WXMOTIF__)
	#include "wx/wx.h"
#endif
#include "wx/wxprec.h"
#include "RCS.h"
RCS_ID($Id: ServerUIConsole.cpp,v 1.3 2003-02-14 04:39:57 jason Exp $)

#include "ServerUIConsole.h"
#include "ServerDefault.h"
#include "LogControl.h"
#include "util.h"

ServerUIConsole::ServerUIConsole()
{
	m_server = new ServerDefault(this);
}

ServerUIConsole::~ServerUIConsole()
{
	delete m_server;
}

void ServerUIConsole::Output(const wxString &line)
{
	Console::Output(GetLongTimestamp() + LogControl::ConvertModifiersIntoHtml(line, true));
}

void ServerUIConsole::OnServerInformation(const wxString &line)
{
	Output(line);
}

void ServerUIConsole::OnServerWarning(const wxString &line)
{
	Output(line);
}

void ServerUIConsole::OnInput(const wxString &line)
{
	m_server->ProcessInput(line);
}

void ServerUIConsole::OnEOF()
{
	m_server->ProcessInput("/exit");
}

bool ServerUIConsole::OnServerPreprocess(wxString &cmd, wxString &params)
{
	if (cmd == "EXIT")
	{
		ExitMainLoop();
		return true;
	}
	else if (cmd == "HELP")
	{
		OnServerInformation("Supported commands: EXIT");
		return false;
	}
	else
	{
		return false;
	}
}
