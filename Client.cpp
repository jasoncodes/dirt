#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: Client.cpp,v 1.10 2003-02-13 13:16:49 jason Exp $)

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

void Client::ProcessInput(const wxString &context, const wxString &text)
{
	
	wxString cmd, params;
	
	if (text[0] == '/')
	{
		SplitHeadTail(text, cmd, params);
		cmd = cmd.Mid(1);
		cmd.MakeUpper();
	}
	else
	{
		cmd = "SAY";
		params = text;
	}
	cmd.Trim(true);
	cmd.Trim(false);
	if (cmd != "SAY")
	{
		params.Trim(true);
		params.Trim(false);
	}

	//Debug(context, wxString() << "Command: \"" << cmd << "\", Params: \"" << params << (char)OriginalModifier << "\"");

	if (m_event_handler->OnClientPreprocess(context, cmd, params))
	{
		return;
	}

	if (cmd == "SAY")
	{
		SendMessage(context, params);
	}
	else if (cmd == "MSG")
	{
		wxString nick, msg;
		SplitHeadTail(params, nick, msg);
		if (nick.Length() == 0)
		{
			m_event_handler->OnClientInformation(context, "/msg: insufficient parameters");
		}
		else if (msg.Length() > 0)
		{
			SendMessage(nick, msg);
		}
	}
	/*else if (cmd == "CONNECT")
	{
		if (!CheckNick()) return;
		wxArrayString paramarray = SplitString(params, " ");
		if (paramarray.Count() != 2)
		{
			Output("*** You need to specify a destination hostname/ip and port");
			Output(wxString("DEBUG: ") << "You specified " << paramarray.Count() << " params");
			return;
		}
		wxIPV4address addr;
		addr.Hostname(paramarray.Item(0));
		addr.Service(paramarray.Item(1));
		wxString output;
		output << "*** Connecting to " << addr.Hostname() << ":" << addr.Service() << "...";
		Output(output);
		connecting = true;
		socket->Connect(addr, false);
	}
	else if (cmd == "DISCONNECT")
	{
		socket->Close();
	}
	else if (cmd == "NICK")
	{
		if (params.Length() == 0)
		{
			Output("*** Your nickname is " + nickname);
		}
		else
		{
			if (socket->IsConnected())
			{
				SendData("*** " + nickname + " is now known as " + params);
			}
			else
			{
				Output("*** Your nickname is now " + params);
			}
			nickname = params;
		}
	}*/
	else if (cmd == "HELP")
	{
		m_event_handler->OnClientInformation(context, "Supported commands: SAY HELP MSG");
	}
	else if (cmd == "LIZARD")
	{
		m_event_handler->OnClientInformation(context, "Support for Lizard technology is not available at this time");
	}
	else
	{
		m_event_handler->OnClientWarning(context, "Unrecognized command: " + cmd);
	}

}
