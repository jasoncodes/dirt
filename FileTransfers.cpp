#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: FileTransfers.cpp,v 1.5 2003-02-13 13:16:50 jason Exp $)

#include "FileTransfer.h"
#include "FileTransfers.h"
#include "Client.h"

#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY(FileTransferArray);

enum
{
	ID_TIMER = 1
};

BEGIN_EVENT_TABLE(FileTransfers, wxEvtHandler)
	EVT_TIMER(ID_TIMER, FileTransfers::OnTimer)
END_EVENT_TABLE()

FileTransfers::FileTransfers(Client *client)
	: wxEvtHandler(), m_client(client)
{
	tmr = new wxTimer(this, ID_TIMER);
}

FileTransfers::~FileTransfers()
{
	delete tmr;
}

void FileTransfers::OnTimer(wxTimerEvent &event)
{
	for (int i = 0; i < GetTransferCount(); ++i)
	{
		FileTransfer &transfer = m_transfers.Item(i);
		if (transfer.transferid > -1)
		{
			transfer.OnTimer();
			m_client->m_event_handler->OnClientTransferTimer(transfer);
		}
	}
}

int FileTransfers::GetNewId()
{
	int index = 1;
	while (FindTransfer(index) != -1)
	{
		index++;
	}
	return index;
}

void FileTransfers::Test()
{

	FileTransfer t(this);
	
	t.transferid = GetNewId();
	t.issend = true;
	t.state = ftsSendTransfer;
	t.nickname = "Jason";
	t.filename = "D:\\Archive\\Stuff\\Dirt.exe";
	t.filesize = 363520;
	t.time = 133;
	t.timeleft = -1;
	t.cps = -1;
	t.filesent = 363520/3*2;
	t.status = "Sending...";

	m_transfers.Add(t);
	
	m_client->m_event_handler->OnClientTransferNew(t);

	if (!tmr->IsRunning())
	{
		tmr->Start(1000);
	}

}

void FileTransfers::DeleteTransfer(int transferid)
{
	
	int index = FindTransfer(transferid);
	
	wxASSERT(index > -1);
	
	// scope to prevent invalid reference when remove below
	{
		const FileTransfer &transfer = m_transfers.Item(index);
		m_client->m_event_handler->OnClientTransferDelete(transfer);
	}
	
	m_transfers.RemoveAt(index);
	
	if (GetTransferCount() == 0)
	{
		tmr->Stop();
	}

}

int FileTransfers::GetTransferCount()
{
	return m_transfers.GetCount();
}

int FileTransfers::FindTransfer(int transferid)
{
	for (int i = 0; i < GetTransferCount(); ++i)
	{
		if (m_transfers.Item(i).transferid == transferid)
		{
			return i;
		}
	}
	return -1;
}

const FileTransfer& FileTransfers::GetTransferById(int transferid)
{
	return GetTransferByIndex(FindTransfer(transferid));
}

const FileTransfer& FileTransfers::GetTransferByIndex(int index)
{
	wxASSERT(index > -1 && index < GetTransferCount());
	return m_transfers.Item(index);
}
