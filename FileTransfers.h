#ifndef FileTransfers_H_
#define FileTransfers_H_

#include <wx/dynarray.h>

class Client;
class FileTransfer;
class ByteBuffer;

WX_DECLARE_OBJARRAY(FileTransfer, FileTransferArray);

class FileTransfers : public wxEvtHandler
{

	friend class FileTransfer;
	friend class Client;

public:
	FileTransfers(Client *client);
	virtual ~FileTransfers();

	int GetTransferCount();

	int FindTransfer(int transferid);

	const FileTransfer& GetTransferById(int transferid);
	const FileTransfer& GetTransferByIndex(int index);

	int SendFile(const wxString &nickname, const wxString &filename);
	bool AcceptTransfer(int transferid, const wxString &filename, bool resume);
	bool DeleteTransfer(int transferid, bool user_initiated);

protected:
	void OnTimer(wxTimerEvent &event);

protected:
	int GetNewId();
	wxArrayString GetSupportedCommands();
	void ProcessConsoleInput(const wxString &context, const wxString &cmd, const wxString &params);
	void Information(const wxString &context, const wxString &text);
	void Warning(const wxString &context, const wxString &text);

protected:
	virtual void OnClientUserNick(const wxString &old_nick, const wxString &new_nick);
	virtual bool OnClientCTCPIn(const wxString &context, const wxString &nick, const wxString &type, const ByteBuffer &data);
	virtual bool OnClientCTCPOut(const wxString &context, const wxString &nick, const wxString &type, const ByteBuffer &data);
	virtual bool OnClientCTCPReplyIn(const wxString &context, const wxString &nick, const wxString &type, const ByteBuffer &data);
	virtual bool OnClientCTCPReplyOut(const wxString &context, const wxString &nick, const wxString &type, const ByteBuffer &data);
	virtual bool OnClientPreprocess(const wxString &context, const wxString &cmd, const wxString &params);
	virtual wxArrayString OnClientSupportedCommands();

protected:
	Client *m_client;
	FileTransferArray m_transfers;
	wxTimer *tmr;

private:
	DECLARE_EVENT_TABLE()
	DECLARE_NO_COPY_CLASS(FileTransfers)

};

#endif
