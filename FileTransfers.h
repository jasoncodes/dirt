#ifndef FileTransfers_H_
#define FileTransfers_H_

#include <wx/dynarray.h>

class Client;
class FileTransfer;
class ByteBuffer;
class CryptSocketBase;
class CryptSocketEvent;
class DirtApp;
class Uint16Array;

WX_DECLARE_OBJARRAY(FileTransfer, FileTransferArray);

class FileTransfers : public wxEvtHandler
{

	friend class FileTransfer;
	friend class Client;
	friend class DirtApp;

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

	wxString GetLastCompletedGetFilename() const { return m_last_completed_get_filename; }

protected:
	void OnTimer(wxTimerEvent &event);
	void OnSocket(CryptSocketEvent &event);
	void OnAppIdle(wxIdleEvent &event);

protected:
	int GetNewId();
	wxArrayString GetSupportedCommands();
	void ProcessConsoleInput(const wxString &context, const wxString &cmd, const wxString &params);
	void Information(const wxString &context, const wxString &text);
	void Warning(const wxString &context, const wxString &text);
	void MaybeSendData(FileTransfer &t);
	int FindRemoteTransfer(int remoteid);
	int FindTransferBySocket(CryptSocketBase *sck);
	wxArrayString GetMyIPs() const;
	bool ExtractIPsAndPorts(const ByteBufferArray &fields, size_t i, wxArrayString &IPs, Uint16Array &ports) const;
	void AppendMyIPs(ByteBufferArray &data, wxUint16 port) const;

protected:
	void OnGetConnection(FileTransfer &t);
	void OnRemoteCancel(FileTransfer &t, const wxString &msg);
	void OnGetData(FileTransfer &t, const wxString &cmd, const ByteBuffer &data);
	void OnSendData(FileTransfer &t, const wxString &cmd, const ByteBuffer &data);
	void OnCommonData(FileTransfer &t, const wxString &cmd, const ByteBuffer &data);
	void OnClose(FileTransfer &t);

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
	wxTimer *m_tmr;
	#ifdef __WXMSW__
		wxTimer *m_tmrIdleEventFaker;
	#endif
	wxString m_last_completed_get_filename;

private:
	DECLARE_EVENT_TABLE()
	DECLARE_NO_COPY_CLASS(FileTransfers)

};

#endif
