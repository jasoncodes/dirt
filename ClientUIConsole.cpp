#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: ClientUIConsole.cpp,v 1.23 2003-02-13 13:16:50 jason Exp $)

#include "ClientUIConsole.h"
#include "LogControl.h"
#include "ClientDefault.h"
#include "util.h"
#include "FileTransfer.h"

ClientUIConsole::ClientUIConsole()
{
	m_client = new ClientDefault(this);
}

ClientUIConsole::~ClientUIConsole()
{
	delete m_client;
}

void ClientUIConsole::Output(const wxString &line)
{
	Console::Output(Timestamp() + LogControl::ConvertModifiersIntoHtml(line, true));
}

void ClientUIConsole::OnInput(const wxString &line)
{
	m_client->ProcessInput(wxEmptyString, line);
}

void ClientUIConsole::OnEOF()
{
	m_client->ProcessInput(wxEmptyString, "/exit");
}

bool ClientUIConsole::OnClientPreprocess(const wxString &context, wxString &cmd, wxString &params)
{
	if (cmd == "EXIT")
	{
		ExitMainLoop();
		return true;
	}
	else if (cmd == "HELP")
	{
		OnClientInformation(context, "Supported commands: EXIT");
		return false;
	}
	else
	{
		return false;
	}
}

void ClientUIConsole::OnClientDebug(const wxString &context, const wxString &text)
{
	Output(wxString() << "Debug: " << text);
}

void ClientUIConsole::OnClientWarning(const wxString &context, const wxString &text)
{
	Output(wxString() << "* " << text);
}

void ClientUIConsole::OnClientInformation(const wxString &context, const wxString &text)
{
	Output(wxString() << "* " << text);
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

void ClientUIConsole::OnClientUserList(const wxArrayString &nicklist)
{
	wxString msg = "* Chatting with: ";
	if (nicklist.GetCount() > 0)
	{
		for (size_t i = 0; i < nicklist.GetCount(); ++i)
		{
			if (i > 0)
			{
				msg += ", ";
			}
			msg += nicklist[i];
		}
	}
	else
	{
		msg += "(Nobody)";
	}
	Output(msg);
}

void ClientUIConsole::OnClientUserJoin(const wxString &nick, const wxString &details)
{
	wxString msg;
	msg << "* " << nick;
	if (details.Length() > 0)
	{
		msg << " (" << details << ")";
	}
	msg << " has joined the chat";
	Output(msg);
}

void ClientUIConsole::OnClientUserPart(const wxString &nick, const wxString &details, const wxString &message)
{
	wxString msg;
	msg << "* " << nick;
	if (details.Length() > 0)
	{
		msg << " (" << details << ")";
	}
	msg << " has left the chat";
	if (message.Length() > 0)
	{
		msg << " (" << message << ")";
	}
	Output(msg);
}

void ClientUIConsole::OnClientTransferNew(const FileTransfer &transfer)
{
	// not implemented
}

void ClientUIConsole::OnClientTransferDelete(const FileTransfer &transfer)
{
	// not implemented
}

void ClientUIConsole::OnClientTransferState(const FileTransfer &transfer)
{
	bool bIsError = ((transfer.state == ftsSendFail) || (transfer.state == ftsGetFail));
	if (bIsError)
	{
		OnClientInformation(wxEmptyString, transfer.status);
	}
	else
	{
		OnClientWarning(wxEmptyString, transfer.status);
	}
}

void ClientUIConsole::OnClientTransferTimer(const FileTransfer &transfer)
{
}
