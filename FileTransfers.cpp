#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: FileTransfers.cpp,v 1.21 2003-05-08 00:56:26 jason Exp $)

#include "FileTransfer.h"
#include "FileTransfers.h"
#include "Client.h"
#include "CryptSocket.h"
#include "IPInfo.h"
#include "URL.h"

#include <wx/filename.h>

#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY(FileTransferArray);

#ifdef ASSERT_CONNECTED
	#undef ASSERT_CONNECTED
#endif
#define ASSERT_CONNECTED() { if (!m_client->IsConnected()) { m_client->m_event_handler->OnClientWarning(context, wxT("Not connected")); return; } }

enum
{
	ID_TIMER = 1,
	ID_SOCKET
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

int FileTransfers::SendFile(const wxString &nickname, const wxString &filename)
{

	ClientContact *contact = m_client->GetContact(nickname);
	wxASSERT(contact);

	FileTransfer *t = new FileTransfer(this);
	
	t->transferid = GetNewId();
	t->remoteid = -1;
	t->issend = true;
	t->state = ftsSendListening;
	t->nickname = contact->GetNickname();
	wxFileName fn(filename);
	wxASSERT(fn.FileExists());
	t->filename = fn.GetFullPath();
	t->filesize = GetFileLength(t->filename);
	t->time = 0;
	t->timeleft = -1;
	t->cps = -1;
	t->filesent = 0;
	t->m_cps.Reset(t->filesent);
	t->status = wxT("Waiting for accept...");

	if (!t->m_file.Open(t->filename, wxFile::read))
	{
		delete t;
		return -1;
	}

	CryptSocketServer *sck = new CryptSocketServer;
	sck->SetEventHandler(this, ID_SOCKET);
	sck->SetKey(m_client->GetKeyLocalPublic(), m_client->GetKeyLocalPrivate());
	wxIPV4address addr;
	addr.AnyAddress();
	addr.Service(0);
	if (!sck->Listen(addr))
	{
		delete t;
		return -1;
	}
	sck->GetLocal(addr);
	t->m_sck = sck;

	m_transfers.Add(t);

	ByteBufferArray data;
	data.Add(wxString(wxT("SEND")));
	data.Add(wxFileName(t->filename).GetFullName());
	data.Add(wxString() << t->filesize);
	data.Add(wxString() << t->transferid);
	data.Add(ByteBuffer());
	wxArrayString IPs = GetIPAddresses();
	int i;
	if ((i = IPs.Index(wxT("127.0.0.1"))) > -1)
	{
		IPs.Remove(i);
	}
	if (m_client->m_ipself.Length())
	{
		if ((i = IPs.Index(m_client->m_ipself)) > -1)
		{
			IPs.Remove(i);
		}
		IPs.Insert(m_client->m_ipself, 0);
	}
	for (size_t i = 0; i < IPs.GetCount(); ++i)
	{
		data.Add(IPs[i]);
		data.Add(wxString() << (int)addr.Service());
	}
	m_client->CTCP(wxEmptyString, t->nickname, wxT("DCC"), Pack(data));
	
	m_client->m_event_handler->OnClientTransferNew(*t);
	m_client->m_event_handler->OnClientTransferState(*t);

	if (!tmr->IsRunning())
	{
		tmr->Start(1000);
	}

	return t->transferid;

}

bool FileTransfers::DeleteTransfer(int transferid, bool user_initiated)
{
	
	int index = FindTransfer(transferid);
	
	if (index > -1)
	{
	
		FileTransfer &transfer = m_transfers.Item(index);
		if (transfer.state != ftsSendComplete && transfer.state != ftsGetComplete)
		{
			transfer.state = transfer.issend ? ftsSendFail : ftsGetFail;
			transfer.status = wxT("Transfer cancelled");
			m_client->m_event_handler->OnClientTransferState(transfer);
			ByteBufferArray data;
			if (transfer.issend)
			{
				data.Add(wxString(wxT("CANCELSEND")));
				data.Add(wxString() << transfer.transferid);
			}
			else
			{
				data.Add(wxString(wxT("CANCELGET")));
				data.Add(wxString() << transfer.remoteid);
			}
			m_client->CTCP(wxEmptyString, transfer.nickname, wxT("DCC"), Pack(data));
		}
		m_client->m_event_handler->OnClientTransferDelete(transfer, user_initiated);
		
 		m_transfers.RemoveAt(index);
		
		if (GetTransferCount() == 0)
		{
			tmr->Stop();
		}

		return true;

	}

	return false;

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

	if (type == wxT("DCC"))
	{

		ByteBufferArray fields = Unpack(data);
		if (fields.GetCount() && fields[0u].Length())
		{

			wxString dcc_type = ((wxString)fields[0u]).Upper();
			if (dcc_type == wxT("SEND") && fields.GetCount() >= 7 && (fields.Count() % 2) == 1)
			{

				wxString filename = wxFileName(fields[1u]).GetFullName();
				wxString size_str = fields[2u];
				wxString id_str = fields[3u];
				size_t i = 4u;
				while (i < fields.GetCount() && fields[i].Length())
				{
					++i;
				}
				if (i+2 >= fields.GetCount())
				{
					return false;
				}
				wxString ip = fields[i+1];
				wxString port_str = fields[i+2];
				unsigned long size, id, port;
				if (!size_str.ToULong(&size) ||
					!id_str.ToULong(&id) ||
					!port_str.ToULong(&port))
				{
					return false;
				}

				wxString last_server_hostname = m_client->GetLastURL().GetHostname();
				if (last_server_hostname.Length() &&
					m_client->m_server_ip_list.Index(ip) > -1)
				{
					ip = last_server_hostname;
				}

				FileTransfer *t = new FileTransfer(this);
				t->transferid = GetNewId();
				t->remoteid = id;
				t->issend = false;
				t->state = ftsGetPending;
				t->nickname = nick;
				t->filename = filename;
				t->filesize = size;
				t->time = 0;
				t->timeleft = -1;
				t->cps = -1;
				t->filesent = 0;
				t->status = wxT("Accept pending...");
				t->m_ip = ip;
				t->m_port = port;

				m_transfers.Add(t);
				m_client->m_event_handler->OnClientTransferNew(*t);
				m_client->m_event_handler->OnClientTransferState(*t);

				if (!tmr->IsRunning())
				{
					tmr->Start(1000);
				}

				return true;

			}

		}

	}

	return false;

}

bool FileTransfers::OnClientCTCPOut(const wxString &context, const wxString &nick, const wxString &type, const ByteBuffer &data)
{
	return (type == wxT("DCC"));
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
	return SplitString(wxT("ACCEPT CANCEL HELP SEND"), wxT(" "));
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
	else if (cmd == wxT("SEND"))
	{
		ASSERT_CONNECTED();
		HeadTail ht = SplitQuotedHeadTail(params);
		if (m_client->GetContact(ht.head))
		{
			ht.tail = StripQuotes(ht.tail);
			if (wxFileName(ht.tail).FileExists())
			{
				int transferid = SendFile(ht.head, ht.tail);
				if (transferid < 0)
				{
					Warning(context, wxT("Error sending \"") + ht.tail + wxT("\" to ") + ht.head);
				}
			}
			else
			{
				Warning(context, wxT("No such file: ") + ht.tail);
			}
		}
		else
		{
			Warning(context, wxT("No such nick: ") + ht.head);
		}
	}
	else if (cmd == wxT("ACCEPT"))
	{
		ASSERT_CONNECTED();
		HeadTail ht = SplitQuotedHeadTail(params);
		long x;
		int i;
		if (ht.head.ToLong(&x) && (i = FindTransfer(x)) > -1)
		{
			ht.tail = StripQuotes(ht.tail);
			const FileTransfer &t = GetTransferByIndex(i);
			ResumeState resume = rsOverwrite;
			if (wxFileName(ht.tail).FileExists())
			{
				off_t size = GetFileLength(ht.tail);
				resume = m_client->m_event_handler->OnClientTransferResumePrompt(t, ht.tail, size < t.filesize);
			}
			if ((resume != rsCancel) && !AcceptTransfer(x, ht.tail, resume == rsResume))
			{
				Warning(context, wxT("Error accepting transfer ") + ht.head);
			}
		}
		else
		{
			Warning(context, wxT("No such transfer: ") + ht.head);
		}
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
					str << wxT("    ") << t;
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

bool FileTransfers::AcceptTransfer(int transferid, const wxString &filename, bool resume)
{
	
	int index = FindTransfer(transferid);
	
	wxFileName fn(filename);
	wxASSERT(!resume || fn.FileExists());
	
	if (index > -1)
	{

		FileTransfer &t = m_transfers[index];
		
		bool open_ok;
		if (resume)
		{
			open_ok = t.m_file.Open(filename, wxFile::write_append);
		}
		else
		{
			open_ok = t.m_file.Create(filename, true);
		}
		if (!open_ok)
		{
			return false;
		}

		t.filename = fn.GetFullPath();
		t.state = ftsGetConnecting;
		t.filesent = resume ? GetFileLength(t.filename) : 0;
		t.status = wxT("Connecting...");
		
		CryptSocketClient *sck = new CryptSocketClient;
		t.m_sck = sck;
		sck->SetEventHandler(this, ID_SOCKET);
		sck->SetKey(m_client->GetKeyLocalPublic(), m_client->GetKeyLocalPrivate());
		wxIPV4address addr;
		addr.Hostname(t.m_ip);
		addr.Service(t.m_port);
		sck->Connect(addr);

		m_client->m_event_handler->OnClientTransferState(t);

		ByteBufferArray data;
		data.Add(wxString(wxT("ACCEPT")));
		data.Add(wxString() << t.remoteid);
		data.Add(wxString() << t.filesent);
		data.Add(ByteBuffer());
		m_client->CTCP(wxEmptyString, t.nickname, wxT("DCC"), Pack(data));

		return true;

	}

	return false;

}
