#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: Server.cpp,v 1.38 2003-03-19 12:21:30 jason Exp $)

#include "Server.h"
#include "Modifiers.h"
#include "util.h"

//////// ServerConnection ////////

ServerConnection::ServerConnection()
	: m_jointime(wxDateTime::Now())
{
	m_nickname = wxEmptyString;
	m_remotehost = wxEmptyString;
	m_remotehostandport = wxEmptyString;
	m_userdetails = wxEmptyString;
	m_isaway = false;
	m_awaymessage = wxEmptyString;
	m_latency = -1;
	m_useragent = wxEmptyString;
	m_authenticated = false;
	m_admin = false;
	ResetIdleTime();
}

ServerConnection::~ServerConnection()
{
}

ServerConnection::operator wxString() const
{
	wxString retval;
	retval << (GetNickname().Length()?GetNickname():wxT("N/A"));
	retval << wxT("@") << (GetRemoteHostAndPort().Length()?GetRemoteHostAndPort():wxT("N/A"));
	retval << wxT(" (") << (GetUserDetails().Length()?GetUserDetails():wxT("N/A"));
	retval << wxT(") (");
	if (IsAdmin())
	{
		retval << wxT("Admin; ");
	}
	if (IsAway())
	{
		retval << wxT("Away: ") << GetAwayMessage() << (wxChar)OriginalModifier << wxT("; ");
	}
	retval << wxT("Idle: ") << GetIdleTimeString() << wxT("; ");
	retval << wxT("Lag: ") << GetLatencyString() << wxT("; ");
	if (GetUserAgent().Length())
	{
		retval << wxT("Agent: ") << GetUserAgent() << wxT("; ");
	}
	retval << wxT("Joined: ") << GetJoinTimeString();
	retval << wxT(")");
	return retval;
}

wxString ServerConnection::GetId() const
{
	return wxString()
		<< ((GetNickname().Length()) ? GetNickname() : wxT("*"))
		<< wxT("@") + GetRemoteHostAndPort();
}

void ServerConnection::Send(const wxString &context, const wxString &cmd, const ByteBuffer &data)
{
	SendData(EncodeMessage(context, cmd, data));
}

//////// ServerConfig ////////

#include "Crypt.h"
#include <wx/filename.h>

ServerConfig::ServerConfig()
{
	m_config = new wxFileConfig(wxT("dirt"));
	m_config->SetUmask(0077);
}

ServerConfig::~ServerConfig()
{
	delete m_config;
}

bool ServerConfig::Flush()
{
	return m_config->Flush();
}

bool ServerConfig::ResetToDefaults()
{
	return m_config->DeleteGroup(wxT("Server"));
}

long ServerConfig::GetListenPort() const
{
	return m_config->Read(wxT("Server/Listen Port"), 11626);
}

wxString ServerConfig::GetUserPassword(bool decrypt) const
{
	return GetPassword(wxT("Server/User Password"), decrypt);
}

wxString ServerConfig::GetAdminPassword(bool decrypt) const
{
	return GetPassword(wxT("Server/Admin Password"), decrypt);
}

long ServerConfig::GetMaxUsers() const
{
	return m_config->Read(wxT("Server/Max Users"), 32);
}

long ServerConfig::GetMaxUsersIP() const
{
	return m_config->Read(wxT("Server/Max Users Per IP"), 4);
}

wxString ServerConfig::GetSoundConnection() const
{
	wxString default_sound = wxEmptyString;
	#ifdef __WXMSW__
		default_sound = wxGetOSDirectory() + wxT("\\Media\\ringout.wav");
		if (!wxFileName(default_sound).FileExists())
		{
			default_sound = wxEmptyString;
		}
	#endif
	return m_config->Read(wxT("Server/Sound/Connection"), default_sound);
}

wxString ServerConfig::GetSoundJoin() const
{
	wxString default_sound = wxEmptyString;
	#ifdef __WXMSW__
		default_sound = wxGetOSDirectory() + wxT("\\Media\\ringin.wav");
		if (!wxFileName(default_sound).FileExists())
		{
			default_sound = wxEmptyString;
		}
	#endif
	return m_config->Read(wxT("Server/Sound/Join"), default_sound);
}

wxString ServerConfig::GetServerName() const
{
	return m_config->Read(wxT("Server/Server Name"), wxEmptyString);
}

wxString ServerConfig::GetHostname() const
{
	return m_config->Read(wxT("Server/Hostname"), wxEmptyString);
}

bool ServerConfig::GetPublicListEnabled() const
{
	bool value;
	bool success = m_config->Read(wxT("Server/Public List/Enabled"), &value, false);
	return success?value:false;
}

wxString ServerConfig::GetPublicListAuthentication(bool decrypt) const
{
	return GetPassword(wxT("Server/Public List/Authentication"), decrypt);
}

wxString ServerConfig::GetPublicListComment() const
{
	return m_config->Read(wxT("Server/Public List/Comment"), wxEmptyString);
}

bool ServerConfig::GetHTTPProxyEnabled() const
{
	bool value;
	bool success = m_config->Read(wxT("Server/HTTP Proxy/Enabled"), &value, false);
	return success?value:false;
}

wxString ServerConfig::GetHTTPProxyHostname() const
{
	return m_config->Read(wxT("Server/HTTP Proxy/Hostname"), wxEmptyString);
}

long ServerConfig::GetHTTPProxyPort() const
{
	return m_config->Read(wxT("Server/HTTP Proxy/Port"), 80);
}

wxString ServerConfig::GetHTTPProxyUsername() const
{
	return m_config->Read(wxT("Server/HTTP Proxy/Username"), wxEmptyString);
}

wxString ServerConfig::GetHTTPProxyPassword(bool decrypt) const
{
	return GetPassword(wxT("Server/HTTP Proxy/Password"), decrypt);
}

bool ServerConfig::SetListenPort(long port)
{
	return m_config->Write(wxT("Server/Listen Port"), port);
}

bool ServerConfig::SetUserPassword(const wxString &password)
{
	return SetPassword(wxT("Server/User Password"), password);
}

bool ServerConfig::SetAdminPassword(const wxString &password)
{
	return SetPassword(wxT("Server/Admin Password"), password);
}

bool ServerConfig::SetMaxUsers(long max_users)
{
	return m_config->Write(wxT("Server/Max Users"), max_users);
}

bool ServerConfig::SetMaxUsersIP(long max_users_ip)
{
	return m_config->Write(wxT("Server/Max Users Per IP"), max_users_ip);
}

bool ServerConfig::SetSoundConnection(const wxString &filename)
{
	return
		(filename.Length() == 0 || wxFileName(filename).FileExists()) &&
		m_config->Write(wxT("Server/Sound/Connection"), filename);
}

bool ServerConfig::SetSoundJoin(const wxString &filename)
{
	return
		(filename.Length() == 0 || wxFileName(filename).FileExists()) &&
		m_config->Write(wxT("Server/Sound/Join"), filename);
}

bool ServerConfig::SetServerName(const wxString &server_name)
{
	return m_config->Write(wxT("Server/Server Name"), server_name);
}

bool ServerConfig::SetHostname(const wxString &hostname)
{
	return m_config->Write(wxT("Server/Hostname"), hostname);
}

bool ServerConfig::SetPublicListEnabled(bool enabled)
{
	return m_config->Write(wxT("Server/Public List/Enabled"), enabled);
}

bool ServerConfig::SetPublicListAuthentication(const wxString &auth)
{
	return SetPassword(wxT("Server/Public List/Authentication"), auth);
}

bool ServerConfig::SetPublicListComment(const wxString &comment)
{
	return m_config->Write(wxT("Server/Public List/Comment"), comment);
}

bool ServerConfig::SetHTTPProxyEnabled(bool enabled)
{
	return m_config->Write(wxT("Server/HTTP Proxy/Enabled"), enabled);
}

bool ServerConfig::SetHTTPProxyHostname(const wxString &hostname)
{
	return m_config->Write(wxT("Server/HTTP Proxy/Hostname"), hostname);
}

bool ServerConfig::SetHTTPProxyPort(long port)
{
	return m_config->Write(wxT("Server/HTTP Proxy/Port"), port);
}

bool ServerConfig::SetHTTPProxyUsername(const wxString &username)
{
	return m_config->Write(wxT("Server/HTTP Proxy/Username"), username);
}

bool ServerConfig::SetHTTPProxyPassword(const wxString &password)
{
	return SetPassword(wxT("Server/HTTP Proxy/Password"), password);
}


static const wxString EncodedPrefix = wxT("Encoded:");

static wxString DecodePassword(const wxString &value, bool decrypt)
{

	if (!decrypt || value.Length() == 0 || !LeftEq(value, EncodedPrefix))
	{
		return value;
	}

	ByteBuffer data = Crypt::Base64Decode(value.Mid(EncodedPrefix.Length()));
	
	if (data.Length() < 40)
	{
		return wxEmptyString;
	}
	
	if (((data.Length() - 40) % 16) != 0)
	{
		return wxEmptyString;
	}

	const byte *ptr = data.LockRead();
	ByteBuffer crc32(ptr, 4);
	ByteBuffer len_buff(ptr+4, 4);
	size_t len = BytesToUint32(len_buff.LockRead(), len_buff.Length());
	len_buff.Unlock();
	ByteBuffer AESKey(ptr+8, 32);
	ByteBuffer enc(ptr+40, data.Length()-40);
	data.Unlock();	
	
	try
	{
		Crypt crypt;
		crypt.SetAESDecryptKey(AESKey);
		data = crypt.AESDecrypt(enc);
	}
	catch (...)
	{
		return wxEmptyString;
	}
	
	ByteBuffer dec(data.LockRead(), len);
	data.Unlock();
	
	if (crc32 == Crypt::CRC32(AESKey + dec))
	{
		return dec;
	}

	return wxEmptyString;

}

wxString ServerConfig::GetPassword(const wxString &key, bool decrypt) const
{
	return DecodePassword(m_config->Read(key), decrypt);
}

bool ServerConfig::SetPassword(const wxString &key, const wxString &password)
{
	ByteBuffer data;
	if (password.Length() > 0)
	{
		if (LeftEq(password, EncodedPrefix))
		{
			return (DecodePassword(password, true).Length() > 0);
		}
		else
		{
			ByteBuffer AESKey = Crypt::Random(32);
			Crypt crypt;
			crypt.SetAESEncryptKey(AESKey);
			data =
				Crypt::CRC32(AESKey + ByteBuffer(password)) + 
				Uint32ToBytes(password.Length()) +
				AESKey +
				crypt.AESEncrypt(password);
			data = EncodedPrefix + Crypt::Base64Encode(data, false);
		}
	}
	return m_config->Write(key, data);
}

//////// Server ////////

const wxString Server::s_server_nickname = wxT("Console@Server");

BEGIN_EVENT_TABLE(Server, wxEvtHandler)
END_EVENT_TABLE()

Server::Server(ServerEventHandler *event_handler)
	: wxEvtHandler(), m_event_handler(event_handler),
	m_log(LogWriter::GenerateFilename(wxT("Server"), LogWriter::GenerateNewLogDate(wxT("Server")))), m_log_warning_given(false)
{
	m_connections.Alloc(10);
	m_config = new ServerConfig;
	m_peak_users = 0;
}

Server::~Server()
{
	CloseAllConnections();
	delete m_config;
}

void Server::Information(const wxString &line)
{
	m_event_handler->OnServerInformation(line);
	for (size_t i = 0; i < GetConnectionCount(); ++i)
	{
		ServerConnection *conn = m_connections.Item(i);
		if (conn->IsAdmin())
		{
			conn->Send(wxEmptyString, wxT("PRIVMSG"), Pack(GetServerNickname(), line));
		}
	}
	if (m_log.Ok())
	{
		m_log.AddText(line, *wxBLACK, false);
	}
	else if (!m_log_warning_given)
	{
		m_log_warning_given = true;
		Warning("Error writing log file");
	}
}

void Server::Warning(const wxString &line)
{
	m_event_handler->OnServerWarning(line);
	for (size_t i = 0; i < GetConnectionCount(); ++i)
	{
		ServerConnection *conn = m_connections.Item(i);
		if (conn->IsAdmin())
		{
			conn->Send(wxEmptyString, wxT("PRIVMSG"), Pack(GetServerNickname(), line));
		}
	}
	if (m_log.Ok())
	{
		m_log.AddText(line, *wxRED, false);
	}
	else if (!m_log_warning_given)
	{
		m_log_warning_given = true;
		Warning("Error writing log file");
	}
}

wxArrayString Server::GetSupportedCommands() const
{
	wxArrayString cmds;
	WX_APPEND_ARRAY(cmds, SplitString(wxT("HELP KICK START STOP USERS"), wxT(" ")));
	WX_APPEND_ARRAY(cmds, m_event_handler->OnServerSupportedCommands());
	cmds.Sort();
	return cmds;
}

void Server::ProcessConsoleInput(const wxString &input)
{

	wxString cmd, params;
	
	SplitQuotedHeadTail(input, cmd, params);
	if (input[0] == wxT('/'))
	{
		cmd = cmd.Mid(1);
	}
	cmd.MakeUpper();
	cmd.Trim(true);
	cmd.Trim(false);
	params.Trim(true);
	params.Trim(false);

	if (m_event_handler->OnServerPreprocess(cmd, params))
	{
		return;
	}

	if (cmd == wxT("START"))
	{
		if (IsRunning())
		{
			Warning(wxT("Server is already running"));
		}
		else
		{
			Start();
		}
	}
	else if (cmd == wxT("STOP"))
	{
		if (IsRunning())
		{
			Stop();
		}
		else
		{
			Warning(wxT("Server is not running"));
		}
	}
	else if (cmd == wxT("USERS"))
	{
		if (IsRunning())
		{
			Information(wxString() << wxT("There are currently ") << GetConnectionCount() << wxT(" connections"));
			for (size_t i = 0; i < GetConnectionCount(); ++i)
			{
				Information(wxT("    ") + *GetConnection(i));
			}
		}
		else
		{
			Warning(wxT("Server is not running"));
		}
	}
	else if (cmd == wxT("KICK"))
	{
		HeadTail ht = SplitQuotedHeadTail(params);
		if (!ht.head.Length())
		{
			Warning(wxT("No nickname specified"));
			return;
		}
		ServerConnection *conn = GetConnection(ht.head);
		if (conn)
		{
			if (ht.tail.Length())
			{
				ht.tail = wxT("Kicked: ") + ht.tail;
			}
			else
			{
				ht.tail = wxT("Kicked");
			}
			conn->Terminate(ht.tail);
		}
		else
		{
			Warning(wxT("No such nickname: ") + ht.head);
		}
	}

	else if (cmd == wxT("HELP"))
	{
		Information(wxT("Supported commands: ") + JoinArray(GetSupportedCommands(), wxT(" ")));
	}
	else
	{
		Warning(wxT("Unrecognized command: ") + cmd);
	}

}

void Server::CloseAllConnections()
{
	for (size_t i = 0; i < m_connections.GetCount(); ++i)
	{
		delete m_connections.Item(i);
	}
	m_connections.Empty();
	m_event_handler->OnServerConnectionChange();
}

void Server::ProcessClientInput(ServerConnection *conn, const ByteBuffer &msg)
{
	
	wxString context, cmd;
	ByteBuffer data;
	bool success = DecodeMessage(msg, context, cmd, data);
	if (success)
	{
		cmd.MakeUpper();
		ProcessClientInput(conn, context, cmd, data);
	}
	else
	{
		conn->Send(wxEmptyString, wxT("ERROR"), Pack(wxString(wxT("PROTOCOL")), wxString(wxT("Unable to decode message. Discarded"))));
	}

}

size_t Server::GetUserCount() const
{
	size_t count = 0;
	for (size_t i = 0; i < GetConnectionCount(); ++i)
	{
		if (GetConnection(i)->GetNickname().Length())
		{
			count++;
		}
	}
	return count;
}

size_t Server::GetAwayCount() const
{
	size_t count = 0;
	for (size_t i = 0; i < GetConnectionCount(); ++i)
	{
		if (GetConnection(i)->IsAway())
		{
			count++;
		}
	}
	return count;
}

long Server::GetLowestIdleTime() const
{
	long lowest_idle = 0;
	for (size_t i = 0; i < GetConnectionCount(); ++i)
	{
		long idle_time = GetConnection(i)->GetIdleTime();
		if (i == 0 || lowest_idle > idle_time)
		{
			lowest_idle = idle_time;
		}
	}
	return lowest_idle;
}

time_t Server::GetAverageLatency() const
{

	size_t count = 0;
	time_t sum = 0;

	for (size_t i = 0; i < GetConnectionCount(); ++i)
	{
		ServerConnection *conn = GetConnection(i);
		if (conn->GetNickname().Length())
		{
			if (conn->GetLatency() > 0)
			{
				if (!LeftEq(conn->GetRemoteIPString(), wxT("127.")) && (m_ip_list.Index(conn->GetRemoteIPString()) == -1))
				{
					count++;
					sum += conn->GetLatency();
				}
			}
		}
	}

	return (count)?(sum / count):0;

}

size_t Server::GetConnectionsFromHost(const wxString &hostname) const
{
	size_t count = 0;
	for (size_t i = 0; i < GetConnectionCount(); ++i)
	{
		if (GetConnection(i)->GetRemoteHost() == hostname)
		{
			count++;
		}
	}
	return count;
}

ServerConnection* Server::GetConnection(const wxString &nickname) const
{
	for (size_t i = 0; i < GetConnectionCount(); ++i)
	{
		if (m_connections.Item(i)->GetNickname().CmpNoCase(nickname) == 0)
		{
			return m_connections.Item(i);
		}
	}
	return NULL;
}

void Server::SendToAll(const wxString &context, const wxString &cmd, const ByteBuffer &data, bool with_nicks_only)
{
	ByteBuffer msg = EncodeMessage(context, cmd, data);
	for (size_t i = 0; i < GetConnectionCount(); ++i)
	{
		ServerConnection *conn = GetConnection(i);
		if (!with_nicks_only || conn->GetNickname().Length())
		{
			conn->SendData(msg);
		}
	}
}

ServerConnection* Server::SendToNick(const wxString &nickname, const wxString &context, const wxString &cmd, const ByteBuffer &data)
{
	ServerConnection *conn = GetConnection(nickname);
	if (conn)
	{
		ByteBuffer msg = EncodeMessage(context, cmd, data);
		conn->SendData(msg);
		return conn;
	}
	else
	{
		return NULL;
	}
}

bool Server::IsValidNickname(const wxString &nickname)
{
    if (nickname.Length() < 1 || nickname.Length() > 16 || nickname == GetServerNickname())
	{
		return false;
	}
	for (size_t i = 0; i < nickname.Length(); ++i)
	{
		wxChar c = nickname[i];
		if (!wxIsalnum(c))
		{
			switch (c)
			{
				case wxT('_'):
				case wxT('^'):
				case wxT('|'):
				case wxT('\\'):
				case wxT('-'):
				case wxT('['):
				case wxT(']'):
				case wxT('{'):
				case wxT('}'):
					break;
				default:
					return false;
			}
		}
	}
	return true;
}

ByteBuffer Server::GetNickList() const
{
	ByteBufferArray nicks;
	for (size_t i = 0; i < GetConnectionCount(); ++i)
	{
		ServerConnection *conn = GetConnection(i);
		if (conn->GetNickname().Length())
		{
			nicks.Add(conn->GetNickname());
		}
	}
	return Pack(nicks);
}

void Server::ProcessClientInput(ServerConnection *conn, const wxString &context, const wxString &cmd, const ByteBuffer &data)
{

	if (cmd != wxT("PING") && cmd != wxT("PONG"))
	{
		conn->ResetIdleTime();
	}

	if (ProcessClientInputExtra(true, true, conn, context, cmd, data))
	{
		return;
	}

	if (cmd == wxT("PING"))
	{
		conn->Send(context, wxT("PONG"), data);
		return;
	}

	if (!conn->IsAuthenticated() && cmd != wxT("AUTH") && cmd != wxT("USERDETAILS") && cmd != wxT("USERAGENT"))
	{
		conn->Send(context, wxT("ERROR"), Pack(wxString(wxT("NOAUTH")), wxString(wxT("You need to authenticate first"))));
		return;
	}

	if (conn->GetNickname().Length() == 0 && cmd != wxT("NICK") && cmd != wxT("USERDETAILS") && cmd != wxT("USERAGENT"))
	{
		conn->Send(context, wxT("ERROR"), Pack(wxString(wxT("NONICK")), wxString(wxT("You need to set a nickname first"))));
		return;
	}

	if (ProcessClientInputExtra(true, false, conn, context, cmd, data))
	{
		return;
	}

	if (cmd == wxT("PUBMSG") || cmd == wxT("PUBACTION"))
	{
		SendToAll(wxEmptyString, cmd, Pack(conn->GetNickname(), data), true);
	}
	else if (cmd == wxT("PRIVMSG") || cmd == wxT("PRIVACTION"))
	{
		ByteBuffer nick;
		ByteBuffer msg;
		if (Unpack(data, nick, msg))
		{
			if (GetServerNickname().CmpNoCase((wxString)nick) == 0)
			{
				if (conn->IsAdmin())
				{
					conn->Send(context, cmd + wxT("OK"), Pack(GetServerNickname(), msg));
					Information(conn->GetId() + wxT(" issued command: ") + msg);
					ProcessConsoleInput(msg);
				}
				else
				{
					conn->Send(context, wxT("ERROR"), Pack(wxString(wxT("NONICK")), wxString(wxT("You are not a server administrator"))));
				}
			}
			else
			{
				ServerConnection *dest = GetConnection(nick);
				if (dest)
				{
					conn->Send(context, cmd + wxT("OK"), Pack(dest->GetNickname(), msg));
					dest->Send(wxEmptyString, cmd, Pack(conn->GetNickname(), msg));
				}
				else
				{
					conn->Send(context, wxT("ERROR"), Pack(wxString(wxT("NONICK")), wxT("No suck nick: ") + nick));
				}
			}
		}
		else
		{
			conn->Send(context, wxT("ERROR"), Pack(wxString(wxT("MALFORMED")), wxT("Malformed data in ") + cmd));
		}
	}
	else if (cmd == wxT("USERDETAILS"))
	{
		if (conn->m_userdetails != (wxString)data)
		{
			conn->m_userdetails = (wxString)data;
			Information(conn->GetId() + wxT(" is ") + conn->GetUserDetails());
		}
	}
	else if (cmd == wxT("USERAGENT"))
	{
		if (conn->m_useragent != (wxString)data)
		{
			conn->m_useragent = (wxString)data;
			Information(conn->GetId() + wxT(" is running ") + conn->GetUserAgent());
		}
	}
	else if (cmd == wxT("QUIT"))
	{
		wxString msg;
		if (data.Length())
		{
			msg = wxT("Quit: ") + data;
		}
		else
		{
			msg = wxT("Quit");
		}
		conn->Terminate(msg);
	}
	else if (cmd == wxT("AWAY"))
	{
		if (!conn->m_isaway || conn->m_awaymessage != (wxString)data)
		{
			conn->m_isaway = true;
			conn->m_awaymessage = (wxString)data;
			SendToAll(wxEmptyString, cmd, Pack(conn->GetNickname(), data), true);
		}
	}
	else if (cmd == wxT("BACK"))
	{
		if (conn->m_awaymessage.Length() > 0)
		{
			SendToAll(wxEmptyString, cmd, Pack(conn->GetNickname(), conn->m_awaymessage), true);
			conn->m_isaway = false;
			conn->m_awaymessage = wxEmptyString;
		}
	}
	else if (cmd == wxT("WHOIS"))
	{
		const ServerConnection *user = GetConnection(data);
		if (user)
		{
			StringHashMap map;
			map[wxT("NICK")] = user->GetNickname();
			map[wxT("HOSTNAME")] = user->GetRemoteHost();
			map[wxT("DETAILS")] = user->GetUserDetails();
			if (user->IsAdmin())
			{
				map[wxT("ISADMIN")] = wxT("");
			}
			if (user->IsAway())
			{
				map[wxT("AWAY")] = user->GetAwayMessage();
			}
			map[wxT("IDLE")] = wxString() << user->GetIdleTime();
			map[wxT("IDLESTRING")] = user->GetIdleTimeString();
			map[wxT("LATENCY")] = wxString() << user->GetLatency();
			map[wxT("LATENCYSTRING")] = user->GetLatencyString();
			map[wxT("AGENT")] = user->GetUserAgent();
			map[wxT("JOINTIME")] = wxString() << user->GetJoinTime().ToGMT().GetTicks();
			map[wxT("JOINTIMESTRING")] = user->GetJoinTimeString();
			conn->Send(context, cmd, PackHashMap(map));
		}
		else
		{
			conn->Send(context, wxT("ERROR"), Pack(wxString(wxT("NONICK")), wxT("No suck nick: ") + data));
		}
	}
	else if (cmd == wxT("NICK"))
	{
		ServerConnection *tmp = GetConnection(data);
		if (tmp && tmp != conn)
		{
			conn->Send(context, wxT("ERROR"), Pack(wxString(wxT("NICK")), wxT("Nickname in use: ") + tmp->GetNickname()));
		}
		else
		{
			wxString new_nick = data;
			if (!IsValidNickname(new_nick))
			{
				conn->Send(context, wxT("ERROR"), Pack(wxString(wxT("NICK")), wxT("Invalid nickname: ") + new_nick));
			}
			else if (conn->m_nickname != new_nick)
			{
				if (conn->m_nickname.Length() == 0)
				{
					ByteBuffer nicklist = GetNickList();
					conn->Send(context, wxT("NICK"), data);
					conn->m_nickname = new_nick;
					Information(conn->GetId() + wxT(" has entered the chat"));
					SendToAll(wxEmptyString, wxT("JOIN"), Pack(data, conn->GetInlineDetails()), true);
					conn->Send(context, wxT("NICKLIST"), nicklist);
					size_t m_user_count = GetUserCount();
					m_peak_users = wxMax(m_peak_users, m_user_count);
					#if wxUSE_WAVE
						wxString filename = m_config->GetSoundJoin();
						if (filename.Length() && wxFileName(filename).FileExists())
						{
							m_wave.Create(filename, false);
							if (!m_wave.IsOk() || !m_wave.Play())
							{
								Warning(wxT("Error playing ") + filename);
							}
						}
					#endif
					if (m_user_count < 3 && GetMillisecondTicks() < GetNextPublicListUpdateTick())
					{
						ResetPublicListUpdate(10, false);
					}
					for (size_t i = 0; i < GetConnectionCount(); ++i)
					{
						ServerConnection *conn2 = GetConnection(i);
						if (conn2->IsAway())
						{
							conn->Send(wxEmptyString, wxT("AWAY"), Pack(conn2->GetNickname(), conn2->GetAwayMessage()));
						}
					}
				}
				else
				{
					SendToAll(wxEmptyString, wxT("NICK"), Pack(conn->m_nickname, data), true);
					Information(conn->GetId() + wxT(" is now known as ") + new_nick);
					conn->m_nickname = new_nick;
				}
			}
		}
	}
	else if (!ProcessClientInputExtra(false, false, conn, context, cmd, data))
	{
		Information(wxT("Unknown message recv'd:"));
		Information(wxT("Context: \"") + context + wxT("\""));
		Information(wxT("Command: \"") + cmd + wxT("\""));
		Information(wxT("Data: ") + data.GetHexDump());
		conn->Send(context, wxT("ERROR"), Pack(wxString(wxT("NOCMD")), wxT("Unrecognized command: ") + cmd));
	}

}
