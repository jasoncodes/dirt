#ifndef ServerDefault_H_
#define ServerDefault_H_

#include "Server.h"
#include "CryptSocket.h"

class ServerDefault : public Server
{

public:
	ServerDefault(ServerEventHandler *event_handler);
	virtual ~ServerDefault();

	virtual void Start();
	virtual void Stop();
	virtual bool IsRunning();
	virtual int GetListenPort();

protected:
	void OnSocket(CryptSocketEvent &event);

protected:
	CryptSocketServer *m_sckListen;

private:
	DECLARE_EVENT_TABLE()

};

#endif
