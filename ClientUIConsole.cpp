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

#include <io.h>

class ReadThread : public wxThread
{

public:

	ReadThread(ClientUIConsole *console, Client *client)
		: wxThread(wxTHREAD_JOINABLE)
	{
		Output(wxString()<<"ReadThread()");
		m_console = console;
		m_client = client;
	}

	virtual ~ReadThread()
	{
		Output(wxString()<<"~ReadThread()");
	}

	virtual ExitCode Entry()
	{

		Output(wxString()<<"read thread starting");

		int x = 0;

		wxString line;
		line.Alloc(128);

		while (!TestDestroy() && x != -1)
		{
			
			line.Empty();

			bool bGotInput = false;

			while (!TestDestroy() && !bGotInput)
			{

				#ifdef __WXMSW__

					HANDLE handle = ::GetStdHandle(STD_INPUT_HANDLE);

					switch (::WaitForSingleObject(handle, 100))
					{

						case WAIT_OBJECT_0:
							bGotInput = true;
							break;

						case WAIT_TIMEOUT:
							break;

						case WAIT_ABANDONED:
						case WAIT_FAILED:
						default:
							wxFAIL_MSG("Unexpected return value from WaitForSingleObject()");
							break;
					}

				#else

					// The following is UNTESTED CODE!

					fd_set rfds;
					struct timeval tv;
					int retval;

					/* Watch stdin (fd 0) to see when it has input. */
					FD_ZERO(&rfds);
					FD_SET(0, &rfds);
					/* Wait up to five seconds. */
					tv.tv_sec = 1;
					tv.tv_usec = 0;

					retval = select(1, &rfds, NULL, NULL, &tv);
					/* Don't rely on the value of tv now! */

					if (retval < 0)
					{
					printf("error!");
					}
					else if (retval > 0)
					{
					printf("Data is available now.\n");
					bGotInput = true;
					/* FD_ISSET(0, &rfds) will be true. */
					}
					else
					{
					printf("No data within 1 second.\n");
					}

				#endif

			}

			while (!TestDestroy() && (x = getc(stdin)) != -1)
			{

				char c = (char)x;

				if (c == '\n')
				{
					break;
				}
				else
				{
					line += c;
				}

			}

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
	else if (cmd == "HELP")
	{
		OnClientInformation(context, "Supported commands: EXIT");
		return false;
	}
	else if (cmd == "TEST")
	{
		OnClientDebug(context, "Screwing w/ stdin...");
		fputs("\nblahblah\n",stdin);
		return true;
	}
	else
	{
		return false;
	}
}

void ClientUIConsole::OnClientDebug(const wxString &context, const wxString &text)
{
	Output(wxString() << "Debug: " << text);
}

void ClientUIConsole::OnClientWarning(const wxString &context, const wxString &text)
{
	Output(wxString() << "* " << text);
}

void ClientUIConsole::OnClientInformation(const wxString &context, const wxString &text)
{
	Output(wxString() << "* " << text);
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
