#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif

#include "Dirt.h"
#include "ClientUIMDIFrame.h"

IMPLEMENT_APP(TestApp)

bool TestApp::OnInit()
{
	ClientUIMDIFrame *frame = new ClientUIMDIFrame();
	SetTopWindow(frame);
	return TRUE;
}
