#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: Client.cpp,v 1.32 2003-03-13 00:48:36 jason Exp $)

#include "Client.h"
#include "util.h"
#include "Modifiers.h"
#include "FileTransfers.h"
#include "Crypt.h"

const wxLongLong_t initial_ping_delay = 5000;
const wxLongLong_t ping_interval = 30000;
const wxLongLong_t ping_timeout_delay = 45000;

enum
{
	ID_TIMER_PING = 100,
};

BEGIN_EVENT_TABLE(Client, wxEvtHandler)
	EVT_TIMER(ID_TIMER_PING, Client::OnTimerPing)
END_EVENT_TABLE()

Client::Client(ClientEventHandler *event_handler)
	: wxEvtHandler(), m_event_handler(event_handler)
{
	m_file_transfers = new FileTransfers(this);
	m_nickname = wxEmptyString;
	m_server_name = wxEmptyString;
	m_tmrPing = new wxTimer(this, ID_TIMER_PING);
	m_config = new wxFileConfig(wxT("dirt"));
	m_config->SetUmask(0077);
}

Client::~Client()
{
	delete m_config;
	delete m_tmrPing;
	delete m_file_transfers;
}

void Client::Debug(const wxString &context, const wxString &text)
{
	m_event_handler->OnClientDebug(context, text);
}

void Client::ProcessConsoleInput(const wxString &context, const wxString &input)
{
	
	wxString cmd, params;
	
	if (input[0] == wxT('/'))
	{
		SplitQuotedHeadTail(input, cmd, params);
		cmd = cmd.Mid(1);
		cmd.MakeUpper();
	}
	else
	{
		cmd = wxT("SAY");
		params = input;
	}
	cmd.Trim(true);
	cmd.Trim(false);
	if (cmd != wxT("SAY"))
	{
		params.Trim(true);
		params.Trim(false);
	}

	//Debug(context, wxString() << wxT("Command: \"") << cmd << wxT("\", Params: \"") << params << (wxChar)OriginalModifier << wxT("\""));

	if (m_event_handler->OnClientPreprocess(context, cmd, params))
	{
		return;
	}

	if (cmd == wxT("SAY") || cmd == wxT("ME"))
	{
		ASSERT_CONNECTED();
		SendMessage(context, context, params, cmd == wxT("ME"));
	}
	else if (cmd == wxT("MSG") || cmd == wxT("MSGME"))
	{
		ASSERT_CONNECTED();
		wxString nick, msg;
		SplitQuotedHeadTail(params, nick, msg);
		if (nick.Length() == 0)
		{
			m_event_handler->OnClientInformation(context, wxT("/msg: insufficient parameters"));
		}
		else if (msg.Length() > 0)
		{
			SendMessage(context, nick, msg, cmd == wxT("MSGME"));
		}
	}
	else if (cmd == wxT("CONNECT") || cmd == wxT("SERVER"))
	{
		if (IsConnected())
		{
			Disconnect();
		}
		if (!Connect(params))
		{
			m_event_handler->OnClientWarning(context, wxT("Error connecting to ") + params);
		}
	}
	else if (cmd == wxT("DISCONNECT"))
	{
		ASSERT_CONNECTED();
		Disconnect();
	}
	else if (cmd == wxT("RECONNECT"))
	{
		ProcessConsoleInput(context, wxT("/connect ") + GetLastURL());
	}
	else if (cmd == wxT("NICK"))
	{
		ASSERT_CONNECTED();
		if (params.Length())
		{
			SetNickname(context, params);
		}
		else
		{
			m_event_handler->OnClientUserNick(m_nickname, m_nickname);
		}
	}
	else if (cmd == wxT("WHOIS"))
	{
		ASSERT_CONNECTED();
		WhoIs(context, params);
	}
	else if (cmd == wxT("OPER"))
	{
		ASSERT_CONNECTED();
		Oper(context, params);
	}
	else if (cmd == wxT("AWAY"))
	{
		ASSERT_CONNECTED();
		Away(params);
	}
	else if (cmd == wxT("BACK"))
	{
		ASSERT_CONNECTED();
		Back();
	}
	else if (cmd == wxT("HELP"))
	{
		m_event_handler->OnClientInformation(context, wxT("Supported commands: AWAY BACK CONNECT DISCONNECT HELP ME MSG MSGME NICK RECONNECT SAY SERVER WHOIS"));
	}
	else if (cmd == wxT("LIZARD"))
	{
		m_event_handler->OnClientInformation(context, wxT("Support for Lizard technology is not available at this time"));
	}
	else if (cmd == wxT("ALIAS"))
	{
		HeadTail ht = SplitQuotedHeadTail(params);
		if (ht.head.Length())
		{
			if (SetAlias(ht.head, ht.tail))
			{
				m_event_handler->OnClientInformation(context, ht.head + wxT(" = ") + (ht.tail.Length()?(wxT('"')+ht.tail+wxT('"')):wxString(wxT("(Nothing)"))));
			}
			else
			{
				m_event_handler->OnClientWarning(context, wxT("Error setting alias"));
			}
		}
		else
		{
			m_event_handler->OnClientInformation(context, wxT("Current aliases:"));
			wxArrayString list = GetAliasList();
			if (list.GetCount())
			{
				for (size_t i = 0; i < list.GetCount(); ++i)
				{
					m_event_handler->OnClientInformation(context, wxT("    ")+list.Item(i)+wxT(" = \"")+GetAlias(list.Item(i))+wxT('"'));
				}
			}
			else
			{
				m_event_handler->OnClientInformation(context, wxT("    (None)"));
			}
		}
	}
	else
	{
		wxString cmds = GetAlias(cmd);
		if (cmds.Length())
		{
			ProcessAlias(context, cmds, params);
		}
		else
		{
			m_event_handler->OnClientWarning(context, wxT("Unrecognized command: ") + cmd);
		}
	}

}

void Client::ProcessServerInput(const ByteBuffer &msg)
{
	wxString context, cmd;
	ByteBuffer data;
	if (DecodeMessage(msg, context, cmd, data))
	{
		ProcessServerInput(context, cmd, data);
	}
	else
	{
		m_event_handler->OnClientWarning(context, wxT("Error decoding message from server. Ignored."));
		m_event_handler->OnClientDebug(context, msg.GetHexDump());
	}
}

void Client::ProcessServerInput(const wxString &context, const wxString &cmd, const ByteBuffer &data)
{

	if (ProcessServerInputExtra(true, context, cmd, data))
	{
		return;
	}

	if (cmd == wxT("PUBMSG") || cmd == wxT("PUBACTION"))
	{
		ByteBuffer nick, text;
		if (!Unpack(data, nick, text))
		{
			nick = data;
			text = ByteBuffer();
		}
		m_event_handler->OnClientMessageIn(nick, text, cmd == wxT("PUBACTION"), false);
	}
	else if (cmd == wxT("PRIVMSG") || cmd == wxT("PRIVACTION"))
	{
		ByteBuffer nick, text;
		if (!Unpack(data, nick, text))
		{
			nick = data;
			text = ByteBuffer();
		}
		m_event_handler->OnClientMessageIn(nick, text, cmd == wxT("PRIVACTION"), true);
	}
	else if (cmd == wxT("PRIVMSGOK") || cmd == wxT("PRIVACTIONOK"))
	{
		ByteBuffer nick, text;
		if (!Unpack(data, nick, text))
		{
			nick = data;
			text = ByteBuffer();
		}
		m_event_handler->OnClientMessageOut(context, nick, text, cmd == wxT("PRIVACTIONOK"));
	}
	else if (cmd == wxT("PONG"))
	{
		wxLongLong_t now = GetMillisecondTicks();
		m_latency = (long)(now - m_ping_next);
		m_ping_next = now + ping_interval;
		m_ping_data = wxEmptyString;
		m_event_handler->OnClientStateChange();
	}
	else if (cmd == wxT("ERROR"))
	{
		ByteBuffer type, text;
		if (!Unpack(data, type, text))
		{
			type = data;
			text = ByteBuffer();
		}
		m_event_handler->OnClientError(context, type, text);
	}
	else if (cmd == wxT("INFO"))
	{
		m_event_handler->OnClientInformation(context, data);
	}
	else if (cmd == wxT("WHOIS"))
	{
		m_event_handler->OnClientWhoIs(context, UnpackHashMap(data));
	}
	else if (cmd == wxT("AWAY") || cmd == wxT("BACK"))
	{
		ByteBuffer nick, text;
		if (!Unpack(data, nick, text))
		{
			nick = data;
			text = ByteBuffer();
		}
		if (cmd == wxT("AWAY"))
		{
			m_event_handler->OnClientUserAway(nick, text);
		}
		else
		{
			m_event_handler->OnClientUserBack(nick, text);
		}
	}
	else if (cmd == wxT("JOIN"))
	{
		ByteBuffer nick, details;
		if (!Unpack(data, nick, details))
		{
			nick = data;
			details = ByteBuffer();
		}
		m_event_handler->OnClientUserJoin(nick, details);
		if ((wxString)nick == m_nickname)
		{
			m_event_handler->OnClientStateChange();
		}
	}
	else if (cmd == wxT("PART"))
	{
		ByteBuffer nick, details, msg;
		if (!Unpack(data, nick, details, msg))
		{
			nick = data;
			details = ByteBuffer();
			msg = ByteBuffer();
		}
		m_event_handler->OnClientUserPart(nick, details, msg);
		if ((wxString)nick == m_nickname)
		{
			m_event_handler->OnClientStateChange();
		}
	}
	else if (cmd == wxT("NICK"))
	{
		ByteBuffer nick1, nick2;
		if (Unpack(data, nick1, nick2))
		{
			if ((wxString)nick1 == m_nickname)
			{
				m_nickname = nick2;
				m_event_handler->OnClientStateChange();
			}
			m_event_handler->OnClientUserNick(nick1, nick2);
			// add handling for file transfers here
		}
		else
		{
			m_nickname = data;
		}
	}
	else if (cmd == wxT("NICKLIST"))
	{
		wxArrayString nicks;
		ByteBufferArray nickbuff = Unpack(data);
		for (size_t i = 0; i < nickbuff.GetCount(); ++i)
		{
			nicks.Add(nickbuff.Item(i));
		}
		m_event_handler->OnClientUserList(nicks);
	}
	else if (cmd == wxT("SERVERNAME"))
	{
		m_server_name = data;
		m_event_handler->OnClientStateChange();
	}
	else
	{
		if (!ProcessServerInputExtra(false, context, cmd, data))
		{
			m_event_handler->OnClientDebug(context, wxT("Unknown message recv'd:"));
			m_event_handler->OnClientDebug(context, wxT("Context: \"") + context + wxT("\""));
			m_event_handler->OnClientDebug(context, wxT("Command: \"") + cmd + wxT("\""));
			m_event_handler->OnClientDebug(context, wxT("Data: ") + data.GetHexDump());
		}
	}

}

void Client::WhoIs(const wxString &context, const wxString &nick)
{
	ASSERT_CONNECTED();
	SendToServer(EncodeMessage(context, wxT("WHOIS"), nick));
}

void Client::Away(const wxString &msg)
{
	wxString context = wxEmptyString;
	ASSERT_CONNECTED();
	SendToServer(EncodeMessage(context, msg.Length()?wxT("AWAY"):wxT("BACK"), msg));
}

void Client::OnConnect()
{
	m_tmrPing->Start(2500);
	m_ping_next = GetMillisecondTicks() + initial_ping_delay;
	m_ping_data = wxEmptyString;
	m_latency = -1;
	m_event_handler->OnClientInformation(wxEmptyString, wxT("Connected"));
	m_event_handler->OnClientStateChange();
	wxString userdetails;
	userdetails << ::wxGetUserId() << wxT('@') << ::wxGetHostName();
	userdetails << wxT(" (\"") << ::wxGetUserName() << wxT("\")");
	userdetails << wxT(" on ") << ::wxGetOsDescription();
	SendToServer(EncodeMessage(wxEmptyString, wxT("USERDETAILS"), userdetails));
	SendToServer(EncodeMessage(wxEmptyString, wxT("USERAGENT"), GetProductVersion() + wxT(' ') + GetRCSDate()));
}

void Client::OnTimerPing(wxTimerEvent &event)
{
	if (IsConnected())
	{
		wxLongLong_t now = GetMillisecondTicks();
		if (now > m_ping_next)
		{
			if (m_ping_data.Length() == 0)
			{
				m_ping_data = Crypt::Random(8).GetHexDump(false, false);
				m_ping_timeout_tick = now + ping_timeout_delay;
				SendToServer(EncodeMessage(wxEmptyString, wxT("PING"), m_ping_data));
			}
			else if (now > m_ping_timeout_tick)
			{
				Disconnect(wxT("Ping timeout"));
			}
		}
	}
	else
	{
		m_tmrPing->Stop();
	}
}

wxString Client::GetNickname() const
{
	return m_nickname;
}

wxString Client::GetServerName() const
{
	return m_server_name;
}

wxString Client::GetDefaultNick() const
{
	wxString nick = ::wxGetUserId();
	int i = nick.Index(wxT(' '));
	if (i > -1)
	{
		nick = nick.Left(i);
	}
	return nick;
}

static inline wxString GetToken(wxString &tokens, const wxString &sep)
{
	HeadTail ht = SplitHeadTail(tokens, sep);
	tokens = ht.tail;
	return ht.head;
}

void Client::ProcessAlias(const wxString &context, const wxString &cmds, const wxString &params)
{
	wxString tokens(cmds);
	wxArrayString param_list = SplitQuotedString(params);
	while (tokens.Length())
	{
		wxString line = GetToken(tokens, wxT("|"));
		while (tokens.Length() && tokens[0] == wxT('|'))
		{
			tokens = tokens.Mid(1);
			line += wxT('|') + GetToken(tokens, wxT("|"));
		}
		wxString output;
		output.Alloc(line.Length()*2);
		int dollar_pos = 0;
		int num[2];
		bool num_valid[2];
		bool delim = false;
		wxString tmp;
		for (size_t i = 0; i <= line.Length(); ++i)
		{
			bool at_end = i == line.Length();
			wxChar c = at_end?0:line[i];
			if (dollar_pos)
			{
				if (dollar_pos == 1 && c == wxT('$')) // $$
				{
					dollar_pos = 0;
					output += c;
				}
				else if (dollar_pos == 1 && c == wxT('*')) // $*
				{
					dollar_pos = 0;
					output += params;
				}
				else if (wxIsdigit(c))
				{
					tmp += c;
					int index = delim?1:0;
					num_valid[index] = true;
					num[index] *= 10;
					num[index] += (c - wxT('0'));
					dollar_pos++;
				}
				else if (c == wxT('-') && !delim && num_valid[0])
				{
					tmp += c;
					delim = true;
					dollar_pos++;
				}
				else
				{
					size_t start, end;
					bool valid;
					if (num_valid[0] && num_valid[1] && delim) // $x-y
					{
						start = num[0] - 1;
						end = num[1] - 1;
						valid = true;
					}
					else if (num_valid[0] && !num_valid[1] && !delim) // $x
					{
						start = num[0] - 1;
						end = start;
						valid = true;
					}
					else if (num_valid[0] && !num_valid[1] && delim) // $x-
					{
						start = num[0] - 1;
						end = param_list.GetCount() - 1;
						valid = true;
					}
					else
					{
						output += tmp;
						start = 0;
						end = 0;
						valid = false;
					}
					if (valid && start < param_list.GetCount() && start >= 0 && start <= end)
					{
						if (end >= param_list.GetCount())
						{
							end = param_list.GetCount() - 1;
						}
						for (size_t i = start; i <= end; ++i)
						{
							if (i > start)
							{
								output += wxT(' ');
							}
							if (param_list.Item(i).Find(wxT(' ')) > -1)
							{
								output += wxT('"') + param_list.Item(i) + wxT('"');
							}
							else
							{
								output += param_list.Item(i);
							}
						}
					}
					dollar_pos = 0;
					if (!at_end)
					{
						output += c;
					}
				}
			}
			else if (c == wxT('$'))
			{
				dollar_pos = 1;
				num[0] = 0;
				num[1] = 0;
				num_valid[0] = false;
				num_valid[1] = false;
				delim = false;
				tmp.Empty();
			}
			else
			{
				output += c;
			}
		}
		ProcessConsoleInput(context, output);
	}
}

wxArrayString Client::GetAliasList() const
{
	wxArrayString list;
	wxString old_path = m_config->GetPath();
	m_config->SetPath(wxT("Client/Aliases"));
	list.Alloc(m_config->GetNumberOfEntries(false));
	wxString val;
	long i;
	if (m_config->GetFirstEntry(val, i))
	{
		do
		{
			list.Add(val);
		}
		while (m_config->GetNextEntry(val, i));
	}
	m_config->SetPath(old_path);
	return list;
}

wxString Client::GetAlias(const wxString &name) const
{
	return m_config->Read(wxT("Client/Aliases/")+name, wxEmptyString);
}

bool Client::SetAlias(const wxString &name, const wxString &value)
{
	if (value.Length())
	{
		return m_config->Write(wxT("Client/Aliases/")+name, value) && m_config->Flush();
	}
	else if (GetAlias(name).Length())
	{
		return m_config->DeleteEntry(wxT("Client/Aliases/")+name) && m_config->Flush();
	}
	else
	{
		return true;
	}
}
