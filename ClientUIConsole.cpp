#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif

#include "ClientUIConsole.h"
#include "LogControl.h"
#include "ClientDefault.h"
#include <stdio.h>
#include "util.h"
#include <wx/wfstream.h>
#include <wx/txtstrm.h>
#include "Dirt.h"

DECLARE_APP(DirtApp)

enum
{
	ID_CONSOLE_INPUT = 443
};

static void Output(wxString &line)
{
	puts(Timestamp() + LogControl::ConvertModifiersIntoHtml(line, true));
}

class ReadThread : public wxThread
{

public:

	ReadThread(ClientUIConsole *console, Client *client)
		: wxThread()
	{
		m_console = console;
		m_client = client;
	}

	virtual ExitCode Entry()
	{
		wxFFileInputStream in(stdin);
		wxTextInputStream cin(in);
		while (!in.Eof())
		{
			wxString line(cin.ReadLine());
			if (line.Length() > 0)
			{
				ProcessInput(line);
			}
		}
		ProcessInput("/exit");
		return 0;
	}

protected:
	void ProcessInput(const wxString &input)
	{
		wxCommandEvent event(wxEVT_COMMAND_TEXT_ENTER, ID_CONSOLE_INPUT);
		event.SetString(input);
		m_console->ProcessEvent(event);
	}

protected:
	Client *m_client;
	ClientUIConsole *m_console;

};

BEGIN_EVENT_TABLE(ClientUIConsole, wxEvtHandler)
	EVT_TEXT_ENTER(ID_CONSOLE_INPUT, ClientUIConsole::OnTextEnter)
END_EVENT_TABLE()

ClientUIConsole::ClientUIConsole()
{
	m_client = new ClientDefault(this);
	read_thread = new ReadThread(this, m_client);
	read_thread->Create();
	read_thread->Run();
}

ClientUIConsole::~ClientUIConsole()
{
	delete m_client;
}

void ClientUIConsole::OnTextEnter(wxCommandEvent& event)
{
	m_client->ProcessInput(wxEmptyString, event.GetString());
}

bool ClientUIConsole::OnClientPreprocess(const wxString &context, wxString &cmd, wxString &params)
{
	if (cmd == "EXIT")
	{
		wxGetApp().ExitMainLoop(); // this no working :(
		return true;
	}
	else
	{
		return false;
	}
}

/*void DirtConsole::MainLoop()
{
	StillGoing = true;
	while (StillGoing)
	{
		app->ProcessPendingEvents();
	}
}

void DirtConsole::ExitMainLoop()
{
	StillGoing = false;
	#if defined(__WXMSW__)
		wxWakeUpMainThread();
	#endif
}*/

void ClientUIConsole::OnClientDebug(const wxString &context, const wxString &text)
{
	Output(wxString() << "Debug: Context = \"" + context + "\", Text = \"" + text + "\"");
}

void ClientUIConsole::OnClientWarning(const wxString &context, const wxString &text)
{
	Output(wxString() << "Warning: Context = \"" + context + "\", Text = \"" + text + "\"");
}

void ClientUIConsole::OnClientInformation(const wxString &context, const wxString &text)
{
	Output(wxString() << "Information: Context = \"" + context + "\", Text = \"" + text + "\"");
}

void ClientUIConsole::OnClientMessageOut(const wxString &nick, const wxString &text)
{
	bool is_private = (nick.Length() > 0);
	if (is_private)
	{
		Output(wxString() << "-> *" + nick + "* " + text);
	}
	else
	{
		Output(wxString() << "<" + m_client->GetNickname() + "> " + text);
	}
}

void ClientUIConsole::OnClientMessageIn(const wxString &nick, const wxString &text, bool is_private)
{
	if (is_private)
	{
		Output(wxString() << "*" + nick + "* " + text);
	}
	else
	{
		Output(wxString() << "<" + nick + "> " + text);
	}
}
