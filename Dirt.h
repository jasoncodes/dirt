#ifndef Dirt_H_
#define Dirt_H_

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
