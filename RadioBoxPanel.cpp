#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: RadioBoxPanel.cpp,v 1.3 2003-08-01 07:48:02 jason Exp $)

#include "RadioBoxPanel.h"

enum
{
	ID_RADIO = 100,
};

BEGIN_EVENT_TABLE(RadioBoxPanel, wxPanel)
	EVT_SIZE(RadioBoxPanel::OnSize)
	EVT_RADIOBOX(ID_RADIO, RadioBoxPanel::OnRadio)
END_EVENT_TABLE()

RadioBoxPanel::RadioBoxPanel(wxWindow *parent, wxWindowID id,
	const wxString &caption, const wxPoint& pos,
	size_t num, const wxString *choices)
	: wxPanel(parent, id, pos, wxDefaultSize), m_sizes_set(false)
{
	m_boxRadio = new wxRadioBox(this, ID_RADIO, caption, wxPoint(0,0), wxDefaultSize, num, choices, 0, wxRA_SPECIFY_COLS);
	m_pnl = new wxPanel(this, wxID_ANY);
}

RadioBoxPanel::~RadioBoxPanel()
{
}

void RadioBoxPanel::SetSizes()
{
	if (!m_sizes_set)
	{
		OnSelectionChanged(GetSelection());
	}
	m_sizes_set = true;
	wxSize size;
	m_boxRadio->GetSize(&size.x, &size.y);
	if (m_pnl->GetSizer())
	{
		m_pnl->Fit();
	}
	m_pnl->Move(4, size.y);
	m_pnl->SetSize(size.x - 8, -1);
	size.y += m_pnl->GetSize().y + 8;
	m_boxRadio->SetSize(size);
	SetSize(size);
}

void RadioBoxPanel::OnSize(wxSizeEvent &WXUNUSED(event))
{
	wxASSERT_MSG(m_sizes_set,
		wxT("You need to call SetSizes() in the subclass constructor"));
	m_boxRadio->SetSize(GetClientSize());
	m_pnl->SetSize(GetClientSize().x - 8, -1);
}

void RadioBoxPanel::OnRadio(wxCommandEvent &event)
{
	OnSelectionChanged(event.GetInt());
	SendChangeEvent();
}

int RadioBoxPanel::GetSelection() const
{
	return m_boxRadio->GetSelection();
}

void RadioBoxPanel::SetSelection(int n)
{
	m_boxRadio->SetSelection(n);
	OnSelectionChanged(n);
}

void RadioBoxPanel::SendChangeEvent()
{
	wxCommandEvent evt(wxEVT_COMMAND_TEXT_UPDATED, GetId());
	evt.SetInt(m_boxRadio->GetSelection());
	GetParent()->AddPendingEvent(evt);
}

bool RadioBoxPanel::Enable(bool enabled)
{
	return wxPanel::Enable(enabled);
}

void RadioBoxPanel::Enable(int n, bool enabled)
{
	m_boxRadio->Enable(n, enabled);
}
