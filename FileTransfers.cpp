#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: FileTransfers.cpp,v 1.16 2003-05-07 06:47:02 jason Exp $)

#include "FileTransfer.h"
#include "FileTransfers.h"
#include "Client.h"

#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY(FileTransferArray);

#ifdef ASSERT_CONNECTED
	#undef ASSERT_CONNECTED
#endif
#define ASSERT_CONNECTED() { if (!m_client->IsConnected()) { m_client->m_event_handler->OnClientWarning(context, wxT("Not connected")); return; } }

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
	for (int i = GetTransferCount() - 1; i >= 0; --i)
	{
		FileTransfer &transfer = m_transfers.Item(i);
		if (transfer.transferid > -1)
		{
			if (transfer.OnTimer())
			{
				m_client->m_event_handler->OnClientTransferTimer(transfer);
			}
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

#include <sys/stat.h>

static off_t GetFileLength(const wxString &filename)
{
	wxStructStat st;
	if (wxStat(filename, &st) == 0)
	{
		return st.st_size;
	}
	return -1;
}

void FileTransfers::Test()
{

	wxString context;
	ASSERT_CONNECTED();

	FileTransfer t(this);
	
	t.transferid = GetNewId();
	t.issend = true;
	t.state = ftsSendTransfer;
	t.nickname = m_client->GetNickname();
	t.filename = GetSelf();
	t.filesize = GetFileLength(t.filename);
	t.time = t.filesize / 3000;
	t.timeleft = -1;
	t.cps = -1;
	t.filesent = t.filesize/3*2;
	t.m_cps.Reset(t.filesent);
	t.status = wxT("Sending...");

	m_transfers.Add(t);
	
	m_client->m_event_handler->OnClientTransferNew(t);
	m_client->m_event_handler->OnClientTransferState(t);

	if (!tmr->IsRunning())
	{
		tmr->Start(1000);
	}

}

void FileTransfers::DeleteTransfer(int transferid, bool user_initiated)
{
	
	int index = FindTransfer(transferid);
	
	wxASSERT(index > -1);
	
	FileTransfer &transfer = m_transfers.Item(index);
	if (transfer.state != ftsSendComplete && transfer.state != ftsGetComplete)
	{
		transfer.state = transfer.issend ? ftsSendFail : ftsGetFail;
		transfer.status = wxT("Transfer cancelled");
		m_client->m_event_handler->OnClientTransferState(transfer);
	}
	m_client->m_event_handler->OnClientTransferDelete(transfer, user_initiated);
	
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

void FileTransfers::OnClientUserNick(const wxString &old_nick, const wxString &new_nick)
{
	for (int i = 0; i < GetTransferCount(); ++i)
	{
		if (m_transfers.Item(i).nickname == old_nick)
		{
			m_transfers.Item(i).nickname = new_nick;
			m_client->m_event_handler->OnClientTransferTimer(m_transfers.Item(i));
		}
	}
}
	
bool FileTransfers::OnClientCTCPIn(const wxString &context, const wxString &nick, const wxString &type, const ByteBuffer &data)
{
	return false;
}

bool FileTransfers::OnClientCTCPOut(const wxString &context, const wxString &nick, const wxString &type, const ByteBuffer &data)
{
	return false;
}

bool FileTransfers::OnClientCTCPReplyIn(const wxString &context, const wxString &nick, const wxString &type, const ByteBuffer &data)
{
	return false;
}

bool FileTransfers::OnClientCTCPReplyOut(const wxString &context, const wxString &nick, const wxString &type, const ByteBuffer &data)
{
	return false;
}

wxArrayString FileTransfers::GetSupportedCommands()
{
	return SplitString(wxT("CANCEL HELP TEST"), wxT(" "));
}

void FileTransfers::ProcessConsoleInput(const wxString &context, const wxString &cmd, const wxString &params)
{
	if (cmd == wxT("HELP"))
	{
		Information(context, wxT("Supported DCC commands: ") + JoinArray(GetSupportedCommands(), wxT(" ")));
	}
	else if (cmd == wxT("CANCEL"))
	{
		long x;
		int i;
		if (params.ToLong(&x) && (i = FindTransfer(x)) > -1)
		{
			DeleteTransfer(x, true);
		}
		else
		{
			Warning(context, wxT("No such transfer: ") + params);
		}
	}
	else if (cmd == wxT("TEST"))
	{
		ASSERT_CONNECTED();
		Test();
	}
	else if (cmd == wxT("STATUS") || cmd == wxT(""))
	{
		if (params.Length() && cmd.Length())
		{
			long x;
			int i;
			if (params.ToLong(&x) && (i = FindTransfer(x)) > -1)
			{
				const FileTransfer& t = GetTransferByIndex(i);
				wxString msg;
				msg << wxT("Information on transfer number ") << t.transferid << wxT(":");
				Information(context, msg);
				msg.Empty();
				msg << wxT("    Nickname:  ")
					<< t.nickname;
				Information(context, msg);
				msg.Empty();
				msg << wxT("    Filename:  ")
					<< t.filename;
				Information(context, msg);
				msg.Empty();
				msg << wxT("    Type:      ")
					<< (t.issend?wxT("Send"):wxT("Get"));
				Information(context, msg);
				msg.Empty();
				msg << wxT("    Status:    ")
					<< t.status;
				Information(context, msg);
				msg.Empty();
				msg << wxT("    File Size: ")
					<< SizeToLongString(t.filesize);
				Information(context, msg);
				msg.Empty();
				msg << wxT("    File Sent: ")
					<< SizeToLongString(t.filesent);
				Information(context, msg);
				msg.Empty();
				msg << wxT("    Time:      ")
					<< ((t.time>-1)?SecondsToMMSS(t.time, false, true):wxT("N/A"));
				Information(context, msg);
				msg.Empty();
				msg << wxT("    Time Left: ")
					<< ((t.timeleft>-1)?SecondsToMMSS(t.timeleft, false, true):wxT("N/A"));
				Information(context, msg);
				msg.Empty();
				msg << wxT("    Speed:     ")
					<< ((t.cps>-1)?SizeToLongString(t.cps, wxT("/sec")):wxT("N/A"));
				Information(context, msg);
				msg.Empty();
			}
			else
			{
				Warning(context, wxT("No such transfer: ") + params);
			}
		}
		else
		{
			if (GetTransferCount())
			{
				Information(context, wxString() << wxT("There are ") << GetTransferCount() << wxT(" active transfers:"));
				for (int i = 0; i < GetTransferCount(); ++i)
				{
					const FileTransfer& t = GetTransferByIndex(i);
					wxString str;
					str << wxT("    ") << t.transferid << wxT(". ") << t;
					Information(context, str);
				}
			}
			else
			{
				Information(context, wxT("No active transfers"));
			}
		}
	}
	else
	{
		Warning(context, wxT("Unrecognized DCC command: ") + cmd);
	}
}

bool FileTransfers::OnClientPreprocess(const wxString &context, const wxString &cmd, const wxString &params)
{
	if (cmd == wxT("DCC"))
	{
		HeadTail ht = SplitQuotedHeadTail(params);
		ht.head.MakeUpper();
		ProcessConsoleInput(context, ht.head, ht.tail);
		return true;
	}
	else
	{
		return false;
	}
}

wxArrayString FileTransfers::OnClientSupportedCommands()
{
	return SplitString(wxT("DCC"), wxT(" "));
}

void FileTransfers::Information(const wxString &context, const wxString &text)
{
	m_client->m_event_handler->OnClientInformation(context, text);
}

void FileTransfers::Warning(const wxString &context, const wxString &text)
{
	m_client->m_event_handler->OnClientWarning(context, text);
}
