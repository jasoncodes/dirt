#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: Client.cpp,v 1.62 2003-05-11 09:55:11 jason Exp $)

#include "Client.h"
#include "util.h"
#include "Modifiers.h"
#include "FileTransfers.h"
#include "Crypt.h"
#include "ClientTimers.h"
#include "URL.h"

const wxLongLong_t initial_ping_delay = 5000;
const wxLongLong_t ping_interval = 30000;
const wxLongLong_t ping_timeout_delay = 45000;

ClientConfig::ClientConfig()
{
	m_config = new ConfigFile;
}

ClientConfig::~ClientConfig()
{
	delete m_config;
}

wxString ClientConfig::GetLastSendDir() const
{
	return m_config->Read(wxT("Client/Last Send Directory"));
}

wxString ClientConfig::GetLastGetDir() const
{
	return m_config->Read(wxT("Client/Last Get Directory"));
}

bool ClientConfig::SetLastSendDir(const wxString &dir)
{
	return m_config->Write(wxT("Client/Last Send Directory"), dir);
}

bool ClientConfig::SetLastGetDir(const wxString &dir)
{
	return m_config->Write(wxT("Client/Last Get Directory"), dir);
}

enum
{
	ID_TIMER_PING = 100,
	ID_TIMERS
};

BEGIN_EVENT_TABLE(Client, wxEvtHandler)
	EVT_TIMER(ID_TIMER_PING, Client::OnTimerPing)
	EVT_CLIENT_TIMERS(ID_TIMERS, Client::OnClientTimers)
END_EVENT_TABLE()

Client::Client(ClientEventHandler *event_handler)
	: wxEvtHandler(), m_event_handler(event_handler)
{
	m_file_transfers = new FileTransfers(this);
	m_nickname = wxEmptyString;
	m_server_name = wxEmptyString;
	m_tmrPing = new wxTimer(this, ID_TIMER_PING);
	m_timers = new ClientTimers(this, ID_TIMERS);
	m_contact_self = NULL;
}

Client::~Client()
{
	EmptyContacts();
	delete m_timers;
	delete m_tmrPing;
	delete m_file_transfers;
}

void Client::Debug(const wxString &context, const wxString &text)
{
	m_event_handler->OnClientDebug(context, text);
}

wxArrayString Client::GetSupportedCommands() const
{
	wxArrayString cmds;
	WX_APPEND_ARRAY(cmds, SplitString(wxT("ALIAS AWAY BACK BIND CONNECT CTCP DISCONNECT ECHO HELP ME ME'S MSG MSGME MY NICK TIMER TIMERS QUIT RECONNECT SAY SERVER PING WHO WHOIS"), wxT(" ")));
	WX_APPEND_ARRAY(cmds, m_file_transfers->OnClientSupportedCommands());
	WX_APPEND_ARRAY(cmds, m_event_handler->OnClientSupportedCommands());
	cmds.Sort();
	return cmds;
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
	else if (cmd == wxT("ME'S") || cmd == wxT("MY"))
	{
		ProcessConsoleInput(context, wxT("/me 's ") + params);
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
	else if (cmd == wxT("WHO"))
	{
		ASSERT_CONNECTED();
		wxArrayString nicks;
		for (size_t i = 0; i < GetContactCount(); ++i)
		{
			ClientContact *contact = GetContact(i);
			if (!contact->IsSelf())
			{
				nicks.Add(contact->GetNickname());
			}
		}
		nicks.Sort();
		if (!nicks.GetCount())
		{
			nicks.Add(wxT("(Nobody)"));
		}
		m_event_handler->OnClientInformation(
			context, wxT("Chatting with: ") +
			JoinArray(nicks, wxT(", ")));
	}
	else if (cmd == wxT("CTCP"))
	{
		ASSERT_CONNECTED();
		wxString nick, type, data;
		HeadTail ht = SplitQuotedHeadTail(params);
		nick = ht.head;
		ht = SplitQuotedHeadTail(ht.tail);
		type = ht.head;
		data = ht.tail;
		if (type.Upper() == wxT("DCC"))
		{
			m_event_handler->OnClientWarning(context, wxT("DCC is a reserved CTCP type"));
			return;
		}
		CTCP(context, nick, type, data);
	}
	else if (cmd == wxT("PING"))
	{
		ASSERT_CONNECTED();
		HeadTail ht = SplitQuotedHeadTail(params);
		CTCP(context, ht.head, cmd, params);
	}
	else if (cmd == wxT("OPER"))
	{
		ASSERT_CONNECTED();
		Oper(context, params);
	}
	else if (cmd == wxT("QUIT"))
	{
		ASSERT_CONNECTED();
		Quit(params);
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
	else if (cmd == wxT("ECHO"))
	{
		m_event_handler->OnClientInformation(context, params);
	}
	else if (cmd == wxT("HELP"))
	{
		m_event_handler->OnClientInformation(context, wxT("Supported commands: ") + JoinArray(GetSupportedCommands(), wxT(" ")));
	}
	else if (cmd == wxT("LIZARD"))
	{
		m_event_handler->OnClientInformation(context, wxT("Support for Lizard technology is not available at this time"));
	}
	else if (cmd == wxT("TIMER"))
	{

		if (params.Length() == 0)
		{

			m_event_handler->OnClientInformation(context, wxT("Timer help:"));
			m_event_handler->OnClientInformation(context, wxT("    /timers"));
			m_event_handler->OnClientInformation(context, wxT("      Lists active timers"));
			m_event_handler->OnClientInformation(context, wxT("    /timers off"));
			m_event_handler->OnClientInformation(context, wxT("      Halts all timers"));
			m_event_handler->OnClientInformation(context, wxT("    /timer id"));
			m_event_handler->OnClientInformation(context, wxT("      Lists information about timer called id"));
			m_event_handler->OnClientInformation(context, wxT("    /timer id [start] times interval commands"));
			m_event_handler->OnClientInformation(context, wxT("      Sets a timer called id that executes <commands>, <times> times (0 for unlimited), every <interval> seconds, starting at <start> (or immediately if <start> is ommited)"));

		}
		else
		{

			HeadTail ht = SplitQuotedHeadTail(params);
			wxString id = ht.head;
			params = ht.tail;

			if (ht.tail.Length() == 0)
			{

				const ClientTimer *tmr = m_timers->Item(id);

				if (tmr)
				{

					m_event_handler->OnClientInformation(context, wxT("Timer ") + tmr->GetName() + wxT(" information:"));

					wxString msg;
					
					msg.Empty();
					msg << wxT("    Context:     ") << (tmr->GetContext().Length() ? tmr->GetContext() : wxT("(None)"));
					m_event_handler->OnClientInformation(context, msg);

					msg.Empty();
					msg << wxT("    Start Time:  ");
					if (tmr->GetStartTime().IsValid())
					{
						if ((tmr->GetStartTime() - wxDateTime::Now()).Abs() > wxTimeSpan::Day())
						{
							msg << tmr->GetStartTime().Format(wxT("%Y/%m/%d %H:%M:%S"));
						}
						else
						{
							msg << tmr->GetStartTime().Format(wxT("%H:%M:%S"));
						}
					}
					else
					{
						msg << wxT("N/A");
					}
					m_event_handler->OnClientInformation(context, msg);

					msg.Empty();
					msg << wxT("    Interval:    ");
					if (tmr->GetInterval() % 1000)
					{
						msg << AddCommas((wxLongLong_t)(tmr->GetInterval())) << wxT(" msec(s)");
					}
					else
					{
						msg << AddCommas((wxLongLong_t)(tmr->GetInterval()/1000)) << wxT(" sec(s)");
					}
					m_event_handler->OnClientInformation(context, msg);

					msg.Empty();
					msg << wxT("    Next Update: ");
					if (tmr->GetNextTick())
					{
						msg << AddCommas((wxLongLong_t)((tmr->GetNextTick() - GetMillisecondTicks())/1000)) << wxT(" sec(s)");
					}
					else
					{
						msg << wxT("N/A");
					}
					m_event_handler->OnClientInformation(context, msg);

					msg.Empty();
					msg << wxT("    Times Left:  ");
					if (tmr->GetTimesRemaining() >= 0)
					{
						msg << tmr->GetTimesRemaining();
					}
					else
					{
						msg << wxT("Infinite");
					}
					m_event_handler->OnClientInformation(context, msg);

					msg.Empty();
					msg << wxT("    Command(s):  ") << tmr->GetCommands();
					m_event_handler->OnClientInformation(context, msg);

				}
				else
				{
					m_event_handler->OnClientInformation(context, wxT("No such timer: ") + id);
				}

			}
			else if (params.CmpNoCase(wxT("off")) == 0)
			{

				if (!m_timers->Remove(id))
				{
					m_event_handler->OnClientInformation(context, wxT("No such timer: ") + id);
				}

			}
			else
			{

				const wxString usage = wxT("/timer id [start] times interval commands");

				ht = SplitQuotedHeadTail(params);

				wxDateTime start_time = ParseDateTime(ht.head, true);

				if (start_time.IsValid())
				{
					if (start_time < wxDateTime::Now())
					{
						m_event_handler->OnClientWarning(context, usage);
						m_event_handler->OnClientWarning(context, wxString() << wxT("Start time must be in the future"));
						return;
					}
					ht = SplitQuotedHeadTail(ht.tail);
				}

				long times;
				
				if (!ht.head.ToLong(&times) || times < 0)
				{
					m_event_handler->OnClientWarning(context, usage);
					if (ht.head.Length())
					{
						m_event_handler->OnClientWarning(context, wxString() << wxT("\"") << ht.head << wxT("\" is not a valid number of times (or optional start time)"));
					}
					else
					{
						m_event_handler->OnClientWarning(context, wxT("You must specify the number of times"));
					}
					return;
				}

				if (times <= 0)
				{
					times = -1;
				}

				ht = SplitQuotedHeadTail(ht.tail);

				double interval_secs;

				if (!ht.head.ToDouble(&interval_secs) || interval_secs <= 0)
				{
					m_event_handler->OnClientWarning(context, usage);
					if (ht.head.Length())
					{
						m_event_handler->OnClientWarning(context, wxString() << wxT("\"") << ht.head << wxT("\" is not a valid interval"));
					}
					else
					{
						m_event_handler->OnClientWarning(context, wxT("You must specify the interval"));
					}
					return;
				}

				wxLongLong_t interval_msecs = (wxLongLong_t)(interval_secs * 1000);

				wxString commands = ht.tail;

				if (!commands.Length())
				{
					m_event_handler->OnClientWarning(context, usage);
					m_event_handler->OnClientWarning(context, wxString() << wxT("You must specify the list of commands to execute"));
					return;
				}

				if (!m_timers->Add(id, context, commands, start_time, interval_msecs, times))
				{
					m_event_handler->OnClientWarning(context, wxString() << wxT("Error starting timer"));
					return;
				}

			}

		}
	}
	else if (cmd == wxT("TIMERS"))
	{
		if (params.CmpNoCase(wxT("off")) == 0)
		{
			if (m_timers->Clear())
			{
				m_event_handler->OnClientInformation(context, wxT("All timers halted"));
			}
			else
			{
				m_event_handler->OnClientInformation(context, wxT("No active timers"));
			}
		}
		else
		{
			if (m_timers->GetCount())
			{
				m_event_handler->OnClientInformation(context, wxT("Active timers:"));
				for (size_t i = 0; i < m_timers->GetCount(); ++i)
				{
					const ClientTimer *tmr = m_timers->Item(i);
					wxString msg;
					msg << wxT("    ") << tmr->GetName() << wxT(": ");
					if (tmr->GetStartTime().IsValid())
					{
						msg << wxT("Starting ");
						if ((tmr->GetStartTime() - wxDateTime::Now()).Abs() > wxTimeSpan::Day())
						{
							msg << tmr->GetStartTime().Format(wxT("%Y/%m/%d %H:%M:%S"));
						}
						else
						{
							msg << tmr->GetStartTime().Format(wxT("%H:%M:%S"));
						}
						msg << wxT(", ");
					}
					if (tmr->GetInterval() % 1000)
					{
						msg << AddCommas((wxLongLong_t)tmr->GetInterval()) << wxT(" msec(s)");
					}
					else
					{
						msg << AddCommas((wxLongLong_t)(tmr->GetInterval()/1000)) << wxT(" sec(s)");
					}
					if (tmr->GetTimesRemaining() >= 0)
					{
						msg << wxT(", ") << tmr->GetTimesRemaining() << wxT(" time(s) left");
					}
					if (tmr->GetNextTick())
					{
						msg << wxT(", next in ") << AddCommas((wxLongLong_t)(tmr->GetNextTick() - GetMillisecondTicks())) << wxT(" sec(s)");
					}
					msg << wxT(": ") << tmr->GetCommands();
					m_event_handler->OnClientInformation(context, msg);
				}
			}
			else
			{
				m_event_handler->OnClientInformation(context, wxT("No active timers"));
			}
		}
	}
	else if (cmd == wxT("ALIAS") || cmd == wxT("BIND"))
	{
		HeadTail ht = SplitQuotedHeadTail(params);
		bool bIsAlias = (cmd == wxT("ALIAS"));
		if (ht.head.Length())
		{
			int index = bIsAlias ? -1 : GetValidBindNames().Index(ht.head.Upper());
			if (index > -1)
			{
				ht.head = GetValidBindNames().Item(index);
			}
			bool b = bIsAlias ?
				SetAlias(ht.head, ht.tail) :
				((index > -1) && SetBinding(ht.head, ht.tail));
			if (b)
			{
				m_event_handler->OnClientInformation(context, ht.head + wxT(" = ") + (ht.tail.Length()?(wxT('"')+ht.tail+wxT('"')):wxString(wxT("(Nothing)"))));
			}
			else
			{
				m_event_handler->OnClientWarning(context, wxString()<<wxT("Error setting ")<<(bIsAlias?wxT("alias: "):wxT("binding: "))<<ht.head);
			}
		}
		else
		{
			m_event_handler->OnClientInformation(context, wxString()<<wxT("Current ")<<(bIsAlias?wxT("aliases:"):wxT("bindings:")));
			wxArrayString list = bIsAlias?GetAliasList():GetBindingList();
			if (list.GetCount())
			{
				for (size_t i = 0; i < list.GetCount(); ++i)
				{
					m_event_handler->OnClientInformation(context,
						wxString()<<wxT("    ")<<list.Item(i)<<wxT(" = \"")<<
						(bIsAlias?GetAlias(list.Item(i)):GetBinding(list.Item(i)))
						<<wxT('"'));
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
		else if (!m_file_transfers->OnClientPreprocess(context, cmd, params))
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
	else if (cmd == wxT("CTCP") || cmd == wxT("CTCPOK") || cmd == wxT("CTCPREPLY") || cmd == wxT("CTCPREPLYOK"))
	{
		ByteBuffer nick, type, ctcp_data;
		if (!Unpack(data, nick, type, ctcp_data))
		{
			return;
		}
		wxString type_str = ((wxString)type).Upper();
		
		if (cmd == wxT("CTCP"))
		{
			if (!ProcessCTCPIn(context, nick, type_str, ctcp_data))
			{
				m_event_handler->OnClientCTCPIn(context, nick, type_str, ctcp_data);
			}
		}
		else if (cmd == wxT("CTCPOK"))
		{
			if (!ProcessCTCPOut(context, nick, type_str, ctcp_data))
			{
				m_event_handler->OnClientCTCPOut(context, nick, type_str, ctcp_data);
			}
		}
		else if (cmd == wxT("CTCPREPLY"))
		{
			if (!ProcessCTCPReplyIn(context, nick, type_str, ctcp_data))
			{
				m_event_handler->OnClientCTCPReplyIn(context, nick, type_str, ctcp_data);
			}
		}
		else if (cmd == wxT("CTCPREPLYOK"))
		{
			if (!ProcessCTCPReplyOut(context, nick, type_str, ctcp_data))
			{
				m_event_handler->OnClientCTCPReplyOut(context, nick, type_str, ctcp_data);
			}
		}
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
		ByteBufferHashMap WhoIsData = UnpackByteBufferHashMap(data);
		wxString nick = WhoIsData[wxT("NICK")];
		ClientContact *contact = GetContact(nick);
		if (contact)
		{
			contact->m_whois_cache = WhoIsData;
			contact->m_whois_cache_time = GetMillisecondTicks();
		}
		m_event_handler->OnClientWhoIs(context, WhoIsData);
	}
	else if (cmd == wxT("AWAY") || cmd == wxT("BACK"))
	{
		ByteBuffer nick, text;
		if (!Unpack(data, nick, text))
		{
			nick = data;
			text = ByteBuffer();
		}
		bool bIsAway = (cmd == wxT("AWAY"));
		ClientContact *contact = GetContact(nick);
		if (contact)
		{
			contact->m_is_away = bIsAway;
			contact->m_away_message = text;
		}
		if (bIsAway)
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
		ClientContact *contact = new ClientContact(this, nick);
		m_contacts.Add(contact);
		m_event_handler->OnClientUserJoin(nick, details);
		if ((wxString)nick == m_nickname)
		{
			m_event_handler->OnClientStateChange();
			m_contact_self = contact;
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
		ClientContact *contact = GetContact(nick);
		if (contact)
		{
			delete contact;
			m_contacts.Remove(contact);
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
			ClientContact *contact = GetContact(nick1);
			if (contact)
			{
				contact->m_nickname = nick2;
			}
			if ((wxString)nick1 == m_nickname)
			{
				m_nickname = nick2;
				m_event_handler->OnClientStateChange();
			}
			m_file_transfers->OnClientUserNick(nick1, nick2);
			m_event_handler->OnClientUserNick(nick1, nick2);
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
		ClientContact *old_self = m_contact_self;
		m_contacts.Remove(m_contact_self);
		EmptyContacts();
		for (size_t i = 0; i < nickbuff.GetCount(); ++i)
		{
			wxString nick = nickbuff.Item(i);
			nicks.Add(nick);
			ClientContact *contact = new ClientContact(this, nick);
			m_contacts.Add(contact);
			if ((wxString)nick == m_nickname)
			{
				delete old_self;
				old_self = NULL;
				m_contact_self = contact;
			}
		}
		if (old_self)
		{
			m_contacts.Insert(old_self, 0);
			m_contact_self = old_self;
		}
		m_event_handler->OnClientUserList(nicks);
	}
	else if (cmd == wxT("IPLIST"))
	{
		m_server_ip_list = ByteBufferArrayToArrayString(Unpack(data));
	}
	else if (cmd == wxT("IPSELF"))
	{
		m_ipself = data;
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

void Client::Quit(const wxString &msg)
{
	wxString context = wxEmptyString;
	ASSERT_CONNECTED();
	SendToServer(EncodeMessage(context, wxT("QUIT"), msg));
}

void Client::Away(const wxString &msg)
{
	wxString context = wxEmptyString;
	ASSERT_CONNECTED();
	SendToServer(EncodeMessage(context, msg.Length()?wxT("AWAY"):wxT("BACK"), msg));
}

void Client::CTCP(const wxString &context, const wxString &nick, const wxString &type, const ByteBuffer &data)
{
	ASSERT_CONNECTED();
	ByteBuffer data2(data);
	if (type.Upper() == wxT("PING"))
	{
		data2 = Uint64ToBytes(GetMillisecondTicks());
	}
	SendToServer(EncodeMessage(context, wxT("CTCP"), Pack(nick, type.Upper(), data2)));
}

void Client::CTCPReply(const wxString &context, const wxString &nick, const wxString &type, const ByteBuffer &data)
{
	ASSERT_CONNECTED();
	SendToServer(EncodeMessage(context, wxT("CTCPREPLY"), Pack(nick, type.Upper(), data)));
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
	if (nick == nick.Upper() || nick == nick.Lower())
	{
		nick = nick.Lower();
		nick[0u] = wxToupper(nick[0u]);
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
		while (tokens.Length() && tokens[0u] == wxT('|'))
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
	wxString old_path = m_config.GetConfig()->GetPath();
	m_config.GetConfig()->SetPath(wxT("Client/Aliases"));
	list.Alloc(m_config.GetConfig()->GetNumberOfEntries(false));
	wxString val;
	long i;
	if (m_config.GetConfig()->GetFirstEntry(val, i))
	{
		do
		{
			list.Add(val);
		}
		while (m_config.GetConfig()->GetNextEntry(val, i));
	}
	m_config.GetConfig()->SetPath(old_path);
	return list;
}

wxString Client::GetAlias(const wxString &name) const
{
	return m_config.GetConfig()->Read(wxT("Client/Aliases/")+name, wxEmptyString);
}

bool Client::SetAlias(const wxString &name, const wxString &value)
{
	if (value.Length())
	{
		return m_config.GetConfig()->Write(wxT("Client/Aliases/")+name, value) && m_config.GetConfig()->Flush();
	}
	else if (GetAlias(name).Length())
	{
		return m_config.GetConfig()->DeleteEntry(wxT("Client/Aliases/")+name) && m_config.GetConfig()->Flush();
	}
	else
	{
		return true;
	}
}

wxArrayString Client::GetBindingList() const
{
	wxArrayString list;
	wxString old_path = m_config.GetConfig()->GetPath();
	m_config.GetConfig()->SetPath(wxT("Client/Bindings"));
	list.Alloc(m_config.GetConfig()->GetNumberOfEntries(false));
	wxString val;
	long i;
	if (m_config.GetConfig()->GetFirstEntry(val, i))
	{
		do
		{
			list.Add(val);
		}
		while (m_config.GetConfig()->GetNextEntry(val, i));
	}
	m_config.GetConfig()->SetPath(old_path);
	return list;
}

wxArrayString Client::GetValidBindNames() const
{
	static wxArrayString names;
	if (!names.GetCount())
	{
		WX_APPEND_ARRAY(names, SplitString(wxT(""), wxT(" ")));
		for (size_t i = 1; i <= 12; ++i)
		{
			names.Add(wxString() << wxT('F') << i);
		}
		names.Sort();
	}
	return names;
}

wxString Client::GetBinding(const wxString &name) const
{
	return m_config.GetConfig()->Read(wxT("Client/Bindings/")+name, wxEmptyString);
}

bool Client::SetBinding(const wxString &name, const wxString &value)
{
	if (value.Length())
	{
		return m_config.GetConfig()->Write(wxT("Client/Bindings/")+name, value) && m_config.GetConfig()->Flush();
	}
	else if (GetBinding(name).Length())
	{
		return m_config.GetConfig()->DeleteEntry(wxT("Client/Bindings/")+name) && m_config.GetConfig()->Flush();
	}
	else
	{
		return true;
	}
}

void Client::OnClientTimers(ClientTimersEvent &event)
{
	const ClientTimer &timer = event.GetTimer();
	ProcessAlias(timer.GetContext(), timer.GetCommands(), wxEmptyString);
}

bool Client::ProcessCTCPIn(const wxString &context, const wxString &nick, wxString &type, ByteBuffer &data)
{
	if (type == wxT("PING"))
	{
		CTCPReply(context, nick, type, data);
		data = ByteBuffer();
	}
	return m_file_transfers->OnClientCTCPIn(context, nick, type, data);
}

bool Client::ProcessCTCPOut(const wxString &context, const wxString &nick, wxString &type, ByteBuffer &data)
{
	if (type == wxT("PING"))
	{
		data = ByteBuffer();
	}
	return m_file_transfers->OnClientCTCPOut(context, nick, type, data);
}

bool Client::ProcessCTCPReplyIn(const wxString &context, const wxString &nick, wxString &type, ByteBuffer &data)
{
	if (type == wxT("PING") && data.Length() == sizeof(wxUint64))
	{
		unsigned wxLongLong_t num = BytesToUint64(data.LockRead(), data.Length());
		data.Unlock();
		long latency = (long)(GetMillisecondTicks() - num);
		data = ByteBuffer(SecondsToMMSS(latency, true, true)) + ByteBuffer(1) + data;
	}
	return m_file_transfers->OnClientCTCPReplyIn(context, nick, type, data);
}

bool Client::ProcessCTCPReplyOut(const wxString &context, const wxString &nick, wxString &type, ByteBuffer &data)
{
	return m_file_transfers->OnClientCTCPReplyOut(context, nick, type, data);
}

size_t Client::GetContactCount() const
{
	return m_contacts.GetCount();
}

ClientContact* Client::GetContact(size_t index) const
{
	if (index >= 0 && index < m_contacts.GetCount())
	{
		return m_contacts.Item(index);
	}
	else
	{
		return NULL;
	}
}

ClientContact* Client::GetContact(const wxString &nick) const
{
	for (size_t i = 0; i < m_contacts.GetCount(); ++i)
	{
		ClientContact *contact = m_contacts.Item(i);
		if (contact->GetNickname().CmpNoCase(nick) == 0)
		{
			return contact;
		}
	}
	return NULL;
}

void Client::EmptyContacts()
{
	m_contact_self = NULL;
	for (size_t i = 0; i < m_contacts.GetCount(); ++i)
	{
		delete m_contacts.Item(i);
	}
	m_contacts.Empty();
}
