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


#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: FileTransfer.cpp,v 1.28 2004-05-16 04:42:45 jason Exp $)

#include "FileTransfer.h"
#include "FileTransfers.h"
#include "CPSCalc.h"
#include "Client.h"
#include "CryptSocket.h"

#include <wx/filename.h>

FileTransfer::FileTransfer(FileTransfers *transfers)
	:
		transferid(-1), remoteid(-1), issend(false), state(ftsUnknown),
		nickname(wxEmptyString), isself(false), filename(wxEmptyString),
		filesize(0), time(0), timeleft(-1), cps(-1),
		filesent(0), resume(0),
		status(wxEmptyString), m_transfers(transfers),
		m_connect_ok(false), m_cant_connect(false), m_accept_sent(false),
		m_got_accept(false), m_more_idle(false), m_pos(0)
{
}

FileTransfer::~FileTransfer()
{
	WX_CLEAR_ARRAY(m_scks);
	m_file.Close();
	if (filesize && wxFileName::FileExists(filename) && !File::Length(filename))
	{
		wxRemoveFile(filename);
	}
}

static const wxLongLong_t s_timeout_pending = 180000;
static const wxLongLong_t s_timeout_transfer = 60000;

bool FileTransfer::OnTimer(wxLongLong_t now)
{
	cps = 0;
	timeleft = 0;
	wxLongLong_t timeout_tick;
	if (state == ftsSendTransfer || state == ftsGetTransfer)
	{
		if (filesent < filesize)
		{
			cps = m_cps.Update(filesent);
			time++;
			int bytesleft = filesize - filesent;
			if (cps > 0)
			{
				timeleft = bytesleft / cps;
			}
			else
			{
				timeleft = -1;
			}
		}
		timeout_tick = m_last_tick + s_timeout_transfer;
	}
	else
	{
		timeout_tick = m_last_tick + s_timeout_pending;
	}
	if (now >= timeout_tick)
	{
		state = issend ? ftsSendFail : ftsGetFail;
		status = wxT("Transfer timed out");
		m_transfers->m_client->m_event_handler->OnClientTransferState(*this);
		m_transfers->DeleteTransfer(transferid, false);
		return false;
	}
	if (state == ftsSendListening || state == ftsGetPending)
	{
		if (state == ftsSendListening)
		{
			status = wxT("Waiting for accept");
		}
		else
		{
			status = wxT("Accept pending");
		}
		status << wxT("... [")
			<< SecondsToMMSS((timeout_tick - now) / 1000)
			<< wxT("]");
	}
	return true;
}

wxString FileTransfer::GetPrefixString() const
{
	if (filename.Length() && nickname.Length())
	{
		wxString str;
		str << (issend ? wxT("Send ") : wxT("Get "))
			<< wxFileName(filename).GetFullName()
			<< (issend ? wxT(" to ") : wxT(" from "))
			<< nickname
			<< wxT(": ");
		return str;
	}
	return wxEmptyString;
}

wxString FileTransfer::GetStateString() const
{

	wxString str;

	str << status << wxT(" (");

	str << SizeToString(filesent);
	str << wxT(" of ");
	str << SizeToString(filesize);

	str << wxT(" @ ");

	if (cps > -1)
	{
		str << SizeToString(cps);
	}
	else
	{
		str << wxT("??? KB");
	}
	str << wxT("/sec, ");
	
	if (timeleft > -1)
	{
		str << SecondsToMMSS(timeleft);
	}
	else
	{
		str << wxT("??:??");
	}
	str << wxT(" left");
	
	str << wxT(")");

	return str;

}

FileTransfer::operator wxString() const
{
	wxString str;
	str	<< transferid << wxT(": ")
		<< GetPrefixString() << GetStateString();
	return str;
}

size_t FileTransfer::GetConnectCount() const
{
	size_t num_connects = 0;
	for (size_t i = 0; i < m_scks.GetCount(); ++i)
	{
		if (m_scks[i]->GetType() == cstClient)
		{
			num_connects++;
		}
	}
	return num_connects;
}

wxLongLong_t FileTransfer::GetUnacknowledgedCount() const
{
	wxASSERT(issend);
	wxASSERT(filesent <= m_pos);
	return m_pos - filesent;
}
