#ifndef ServerDefault_H_
#define ServerDefault_H_

#include "Server.h"
#include "CryptSocket.h"

class ServerDefault;

class ServerDefaultConnection : public ServerConnection
{

	friend ServerDefault;

protected:
	ServerDefaultConnection();

public:
	virtual ~ServerDefaultConnection();

protected:
	virtual void SendData(const ByteBuffer &data);

protected:
	CryptSocketClient *m_sck;
	ByteBuffer m_authkey;
	int m_auth_fail_count;

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
	virtual bool ProcessClientInputExtra(bool preprocess, bool prenickauthcheck, ServerConnection *conn, const wxString &context, const wxString &cmd, const ByteBuffer &data);

protected:
	void OnSocket(CryptSocketEvent &event);

protected:
	CryptSocketServer *m_sckListen;

private:
	DECLARE_EVENT_TABLE()

};

#endif
