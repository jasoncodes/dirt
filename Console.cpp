#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif

#include "Console.h"
#include <stdio.h>

#include "Dirt.h"
DECLARE_APP(DirtApp)

enum
{
	ID_CONSOLE_INPUT = 12542
};

class ConsoleReadThread : public wxThread
{

public:

	ConsoleReadThread(Console *console)
		: wxThread(wxTHREAD_JOINABLE)
	{
		m_console = console;
	}

	virtual ~ConsoleReadThread()
	{
	}

	virtual ExitCode Entry()
	{

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

					switch (::WaitForSingleObject(handle, 250))
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

					fd_set rfds;
					struct timeval tv;
					int retval;

					FD_ZERO(&rfds);
					FD_SET(0, &rfds); // file descriptor 0 is stdin

					tv.tv_sec = 0;
					tv.tv_usec = 1000 * 500; // 500 ms

					retval = select(1, &rfds, NULL, NULL, &tv);

					if (retval < 0)
					{
						wxFAIL_MSG("Unexpected return value from select()");
					}
					else if (retval > 0)
					{
						bGotInput = true;
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

		if (!TestDestroy())
		{
			ProcessInput(wxEmptyString);
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
	Console *m_console;

};

#ifdef __WXMSW__

	ConsoleReadThread *ctrl_handler_read_thread;

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
				ctrl_handler_read_thread->ProcessInput(wxEmptyString);
				return TRUE;

			default:
				// unknown event -- better pass it on.
				return FALSE;

		}

	}

#endif

BEGIN_EVENT_TABLE(Console, wxEvtHandler)
	EVT_TEXT_ENTER(ID_CONSOLE_INPUT, Console::OnTextEnter)
END_EVENT_TABLE()

Console::Console()
{

	m_read_thread = new ConsoleReadThread(this);
	m_read_thread->Create();
	m_read_thread->Run();

	#ifdef __WXMSW__
		ctrl_handler_read_thread = m_read_thread;
		SetConsoleCtrlHandler(CtrlHandler, TRUE);
	#endif

}

Console::~Console()
{
	m_read_thread->Delete();
	delete m_read_thread;
}

void Console::OnTextEnter(wxCommandEvent& event)
{
	wxASSERT(wxThread::IsMain());
	wxString line(event.GetString());
	if (line.Length() > 0)
	{
		OnInput(line);
	}
	else
	{
		OnEOF();
	}
}

void Console::ExitMainLoop()
{
	wxGetApp().ExitMainLoop();
}

void Console::Output(const wxString &line)
{
	puts(line);
	fflush(stdout);
}

