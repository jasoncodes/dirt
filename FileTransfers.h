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

	void Test();

	void DeleteTransfer(int transferid);

protected:
	void OnTimer(wxTimerEvent &event);

protected:
	int GetNewId();
	wxArrayString GetSupportedCommands();
	void ProcessConsoleInput(const wxString &context, const wxString &cmd, const wxString &params);

protected:
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
