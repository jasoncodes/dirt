#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: Client.cpp,v 1.24 2003-02-21 04:40:37 jason Exp $)

#include "Client.h"
#include "util.h"
#include "Modifiers.h"
#include "FileTransfers.h"

BEGIN_EVENT_TABLE(Client, wxEvtHandler)
END_EVENT_TABLE()

Client::Client(ClientEventHandler *event_handler)
	: wxEvtHandler(), m_event_handler(event_handler)
{
	m_file_transfers = new FileTransfers(this);
	m_nickname = wxEmptyString;
}

Client::~Client()
{
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

	if (cmd == wxT("SAY"))
	{
		ASSERT_CONNECTED();
		SendMessage(context, context, params);
	}
	else if (cmd == wxT("MSG"))
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
			SendMessage(context, nick, msg);
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
	else if (cmd == wxT("HELP"))
	{
		m_event_handler->OnClientInformation(context, wxT("Supported commands: CONNECT DISCONNECT HELP MSG NICK RECONNECT SAY SERVER"));
	}
	else if (cmd == wxT("LIZARD"))
	{
		m_event_handler->OnClientInformation(context, wxT("Support for Lizard technology is not available at this time"));
	}
	else
	{
		m_event_handler->OnClientWarning(context, wxT("Unrecognized command: ") + cmd);
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

	if (cmd == wxT("PUBMSG"))
	{
		ByteBuffer nick, text;
		if (!Unpack(data, nick, text))
		{
			nick = data;
			text = ByteBuffer();
		}
		m_event_handler->OnClientMessageIn(nick, text, false);
	}
	else if (cmd == wxT("PRIVMSG"))
	{
		ByteBuffer nick, text;
		if (!Unpack(data, nick, text))
		{
			nick = data;
			text = ByteBuffer();
		}
		m_event_handler->OnClientMessageIn(nick, text, true);
	}
	else if (cmd == wxT("PRIVMSGOK"))
	{
		ByteBuffer nick, text;
		if (!Unpack(data, nick, text))
		{
			nick = data;
			text = ByteBuffer();
		}
		m_event_handler->OnClientMessageOut(nick, text);
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

void Client::OnConnect()
{
	m_event_handler->OnClientInformation(wxEmptyString, wxT("Connected"));
	m_event_handler->OnClientStateChange();
	SendToServer(EncodeMessage(wxEmptyString, wxT("USERAGENT"), GetProductVersion() + wxT(' ') + GetRCSDate()));
	wxString userdetails;
	userdetails << ::wxGetUserId() << wxT('@') << ::wxGetHostName();
	userdetails << wxT(" (\"") << ::wxGetUserName() << wxT("\")");
	userdetails << wxT(" on ") << ::wxGetOsDescription();
	SendToServer(EncodeMessage(wxEmptyString, wxT("USERDETAILS"), userdetails));
}

wxString Client::GetNickname()
{
	return m_nickname;
}

wxString Client::GetDefaultNick()
{
	wxString nick = ::wxGetUserId();
	int i = nick.Index(wxT(' '));
	if (i > -1)
	{
		nick = nick.Left(i);
	}
	return nick;
}

