#ifndef ServerDefault_H_
#define ServerDefault_H_

#include "Server.h"

class ServerDefault : public Server
{

public:
	ServerDefault(ServerEventHandler *event_handler);
	virtual ~ServerDefault();

private:
	DECLARE_EVENT_TABLE()

};

#endif
