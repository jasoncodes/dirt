#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: Client.cpp,v 1.15 2003-02-17 03:25:58 jason Exp $)

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
}

Client::~Client()
{
	delete m_file_transfers;
}

void Client::Debug(const wxString &context, const wxString &text)
{
	m_event_handler->OnClientDebug(context, text);
}

void Client::ProcessInput(const wxString &context, const wxString &input)
{
	
	wxString cmd, params;
	
	if (input[0] == wxT('/'))
	{
		SplitHeadTail(input, cmd, params);
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
		SendMessage(context, params);
	}
	else if (cmd == wxT("MSG"))
	{
		wxString nick, msg;
		SplitHeadTail(params, nick, msg);
		if (nick.Length() == 0)
		{
			m_event_handler->OnClientInformation(context, wxT("/msg: insufficient parameters"));
		}
		else if (msg.Length() > 0)
		{
			SendMessage(nick, msg);
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
		Disconnect();
	}
	/*else if (cmd == wxT("NICK"))
	{
		if (params.Length() == 0)
		{
			Output(wxT("*** Your nickname is ") + nickname);
		}
		else
		{
			if (socket->IsConnected())
			{
				SendData(wxT("*** ") + nickname + wxT(" is now known as ") + params);
			}
			else
			{
				Output(wxT("*** Your nickname is now ") + params);
			}
			nickname = params;
		}
	}*/
	else if (cmd == wxT("HELP"))
	{
		m_event_handler->OnClientInformation(context, wxT("Supported commands: CONNECT DISCONNECT HELP MSG SAY SERVER"));
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
