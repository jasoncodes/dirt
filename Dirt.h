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

	bool IsConsole();
	void RegisterDirtProtocol();

protected:
	bool ProcessCommandLine();

protected:
	Console *m_console;
	wxCmdLineParser *m_cmdline;
	AppMode m_appmode;
	wxString m_host;
	bool m_no_input;
		
};

#endif
