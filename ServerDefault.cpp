#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: ServerDefault.cpp,v 1.13 2003-02-16 11:01:17 jason Exp $)

#include "ServerDefault.h"

ServerDefaultConnection::ServerDefaultConnection()
{
}

ServerDefaultConnection::~ServerDefaultConnection()
{
	delete m_sck;
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
		m_event_handler->OnServerInformation(wxT("Server started on ") + GetIPV4String(addr));
		m_event_handler->OnServerStateChange();
	}
	else
	{
		m_event_handler->OnServerWarning(wxT("Error starting server. Maybe a server is already running on ") + GetIPV4String(addr));
	}
}

void ServerDefault::Stop()
{
	wxCHECK_RET(IsRunning(), wxT("Cannot stop server. Server not running."));
	m_sckListen->Close();
	m_connections.Empty();
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
					m_event_handler->OnServerInformation(wxT("Incoming connection from ") + ::GetIPV4String(addr));
				}
				break;

			case CRYPTSOCKET_LOST:
				{
					wxIPV4address addr;
					event.GetSocket()->GetPeer(addr);
					wxASSERT(conn->m_sck == event.GetSocket());
					size_t old_count = m_connections.GetCount();
					int index = m_connections.Index(*conn);
					wxASSERT(index > -1);
					m_connections.Detach(index);
					size_t new_count = m_connections.GetCount();
					wxASSERT(old_count - 1 == new_count);
					delete conn;
					m_event_handler->OnServerInformation(wxT("Connection to ") + ::GetIPV4String(addr) + wxT(" lost"));
					m_event_handler->OnServerConnectionChange();
				}
				break;

			case CRYPTSOCKET_INPUT:
				{
					m_event_handler->OnServerInformation(wxT("Input on socket: ") + event.GetData());
					conn->m_sck->Send(event.GetData());
				}
				break;

			case CRYPTSOCKET_OUTPUT:
				{
					m_event_handler->OnServerInformation(wxT("Ready to output"));
					conn->m_sck->Send(wxString(wxT("Welcome!")));
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