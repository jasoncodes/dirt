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

class ServerConnection
{

protected:
	ServerConnection();

public:
	virtual ~ServerConnection();

public:
	virtual wxString GetNickname() const { return m_nickname; }
	virtual wxString GetRemoteHost() const { return m_remotehost; }
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

	virtual void Send(const ByteBuffer &data) = 0;

protected:
	virtual void ResetIdleTime() { m_lastactive = ::wxGetUTCTime(); }

protected:
	wxString m_nickname;
	wxString m_remotehost;
	wxString m_userdetails;
	wxString m_awaymessage;
	long m_lastactive;
	time_t m_latency;
	wxString m_useragent;
	const wxDateTime m_jointime;

};

#include <wx/dynarray.h>
WX_DEFINE_ARRAY(ServerConnection*, ServerConnectionArray);

class Server : public wxEvtHandler
{

public:
	Server(ServerEventHandler *event_handler);
	virtual ~Server();

	virtual void ProcessInput(const wxString &input);
	virtual void Start() = 0;
	virtual void Stop() = 0;
	virtual bool IsRunning() = 0;
	virtual int GetListenPort() = 0;
	virtual size_t GetConnectionCount() { return m_connections.GetCount(); }
	virtual const ServerConnection& GetConnection(size_t index) { return *m_connections.Item(index); }

protected:
	virtual void CloseAllConnections();

protected:
	ServerEventHandler *m_event_handler;
	ServerConnectionArray m_connections;

private:
	DECLARE_EVENT_TABLE()

};

#endif
