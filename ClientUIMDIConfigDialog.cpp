#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: ClientUIMDIConfigDialog.cpp,v 1.6 2003-05-31 05:02:14 jason Exp $)

#include "ClientUIMDIConfigDialog.h"
#include "ClientUIMDIFrame.h"
#include "TristateConfigPanel.h"
#include "StaticCheckBoxSizer.h"
#include "RadioBoxPanel.h"

static const wxString choices[] = { wxT("Any"), wxT("Allow only"), wxT("Exclude only") };

class DestNetworkPanel : public RadioBoxPanel
{

public:
	DestNetworkPanel(wxWindow *parent, wxWindowID id)
		: RadioBoxPanel(
			parent, id, wxT("Destination Network"),
			wxDefaultPosition,
			WXSIZEOF(choices), choices)
	{
		
		wxPanel *pnl = GetPanel();
		
		m_lblNetwork = new wxStaticText(pnl, wxID_ANY, wxT("Network:"));
		m_txtNetwork = new wxTextCtrl(pnl, wxID_ANY);
		m_lblSubnet = new wxStaticText(pnl, wxID_ANY, wxT("Subnet:"));
		m_txtSubnet = new wxTextCtrl(pnl, wxID_ANY);

		wxFlexGridSizer *szr = new wxFlexGridSizer(4, 8, 8);
		szr->AddGrowableCol(1);
		szr->AddGrowableCol(3);
		{
			szr->Add(m_lblNetwork, 0, wxALIGN_CENTER_VERTICAL);
			szr->Add(m_txtNetwork, 0, wxEXPAND);
			szr->Add(m_lblSubnet, 0, wxALIGN_CENTER_VERTICAL);
			szr->Add(m_txtSubnet, 0, wxEXPAND);
		}
		pnl->SetSizer(szr);
		
		SetSizes();

	}

protected:
	virtual void OnSelectionChanged(int n)
	{
	}

protected:
	wxStaticText *m_lblNetwork;
	wxTextCtrl *m_txtNetwork;
	wxStaticText *m_lblSubnet;
	wxTextCtrl *m_txtSubnet;

};

class DestPortsPanel : public RadioBoxPanel
{

public:
	DestPortsPanel(wxWindow *parent, wxWindowID id)
		: RadioBoxPanel(
			parent, id, wxT("Destination Ports"),
			wxDefaultPosition,
			WXSIZEOF(choices), choices)
	{

		wxPanel *pnl = GetPanel();
		
		m_lblPorts = new wxStaticText(pnl, wxID_ANY, wxT("Ports:"));
		m_txtPorts = new wxTextCtrl(pnl, wxID_ANY);

		wxFlexGridSizer *szr = new wxFlexGridSizer(2, 8, 8);
		szr->AddGrowableCol(1);
		{
			szr->Add(m_lblPorts, 0, wxALIGN_CENTER_VERTICAL);
			szr->Add(m_txtPorts, 0, wxEXPAND);
		}
		pnl->SetSizer(szr);
		
		SetSizes();

	}

protected:
	virtual void OnSelectionChanged(int n)
	{
	}

protected:
	wxStaticText *m_lblPorts;
	wxTextCtrl *m_txtPorts;

};

enum
{
	ID_LOG = 1,
	ID_SOUND
};

BEGIN_EVENT_TABLE(ClientUIMDIConfigDialog, wxDialog)
	EVT_BUTTON(wxID_OK, ClientUIMDIConfigDialog::OnOK)
END_EVENT_TABLE()

ClientUIMDIConfigDialog::ClientUIMDIConfigDialog(ClientUIMDIFrame *parent)
	: wxDialog(parent, wxID_ANY, wxT("Configuration"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxCLIP_CHILDREN | wxNO_FULL_REPAINT_ON_RESIZE | wxTAB_TRAVERSAL)
{
	
	m_config = &(parent->GetClient()->GetConfig());
	
	wxPanel *panel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNO_BORDER | wxCLIP_CHILDREN | wxNO_FULL_REPAINT_ON_RESIZE | wxTAB_TRAVERSAL);

	m_chkProxy = new wxCheckBox(panel, wxID_ANY, wxT("&Proxy Support"));
	m_lblProtocol = new wxStaticText(panel, wxID_ANY, wxT("Protocol:"));
	const wxString protocols[] = { wxT("SOCKS 4"), wxT("SOCKS 5"), wxT("HTTP CONNECT") };
	m_cmbProtocol = new wxComboBox(panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, WXSIZEOF(protocols), protocols, wxCB_DROPDOWN|wxCB_READONLY);
	m_lblHostname = new wxStaticText(panel, wxID_ANY, wxT("Hostname:"));
	m_txtHostname = new wxTextCtrl(panel, wxID_ANY);
	m_lblPort = new wxStaticText(panel, wxID_ANY, wxT("Port:"));
	m_txtPort = new wxTextCtrl(panel, wxID_ANY);
	m_txtPort->SetSize(48, -1);
	m_lblUsername = new wxStaticText(panel, wxID_ANY, wxT("Username:"));
	m_txtUsername = new wxTextCtrl(panel, wxID_ANY);
	m_txtUsername->SetSize(64, -1);
	m_lblPassword = new wxStaticText(panel, wxID_ANY, wxT("Password:"));
	m_txtPassword = new wxTextCtrl(panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD);
	m_txtPassword->SetSize(64, -1);
	m_fraProxyTypes = new wxStaticBox(panel, wxID_ANY, wxT("Connection Types"));
	m_chkTypeServer = new wxCheckBox(panel, wxID_ANY, wxT("Chat Server"));
	m_chkTypeDCCConnect = new wxCheckBox(panel, wxID_ANY, wxT("DCC Connect"));
	m_chkTypeDCCListen = new wxCheckBox(panel, wxID_ANY, wxT("DCC Listen"));
	m_pnlDestNetwork = new DestNetworkPanel(panel, wxID_ANY);
	m_pnlDestPorts = new DestPortsPanel(panel, wxID_ANY);

/*	m_chkProxy->Enable(false);
	m_lblProtocol->Enable(false);
	m_cmbProtocol->Enable(false);
	m_lblHostname->Enable(false);
	m_txtHostname->Enable(false);
	m_lblPort->Enable(false);
	m_txtPort->Enable(false);
	m_lblUsername->Enable(false);
	m_txtUsername->Enable(false);
	m_lblPassword->Enable(false);
	m_txtPassword->Enable(false);
	m_fraProxyTypes->Enable(false);
	m_chkTypeServer->Enable(false);
	m_chkTypeDCCConnect->Enable(false);
	m_chkTypeDCCListen->Enable(false);
	m_pnlDestNetwork->Enable(false);
	m_pnlDestPorts->Enable(false);
*/
	m_pnlLog = new TristateConfigPanel(panel, ID_LOG, wxT("Log File Directory"));
	m_pnlSound = new TristateConfigPanel(panel, ID_SOUND, wxT("Notification Sound"), wxT("Wave Files|*.wav|All Files|*"), true);

	wxButton *cmdOK = new wxButton(panel, wxID_OK, wxT("OK"));
	wxButton *cmdCancel = new wxButton(panel, wxID_CANCEL, wxT("Cancel"));

	wxBoxSizer *szrAll = new wxBoxSizer(wxHORIZONTAL);
	{

		wxBoxSizer *szrMain = new wxBoxSizer(wxHORIZONTAL);
		{

			wxStaticBox *fraProxy = new wxStaticBox(panel, wxID_ANY, wxString(wxT(' '), 28));
			wxSizer *szrProxy = new StaticCheckBoxSizer(fraProxy, m_chkProxy, wxVERTICAL);
			{

				wxFlexGridSizer *szrProxyTop = new wxFlexGridSizer(2, 8, 8);
				szrProxyTop->AddGrowableCol(1);
				{

					szrProxyTop->Add(m_lblProtocol, 0, wxALIGN_CENTER_VERTICAL);
					szrProxyTop->Add(m_cmbProtocol, 0, wxEXPAND);

					szrProxyTop->Add(m_lblHostname, 0, wxALIGN_CENTER_VERTICAL);
					wxFlexGridSizer *szrProxyTopHostPort = new wxFlexGridSizer(3, 8 ,8);
					szrProxyTopHostPort->AddGrowableCol(0);
					{
						szrProxyTopHostPort->Add(m_txtHostname, 0, wxEXPAND);
						szrProxyTopHostPort->Add(m_lblPort, 0, wxALIGN_CENTER_VERTICAL);
						szrProxyTopHostPort->Add(m_txtPort, 0, 0);
					}
					szrProxyTop->Add(szrProxyTopHostPort, 0, wxEXPAND);

					szrProxyTop->Add(m_lblUsername, 0, wxALIGN_CENTER_VERTICAL);
					wxFlexGridSizer *szrProxyTopUserPass = new wxFlexGridSizer(3, 8 ,8);
					szrProxyTopUserPass->AddGrowableCol(0);
					szrProxyTopUserPass->AddGrowableCol(2);
					{
						szrProxyTopUserPass->Add(m_txtUsername, 0, wxEXPAND);
						szrProxyTopUserPass->Add(m_lblPassword, 0, wxALIGN_CENTER_VERTICAL);
						szrProxyTopUserPass->Add(m_txtPassword, 0, wxEXPAND);
					}
					szrProxyTop->Add(szrProxyTopUserPass, 0, wxEXPAND);

				}
				szrProxy->Add(szrProxyTop, 0, wxEXPAND | wxBOTTOM, 0);

				wxStaticBoxSizer *szrProxyTypes = new wxStaticBoxSizer(m_fraProxyTypes, wxHORIZONTAL);
				{
					szrProxyTypes->Add(m_chkTypeServer, 1, wxRIGHT, 8);
					szrProxyTypes->Add(m_chkTypeDCCConnect, 1, wxRIGHT, 8);
					szrProxyTypes->Add(m_chkTypeDCCListen, 1, 0, 0);
				}
				szrProxy->Add(szrProxyTypes, 0, wxEXPAND | wxBOTTOM, 0);

				szrProxy->Add(m_pnlDestNetwork, 0, wxEXPAND | wxBOTTOM, 0);

				szrProxy->Add(m_pnlDestPorts, 0, wxEXPAND | wxBOTTOM, 0);
				
			}
			szrMain->Add(szrProxy, 3, wxRIGHT, 8);

			wxBoxSizer *szrTristate = new wxBoxSizer(wxVERTICAL);
			{

				szrTristate->Add(m_pnlLog, 0, wxBOTTOM | wxEXPAND, 8);
				szrTristate->Add(m_pnlSound, 0, wxEXPAND, 0);

			}
			szrMain->Add(szrTristate, 2, 0, 0);

		}
		szrAll->Add(szrMain, 1, wxALL | wxEXPAND, 8);

		wxBoxSizer *szrButtons = new wxBoxSizer(wxVERTICAL);
		{
			szrButtons->Add(cmdOK, 0, wxTOP | wxBOTTOM | wxEXPAND, 8);
			szrButtons->Add(cmdCancel, 0, wxBOTTOM | wxEXPAND, 8);
		}
		szrAll->Add(szrButtons, 0, wxTOP | wxRIGHT | wxEXPAND, 8);

	}

	LoadSettings();

	m_chkProxy->SetFocus();
	
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

	m_pnlLog->SetMode(m_config->GetLogDirType());
	if (m_config->GetLogDirType() == Config::tsmCustom)
	{
		m_pnlLog->SetPath(m_config->GetActualLogDir());
	}

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
	
	bool bLogChanged = (m_config->GetLogDirType() != m_pnlLog->GetMode());
	if (!bLogChanged && m_config->GetLogDirType() == Config::tsmCustom)
	{
		bLogChanged = m_pnlLog->GetPath() != m_config->GetActualLogDir();
	}
	if (bLogChanged)
	{
		success &= m_config->SetLogDir(m_pnlLog->GetMode(), m_pnlLog->GetPath());
		if (success)
		{
			((ClientUIMDIFrame*)GetParent())->InitLogs();
		}
		else
		{
			wxMessageBox(wxT("Error setting log directory"), GetTitle(), wxOK|wxICON_ERROR, this);
		}
	}
	
	success &= m_config->SetSoundFile(m_pnlSound->GetMode(), m_pnlSound->GetPath());
	if (!success)
	{
		wxMessageBox(wxT("Error setting notification sound"), GetTitle(), wxOK|wxICON_ERROR, this);
	}
	
	success &= m_config->Flush();

	return success;

}
