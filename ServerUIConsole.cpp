#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: ServerUIConsole.cpp,v 1.10 2003-03-10 13:04:44 jason Exp $)

#include "ServerUIConsole.h"
#include "ServerDefault.h"
#include "LogControl.h"
#include "util.h"

ServerUIConsole::ServerUIConsole(bool no_input)
	: Console(no_input)
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
	else if (cmd == wxT("HELP"))
	{
		OnServerInformation(wxT("Supported commands: EXIT"));
		return false;
	}
	else
	{
		return false;
	}
}

void ServerUIConsole::OnServerStateChange()
{
}

void ServerUIConsole::OnServerConnectionChange()
{
}
