#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: ClientUIConsole.cpp,v 1.29 2003-02-21 01:08:37 jason Exp $)

#include "ClientUIConsole.h"
#include "LogControl.h"
#include "ClientDefault.h"
#include "util.h"
#include "FileTransfer.h"
#include "Modifiers.h"

ClientUIConsole::ClientUIConsole(bool no_input)
	: Console(no_input)
{
	m_passmode = false;
	m_client = new ClientDefault(this);
}

ClientUIConsole::~ClientUIConsole()
{
	delete m_client;
}

void ClientUIConsole::Output(const wxString &line)
{
	Console::Output(GetShortTimestamp() + LogControl::ConvertModifiersIntoHtml(line, true));
}

void ClientUIConsole::OnInput(const wxString &line)
{
	if (m_passmode)
	{
		m_passmode = false;
		m_client->Authenticate(line);
	}
	else
	{
		m_client->ProcessConsoleInput(wxEmptyString, line);
	}
}

void ClientUIConsole::OnEOF()
{
	m_client->ProcessConsoleInput(wxEmptyString, wxT("/exit"));
}

bool ClientUIConsole::OnClientPreprocess(const wxString &context, wxString &cmd, wxString &params)
{
	if (cmd == wxT("EXIT"))
	{
		ExitMainLoop();
		return true;
	}
	else if (cmd == wxT("HELP"))
	{
		OnClientInformation(context, wxT("Supported commands: EXIT"));
		return false;
	}
	else
	{
		return false;
	}
}

void ClientUIConsole::OnClientDebug(const wxString &context, const wxString &text)
{
	Output(wxString() << wxT("Debug: ") << text);
}

void ClientUIConsole::OnClientWarning(const wxString &context, const wxString &text)
{
	Output(wxString() << wxT("* ") << text);
}

void ClientUIConsole::OnClientError(const wxString &context, const wxString &type, const wxString &text)
{
	Output(wxString() << wxT("* Error ") << type << wxT(": ") << text);
}

void ClientUIConsole::OnClientInformation(const wxString &context, const wxString &text)
{
	Output(wxString() << wxT("* ") << text);
}

void ClientUIConsole::OnClientStateChange()
{
	m_passmode = false;
}

void ClientUIConsole::OnClientAuthNeeded(const wxString &text)
{
	OnClientInformation(wxEmptyString, text);
	m_passmode = true;
}

void ClientUIConsole::OnClientAuthDone(const wxString &text)
{
	m_passmode = false;
	if (text.Length())
	{
		OnClientInformation(wxEmptyString, text);
	}
	m_client->SetNickname(wxEmptyString, m_client->GetDefaultNick());
}

void ClientUIConsole::OnClientAuthBad(const wxString &text)
{
	OnClientWarning(wxEmptyString, text);
	m_passmode = true;
}

void ClientUIConsole::OnClientMessageOut(const wxString &nick, const wxString &text)
{
	bool is_private = (nick.Length() > 0);
	if (is_private)
	{
		Output(wxString() << wxT("-> *") + nick + wxT("* ") + text);
	}
	else
	{
		Output(wxString() << wxT("<") + m_client->GetNickname() + wxT("> ") + text);
	}
}

void ClientUIConsole::OnClientMessageIn(const wxString &nick, const wxString &text, bool is_private)
{
	if (is_private)
	{
		Output(wxString() << wxT("*") + nick + wxT("* ") + text);
	}
	else
	{
		Output(wxString() << wxT("<") + nick + wxT("> ") + text);
	}
}

void ClientUIConsole::OnClientUserList(const wxArrayString &nicklist)
{
	wxString msg = wxT("* Chatting with: ");
	if (nicklist.GetCount() > 0)
	{
		for (size_t i = 0; i < nicklist.GetCount(); ++i)
		{
			if (i > 0)
			{
				msg += wxT(", ");
			}
			msg += nicklist[i];
		}
	}
	else
	{
		msg += wxT("(Nobody)");
	}
	Output(msg);
}

void ClientUIConsole::OnClientUserJoin(const wxString &nick, const wxString &details)
{
	wxString msg;
	if (nick == m_client->GetNickname())
	{
		msg << wxT("* Now chatting as ") << nick;
	}
	else
	{
		msg << wxT("* ") << nick;
		if (details.Length() > 0)
		{
			msg << wxT(" (") << details << (wxChar)OriginalModifier << wxT(")");
		}
		msg << wxT(" has joined the chat");
	}
	Output(msg);
}

void ClientUIConsole::OnClientUserPart(const wxString &nick, const wxString &details, const wxString &message)
{
	wxString msg;
	msg << wxT("* ") << nick;
	if (details.Length() > 0)
	{
		msg << wxT(" (") << details << wxT(")");
	}
	msg << wxT(" has left the chat");
	if (message.Length() > 0)
	{
		msg << wxT(" (" << message << ")");
	}
	Output(msg);
}

void ClientUIConsole::OnClientUserNick(const wxString &old_nick, const wxString &new_nick)
{

	wxString msg;
	
	if (old_nick == new_nick)
	{
		msg << wxT("* You nickname is ") << new_nick;
	}
	else if (new_nick == m_client->GetNickname())
	{
		msg << wxT("* You are now known as ") << new_nick;
	}
	else
	{
		msg << wxT("* ") << old_nick;
		msg << wxT(" is now known as ");
		msg << new_nick;
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
