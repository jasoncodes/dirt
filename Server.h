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


#ifndef Server_H_
#define Server_H_

#include <wx/datetime.h>
#include <wx/sound.h>
#include "ByteBuffer.h"
#include "util.h"
#include "LogWriter.h"

//////// ServerEventHandler ////////

class ServerEventHandler
{

public:
	virtual bool OnServerConsoleInputPreprocess(wxString &cmd, wxString &params, const wxString &nick) = 0;
	virtual wxArrayString OnServerSupportedCommands() = 0;
	virtual void OnServerStateChange() = 0;
	virtual void OnServerConnectionChange() = 0;
	virtual void OnServerInformation(const wxString &line) = 0;
	virtual void OnServerWarning(const wxString &line) = 0;

};

//////// ServerConnection ////////

class Server;

class ServerConnection
{

	friend class Server;

protected:
	ServerConnection();

public:
	virtual ~ServerConnection();

public:
	virtual wxString GetNickname() const { return m_nickname; }
	virtual wxString GetRemoteHost() const { return m_remotehost; }
	virtual wxString GetRemoteHostAndPort() const { return m_remotehostandport; }
	virtual wxString GetRemoteIPString() const { return m_remoteipstring; }
	virtual wxString GetUserDetails() const { return m_userdetails; }
	virtual bool IsAway() const { return m_isaway; }
	virtual long GetAwayTime() const { return m_awaytime; }
	virtual ByteBuffer GetAwayMessage() const { return m_awaymessage; }
	virtual long GetIdleTime() const { return wxGetUTCTime() - m_last_active; }
	virtual wxString GetIdleTimeString() const { return (GetIdleTime() > -1) ? SecondsToMMSS(GetIdleTime()) : wxString(wxT("N/A")); }
	virtual time_t GetLatency() const { return m_latency; }
	virtual wxString GetLatencyString() const { return (GetLatency() > -1) ? AddCommas((wxLongLong_t)GetLatency()) + wxT(" ms") : wxString(wxT("N/A")); }
	virtual wxString GetUserAgent() const { return m_useragent; }
	virtual wxDateTime GetJoinTime() const { return m_jointime; }
	virtual wxString GetJoinTimeString() const { return FormatISODateTime(GetJoinTime()); }
	virtual operator wxString() const;
	virtual wxString GetInlineDetails() const { return GetRemoteHost(); }
	virtual wxString GetId() const;
	virtual bool IsAuthenticated() const { return m_authenticated; }
	virtual bool IsAdmin() const { return m_admin; }
	virtual void Terminate(const ByteBuffer &reason) = 0;

protected:
	virtual void Send(const wxString &context, const wxString &cmd, const ByteBuffer &data);
	virtual void SendData(const ByteBuffer &data) = 0;

protected:
	wxString m_nickname;
	wxString m_remotehost;
	wxString m_remotehostandport;
	wxString m_remoteipstring;
	wxString m_userdetails;
	bool m_isaway;
	long m_awaytime;
	ByteBuffer m_awaymessage;
	long m_last_active;
	time_t m_latency;
	wxString m_useragent;
	const wxDateTime m_jointime;
	bool m_authenticated;
	bool m_admin;
	
private:
	DECLARE_NO_COPY_CLASS(ServerConnection)

};

#include <wx/dynarray.h>
#ifndef WX_DEFINE_ARRAY_PTR
	#define WX_DEFINE_ARRAY_PTR WX_DEFINE_ARRAY
#endif
WX_DEFINE_ARRAY_PTR(ServerConnection*, ServerConnectionArray);

//////// ServerConfig ////////

#include "ConfigFile.h"

class ServerConfig : public Config
{

public:
	ServerConfig();
	virtual ~ServerConfig();

	long GetListenPort() const;
	wxString GetUserPassword(bool decrypt) const;
	wxString GetAdminPassword(bool decrypt) const;
	long GetMaxUsers() const;
	long GetMaxUsersIP() const;
	
	wxString GetSoundConnection() const;
	wxString GetSoundJoin() const;

	wxString GetServerName() const;
	wxString GetHostname() const;
	bool GetPublicListEnabled() const;
	wxString GetPublicListAuthentication(bool decrypt) const;
	wxString GetPublicListComment() const;
	
	bool GetHTTPProxyEnabled() const;
	wxString GetHTTPProxyHostname() const;
	long GetHTTPProxyPort() const;
	wxString GetHTTPProxyUsername() const;
	wxString GetHTTPProxyPassword(bool decrypt) const;

	bool SetListenPort(long port);
	bool SetUserPassword(const wxString &password);
	bool SetAdminPassword(const wxString &password);
	bool SetMaxUsers(long max_users);
	bool SetMaxUsersIP(long max_users_ip);

	bool SetSoundConnection(const wxString &filename);
	bool SetSoundJoin(const wxString &filename);

	bool SetServerName(const wxString &server_name);
	bool SetHostname(const wxString &hostname);
	bool SetPublicListEnabled(bool enabled);
	bool SetPublicListAuthentication(const wxString &auth);
	bool SetPublicListComment(const wxString &comment);

	bool SetHTTPProxyEnabled(bool enabled);
	bool SetHTTPProxyHostname(const wxString &hostname);
	bool SetHTTPProxyPort(long port);
	bool SetHTTPProxyUsername(const wxString &username);
	bool SetHTTPProxyPassword(const wxString &password);

};

//////// Server ////////

class Server : public wxEvtHandler
{

public:
	Server(ServerEventHandler *event_handler);
	virtual ~Server();

	virtual void Information(const wxString &line);
	virtual void Warning(const wxString &line);
	virtual void ProcessConsoleInput(const wxString &input, const wxString &nick = wxEmptyString);
	virtual void Start() = 0;
	virtual void Stop() = 0;
	virtual bool IsRunning() const = 0;
	virtual long GetListenPort() const = 0;
	virtual ServerConfig& GetConfig() { return m_config; }
	virtual size_t GetConnectionCount() const { return m_connections.GetCount(); }
	virtual ServerConnection* GetConnection(size_t index) const { return m_connections.Item(index); }
	virtual ServerConnection* GetConnection(const wxString &nickname) const;
	virtual void SendToAll(const wxString &context, const wxString &cmd, const ByteBuffer &data, bool with_nicks_only);
	virtual ServerConnection* SendToNick(const wxString &nickname, const wxString &context, const wxString &cmd, const ByteBuffer &data);
	virtual ByteBuffer GetNickList() const;
	static wxString GetServerNickname() { return s_server_nickname; }
	virtual void ResetPublicListUpdate(int num_secs_till_next_update, bool force_show) = 0;
	virtual wxLongLong_t GetNextPublicListUpdateTick() const = 0;
	virtual size_t GetUserCount() const;
	virtual size_t GetAwayCount() const;
	virtual time_t GetAverageLatency() const;
	virtual size_t GetConnectionsFromHost(const wxString &hostname) const;
	virtual wxArrayString GetSupportedCommands() const;
	virtual void InitLog();
	static bool IsValidNickname(const wxString &nickname);
	static ByteBuffer MakeNicknameValid(const ByteBuffer &src);
	void LogConsoleInput(const wxString &cmd, const wxString &params, const wxString &nick);
	void LogConsoleInput(const wxString &input, const wxString &nick);
	virtual long GetServerUptime() const = 0;

protected:
	void OnConfigFileChanged(wxCommandEvent &event);

protected:
	virtual void CloseAllConnections();
	virtual void PopulateFilteredWords();
	virtual void ProcessClientInput(ServerConnection *conn, const ByteBuffer &msg);
	virtual void ProcessClientInput(ServerConnection *conn, const wxString &context, const wxString &cmd, const ByteBuffer &data);
	virtual bool ProcessClientInputExtra(bool preprocess, bool prenickauthcheck, ServerConnection *conn, const wxString &context, const wxString &cmd, const ByteBuffer &data) = 0;
	virtual wxString ProcessWordFilters(const wxString &text) const;
	virtual ByteBuffer ProcessWordFilters(const ByteBuffer &data) const;

protected:
	ServerEventHandler *m_event_handler;
	ServerConnectionArray m_connections;
	ServerConfig m_config;
	static const wxString s_server_nickname;
	size_t m_peak_users;
	long m_last_active;
	wxArrayString m_ip_list;
	#if wxUSE_SOUND
		wxSound m_sound;
	#endif
	LogWriter *m_log;
	bool m_log_warning_given;
	wxArrayString m_filtered_words_list;

private:
	DECLARE_EVENT_TABLE()
	DECLARE_NO_COPY_CLASS(Server)

};

#endif
