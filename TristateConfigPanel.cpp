#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: TristateConfigPanel.cpp,v 1.8 2004-02-14 02:56:21 jason Exp $)

#include "TristateConfigPanel.h"
#include <wx/filename.h>
#include "res/sound.xpm"
#include "res/sound_disabled.xpm"
#include "util.h"

enum
{
	ID_TEXT = 1,
	ID_BROWSE,
	ID_SOUND_PREVIEW
};

BEGIN_EVENT_TABLE(TristateConfigPanel, RadioBoxPanel)
	EVT_TEXT(ID_TEXT, TristateConfigPanel::OnText)
	EVT_BUTTON(ID_BROWSE, TristateConfigPanel::OnBrowse)
	EVT_BUTTON(ID_SOUND_PREVIEW, TristateConfigPanel::OnSoundPreview)
END_EVENT_TABLE()

wxString choices[3] = { wxT("None"), wxT("Default"), wxT("Custom") }; 

TristateConfigPanel::TristateConfigPanel(
	wxWindow *parent, wxWindowID id, const wxString &caption,
	const wxString &filespec, bool sound_preview, const wxPoint& pos)
	: RadioBoxPanel(parent, id, caption, pos, WXSIZEOF(choices), choices),
		m_filespec(filespec), m_sound_preview(sound_preview)
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
		wxASSERT(!m_sound_preview || (m_sound_preview == (m_filespec.Length() > 0)));
		if (m_sound_preview)
		{
			wxBitmap bmpSound(sound_xpm);
			m_cmdSoundPreview = new wxBitmapButton(pnl, ID_SOUND_PREVIEW, bmpSound, wxDefaultPosition, size_button);
			m_cmdSoundPreview->SetBitmapDisabled(wxBitmap(sound_disabled_xpm));
			m_cmdSoundPreview->Enable(false);
			szrPanel->Add(m_cmdSoundPreview, 0, wxEXPAND, 0);
		}
		else
		{
			m_cmdSoundPreview = NULL;
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
	if (m_cmdSoundPreview)
	{
		#if wxUSE_SOUND
			m_cmdSoundPreview->Enable(b && m_txt->GetValue().Length());
		#else
			m_cmdSoundPreview->Enable(false);
		#endif
	}
}

void TristateConfigPanel::OnText(wxCommandEvent &WXUNUSED(event))
{
	if (m_cmdSoundPreview)
	{
		#if wxUSE_SOUND
			m_cmdSoundPreview->Enable(m_txt->IsEnabled() && m_txt->GetValue().Length());
		#else
			m_cmdSoundPreview->Enable(false);
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

void TristateConfigPanel::OnSoundPreview(wxCommandEvent &WXUNUSED(event))
{
	#if wxUSE_SOUND
		if (wxFileName(GetPath()).FileExists())
		{
			m_sound.Create(GetPath(), false);
			if (m_sound.IsOk() && m_sound.Play())
			{
				return;
			}
		}
		wxMessageBox(wxT("Error playing sound file: ") + GetPath(), wxT("Unable to play sound file"), wxICON_ERROR);
	#else
		wxMessageBox(wxT("Sound file support not available"), wxT("Unable to play sound file"), wxICON_ERROR);
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
