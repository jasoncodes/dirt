#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: ServerDefault.cpp,v 1.8 2003-02-15 02:46:18 jason Exp $)

#include "ServerDefault.h"

#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY(ServerDefaultConnectionArray);

ServerDefaultConnection::ServerDefaultConnection()
{
}

ServerDefaultConnection::~ServerDefaultConnection()
{
	delete m_sck;
}

enum
{
	ID_SOCK = 1
};

BEGIN_EVENT_TABLE(ServerDefault, Server)
	EVT_CRYPTSOCKET(ID_SOCK, ServerDefault::OnSocket)
END_EVENT_TABLE()

ServerDefault::ServerDefault(ServerEventHandler *event_handler)
	: Server(event_handler)
{
	m_sckListen = new CryptSocketServer;
	m_sckListen->SetEventHandler(this, ID_SOCK);
	m_connections.Alloc(10);
}

ServerDefault::~ServerDefault()
{
	delete m_sckListen;
}

void ServerDefault::Start()
{
	wxCHECK_RET(!IsRunning(), "Cannot start server. Server is already running.");
	wxIPV4address addr;
	addr.AnyAddress();
	addr.Service(11626);
	if (m_sckListen->Listen(addr))
	{
		m_sckListen->GetLocal(addr);
		m_event_handler->OnServerInformation("Server started on " + GetIPV4String(addr));
		m_event_handler->OnServerStateChange();
	}
	else
	{
		m_event_handler->OnServerWarning("Error starting server. Maybe a server is already running on " + GetIPV4String(addr));
	}
}

void ServerDefault::Stop()
{
	wxCHECK_RET(IsRunning(), "Cannot stop server. Server not running.");
	m_sckListen->Close();
	m_connections.Empty();
	m_event_handler->OnServerInformation("Server stopped");
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
			conn->m_sck = m_sckListen->Accept();
			conn->m_sck->SetUserData(conn);
			m_connections.Add(conn);
		}
		else
		{
			wxFAIL_MSG("Unexpected message in ServerDefault::OnSocket");
		}

	}
	else
	{

		switch (event.GetSocketEvent())
		{

			case CRYPTSOCKET_CONNECTION:
				{
					wxIPV4address addr;
					event.GetSocket()->GetPeer(addr);
					m_event_handler->OnServerInformation("Incoming connection from " + ::GetIPV4String(addr));
				}
				break;

			case CRYPTSOCKET_LOST:
				{
					wxIPV4address addr;
					event.GetSocket()->GetPeer(addr);
					m_event_handler->OnServerInformation("Connection to " + ::GetIPV4String(addr) + " lost");
					ServerDefaultConnection *conn = (ServerDefaultConnection*)event.GetUserData();
					wxASSERT(conn->m_sck == event.GetSocket());
					size_t old_count = m_connections.GetCount();
					int index = m_connections.Index(*conn);
					wxASSERT(index > -1);
					m_connections.Detach(index);
					size_t new_count = m_connections.GetCount();
					wxASSERT(old_count - 1 == new_count);
					delete conn;
				}
				break;

			case CRYPTSOCKET_INPUT:
				break;

			case CRYPTSOCKET_OUTPUT:
				break;

			default:
				wxFAIL_MSG("Unexpected message in ServerDefault::OnSocket");

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
