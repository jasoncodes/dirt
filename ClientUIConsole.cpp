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
		: wxThread(wxTHREAD_JOINABLE)
	{
		Output(wxString()<<"ReadThread()");
		m_console = console;
		m_client = client;
		in = new wxFFileInputStream(stdin);
		cin = new wxTextInputStream(*in);
	}

	virtual ~ReadThread()
	{
		Output(wxString()<<"~ReadThread()");
		delete cin;
		delete in;
	}

	virtual ExitCode Entry()
	{
		Output(wxString()<<"read thread starting");
		while (!in->Eof() && !TestDestroy())
		{
			wxString line(cin->ReadLine());
			if (line.Length() > 0 && !TestDestroy())
			{
				ProcessInput(line);
			}
		}
		Output(wxString()<<"read thread gracefully exiting");
		if (!TestDestroy())
		{
			Output(wxString()<<"sending /exit (for read thread exit)");
			ProcessInput("/exit");
		}
		return 0;
	}

protected:
	void ProcessInput(const wxString &input)
	{
		wxCommandEvent event(wxEVT_COMMAND_TEXT_ENTER, ID_CONSOLE_INPUT);
		event.SetString(input);
		m_console->AddPendingEvent(event);
	}

protected:
	Client *m_client;
	ClientUIConsole *m_console;
	wxFFileInputStream *in;
	wxTextInputStream *cin;

};

BEGIN_EVENT_TABLE(ClientUIConsole, wxEvtHandler)
	EVT_TEXT_ENTER(ID_CONSOLE_INPUT, ClientUIConsole::OnTextEnter)
END_EVENT_TABLE()

ClientUIConsole::ClientUIConsole()
{
	Output(wxString()<<"ClientUIConsole()");
	m_client = new ClientDefault(this);
	m_read_thread = new ReadThread(this, m_client);
	m_read_thread->Create();
	m_read_thread->Run();
}

ClientUIConsole::~ClientUIConsole()
{
	Output(wxString()<<"~ClientUIConsole()");
	delete m_client;
	Output(wxString()<<"shutting down read thread");
	m_read_thread->Delete();
	Output(wxString()<<"read thread shut down");
	delete m_read_thread;
}

void ClientUIConsole::OnTextEnter(wxCommandEvent& event)
{
	wxASSERT(wxThread::IsMain());
	m_client->ProcessInput(wxEmptyString, event.GetString());
}

bool ClientUIConsole::OnClientPreprocess(const wxString &context, wxString &cmd, wxString &params)
{
	if (cmd == "EXIT")
	{
		wxGetApp().ExitMainLoop();
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
