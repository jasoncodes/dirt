/*
    Copyright 2002, 2003 General Software Laboratories


    This file is part of Dirt Secure Chat.

    Dirt Secure Chat is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Dirt Secure Chat is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Dirt Secure Chat; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: ServerUIConsole.cpp,v 1.16 2004-07-18 18:28:57 jason Exp $)

#include "ServerUIConsole.h"
#include "ServerDefault.h"
#include "TextTools.h"
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
	Console::Output(GetLongTimestamp() + ConvertModifiersIntoHtml(line, true));
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

bool ServerUIConsole::OnServerConsoleInputPreprocess(wxString &cmd, wxString &params, const wxString &nick)
{
	if (cmd == wxT("EXIT"))
	{
		m_server->LogConsoleInput(cmd, params, nick);
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
