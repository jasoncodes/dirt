#ifndef FileTransfer_H_
#define FileTransfer_H_

#include "CPSCalc.h"
#include "File.h"
#include "CryptSocket.h"

#include <wx/dynarray.h>
WX_DECLARE_OBJARRAY(wxUint16, Uint16Array);

class FileTransfers;

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
	wxLongLong_t filesize;
	long time;
	long timeleft;
	long cps;
	wxLongLong_t filesent;
	wxString status;

protected:
	FileTransfer(FileTransfers *transfers);

public:
	FileTransfer(const FileTransfer &other) { wxFAIL; }
	~FileTransfer();
	wxString GetPrefixString() const;
	wxString GetStateString() const;
	operator wxString() const;
	size_t GetConnectCount() const;

protected:
	bool OnTimer(wxLongLong_t now);

protected:
	FileTransfers *m_transfers;
	File m_file;
	CPSCalc m_cps;
	CryptSocketArray m_scks;
	bool m_connect_ok;
	wxLongLong_t m_last_tick;
	bool m_cant_connect;

	//  get only
	wxArrayString m_IPs;
	Uint16Array m_ports;

	// send only
	bool m_got_accept;
	bool m_more_idle;
	wxLongLong_t m_pos;

};

#endif
