#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: ServerDefault.cpp,v 1.41 2003-03-12 06:59:06 jason Exp $)

#include "ServerDefault.h"
#include <wx/filename.h>

const wxLongLong_t initial_ping_delay = 5000;
const wxLongLong_t ping_interval = 30000;
const wxLongLong_t ping_timeout_delay = 45000;
const long ping_timer_interval = 2500;
const long http_update_good = 5*60;
const long http_update_bad1 = 30;
const long http_update_bad2 = 15*60;

ServerDefaultConnection::ServerDefaultConnection()
{
	m_auth_fail_count = 0;
	m_nextping = GetMillisecondTicks() + initial_ping_delay;
	m_pingid = wxEmptyString;
	m_lastping = 0;
}

ServerDefaultConnection::~ServerDefaultConnection()
{
	delete m_sck;
}

void ServerDefaultConnection::SendData(const ByteBuffer &data)
{
	m_sck->Send(data);
}

void ServerDefaultConnection::Terminate(const wxString &reason)
{
	m_quitmsg = reason;
	m_sck->CloseWithEvent();
}

enum
{
	ID_SOCKET = 1,
	ID_TIMER_PING,
	ID_HTTP
};

BEGIN_EVENT_TABLE(ServerDefault, Server)
	EVT_CRYPTSOCKET(ID_SOCKET, ServerDefault::OnSocket)
	EVT_TIMER(ID_TIMER_PING, ServerDefault::OnTimerPing)
	EVT_HTTP(ID_HTTP, ServerDefault::OnHTTP)
END_EVENT_TABLE()

ServerDefault::ServerDefault(ServerEventHandler *event_handler)
	: Server(event_handler)
{
	m_sckListen = new CryptSocketServer;
	m_sckListen->SetEventHandler(this, ID_SOCKET);
	m_tmrPing = new wxTimer(this, ID_TIMER_PING);
	m_http.SetEventHandler(this, ID_HTTP);
}

ServerDefault::~ServerDefault()
{
	delete m_sckListen;
	delete m_tmrPing;
}

void ServerDefault::Start()
{
	wxCHECK_RET(!IsRunning(), wxT("Cannot start server. Server is already running."));
	wxIPV4address addr;
	addr.AnyAddress();
	addr.Service(GetConfig()->GetListenPort());
	if (m_sckListen->Listen(addr))
	{
		m_sckListen->GetLocal(addr);
		Information(wxT("Server started on ") + GetIPV4String(addr, true));
		m_event_handler->OnServerStateChange();
		m_peak_users = 0;
		m_start_tick = GetMillisecondTicks();
		m_ip_list.Empty();
		m_tmrPing->Start(ping_timer_interval);
		m_last_failed = false;
		m_last_server_name = m_config->GetServerName();
		ResetPublicListUpdate(3, true);
		wxTimerEvent evt;
		OnTimerPing(evt);
	}
	else
	{
		Warning(wxT("Error starting server. Maybe a server is already running on ") + GetIPV4String(addr, true));
	}
}

void ServerDefault::Stop()
{
	wxCHECK_RET(IsRunning(), wxT("Cannot stop server. Server not running."));
	m_sckListen->Close();
	CloseAllConnections();
	Information(wxT("Server stopped"));
	m_event_handler->OnServerStateChange();
	m_tmrPing->Stop();
	m_http.Close();
	m_list_updating = false;
}

bool ServerDefault::IsRunning() const
{
	return m_sckListen->Ok();
}

void ServerDefault::OnSocket(CryptSocketEvent &event)
{

	if (event.GetSocket() == m_sckListen)
	{

		if (event.GetSocketEvent() == CRYPTSOCKET_CONNECTION)
		{
			ServerDefaultConnection *conn = new ServerDefaultConnection;
			conn->m_sck = m_sckListen->Accept(this, ID_SOCKET, conn);
			m_connections.Add(conn);
			m_event_handler->OnServerConnectionChange();
		}
		else
		{
			wxFAIL_MSG(wxT("Unexpected message in ServerDefault::OnSocket"));
		}

	}
	else
	{

		ServerDefaultConnection *conn = (ServerDefaultConnection*)event.GetUserData();
		wxASSERT(conn);

		switch (event.GetSocketEvent())
		{

			case CRYPTSOCKET_CONNECTION:
				{
					wxIPV4address addr;
					event.GetSocket()->GetPeer(addr);
					conn->m_remotehost = ::GetIPV4String(addr, false);
					conn->m_remotehostandport =
						wxString() << conn->m_remotehost << wxT(':') << addr.Service();
					conn->m_remoteipstring = ::GetIPV4AddressString(addr);
					Information(wxT("Incoming connection from ") + conn->GetId());
					if (GetConnectionCount() > (size_t)m_config->GetMaxUsers())
					{
						conn->m_quitmsg = wxT("Too many connections");
						conn->m_sck->CloseWithEvent();
						return;
					}
					if (GetConnectionsFromHost(conn->GetRemoteHost()) > (size_t)m_config->GetMaxUsersIP())
					{
						conn->m_quitmsg = wxT("Too many connections from this IP");
						conn->m_sck->CloseWithEvent();
						return;
					}
					conn->Send(wxEmptyString, wxT("SERVERNAME"), m_config->GetServerName());
					#if wxUSE_WAVE
						wxString filename = m_config->GetSoundConnection();
						if (filename.Length() && wxFileName(filename).FileExists())
						{
							m_wave.Create(filename, false);
							if (!m_wave.IsOk() || !m_wave.Play())
							{
								Warning(wxT("Error playing ") + filename);
							}
						}
					#endif
				}
				break;

			case CRYPTSOCKET_LOST:
				{
					wxIPV4address addr;
					event.GetSocket()->GetPeer(addr);
					m_connections.Remove(conn);
					if (conn->m_quitmsg.Length() == 0)
					{
						conn->m_quitmsg = wxString(wxT("Connection lost"));
					}
					if (conn->GetNickname().Length())
					{
						Information(conn->GetId() + wxT(" has left the chat (") + conn->m_quitmsg + wxT(")"));
						SendToAll(wxEmptyString, wxT("PART"), Pack(conn->GetNickname(), conn->GetInlineDetails(), conn->m_quitmsg), true);
					}
					else
					{
						Information(conn->GetId() + wxT(" disconnected (") + conn->m_quitmsg + wxT(")"));
					}
					delete conn;
					m_event_handler->OnServerConnectionChange();
					if (GetUserCount() < 3 && GetMillisecondTicks() < GetNextPublicListUpdateTick())
					{
						ResetPublicListUpdate(10, false);
					}
				}
				break;

			case CRYPTSOCKET_INPUT:
				{
					ProcessClientInput(conn, event.GetData());
				}
				break;

			case CRYPTSOCKET_OUTPUT:
				{
					conn->Send(wxEmptyString, wxT("INFO"), wxString(wxT("Welcome to Dirt Secure Chat!")));
					if (m_config->GetPublicListComment().Length())
					{
						conn->Send(wxEmptyString, wxT("INFO"), wxT("Server comment: ") + m_config->GetPublicListComment());
					}
					conn->m_authkey = Crypt::Random(Crypt::MD5MACKeyLength);
					conn->Send(wxEmptyString, wxT("AUTHSEED"), conn->m_authkey);
					conn->m_authenticated = (m_config->GetUserPassword(true).Length() == 0);
					conn->m_nextping = 0;
					if (conn->m_authenticated)
					{
						conn->m_authenticated = true;
						conn->Send(wxEmptyString, wxT("AUTHOK"), ByteBuffer());
					}
					else
					{
						conn->Send(wxEmptyString, wxT("AUTH"), wxString(wxT("Please enter your authentication information")));
					}
				}
				break;

			default:
				wxFAIL_MSG(wxT("Unexpected message in ServerDefault::OnSocket"));

		}

	}

}

void ServerDefault::OnTimerPing(wxTimerEvent &event)
{
	wxLongLong_t now = GetMillisecondTicks();
	for (size_t i = 0; i < GetConnectionCount(); ++i)
	{
		ServerDefaultConnection *conn = (ServerDefaultConnection*)GetConnection(i);
		if (conn->m_pingid.Length())
		{
			if ((now-conn->m_lastping) > ping_timeout_delay)
			{
				conn->Terminate(wxT("Ping timeout"));
			}
		}
		else
		{
			if (conn->m_nextping < now)
			{
				conn->m_lastping = now;
				conn->m_pingid = Crypt::Random(8).GetHexDump(false, false);
				conn->Send(wxEmptyString, wxT("PING"), conn->m_pingid);
			}
		}
		if (!conn->m_authenticated)
		{
			wxTimeSpan t = wxDateTime::Now() - conn->GetJoinTime();
			if (t > wxTimeSpan(0,1,0))
			{
				conn->Terminate(wxT("Failed to authenticate in 1 minute"));
			}
		}
	}
	if (m_last_server_name != m_config->GetServerName())
	{
		m_last_server_name = m_config->GetServerName();
		SendToAll(wxEmptyString, wxT("SERVERNAME"), m_config->GetServerName(), false);
	}
	if (!(m_list_updating && !m_http.IsActive())) // don't want multiple DNS lookups at the same time
	{
		wxArrayString ip_list = GetIPAddresses();
		if (m_ip_list != ip_list)
		{
			m_ip_list = ip_list;
			if (m_ip_list.GetCount() == 1)
			{
				Information(wxT("Your IP address is: ") + m_ip_list.Item(0));
			}
			else
			{
				Information(wxT("Your IP addresses are: ") + JoinArray(m_ip_list, wxT(", ")));
			}
		}
	}
	if (m_public_server && m_next_list_update <= now && !m_list_updating)
	{
		m_list_updating = true;
		m_list_connect_ok = false;
		m_http_data.Empty();
		m_http.Close();
		m_http.ResetURLSettings();
		m_http.SetPostData(GetPublicPostData(true));
		if (m_config->GetHTTPProxyEnabled())
		{
			URL proxy;
			proxy.SetProtocol(wxT("http"));
			proxy.SetHostname(m_config->GetHTTPProxyHostname());
			proxy.SetPort(m_config->GetHTTPProxyPort());
			proxy.SetUsername(m_config->GetHTTPProxyUsername());
			proxy.SetPassword(m_config->GetHTTPProxyPassword(true));
			m_http.SetProxy(proxy);
		}
		m_http.Connect(GetPublicListURL());
	}
}

void ServerDefault::HTTPError(const wxString &errmsg)
{
	if (m_show_http_result || !m_last_failed)
	{
		Warning(wxT("Error updating public server list: ") + errmsg);
	}
	if (m_last_failed)
	{
		ResetPublicListUpdate(http_update_bad2, false);
	}
	else
	{
		ResetPublicListUpdate(http_update_bad1, false);
	}
	m_last_failed = true;
}

void ServerDefault::HTTPSuccess()
{
	if (m_show_http_result || m_last_failed)
	{
		Information(wxT("Public server list successfully updated"));
	}
	ResetPublicListUpdate(http_update_good, false);
	m_last_failed = false;
}

void ServerDefault::ResetPublicListUpdate(int num_secs_till_next_update, bool force_show)
{
	m_public_server = m_config->GetPublicListEnabled();
	m_next_list_update = GetMillisecondTicks() + num_secs_till_next_update*1000;
	m_list_updating = false;
	m_show_http_result = force_show;
	m_last_failed = false;
	m_http.Close();
	m_http_data.Empty();
}

void ServerDefault::OnHTTP(HTTPEvent &event)
{

	switch (event.GetHTTPEvent())
	{

		case HTTP_CONNECTION:
			m_list_connect_ok = true;
			m_http_data.Empty();
			break;

		case HTTP_LOST:
			if (m_list_connect_ok)
			{
				HTTPError(wxT("Transfer incomplete"));
			}
			else
			{
				HTTPError(wxT("Cannot connect to ") + m_http.GetURL().GetHostname());
			}
			break;

		case HTTP_COMPLETE:
			{
				int i = m_http_data.Lower().Find(wxT("<pre>"));
				int j = m_http_data.Lower().Find(wxT("</pre>"));
				if (i > -1 && j > i+5)
				{
					m_http_data = m_http_data.Mid(i+5, j-i-5);
					if (LeftEq(m_http_data, wxT("OK")))
					{
						HTTPSuccess();
					}
					else
					{
						HTTPError(m_http_data);
					}
				}
				else
				{
					if (m_http_data.Length() == 0)
					{
						HTTPError(wxT("Empty response"));
					}
					else
					{
						HTTPError(wxT("Error parsing response"));
					}
				}
			}
			break;

		case HTTP_HEADER:
			{
				const HTTPHeader &header = event.GetHeader();
				if (header.IsFinal())
				{
					if (header.IsError())
					{
						HTTPError(header.GetStatusLine());
					}
				}
			}
			break;

		case HTTP_DATA:
			m_http_data += event.GetData();
			break;

		default:
			HTTPError(wxT("Unexpected event type in ServerDefault::OnHTTP"));
			break;

	}

}

StringHashMap ServerDefault::GetPublicPostData(bool include_auth)
{
    
    StringHashMap post_data;
    
	post_data[wxT("name")] = m_config->GetServerName();
    if (include_auth)
	{
        wxString auth;
		auth = m_config->GetPublicListAuthentication(true);
		if (auth.Length())
		{
			auth = Crypt::MD5(auth).GetHexDump(false, false);
		}
		post_data[wxT("auth")] = auth;
    }
	wxString colon_port;
	colon_port << wxT(':') << m_config->GetListenPort();
	post_data[wxT("iplist")] = JoinArray(GetIPAddresses(), wxT(' '), wxEmptyString, colon_port);
	post_data[wxT("usercount")] = wxString() << GetUserCount();
	post_data[wxT("maxusers")] = wxString() << m_config->GetMaxUsers();
	post_data[wxT("avgping")] = wxString() << GetAverageLatency();
	post_data[wxT("version")] = GetProductVersion() + wxT(' ') + SplitHeadTail(GetRCSDate(), wxT(' ')).head;
	post_data[wxT("peakusers")] = wxString() << m_peak_users;
	post_data[wxT("uptime")] = wxString() << (long)((GetMillisecondTicks() - m_start_tick) / 1000);
	post_data[wxT("idletime")] = wxString() << GetLowestIdleTime();
	post_data[wxT("hostname")] = m_config->GetHostname() + colon_port;
	post_data[wxT("away")] = wxString() << GetAwayCount();
	post_data[wxT("comment")] = m_config->GetPublicListComment();
    
	return post_data;

}

long ServerDefault::GetListenPort() const
{
	wxASSERT(m_sckListen);
	wxIPV4address addr;
	m_sckListen->GetLocal(addr);
	return addr.Service();
}

wxLongLong_t ServerDefault::GetNextPublicListUpdateTick() const
{
	return (IsRunning() && m_public_server)?m_next_list_update:0;
}

bool ServerDefault::ProcessClientInputExtra(bool preprocess, bool prenickauthcheck, ServerConnection *conn, const wxString &context, const wxString &cmd, const ByteBuffer &data)
{
	ServerDefaultConnection *conn2 = (ServerDefaultConnection*)conn;
	if (prenickauthcheck)
	{
		if (cmd == wxT("PONG"))
		{
			if (conn2->m_pingid == data)
			{
				wxLongLong_t now = GetMillisecondTicks();
				conn2->m_nextping = now + ping_interval;
				conn2->m_pingid.Empty();
				conn2->m_latency = (now - conn2->m_lastping);
			}
			return true;
		}
		else if (cmd == wxT("AUTH"))
		{
			bool success;
			try
			{
				success = Crypt::MD5MACVerify(conn2->m_authkey, wxString(m_config->GetUserPassword(true)), data);
			}
			catch (...)
			{
				success = false;
			}
			if (success)
			{
				conn2->m_authenticated = true;
				conn2->Send(context, wxT("AUTHOK"), wxString(wxT("Authentication successful")));
				Information(conn->GetId() + wxT(" successfully authenticated"));
			}
			else
			{
				conn2->m_auth_fail_count++;
				const int max_attempts = 3;
				if (conn2->m_auth_fail_count < max_attempts)
				{
					int left = max_attempts - conn2->m_auth_fail_count;
					conn2->Send(context, wxT("AUTHBAD"), wxString::Format(wxT("Authentication failed. You have %d attempt%s remaining."), left, (left == 1)?wxT(""):wxT("s")));
					Warning(conn->GetId() + wxString::Format(wxT(" failed to authenticate (attempt %d)"), conn2->m_auth_fail_count));
				}
				else
				{
					conn2->Send(context, wxT("AUTHBAD"), wxString::Format(wxT("Failed to authenticate %d times. Disconnecting"), max_attempts));
					conn2->Terminate(wxString::Format(wxT("Authentication failed after %d attempts"), max_attempts));
				}
			}
			return true;
		}
		else if (cmd == wxT("OPER"))
		{
			ByteBuffer pass = m_config->GetAdminPassword(true);
			if (pass.Length() > 0)
			{
				bool success;
				try
				{
					success = Crypt::MD5MACVerify(conn2->m_authkey, wxString(pass), data);
				}
				catch (...)
				{
					success = false;
				}
				if (success)
				{
					if (!conn2->m_admin)
					{
						conn2->m_admin = true;
						Information(conn->GetId() + wxT(" is now a server administrator"));
					}
				}
				else
				{
					if (conn2->m_admin)
					{
						Information(conn->GetId() + wxT(" is no longer a server administrator"));
						conn2->m_admin = false;
					}
					else
					{
						Warning(conn->GetId() + wxT(" failed to become a server administrator"));
						conn2->Send(context, wxT("ERROR"), Pack(cmd, wxString(wxT("Invalid server administrator password"))));
					}
				}
			}
			else
			{
				conn2->Send(context, wxT("ERROR"), Pack(cmd, wxString(wxT("Remote administration not enabled"))));
			}
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}
