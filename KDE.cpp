#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: KDE.cpp,v 1.3 2003-05-27 17:02:20 jason Exp $)

#ifdef KDE_AVAILABLE

#include "KDE.h"

#include <kapplication.h>
#include <wx/thread.h>
#include <dcopclient.h>
#include <wx/filename.h>
#include <kcrash.h>

bool DoesDCOPFileExist()
{
	return wxFileName(wxString((const char*)DCOPClient::dcopServerFile(), wxConvLocal)).FileExists();
}

KDEThread::KDEThread()
	: wxThread(wxTHREAD_DETACHED)
{
	Create();
	Run();
}

void KDEThread::Init()
{
	s_section.Enter();
	if (!s_thread)
	{
		s_mutex.Lock();
		s_thread = new KDEThread;
		s_cond.Wait();
	}
	s_section.Leave();
}

void KDEThread::CleanUp()
{
	s_section.Enter();
	if (s_thread)
	{
		if (s_thread->m_kapp)
		{
			s_thread->m_kapp->exit();
			s_thread->m_kapp = NULL;
		}
		if (This() != s_thread)
		{
			s_thread->Delete();
			delete s_thread;
			s_thread = NULL;
		}
	}
	s_section.Leave();
}

static void CrashHandler(int)
{
	wxExit();
}

wxThread::ExitCode KDEThread::Entry()
{

	int disp_argc = 1;
	char *disp_argv[2];
	disp_argv[0] = "";
	disp_argv[1] = NULL;
	m_kapp = new KApplication(disp_argc, disp_argv, "KDE Test");
	KCrash::setCrashHandler(CrashHandler);	

	s_mutex.Lock();
	s_cond.Broadcast();
	s_mutex.Unlock();

	ExitCode retval = (ExitCode)m_kapp->exec();
	CleanUp();
	return retval;

}

KDEThread* KDEThread::s_thread = NULL;
wxCriticalSection KDEThread::s_section;
wxMutex KDEThread::s_mutex;
wxCondition KDEThread::s_cond(KDEThread::s_mutex);

class KDEModule: public wxModule
{

	DECLARE_DYNAMIC_CLASS(KDEModule)

public:
	KDEModule() : wxModule() {}
	bool OnInit()
	{
		return true;
	}

	void OnExit()
	{
		KDEThread::CleanUp();
	}

};
IMPLEMENT_DYNAMIC_CLASS(KDEModule, wxModule)

#endif
