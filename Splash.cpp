#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: Splash.cpp,v 1.4 2003-02-13 17:29:17 jason Exp $)

#include "Splash.h"
#include "ClientUIMDIFrame.h"
#include "wx/image.h"
#include "wx/mstream.h"

#include "res/dirt.xpm"
#include "res/splash.h"

enum
{
	ID_CLIENT = 1,
	ID_SERVER
};

BEGIN_EVENT_TABLE(Splash, wxFrame)
	EVT_ERASE_BACKGROUND(Splash::OnErase)
	EVT_PAINT(Splash::OnPaint)
	EVT_BUTTON(ID_CLIENT, Splash::OnClient)
END_EVENT_TABLE()

Splash::Splash()
	: wxFrame(NULL, -1, "Dirt Secure Chat" + GetProductVersion(), wxDefaultPosition, wxDefaultSize, wxCAPTION | wxCLIP_CHILDREN | wxMINIMIZE_BOX | wxSYSTEM_MENU)
{
	SetIcon(wxIcon( dirt_xpm ));
	wxButton *cmdClient = new wxButton(this, ID_CLIENT, "&Client", wxPoint(32, 140));
	wxButton *cmdServer = new wxButton(this, ID_SERVER, "&Server", wxPoint(128, 140));
	wxImage::AddHandler(new wxJPEGHandler);
	wxMemoryInputStream is(splash_jpg, splash_jpg_len);
	wxImage img(is, wxBITMAP_TYPE_ANY);
	m_bmp = new wxBitmap(img);
	SetClientSize(m_bmp->GetWidth(), cmdClient->GetRect().GetBottom() + 16);
	CentreOnScreen();
	Show();
}

Splash::~Splash()
{
	delete m_bmp;
}

void Splash::OnErase(wxEraseEvent &event)
{
}

void Splash::OnPaint(wxPaintEvent &event)
{
	wxPaintDC dc(this);
	dc.DrawBitmap(*m_bmp, 0, 0);
}

void Splash::OnClient(wxCommandEvent &event)
{
	new ClientUIMDIFrame;
	Destroy();
}
