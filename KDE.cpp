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


#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: KDE.cpp,v 1.5 2004-05-16 04:42:46 jason Exp $)

#ifdef KDE_AVAILABLE

#include "KDE.h"

#include <kapplication.h>
#include <wx/thread.h>
#include <wx/module.h>
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
