#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: FileTransfer.cpp,v 1.11 2003-05-07 09:26:14 jason Exp $)

#include "FileTransfer.h"
#include "FileTransfers.h"
#include "CPSCalc.h"
#include "Client.h"
#include "CryptSocket.h"

#include <wx/filename.h>

FileTransfer::FileTransfer(FileTransfers *transfers)
	:
		transferid(-1), remoteid(-1), issend(false), state(ftsUnknown),
		nickname(wxEmptyString), filename(wxEmptyString),
		filesize(0), time(0), timeleft(-1), cps(-1),
		filesent(0), status(wxEmptyString), m_transfers(transfers)
{
	sck = NULL;
}

FileTransfer::~FileTransfer()
{
	delete sck;
}

bool FileTransfer::OnTimer()
{
	if (filesent >= filesize * 17 / 20)
	{
		filesent += filesize / 50;
	}
	else
	{
		filesent += filesent / 25;
	}
	if (filesent >= filesize)
	{
		filesent = filesize;
	}
	cps = m_cps.Update(filesent);
	if (filesent < filesize)
	{
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
		return true;
	}
	else
	{
		state = ftsGetComplete;
		status = wxT("Transfer complete");
		m_transfers->m_client->m_event_handler->OnClientTransferState(*this);
		m_transfers->m_client->m_event_handler->OnClientTransferTimer(*this);
		m_transfers->DeleteTransfer(transferid, false);
		return false;
	}
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
