/*
    Copyright 2002, 2003 General Software Laboratories
    
    
    This file is part of Dirt Secure Chat.

    Dirt Secure Chat is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Dirt Secure Chat is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Dirt Secure Chat; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


#ifndef ServerDefault_H_
#define ServerDefault_H_

#include "Server.h"
#include "HTTP.h"
#include "DNS.h"

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
	ServerDefaultConnection(ServerDefault *server);

public:
	virtual ~ServerDefaultConnection();

protected:
	virtual void SendData(const ByteBuffer &data);
	virtual void Terminate(const ByteBuffer &reason);

protected:
	ServerDefault *m_server;
	CryptSocketClient *m_sck;
	ByteBuffer m_authkey;
	int m_auth_fail_count;
	wxLongLong_t m_nextping;
	wxString m_pingid;
	wxLongLong_t m_lastping;
	wxString m_quitmsg;
	wxLongLong_t m_last_auth_time;
	int m_remoteport;

	DECLARE_NO_COPY_CLASS(ServerDefaultConnection)

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
	virtual long GetServerUptime() const;

protected:
	virtual StringHashMap GetPublicPostData(bool include_auth);
	virtual void HTTPError(const wxString &errmsg);
	virtual void HTTPSuccess();
	void NewConnection(ServerDefaultConnection *conn);

protected:
	void OnSocket(CryptSocketEvent &event);
	void OnBroadcast(BroadcastSocketEvent &event);
	void OnTimerPing(wxTimerEvent &event);
	void OnHTTP(HTTPEvent &event);
	void OnDNS(DNSEvent &event);

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
	long m_start_tick;
	bool m_last_failed;
	bool m_show_http_result;
	wxString m_last_server_name;
	DNS *m_dns;

private:
	DECLARE_EVENT_TABLE()
	DECLARE_NO_COPY_CLASS(ServerDefault)

};

#endif
