#ifndef ServerDefault_H_
#define ServerDefault_H_

#include "Server.h"
#include "CryptSocket.h"
#include "HTTP.h"

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
	virtual void Terminate(const wxString &reason);

protected:
	CryptSocketClient *m_sck;
	ByteBuffer m_authkey;
	int m_auth_fail_count;
	wxLongLong_t m_nextping;
	wxString m_pingid;
	wxLongLong_t m_lastping;
	wxString m_quitmsg;

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
	virtual long GetListenPort();
	virtual bool ProcessClientInputExtra(bool preprocess, bool prenickauthcheck, ServerConnection *conn, const wxString &context, const wxString &cmd, const ByteBuffer &data);
	virtual void ResetPublicListUpdate(int num_secs_till_next_update);

protected:
	virtual StringHashMap GetPublicPostData(bool include_auth);
	virtual void HTTPError(const wxString &errmsg);
	virtual void HTTPSuccess();

protected:
	void OnSocket(CryptSocketEvent &event);
	void OnTimerPing(wxTimerEvent &event);
	void OnHTTP(HTTPEvent &event);

protected:
	CryptSocketServer *m_sckListen;
	wxTimer *m_tmrPing;
	HTTP m_http;
	bool m_public_server;
	wxLongLong_t m_next_list_update;
	bool m_list_updating;
	wxString m_http_data;
	bool m_list_connect_ok;

private:
	DECLARE_EVENT_TABLE()

};

#endif
