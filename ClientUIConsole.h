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


#ifndef ClientUIConsole_H_
#define ClientUIConsole_H_

#include "Client.h"
#include "Console.h"
#include "LogWriter.h"

class ClientUIConsole : public Console, public ClientEventHandler
{

public:
	ClientUIConsole(bool no_input = false);
	virtual ~ClientUIConsole();

	Client* GetClient() { return m_client; }

	virtual void Output(const wxString &line);

protected:
	virtual bool OnClientPreprocess(const wxString &context, wxString &cmd, wxString &params);
	virtual wxArrayString OnClientSupportedCommands();
	virtual wxString OnClientExtraVersionInfo();
	virtual void OnClientDebug(const wxString &context, const wxString &text);
	virtual void OnClientWarning(const wxString &context, const wxString &text);
	virtual void OnClientError(const wxString &context, const wxString &type, const wxString &text);
	virtual void OnClientInformation(const wxString &context, const wxString &text);
	virtual void OnClientStateChange();
	virtual void OnClientAuthNeeded(const wxString &text);
	virtual void OnClientAuthDone(const wxString &text);
	virtual void OnClientAuthBad(const wxString &text);
	virtual void OnClientMessageOut(const wxString &context, const wxString &nick, const wxString &text, bool is_action);
	virtual void OnClientMessageIn(const wxString &nick, const wxString &text, bool is_action, bool is_private);
	virtual void OnClientCTCPIn(const wxString &context, const wxString &nick, const wxString &type, const ByteBuffer &data);
	virtual void OnClientCTCPOut(const wxString &context, const wxString &nick, const wxString &type, const ByteBuffer &data);
	virtual void OnClientCTCPReplyIn(const wxString &context, const wxString &nick, const wxString &type, const ByteBuffer &data);
	virtual void OnClientCTCPReplyOut(const wxString &context, const wxString &nick, const wxString &type, const ByteBuffer &data);
	virtual void OnClientUserList(const wxArrayString &nicklist);
	virtual void OnClientUserJoin(const wxString &nick, const wxString &details);
	virtual void OnClientUserPart(const wxString &nick, const wxString &details, const wxString &message);
	virtual void OnClientUserNick(const wxString &old_nick, const wxString &new_nick);
	virtual void OnClientUserAway(const wxString &nick, const wxString &msg, long away_time, long away_time_diff, bool already_away, long last_away_time, const wxString &last_msg);
	virtual void OnClientUserBack(const wxString &nick, const wxString &msg, long away_time, long away_time_diff);
	virtual void OnClientWhoIs(const wxString &context, const ByteBufferHashMap &details);
	virtual void OnClientTransferNew(const FileTransfer &transfer);
	virtual void OnClientTransferDelete(const FileTransfer &transfer, bool user_initiated);
	virtual void OnClientTransferState(const FileTransfer &transfer);
	virtual void OnClientTransferTimer(const FileTransfer &transfer);
	virtual ResumeState OnClientTransferResumePrompt(const FileTransfer &transfer, const wxString &new_filename, bool can_resume);

protected:
	virtual void OnInput(const wxString &line);
	virtual void OnEOF();

protected:
	Client *m_client;
	bool m_passmode;
	LogWriter *m_log;
	bool m_log_warning_given;

	DECLARE_NO_COPY_CLASS(ClientUIConsole)

};

#endif
