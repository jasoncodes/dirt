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
	ServerConnection()
		: m_jointime(wxDateTime::Now())
	{
		ResetIdleTime();
	}

public:
	virtual ~ServerConnection()
	{
	}

public:
	virtual wxString GetNickname() const { return wxEmptyString; } // not implemented
	virtual wxString GetRemoteHost() const { return wxEmptyString; } // not implemented
	virtual wxString GetUserDetails() const { return wxEmptyString; } // not implemented
	virtual wxString GetAwayMessage() const { return wxEmptyString; } // not implemented
	virtual long GetIdleTime() const { return wxGetUTCTime() - m_lastactive; }
	virtual wxString GetIdleTimeString() const { return GetIdleTime() > -1 ? SecondsToMMSS(GetIdleTime()) : "N/A"; }
	virtual time_t GetLatency() const { return -1; } // not implemented
	virtual wxString GetLatencyString() const { return GetLatency() > -1 ? AddCommas((off_t)GetLatency()) + " ms" : "N/A"; }
	virtual wxString GetUserAgent() const { return wxEmptyString; } // not implemented
	virtual wxDateTime GetJoinTime() const { return m_jointime; }
	virtual wxString GetJoinTimeString() const { return FormatISODateTime(GetJoinTime()); }

protected:
	virtual void ResetIdleTime() { m_lastactive = ::wxGetUTCTime(); }

protected:
	const wxDateTime m_jointime;
	long m_lastactive;

};

#include <wx/dynarray.h>
WX_DECLARE_OBJARRAY(ServerConnection, ServerConnectionArray);

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
	virtual const ServerConnection& GetConnection(size_t index) { return m_connections.Item(index); }


protected:
	ServerEventHandler *m_event_handler;
	ServerConnectionArray m_connections;

private:
	DECLARE_EVENT_TABLE()

};

#endif
