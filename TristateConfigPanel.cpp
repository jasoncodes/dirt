#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: TristateConfigPanel.cpp,v 1.1 2003-05-19 13:27:11 jason Exp $)

#include "TristateConfigPanel.h"
#include <wx/filename.h>

enum
{
	ID_RADIO = 1,
	ID_TEXT,
	ID_BROWSE
};

BEGIN_EVENT_TABLE(TristateConfigPanel, wxPanel)
	EVT_SIZE(TristateConfigPanel::OnSize)
	EVT_RADIOBOX(ID_RADIO, TristateConfigPanel::OnRadio)
	EVT_TEXT(ID_TEXT, TristateConfigPanel::OnText)
	EVT_BUTTON(ID_BROWSE, TristateConfigPanel::OnBrowse)
END_EVENT_TABLE()

TristateConfigPanel::TristateConfigPanel(wxWindow *parent, wxWindowID id,
	const wxString &caption, const wxString &filespec,
	const wxPoint& pos)
	: wxPanel(parent, id, pos, wxDefaultSize),
		m_filespec(filespec)
{

	wxString choices[3] = { wxT("None"), wxT("Default"), wxT("Custom") }; 
	m_boxRadio = new wxRadioBox(this, ID_RADIO, caption, wxPoint(0,0), wxDefaultSize, 3, choices, 0, wxRA_SPECIFY_COLS);

	m_pnlPath = new wxPanel(this, wxID_ANY);
	wxSizer *szrPanel = new wxBoxSizer(wxHORIZONTAL);
	{
		m_txt = new wxTextCtrl(m_pnlPath, ID_TEXT);
		m_txt->Enable(false);
		szrPanel->Add(m_txt, 1, wxEXPAND, 0);
		m_cmdBrowse = new wxButton(m_pnlPath, ID_BROWSE, wxT("..."), wxDefaultPosition, wxSize(m_txt->GetBestSize().y, m_txt->GetBestSize().y));
		m_cmdBrowse->Enable(false);
		szrPanel->Add(m_cmdBrowse, 0, wxEXPAND, 0);
	}
	wxSize size;
	m_boxRadio->GetSize(&size.x, &size.y);
	m_pnlPath->SetSizer(szrPanel);
	m_pnlPath->Fit();
	m_pnlPath->Move(4, size.y);
	m_pnlPath->SetSize(size.x - 8, -1);
	size.y += m_pnlPath->GetSize().y + 8;
	m_boxRadio->SetSize(size);
	SetSize(size);

}

TristateConfigPanel::~TristateConfigPanel()
{
}

void TristateConfigPanel::OnSize(wxSizeEvent &event)
{
	m_boxRadio->SetSize(GetClientSize());
	m_pnlPath->SetSize(GetClientSize().x - 8, -1);
}

void TristateConfigPanel::OnRadio(wxCommandEvent &event)
{
	bool b = (event.GetInt() == 2);
	m_txt->Enable(b);
	m_cmdBrowse->Enable(b);
	SendChangeEvent();
}

void TristateConfigPanel::OnText(wxCommandEvent &event)
{
	SendChangeEvent();
}

void TristateConfigPanel::OnBrowse(wxCommandEvent &event)
{
	wxWindow *parent = GetParent();
	while (parent->GetParent())
	{
		parent = parent->GetParent();
	}
	if (m_filespec.Length())
	{
		wxFileName fn(m_txt->GetValue());
		wxFileDialog dlg(parent, m_boxRadio->GetTitle(), fn.GetPath(wxPATH_GET_VOLUME|wxPATH_GET_SEPARATOR), fn.GetFullName(), m_filespec, wxOPEN|wxFILE_MUST_EXIST);
		if (dlg.ShowModal() == wxID_OK)
		{
			m_txt->SetValue(dlg.GetPath());
		}
	}
	else
	{
		wxDirDialog dlg(parent, m_boxRadio->GetTitle(), m_txt->GetValue(), wxDD_NEW_DIR_BUTTON);
		if (dlg.ShowModal() == wxID_OK)
		{
			m_txt->SetValue(dlg.GetPath());
		}
	}
}

Config::TristateMode TristateConfigPanel::GetMode() const
{
	return (Config::TristateMode)m_boxRadio->GetSelection();
}

wxString TristateConfigPanel::GetPath() const
{
	return m_txt->GetValue();
}

void TristateConfigPanel::SetMode(Config::TristateMode mode)
{
	m_boxRadio->SetSelection(mode);
	bool b = (mode == Config::tsmCustom);
	m_txt->Enable(b);
	m_cmdBrowse->Enable(b);
}

void TristateConfigPanel::SetPath(const wxString &path)
{
	m_txt->SetValue(path);
}

void TristateConfigPanel::SendChangeEvent()
{
	wxCommandEvent evt(wxEVT_COMMAND_TEXT_UPDATED, GetId());
	evt.SetInt(m_boxRadio->GetSelection());
	evt.SetString(m_txt->GetValue());
	GetParent()->AddPendingEvent(evt);
}

bool TristateConfigPanel::Enable(bool enabled)
{
	return wxPanel::Enable(enabled);
}

void TristateConfigPanel::Enable(Config::TristateMode mode, bool enabled)
{
	m_boxRadio->Enable(mode, enabled);
}
