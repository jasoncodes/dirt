#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: FontControl.cpp,v 1.1 2003-08-22 14:41:35 jason Exp $)

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
