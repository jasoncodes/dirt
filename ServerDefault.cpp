#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: ServerDefault.cpp,v 1.21 2003-02-21 01:08:38 jason Exp $)

#include "ServerDefault.h"

ServerDefaultConnection::ServerDefaultConnection()
{
	m_auth_fail_count = 0;
}

ServerDefaultConnection::~ServerDefaultConnection()
{
	delete m_sck;
}

void ServerDefaultConnection::SendData(const ByteBuffer &data)
{
	m_sck->Send(data);
}

enum
{
	ID_SOCKET = 1
};

BEGIN_EVENT_TABLE(ServerDefault, Server)
	EVT_CRYPTSOCKET(ID_SOCKET, ServerDefault::OnSocket)
END_EVENT_TABLE()

ServerDefault::ServerDefault(ServerEventHandler *event_handler)
	: Server(event_handler)
{
	m_sckListen = new CryptSocketServer;
	m_sckListen->SetEventHandler(this, ID_SOCKET);
}

ServerDefault::~ServerDefault()
{
	delete m_sckListen;
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
					m_event_handler->OnServerInformation(wxT("Connection to ") + conn->GetId() + wxT(" lost"));
					SendToAll(wxEmptyString, wxT("PART"), Pack(conn->GetNickname(), conn->GetInlineDetails(), wxString(wxT("Connection lost"))), true);
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
		if (cmd == wxT("AUTH"))
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
				const int max_tries = 3;
				if (conn2->m_auth_fail_count < max_tries)
				{
					conn2->Send(context, wxT("AUTHBAD"), wxString::Format(wxT("Authentication failed. You have %d tries remaining."), max_tries - conn2->m_auth_fail_count));
					m_event_handler->OnServerInformation(conn->GetId() + wxString::Format(wxT(" failed to authenticate (try %d)"), conn2->m_auth_fail_count));
				}
				else
				{
					conn2->Send(context, wxT("AUTHBAD"), wxString::Format(wxT("Failed to authenticate %d times. Disconnecting"), max_tries));
					m_event_handler->OnServerInformation(conn->GetId() + wxString::Format(wxT(" failed to authenticate (try %d, disconnecting)"), conn2->m_auth_fail_count));
					conn2->m_sck->CloseWithEvent();
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