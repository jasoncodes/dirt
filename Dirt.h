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

protected:
	void ProcessCommandLine();

protected:
	Console *m_console;
	wxCmdLineParser *cmdline;
	AppMode m_appmode;
		
};

#endif
