#ifndef FileTransfer_H_
#define FileTransfer_H_

#include "CPSCalc.h"
#include <wx/file.h>

class FileTransfers;
class CryptSocketBase;

enum FileTransferState
{
    
	ftsUnknown = 0,    // unknown state

	ftsSendListening,  // waiting for other user to accept
	ftsSendStarting,   // connected, other side accepted, handshaking (resume pos, etc)
	ftsSendTransfer,   // transferring
	ftsSendComplete,   // transfer complete
	ftsSendFail,       // transfer failed (connect broken, other side abort, etc)

	ftsGetPending,     // waiting for user to accept
	ftsGetConnecting,  // accepted, connecting to other host
	ftsGetStarting,    // connected, handshaking (resume pos, etc)
	ftsGetTransfer,    // transferring
	ftsGetComplete,    // transfer complete
	ftsGetFail         // transfer failed (connect broken, other side abort, etc)

};

class FileTransfer
{

	friend class FileTransfers;

public:
	int transferid;
	int remoteid;
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
	FileTransfer(FileTransfers *transfers);

public:
	FileTransfer(const FileTransfer &other) { wxFAIL; }
	virtual ~FileTransfer();
	wxString GetPrefixString() const;
	wxString GetStateString() const;
	operator wxString() const;

protected:
	bool OnTimer();

protected:
	FileTransfers *m_transfers;
	wxFile m_file;
	CPSCalc m_cps;
	CryptSocketBase *m_sck;

	//  get only
	wxString m_ip;
	unsigned short m_port;

	// send only
	bool m_connect_ok;
	bool m_got_accept;
	bool m_more_idle;

};

#endif
