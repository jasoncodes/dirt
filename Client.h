#ifndef Client_H_
#define Client_H_

#include "FileTransfer.h"
#include "URL.h"
#include "ByteBuffer.h"

#define ASSERT_CONNECTED() { if (!IsConnected()) { m_event_handler->OnClientWarning(wxEmptyString, wxT("Not connected")); return; } }

class ClientEventHandler
{

public:
	virtual bool OnClientPreprocess(const wxString &context, wxString &cmd, wxString &params) { return false; }
	virtual void OnClientDebug(const wxString &context, const wxString &text) = 0;
	virtual void OnClientWarning(const wxString &context, const wxString &text) = 0;
	virtual void OnClientError(const wxString &context, const wxString &type, const wxString &text) = 0;
	virtual void OnClientInformation(const wxString &context, const wxString &text) = 0;
	virtual void OnClientStateChange() = 0;
	virtual void OnClientMessageOut(const wxString &nick, const wxString &text) = 0;
	virtual void OnClientMessageIn(const wxString &nick, const wxString &text, bool is_private) = 0;
	virtual void OnClientUserList(const wxArrayString &nicklist) = 0;
	virtual void OnClientUserJoin(const wxString &nick, const wxString &details) = 0;
	virtual void OnClientUserPart(const wxString &nick, const wxString &details, const wxString &message) = 0;
	virtual void OnClientUserNick(const wxString &old_nick, const wxString &new_nick) = 0;
	virtual void OnClientTransferNew(const FileTransfer &transfer) = 0;
	virtual void OnClientTransferDelete(const FileTransfer &transfer) = 0;
	virtual void OnClientTransferState(const FileTransfer &transfer) = 0;
	virtual void OnClientTransferTimer(const FileTransfer &transfer) = 0;

};

class Client : public wxEvtHandler
{

	friend FileTransfers;

public:
	Client(ClientEventHandler *event_handler);
	virtual ~Client();

	virtual void ProcessConsoleInput(const wxString &context, const wxString &input);
	virtual void Debug(const wxString &context, const wxString &text);

	virtual void SendMessage(const wxString &context, const wxString &nick, const wxString &message) = 0;
	virtual bool Connect(const URL &url) = 0;
	virtual void Disconnect() = 0;
	virtual bool IsConnected() = 0;

	virtual wxString GetNickname();
	virtual void SetNickname(const wxString &context, const wxString &nickname) = 0;
	virtual FileTransfers* GetFileTransfers() { return m_file_transfers; }

protected:
	virtual void ProcessServerInput(const ByteBuffer &msg);
	virtual void ProcessServerInput(const wxString &context, const wxString &cmd, const ByteBuffer &data);
	virtual void OnConnect();
	virtual void SendToServer(const ByteBuffer &msg) = 0;

protected:
	ClientEventHandler *m_event_handler;
	FileTransfers *m_file_transfers;
	wxString m_nickname;

private:
	DECLARE_EVENT_TABLE()

};

#endif
