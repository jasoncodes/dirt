#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: Splash.cpp,v 1.22 2003-04-28 04:53:28 jason Exp $)

#include "Splash.h"
#include "ClientUIMDIFrame.h"
#include "ServerUIFrame.h"
#include "LogViewerFrame.h"
#include "util.h"

#include <wx/image.h>
#include <wx/mstream.h>

#include "res/dirt.xpm"
#include "res/splash.h"

class SplashPanel : public wxPanel
{

public:
	SplashPanel(wxWindow *parent, wxBitmap *bmp)
		: wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNO_BORDER | wxTAB_TRAVERSAL | wxCLIP_CHILDREN), m_bmp(bmp)
	{
	}

	virtual ~SplashPanel()
	{
		delete m_bmp;
	}

protected:
	void OnErase(wxEraseEvent &event)
	{
	}

	void OnPaint(wxPaintEvent &event)
	{
		wxPaintDC dc(this);
		dc.DrawBitmap(*m_bmp, 0, 0);
	}

protected:
	wxBitmap *m_bmp;

private:
	DECLARE_EVENT_TABLE()

};

BEGIN_EVENT_TABLE(SplashPanel, wxPanel)
	EVT_ERASE_BACKGROUND(SplashPanel::OnErase)
	EVT_PAINT(SplashPanel::OnPaint)
END_EVENT_TABLE()

enum
{
	ID_CLIENT = 1,
	ID_SERVER,
	ID_INTERNET,
	ID_LOGS
};

BEGIN_EVENT_TABLE(Splash, wxFrame)
	EVT_BUTTON(ID_CLIENT, Splash::OnButton)
	EVT_BUTTON(ID_SERVER, Splash::OnButton)
	EVT_BUTTON(ID_INTERNET, Splash::OnButton)
	EVT_BUTTON(ID_LOGS, Splash::OnButton)
END_EVENT_TABLE()

Splash::Splash()
	: wxFrame(NULL, wxID_ANY, AppTitle(), wxDefaultPosition, wxDefaultSize, (wxDEFAULT_FRAME_STYLE | wxTAB_TRAVERSAL) & ~(wxRESIZE_BORDER|wxMAXIMIZE_BOX), wxT("Dirt"))
{

	wxImage::AddHandler(new wxJPEGHandler);
	wxMemoryInputStream is(splash_jpg, splash_jpg_len);
	wxImage img(is, wxBITMAP_TYPE_ANY);
	wxBitmap *bmp = new wxBitmap(img);

	SplashPanel *panel = new SplashPanel(this, bmp);

	SetIcon(wxIcon(dirt_xpm));

	const int pos_y = 140;
	const int gap_x = 16;
	const int gap_y = 16;

	wxButton *btns[4];
	const int btn_count = 4;

	btns[0] = new wxButton(panel, ID_CLIENT, wxT("&Client"));
	btns[1] = new wxButton(panel, ID_SERVER, wxT("&Server"));
	btns[2] = new wxButton(panel, ID_INTERNET, wxT("&Internet"));
	btns[3] = new wxButton(panel, ID_LOGS, wxT("&Logs"));

	wxAcceleratorEntry entries[btn_count];
	entries[0].Set(0, 'C', ID_CLIENT);
	entries[1].Set(0, 'S', ID_SERVER);
	entries[2].Set(0, 'I', ID_INTERNET);
	entries[3].Set(0, 'L', ID_LOGS);
	wxAcceleratorTable accel(btn_count, entries);
	SetAcceleratorTable(accel);

	int btn_width = 0;
	
	for (int i = 0; i < btn_count; ++i)
	{
		btn_width = wxMax(btns[i]->GetSize().x, btn_width);
	}
	
	int total_width = (btn_width * btn_count) + (gap_x * (btn_count - 1));

	if (total_width > bmp->GetWidth()-gap_x*2)
	{
		total_width = bmp->GetWidth()-gap_x*2;
		btn_width = (total_width - (gap_x * (btn_count - 1))) / btn_count;
		total_width = (btn_width * btn_count) + (gap_x * (btn_count - 1));
	}

	int start_pos = (bmp->GetWidth() - total_width) / 2;

	for (int i = 0; i < btn_count; ++i)
	{
		btns[i]->SetSize(start_pos + ((btn_width + gap_x) * i), pos_y, btn_width, -1);
	}

	SetClientSize(bmp->GetWidth(), btns[0]->GetRect().GetBottom() + gap_y);

	CentreOnScreen();

	m_button_clicked = false;

	Show();

	btns[0]->SetFocus();

}

Splash::~Splash()
{
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
			(new ServerUIFrame)->SetPositionAndShow(false);
			break;

		case ID_INTERNET:
			if (OpenBrowser(this, GetPublicListURL()))
			{
				Destroy();
			}
			break;

		case ID_LOGS:
			Destroy();
			new LogViewerFrame;
			break;

		default:
			m_button_clicked = false;

	}

}
