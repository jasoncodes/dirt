#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: FileTransfers.cpp,v 1.27 2003-05-11 10:40:01 jason Exp $)

#include "FileTransfer.h"
#include "FileTransfers.h"
#include "Client.h"
#include "CryptSocket.h"
#include "IPInfo.h"
#include "URL.h"
#include "CryptSocket.h"

#include <wx/filename.h>
#include "File.h"

#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY(FileTransferArray);

#ifdef ASSERT_CONNECTED
	#undef ASSERT_CONNECTED
#endif
#define ASSERT_CONNECTED() { if (!m_client->IsConnected()) { m_client->m_event_handler->OnClientWarning(context, wxT("Not connected")); return; } }

enum
{
	ID_TIMER = 1,
	ID_SOCKET_CLIENT,
	ID_SOCKET_SERVER
};

BEGIN_EVENT_TABLE(FileTransfers, wxEvtHandler)
	EVT_TIMER(ID_TIMER, FileTransfers::OnTimer)
	EVT_CRYPTSOCKET(ID_SOCKET_CLIENT, FileTransfers::OnSocket)
	EVT_CRYPTSOCKET(ID_SOCKET_SERVER, FileTransfers::OnSocket)
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

wxArrayString FileTransfers::GetMyIPs() const
{
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
	return IPs;
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
	m_client->GetConfig().SetLastSendDir(fn.GetPath(wxPATH_GET_VOLUME|wxPATH_GET_SEPARATOR));
	t->filename = fn.GetFullPath();
	t->filesize = File::Length(t->filename);
	t->time = 0;
	t->timeleft = -1;
	t->cps = -1;
	t->filesent = 0;
	t->m_cps.Reset(t->filesent);
	t->status = wxT("Waiting for accept...");

	if (!t->m_file.Open(t->filename, File::read))
	{
		delete t;
		return -1;
	}

	CryptSocketServer *sck = new CryptSocketServer;
	sck->SetEventHandler(this, ID_SOCKET_SERVER);
	sck->SetUserData(t);
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
	data.Add(wxLongLong(t->filesize).ToString());
	data.Add(wxLongLong(t->transferid).ToString());
	data.Add(ByteBuffer());
	wxArrayString IPs = GetMyIPs();
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
		if (transfer.state != ftsSendComplete && transfer.state != ftsGetComplete &&
			transfer.state != ftsSendFail && transfer.state != ftsGetFail)
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
			if (transfer.m_sck && transfer.m_sck->Ok())
			{
				transfer.m_sck->Send(wxString(wxT("CANCEL")));
			}
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

int FileTransfers::FindRemoteTransfer(int remoteid)
{
	for (int i = 0; i < GetTransferCount(); ++i)
	{
		if (m_transfers.Item(i).remoteid == remoteid)
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

wxString FileTransfers::IPMappingForConnect(const wxString &ip)
{
	wxString last_server_hostname = m_client->GetLastURL().GetHostname();
	if (last_server_hostname.Length() &&
		m_client->m_server_ip_list.Index(ip) > -1)
	{
		return last_server_hostname;
	}
	return ip;
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
				wxString ip = IPMappingForConnect(fields[i+1]);
				wxString port_str = fields[i+2];
				wxLongLong_t size;
				unsigned long id, port;
				if (!StringToLongLong(size_str, &size) ||
					!id_str.ToULong(&id) ||
					!port_str.ToULong(&port))
				{
					return false;
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
			else if (dcc_type == wxT("ACCEPT") && fields.GetCount() >= 4 && (fields.GetCount() % 2) == 0)
			{

				long id;
				int index = -1;

				if (wxString(fields[1u]).ToLong(&id) && (index = FindTransfer(id)) > -1)
				{

					FileTransfer &t = m_transfers[index];

					if (t.issend)
					{

						wxLongLong_t ll;

						if (StringToLongLong(fields[2u], &ll) && (ll < t.filesize) && t.m_file.Seek(ll) == ll)
						{
							t.filesent = ll;
							t.m_got_accept = true;
							MaybeSendData(t);
							return true;
						}

					}


				}

			}
			else if ((dcc_type == wxT("CANCELSEND") || dcc_type == wxT("CANCELGET")) && fields.GetCount() >= 2)
			{

				long id;
				bool is_cancel_get = (dcc_type == wxT("CANCELGET"));

				if (wxString(fields[1u]).ToLong(&id))
				{

					int index;
					if (is_cancel_get)
					{
						index = FindTransfer(id);
					}
					else
					{
						index = FindRemoteTransfer(id);
					}

					if (index > -1)
					{

						FileTransfer &t = m_transfers[index];

						if (is_cancel_get == t.issend)
						{
							OnRemoteCancel(t);
						}
						
					}

					return true;

				}

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
				off_t size = File::Length(ht.tail);
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

		m_client->GetConfig().SetLastGetDir(wxFileName(filename).GetPath(wxPATH_GET_VOLUME|wxPATH_GET_SEPARATOR));

		FileTransfer &t = m_transfers[index];
		
		bool open_ok;
		if (resume)
		{
			open_ok = t.m_file.Open(filename, File::write_append);
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
		t.filesent = resume ? File::Length(t.filename) : 0;
		t.status = wxT("Connecting...");
		
		CryptSocketClient *sck = new CryptSocketClient;
		t.m_sck = sck;
		sck->SetEventHandler(this, ID_SOCKET_CLIENT);
		sck->SetUserData(&t);
		sck->SetKey(m_client->GetKeyLocalPublic(), m_client->GetKeyLocalPrivate());
		wxIPV4address addr;
		addr.Hostname(t.m_ip);
		addr.Service(t.m_port);
		sck->Connect(addr);

		m_client->m_event_handler->OnClientTransferState(t);

		ByteBufferArray data;
		data.Add(wxString(wxT("ACCEPT")));
		data.Add(wxString() << t.remoteid);
		data.Add(wxLongLong(t.filesent).ToString());
		data.Add(ByteBuffer());
		m_client->CTCP(wxEmptyString, t.nickname, wxT("DCC"), Pack(data));

		return true;

	}

	return false;

}

int FileTransfers::FindTransferBySocket(CryptSocketBase *sck)
{
	for (int i = 0; i < GetTransferCount() && sck; ++i)
	{
		if (m_transfers[i].m_sck == sck)
		{
			return i;
		}
	}
	return -1;
}

void FileTransfers::OnSocket(CryptSocketEvent &event)
{

	FileTransfer *t = (FileTransfer*)event.GetUserData();
	wxASSERT(t);

	int index = FindTransferBySocket(event.GetSocket());
	
	if (index > -1)
	{

		wxASSERT(&m_transfers[index] == t);

		switch (event.GetSocketEvent())
		{

			case CRYPTSOCKET_INPUT:
				{
					ByteBuffer cmd, data;
					if (!Unpack(event.GetData(), cmd, data))
					{
						cmd = event.GetData();
						data = ByteBuffer();
					}
					if (t->issend)
					{
						OnSendData(*t, ((wxString)cmd).Upper(), data);
					}
					else
					{
						OnGetData(*t, ((wxString)cmd).Upper(), data);
					}
				}
				break;

			case CRYPTSOCKET_OUTPUT:
				if (t->issend)
				{
					MaybeSendData(*t);
				}
				break;

			case CRYPTSOCKET_CONNECTION:
				if (t->issend)
				{
					if (event.GetId() == ID_SOCKET_SERVER)
					{
						CryptSocketServer *sckListen = (CryptSocketServer*)t->m_sck;
						sckListen->SetEventHandler(NULL, wxID_ANY);
						t->m_sck = sckListen->Accept(this, ID_SOCKET_CLIENT, t);
						delete sckListen;
					}
					else
					{
						t->m_connect_ok = true;
						MaybeSendData(*t);
					}
				}
				else
				{
					t->m_connect_ok = true;
					OnGetConnection(*t);
				}
				break;

			case CRYPTSOCKET_LOST:
				OnClose(*t);
				break;

			default:
				wxFAIL_MSG(wxT("Unknown event type in FileTransfers::OnSocket"));
				break;

		}

	}

}

void FileTransfers::OnGetConnection(FileTransfer &t)
{
	t.state = ftsGetStarting;
	t.status = wxT("Connected");
	m_client->m_event_handler->OnClientTransferState(t);
}

void FileTransfers::OnRemoteCancel(FileTransfer &t)
{
	t.state = t.issend ? ftsSendFail : ftsGetFail;
	t.status = wxT("Cancelled by remote");
	m_client->m_event_handler->OnClientTransferState(t);
	DeleteTransfer(t.transferid, false);
}

void FileTransfers::OnClose(FileTransfer &t)
{
	if (t.issend)
	{
		t.state = ftsSendFail;
		t.status = t.m_connect_ok?wxT("Connection lost"):wxT("Error connecting to remote");
	}
	else
	{
		if (t.filesent == t.filesize)
		{
			t.state = ftsGetComplete;
			t.status = wxT("Transfer complete");
		}
		else
		{
			t.state = ftsGetFail;
			t.status = t.m_connect_ok?wxT("Connection lost"):wxT("Error connecting to remote");
		}
	}
	t.m_file.Close();
	m_client->m_event_handler->OnClientTransferState(t);
	DeleteTransfer(t.transferid, false);
}

void FileTransfers::OnSendData(FileTransfer &t, const wxString &cmd, const ByteBuffer &data)
{
	if (cmd == wxT("THANKS"))
	{
		wxASSERT(t.filesent == t.filesize);
		t.state = ftsGetComplete;
		t.status = wxT("Transfer complete");
		m_client->m_event_handler->OnClientTransferState(t);
		DeleteTransfer(t.transferid, false);
	}
	else if (cmd == wxT("CANCEL"))
	{
		OnRemoteCancel(t);
	}
}

void FileTransfers::OnGetData(FileTransfer &t, const wxString &cmd, const ByteBuffer &data)
{
	if (cmd == wxT("DATA"))
	{
		if (t.state != ftsSendTransfer)
		{
			t.state = ftsSendTransfer;
			t.status = wxT("Receiving...");
			m_client->m_event_handler->OnClientTransferState(t);
		}
		off_t bytes_written = t.m_file.Write(data.LockRead(), data.Length());
		data.Unlock();
		if (bytes_written != (off_t)data.Length())
		{
			t.state = ftsGetFail;
			t.status = wxT("Error writing to file");
			DeleteTransfer(t.transferid, false);
			return;
		}
		t.filesent += bytes_written;
		wxASSERT (t.filesent <= t.filesize);
		if (t.filesent == t.filesize)
		{
			t.m_sck->Send(wxString(wxT("THANKS")));
		}
	}
	else if (cmd == wxT("CANCEL"))
	{
		OnRemoteCancel(t);
	}
}

static int s_idle_stack = 0;

void FileTransfers::MaybeSendData(FileTransfer &t)
{
	wxASSERT(t.issend);
	if (t.m_connect_ok && t.m_got_accept)
	{
		if (t.state != ftsSendTransfer)
		{
			t.state = ftsSendTransfer;
			t.status = wxT("Sending...");
			m_client->m_event_handler->OnClientTransferState(t);
		}

		if (t.m_sck && t.m_sck->Ok() &&
			!t.m_sck->IsSendBufferFull() && t.filesent < t.filesize)
		{
			const off_t max_block_size = 4096;
			off_t block_size = wxMin(t.filesize - t.filesent, max_block_size);
			wxASSERT(block_size > 0);
			ByteBuffer buff(block_size);
			off_t num_read = t.m_file.Read(buff.LockReadWrite(), buff.Length());
			buff.Unlock();
			if (num_read < 0)
			{
				t.state = ftsSendFail;
				t.status = wxT("Error reading from file");
				DeleteTransfer(t.transferid, false);
				return;
			}
			wxASSERT(num_read <= block_size);
			t.m_sck->Send(Pack(wxString(wxT("DATA")),buff));
			t.filesent += buff.Length();
		}

		if (t.m_sck && t.m_sck->Ok() &&
			!t.m_sck->IsSendBufferFull() && t.filesent < t.filesize)
		{
			t.m_more_idle = true;
			if (!s_idle_stack)
			{
				wxWakeUpIdle();
			}
		}

	}
	else if (t.m_connect_ok && !t.m_got_accept)
	{
		if (t.state != ftsSendStarting)
		{
			wxASSERT(t.state == ftsSendListening);
			t.state = ftsSendStarting;
			t.status = wxT("Connecting...");
			m_client->m_event_handler->OnClientTransferState(t);
		}
		t.status = wxT("Handshaking...");
	}
	else if (!t.m_connect_ok && t.m_got_accept)
	{
		if (t.state != ftsSendStarting)
		{
			wxASSERT(t.state == ftsSendListening);
			t.state = ftsSendStarting;
			t.status = wxT("Connecting...");
			m_client->m_event_handler->OnClientTransferState(t);
		}
	}
	else
	{
		wxFAIL_MSG(wxT("Logic error in FileTransfers::MaybeSendData"));
	}
}

void FileTransfers::OnAppIdle(wxIdleEvent &event)
{
	bool more = false;
	s_idle_stack++;
	for (int i = 0; i < GetTransferCount(); ++i)
	{
		FileTransfer &t = m_transfers[i];
		if (t.m_more_idle)
		{
			t.m_more_idle = false;
			MaybeSendData(t);
			more |= t.m_more_idle;
		}
	}
	s_idle_stack--;
	if (more)
	{
		#ifdef __WXMSW__
			event.RequestMore();
		#else
			wxIdleEvent evt;
			wxTheApp->AddPendingEvent(evt);
		#endif
	}
}
