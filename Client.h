#ifndef Client_H_
#define Client_H_

#include "FileTransfer.h"
#include "URL.h"
#include "ByteBuffer.h"
#include "util.h"
#include <wx/confbase.h>
#include <wx/fileconf.h>
#include "ClientTimers.h"

#define ASSERT_CONNECTED() { if (!IsConnected()) { m_event_handler->OnClientWarning(context, wxT("Not connected")); return; } }

class ClientEventHandler
{

public:
	virtual bool OnClientPreprocess(const wxString &context, wxString &cmd, wxString &params) { return false; }
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
	virtual void OnClientUserAway(const wxString &nick, const wxString &msg) = 0;
	virtual void OnClientUserBack(const wxString &nick, const wxString &msg) = 0;
	virtual void OnClientWhoIs(const wxString &context, const ByteBufferHashMap &details) = 0;
	virtual void OnClientTransferNew(const FileTransfer &transfer) = 0;
	virtual void OnClientTransferDelete(const FileTransfer &transfer) = 0;
	virtual void OnClientTransferState(const FileTransfer &transfer) = 0;
	virtual void OnClientTransferTimer(const FileTransfer &transfer) = 0;

};

class ClientConfig
{

public:
	ClientConfig();
	virtual ~ClientConfig();

	virtual wxConfigBase *GetConfig() const { return m_config; }

protected:
	wxFileConfig *m_config;

};

class Client : public wxEvtHandler
{

	friend class FileTransfers;

public:
	Client(ClientEventHandler *event_handler);
	virtual ~Client();

	virtual void ProcessConsoleInput(const wxString &context, const wxString &input);
	virtual void Debug(const wxString &context, const wxString &text);
	virtual void SendMessage(const wxString &context, const wxString &nick, const wxString &message, bool is_action) = 0;
	virtual bool Connect(const URL &url) = 0;
	virtual void Disconnect(const wxString &msg = wxT("Disconnected")) = 0;
	virtual bool IsConnected() const = 0;
	virtual const URL& GetLastURL() const = 0;
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
	virtual wxString GetServerName() const;
	virtual wxString GetDefaultNick() const;
	virtual void SetNickname(const wxString &context, const wxString &nickname) = 0;
	virtual FileTransfers* GetFileTransfers() const { return m_file_transfers; }
	virtual wxArrayString GetSupportedCommands() const;
	virtual ClientConfig& GetConfig() { return m_config; }
	virtual ClientTimers& GetTimers() { return *m_timers; }

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

protected:
	ClientEventHandler *m_event_handler;
	FileTransfers *m_file_transfers;
	ClientTimers *m_timers;
	wxString m_nickname;
	wxString m_server_name;
	wxTimer *m_tmrPing;
	wxLongLong_t m_ping_next;
	wxString m_ping_data;
	wxLongLong_t m_ping_timeout_tick;
	long m_latency;
	ClientConfig m_config;
	wxArrayString m_server_ip_list;

private:
	DECLARE_EVENT_TABLE()

};

#endif
