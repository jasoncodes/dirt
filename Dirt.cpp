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

	bool bIsConsole;

	#ifdef __WXMSW__
		// maybe use return val of SHGetFileInfo(.., SHGFI_EXETYPE)
		// or check the subsystem field in IMAGE_OPTIONAL_HEADER
		#ifdef _CONSOLE
			bIsConsole = true;
		#else
			bIsConsole = false;
		#endif
	#else
		// no detection for non-Win32 yet.. defaulting to GUI
		bIsConsole = false;
	#endif




	return bIsConsole;

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
