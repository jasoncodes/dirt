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

IMPLEMENT_APP(TestApp)

static bool IsConsole()
{
	#ifdef __WXMSW__
		#ifdef _CONSOLE
			return true;
		#else
			return false;
		#endif
	#else
		wxFAIL_MSG("Your platform is not supported yet :)");
	#endif
}

bool TestApp::OnInit()
{
	if (IsConsole())
	{
		new ClientUIConsole;
	}
	else
	{
		ClientUIMDIFrame *frame = new ClientUIMDIFrame();
		SetTopWindow(frame);
	}
	return true;
}
