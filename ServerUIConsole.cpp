#if defined(__WXGTK__) || defined(__WXMOTIF__)
	#include "wx/wx.h"
#endif
#include "wx/wxprec.h"
#include "RCS.h"
RCS_ID($Id: ServerUIConsole.cpp,v 1.1 2003-02-14 03:57:00 jason Exp $)

#include "ServerUIConsole.h"
#include "ServerDefault.h"
#include "LogControl.h"
#include "Util.h"

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

void ServerUIConsole::OnServerLog(const wxString &line)
{
	Output(line);
}

void ServerUIConsole::OnInput(const wxString &line)
{
	m_server->ProcessInput(line);
}

void ServerUIConsole::OnEOF()
{
	ExitMainLoop();
}
