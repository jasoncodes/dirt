/*
    Copyright 2002, 2003 General Software Laboratories
    
    
    This file is part of Dirt Secure Chat.

    Dirt Secure Chat is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Dirt Secure Chat is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Dirt Secure Chat; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: Console.cpp,v 1.18 2004-05-16 04:42:44 jason Exp $)

#include "Console.h"
#include <stdio.h>
#ifdef __WXMSW__
	#include <windows.h>
	#include <wx/msw/winundef.h>
#endif
#ifdef __WXMAC__
        #include <sys/kernel.h>
        #include <sys/types.h>
        #include <sys/time.h>
        #include <unistd.h>
#endif

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
							wxFAIL_MSG(wxT("Unexpected return value from WaitForSingleObject()"));
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
						wxFAIL_MSG(wxT("Unexpected return value from select()"));
					}
					else if (retval > 0)
					{
						bGotInput = true;
					}

				#endif

			}

			while (!TestDestroy() && (x = wxGetc(stdin)) != -1)
			{

				wxChar c = (wxChar)x;

				if (c == wxT('\n'))
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
		#if defined(__WXMSW__)
			::wxWakeUpMainThread();
		#endif
	}

protected:
	Console *m_console;

	DECLARE_NO_COPY_CLASS(ConsoleReadThread)

};

ConsoleReadThread *g_read_thread;

#ifdef __WXMSW__

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
				g_read_thread->ProcessInput(wxEmptyString);
				return TRUE;

			default:
				// unknown event -- better pass it on.
				return FALSE;

		}

	}

#else

	#include <signal.h>

	bool g_already_done = false;

	void sighandler(int signum)
	{
		signal(signum, sighandler);
		if (!g_already_done)
		{
			g_read_thread->ProcessInput(wxEmptyString);
			g_already_done = true;
		}
	}

	void setsignals()
	{
		signal(SIGHUP, sighandler); // terminal hang-up
		signal(SIGINT, sighandler); // interrupt by user (ctrl-c)
		signal(SIGQUIT, sighandler); // quit by user (ctrl-\)
		signal(SIGTERM, sighandler); // termination request
		//signal(SIGTSTP, sighandler); // terminal stop (ctrl-z)
		//signal(SIGWINCH, sighandler); // terminal resize
	}

#endif

BEGIN_EVENT_TABLE(Console, wxEvtHandler)
	EVT_TEXT_ENTER(ID_CONSOLE_INPUT, Console::OnTextEnter)
END_EVENT_TABLE()

Console::Console(bool no_input)
{

	m_read_thread = new ConsoleReadThread(this);
	if (!no_input)
	{
		m_read_thread->Create();
		m_read_thread->Run();
	}

	g_read_thread = m_read_thread;
	#ifdef __WXMSW__
		SetConsoleCtrlHandler(CtrlHandler, TRUE);
	#else
		setsignals();
	#endif

}

Console::~Console()
{
	wxLogNull supress_log;
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
	wxTheApp->ExitMainLoop();
}

void Console::Output(const wxString &line)
{
	#ifdef __WXMSW__
		wxPuts(line);
	#else
		puts(line.mb_str());
	#endif
	fflush(stdout);
}
