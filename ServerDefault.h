#ifndef ServerDefault_H_
#define ServerDefault_H_

#include "Server.h"
#include "HTTP.h"

class CryptSocketClient;
class CryptSocketServer;
class CryptSocketEvent;
class BroadcastSocket;
class BroadcastSocketEvent;
class ServerDefault;

class ServerDefaultConnection : public ServerConnection
{

	friend class ServerDefault;

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

	friend class ServerDefaultConnection;

public:
	ServerDefault(ServerEventHandler *event_handler);
	virtual ~ServerDefault();

	virtual void Start();
	virtual void Stop();
	virtual bool IsRunning() const;
	virtual long GetListenPort() const;
	virtual bool ProcessClientInputExtra(bool preprocess, bool prenickauthcheck, ServerConnection *conn, const wxString &context, const wxString &cmd, const ByteBuffer &data);
	virtual void ResetPublicListUpdate(int num_secs_till_next_update, bool force_show);
	virtual wxLongLong_t GetNextPublicListUpdateTick() const;

protected:
	virtual StringHashMap GetPublicPostData(bool include_auth);
	virtual void HTTPError(const wxString &errmsg);
	virtual void HTTPSuccess();

protected:
	void OnSocket(CryptSocketEvent &event);
	void OnBroadcast(BroadcastSocketEvent &event);
	void OnTimerPing(wxTimerEvent &event);
	void OnHTTP(HTTPEvent &event);

protected:
	CryptSocketServer *m_sckListen;
	BroadcastSocket *m_bcast;
	wxTimer *m_tmrPing;
	HTTP m_http;
	bool m_public_server;
	wxLongLong_t m_next_list_update;
	bool m_list_updating;
	wxString m_http_data;
	bool m_list_connect_ok;
	wxLongLong_t m_start_tick;
	bool m_last_failed;
	bool m_show_http_result;
	wxString m_last_server_name;

private:
	DECLARE_EVENT_TABLE()

};

#endif
