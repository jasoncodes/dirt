#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: ServerUIFrameConfig.cpp,v 1.6 2003-02-22 05:50:56 jason Exp $)

#include "ServerUIFrameConfig.h"

enum
{
	ID_RESET = 1
};

BEGIN_EVENT_TABLE(ServerUIFrameConfig, wxDialog)
	EVT_BUTTON(wxID_OK, ServerUIFrameConfig::OnOK)
	EVT_BUTTON(wxID_APPLY, ServerUIFrameConfig::OnOK)
	EVT_BUTTON(ID_RESET, ServerUIFrameConfig::OnReset)
	EVT_TEXT(wxID_ANY, ServerUIFrameConfig::OnChange)
END_EVENT_TABLE()

ServerUIFrameConfig::ServerUIFrameConfig(ServerUIFrame *parent, Server *server)
	: wxDialog(parent, -1, wxT("Server Configuration"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxCLIP_CHILDREN | wxNO_FULL_REPAINT_ON_RESIZE | wxTAB_TRAVERSAL)
{
	
	m_server = server;
	
	wxPanel *panel = new wxPanel(this, -1, wxDefaultPosition, wxDefaultSize, wxNO_BORDER | wxCLIP_CHILDREN | wxNO_FULL_REPAINT_ON_RESIZE | wxTAB_TRAVERSAL);

	wxButton *cmdOK = new wxButton(panel, wxID_OK, wxT("OK"));
	wxButton *cmdCancel = new wxButton(panel, wxID_CANCEL, wxT("Cancel"));
	m_cmdApply = new wxButton(panel, wxID_APPLY, wxT("&Apply"));
	wxButton *cmdReset = new wxButton(panel, ID_RESET, wxT("&Reset"));

	wxStaticText *lblListenPort = new wxStaticText(panel, -1, wxT("Listen &Port:"));
	m_txtListenPort = new wxTextCtrl(panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	FixBorder(m_txtListenPort);
	wxStaticText *lblUserPass = new wxStaticText(panel, -1, wxT("&User Password:"));
	m_txtUserPass = new wxTextCtrl(panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD);
	FixBorder(m_txtUserPass);
	wxStaticText *lblAdminPass = new wxStaticText(panel, -1, wxT("A&dmin Password:"));
	m_txtAdminPass = new wxTextCtrl(panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD);
	FixBorder(m_txtAdminPass);

	wxBoxSizer *szrAll = new wxBoxSizer(wxHORIZONTAL);
	{

		wxBoxSizer *szrLeft = new wxBoxSizer(wxVERTICAL);
		{

			wxBoxSizer *szrLeftTop = new wxBoxSizer(wxHORIZONTAL);
			{

				wxBoxSizer *szrLabels = new wxBoxSizer(wxVERTICAL);
				{
					szrLabels->Add(lblListenPort, 1, wxBOTTOM, 8);
					szrLabels->Add(lblUserPass, 1, wxBOTTOM, 8);
					szrLabels->Add(lblAdminPass, 1, wxBOTTOM, 8);
				}
				szrLeftTop->Add(szrLabels, 0, wxEXPAND, 0);

				wxBoxSizer *szrTextBoxes = new wxBoxSizer(wxVERTICAL);
				{
					szrTextBoxes->Add(m_txtListenPort, 1, wxBOTTOM | wxEXPAND, 8);
					szrTextBoxes->Add(m_txtUserPass, 1, wxBOTTOM | wxEXPAND, 8);
					szrTextBoxes->Add(m_txtAdminPass, 1, wxBOTTOM | wxEXPAND, 8);
				}
				szrLeftTop->Add(szrTextBoxes, 1, wxEXPAND, 0);

			}
			szrLeft->Add(szrLeftTop, 0, wxEXPAND, 0);

			wxBoxSizer *szrLeftFill = new wxBoxSizer(wxHORIZONTAL);
			{
			}
			szrLeft->Add(szrLeftFill, 1, wxEXPAND, 0);

		}
		szrAll->Add(szrLeft, 1, wxLEFT | wxTOP | wxBOTTOM | wxEXPAND, 8);

		wxBoxSizer *szrRight = new wxBoxSizer(wxVERTICAL);
		{
			szrRight->Add(cmdOK, 0, wxTOP | wxBOTTOM | wxEXPAND, 8);
			szrRight->Add(cmdCancel, 0, wxBOTTOM | wxEXPAND, 8);
			szrRight->Add(m_cmdApply, 0, wxBOTTOM | wxEXPAND, 8);
			szrRight->Add(cmdReset, 0, wxBOTTOM | wxEXPAND, 8);
		}
		szrAll->Add(szrRight, 0, wxALL | wxEXPAND, 8);

	}

	LoadSettings();

	panel->SetAutoLayout(TRUE);
	panel->SetSizer(szrAll);
	szrAll->SetSizeHints( this );

	SetClientSize(450, 250);
	CentreOnParent();
	ShowModal();

}

ServerUIFrameConfig::~ServerUIFrameConfig()
{
}

void ServerUIFrameConfig::OnOK(wxCommandEvent &event)
{
	if (SaveSettings())
	{
		event.Skip();
	}
}

void ServerUIFrameConfig::OnReset(wxCommandEvent &event)
{
	if (wxMessageBox(wxT("Warning: This will reset the server configuration to default values. This operation is not undoable.\n\nAre you sure you want to reset to defaults?"), wxT("Reset Confirmation"), wxOK|wxCANCEL|wxICON_QUESTION, this) == wxOK)
	{
		ServerConfig *config = m_server->GetConfig();
		if (config->ResetToDefaults())
		{
			LoadSettings();
		}
		else
		{
			wxMessageBox(wxT("Unable to reset to defaults"), wxT("Error"), wxOK|wxICON_ERROR, this);
		}
	}
}

void ServerUIFrameConfig::OnChange(wxCommandEvent &event)
{
	m_cmdApply->Enable(true);
}

void ServerUIFrameConfig::LoadSettings()
{
	ServerConfig *config = m_server->GetConfig();
	m_txtListenPort->SetValue(wxString()<<config->GetListenPort());
	m_cmdApply->Enable(false);
}

bool ServerUIFrameConfig::SaveSettings()
{
	ServerConfig *config = m_server->GetConfig();
	long x;
	if (!m_txtListenPort->GetValue().ToLong(&x) || !config->SetListenPort(x))
	{
		wxMessageBox(wxT("Invalid listen port"), wxT("Error"), wxOK|wxICON_ERROR, this);
		m_txtListenPort->SetFocus();
		m_txtListenPort->SetSelection(-1, -1);
		return false;
	}
	config->Flush();
	m_cmdApply->Enable(false);
	return true;
}
