/*
    Copyright 2002, 2003 General Software Laboratories
    
    
    This file is part of Dirt Secure Chat.

    Dirt Secure Chat is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Dirt Secure Chat is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Dirt Secure Chat; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


#ifndef FileTransfer_H_
#define FileTransfer_H_

#include "CPSCalc.h"
#include "File.h"
#include "CryptSocket.h"
#include "util.h"

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
	bool isself;
	wxString filename;
	wxLongLong_t filesize;
	long time;
	long timeleft;
	long cps;
	wxLongLong_t filesent;
	wxLongLong_t resume;
	wxString status;

protected:
	FileTransfer(FileTransfers *transfers);

public:
	FileTransfer(const FileTransfer &) { wxFAIL; }
	~FileTransfer();
	wxString GetPrefixString() const;
	wxString GetStateString() const;
	operator wxString() const;
	size_t GetConnectCount() const;
	wxLongLong_t GetUnacknowledgedCount() const;

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
	bool m_accept_sent;

	// send only
	bool m_got_accept;
	bool m_more_idle;
	wxLongLong_t m_pos;

};

#endif
