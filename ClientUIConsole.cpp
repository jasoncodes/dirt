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
RCS_ID($Id: ClientUIConsole.cpp,v 1.60 2004-05-22 17:08:47 jason Exp $)

#include "ClientUIConsole.h"
#include "LogControl.h"
#include "ClientDefault.h"
#include "util.h"
#include "FileTransfer.h"
#include "Modifiers.h"
#include "Dirt.h"

DECLARE_APP(DirtApp)

ClientUIConsole::ClientUIConsole(bool no_input)
	: Console(no_input), m_log_warning_given(false)
{
	m_passmode = false;
	m_log = NULL;
	OnClientInformation(wxEmptyString, AppTitle(wxT("Client")));
	m_client = new ClientDefault(this);
	wxString log_dir = m_client->GetConfig().GetActualLogDir();
	wxDateTime log_date = LogWriter::GenerateNewLogDate(log_dir, wxT("Client"));
	wxString log_filename = LogWriter::GenerateFilename(log_dir, wxT("Client"), log_date);
	if (log_filename.Length())
	{
		m_log = new LogWriter(log_filename);
	}
}

ClientUIConsole::~ClientUIConsole()
{
	delete m_client;
	delete m_log;
}

void ClientUIConsole::Output(const wxString &line)
{
	wxString text = GetShortTimestamp() + line;
	Console::Output(LogControl::ConvertModifiersIntoHtml(text, true));
	if (m_log)
	{
		if (m_log->Ok())
		{
			m_log->AddText(text);
		}
		else if (!m_log_warning_given)
		{
			m_log_warning_given = true;
			OnClientWarning(wxEmptyString, wxT("Error writing log file"));
		}
	}
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

wxArrayString ClientUIConsole::OnClientSupportedCommands()
{
	return SplitString(wxT("EXIT"), wxT(" "));
}

bool ClientUIConsole::OnClientPreprocess(const wxString &WXUNUSED(context), wxString &cmd, wxString &params)
{
	if (cmd == wxT("EXIT"))
	{
		if (m_client->IsConnected())
		{
			m_client->Quit(params.Length() ? params : wxGetApp().GetDefaultQuitMessage());
		}
		ExitMainLoop();
		return true;
	}
	else
	{
		return false;
	}
}

void ClientUIConsole::OnClientDebug(const wxString &WXUNUSED(context), const wxString &text)
{
	Output(wxString() << wxT("Debug: ") << text);
}

void ClientUIConsole::OnClientWarning(const wxString &WXUNUSED(context), const wxString &text)
{
	Output(wxString() << wxT("*** ") << text);
}

void ClientUIConsole::OnClientError(const wxString &WXUNUSED(context), const wxString &type, const wxString &text)
{
	Output(wxString() << wxT("*** Error ") << type << wxT(": ") << text);
}

void ClientUIConsole::OnClientInformation(const wxString &WXUNUSED(context), const wxString &text)
{
	Output(wxString() << wxT("*** ") << text);
}

void ClientUIConsole::OnClientStateChange()
{
	if (!m_client->IsConnected())
	{
		m_passmode = false;
	}
}

void ClientUIConsole::OnClientAuthNeeded(const wxString &text)
{
	wxString pass = m_client->GetLastURL().GetPassword();
	if (!pass.Length())
	{
		pass = m_client->GetConfig().GetSavedPassword(m_client->GetServerName(), true);
	}
	if (pass.Length())
	{
		m_client->Authenticate(pass);
	}
	else
	{
		OnClientInformation(wxEmptyString, text);
		m_passmode = true;
	}
}

void ClientUIConsole::OnClientAuthDone(const wxString &text)
{
	m_passmode = false;
	if (text.Length())
	{
		OnClientInformation(wxEmptyString, text);
	}
	wxString nick = m_client->GetLastURL().GetUsername();
	if (!nick.Length())
	{
		nick = m_client->GetLastNickname();
	}
	if (!nick.Length())
	{
		nick = m_client->GetConfig().GetNickname();
	}
	if (!nick.Length())
	{
		nick = Client::GetDefaultNick();
	}
	m_client->SetNickname(wxEmptyString, nick);
}

void ClientUIConsole::OnClientAuthBad(const wxString &text)
{
	OnClientWarning(wxEmptyString, text);
	m_passmode = true;
}

void ClientUIConsole::OnClientMessageOut(const wxString &WXUNUSED(context), const wxString &nick, const wxString &text, bool is_action)
{
	bool is_private = (nick.Length() > 0);
	if (is_private)
	{
		if (is_action)
		{
			wxString sep = (text.Left(2)==wxT("'s")) ? wxT("") : wxT(" ");
			Output(wxString() << wxT("-> *") + nick + wxT("* * ") + m_client->GetNickname() + sep + text);
		}
		else
		{
			Output(wxString() << wxT("-> *") + nick + wxT("* ") + text);
		}
	}
	else
	{
		if (is_action)
		{
			wxString sep = (text.Left(2)==wxT("'s")) ? wxT("") : wxT(" ");
			Output(wxString() << wxT("* ") + m_client->GetNickname() + sep + text);
		}
		else
		{
			Output(wxString() << wxT("<") + m_client->GetNickname() + wxT("> ") + text);
		}
	}
}

void ClientUIConsole::OnClientMessageIn(const wxString &nick, const wxString &text, bool is_action, bool is_private)
{
	if (is_private)
	{
		if (is_action)
		{
			wxString sep = (text.Left(2)==wxT("'s")) ? wxT("") : wxT(" ");
			Output(wxString() << wxT("* *") + nick + wxT("*") + sep + text);
		}
		else
		{
			Output(wxString() << wxT("*") + nick + wxT("* ") + text);
		}
	}
	else
	{
		if (is_action)
		{
			wxString sep = (text.Left(2)==wxT("'s")) ? wxT("") : wxT(" ");
			Output(wxString() << wxT("* ") + nick + sep + text);
		}
		else
		{
			Output(wxString() << wxT("<") + nick + wxT("> ") + text);
		}
	}
}

void ClientUIConsole::OnClientCTCPIn(const wxString &WXUNUSED(context), const wxString &nick, const wxString &type, const ByteBuffer &data)
{
	wxString msg;
	msg << wxT('[') << nick;
	if (type.Length())
	{
		msg << wxT(' ') << type << (wxChar)OriginalModifier;
	}
	msg << wxT(']');
	if (data.Length())
	{
		msg << wxT(' ') << data;
	}
	Output(msg);
}

void ClientUIConsole::OnClientCTCPOut(const wxString &WXUNUSED(context), const wxString &nick, const wxString &type, const ByteBuffer &data)
{
	wxString msg;
	msg << wxT("-> [") << nick << wxT(']');
	if (type.Length())
	{
		msg << wxT(' ') << type << (wxChar)OriginalModifier;
	}
	if (data.Length())
	{
		msg << wxT(' ') << data;
	}
	Output(msg);
}

void ClientUIConsole::OnClientCTCPReplyIn(const wxString &WXUNUSED(context), const wxString &nick, const wxString &type, const ByteBuffer &data)
{
	wxString msg;
	msg << wxT('[') << nick;
	if (type.Length())
	{
		msg << wxT(' ') << type << (wxChar)OriginalModifier;
	}
	msg << wxT(" reply]");
	if (data.Length())
	{
		msg << wxT(": ") << data;
	}
	Output(msg);
}

void ClientUIConsole::OnClientCTCPReplyOut(const wxString &WXUNUSED(context), const wxString &WXUNUSED(nick), const wxString &WXUNUSED(type), const ByteBuffer &WXUNUSED(data))
{
}

void ClientUIConsole::OnClientUserList(const wxArrayString &nicklist)
{
	wxString msg = wxT("*** Chatting with: ");
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
		msg << wxT("*** Now chatting as ") << nick;
	}
	else
	{
		msg << wxT("*** ") << nick;
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
	msg << wxT("*** ") << nick;
	if (details.Length() > 0)
	{
		msg << wxT(" (") << details << wxT(")");
	}
	msg << wxT(" has left the chat");
	if (message.Length() > 0)
	{
		msg << wxT(" (" << message << wxT(")"));
	}
	Output(msg);
}

void ClientUIConsole::OnClientUserNick(const wxString &old_nick, const wxString &new_nick)
{

	wxString msg;
	
	if (old_nick == new_nick)
	{
		msg << wxT("*** You nickname is ") << new_nick;
	}
	else if (new_nick == m_client->GetNickname())
	{
		msg << wxT("*** You are now known as ") << new_nick;
	}
	else
	{
		msg << wxT("*** ") << old_nick;
		msg << wxT(" is now known as ");
		msg << new_nick;
	}

	Output(msg);

}

void ClientUIConsole::OnClientUserAway(const wxString &nick, const wxString &msg, long WXUNUSED(away_time), long away_time_diff, bool already_away, long last_away_time, const wxString &last_msg)
{
	wxString text;
	text << wxT("*** ") << nick;
	if (away_time_diff > 0)
	{
		text << wxT(" has been away for ") << SecondsToMMSS(away_time_diff);
	}
	else
	{
		text << wxT(" is away");
	}
	if (msg.Length())
	{
		text << wxT(": ") << msg << (wxChar)OriginalModifier;
	}
	if (already_away)
	{
		text << wxT(" (was: ") << last_msg << (wxChar)OriginalModifier;
		if (last_away_time > -1)
		{
			text << wxT(" for ") << SecondsToMMSS(last_away_time);
		}
		text << wxT(")");
	}
	Output(text);
}

void ClientUIConsole::OnClientUserBack(const wxString &nick, const wxString &msg, long WXUNUSED(away_time), long away_time_diff)
{
	wxString text;
	text << wxT("*** ") << nick << wxT(" has returned");
	if (msg.Length())
	{
		text << wxT(" (msg: ") << msg << (wxChar)OriginalModifier << wxT(")");
	}
	if (away_time_diff >= 0)
	{
		text << wxT(" (away for ") << SecondsToMMSS(away_time_diff) << wxT(")");
	}
	Output(text);
}

void ClientUIConsole::OnClientWhoIs(const wxString &WXUNUSED(context), const ByteBufferHashMap &details)
{
	ByteBufferHashMap details2(details);
	wxString nickname = details2[wxT("NICK")];
	Output(nickname + wxT(" is ") + details2[wxT("DETAILS")]);
	Output(nickname + wxT(" is connecting from ") + details2[wxT("HOSTNAME")]);
	if (details2.find(wxT("ISADMIN")) != details2.end())
	{
		Output(nickname + wxT(" is a server administrator"));
	}
	if (details2.find(wxT("AWAY")) != details2.end())
	{
		Output(nickname + wxT(" is away: ") + details2[wxT("AWAY")]);
	}
	if (details2.find(wxT("AWAYTIMEDIFFSTRING")) != details2.end())
	{
		Output(nickname + wxT(" has been away for ") + details2[wxT("AWAYTIMEDIFFSTRING")]);
	}
	Output(nickname + wxT(" is using ") + details2[wxT("AGENT")]);
	Output(nickname + wxT(" has been idle for ") + details2[wxT("IDLESTRING")] + wxT(" (") + details2[wxT("LATENCYSTRING")] + wxT(" lag)"));
	Output(nickname + wxT(" signed on at ") + details2[wxT("JOINTIMESTRING")]);
	details2.erase(wxT("NICK"));
	details2.erase(wxT("DETAILS"));
	details2.erase(wxT("HOSTNAME"));
	details2.erase(wxT("ISADMIN"));
	details2.erase(wxT("AWAY"));
	details2.erase(wxT("AWAYTIME"));
	details2.erase(wxT("AWAYTIMEDIFF"));
	details2.erase(wxT("AWAYTIMEDIFFSTRING"));
	details2.erase(wxT("AGENT"));
	details2.erase(wxT("IDLE"));
	details2.erase(wxT("IDLESTRING"));
	details2.erase(wxT("LATENCY"));
	details2.erase(wxT("LATENCYSTRING"));
	details2.erase(wxT("JOINTIME"));
	details2.erase(wxT("JOINTIMESTRING"));
	for (ByteBufferHashMap::iterator i = details2.begin(); i != details2.end(); ++i)
	{
		Output(nickname + wxT(" ") + i->first + wxT(" = ") + i->second);
	}
	Output(nickname + wxT(" End of /WHOIS"));
}

void ClientUIConsole::OnClientTransferNew(const FileTransfer &WXUNUSED(transfer))
{
}

void ClientUIConsole::OnClientTransferDelete(const FileTransfer &WXUNUSED(transfer), bool WXUNUSED(user_initiated))
{
}

void ClientUIConsole::OnClientTransferState(const FileTransfer &transfer)
{
	bool bIsError = ((transfer.state == ftsSendFail) || (transfer.state == ftsGetFail));
	if (bIsError)
	{
		OnClientWarning(wxEmptyString, transfer.GetPrefixString() + transfer.status);
	}
	else
	{
		OnClientInformation(wxEmptyString, transfer.GetPrefixString() + transfer.status);
	}
}

void ClientUIConsole::OnClientTransferTimer(const FileTransfer &WXUNUSED(transfer))
{
}

ResumeState ClientUIConsole::OnClientTransferResumePrompt(const FileTransfer &WXUNUSED(transfer), const wxString &WXUNUSED(new_filename), bool WXUNUSED(can_resume))
{
	return rsNotSupported;
}
