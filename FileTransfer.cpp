#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: FileTransfer.cpp,v 1.4 2003-02-13 13:16:50 jason Exp $)

#include "FileTransfer.h"
#include "FileTransfers.h"
#include "CPSCalc.h"

FileTransfer::FileTransfer(FileTransfers *transfers)
	: m_transfers(transfers),
		transferid(-1), issend(false), state(ftsUnknown),
		nickname(wxEmptyString), filename(wxEmptyString),
		filesize(0), time(0), timeleft(-1), cps(-1), filesent(0),
		status(wxEmptyString)
{
}

void FileTransfer::OnTimer()
{
	if (filesent >= 300 * 1024)
	{
		filesent += 2048;
	}
	else
	{
		filesent += filesent / 50;
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
	}

}
