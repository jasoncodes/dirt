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


#ifndef Dirt_H_
#define Dirt_H_

#ifdef __WXMSW__
	#include <windows.h>
	#include <wx/msw/winundef.h>
#endif

#include <wx/file.h>

class Console;
class Client;

enum AppMode
{
	appDefault,
	appClient,
	appServer,
	appLog,
	appLanList
};

class DirtApp : public wxApp
{

public:
	DirtApp();

	virtual bool OnInit();
	virtual int OnExit();
	virtual int FilterEvent(wxEvent& event);

	bool IsConsole();
	void RegisterDirtProtocol();

	bool IsControlDown() const;
	bool IsAltDown() const;
	bool IsShiftDown() const;

	wxString GetConfigFilename() { return m_config_filename; }
	wxString GetDefaultQuitMessage() { return m_default_quit_message; }

	void SetClient(Client *client) { wxASSERT(!m_client && client); m_client = client; }

protected:
	bool ProcessCommandLine();

protected:
	void OnIdle(wxIdleEvent &idle);

protected:
	Console *m_console;
	wxCmdLineParser *m_cmdline;
	AppMode m_appmode;
	wxString m_host;
	wxString m_logfile;
	bool m_no_input;
	bool m_control_down;
	bool m_alt_down;
	bool m_shift_down;
	wxString m_config_filename;
	wxString m_default_quit_message;
	Client *m_client;
	wxString m_pid_filename;
	wxFile m_pid_file;

	#ifdef __WXMSW__
		static LRESULT CALLBACK MsgHookProc(int nCode, WPARAM wParam, LPARAM lParam);
		static HHOOK s_hMsgHookProc;
	#endif

private:
	DECLARE_EVENT_TABLE()
	DECLARE_NO_COPY_CLASS(DirtApp)

};

#endif
