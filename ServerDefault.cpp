#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: ServerDefault.cpp,v 1.5 2003-02-15 00:36:43 jason Exp $)

#include "ServerDefault.h"

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
	m_event_handler->OnServerInformation("Server stopped");
	m_event_handler->OnServerStateChange();
}

bool ServerDefault::IsRunning()
{
	return m_sckListen->Ok();
}

void ServerDefault::OnSocket(CryptSocketEvent &event)
{
	wxFAIL_MSG("Unexpected message in ServerDefault::OnSocket");
}

int ServerDefault::GetListenPort()
{
	wxASSERT(m_sckListen);
	wxIPV4address addr;
	m_sckListen->GetLocal(addr);
	return addr.Service();
}
