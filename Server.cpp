#if defined(__WXGTK__) || defined(__WXMOTIF__)
	#include "wx/wx.h"
#endif
#include "wx/wxprec.h"
#include "RCS.h"
RCS_ID($Id: Server.cpp,v 1.13 2003-02-19 09:46:57 jason Exp $)

#include "Server.h"
#include "Modifiers.h"
#include "util.h"

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

BEGIN_EVENT_TABLE(Server, wxEvtHandler)
END_EVENT_TABLE()

Server::Server(ServerEventHandler *event_handler)
	: wxEvtHandler(), m_event_handler(event_handler)
{
	m_connections.Alloc(10);
}

Server::~Server()
{
	CloseAllConnections();
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
	return true;
    /*if (nickname.Length() < 1 || nickname.Length() > 9)
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
	return true;*/
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

	conn->ResetIdleTime();

	if (conn->GetNickname().Length() == 0 && cmd != wxT("NICK") && cmd != wxT("USERDETAILS") && cmd != wxT("USERAGENT"))
	{
		conn->Send(context, wxT("ERROR"), Pack(wxString(wxT("NONICK")), wxString(wxT("You need to set a nickname first"))));
		return;
	}

	if (cmd == wxT("PUBMSG"))
	{
		SendToAll(wxEmptyString, cmd, Pack(conn->GetNickname(), data), true);
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
	else if (cmd == wxT("PRIVMSG"))
	{
		ByteBuffer nick;
		ByteBuffer msg;
		if (Unpack(data, nick, msg))
		{
			const ServerConnection *dest = SendToNick(nick, wxEmptyString, cmd, Pack(conn->GetNickname(), msg));
			if (dest)
			{
				conn->Send(context, wxT("PRIVMSGOK"), Pack(dest->GetNickname(), msg));
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
					
					conn->Send(context, wxT("NICKLIST"), GetNickList());
					conn->Send(context, wxT("NICK"), data);
					conn->m_nickname = new_nick;
					m_event_handler->OnServerInformation(conn->GetId() + wxT(" has entered the chat"));
					SendToAll(wxEmptyString, wxT("JOIN"), Pack(data, conn->GetInlineDetails()), true);
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
	else
	{

		m_event_handler->OnServerInformation(wxT("Unknown message recv'd:"));
		m_event_handler->OnServerInformation(wxT("Context: \"") + context + wxT("\""));
		m_event_handler->OnServerInformation(wxT("Command: \"") + cmd + wxT("\""));
		m_event_handler->OnServerInformation(wxT("Data: ") + data.GetHexDump());
		conn->Send(context, wxT("ERROR"), Pack(wxString(wxT("NOCMD")), wxT("Unrecognized command: ") + cmd));

	}

}

