#ifndef KDE_H_
#define KDE_H_

#ifdef KDE_AVAILABLE

class KApplication;

bool DoesDCOPFileExist();

class KDEThread : public wxThread
{

protected:
	KDEThread();

public:
	static void Init();
	static void CleanUp();

protected:
	virtual ExitCode Entry();

protected:
	static KDEThread *s_thread;
	KApplication *m_kapp;
	static wxCriticalSection s_section;
	static wxMutex s_mutex;
	static wxCondition s_cond;

};

#endif

#endif
