#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif

#include "Dirt.h"
#include "ClientUIMDIFrame.h"
#include "ClientUIConsole.h"

IMPLEMENT_APP(DirtApp)

static bool IsConsole()
{
	#ifdef __WXMSW__
		#ifdef _CONSOLE
			return true;
		#else
			return false;
		#endif
	#else
		// no detection for non-Win32 yet.. defaulting to GUI
		return false;
	#endif
}

bool DirtApp::OnInit()
{
	if (IsConsole())
	{
		m_console = new ClientUIConsole();
	}
	else
	{
		ClientUIMDIFrame *frame = new ClientUIMDIFrame();
		SetTopWindow(frame);
	}
	return true;
}

int DirtApp::OnExit()
{
	if (IsConsole())
	{
		delete m_console;
	}
	return wxApp::OnExit();
}
