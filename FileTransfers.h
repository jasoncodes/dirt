#ifndef FileTransfers_H_
#define FileTransfers_H_

#include <wx/dynarray.h>

class Client;
class FileTransfers;

enum FileTransferState
{
    
	ftsUnknown = 0,    // unknown state

	ftsSendListening,  // waiting for other user to accept
    ftsSendStarting,   // connected, other side accepted, handshaking (resume pos, etc)
    ftsSendTransfer,   // transferring
    ftsSendConnecting, // firewall, other side try listening
	ftsSendComplete,   // transfer complete
	ftsSendFail,       // transfer failed (connect broken, other side abort, etc)
    
	ftsGetPending,     // waiting for user to accept
    ftsGetConnecting,  // accepted, connecting to other host
    ftsGetStarting,    // connected, handshaking (resume pos, etc)
    ftsGetTransfer,    // transferring
    ftsGetListening,   // firewall, other side try connecting
	ftsGetComplete,    // transfer complete
	ftsGetFail         // transfer failed (connect broken, other side abort, etc)

};

class FileTransfer
{

	friend FileTransfers;

public:
	int transferid;
	bool issend;
	FileTransferState state;
	wxString nickname;
	wxString filename;
	off_t filesize;
	long time;
	long timeleft;
	long cps;
	off_t filesent;
	wxString status;

protected:
	FileTransfer()
		: transferid(-1), issend(false), state(ftsUnknown),
		nickname(wxEmptyString), filename(wxEmptyString),
		filesize(0), time(0), timeleft(0), cps(0), filesent(0),
		status(wxEmptyString)
	{}

};

WX_DECLARE_OBJARRAY(FileTransfer, FileTransferArray);

class FileTransfers : public wxEvtHandler
{

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

protected:
	Client *m_client;
	FileTransferArray m_transfers;
	wxTimer *tmr;

private:
	DECLARE_EVENT_TABLE()

};

#endif
