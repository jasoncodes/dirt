#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: ServerUIConsole.cpp,v 1.12 2003-06-30 12:36:35 jason Exp $)

#include "ServerUIConsole.h"
#include "ServerDefault.h"
#include "LogControl.h"
#include "util.h"

ServerUIConsole::ServerUIConsole(bool no_input, bool quit_on_stop)
	: Console(no_input), m_quit_on_stop(quit_on_stop)
{
	m_server = new ServerDefault(this);
	m_server->Start();
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
	m_server->ProcessConsoleInput(line);
}

void ServerUIConsole::OnEOF()
{
	m_server->ProcessConsoleInput(wxT("/exit"));
}

bool ServerUIConsole::OnServerPreprocess(wxString &cmd, wxString &params)
{
	if (cmd == wxT("EXIT"))
	{
		ExitMainLoop();
		return true;
	}
	else
	{
		return false;
	}
}

wxArrayString ServerUIConsole::OnServerSupportedCommands()
{
	return SplitString(wxT("EXIT"), wxT(" "));
}

void ServerUIConsole::OnServerStateChange()
{
	if (m_quit_on_stop)
	{
		if (!m_server || !m_server->IsRunning())
		{
			OnEOF();
		}
	}
}

void ServerUIConsole::OnServerConnectionChange()
{
}
