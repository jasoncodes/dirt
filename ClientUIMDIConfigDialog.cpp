#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: ClientUIMDIConfigDialog.cpp,v 1.13 2003-06-12 06:00:05 jason Exp $)

#include "ClientUIMDIConfigDialog.h"
#include "ClientUIMDIFrame.h"
#include "TristateConfigPanel.h"
#include "StaticCheckBoxSizer.h"
#include "RadioBoxPanel.h"
#include "CryptSocketProxy.h"

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
		FixBorder(m_txtNetwork);
		m_lblSubnet = new wxStaticText(pnl, wxID_ANY, wxT("Subnet:"));
		m_txtSubnet = new wxTextCtrl(pnl, wxID_ANY);
		FixBorder(m_txtSubnet);

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

	wxString GetNetwork() const { return m_txtNetwork->GetValue(); }
	void SetNetwork(const wxString &str) { m_txtNetwork->SetValue(str); }
	wxString GetSubnet() const { return m_txtSubnet->GetValue(); }
	void SetSubnet(const wxString &str) { m_txtSubnet->SetValue(str); }

protected:
	virtual void OnSelectionChanged(int n)
	{
		bool enabled = (n > 0);
		m_lblNetwork->Enable(enabled);
		m_txtNetwork->Enable(enabled);
		m_lblSubnet->Enable(enabled);
		m_txtSubnet->Enable(enabled);
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
		FixBorder(m_txtPorts);

		wxFlexGridSizer *szr = new wxFlexGridSizer(2, 8, 8);
		szr->AddGrowableCol(1);
		{
			szr->Add(m_lblPorts, 0, wxALIGN_CENTER_VERTICAL);
			szr->Add(m_txtPorts, 0, wxEXPAND);
		}
		pnl->SetSizer(szr);
		
		SetSizes();

	}

	wxString GetString() const { return m_txtPorts->GetValue(); }
	void SetString(const wxString &str) { m_txtPorts->SetValue(str); }

protected:
	virtual void OnSelectionChanged(int n)
	{
		bool enabled = (n > 0);
		m_lblPorts->Enable(enabled);
		m_txtPorts->Enable(enabled);
	}

protected:
	wxStaticText *m_lblPorts;
	wxTextCtrl *m_txtPorts;

};

enum
{
	ID_LOG = 1,
	ID_SOUND,
	ID_PROXY_ENABLED,
	ID_PROXY_PROTOCOL
};

BEGIN_EVENT_TABLE(ClientUIMDIConfigDialog, wxDialog)
	EVT_BUTTON(wxID_OK, ClientUIMDIConfigDialog::OnOK)
	EVT_CHECKBOX(ID_PROXY_ENABLED, ClientUIMDIConfigDialog::OnProxy)
	EVT_COMBOBOX(ID_PROXY_PROTOCOL, ClientUIMDIConfigDialog::OnProxy)
END_EVENT_TABLE()

ClientUIMDIConfigDialog::ClientUIMDIConfigDialog(ClientUIMDIFrame *parent)
	: wxDialog(parent, wxID_ANY, wxT("Configuration"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxCLIP_CHILDREN | wxNO_FULL_REPAINT_ON_RESIZE | wxTAB_TRAVERSAL)
{
	
	m_config = &(parent->GetClient()->GetConfig());
	m_proxy_settings = new CryptSocketProxySettings(*m_config);
	
	wxPanel *panel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNO_BORDER | wxCLIP_CHILDREN | wxNO_FULL_REPAINT_ON_RESIZE | wxTAB_TRAVERSAL);

	m_chkProxy = new wxCheckBox(panel, ID_PROXY_ENABLED, wxT("&Proxy Support"));
	m_lblProtocol = new wxStaticText(panel, wxID_ANY, wxT("Protocol:"));
	const wxString* const protocol_names = CryptSocketProxySettings::GetProtocolNames();
	size_t protocol_count = CryptSocketProxySettings::GetProtocolCount();
	m_cmbProtocol = new wxComboBox(panel, ID_PROXY_PROTOCOL, protocol_names[0], wxDefaultPosition, wxDefaultSize, protocol_count, protocol_names, wxCB_DROPDOWN|wxCB_READONLY);
	m_lblHostname = new wxStaticText(panel, wxID_ANY, wxT("Hostname:"));
	m_txtHostname = new wxTextCtrl(panel, wxID_ANY);
	FixBorder(m_txtHostname);
	m_lblPort = new wxStaticText(panel, wxID_ANY, wxT("Port:"));
	m_txtPort = new wxTextCtrl(panel, wxID_ANY);
	FixBorder(m_txtPort);
	m_txtPort->SetSize(48, -1);
	m_lblUsername = new wxStaticText(panel, wxID_ANY, wxT("Username:"));
	m_txtUsername = new wxTextCtrl(panel, wxID_ANY);
	FixBorder(m_txtUsername);
	m_txtUsername->SetSize(64, -1);
	m_lblPassword = new wxStaticText(panel, wxID_ANY, wxT("Password:"));
	m_txtPassword = new wxTextCtrl(panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD);
	FixBorder(m_txtPassword);
	m_txtPassword->SetSize(64, -1);
	m_fraProxyTypes = new wxStaticBox(panel, wxID_ANY, wxT("Connection Types"));
	m_chkTypeServer = new wxCheckBox(panel, wxID_ANY, wxT("Chat Server"));
	m_chkTypeDCCConnect = new wxCheckBox(panel, wxID_ANY, wxT("DCC Connect"));
	m_chkTypeDCCListen = new wxCheckBox(panel, wxID_ANY, wxT("DCC Listen"));
	m_pnlDestNetwork = new DestNetworkPanel(panel, wxID_ANY);
	m_pnlDestPorts = new DestPortsPanel(panel, wxID_ANY);

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
	delete m_proxy_settings;
}

void ClientUIMDIConfigDialog::OnOK(wxCommandEvent &event)
{
	if (SaveSettings())
	{
		event.Skip();
	}
}

void ClientUIMDIConfigDialog::OnProxy(wxCommandEvent &event)
{
	
	bool enabled = m_chkProxy->IsChecked();
	
	CryptSocketProxyProtocol protocol =
		CryptSocketProxySettings::ProtocolFromString(m_cmbProtocol->GetValue());
	
	wxASSERT(protocol != ppUnknown);

	m_lblProtocol->Enable(enabled);
	m_cmbProtocol->Enable(enabled);
	m_lblHostname->Enable(enabled);
	m_txtHostname->Enable(enabled);
	m_lblPort->Enable(enabled);
	m_txtPort->Enable(enabled);
	m_lblUsername->Enable(enabled && CryptSocketProxySettings::DoesProtocolSupportUsername(protocol));
	m_txtUsername->Enable(enabled && CryptSocketProxySettings::DoesProtocolSupportUsername(protocol));
	m_lblPassword->Enable(enabled && CryptSocketProxySettings::DoesProtocolSupportPassword(protocol));
	m_txtPassword->Enable(enabled && CryptSocketProxySettings::DoesProtocolSupportPassword(protocol));
	m_fraProxyTypes->Enable(enabled);
	m_chkTypeServer->Enable(enabled && CryptSocketProxySettings::DoesProtocolSupportConnectionType(protocol, pctServer));
	m_chkTypeDCCConnect->Enable(enabled && CryptSocketProxySettings::DoesProtocolSupportConnectionType(protocol, pctDCCConnect));
	m_chkTypeDCCListen->Enable(enabled && CryptSocketProxySettings::DoesProtocolSupportConnectionType(protocol, pctDCCListen));
	m_pnlDestNetwork->Enable(enabled);
	m_pnlDestPorts->Enable(enabled);

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

	m_proxy_settings->LoadSettings();

	m_chkProxy->SetValue(m_proxy_settings->GetEnabled());

	m_cmbProtocol->SetValue(
		CryptSocketProxySettings::ProtocolToString(
		m_proxy_settings->GetProtocol()));

	m_txtHostname->SetValue(m_proxy_settings->GetHostname());
	m_txtPort->SetValue(wxString() << (int)m_proxy_settings->GetPort());
	m_txtUsername->SetValue(m_proxy_settings->GetUsername());
	m_txtPassword->SetValue(m_proxy_settings->GetPassword(false));

	m_chkTypeServer->SetValue(m_proxy_settings->GetConnectionType(pctServer));
	m_chkTypeDCCConnect->SetValue(m_proxy_settings->GetConnectionType(pctDCCConnect));
	m_chkTypeDCCListen->SetValue(m_proxy_settings->GetConnectionType(pctDCCListen));

	m_pnlDestNetwork->SetSelection(m_proxy_settings->GetDestNetworkMode());
	m_pnlDestNetwork->SetNetwork(m_proxy_settings->GetDestNetworkNetwork());
	m_pnlDestNetwork->SetSubnet(m_proxy_settings->GetDestNetworkSubnet());

	m_pnlDestPorts->SetSelection(m_proxy_settings->GetDestPortsMode());
	m_pnlDestPorts->SetString(m_proxy_settings->GetDestPortRanges());

	//m_chkProxy->Enable(false);
	//m_chkProxy->SetValue(false);

	wxCommandEvent evt;
	OnProxy(evt);

}

bool ClientUIMDIConfigDialog::ErrMsg(const wxString &msg)
{
	wxMessageBox(msg, GetTitle(), wxOK|wxICON_ERROR, this);
	return false;
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
		if (m_config->SetLogDir(m_pnlLog->GetMode(), m_pnlLog->GetPath()))
		{
			((ClientUIMDIFrame*)GetParent())->InitLogs();
		}
		else
		{
			ErrMsg(wxT("Error setting log directory"));
			success = false;
		}
	}
	
	if (success)
	{
		if (!m_config->SetSoundFile(m_pnlSound->GetMode(), m_pnlSound->GetPath()))
		{
			ErrMsg(wxT("Error setting notification sound"));
			success = false;
		}
	}

	if (success)
	{
		if (!m_proxy_settings->SetEnabled(m_chkProxy->GetValue()))
		{
			ErrMsg(wxT("Error setting proxy enabled"));
			success = false;
		}
	}

	if (success)
	{
		if (!m_proxy_settings->SetProtocol(m_cmbProtocol->GetValue()))
		{
			ErrMsg(wxT("Error setting proxy protocol"));
			success = false;
		}
	}

	if (success)
	{
		if (!m_proxy_settings->SetHostname(m_txtHostname->GetValue()))
		{
			ErrMsg(wxT("Error setting proxy hostname"));
			success = false;
		}
	}

	if (success)
	{
		if (!m_proxy_settings->SetPort(m_txtPort->GetValue()))
		{
			ErrMsg(wxT("Error setting proxy port"));
			success = false;
		}
	}

	if (success)
	{
		if (!m_proxy_settings->SetUsername(m_txtUsername->GetValue()))
		{
			ErrMsg(wxT("Error setting proxy username"));
			success = false;
		}
	}

	if (success)
	{
		if (!m_proxy_settings->SetPassword(m_txtPassword->GetValue()))
		{
			ErrMsg(wxT("Error setting proxy password"));
			success = false;
		}
	}

	if (success)
	{
		if (!m_proxy_settings->SetConnectionType(pctServer, m_chkTypeServer->GetValue()))
		{
			ErrMsg(wxT("Error setting proxy connection type (Server)"));
			success = false;
		}
	}

	if (success)
	{
		if (!m_proxy_settings->SetConnectionType(pctDCCConnect, m_chkTypeDCCConnect->GetValue()))
		{
			ErrMsg(wxT("Error setting proxy connection type (DCC Connect)"));
			success = false;
		}
	}

	if (success)
	{
		if (!m_proxy_settings->SetConnectionType(pctDCCListen, m_chkTypeDCCListen->GetValue()))
		{
			ErrMsg(wxT("Error setting proxy connection type (DCC Listen)"));
			success = false;
		}
	}

	if (success)
	{
		if (!m_proxy_settings->SetDestNetworkMode((CryptSocketProxyDestMode)m_pnlDestNetwork->GetSelection()))
		{
			ErrMsg(wxT("Error setting destination network mode"));
			success = false;
		}
	}

	if (success)
	{
		if (!m_proxy_settings->SetDestNetworkNetwork(m_pnlDestNetwork->GetNetwork()))
		{
			ErrMsg(wxT("Error setting destination network"));
			success = false;
		}
	}

	if (success)
	{
		if (!m_proxy_settings->SetDestNetworkSubnet(m_pnlDestNetwork->GetSubnet()))
		{
			ErrMsg(wxT("Error setting destination subnet"));
			success = false;
		}
	}

	if (success)
	{
		if (!m_proxy_settings->SetDestPortsMode((CryptSocketProxyDestMode)m_pnlDestPorts->GetSelection()))
		{
			ErrMsg(wxT("Error setting destination ports mode"));
			success = false;
		}
	}

	if (success)
	{
		if (!m_proxy_settings->SetDestPortRanges(m_pnlDestPorts->GetString()))
		{
			ErrMsg(wxT("Error setting destination ports"));
			success = false;
		}
	}

	if (success)
	{
		if (!m_proxy_settings->SaveSettings())
		{
			ErrMsg(wxT("Error saving proxy settings"));
			success = false;
		}
		else
		{
			((ClientUIMDIFrame*)GetParent())->GetClient()->NewProxySettings();
		}
	}
	

	if (!m_config->Flush())
	{
		if (success)
		{
			ErrMsg(wxT("Error flushing settings to disk"));
			success = false;
		}
	}

	return success;

}
