#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif

#include "ClientUIConsole.h"
#include "LogControl.h"
#include "ClientDefault.h"
#include <stdio.h>
#include "util.h"

static void Output(wxString &line)
{
	puts(Timestamp() + LogControl::ConvertModifiersIntoHtml(line, true));
}

ClientUIConsole::ClientUIConsole()
{
	m_client = new ClientDefault(this);
}

ClientUIConsole::~ClientUIConsole()
{
	delete m_client;
}

bool ClientUIConsole::OnClientPreprocess(const wxString &context, const wxString &cmd, const wxString &params)
{
	return false;
}

void ClientUIConsole::OnClientDebug(const wxString &context, const wxString &text)
{
	Output(wxString() << "Debug: Context = \"" + context + "\", Text = \"" + text + "\"");
}

void ClientUIConsole::OnClientWarning(const wxString &context, const wxString &text)
{
	Output(wxString() << "Warning: Context = \"" + context + "\", Text = \"" + text + "\"");
}

void ClientUIConsole::OnClientInformation(const wxString &context, const wxString &text)
{
	Output(wxString() << "Information: Context = \"" + context + "\", Text = \"" + text + "\"");
}

void ClientUIConsole::OnClientMessageOut(const wxString &nick, const wxString &text)
{
	bool is_private = (nick.Length() > 0);
	if (is_private)
	{
		Output(wxString() << "-> *" + nick + "* " + text);
	}
	else
	{
		Output(wxString() << "<" + m_client->GetNickname() + "> " + text);
	}
}

void ClientUIConsole::OnClientMessageIn(const wxString &nick, const wxString &text, bool is_private)
{
	if (is_private)
	{
		Output(wxString() << "*" + nick + "* " + text);
	}
	else
	{
		Output(wxString() << "<" + nick + "> " + text);
	}
}
