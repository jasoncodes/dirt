#ifndef ServerUIFrameConfig_H_
#define ServerUIFrameConfig_H_

#include "ServerUIFrame.h"

class ServerUIFrameConfig : public wxDialog
{

public:
	ServerUIFrameConfig(ServerUIFrame *parent, Server *server);
	virtual ~ServerUIFrameConfig();

private:
	DECLARE_EVENT_TABLE()
	Server *m_server;

};

#endif
