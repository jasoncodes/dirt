#if defined(__WXGTK__) || defined(__WXMOTIF__)
	#include "wx/wx.h"
#endif
#include "wx/wxprec.h"
#include "RCS.h"
RCS_ID($Id: Server.cpp,v 1.21 2003-02-26 05:48:26 jason Exp $)

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
	m_awaymessage = wxEmptyString;
	m_latency = -1;
	m_useragent = wxEmptyString;
	m_authenticated = false;
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
	if (GetAwayMessage().Length())
	{
		retval << wxT("Away: ") << GetAwayMessage() << wxT("; ");
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

bool ServerConfig::SetListenPort(long port)
{
	return m_config->Write(wxT("Server/Listen Port"), port);
}

wxString ServerConfig::GetUserPassword(bool decrypt) const
{
	return GetPassword(wxT("Server/User Password"), decrypt);
}

bool ServerConfig::SetUserPassword(const wxString &password)
{
	return SetPassword(wxT("Server/User Password"), password);
}

wxString ServerConfig::GetAdminPassword(bool decrypt) const
{
	return GetPassword(wxT("Server/Admin Password"), decrypt);
}

bool ServerConfig::SetAdminPassword(const wxString &password)
{
	return SetPassword(wxT("Server/Admin Password"), password);
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

	const byte *ptr = data.Lock();
	ByteBuffer crc32(ptr, 4);
	ByteBuffer len_buff(ptr+4, 4);
	size_t len = BytesToUint32(len_buff.Lock(), len_buff.Length());
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
	
	ByteBuffer dec(data.Lock(), len);
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

BEGIN_EVENT_TABLE(Server, wxEvtHandler)
END_EVENT_TABLE()

Server::Server(ServerEventHandler *event_handler)
	: wxEvtHandler(), m_event_handler(event_handler)
{
	m_connections.Alloc(10);
	m_config = new ServerConfig;
}

Server::~Server()
{
	CloseAllConnections();
	delete m_config;
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
			m_event_handler->OnServerWarning(wxT("Server is already running"));
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
			m_event_handler->OnServerWarning(wxT("Server is not running"));
		}
	}
	else if (cmd == wxT("USERS"))
	{
		if (IsRunning())
		{
			m_event_handler->OnServerInformation(wxString() << wxT("There are currently ") << GetConnectionCount() << wxT(" connections"));
			for (size_t i = 0; i < GetConnectionCount(); ++i)
			{
				m_event_handler->OnServerInformation(wxT("    ") + *GetConnection(i));
			}
		}
		else
		{
			m_event_handler->OnServerWarning(wxT("Server is not running"));
		}
	}
	else if (cmd == wxT("HELP"))
	{
		m_event_handler->OnServerInformation(wxT("Supported commands: HELP START STOP USERS"));
	}
	else
	{
		m_event_handler->OnServerWarning(wxT("Unrecognized command: ") + cmd);
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

ServerConnection* Server::GetConnection(const wxString &nickname)
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
    if (nickname.Length() < 1 || nickname.Length() > 16)
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

ByteBuffer Server::GetNickList()
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
		else
		{
			conn->Send(context, wxT("ERROR"), Pack(wxString(wxT("MALFORMED")), wxT("Malformed data in ") + cmd));
		}
	}
	else if (cmd == wxT("USERDETAILS"))
	{
		conn->m_userdetails = data;
		m_event_handler->OnServerInformation(conn->GetId() + wxT(" is ") + conn->GetUserDetails());
	}
	else if (cmd == wxT("USERAGENT"))
	{
		conn->m_useragent = data;
		m_event_handler->OnServerInformation(conn->GetId() + wxT(" is running ") + conn->GetUserAgent());
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
			map[wxT("AWAY")] = user->GetAwayMessage();
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
				conn->Send(context, wxT("ERROR"), Pack(wxString(wxT("NICK")), wxString(wxT("Invalid nickname"))));
			}
			else if (conn->m_nickname != new_nick)
			{
				if (conn->m_nickname.Length() == 0)
				{
					ByteBuffer nicklist = GetNickList();
					conn->Send(context, wxT("NICK"), data);
					conn->m_nickname = new_nick;
					m_event_handler->OnServerInformation(conn->GetId() + wxT(" has entered the chat"));
					SendToAll(wxEmptyString, wxT("JOIN"), Pack(data, conn->GetInlineDetails()), true);
					conn->Send(context, wxT("NICKLIST"), nicklist);
				}
				else
				{
					SendToAll(wxEmptyString, wxT("NICK"), Pack(conn->m_nickname, data), true);
					m_event_handler->OnServerInformation(conn->GetId() + wxT(" is now known as ") + new_nick);
					conn->m_nickname = new_nick;
				}
			}
		}
	}
	else if (!ProcessClientInputExtra(false, false, conn, context, cmd, data))
	{
		m_event_handler->OnServerInformation(wxT("Unknown message recv'd:"));
		m_event_handler->OnServerInformation(wxT("Context: \"") + context + wxT("\""));
		m_event_handler->OnServerInformation(wxT("Command: \"") + cmd + wxT("\""));
		m_event_handler->OnServerInformation(wxT("Data: ") + data.GetHexDump());
		conn->Send(context, wxT("ERROR"), Pack(wxString(wxT("NOCMD")), wxT("Unrecognized command: ") + cmd));
	}

}
