#ifndef Server_H_
#define Server_H_

#include <wx/datetime.h>
#include "util.h"

class ServerEventHandler
{

public:
	virtual bool OnServerPreprocess(wxString &cmd, wxString &params) { return false; }
	virtual void OnServerStateChange() = 0;
	virtual void OnServerConnectionChange() = 0;
	virtual void OnServerInformation(const wxString &line) = 0;
	virtual void OnServerWarning(const wxString &line) = 0;

};

class Server;

class ServerConnection
{

	friend Server;

protected:
	ServerConnection();

public:
	virtual ~ServerConnection();

public:
	virtual wxString GetNickname() const { return m_nickname; }
	virtual wxString GetRemoteHost() const { return m_remotehost; }
	virtual wxString GetRemoteHostAndPort() const { return m_remotehostandport; }
	virtual wxString GetUserDetails() const { return m_userdetails; }
	virtual wxString GetAwayMessage() const { return m_awaymessage; }
	virtual long GetIdleTime() const { return wxGetUTCTime() - m_lastactive; }
	virtual wxString GetIdleTimeString() const { return GetIdleTime() > -1 ? SecondsToMMSS(GetIdleTime()) : wxT("N/A"); }
	virtual time_t GetLatency() const { return m_latency; }
	virtual wxString GetLatencyString() const { return GetLatency() > -1 ? AddCommas((off_t)GetLatency()) + wxT(" ms") : wxT("N/A"); }
	virtual wxString GetUserAgent() const { return m_useragent; }
	virtual wxDateTime GetJoinTime() const { return m_jointime; }
	virtual wxString GetJoinTimeString() const { return FormatISODateTime(GetJoinTime()); }
	virtual operator wxString() const;
	virtual wxString GetInlineDetails() const { return GetRemoteHost(); }
	virtual wxString GetId() const;
	virtual bool IsAuthenticated() const { return m_authenticated; }

protected:
	virtual void Send(const wxString &context, const wxString &cmd, const ByteBuffer &data);
	virtual void SendData(const ByteBuffer &data) = 0;
	virtual void ResetIdleTime() { m_lastactive = ::wxGetUTCTime(); }

protected:
	wxString m_nickname;
	wxString m_remotehost;
	wxString m_remotehostandport;
	wxString m_userdetails;
	wxString m_awaymessage;
	long m_lastactive;
	time_t m_latency;
	wxString m_useragent;
	const wxDateTime m_jointime;
	bool m_authenticated;

};

#include <wx/dynarray.h>
WX_DEFINE_ARRAY(ServerConnection*, ServerConnectionArray);

class Server : public wxEvtHandler
{

public:
	Server(ServerEventHandler *event_handler);
	virtual ~Server();

	virtual void ProcessConsoleInput(const wxString &input);
	virtual void Start() = 0;
	virtual void Stop() = 0;
	virtual bool IsRunning() = 0;
	virtual int GetListenPort() = 0;
	virtual size_t GetConnectionCount() { return m_connections.GetCount(); }
	virtual ServerConnection* GetConnection(size_t index) { return m_connections.Item(index); }
	virtual ServerConnection* GetConnection(const wxString &nickname);
	virtual void SendToAll(const wxString &context, const wxString &cmd, const ByteBuffer &data, bool with_nicks_only);
	virtual ServerConnection* SendToNick(const wxString &nickname, const wxString &context, const wxString &cmd, const ByteBuffer &data);
	virtual ByteBuffer GetNickList();

protected:
	virtual bool IsValidNickname(const wxString &nickname);
	virtual void CloseAllConnections();
	virtual void ProcessClientInput(ServerConnection *conn, const ByteBuffer &msg);
	virtual void ProcessClientInput(ServerConnection *conn, const wxString &context, const wxString &cmd, const ByteBuffer &data);
	virtual bool ProcessClientInputExtra(bool preprocess, bool prenickauthcheck, ServerConnection *conn, const wxString &context, const wxString &cmd, const ByteBuffer &data) = 0;

protected:
	ServerEventHandler *m_event_handler;
	ServerConnectionArray m_connections;

private:
	DECLARE_EVENT_TABLE()

};

#endif
