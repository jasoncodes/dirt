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
#include "FileTransfer.h"

DECLARE_APP(DirtApp)

enum
{
	ID_CONSOLE_INPUT = 443
};

static void Output(wxString &line)
{
	puts(Timestamp() + LogControl::ConvertModifiersIntoHtml(line, true));
}

#if 0
	#define DebugOutput(s) Output(s)
#else
	#define DebugOutput(s)
#endif

class ReadThread : public wxThread
{

public:

	ReadThread(ClientUIConsole *console, Client *client)
		: wxThread(wxTHREAD_JOINABLE)
	{
		DebugOutput(wxString()<<"ReadThread()");
		m_console = console;
		m_client = client;
	}

	virtual ~ReadThread()
	{
		DebugOutput(wxString()<<"~ReadThread()");
	}

	virtual ExitCode Entry()
	{

		DebugOutput(wxString()<<"read thread starting");

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

		DebugOutput(wxString()<<"read thread gracefully exiting");

		if (!TestDestroy())
		{
			DebugOutput(wxString()<<"sending /exit (for read thread exit)");
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

	DebugOutput(wxString()<<"ClientUIConsole()");
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

	DebugOutput(wxString()<<"~ClientUIConsole()");
	delete m_client;

	DebugOutput(wxString()<<"shutting down read thread");
	m_read_thread->Delete();
	DebugOutput(wxString()<<"read thread shut down");
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

void ClientUIConsole::OnClientUserList(const wxArrayString &nicklist)
{
	wxString msg = "* Chatting with: ";
	if (nicklist.GetCount() > 0)
	{
		for (size_t i = 0; i < nicklist.GetCount(); ++i)
		{
			if (i > 0)
			{
				msg += ", ";
			}
			msg += nicklist[i];
		}
	}
	else
	{
		msg += "(Nobody)";
	}
	Output(msg);
}

void ClientUIConsole::OnClientUserJoin(const wxString &nick, const wxString &details)
{
	wxString msg;
	msg << "* " << nick;
	if (details.Length() > 0)
	{
		msg << " (" << details << ")";
	}
	msg << " has joined the chat";
	Output(msg);
}

void ClientUIConsole::OnClientUserPart(const wxString &nick, const wxString &details, const wxString &message)
{
	wxString msg;
	msg << "* " << nick;
	if (details.Length() > 0)
	{
		msg << " (" << details << ")";
	}
	msg << " has left the chat";
	if (message.Length() > 0)
	{
		msg << " (" << message << ")";
	}
	Output(msg);
}

void ClientUIConsole::OnClientTransferNew(const FileTransfer &transfer)
{
	// not implemented
}

void ClientUIConsole::OnClientTransferDelete(const FileTransfer &transfer)
{
	// not implemented
}

void ClientUIConsole::OnClientTransferState(const FileTransfer &transfer)
{
	bool bIsError = ((transfer.state == ftsSendFail) || (transfer.state == ftsGetFail));
	if (bIsError)
	{
		OnClientInformation(wxEmptyString, transfer.status);
	}
	else
	{
		OnClientWarning(wxEmptyString, transfer.status);
	}
}

void ClientUIConsole::OnClientTransferTimer(const FileTransfer &transfer)
{
}
