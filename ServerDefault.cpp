#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: ServerDefault.cpp,v 1.2 2003-02-14 04:39:57 jason Exp $)

#include "ServerDefault.h"

BEGIN_EVENT_TABLE(ServerDefault, Server)
END_EVENT_TABLE()

ServerDefault::ServerDefault(ServerEventHandler *event_handler)
	: Server(event_handler)
{
	m_event_handler->OnServerInformation("ServerDefault Ready");
}

ServerDefault::~ServerDefault()
{
}
