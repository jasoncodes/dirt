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


#ifndef ServerUIConsole_H_
#define ServerUIConsole_H_

#include "Server.h"
#include "Console.h"

class ServerUIConsole : public Console, public ServerEventHandler
{

public:
	ServerUIConsole(bool no_input = false, bool quit_on_stop  = false);
	virtual ~ServerUIConsole();

	virtual void Output(const wxString &line);

protected:
	virtual bool OnServerConsoleInputPreprocess(wxString &cmd, wxString &params, const wxString &nick);
	virtual wxArrayString OnServerSupportedCommands();
	virtual void OnServerStateChange();
	virtual void OnServerConnectionChange();
	virtual void OnServerInformation(const wxString &line);
	virtual void OnServerWarning(const wxString &line);

protected:
	virtual void OnInput(const wxString &line);
	virtual void OnEOF();

protected:
	Server *m_server;
	bool m_quit_on_stop;

	DECLARE_NO_COPY_CLASS(ServerUIConsole)

};

#endif
