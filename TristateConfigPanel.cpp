#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: TristateConfigPanel.cpp,v 1.7 2003-08-01 07:48:04 jason Exp $)

#include "TristateConfigPanel.h"
#include <wx/filename.h>
#include "res/sound.xpm"
#include "res/sound_disabled.xpm"
#include "util.h"

enum
{
	ID_TEXT = 1,
	ID_BROWSE,
	ID_WAVE_PREVIEW
};

BEGIN_EVENT_TABLE(TristateConfigPanel, RadioBoxPanel)
	EVT_TEXT(ID_TEXT, TristateConfigPanel::OnText)
	EVT_BUTTON(ID_BROWSE, TristateConfigPanel::OnBrowse)
	EVT_BUTTON(ID_WAVE_PREVIEW, TristateConfigPanel::OnWavePreview)
END_EVENT_TABLE()

wxString choices[3] = { wxT("None"), wxT("Default"), wxT("Custom") }; 

TristateConfigPanel::TristateConfigPanel(
	wxWindow *parent, wxWindowID id, const wxString &caption,
	const wxString &filespec, bool wave_preview, const wxPoint& pos)
	: RadioBoxPanel(parent, id, caption, pos, WXSIZEOF(choices), choices),
		m_filespec(filespec), m_wave_preview(wave_preview)
{

	wxPanel *pnl = GetPanel();

	wxSizer *szrPanel = new wxBoxSizer(wxHORIZONTAL);
	{
		m_txt = new wxTextCtrl(pnl, ID_TEXT);
		FixBorder(m_txt);
		m_txt->Enable(false);
		szrPanel->Add(m_txt, 1, wxEXPAND, 0);
		wxSize size_button = wxSize(m_txt->GetBestSize().y, m_txt->GetBestSize().y);
		m_cmdBrowse = new wxButton(pnl, ID_BROWSE, wxT("..."), wxDefaultPosition, size_button);
		m_cmdBrowse->Enable(false);
		szrPanel->Add(m_cmdBrowse, 0, wxEXPAND, 0);
		wxASSERT(!m_wave_preview || (m_wave_preview == (m_filespec.Length() > 0)));
		if (m_wave_preview)
		{
			wxBitmap bmpSound(sound_xpm);
			m_cmdWavePreview = new wxBitmapButton(pnl, ID_WAVE_PREVIEW, bmpSound, wxDefaultPosition, size_button);
			m_cmdWavePreview->SetBitmapDisabled(wxBitmap(sound_disabled_xpm));
			m_cmdWavePreview->Enable(false);
			szrPanel->Add(m_cmdWavePreview, 0, wxEXPAND, 0);
		}
		else
		{
			m_cmdWavePreview = NULL;
		}
	}
	pnl->SetSizer(szrPanel);

	SetSizes();

}

TristateConfigPanel::~TristateConfigPanel()
{
}

void TristateConfigPanel::OnSelectionChanged(int n)
{
	bool b = (n == 2);
	m_txt->Enable(b);
	m_cmdBrowse->Enable(b);
	if (m_cmdWavePreview)
	{
		#if wxUSE_WAVE
			m_cmdWavePreview->Enable(b && m_txt->GetValue().Length());
		#else
			m_cmdWavePreview->Enable(false);
		#endif
	}
}

void TristateConfigPanel::OnText(wxCommandEvent &WXUNUSED(event))
{
	if (m_cmdWavePreview)
	{
		#if wxUSE_WAVE
			m_cmdWavePreview->Enable(m_txt->IsEnabled() && m_txt->GetValue().Length());
		#else
			m_cmdWavePreview->Enable(false);
		#endif
	}
	SendChangeEvent();
}

void TristateConfigPanel::OnBrowse(wxCommandEvent &WXUNUSED(event))
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
			SendChangeEvent();
		}
	}
	else
	{
		wxDirDialog dlg(parent, m_boxRadio->GetTitle(), m_txt->GetValue(), wxDD_NEW_DIR_BUTTON);
		if (dlg.ShowModal() == wxID_OK)
		{
			m_txt->SetValue(dlg.GetPath());
			SendChangeEvent();
		}
	}
}

void TristateConfigPanel::OnWavePreview(wxCommandEvent &WXUNUSED(event))
{
	#if wxUSE_WAVE
		if (wxFileName(GetPath()).FileExists())
		{
			m_wave.Create(GetPath(), false);
			if (m_wave.IsOk() && m_wave.Play())
			{
				return;
			}
		}
		wxMessageBox(wxT("Error playing wave file: ") + GetPath(), wxT("Unable to play wave file"), wxICON_ERROR);
	#else
		wxMessageBox(wxT("Wave file support not available"), wxT("Unable to play wave file"), wxICON_ERROR);
	#endif
}

Config::TristateMode TristateConfigPanel::GetMode() const
{
	return (Config::TristateMode)GetSelection();
}

wxString TristateConfigPanel::GetPath() const
{
	return m_txt->GetValue();
}

void TristateConfigPanel::SetMode(Config::TristateMode mode)
{
	SetSelection(mode);
}

void TristateConfigPanel::SetPath(const wxString &path)
{
	m_txt->SetValue(path);
}

bool TristateConfigPanel::Enable(bool enabled)
{
	return RadioBoxPanel::Enable(enabled);
}

void TristateConfigPanel::Enable(Config::TristateMode mode, bool enabled)
{
	RadioBoxPanel::Enable(mode, enabled);
}
