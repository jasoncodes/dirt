#ifndef ServerDefault_H_
#define ServerDefault_H_

#include "Server.h"
#include "CryptSocket.h"

class ServerDefault;

class ServerDefaultConnection : public ServerConnection
{

	friend ServerDefault;

public:
	virtual ~ServerDefaultConnection();

protected:
	ServerDefaultConnection();

protected:
	CryptSocketClient *m_sck;

};

class ServerDefault : public Server
{

	friend ServerDefaultConnection;

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
