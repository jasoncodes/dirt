#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: FileTransfer.cpp,v 1.7 2003-05-05 09:29:07 jason Exp $)

#include "FileTransfer.h"
#include "FileTransfers.h"
#include "CPSCalc.h"
#include "Client.h"

FileTransfer::FileTransfer(FileTransfers *transfers)
	:
		transferid(-1), issend(false), state(ftsUnknown),
		nickname(wxEmptyString), filename(wxEmptyString),
		filesize(0), time(0), timeleft(-1), cps(-1),
		filesent(0), status(wxEmptyString), m_transfers(transfers)
{
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
		m_transfers->DeleteTransfer(transferid);
		return false;
	}
}
