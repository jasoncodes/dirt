#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: Splash.cpp,v 1.2 2003-02-13 16:20:43 jason Exp $)

#include "Splash.h"
#include "ClientUIMDIFrame.h"

#include "res/dirt.xpm"
#include "res/splash.h"

Splash::Splash()
	: wxFrame(NULL, -1, "Dirt Secure Chat" + GetProductVersion(), wxDefaultPosition, wxDefaultSize)
{
	SetIcon(wxIcon( dirt_xpm ));
	Show();
	// new ClientUIMDIFrame;
}

Splash::~Splash()
{
}
