#ifndef FileTransfer_H_
#define FileTransfer_H_

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
	FileTransfer();

};

#endif
