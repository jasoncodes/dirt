#ifndef _DirtApp_H
#define _DirtApp_H

class ClientUIConsole;

class DirtApp : public wxApp
{

public:
	virtual bool OnInit();
	virtual int OnExit();

	bool IsConsole();

protected:
	ClientUIConsole *m_console;

};

#endif
