#ifndef Dirt_H_
#define Dirt_H_

class Console;

enum AppMode
{
	appDefault,
	appClient,
	appServer
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

protected:
	bool ProcessCommandLine();

protected:
	Console *m_console;
	wxCmdLineParser *m_cmdline;
	AppMode m_appmode;
	wxString m_host;
	bool m_no_input;
	bool m_control_down;
	bool m_alt_down;
	bool m_shift_down;
		
};

#endif
