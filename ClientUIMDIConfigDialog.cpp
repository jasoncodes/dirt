#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: ClientUIMDIConfigDialog.cpp,v 1.1 2003-05-19 13:27:11 jason Exp $)

#include "ClientUIMDIConfigDialog.h"
#include "ClientUIMDIFrame.h"
#include "TristateConfigPanel.h"

enum
{
	ID_SOUND = 1
};

BEGIN_EVENT_TABLE(ClientUIMDIConfigDialog, wxDialog)
	EVT_BUTTON(wxID_OK, ClientUIMDIConfigDialog::OnOK)
END_EVENT_TABLE()

ClientUIMDIConfigDialog::ClientUIMDIConfigDialog(ClientUIMDIFrame *parent)
	: wxDialog(parent, wxID_ANY, wxT("Configuration"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxCLIP_CHILDREN | wxNO_FULL_REPAINT_ON_RESIZE | wxTAB_TRAVERSAL)
{
	
	m_config = &(parent->GetClient()->GetConfig());
	
	wxPanel *panel = new wxPanel(this, -1, wxDefaultPosition, wxDefaultSize, wxNO_BORDER | wxCLIP_CHILDREN | wxNO_FULL_REPAINT_ON_RESIZE | wxTAB_TRAVERSAL);

	wxButton *cmdOK = new wxButton(panel, wxID_OK, wxT("OK"));
	wxButton *cmdCancel = new wxButton(panel, wxID_CANCEL, wxT("Cancel"));

	m_pnlSound = new TristateConfigPanel(panel, ID_SOUND, wxT("Notification Sound"), wxT("Wave Files|*.wav|All Files|*"));

	wxBoxSizer *szrAll = new wxBoxSizer(wxHORIZONTAL);
	{

		wxBoxSizer *szrLeft = new wxBoxSizer(wxVERTICAL);
		{

			wxBoxSizer *szrLeftTristate = new wxBoxSizer(wxHORIZONTAL);
			{

				szrLeftTristate->Add(m_pnlSound, 1, 0, 0);

			}
			szrLeft->Add(szrLeftTristate, 0, wxEXPAND, 0);

			wxBoxSizer *szrLeftFill = new wxBoxSizer(wxHORIZONTAL);
			{
			}
			szrLeft->Add(szrLeftFill, 1, wxEXPAND, 0);

		}
		szrAll->Add(szrLeft, 1, wxLEFT | wxTOP | wxEXPAND, 8);

		wxBoxSizer *szrRight = new wxBoxSizer(wxVERTICAL);
		{
			szrRight->Add(cmdOK, 0, wxTOP | wxBOTTOM | wxEXPAND, 8);
			szrRight->Add(cmdCancel, 0, wxBOTTOM | wxEXPAND, 8);
		}
		szrAll->Add(szrRight, 0, wxALL | wxEXPAND, 8);

	}

	LoadSettings();
	
	panel->SetAutoLayout(TRUE);
	panel->SetSizer(szrAll);
	szrAll->SetSizeHints(this);

	FitInside();
	CentreOnParent();
	cmdOK->SetDefault();
	CenterOnParent();
	ShowModal();

}

ClientUIMDIConfigDialog::~ClientUIMDIConfigDialog()
{
}

void ClientUIMDIConfigDialog::OnOK(wxCommandEvent &event)
{
	if (SaveSettings())
	{
		event.Skip();
	}
}

void ClientUIMDIConfigDialog::LoadSettings()
{
	m_pnlSound->SetMode(m_config->GetSoundType());
	if (m_config->GetSoundType() == Config::tsmCustom)
	{
		m_pnlSound->SetPath(m_config->GetActualSoundFile());
	}
	#if wxUSE_WAVE
	#else
		m_pnlSound->Enable(Config::tsmCustom, false);
		if (m_pnlSound->GetMode() == Config::tsmCustom)
		{
			m_pnlSound->SetMode(Config::tsmDefault);
		}
	#endif
}

bool ClientUIMDIConfigDialog::SaveSettings()
{
	bool success = true;
	success &= m_config->SetSoundFile(m_pnlSound->GetMode(), m_pnlSound->GetPath());
	if (!success)
	{
		wxMessageBox(wxT("Error setting notification sound"), GetTitle(), wxOK|wxICON_ERROR, this);
	}
	success &= m_config->Flush();
	return success;
}
