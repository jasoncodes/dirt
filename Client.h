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


#ifndef Client_H_
#define Client_H_

class FileTransfer;
class URL;
class ClientTimers;
class ClientTimersEvent;
class CryptSocketProxySettings;

#include "util.h"

#define ASSERT_CONNECTED() { if (!IsConnected()) { m_event_handler->OnClientWarning(context, wxT("Not connected")); return; } }
#define ASSERT_CONNECTED2(retval) { if (!IsConnected()) { m_event_handler->OnClientWarning(context, wxT("Not connected")); return retval; } }

class ClientContact;

#include <wx/dynarray.h>
#ifndef WX_DEFINE_ARRAY_PTR
	#define WX_DEFINE_ARRAY_PTR WX_DEFINE_ARRAY
#endif
WX_DEFINE_ARRAY_PTR(ClientContact*, ClientContactArray);

enum ResumeState
{
	rsResume = wxID_YES,
	rsOverwrite = wxID_NO,
	rsCancel = wxID_CANCEL,
	rsNotSupported = wxID_DEFAULT
};

class ClientEventHandler
{

public:
	virtual bool OnClientPreprocess(const wxString &WXUNUSED(context), wxString &WXUNUSED(cmd), wxString &WXUNUSED(params)) { return false; }
	virtual wxArrayString OnClientSupportedCommands() { return wxArrayString(); }
	virtual void OnClientDebug(const wxString &context, const wxString &text) = 0;
	virtual void OnClientWarning(const wxString &context, const wxString &text) = 0;
	virtual void OnClientError(const wxString &context, const wxString &type, const wxString &text) = 0;
	virtual void OnClientInformation(const wxString &context, const wxString &text) = 0;
	virtual void OnClientStateChange() = 0;
	virtual void OnClientAuthNeeded(const wxString &text) = 0;
	virtual void OnClientAuthDone(const wxString &text) = 0;
	virtual void OnClientAuthBad(const wxString &text) = 0;
	virtual void OnClientMessageOut(const wxString &context, const wxString &nick, const wxString &text, bool is_action) = 0;
	virtual void OnClientMessageIn(const wxString &nick, const wxString &text, bool is_action, bool is_private) = 0;
	virtual void OnClientCTCPIn(const wxString &context, const wxString &nick, const wxString &type, const ByteBuffer &data) = 0;
	virtual void OnClientCTCPOut(const wxString &context, const wxString &nick, const wxString &type, const ByteBuffer &data) = 0;
	virtual void OnClientCTCPReplyIn(const wxString &context, const wxString &nick, const wxString &type, const ByteBuffer &data) = 0;
	virtual void OnClientCTCPReplyOut(const wxString &context, const wxString &nick, const wxString &type, const ByteBuffer &data) = 0;
	virtual void OnClientUserList(const wxArrayString &nicklist) = 0;
	virtual void OnClientUserJoin(const wxString &nick, const wxString &details) = 0;
	virtual void OnClientUserPart(const wxString &nick, const wxString &details, const wxString &message) = 0;
	virtual void OnClientUserNick(const wxString &old_nick, const wxString &new_nick) = 0;
	virtual void OnClientUserAway(const wxString &nick, const wxString &msg, long away_time, long away_time_diff) = 0;
	virtual void OnClientUserBack(const wxString &nick, const wxString &msg, long away_time, long away_time_diff) = 0;
	virtual void OnClientWhoIs(const wxString &context, const ByteBufferHashMap &details) = 0;
	virtual void OnClientTransferNew(const FileTransfer &transfer) = 0;
	virtual void OnClientTransferDelete(const FileTransfer &transfer, bool user_initiated) = 0;
	virtual void OnClientTransferState(const FileTransfer &transfer) = 0;
	virtual void OnClientTransferTimer(const FileTransfer &transfer) = 0;
	virtual ResumeState OnClientTransferResumePrompt(const FileTransfer &transfer, const wxString &new_filename, bool can_resume) = 0;

};

#include "ConfigFile.h"

class ClientConfig : public Config
{

public:
	enum SystemTrayIconMode
	{
		stimFlash,
		stimAlwaysImage,
		stimAlwaysBlank
	};

	ClientConfig();
	virtual ~ClientConfig();

	virtual wxString GetNickname() const;
	virtual bool SetNickname(const wxString &nickname);

	virtual wxString GetLastSendDir() const;
	virtual wxString GetLastGetDir() const;

	virtual bool SetLastSendDir(const wxString &dir);
	virtual bool SetLastGetDir(const wxString &dir);

	virtual wxString GetActualSoundFile() const;
	virtual TristateMode GetSoundType() const;
	virtual wxString GetSoundFile() const;
	virtual bool SetSoundFile(TristateMode type, const wxString &filename);

	virtual bool GetTaskbarNotification() const;
	virtual bool SetTaskbarNotification(bool mode);

	virtual bool GetFileTransferStatus() const;
	virtual bool SetFileTransferStatus(bool mode);

	virtual SystemTrayIconMode GetSystemTrayIconMode() const;
	virtual bool SetSystemTrayIconMode(SystemTrayIconMode mode);

	virtual wxArrayString GetSavedPasswordServerNames() const;
	virtual wxString GetSavedPassword(const wxString &server_name, bool decrypt) const;
	virtual bool SetSavedPassword(const wxString &server_name, const wxString &password);
	virtual bool DeleteSavedPassword(const wxString &server_name);
	virtual bool DeleteSavedPasswords();

	virtual int GetHotKey(int index, bool mod) const;
	virtual bool SetHotKey(int index, bool mod, int value);

	virtual wxFont GetFont() const;
	virtual bool SetFont(const wxFont &font);

protected:
	virtual wxString GetSoundFileKey() const;

};

class Client : public wxEvtHandler
{

	friend class FileTransfer;
	friend class FileTransfers;

public:
	Client(ClientEventHandler *event_handler);
	virtual ~Client();

	virtual void ProcessConsoleInput(const wxString &context, const wxString &input);
	virtual void Debug(const wxString &context, const wxString &text);
	virtual void SendMessage(const wxString &context, const wxString &nick, const wxString &message, bool is_action) = 0;
	virtual bool Connect(const URL &url, bool is_reconnect) = 0;
	virtual bool Reconnect() { return Connect(GetLastURL(), true); }
	virtual void Disconnect(const wxString &msg = wxT("Disconnected")) = 0;
	virtual bool IsConnected() const = 0;
	virtual const URL& GetLastURL() const = 0;
	virtual wxString GetLastHostname() const = 0;
	virtual void WhoIs(const wxString &context, const wxString &nick);
	virtual void Oper(const wxString &context, const wxString &pass) = 0;
	virtual void Quit(const wxString &msg);
	virtual void Away(const wxString &msg);
	virtual void Back() { Away(wxEmptyString); }
	virtual void CTCP(const wxString &context, const wxString &nick, const wxString &type, const ByteBuffer &data);
	virtual void CTCPReply(const wxString &context, const wxString &nick, const wxString &type, const ByteBuffer &data);
	virtual long GetLatency() const { return m_latency; }
	virtual void ProcessAlias(const wxString &context, const wxString &cmds, const wxString &params);
	wxArrayString GetAliasList() const;
	virtual wxString GetAlias(const wxString &name) const;
	virtual bool SetAlias(const wxString &name, const wxString &value);
	wxArrayString GetBindingList() const;
	wxArrayString GetValidBindNames() const;
	virtual wxString GetBinding(const wxString &name) const;
	virtual bool SetBinding(const wxString &name, const wxString &value);
	virtual void Authenticate(const ByteBuffer &auth) = 0;
	virtual wxString GetNickname() const;
	virtual wxString GetLastNickname() const;
	virtual wxString GetServerName() const;
	static wxString GetDefaultNick();
	virtual void SetNickname(const wxString &context, const wxString &nickname) = 0;
	virtual FileTransfers* GetFileTransfers() const { return m_file_transfers; }
	virtual wxArrayString GetSupportedCommands() const;
	virtual ClientConfig& GetConfig() { return m_config; }
	virtual ClientTimers& GetTimers() { return *m_timers; }
	virtual size_t GetContactCount() const;
	virtual ClientContact* GetContact(size_t index) const;
	virtual ClientContact* GetContact(const wxString &nick) const;
	virtual ClientContact* GetContactSelf() const { return m_contact_self; }
	virtual ByteBuffer GetKeyLocalPublic() const = 0;
	virtual ByteBuffer GetKeyLocalPrivate() const = 0;
	virtual void NewProxySettings() = 0;
	virtual const CryptSocketProxySettings GetProxySettings() const = 0;

protected:
	void OnTimerPing(wxTimerEvent &event);
	void OnClientTimers(ClientTimersEvent &event);

protected:
	virtual void ProcessServerInput(const ByteBuffer &msg);
	virtual void ProcessServerInput(const wxString &context, const wxString &cmd, const ByteBuffer &data);
	virtual bool ProcessServerInputExtra(bool preprocess, const wxString &context, const wxString &cmd, const ByteBuffer &data) = 0;
	virtual void OnConnect();
	virtual void SendToServer(const ByteBuffer &msg) = 0;
	virtual bool ProcessCTCPIn(const wxString &context, const wxString &nick, wxString &type, ByteBuffer &data);
	virtual bool ProcessCTCPOut(const wxString &context, const wxString &nick, wxString &type, ByteBuffer &data);
	virtual bool ProcessCTCPReplyIn(const wxString &context, const wxString &nick, wxString &type, ByteBuffer &data);
	virtual bool ProcessCTCPReplyOut(const wxString &context, const wxString &nick, wxString &type, ByteBuffer &data);
	virtual void EmptyContacts();

protected:
	ClientEventHandler *m_event_handler;
	ClientContactArray m_contacts;
	ClientContact *m_contact_self;
	FileTransfers *m_file_transfers;
	ClientTimers *m_timers;
	ByteBuffer m_last_auth;
	wxString m_nickname;
	wxString m_last_nickname;
	bool m_is_away;
	wxString m_away_message;
	wxString m_server_name;
	wxTimer *m_tmrPing;
	wxLongLong_t m_ping_next;
	wxString m_ping_data;
	wxLongLong_t m_ping_timeout_tick;
	long m_latency;
	ClientConfig m_config;
	wxArrayString m_server_ip_list;
	wxString m_ipself;

private:
	DECLARE_EVENT_TABLE()
	DECLARE_NO_COPY_CLASS(Client)

};

class ClientContact
{

	friend class Client;

protected:
	ClientContact(Client *client, wxString nickname)
		: m_client(client), m_nickname(nickname)
	{
		m_is_away = false;
		m_whois_cache_time = 0;
	}

public:
	wxString GetNickname() const { return m_nickname; }
	bool IsSelf() const { return m_client->GetNickname() == GetNickname(); }
	bool IsAway() const { return m_is_away; }
	long GetAwayTime() const { return m_away_time; }
	wxString GetAwayMessage() const { return m_away_message; }

protected:
	Client *m_client;
	wxString m_nickname;
	bool m_is_away;
	long m_away_time;
	long m_server_clock_diff;
	wxString m_away_message;
	ByteBufferHashMap m_whois_cache;
	wxLongLong_t m_whois_cache_time;

private:
	DECLARE_NO_COPY_CLASS(ClientContact)

};

#endif
