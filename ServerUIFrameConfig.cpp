#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: ServerUIFrameConfig.cpp,v 1.3 2003-02-22 03:21:04 jason Exp $)

#include "ServerUIFrameConfig.h"

enum
{
};

BEGIN_EVENT_TABLE(ServerUIFrameConfig, wxDialog)
	EVT_BUTTON(wxID_OK, ServerUIFrameConfig::OnOK)
	EVT_BUTTON(wxID_APPLY, ServerUIFrameConfig::OnOK)
END_EVENT_TABLE()

ServerUIFrameConfig::ServerUIFrameConfig(ServerUIFrame *parent, Server *server)
	: wxDialog(parent, -1, wxT("Server Configuration"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxCLIP_CHILDREN | wxNO_FULL_REPAINT_ON_RESIZE | wxTAB_TRAVERSAL)
{
	
	m_server = server;
	
	wxPanel *panel = new wxPanel(this, -1, wxDefaultPosition, wxDefaultSize, wxNO_BORDER | wxCLIP_CHILDREN | wxNO_FULL_REPAINT_ON_RESIZE | wxTAB_TRAVERSAL);

	wxButton *cmdOK = new wxButton(panel, wxID_OK, wxT("OK"));
	wxButton *cmdCancel = new wxButton(panel, wxID_CANCEL, wxT("Cancel"));
	wxButton *cmdApply = new wxButton(panel, wxID_APPLY, wxT("Apply"));

	wxStaticText *lblUserPass = new wxStaticText(panel, -1, wxT("&User Password:"));
	wxTextCtrl *txtUserPass = new wxTextCtrl(panel, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD);
	FixBorder(txtUserPass);
	wxStaticText *lblAdminPass = new wxStaticText(panel, -1, wxT("&Admin Password:"));
	wxTextCtrl *txtAdminPass = new wxTextCtrl(panel, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD);
	FixBorder(txtAdminPass);

	wxBoxSizer *szrRight = new wxBoxSizer(wxVERTICAL);
	{
		szrRight->Add(cmdOK, 0, wxTOP | wxBOTTOM | wxEXPAND, 8);
		szrRight->Add(cmdCancel, 0, wxBOTTOM | wxEXPAND, 8);
		szrRight->Add(cmdApply, 0, wxBOTTOM | wxEXPAND, 8);
	}

	wxBoxSizer *szrLabels = new wxBoxSizer(wxVERTICAL);
	{
		szrLabels->Add(lblUserPass, 1, wxRIGHT, 8);
		szrLabels->Add(lblAdminPass, 1, wxRIGHT, 8);
	}

	wxBoxSizer *szrTextBoxes = new wxBoxSizer(wxVERTICAL);
	{
		szrTextBoxes->Add(txtUserPass, 1, wxBOTTOM | wxEXPAND, 8);
		szrTextBoxes->Add(txtAdminPass, 1, wxBOTTOM | wxEXPAND, 8);
	}

	wxBoxSizer *szrLeftTop = new wxBoxSizer(wxHORIZONTAL);
	{
		szrLeftTop->Add(szrLabels, 0, wxEXPAND, 0);
		szrLeftTop->Add(szrTextBoxes, 1, wxEXPAND, 0);
	}
	
	wxBoxSizer *szrLeftFill = new wxBoxSizer(wxHORIZONTAL);
	{
	}

	wxBoxSizer *szrLeft = new wxBoxSizer(wxVERTICAL);
	{
		szrLeft->Add(szrLeftTop, 0, wxEXPAND, 0);
		szrLeft->Add(szrLeftFill, 1, wxEXPAND, 0);
	}

	wxBoxSizer *szrAll = new wxBoxSizer(wxHORIZONTAL);
	{
		szrAll->Add(szrLeft, 1, wxLEFT | wxTOP | wxBOTTOM | wxEXPAND, 8);
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

void ServerUIFrameConfig::LoadSettings()
{

}

bool ServerUIFrameConfig::SaveSettings()
{
	//wxMessageBox(wxT("Not implemented"));
	//return false;
	return true;
}
