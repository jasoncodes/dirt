#ifndef Dirt_H_
#define Dirt_H_

#ifdef __WXMSW__
	#include <windows.h>
	#include <wx/msw/winundef.h>
#endif

class Console;

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
	virtual bool OnInit();
	virtual int OnExit();
	virtual int FilterEvent(wxEvent& event);

	bool IsConsole();
	void RegisterDirtProtocol();

	bool IsControlDown() const { return m_control_down; }
	bool IsAltDown() const { return m_alt_down; }
	bool IsShiftDown() const { return m_shift_down; }

	wxString GetConfigFilename() { return m_config_filename; }
	wxString GetDefaultQuitMessage() { return m_default_quit_message; }

protected:
	bool ProcessCommandLine();

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

	#ifdef __WXMSW__
		static LRESULT CALLBACK MsgHookProc(int nCode, WPARAM wParam, LPARAM lParam);
		static HHOOK s_hMsgHookProc;
	#endif

};

#endif
