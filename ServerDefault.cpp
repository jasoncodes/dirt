#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: ServerDefault.cpp,v 1.1 2003-02-14 03:57:00 jason Exp $)

#include "ServerDefault.h"

BEGIN_EVENT_TABLE(ServerDefault, Server)
END_EVENT_TABLE()

ServerDefault::ServerDefault(ServerEventHandler *event_handler)
	: Server(event_handler)
{
	m_event_handler->OnServerLog("ServerDefault Ready");
}

ServerDefault::~ServerDefault()
{
}
