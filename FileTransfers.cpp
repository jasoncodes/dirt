#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: FileTransfers.cpp,v 1.59 2004-03-15 06:49:34 jason Exp $)

#include "FileTransfer.h"
#include "FileTransfers.h"
#include "Client.h"
#include "CryptSocket.h"
#include "IPInfo.h"
#include "URL.h"
#include "CryptSocket.h"
#include "CryptSocketProxy.h"

#include <wx/filename.h>
#include "File.h"

static const wxLongLong_t max_unack_count = 128 * 1024;
static const off_t max_block_size = 2 * 1024;

#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY(FileTransferArray);

#ifdef ASSERT_CONNECTED
	#undef ASSERT_CONNECTED
#endif
#define ASSERT_CONNECTED() { if (!m_client->IsConnected()) { m_client->m_event_handler->OnClientWarning(context, wxT("Not connected")); return; } }

enum
{
	ID_TIMER = 1,
	ID_TIMER_FAKE_IDLE,
	ID_SOCKET_CLIENT,
	ID_SOCKET_SERVER
};

BEGIN_EVENT_TABLE(FileTransfers, wxEvtHandler)
	EVT_TIMER(ID_TIMER, FileTransfers::OnTimer)
	EVT_TIMER(ID_TIMER_FAKE_IDLE, FileTransfers::OnTimer)
	EVT_CRYPTSOCKET(ID_SOCKET_CLIENT, FileTransfers::OnSocket)
	EVT_CRYPTSOCKET(ID_SOCKET_SERVER, FileTransfers::OnSocket)
END_EVENT_TABLE()

FileTransfers::FileTransfers(Client *client)
	: wxEvtHandler(), m_client(client)
{
	m_tmr = new wxTimer(this, ID_TIMER);
	#ifdef __WXMSW__
		m_tmrIdleEventFaker = new wxTimer(this, ID_TIMER_FAKE_IDLE);
	#endif
}

FileTransfers::~FileTransfers()
{
	delete m_tmr;
	#ifdef __WXMSW__
		delete m_tmrIdleEventFaker;
	#endif
}

void FileTransfers::OnTimer(wxTimerEvent &event)
{
	if (event.GetId() == ID_TIMER)
	{
		wxLongLong_t now = GetMillisecondTicks();
		for (int i = GetTransferCount() - 1; i >= 0; --i)
		{
			FileTransfer &transfer = m_transfers.Item(i);
			if (transfer.transferid > -1)
			{
				if (transfer.OnTimer(now))
				{
					m_client->m_event_handler->OnClientTransferTimer(transfer);
				}
			}
		}
	}
	#ifdef __WXMSW__
		// workaround to keep transfers going during scrolls/menus/drags/etc
		wxIdleEvent evt;
		OnAppIdle(evt);
		if (evt.MoreRequested())
		{
			m_tmrIdleEventFaker->Start(1, true);
		}
	#endif
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
		IPs.RemoveAt(i);
	}
	if (m_client->m_ipself.Length())
	{
		if ((i = IPs.Index(m_client->m_ipself)) > -1)
		{
			IPs.RemoveAt(i);
		}
		IPs.Insert(m_client->m_ipself, 0);
	}
	return IPs;
}

void FileTransfers::AppendMyIPs(ByteBufferArray &data, CryptSocketServer *sckServer) const
{

	if (sckServer && sckServer->GetListenPort())
	{

		wxString IP = sckServer->GetListenIP();
		wxUint16 port = sckServer->GetListenPort();

		wxArrayString IPs;
		if (IP.Length())
		{
			IPs.Add(IP);
		}
		else
		{
			IPs = GetMyIPs();
		}

		for (size_t i = 0; i < IPs.GetCount(); ++i)
		{
			data.Add(IPs[i]);
			data.Add(wxString() << (int)port);
		}

	}

}

void FileTransfers::SetProxyOnSocket(CryptSocketBase *sck, bool is_connect) const
{
	const CryptSocketProxySettings &proxy_settings = m_client->GetProxySettings();
	if ((is_connect && proxy_settings.IsEnabledForConnectionType(pctDCCConnect)) ||
		(!is_connect && proxy_settings.IsEnabledForConnectionType(pctDCCListen)))
	{
		sck->SetProxySettings(&proxy_settings);
	}
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
	t->isself = contact->IsSelf();
	wxFileName fn(filename);
	wxASSERT(fn.FileExists());
	m_client->GetConfig().SetLastSendDir(fn.GetPath(wxPATH_GET_VOLUME|wxPATH_GET_SEPARATOR));
	t->filename = fn.GetFullPath();
	t->filesize = File::Length(t->filename);
	t->time = 0;
	t->timeleft = -1;
	t->cps = -1;
	t->filesent = 0;
	t->resume = 0;
	t->m_cps.Reset();
	t->m_last_tick = GetMillisecondTicks();
	t->OnTimer(t->m_last_tick);
	t->status = wxT("Opening listen socket...");

	if (!t->m_file.Open(t->filename, File::read))
	{
		delete t;
		return -1;
	}

	CryptSocketServer *sck = new CryptSocketServer;
	sck->SetEventHandler(this, ID_SOCKET_SERVER);
	SetProxyOnSocket(sck, false);
	sck->SetUserData(t);
	sck->SetKey(m_client->GetKeyLocalPublic(), m_client->GetKeyLocalPrivate());
	t->m_scks.Add(sck);
	m_transfers.Add(t);
	sck->Listen();

	m_client->m_event_handler->OnClientTransferNew(*t);
	m_client->m_event_handler->OnClientTransferState(*t);

	if (!m_tmr->IsRunning())
	{
		m_tmr->Start(1000);
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
		}
		if (transfer.state != ftsSendComplete && transfer.state != ftsGetComplete)
		{
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
			data.Add(transfer.status);
			m_client->CTCP(wxEmptyString, transfer.nickname, wxT("DCC"), Pack(data));
			for (size_t i = 0; i < transfer.m_scks.GetCount(); ++i)
			{
				CryptSocketBase *sck = transfer.m_scks[i];
				if (sck->Ok())
				{
					sck->Send(Pack(wxString(wxT("CANCEL")), transfer.status));
				}
			}
		}
		if (transfer.state == ftsGetComplete)
		{
			m_last_completed_get_filename = transfer.filename;
		}
		m_client->m_event_handler->OnClientTransferDelete(transfer, user_initiated);
		
 		m_transfers.RemoveAt(index);
		
		if (GetTransferCount() == 0)
		{
			m_tmr->Stop();
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

int FileTransfers::FindRemoteTransfer(const wxString &nick, int remoteid)
{
	for (int i = 0; i < GetTransferCount(); ++i)
	{
		if (m_transfers[i].nickname == nick &&
			m_transfers[i].remoteid == remoteid)
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

bool FileTransfers::ExtractIPsAndPorts(const ByteBufferArray &fields, size_t i, wxArrayString &IPs, Uint16Array &ports) const
{
	while (i < fields.GetCount() && fields[i].Length())
	{
		++i;
	}
	++i;
	wxString last_server_hostname = m_client->GetLastHostname();
	Uint16Array server_ports;
	if (i <= fields.GetCount() && ((fields.GetCount()-i) % 2) == 0)
	{
		while (i < fields.GetCount())
		{
			wxString port_str = fields[i+1];
			unsigned long port;
			if (!port_str.ToULong(&port))
			{
				return false;
			}
			wxString ip = fields[i];
			if (last_server_hostname.Length() &&
				m_client->m_server_ip_list.Index(ip) > -1)
			{
				bool found = false;
				for (size_t i = 0; i < server_ports.GetCount(); ++i)
				{
					if (server_ports[i] == port)
					{
						found = true;
						break;
					}
				}
				if (!found)
				{
					server_ports.Add(port);
				}
			}
			if (wxString(ip) != last_server_hostname)
			{
				IPs.Add(ip);
				ports.Add(port);
			}
			i += 2;
		}
		for (size_t i = 0; i < server_ports.GetCount(); ++i)
		{
			IPs.Insert(last_server_hostname, i);
			ports.Insert(server_ports[i], i);
		}
		wxASSERT(IPs.GetCount() == ports.GetCount());
		return true;
	}
	return false;
}

bool FileTransfers::OnClientCTCPIn(const wxString &WXUNUSED(context), const wxString &nick, const wxString &type, const ByteBuffer &data)
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

				wxArrayString IPs;
				Uint16Array ports;
				if (!ExtractIPsAndPorts(fields, 4, IPs, ports) || IPs.GetCount() < 1)
				{
					return false;
				}

				wxLongLong_t size;
				unsigned long id;
				if (!StringToLongLong(size_str, &size) ||
					!id_str.ToULong(&id))
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
				t->resume = 0;
				t->m_IPs = IPs;
				t->m_ports = ports;
				t->m_last_tick = GetMillisecondTicks();
				t->OnTimer(t->m_last_tick);
				t->status = wxT("Accept pending...");

				m_transfers.Add(t);
				m_client->m_event_handler->OnClientTransferNew(*t);
				m_client->m_event_handler->OnClientTransferState(*t);

				if (!m_tmr->IsRunning())
				{
					m_tmr->Start(1000);
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

						if (StringToLongLong(fields[2u], &ll) && (ll < t.filesize || ll == 0) && t.m_file.Seek(ll) == ll)
						{
							
							wxArrayString IPs;
							Uint16Array ports;
							if (!ExtractIPsAndPorts(fields, 3, IPs, ports))
							{
								return false;
							}

							for (size_t i = 0; i < IPs.GetCount() && !t.m_connect_ok; ++i)
							{
								CryptSocketClient *sckClient = new CryptSocketClient;
								t.m_scks.Add(sckClient);
								sckClient->SetEventHandler(this, ID_SOCKET_CLIENT);
								SetProxyOnSocket(sckClient, true);
								sckClient->SetUserData(&t);
								sckClient->SetKey(m_client->GetKeyLocalPublic(), m_client->GetKeyLocalPrivate());
								sckClient->Connect(IPs[i], ports[i]);
							}

							t.filesent = ll;
							t.resume = ll;
							t.m_pos = ll;
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
						index = FindRemoteTransfer(nick, id);
					}

					if (index > -1)
					{

						FileTransfer &t = m_transfers[index];

						if (is_cancel_get == t.issend)
						{
							wxString msg;
							if (fields.GetCount() >= 3)
							{
								msg = fields[2u];
							}
							OnRemoteCancel(t, msg);
						}
						
					}

					return true;

				}

			}
			else if ((dcc_type == wxT("NOCONNECTSEND") || dcc_type == wxT("NOCONNECTGET")) && fields.GetCount() >= 1)
			{

				long id;
				bool is_noconnect_get = (dcc_type == wxT("NOCONNECTGET"));

				if (wxString(fields[1u]).ToLong(&id))
				{

					int index;
					if (is_noconnect_get)
					{
						index = FindTransfer(id);
					}
					else
					{
						index = FindRemoteTransfer(nick, id);
					}

					if (index > -1)
					{

						FileTransfer &t = m_transfers[index];

						if (is_noconnect_get == t.issend)
						{
							if (t.m_cant_connect)
							{
								t.state = t.issend ? ftsSendFail : ftsGetFail;
								t.status = wxT("Error connecting to remote");
								m_client->m_event_handler->OnClientTransferState(t);
								DeleteTransfer(t.transferid, false);
							}
						}
						
					}

					return true;

				}

			}

		}

	}

	return false;

}

bool FileTransfers::OnClientCTCPOut(const wxString &WXUNUSED(context), const wxString &WXUNUSED(nick), const wxString &type, const ByteBuffer &WXUNUSED(data))
{
	return (type == wxT("DCC"));
}

bool FileTransfers::OnClientCTCPReplyIn(const wxString &WXUNUSED(context), const wxString &WXUNUSED(nick), const wxString &WXUNUSED(type), const ByteBuffer &WXUNUSED(data))
{
	return false;
}

bool FileTransfers::OnClientCTCPReplyOut(const wxString &WXUNUSED(context), const wxString &WXUNUSED(nick), const wxString &WXUNUSED(type), const ByteBuffer &WXUNUSED(data))
{
	return false;
}

wxArrayString FileTransfers::GetSupportedCommands()
{
	return SplitString(wxT("ACCEPT CANCEL HELP OPEN SEND RESUME OVERWRITE"), wxT(" "));
}

void FileTransfers::ProcessConsoleInput(const wxString &context, const wxString &cmd, const wxString &params)
{
	if (cmd == wxT("HELP"))
	{
		Information(context, wxT("Supported DCC commands: ") + JoinArray(GetSupportedCommands(), wxT(" ")));
	}
	else if (cmd == wxT("OPEN"))
	{
		if (GetLastCompletedGetFilename().Length())
		{
			if (!OpenFile(NULL, GetLastCompletedGetFilename(), false))
			{
				Warning(context, wxT("Error opening ") + GetLastCompletedGetFilename());
			}
		}
		else
		{
			Warning(context, wxT("There have been no completed get transfers this session."));
		}
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
	else if (cmd == wxT("ACCEPT") || cmd == wxT("RESUME") || cmd == wxT("OVERWRITE"))
	{
		ASSERT_CONNECTED();
		HeadTail ht = SplitQuotedHeadTail(params);
		long x;
		int i;
		if (ht.head.ToLong(&x) && (i = FindTransfer(x)) > -1)
		{
			ht.tail = StripQuotes(ht.tail);
			const FileTransfer &t = GetTransferByIndex(i);
			if (t.state != ftsGetPending)
			{
				Warning(context, wxT("Transfer is not a pending get"));
				return;
			}
			ResumeState resume = rsOverwrite;
			if (!ht.tail.Length())
			{
				Warning(context, wxT("No filename specified"));
				return;
			}
			if (wxFileName(ht.tail).FileExists())
			{
				bool can_resume = File::Length(ht.tail) < t.filesize;
				if (cmd == wxT("ACCEPT"))
				{
					resume = m_client->m_event_handler->OnClientTransferResumePrompt(t, ht.tail, can_resume);
				}
				else if (cmd == wxT("RESUME"))
				{
					if (can_resume)
					{
						resume = rsResume;
					}
					else
					{
						Warning(context, wxT("Cannot resume to existing file. Please use /dcc overwrite or choose a new filename."));
						return;
					}
				}
				else if (cmd == wxT("OVERWRITE"))
				{
					resume = rsOverwrite;
				}
				else
				{
					wxFAIL_MSG(wxT("Unexpected command (")+cmd+wxT(") in FileTransfers::ProcessConsoleInput"));
					return;
				}
				if (resume == rsNotSupported)
				{
					Warning(context, wxT("File already exists. Please use /dcc resume or /dcc overwrite."));
					return;
				}
			}
			else if (cmd == wxT("RESUME"))
			{
				Warning(context, wxT("Cannot resume to not existant file. Please use /dcc accept."));
				return;
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
					<< ((t.time>-1)?SecondsToMMSS(t.time, false, true):wxString(wxT("N/A")));
				Information(context, msg);
				msg.Empty();
				msg << wxT("    Time Left: ")
					<< ((t.timeleft>-1)?SecondsToMMSS(t.timeleft, false, true):wxString(wxT("N/A")));
				Information(context, msg);
				msg.Empty();
				msg << wxT("    Speed:     ")
					<< ((t.cps>-1)?SizeToLongString(t.cps, wxT("/sec")):wxString(wxT("N/A")));
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

		wxFileName dir(filename);
		dir.SetFullName(wxEmptyString);
		if (dir.DirExists())
		{
			m_client->GetConfig().SetLastGetDir(fn.GetPath(wxPATH_GET_VOLUME|wxPATH_GET_SEPARATOR));
		}

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
		t.resume = t.filesent;
		t.m_last_tick = GetMillisecondTicks();
		
		for (size_t i = 0; i < t.m_IPs.GetCount(); ++i)
		{
			CryptSocketClient *sckClient = new CryptSocketClient;
			t.m_scks.Add(sckClient);
			sckClient->SetEventHandler(this, ID_SOCKET_CLIENT);
			SetProxyOnSocket(sckClient, true);
			sckClient->SetUserData(&t);
			sckClient->SetKey(m_client->GetKeyLocalPublic(), m_client->GetKeyLocalPrivate());
			sckClient->Connect(t.m_IPs[i], t.m_ports[i]);
		}

		t.status = wxT("Connecting...");
		m_client->m_event_handler->OnClientTransferState(t);

		CryptSocketServer *sckServer = new CryptSocketServer;
		sckServer->SetEventHandler(this, ID_SOCKET_SERVER);
		SetProxyOnSocket(sckServer, false);
		sckServer->SetUserData(&t);
		sckServer->SetKey(m_client->GetKeyLocalPublic(), m_client->GetKeyLocalPrivate());
		t.m_scks.Add(sckServer);

		sckServer->Listen();

		return true;

	}

	return false;

}

void FileTransfers::SendCTCPAccept(FileTransfer &t, CryptSocketServer *sckServer)
{
	if (!t.m_accept_sent)
	{
		ByteBufferArray data;
		data.Add(wxString(wxT("ACCEPT")));
		data.Add(wxString() << t.remoteid);
		data.Add(wxLongLong(t.filesent).ToString());
		data.Add(ByteBuffer());
		AppendMyIPs(data, sckServer);
		m_client->CTCP(wxEmptyString, t.nickname, wxT("DCC"), Pack(data));
		t.m_accept_sent = true;
	}
}

int FileTransfers::FindTransferBySocket(CryptSocketBase *sck)
{
	if (sck)
	{
		for (int i = 0; i < GetTransferCount(); ++i)
		{
			if (m_transfers[i].m_scks.Index(sck) > -1)
			{
				return i;
			}
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
					if (t->m_scks.GetCount() > 1)
					{
						t->m_scks.Remove(event.GetSocket());
						WX_CLEAR_ARRAY(t->m_scks);
						t->m_scks.Add(event.GetSocket());
					}
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
				if (event.GetId() == ID_SOCKET_SERVER)
				{
					CryptSocketServer *sckListen = (CryptSocketServer*)event.GetSocket();
					sckListen->SetEventHandler(NULL, wxID_ANY);
					SetProxyOnSocket(sckListen, false);
					sckListen->SetUserData(t);
					CryptSocketBase *sck = sckListen->Accept(this, ID_SOCKET_CLIENT, t);
					WX_CLEAR_ARRAY(t->m_scks);
					t->m_scks.Add(sck);
				}
				else
				{
					t->m_connect_ok = true;
					t->m_scks.Remove(event.GetSocket());
					WX_CLEAR_ARRAY(t->m_scks);
					t->m_scks.Add(event.GetSocket());
					if (t->issend)
					{
						MaybeSendData(*t);
					}
					else
					{
						OnGetConnection(*t);
					}
				}
				if (!t->issend)
				{
					SendCTCPAccept(*t, NULL);
				}
				break;

			case CRYPTSOCKET_LISTEN:
				{
					wxASSERT(event.GetSocket()->GetType() == cstServer);
					CryptSocketServer *sck = (CryptSocketServer*)event.GetSocket();
					if (t->issend)
					{
						ByteBufferArray data;
						data.Add(wxString(wxT("SEND")));
						data.Add(wxFileName(t->filename).GetFullName());
						data.Add(wxLongLong(t->filesize).ToString());
						data.Add(wxLongLong(t->transferid).ToString());
						data.Add(ByteBuffer());
						AppendMyIPs(data, sck);
						m_client->CTCP(wxEmptyString, t->nickname, wxT("DCC"), Pack(data));
						t->status = wxT("Waiting for accept...");
						m_client->m_event_handler->OnClientTransferState(*t);
					}
					else
					{
						SendCTCPAccept(*t, sck);
					}
	
				}
				break;

			case CRYPTSOCKET_LOST:
			case CRYPTSOCKET_ERROR:
				if (event.GetSocket()->GetType() == cstServer)
				{
					if (t->issend)
					{
						t->state = ftsSendFail;
						t->status = wxT("Error opening listen socket");
						if (event.GetData().Length())
						{
							t->status << wxT(" (") << event.GetData() << wxT(")");
						}
						m_client->m_event_handler->OnClientTransferState(*t);
						DeleteTransfer(t->transferid, false);
					}
					else
					{
						SendCTCPAccept(*t, NULL);
					}
				}
				else
				{
					if (t->m_scks.GetCount() > 1)
					{
						t->m_scks.Remove(event.GetSocket());
						delete event.GetSocket();
						if (t->GetConnectCount() == 0)
						{
							t->m_cant_connect = true;
							ByteBufferArray data;
							data.Add(wxString() << wxT("NOCONNECT") << (t->issend?wxT("SEND"):wxT("GET")));
							data.Add(wxString() << (t->issend?t->transferid:t->remoteid));
							m_client->CTCP(wxEmptyString, t->nickname, wxT("DCC"), Pack(data));
						}
					}
					else
					{
						OnClose(*t);
					}
				}
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
	t.m_last_tick = GetMillisecondTicks();
}

void FileTransfers::OnRemoteCancel(FileTransfer &t, const wxString &msg)
{
	t.state = t.issend ? ftsSendFail : ftsGetFail;
	t.status = wxT("Cancelled by remote");
	if (msg.Length())
	{
		t.status << wxT(" (Remote error: ") << msg << wxT(")");
	}
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
	t.m_last_tick = GetMillisecondTicks();
	if (cmd == wxT("THANKS"))
	{
		wxASSERT(t.filesent == t.filesize);
		t.state = ftsSendComplete;
		t.status = wxT("Transfer complete");
		m_client->m_event_handler->OnClientTransferState(t);
		DeleteTransfer(t.transferid, false);
	}
	else if (cmd == wxT("ACK"))
	{
		wxLongLong_t ll;
		if (StringToLongLong(data, &ll))
		{
			wxASSERT((ll <= t.filesize && ll > t.filesent) || (ll == 0 && t.filesize == 0));
			t.m_last_tick = GetMillisecondTicks();
			t.filesent = ll;
			if (!t.isself)
			{
				MaybeSendData(t);
			}
		}
	}
	else
	{
		OnCommonData(t, cmd, data);
	}
}

void FileTransfers::OnGetData(FileTransfer &t, const wxString &cmd, const ByteBuffer &data)
{
	t.m_last_tick = GetMillisecondTicks();
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
			m_client->m_event_handler->OnClientTransferState(t);
			DeleteTransfer(t.transferid, false);
			return;
		}
		t.filesent += bytes_written;
		wxASSERT (t.filesent <= t.filesize);
		wxASSERT(t.m_scks.GetCount() == 1);
		t.m_scks[0u]->Send(Pack(wxString(wxT("ACK")), wxLongLong(t.filesent).ToString()));
		if (t.filesent == t.filesize)
		{
			t.m_scks[0u]->Send(wxString(wxT("THANKS")));
		}
	}
	else
	{
		OnCommonData(t, cmd, data);
	}
}

void FileTransfers::OnCommonData(FileTransfer &t, const wxString &cmd, const ByteBuffer &data)
{
	if (cmd == wxT("CANCEL"))
	{
		ByteBufferArray params = Unpack(data);
		wxString msg;
		if (params.GetCount())
		{
			msg = params[0u];
		}
		OnRemoteCancel(t, msg);
	}
}

static int s_idle_stack = 0;

void FileTransfers::MaybeSendData(FileTransfer &t)
{
	if (!t.isself && t.GetUnacknowledgedCount() >= max_unack_count)
	{
		return;
	}
	bool first_time = false;
	wxASSERT(t.issend);
	if (t.m_connect_ok && t.m_got_accept)
	{
		if (t.state != ftsSendTransfer)
		{
			t.state = ftsSendTransfer;
			t.status = wxT("Sending...");
			m_client->m_event_handler->OnClientTransferState(t);
			first_time = true;
		}

		wxASSERT(t.m_scks.GetCount() == 1);
		CryptSocketBase *sck = t.m_scks[0u];

		if (sck->Ok() && !sck->IsSendBufferFull() && (t.m_pos < t.filesize || first_time))
		{
			off_t block_size = wxMin(t.filesize - t.m_pos, max_block_size);
			wxASSERT(block_size > 0 || first_time);
			ByteBuffer buff(block_size);
			if (block_size)
			{
				off_t num_read = t.m_file.Read(buff.LockReadWrite(), buff.Length());
				buff.Unlock();
				if (num_read < 0)
				{
					t.state = ftsSendFail;
					t.status = wxT("Error reading from file");
					m_client->m_event_handler->OnClientTransferState(t);
					DeleteTransfer(t.transferid, false);
					return;
				}
				wxASSERT(num_read <= block_size);
			}
			sck->Send(Pack(wxString(wxT("DATA")),buff));
			t.m_pos += buff.Length();
		}

		if (sck->Ok() && !sck->IsSendBufferFull() && t.m_pos < t.filesize)
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
		t.m_last_tick = GetMillisecondTicks();
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
		t.m_last_tick = GetMillisecondTicks();
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
