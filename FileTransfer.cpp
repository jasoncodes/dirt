#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif

#include "FileTransfer.h"
#include "FileTransfers.h"

FileTransfer::FileTransfer()
	: transferid(-1), issend(false), state(ftsUnknown),
		nickname(wxEmptyString), filename(wxEmptyString),
		filesize(0), time(0), timeleft(0), cps(0), filesent(0),
		status(wxEmptyString)
{
}

