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
RCS_ID($Id: FontControl.cpp,v 1.2 2004-05-16 04:42:45 jason Exp $)

#include "FontControl.h"
#include <wx/fontdlg.h>

enum
{
	ID_BROWSE = 1
};

class FontControlPreviewPanel : public wxPanel
{

public:
	FontControlPreviewPanel(wxWindow *window, int id, wxFontData &data)
		: wxPanel(window, id, wxDefaultPosition, wxSize(96, 1), wxSUNKEN_BORDER), m_data(data)
	{
	}

protected:
	void OnPaint(wxPaintEvent &WXUNUSED(event))
	{
		wxPaintDC dc(this);
		wxFont font = m_data.GetInitialFont();
		wxString str = font.GetFaceName();
		wxSize size = GetClientSize();
		dc.SetFont(font);
		wxCoord x, y;
		dc.GetTextExtent(str, &x ,&y);
		x = (size.x - x) / 2;
		y = (size.y - y) / 2;
		dc.DrawText(str, x, y);
	}

protected:
	wxFontData &m_data;

private:
	DECLARE_EVENT_TABLE()

};

BEGIN_EVENT_TABLE(FontControlPreviewPanel, wxPanel)
	EVT_PAINT(FontControlPreviewPanel::OnPaint)
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(FontControl, wxPanel)
	EVT_BUTTON(ID_BROWSE, FontControl::OnBrowse)
END_EVENT_TABLE()

FontControl::FontControl(wxWindow *parent, int id, wxPoint pos, wxSize size)
	: wxPanel(parent, id, pos, size)
{

	m_pnlPreview = new FontControlPreviewPanel(this, wxID_ANY, m_data);
	wxButton *cmdBrowse = new wxButton(this, ID_BROWSE, wxT("..."));

	wxSize sizeBrowse = cmdBrowse->GetSize();
	cmdBrowse->SetSize(sizeBrowse.y, sizeBrowse.y);

	wxBoxSizer *szr = new wxBoxSizer(wxHORIZONTAL);
	{
		szr->Add(m_pnlPreview, 1, wxEXPAND, 0);
		szr->Add(cmdBrowse, 0, wxEXPAND, 0);
	}

	SetAutoLayout(true);
	SetSizer(szr);
	szr->SetSizeHints(this);

	m_data.EnableEffects(false);
	m_data.SetAllowSymbols(false);
	m_data.SetInitialFont(*wxSWISS_FONT);

}

FontControl::~FontControl()
{
}

void FontControl::OnBrowse(wxCommandEvent &event)
{
	wxFontDialog dlg(this, m_data);
	if (dlg.ShowModal() == wxID_OK)
	{
		m_data = dlg.GetFontData();
		SetFont(m_data.GetChosenFont());
		event.SetId(GetId());
		event.SetEventObject(this);
		GetParent()->ProcessEvent(event);
	}
}

wxFont FontControl::GetFont() const
{
	return m_data.GetInitialFont();
}

bool FontControl::SetFont(const wxFont &font)
{
	m_data.SetInitialFont(font);
	m_pnlPreview->Refresh();
	return wxPanel::SetFont(font);
}
