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


#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: Server.cpp,v 1.77 2004-07-18 18:28:57 jason Exp $)

#include "Server.h"
#include "Modifiers.h"
#include "util.h"
#include "TextTools.h"
#include "IPInfo.h"
#include "Modifiers.h"

//////// ServerConnection ////////

ServerConnection::ServerConnection()
	: m_jointime(wxDateTime::Now())
{
	m_nickname = wxEmptyString;
	m_remotehost = wxEmptyString;
	m_remotehostandport = wxEmptyString;
	m_userdetails = wxEmptyString;
	m_isaway = false;
	m_awaymessage = ByteBuffer();
	m_latency = -1;
	m_useragent = wxEmptyString;
	m_authenticated = false;
	m_admin = false;
	m_last_active = ::wxGetUTCTime();
}

ServerConnection::~ServerConnection()
{
}

ServerConnection::operator wxString() const
{
	wxString retval;
	retval << (GetNickname().Length()?GetNickname():wxString(wxT("N/A")));
	retval << wxT("@") << (GetRemoteHostAndPort().Length()?GetRemoteHostAndPort():wxString(wxT("N/A")));
	retval << wxT(" (") << (GetUserDetails().Length()?GetUserDetails():wxString(wxT("N/A")));
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
		<< ((GetNickname().Length()) ? GetNickname() : wxString(wxT("*")))
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
	: Config(wxT("/Server"))
{
}

ServerConfig::~ServerConfig()
{
}

long ServerConfig::GetListenPort() const
{
	return m_config->Read(wxT("/Server/Listen Port"), 11626);
}

wxString ServerConfig::GetUserPassword(bool decrypt) const
{
	return GetPassword(wxT("/Server/User Password"), decrypt);
}

wxString ServerConfig::GetAdminPassword(bool decrypt) const
{
	return GetPassword(wxT("/Server/Admin Password"), decrypt);
}

long ServerConfig::GetMaxUsers() const
{
	return m_config->Read(wxT("/Server/Max Users"), 32);
}

long ServerConfig::GetMaxUsersIP() const
{
	return m_config->Read(wxT("/Server/Max Users Per IP"), 4);
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
	return m_config->Read(wxT("/Server/Sound/Connection"), default_sound);
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
	return m_config->Read(wxT("/Server/Sound/Join"), default_sound);
}

bool ServerConfig::GetLogPublicMessages() const
{
	bool value;
	bool success = m_config->Read(wxT("/Server/Log Public Messages"), &value, false);
	return success?value:false;
}

wxString ServerConfig::GetServerName() const
{
	wxString name = m_config->Read(wxT("/Server/Server Name"));
	if (!name.Length())
	{
		name = wxGetHostName();
	}
	return name;
}

wxString ServerConfig::GetHostname() const
{
	return m_config->Read(wxT("/Server/Hostname"), wxEmptyString);
}

bool ServerConfig::GetPublicListEnabled() const
{
	bool value;
	bool success = m_config->Read(wxT("/Server/Public List/Enabled"), &value, false);
	return success?value:false;
}

wxString ServerConfig::GetPublicListAuthentication(bool decrypt) const
{
	return GetPassword(wxT("/Server/Public List/Authentication"), decrypt);
}

wxString ServerConfig::GetPublicListComment() const
{
	return m_config->Read(wxT("/Server/Public List/Comment"), wxEmptyString);
}

bool ServerConfig::GetHTTPProxyEnabled() const
{
	bool value;
	bool success = m_config->Read(wxT("/Server/HTTP Proxy/Enabled"), &value, false);
	return success?value:false;
}

wxString ServerConfig::GetHTTPProxyHostname() const
{
	return m_config->Read(wxT("/Server/HTTP Proxy/Hostname"), wxEmptyString);
}

long ServerConfig::GetHTTPProxyPort() const
{
	return m_config->Read(wxT("/Server/HTTP Proxy/Port"), 80);
}

wxString ServerConfig::GetHTTPProxyUsername() const
{
	return m_config->Read(wxT("/Server/HTTP Proxy/Username"), wxEmptyString);
}

wxString ServerConfig::GetHTTPProxyPassword(bool decrypt) const
{
	return GetPassword(wxT("/Server/HTTP Proxy/Password"), decrypt);
}

bool ServerConfig::SetListenPort(long port)
{
	return m_config->Write(wxT("/Server/Listen Port"), port);
}

bool ServerConfig::SetUserPassword(const wxString &password)
{
	return SetPassword(wxT("/Server/User Password"), password);
}

bool ServerConfig::SetAdminPassword(const wxString &password)
{
	return SetPassword(wxT("/Server/Admin Password"), password);
}

bool ServerConfig::SetMaxUsers(long max_users)
{
	return m_config->Write(wxT("/Server/Max Users"), max_users);
}

bool ServerConfig::SetMaxUsersIP(long max_users_ip)
{
	return m_config->Write(wxT("/Server/Max Users Per IP"), max_users_ip);
}

bool ServerConfig::SetSoundConnection(const wxString &filename)
{
	return
		(filename.Length() == 0 || wxFileName(filename).FileExists()) &&
		m_config->Write(wxT("/Server/Sound/Connection"), filename);
}

bool ServerConfig::SetSoundJoin(const wxString &filename)
{
	return
		(filename.Length() == 0 || wxFileName(filename).FileExists()) &&
		m_config->Write(wxT("/Server/Sound/Join"), filename);
}

bool ServerConfig::SetLogPublicMessages(bool log_public_messages)
{
	return m_config->Write(wxT("/Server/Log Public Messages"), log_public_messages);
}

bool ServerConfig::SetServerName(const wxString &server_name)
{
	return m_config->Write(wxT("/Server/Server Name"), server_name);
}

bool ServerConfig::SetHostname(const wxString &hostname)
{
	return m_config->Write(wxT("/Server/Hostname"), hostname);
}

bool ServerConfig::SetPublicListEnabled(bool enabled)
{
	return m_config->Write(wxT("/Server/Public List/Enabled"), enabled);
}

bool ServerConfig::SetPublicListAuthentication(const wxString &auth)
{
	return SetPassword(wxT("/Server/Public List/Authentication"), auth);
}

bool ServerConfig::SetPublicListComment(const wxString &comment)
{
	return m_config->Write(wxT("/Server/Public List/Comment"), comment);
}

bool ServerConfig::SetHTTPProxyEnabled(bool enabled)
{
	return m_config->Write(wxT("/Server/HTTP Proxy/Enabled"), enabled);
}

bool ServerConfig::SetHTTPProxyHostname(const wxString &hostname)
{
	return m_config->Write(wxT("/Server/HTTP Proxy/Hostname"), hostname);
}

bool ServerConfig::SetHTTPProxyPort(long port)
{
	return m_config->Write(wxT("/Server/HTTP Proxy/Port"), port);
}

bool ServerConfig::SetHTTPProxyUsername(const wxString &username)
{
	return m_config->Write(wxT("/Server/HTTP Proxy/Username"), username);
}

bool ServerConfig::SetHTTPProxyPassword(const wxString &password)
{
	return SetPassword(wxT("/Server/HTTP Proxy/Password"), password);
}

//////// Server ////////

const wxString Server::s_server_nickname = wxT("Console@Server");

enum
{
	ID_CONFIG = 1
};

BEGIN_EVENT_TABLE(Server, wxEvtHandler)
	EVT_CONFIG_FILE_CHANGED(ID_CONFIG, Server::OnConfigFileChanged)
END_EVENT_TABLE()

Server::Server(ServerEventHandler *event_handler)
	: wxEvtHandler(), m_event_handler(event_handler)
{
	m_config.SetEventHandler(this, ID_CONFIG);
	m_log = NULL;
	m_log_public_messages = NULL;
	InitLog();
	m_connections.Alloc(10);
	m_peak_users = 0;
	m_last_active = ::wxGetUTCTime();
	PopulateFilteredWords();
}

Server::~Server()
{
	CloseAllConnections();
	delete m_log;
	delete m_log_public_messages;
}

void Server::InitLog()
{
	delete m_log;
	m_log_warning_given = false;
	wxString log_dir = m_config.GetActualLogDir();
	wxDateTime log_date = LogWriter::GenerateNewLogDate(log_dir, wxT("Server"));
	wxString log_filename = LogWriter::GenerateFilename(log_dir, wxT("Server"), log_date);
	if (log_filename.Length())
	{
		m_log = new LogWriter(log_filename);
	}
	else
	{
		m_log = NULL;
	}
	if (m_log_filename != log_filename)
	{
		m_log_filename = log_filename;
		InitLogPublicMessages(true);
	}
}

void Server::InitLogPublicMessages(bool force)
{
	bool new_state = m_config.GetLogPublicMessages();
	if (new_state != (m_log_public_messages != NULL) || force)
	{
		if (new_state)
		{
			wxFileName fn(m_log_filename);
			fn.SetName(fn.GetName() + wxT(" public messages"));
			wxString log_filename = fn.GetFullPath();
			m_log_public_messages = new LogWriter(log_filename);
			SendToAll(wxEmptyString, wxT("INFO"), wxString(wxT("Note: This server now has public message logging enabled.")), false);
		}
		else
		{
			SendToAll(wxEmptyString, wxT("INFO"), wxString(wxT("Note: This server now has public message logging disabled.")), false);
			delete m_log_public_messages;
			m_log_public_messages = NULL;
		}
	}
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
	if (m_log)
	{
		if (m_log->Ok())
		{
			m_log->AddText(GetLongTimestamp() + line, *wxBLACK, false);
		}
		else if (!m_log_warning_given)
		{
			m_log_warning_given = true;
			Warning(wxT("Error writing log file"));
		}
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
	if (m_log)
	{
		if (m_log->Ok())
		{
			m_log->AddText(GetLongTimestamp() + line, *wxRED, false);
		}
		else if (!m_log_warning_given)
		{
			m_log_warning_given = true;
			Warning(wxT("Error writing log file"));
		}
	}
}

wxArrayString Server::GetSupportedCommands() const
{
	wxArrayString cmds;
	WX_APPEND_ARRAY(cmds, SplitString(wxT("INFO HELP KICK START STOP USERS WORDFILTER"), wxT(" ")));
	WX_APPEND_ARRAY(cmds, m_event_handler->OnServerSupportedCommands());
	cmds.Sort();
	return cmds;
}

void Server::LogConsoleInput(const wxString &cmd, const wxString &params, const wxString &nick)
{
	wxString input = cmd;
	if (params.Length())
	{
		input << wxT(" ") << params;
	}
	LogConsoleInput(input, nick);
}

void Server::LogConsoleInput(const wxString &input, const wxString &nick)
{
	if (nick.Length())
	{
		Information(nick + wxT(" issued command: ") + input);
	}
}

void Server::ProcessConsoleInput(const wxString &input, const wxString &nick)
{

	wxString cmd, params;
	bool is_cmd = false;

	SplitQuotedHeadTail(input, cmd, params);
	if (input[0] == wxT('/'))
	{
		cmd = cmd.Mid(1);
		is_cmd = true;
	}
	cmd.MakeUpper();
	cmd.Trim(true);
	cmd.Trim(false);
	params.Trim(true);
	params.Trim(false);

	if (m_event_handler->OnServerConsoleInputPreprocess(cmd, params, nick))
	{
		return;
	}

	if (cmd == wxT("START"))
	{
		LogConsoleInput(input, nick);
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
		LogConsoleInput(input, nick);
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
		LogConsoleInput(input, nick);
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
		LogConsoleInput(input, nick);
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
	else if (cmd == wxT("WORDFILTER"))
	{
		LogConsoleInput(input, nick);
		HeadTail ht = SplitQuotedHeadTail(params);
		ht.head.MakeUpper();
		if (ht.head == wxT("ADD"))
		{
			ht = SplitQuotedHeadTail(ht.tail);
			wxString name = ht.head;
			wxString value = StripQuotes(ht.tail);
			if (name.Length() && value.Length())
			{
				if (m_config.GetConfig()->Write(wxT("/Server/Word Filters/")+name, value) && m_config.GetConfig()->Flush())
				{
					bool none_before = (m_filtered_words_list.GetCount() == 0);
					PopulateFilteredWords();
					Information(wxT("\"")+name+wxT("\" = \"")+value+wxT("\""));
					if (none_before && m_filtered_words_list.GetCount() == 1)
					{
						SendToAll(wxEmptyString, wxT("INFO"), wxString(wxT("Note: This server now has content filtering enabled.")), false);
					}
				}
				else
				{
					Warning(wxT("Could not set word filter for: ")+name);
				}
			}
			else
			{
				Warning(wxT("Missing parameter"));
			}
		}
		else if (ht.head == wxT("REMOVE"))
		{
			ht = SplitQuotedHeadTail(ht.tail);
			wxString name = ht.head;
			wxString value = ht.tail;
			if (name.Length() && !value.Length())
			{
				if (m_config.GetConfig()->HasEntry(wxT("/Server/Word Filters/") + name))
				{
					if (m_config.GetConfig()->DeleteEntry(wxT("/Server/Word Filters/") + name) && m_config.GetConfig()->Flush())
					{
						PopulateFilteredWords();
						Information(wxT("Filtered word deleted: ") + name);
						if (m_filtered_words_list.GetCount() == 0)
						{
							SendToAll(wxEmptyString, wxT("INFO"), wxString(wxT("Note: This server now has content filtering disabled.")), false);
						}
					}
					else
					{
						Warning(wxT("Could not delete word filter: ") + name);
					}
				}
				else
				{
					Warning(wxT("No such word filter: ") + name);
				}
			}
			else
			{
				Warning(wxT("Invalid parameters"));
			}
		}
		else if (ht.head == wxT("LIST") || !ht.head.Length())
		{
			if (!m_filtered_words_list.IsEmpty())
			{
				wxArrayString list = m_filtered_words_list;
				list.Sort();
				Information(wxT("Current filtered words:"));
				for (size_t i = 0; i < list.GetCount(); ++i)
				{
					wxString key = list[i];
					wxString value = m_config.GetConfig()->Read(wxT("/Server/Word Filters/")+key);
					Information(wxT("    \"") + key + wxT("\" = \"") + value + wxT("\""));
				}
			}
			else
			{
				Information(wxT("No filtered words"));
			}
		}
		else if (ht.head == wxT("HELP"))
		{
			Warning(wxT("Valid word filter commands: ADD, HELP, LIST, REMOVE"));
		}
		else
		{
			Warning(wxT("Invalid word filter command"));
		}
	}
	else if (cmd == wxT("INFO"))
	{
		LogConsoleInput(input, nick);
		if (!IsRunning())
		{
			Warning(wxT("Server is not running"));
			return;
		}
		wxString colon_port;
		colon_port << wxT(':') << m_config.GetListenPort();
		Information(wxT("Server Information:"));
		Information(wxT("    Server Name:  ") + m_config.GetServerName());
		Information(wxT("    Hostname:     ") + (m_config.GetHostname().Length() ? (m_config.GetHostname() + colon_port) : wxString()));
		wxString tmp = wxT("    Listening on: ");
		wxArrayString iplist = GetIPAddresses();
		for (size_t i = 0u; i < iplist.GetCount(); ++i)
		{
			Information(tmp + iplist[i] + colon_port);
			tmp = wxT("                  ");
		}
		Information(wxT("    Users:        ") + wxString()
			<< GetUserCount() << wxT(" (")
			<< GetAwayCount() << wxT(" away, ")
			<< m_peak_users << wxT(" peak, ")
			<< m_config.GetMaxUsers() << wxT(" max)"));
		Information(wxT("    Average Ping: ") + wxString() << GetAverageLatency() << wxT(" ms"));
		Information(wxT("    Version:      ") + GetProductVersion() + wxT(" ") + SplitHeadTail(GetRCSDate(), wxT(' ')).head);
		Information(wxT("    Uptime:       ") + SecondsToMMSS(GetServerUptime()));
		Information(wxT("    Comment:      ") + m_config.GetPublicListComment());
		Information(wxT("End of Server Information"));
	}
	else if (cmd == wxT("HELP"))
	{
		LogConsoleInput(input, nick);
		Information(wxT("Supported commands: ") + JoinArray(GetSupportedCommands(), wxT(" ")));
	}
	else
	{
		if (is_cmd && cmd != wxT("SAY") && cmd != wxT("ME"))
		{
			LogConsoleInput(input, nick);
			Warning(wxT("Unrecognized command: ") + cmd);
			return;
		}
		wxString msg;
		bool is_action = false;
		if (cmd.Left(1) == wxT('.'))
		{
			msg = input.Mid(1);
		}
		else if (cmd == wxT("SAY"))
		{
			msg = params;
		}
		else if (cmd == wxT("ME"))
		{
			msg = params;
			is_action = true;
		}
		else
		{
			msg = input;
		}
		if (msg.Length())
		{
			wxString src;
			if (nick.Length() > 0)
			{
				int i = nick.Find(wxT('@'));
				if (i > -1)
				{
					src = nick.Left(i);
				}
				else
				{
					src = nick;
				}
			}
			else
			{
				src = GetServerNickname();
			}
			if (is_action)
			{
				Information(wxString() << wxT("* ") << src << wxT(" ") << msg);
			}
			else
			{
				Information(wxString() << wxT("<") << src << wxT("> ") << msg);
			}
		}
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

ByteBuffer Server::MakeNicknameValid(const ByteBuffer &src)
{

	wxString org_str = (wxString)src;

	wxString source_str = ConvertModifiersIntoHtml(org_str, true);

	wxString output;
	output.Alloc(source_str.Length());

	// grab any valid characters
	for (size_t i = 0; i < source_str.Length(); ++i)
	{
		wxChar c = source_str[i];
		if (wxIsalnum(c))
		{
			output += c;
		}
		else
		{
			switch (c)
			{
				case wxT('_'):
				case wxT('-'):
				case wxT('['):
				case wxT(']'):
				case wxT('\''):
				case wxT(' '):
					output += c;
					break;
				default:
					break;
			}
		}
	}

	// replace all multiple spaces with a single space
	while (output.Replace(wxT("  "), wxT(" ")) > 0);

	// return the new string if anything changed
	return (org_str == output) ? src : (ByteBuffer)output;

}

bool Server::IsValidNickname(const wxString &nickname)
{

	bool hasAlphaNum = false; /* used to test that the nick has at least one
								 alpha numeric character */

	if (nickname.Length() < 1 || nickname.Length() > 32 || nickname == GetServerNickname())
	{
		return false;
	}

	for (size_t i = 0; i < nickname.Length(); ++i)
	{
		wxChar c = nickname[i];
		if (wxIsalnum(c))
		{
			hasAlphaNum = true;
		}
		else
		{
			switch (c)
			{
				case wxT('_'):
				case wxT('-'):
				case wxT('['):
				case wxT(']'):
				case wxT('\''):
					break;
				case wxT(' '):
					// allow no leading or trailing spaces in nicks
					if ((i == 0) || (i == (nickname.Length() - 1)))
					{
						return false;
					}
					else
					{
						// do not allow a nick with two consecutive spaces
						if (nickname[i+1] == ' ')
						{
							return false;
						}
				    }
				    break;
				default:
					return false;
			}
		}
	}

	return hasAlphaNum;

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

	if (cmd != wxT("PING") && cmd != wxT("PONG") && cmd != wxT("CTCPREPLY"))
	{
		m_last_active = ::wxGetUTCTime();
		conn->m_last_active = m_last_active;
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
		ByteBuffer msg = ProcessWordFilters(data);
		SendToAll(wxEmptyString, cmd, Pack(conn->GetNickname(), msg), true);
		if (m_log_public_messages)
		{
			if (cmd == wxT("PUBMSG"))
			{
				m_log_public_messages->AddText(wxString()
					<< GetLongTimestamp() << wxT("<")
					<< conn->GetNickname() << wxT("> ")
					<< msg);
			}
			else
			{
				wxString sep = (msg.Left(2)==wxT("'s")) ? wxT("") : wxT(" ");
				m_log_public_messages->AddText(wxString()
					<< GetLongTimestamp() << wxT("* ")
					<< conn->GetNickname() << sep
					<< msg, colours[6]);
			}
		}
	}
	else if (cmd == wxT("PRIVMSG") || cmd == wxT("PRIVACTION") || cmd == wxT("CTCP") || cmd == wxT("CTCPREPLY"))
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
					if (cmd == wxT("PRIVACTION"))
					{
						if (msg.Length())
						{
							msg = wxT("/me ") + msg;
						}
						else
						{
							msg = wxString(wxT("/me"));
						}
					}
					ProcessConsoleInput(msg, conn->GetId());
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
					bool is_ctcp = (cmd == wxT("CTCP") || cmd == wxT("CTCPREPLY"));
					wxString new_context = is_ctcp ? context : wxString();
					dest->Send(new_context, cmd, Pack(conn->GetNickname(), msg));
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
		ByteBuffer msg;
		if (data.Length())
		{
			msg = wxT("Quit: ") + ProcessWordFilters(data);
		}
		else
		{
			msg = wxString(wxT("Quit"));
		}
		conn->Terminate(msg);
	}
	else if (cmd == wxT("AWAY"))
	{
		if (!conn->m_isaway || conn->m_awaymessage != data)
		{
			bool last_state = conn->m_isaway;
			wxString last_msg = conn->m_awaymessage;
			long last_time = (::wxGetUTCTime()-conn->m_awaytime);
			conn->m_isaway = true;
			conn->m_awaymessage = ProcessWordFilters(data);
			conn->m_awaytime = ::wxGetUTCTime();
			SendToAll(wxEmptyString, cmd, Pack(conn->GetNickname(), conn->m_awaymessage, Pack(wxString() << conn->m_awaytime, wxString() << 0)), true);
			if (m_log_public_messages)
			{
				wxString str;
				str << GetLongTimestamp() << wxT("*** ") << conn->GetNickname();
				str << wxT(" is away: ") << conn->m_awaymessage;
				if (last_state)
				{
					str << (wxChar)OriginalModifier;
					str << wxT(" (was: ") << last_msg;
					str << (wxChar)OriginalModifier;
					str << wxT(" for ") << SecondsToMMSS(last_time) << wxT(")");
				}
				m_log_public_messages->AddText(str, colours[2]);
			}
		}
	}
	else if (cmd == wxT("BACK"))
	{
		if (conn->m_isaway)
		{
			long away_time = ::wxGetUTCTime() - conn->m_awaytime;
			SendToAll(wxEmptyString, cmd, Pack(conn->GetNickname(), conn->m_awaymessage, Pack(wxString()<<conn->m_awaytime, wxString() << away_time)), true);
			if (m_log_public_messages)
			{
				wxString str;
				str << GetLongTimestamp() << wxT("*** ") << conn->GetNickname();
				str << wxT(" has returned (msg: ") << conn->m_awaymessage;
				str << (wxChar)OriginalModifier;
				str << wxT(") (away for ") << SecondsToMMSS(away_time) << wxT(")");
				m_log_public_messages->AddText(str, colours[2]);
			}
			conn->m_isaway = false;
			conn->m_awaymessage = ByteBuffer();
		}
	}
	else if (cmd == wxT("WHOIS"))
	{
		const ServerConnection *user = GetConnection(data);
		if (user)
		{
			ByteBufferHashMap map;
			map[wxT("NICK")] = user->GetNickname();
			map[wxT("HOSTNAME")] = user->GetRemoteHost();
			map[wxT("DETAILS")] = user->GetUserDetails();
			if (user->IsAdmin())
			{
				map[wxT("ISADMIN")] = ByteBuffer();
			}
			if (user->IsAway())
			{
				map[wxT("AWAY")] = user->GetAwayMessage();
				map[wxT("AWAYTIME")] = wxString() << user->GetAwayTime();
				long away_time_diff = wxGetUTCTime() - user->GetAwayTime();
				map[wxT("AWAYTIMEDIFF")] = wxString() << away_time_diff;
				map[wxT("AWAYTIMEDIFFSTRING")] = SecondsToMMSS(away_time_diff);
			}
			map[wxT("IDLE")] = wxString() << user->GetIdleTime();
			map[wxT("IDLESTRING")] = user->GetIdleTimeString();
			map[wxT("LATENCY")] = wxString() << user->GetLatency();
			map[wxT("LATENCYSTRING")] = user->GetLatencyString();
			map[wxT("AGENT")] = user->GetUserAgent();
			map[wxT("JOINTIME")] = wxString() << user->GetJoinTime().ToGMT().GetTicks();
			map[wxT("JOINTIMESTRING")] = user->GetJoinTimeString();
			conn->Send(context, cmd, PackByteBufferHashMap(map));
		}
		else
		{
			conn->Send(context, wxT("ERROR"), Pack(wxString(wxT("NONICK")), wxT("No suck nick: ") + data));
		}
	}
	else if (cmd == wxT("NICK"))
	{

		if (!IsValidNickname(data))
		{
			conn->Send(context, wxT("ERROR"), Pack(wxString(wxT("NICK")), wxT("Invalid nickname: ") + data));
			return;
		}

		ByteBuffer new_nick = MakeNicknameValid(ProcessWordFilters(data));

		if (!IsValidNickname(new_nick))
		{
			conn->Send(context, wxT("ERROR"), Pack(wxString(wxT("NICK")), wxT("Invalid nickname: ") + data));
			return;
		}

		ServerConnection *tmp = GetConnection(new_nick);

		if (tmp && tmp != conn)
		{
			conn->Send(context, wxT("ERROR"), Pack(wxString(wxT("NICK")), wxT("Nickname in use: ") + tmp->GetNickname()));
		}
		else
		{

			if ((ByteBuffer)conn->m_nickname != new_nick)
			{
				if (conn->m_nickname.Length() == 0)
				{
					ByteBuffer nicklist = GetNickList();
					conn->Send(context, wxT("NICK"), new_nick);
					conn->m_nickname = new_nick;
					Information(conn->GetId() + wxT(" has entered the chat"));
					if (m_log_public_messages)
					{
						m_log_public_messages->AddText(wxString()
							<< GetLongTimestamp() << wxT("*** ")
							<< conn->GetNickname()
							<< wxT(" (") << conn->GetInlineDetails()
							<< wxT(") has joined the chat"), colours[3]);
					}
					SendToAll(wxEmptyString, wxT("JOIN"), Pack(new_nick, conn->GetInlineDetails()), true);
					conn->Send(context, wxT("NICKLIST"), nicklist);
					size_t m_user_count = GetUserCount();
					m_peak_users = wxMax(m_peak_users, m_user_count);
					#if wxUSE_SOUND
						wxString filename = m_config.GetSoundJoin();
						if (filename.Length() && wxFileName(filename).FileExists())
						{
							m_sound.Create(filename, false);
							if (!m_sound.IsOk() || !m_sound.Play())
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
							conn->Send(wxEmptyString, wxT("AWAY"), Pack(conn2->GetNickname(), conn2->GetAwayMessage(), Pack(wxString()<<conn2->GetAwayTime(), wxString()<<(::wxGetUTCTime()-conn2->GetAwayTime()))));
						}
					}
				}
				else
				{
					if (m_log_public_messages)
					{
						m_log_public_messages->AddText(wxString()
							<< GetLongTimestamp() << wxT("*** ")
							<< conn->GetNickname()
							<< wxT(" is now known as ")
							<< new_nick, colours[3]);
					}
					SendToAll(wxEmptyString, wxT("NICK"), Pack(conn->m_nickname, new_nick), true);
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

static int CompareStringLengths(const wxString &a, const wxString &b)
{
	return b.length() - a.length();
}

void Server::PopulateFilteredWords()
{
	m_filtered_words_list.Empty();
	m_config.BeginBatch();
	wxString old_path = m_config.GetConfig()->GetPath();
	m_config.GetConfig()->SetPath(wxT("/Server/Word Filters"));
	m_filtered_words_list.Alloc(m_config.GetConfig()->GetNumberOfEntries(false));
	wxString val;
	long i;
	if (m_config.GetConfig()->GetFirstEntry(val, i))
	{
		do
		{
			m_filtered_words_list.Add(ConvertModifiersIntoHtml(val, true));
		}
		while (m_config.GetConfig()->GetNextEntry(val, i));
	}
	m_config.GetConfig()->SetPath(old_path);
	m_config.EndBatch();
	m_filtered_words_list.Sort(&CompareStringLengths);
}

void Server::OnConfigFileChanged(wxCommandEvent &WXUNUSED(event))
{
	PopulateFilteredWords();
	InitLogPublicMessages(false);
}

wxString Server::ProcessWordFilters(const wxString &text) const
{
	const wxString text_no_formatting = ConvertModifiersIntoHtml(text, true);
	wxString output = text_no_formatting;
	for (size_t i = 0; i < m_filtered_words_list.GetCount(); ++i)
	{
		wxString old_value = m_filtered_words_list[i];
		wxString new_value = m_config.GetConfig()->Read(wxT("/Server/Word Filters/") + old_value);
		output = CaseInsensitiveReplace(output, m_filtered_words_list[i], new_value);
	}
	return (output != text_no_formatting) ? output : text;
}

ByteBuffer Server::ProcessWordFilters(const ByteBuffer &data) const
{
	wxString text = data;
	wxString text_filtered = ProcessWordFilters(text);
	ByteBuffer buff_filtered = text_filtered;
	return (text != text_filtered) ? buff_filtered : data;
}
