#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: Splash.cpp,v 1.13 2003-03-05 01:05:14 jason Exp $)

#include "Splash.h"
#include "ClientUIMDIFrame.h"
#include "ServerUIFrame.h"
#include "wx/image.h"
#include "wx/mstream.h"
#include "util.h"

#include "res/dirt.xpm"
#include "res/splash.h"

enum
{
	ID_CLIENT = 1,
	ID_SERVER,
	ID_INTERNET
};

BEGIN_EVENT_TABLE(Splash, wxFrame)
	EVT_ERASE_BACKGROUND(Splash::OnErase)
	EVT_PAINT(Splash::OnPaint)
	EVT_BUTTON(ID_CLIENT, Splash::OnButton)
	EVT_BUTTON(ID_SERVER, Splash::OnButton)
	EVT_BUTTON(ID_INTERNET, Splash::OnButton)
END_EVENT_TABLE()

Splash::Splash()
	: wxFrame(NULL, -1, AppTitle(), wxDefaultPosition, wxDefaultSize, wxCAPTION | wxCLIP_CHILDREN | wxMINIMIZE_BOX | wxSYSTEM_MENU | wxTAB_TRAVERSAL)
{

	SetIcon(wxIcon(dirt_xpm));

	const int pos_y = 140;
	const int gap_x = 16;
	const int gap_y = 16;

	wxButton *btns[3];
	const int btn_count = 3;

	btns[0] = new wxButton(this, ID_CLIENT, wxT("&Client"));
	btns[1] = new wxButton(this, ID_SERVER, wxT("&Server"));
	btns[2] = new wxButton(this, ID_INTERNET, wxT("&Internet"));

	wxImage::AddHandler(new wxJPEGHandler);
	wxMemoryInputStream is(splash_jpg, splash_jpg_len);
	wxImage img(is, wxBITMAP_TYPE_ANY);
	m_bmp = new wxBitmap(img);

	int btn_width = 0;
	
	for (int i = 0; i < btn_count; ++i)
	{
		btn_width = wxMax(btns[i]->GetSize().x, btn_width);
	}
	
	int total_width = (btn_width * btn_count) + (gap_x * (btn_count - 1));
	int start_pos = (m_bmp->GetWidth() - total_width) / 2;
	
	for (int i = 0; i < btn_count; ++i)
	{
		btns[i]->SetSize(start_pos + ((btn_width + gap_x) * i), pos_y, btn_width, -1);
	}

	SetClientSize(m_bmp->GetWidth(), btns[0]->GetRect().GetBottom() + gap_y);

	CentreOnScreen();

	m_button_clicked = false;

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

void Splash::OnButton(wxCommandEvent &event)
{
	
	if (m_button_clicked) return;
	m_button_clicked = true;
	
	switch (event.GetId())
	{

		case ID_CLIENT:
			Destroy();
			new ClientUIMDIFrame;
			break;

		case ID_SERVER:
			Destroy();
			new ServerUIFrame;
			break;

		case ID_INTERNET:
			if (OpenBrowser(this, GetPublicListURL()))
			{
				Destroy();
			}
			break;

		default:
			m_button_clicked = false;

	}

}
