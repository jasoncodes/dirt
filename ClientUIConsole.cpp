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

	void ProcessInput(const wxString &input)
	{
		wxCommandEvent event(wxEVT_COMMAND_TEXT_ENTER, ID_CONSOLE_INPUT);
		event.SetString(input);
		m_console->AddPendingEvent(event);
		#if defined(__WXMSW__) || defined(__WXMAC__)
			wxWakeUpMainThread();
		#endif
	}

protected:
	Client *m_client;
	ClientUIConsole *m_console;
	wxFFileInputStream *in;
	wxTextInputStream *cin;

};

#ifdef __WXMSW__

	ReadThread *ctrl_handler_read_thread;

	BOOL WINAPI CtrlHandler (DWORD dwEvent)
	{

		switch (dwEvent)
		{

			case CTRL_C_EVENT:
			case CTRL_BREAK_EVENT:
			case CTRL_LOGOFF_EVENT:
			case CTRL_SHUTDOWN_EVENT:
			case CTRL_CLOSE_EVENT:
				// handle all known events
				ctrl_handler_read_thread->ProcessInput("/exit");
				return TRUE;

			default:
				// unknown event -- better pass it on.
				return FALSE;

		}

	}

#endif

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
	#ifdef __WXMSW__
		ctrl_handler_read_thread = m_read_thread;
		SetConsoleCtrlHandler(CtrlHandler, TRUE);
	#endif
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
