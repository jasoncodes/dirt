#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: ServerDefault.cpp,v 1.24 2003-02-21 07:53:14 jason Exp $)

#include "ServerDefault.h"

const wxLongLong_t initial_ping_delay = 5000;
const wxLongLong_t ping_interval = 30000;
const wxLongLong_t ping_timeout_delay = 45000;
const long ping_timer_interval = 2500;

static wxLongLong_t GetMillisecondTicks()
{
	#ifdef __WXMSW__
		return ::timeGetTime();
	#else
		return wxGetLocalTimeMillis().GetValue();
	#endif
}

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
	ID_TIMER_PING
};

BEGIN_EVENT_TABLE(ServerDefault, Server)
	EVT_CRYPTSOCKET(ID_SOCKET, ServerDefault::OnSocket)
	EVT_TIMER(ID_TIMER_PING, ServerDefault::OnTimerPing)
END_EVENT_TABLE()

ServerDefault::ServerDefault(ServerEventHandler *event_handler)
	: Server(event_handler)
{
	m_sckListen = new CryptSocketServer;
	m_sckListen->SetEventHandler(this, ID_SOCKET);
	m_tmrPing = new wxTimer(this, ID_TIMER_PING);
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
	addr.Service(11626);
	if (m_sckListen->Listen(addr))
	{
		m_sckListen->GetLocal(addr);
		m_event_handler->OnServerInformation(wxT("Server started on ") + GetIPV4String(addr, true));
		m_event_handler->OnServerStateChange();
		m_tmrPing->Start(ping_timer_interval);
	}
	else
	{
		m_event_handler->OnServerWarning(wxT("Error starting server. Maybe a server is already running on ") + GetIPV4String(addr, true));
	}
}

void ServerDefault::Stop()
{
	wxCHECK_RET(IsRunning(), wxT("Cannot stop server. Server not running."));
	m_sckListen->Close();
	CloseAllConnections();
	m_event_handler->OnServerInformation(wxT("Server stopped"));
	m_event_handler->OnServerStateChange();
	m_tmrPing->Stop();
}

bool ServerDefault::IsRunning()
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
					m_event_handler->OnServerInformation(wxT("Incoming connection from ") + conn->GetId());
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
						m_event_handler->OnServerInformation(conn->GetId() + wxT(" has left the chat (") + conn->m_quitmsg + wxT(")"));
						SendToAll(wxEmptyString, wxT("PART"), Pack(conn->GetNickname(), conn->GetInlineDetails(), conn->m_quitmsg), true);
					}
					else
					{
						m_event_handler->OnServerInformation(conn->GetId() + wxT(" disconnected (") + conn->m_quitmsg + wxT(")"));
					}
					delete conn;
					m_event_handler->OnServerConnectionChange();
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
					conn->m_authkey = Crypt::Random(Crypt::MD5MACKeyLength);
					conn->Send(wxEmptyString, wxT("AUTHSEED"), conn->m_authkey);
					conn->m_authenticated = false;
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
}

int ServerDefault::GetListenPort()
{
	wxASSERT(m_sckListen);
	wxIPV4address addr;
	m_sckListen->GetLocal(addr);
	return addr.Service();
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
				success = Crypt::MD5MACVerify(conn2->m_authkey, wxString(wxT("test")), data);
			}
			catch (...)
			{
				success = false;
			}
			if (success)
			{
				conn2->m_authenticated = true;
				conn2->Send(context, wxT("AUTHOK"), wxString(wxT("Authentication successful")));
				m_event_handler->OnServerInformation(conn->GetId() + wxT(" successfully authenticated"));
			}
			else
			{
				conn2->m_auth_fail_count++;
				const int max_attempts = 3;
				if (conn2->m_auth_fail_count < max_attempts)
				{
					int left = max_attempts - conn2->m_auth_fail_count;
					conn2->Send(context, wxT("AUTHBAD"), wxString::Format(wxT("Authentication failed. You have %d attempt%s remaining."), left, (left == 1)?wxT(""):wxT("s")));
					m_event_handler->OnServerInformation(conn->GetId() + wxString::Format(wxT(" failed to authenticate (attempt %d)"), conn2->m_auth_fail_count));
				}
				else
				{
					conn2->Send(context, wxT("AUTHBAD"), wxString::Format(wxT("Failed to authenticate %d times. Disconnecting"), max_attempts));
					conn2->Terminate(wxString::Format(wxT("Authentication failed after %d attempts"), max_attempts));
				}
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

