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
RCS_ID($Id: Splash.cpp,v 1.30 2004-05-26 18:26:06 jason Exp $)

#include "Splash.h"
#include "ClientUIMDIFrame.h"
#include "ServerUIFrame.h"
#include "LogViewerFrame.h"
#include "LanListFrame.h"
#include "util.h"
#include "Dirt.h"

DECLARE_APP(DirtApp)

#include <wx/image.h>
#include <wx/mstream.h>

#include "res/dirt.xpm"
#include "res/splash.h"

// shamelessly borrowed from wxWidget's src/univ/themes/win32.cpp
// slight modifications are applied (colour change, unused var warning removal)
static void DrawFocusRect(wxDC& dc, const wxRect& rect)
{
	// draw the pixels manually: note that to behave in the same manner as
	// DrawRect(), we must exclude the bottom and right borders from the
	// rectangle
	wxCoord x1 = rect.GetLeft(),
			y1 = rect.GetTop(),
			x2 = rect.GetRight(),
			y2 = rect.GetBottom();

	dc.SetPen(*wxWHITE_PEN);

	// this seems to be closer than what Windows does than wxINVERT although
	// I'm still not sure if it's correct
	dc.SetLogicalFunction(wxAND_REVERSE);

	wxCoord z = 0;
	for ( z = x1 + 1; z < x2; z += 2 )
		dc.DrawPoint(z, rect.GetTop());

	wxCoord shift = z == x2 ? 0 : 1;
	for ( z = y1 + shift; z < y2; z += 2 )
		dc.DrawPoint(x2, z);

	shift = z == y2 ? 0 : 1;
	for ( z = x2 - shift; z > x1; z -= 2 )
		dc.DrawPoint(z, y2);

	shift = z == x1 ? 0 : 1;
	for ( z = y2 - shift; z > y1; z -= 2 )
		dc.DrawPoint(x1, z);

	dc.SetLogicalFunction(wxCOPY);

}

class SplashPanel : public wxPanel
{

public:
	SplashPanel(wxWindow *parent, const wxImage &img)
		: wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNO_BORDER | wxTAB_TRAVERSAL | wxCLIP_CHILDREN),
		m_img(img.Copy()), m_bmp(NULL)
	{
		SetBestSize(wxSize(m_img.GetWidth(), m_img.GetHeight()));
		m_img2 = m_img.Copy();
		unsigned char *ptr = m_img2.GetData();
		unsigned char *ptr_end = ptr + m_img2.GetWidth() * m_img2.GetHeight() * 3;
		for (; ptr < ptr_end; ++ptr)
		{
			*ptr = (unsigned char)(*ptr * 0.75);
		}
		m_bmp = new wxBitmap(m_img);
		m_bmp2 = new wxBitmap(m_img2);
		m_size.x = m_bmp->GetWidth();
		m_size.y = m_bmp->GetHeight();
	}

	virtual ~SplashPanel()
	{
		delete m_bmp;
		delete m_bmp2;
	}

	wxBitmap *GetDarkBitmap() const
	{
		return m_bmp2;
	}

protected:
	virtual wxSize DoGetBestSize() const
	{
		wxSize size = wxPanel::DoGetBestSize();
		FixBitmap(size);
		if (m_bmp)
		{
			size.x = wxMax(size.x, m_size.x);
			size.y = wxMax(size.y, m_size.y);
		}
		return size;
	}

	void OnErase(wxEraseEvent &WXUNUSED(event))
	{
	}

	void OnPaint(wxPaintEvent &WXUNUSED(event))
	{
		wxPaintDC dc(this);
		dc.BeginDrawing();
		dc.DrawBitmap(*m_bmp, 0, 0);
		dc.EndDrawing();
	}

	void OnSize(wxSizeEvent &event)
	{
		FixBitmap(GetSize());
		event.Skip();
	}

	void FixBitmap(const wxSize &size) const
	{
		if (m_bmp)
		{
			int new_width = m_size.x;
			int new_height = m_size.y;
			if (size.x > new_width)
			{
				new_height = (int)(((double)new_height * size.x / new_width)+0.5);
				new_width = size.x;
			}
			if (size.y > new_height)
			{
				new_width = (int)(((double)new_width * size.y / new_height)+0.5);
				new_height = size.y;
			}
			if (size.x < new_width || size.y < new_height)
			{
				delete m_bmp;
				m_bmp = new wxBitmap(m_img.Scale(new_width, new_height));
				delete m_bmp2;
				m_bmp2 = new wxBitmap(m_img2.Scale(new_width, new_height));
				m_size = wxSize(new_width, new_height);
			}
		}
	}

protected:
	wxImage m_img, m_img2;
	mutable wxBitmap *m_bmp, *m_bmp2;
	mutable wxSize m_size;

private:
	DECLARE_EVENT_TABLE()
	DECLARE_NO_COPY_CLASS(SplashPanel)

};

BEGIN_EVENT_TABLE(SplashPanel, wxPanel)
	EVT_ERASE_BACKGROUND(SplashPanel::OnErase)
	EVT_PAINT(SplashPanel::OnPaint)
	EVT_SIZE(SplashPanel::OnSize)
END_EVENT_TABLE()

class SplashButton : public wxPanel
{

public:
	SplashButton(SplashPanel *parent, int id, const wxString &caption, int mnemonic_pos = -1)
		: wxPanel(parent, id, wxDefaultPosition, wxSize(67, 26))
	{

		m_caption = caption;
		m_mnemonic_pos = mnemonic_pos;

		m_focused = false;
		m_depressed = false;

	}

	inline wxBitmap *GetDarkBitmap()
	{
		SplashPanel *panel = (SplashPanel*)GetParent();
		return panel->GetDarkBitmap();
	}

protected:
	void OnEraseBackground(wxEraseEvent &WXUNUSED(event))
	{
	}

	void DrawOutline(wxDC &dc, const wxPen &pen1, const wxPen &pen2, int depth)
	{
		wxSize size = GetSize();
		if (pen1 != wxNullPen)
		{
			dc.SetPen(pen1);
			dc.DrawLine(depth, depth, size.x-depth, depth);
			dc.DrawLine(depth, depth, depth, size.y-depth);
		}
		if (pen2 != wxNullPen)
		{
			dc.SetPen(pen2);
			dc.DrawLine(size.x-depth-1, depth, size.x-depth-1, size.y-depth);
			dc.DrawLine(depth, size.y-depth-1, size.x-depth, size.y-depth-1);
		}
	}

	void OnPaint(wxPaintEvent &WXUNUSED(event))
	{
		
		wxPaintDC dc(this);
		wxSize size = GetSize();
		
		dc.BeginDrawing();
		wxMemoryDC dcBitmap(&dc);
		dcBitmap.SelectObject(*GetDarkBitmap());
		dc.Blit(0, 0, size.x, size.y, &dcBitmap, GetPosition().x, GetPosition().y);
		dcBitmap.SelectObject(wxNullBitmap);

		wxFont font1 = wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);
		wxFont font2 = font1;
		font2.SetUnderlined(true);

		dc.SetTextForeground(*wxLIGHT_GREY);
		dc.SetFont(font1);
		long x, y;
		dc.GetTextExtent(m_caption, &x, &y);
		x = (size.x-x) / 2;
		y = (size.y-y) / 2;
		if (m_depressed)
		{
			x += 1;
			y += 1;
		}
		if (m_mnemonic_pos == -1)
		{
			dc.DrawText(m_caption, x, y);
		}
		else
		{
			long tmp_x, tmp_y;
			dc.DrawText(m_caption.Left(m_mnemonic_pos), x, y);
			dc.GetTextExtent(m_caption.Left(m_mnemonic_pos), &tmp_x, &tmp_y);
			x += tmp_x;
			dc.SetFont(font2);
			dc.DrawText(m_caption.Mid(m_mnemonic_pos, 1), x, y);
			dc.SetFont(font1);
			dc.GetTextExtent(m_caption.Mid(m_mnemonic_pos, 1), &tmp_x, &tmp_y);
			x += tmp_x;
			dc.DrawText(m_caption.Mid(m_mnemonic_pos+1), x, y);
		}

		DrawOutline(
			dc, 
			m_focused ? ( m_depressed ? *wxBLACK_PEN : *wxBLACK_PEN ) : *wxWHITE_PEN,
			m_focused ? ( m_depressed ? *wxBLACK_PEN : *wxBLACK_PEN ) : *wxGREY_PEN,
			0);

		DrawOutline(
			dc, 
			m_focused ? ( m_depressed ? *wxMEDIUM_GREY_PEN : *wxWHITE_PEN ) : *wxLIGHT_GREY_PEN,
			m_focused ? ( m_depressed ? *wxMEDIUM_GREY_PEN : *wxGREY_PEN ) : *wxMEDIUM_GREY_PEN,
			1);

		DrawOutline(
			dc, 
			m_focused ? ( m_depressed ? wxNullPen : *wxLIGHT_GREY_PEN ) : wxNullPen,
			m_focused ? ( m_depressed ? wxNullPen : *wxMEDIUM_GREY_PEN ) : wxNullPen,
			2);

		if (m_focused)
		{
			wxRect focus_rect(wxPoint(0, 0), size);
			focus_rect.Deflate(4);
			DrawFocusRect(dc, focus_rect);
		}

		dc.EndDrawing();

	}

	void OnSetFocus(wxFocusEvent &event)
	{
		m_focused = true;
		Refresh();
		event.Skip();
	}

	void OnKillFocus(wxFocusEvent &event)
	{
		m_focused = false;
		Refresh();
		event.Skip();
	}

	void OnLeftDown(wxMouseEvent &WXUNUSED(event))
	{
		SetFocus();
		CaptureMouse();
		m_depressed = true;
		Refresh();
	}

	void OnLeftUp(wxMouseEvent &WXUNUSED(event))
	{
		if (HasCapture())
		{
			ReleaseMouse();
		}
		m_depressed = false;
		Refresh();
	}

	void OnMotion(wxMouseEvent &event)
	{
		if (event.LeftIsDown())
		{
			bool new_depressed =
				(event.GetX() >= 0) &&
				(event.GetY() >= 0) &&
				(event.GetX() < GetSize().x) &&
				(event.GetY() < GetSize().y);
			if (m_depressed != new_depressed)
			{
				m_depressed = new_depressed;
				Refresh();
			}
		}
	}

protected:
	wxString m_caption;
	int m_mnemonic_pos;
	bool m_focused;
	bool m_depressed;

private:
	DECLARE_EVENT_TABLE()
	DECLARE_NO_COPY_CLASS(SplashButton)

};

BEGIN_EVENT_TABLE(SplashButton, wxPanel)
	EVT_ERASE_BACKGROUND(SplashButton::OnEraseBackground)
	EVT_PAINT(SplashButton::OnPaint)
	EVT_SET_FOCUS(SplashButton::OnSetFocus)
	EVT_KILL_FOCUS(SplashButton::OnKillFocus)
	EVT_LEFT_DOWN(SplashButton::OnLeftDown)
	EVT_LEFT_DCLICK(SplashButton::OnLeftDown)
	EVT_LEFT_UP(SplashButton::OnLeftUp)
	EVT_MOTION(SplashButton::OnMotion)
END_EVENT_TABLE()

enum
{
	ID_CLIENT = 1,
	ID_SERVER,
	ID_INTERNET,
	ID_LANLIST,
	ID_LOGS
};

BEGIN_EVENT_TABLE(Splash, wxFrame)
	EVT_BUTTON(wxID_ANY, Splash::OnButton)
	EVT_MENU(wxID_ANY, Splash::OnButton)
END_EVENT_TABLE()

Splash::Splash()
	: wxFrame(
		NULL, wxID_ANY, AppTitle(), wxDefaultPosition, wxDefaultSize,
		(wxDEFAULT_FRAME_STYLE | wxTAB_TRAVERSAL)
			& ~(wxRESIZE_BORDER|wxMAXIMIZE_BOX),
		wxT("Dirt"))
{

	wxImage::AddHandler(new wxJPEGHandler);
	wxMemoryInputStream is(splash_jpg, splash_jpg_len);
	wxImage img(is, wxBITMAP_TYPE_ANY);

	SplashPanel *panel = new SplashPanel(this, img);

	SetIcon(wxIcon(dirt_xpm));

	SplashButton *btns[5];
	const int btn_count = 5;

	btns[0] = new SplashButton(panel, ID_CLIENT, wxT("Client"), 0);
	btns[1] = new SplashButton(panel, ID_SERVER, wxT("Server"), 0);
	btns[2] = new SplashButton(panel, ID_INTERNET, wxT("Internet"), 0);
	btns[3] = new SplashButton(panel, ID_LANLIST, wxT("LAN"), 2);
	btns[4] = new SplashButton(panel, ID_LOGS, wxT("Logs"), 0);

	wxAcceleratorEntry entries[btn_count*2];
	entries[0].Set(wxACCEL_ALT, 'C', ID_CLIENT);
	entries[1].Set(0,           'C', ID_CLIENT);
	entries[2].Set(wxACCEL_ALT, 'S', ID_SERVER);
	entries[3].Set(0,           'S', ID_SERVER);
	entries[4].Set(wxACCEL_ALT, 'I', ID_INTERNET);
	entries[5].Set(0,           'I', ID_INTERNET);
	entries[6].Set(wxACCEL_ALT, 'N', ID_LANLIST);
	entries[7].Set(0,           'N', ID_LANLIST);
	entries[8].Set(wxACCEL_ALT, 'L', ID_LOGS);
	entries[9].Set(0,           'L', ID_LOGS);
	wxAcceleratorTable accel(btn_count*2, entries);
	SetAcceleratorTable(accel);

	wxBoxSizer *szrAll = new wxBoxSizer(wxVERTICAL);
	{

		wxBoxSizer *szrDummy = new wxBoxSizer(wxVERTICAL);
		{
		}
		szrAll->Add(szrDummy, 1, wxEXPAND);

		wxGridSizer *szrButtons = new wxGridSizer(1, btn_count, 8, 8);
		{
			for (int i = 0; i < btn_count; ++i)
			{
				szrButtons->Add(btns[i]);
			}
		}
		szrAll->Add(szrButtons, 0, wxALIGN_CENTER|wxBOTTOM, 12);

	}

	panel->SetSizer(szrAll);
	SetClientSize(wxSize(0, 0)); // so the line below always causes a resize event :)
	SetClientSize(panel->GetBestSize());

	CentreOnScreen();
	m_button_clicked = false;

	Show();

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
			{
				Destroy();
				ClientUIMDIFrame *ui = new ClientUIMDIFrame;
				wxGetApp().SetClient(ui->GetClient());
			}
			break;

		case ID_SERVER:
			Destroy();
			(new ServerUIFrame)->SetPositionAndShow(false);
			break;

		case ID_INTERNET:
			if (OpenPreferredBrowser(this, GetPublicListURL()))
			{
				Destroy();
			}
			break;

		case ID_LANLIST:
			Destroy();
			new LanListFrame;
			break;

		case ID_LOGS:
			Destroy();
			new LogViewerFrame;
			break;

		default:
			m_button_clicked = false;

	}

}
