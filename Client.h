#ifndef Client_H_
#define Client_H_

class FileTransfer;
class FileTransfers;
enum FileTransferState;

class ClientEventHandler
{

public:
	virtual bool OnClientPreprocess(const wxString &context, wxString &cmd, wxString &params) { return false; }
	virtual void OnClientDebug(const wxString &context, const wxString &text) = 0;
	virtual void OnClientWarning(const wxString &context, const wxString &text) = 0;
	virtual void OnClientInformation(const wxString &context, const wxString &text) = 0;
	virtual void OnClientMessageOut(const wxString &nick, const wxString &text) = 0;
	virtual void OnClientMessageIn(const wxString &nick, const wxString &text, bool is_private) = 0;
	virtual void OnClientUserList(const wxArrayString &nicklist) = 0;
	virtual void OnClientUserJoin(const wxString &nick, const wxString &details) = 0;
	virtual void OnClientUserPart(const wxString &nick, const wxString &details, const wxString &message) = 0;
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

	virtual void ProcessInput(const wxString &context, const wxString &input);
	virtual void Debug(const wxString &context, const wxString &text);

	virtual void SendMessage(const wxString &nick, const wxString &message) = 0;

	virtual wxString GetNickname() = 0;
	virtual FileTransfers* GetFileTransfers() { return m_file_transfers; }

protected:
	ClientEventHandler *m_event_handler;
	FileTransfers *m_file_transfers;

private:
	DECLARE_EVENT_TABLE()

};

#endif
