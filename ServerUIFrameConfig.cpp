#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: ServerUIFrameConfig.cpp,v 1.43 2003-06-27 11:34:45 jason Exp $)

#include "ServerUIFrame.h"
#include "ServerUIFrameConfig.h"
#include "TristateConfigPanel.h"
#include <wx/filename.h>
#include "StaticCheckBoxSizer.h"

#include "res/sound.xpm"
#include "res/sound_disabled.xpm"

enum
{
	ID_RESET = 1,
	ID_TIMER,
	ID_LOG,
	ID_SOUND_CONNECTION,
	ID_SOUND_JOIN,
	ID_BROWSE_SOUND_CONNECTION,
	ID_BROWSE_SOUND_JOIN,
	ID_PREVIEW_SOUND_CONNECTION,
	ID_PREVIEW_SOUND_JOIN
};

BEGIN_EVENT_TABLE(ServerUIFrameConfig, wxDialog)
	EVT_BUTTON(wxID_OK, ServerUIFrameConfig::OnOK)
	EVT_BUTTON(wxID_APPLY, ServerUIFrameConfig::OnOK)
	EVT_BUTTON(ID_RESET, ServerUIFrameConfig::OnReset)
	EVT_TIMER(ID_TIMER, ServerUIFrameConfig::OnTimer)
	EVT_TEXT(ID_SOUND_CONNECTION, ServerUIFrameConfig::OnSoundText)
	EVT_TEXT(ID_SOUND_JOIN, ServerUIFrameConfig::OnSoundText)
	EVT_BUTTON(ID_BROWSE_SOUND_CONNECTION, ServerUIFrameConfig::OnBrowse)
	EVT_BUTTON(ID_BROWSE_SOUND_JOIN, ServerUIFrameConfig::OnBrowse)
	EVT_BUTTON(ID_PREVIEW_SOUND_CONNECTION, ServerUIFrameConfig::OnPreview)
	EVT_BUTTON(ID_PREVIEW_SOUND_JOIN, ServerUIFrameConfig::OnPreview)
	EVT_TEXT(wxID_ANY, ServerUIFrameConfig::OnChangeText)
	EVT_CHECKBOX(wxID_ANY, ServerUIFrameConfig::OnChangeCheck)
END_EVENT_TABLE()

ServerUIFrameConfig::ServerUIFrameConfig(ServerUIFrame *parent, Server *server)
	: wxDialog(parent, -1, wxT("Server Configuration"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxCLIP_CHILDREN | wxNO_FULL_REPAINT_ON_RESIZE | wxTAB_TRAVERSAL)
{
	
	m_server = server;
	wxPanel *panel = new wxPanel(this, -1, wxDefaultPosition, wxDefaultSize);

	m_cmdOK = new wxButton(panel, wxID_OK, wxT("OK"));
	wxButton *cmdCancel = new wxButton(panel, wxID_CANCEL, wxT("Cancel"));
	m_cmdApply = new wxButton(panel, wxID_APPLY, wxT("&Apply"));
	wxButton *cmdReset = new wxButton(panel, ID_RESET, wxT("&Reset"));

	wxStaticText *m_lblNextUpdateLabel = new wxStaticText(panel, -1, wxT("Next update:"));
	m_lblNextUpdate = new wxStaticText(panel, -1, wxT("N/A"));

	wxStaticText *lblServerName = new wxStaticText(panel, -1, wxT("&Server Name:"));
	m_txtServerName = new wxTextCtrl(panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	FixBorder(m_txtServerName);

	wxStaticText *lblHostname = new wxStaticText(panel, -1, wxT("&Hostname:"));
	m_txtHostname = new wxTextCtrl(panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	FixBorder(m_txtHostname);

	wxStaticText *lblUserPassword = new wxStaticText(panel, -1, wxT("&User Password:"));
	m_txtUserPassword = new wxTextCtrl(panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD);
	FixBorder(m_txtUserPassword);
	
	wxStaticText *lblAdminPassword = new wxStaticText(panel, -1, wxT("A&dmin Password:"));
	m_txtAdminPassword = new wxTextCtrl(panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD);
	FixBorder(m_txtAdminPassword);
	
	wxBitmap bmpSound(sound_xpm);
	wxBitmap bmpSoundDisabled (sound_disabled_xpm);
	wxSize size_button = wxSize(m_txtAdminPassword->GetBestSize().y, m_txtAdminPassword->GetBestSize().y);

	wxStaticText *lblSoundConnection = new wxStaticText(panel, -1, wxT("Connection Sound:"));
	m_txtSoundConnection = new wxTextCtrl(panel, ID_SOUND_CONNECTION, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	FixBorder(m_txtSoundConnection);
	wxButton *cmdSoundConnection = new wxButton(panel, ID_BROWSE_SOUND_CONNECTION, wxT("..."), wxDefaultPosition, size_button);
	m_cmdSoundConnectionPreview = new wxBitmapButton(panel, ID_PREVIEW_SOUND_CONNECTION, bmpSound, wxDefaultPosition, size_button);
	m_cmdSoundConnectionPreview->SetBitmapDisabled(sound_disabled_xpm);
	
	wxStaticText *lblSoundJoin = new wxStaticText(panel, -1, wxT("&Join Sound:"));
	m_txtSoundJoin = new wxTextCtrl(panel, ID_SOUND_JOIN, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	FixBorder(m_txtSoundJoin);
	wxButton *cmdSoundJoin = new wxButton(panel, ID_BROWSE_SOUND_JOIN, wxT("..."), wxDefaultPosition, size_button);
	m_cmdSoundJoinPreview = new wxBitmapButton(panel, ID_PREVIEW_SOUND_JOIN, bmpSound, wxDefaultPosition, size_button);
	m_cmdSoundJoinPreview->SetBitmapDisabled(sound_disabled_xpm);

	wxStaticText *lblListenPort = new wxStaticText(panel, -1, wxT("Listen &Port:"));
	m_txtListenPort = new wxTextCtrl(panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	FixBorder(m_txtListenPort);

	wxStaticText *lblMaxUsers = new wxStaticText(panel, -1, wxT("Max &Users:"));
	m_txtMaxUsers = new wxTextCtrl(panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	FixBorder(m_txtMaxUsers);

	wxStaticText *lblMaxUsersIP = new wxStaticText(panel, -1, wxT("Max Users per &IP:"));
	m_txtMaxUsersIP = new wxTextCtrl(panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	FixBorder(m_txtMaxUsersIP);

	m_pnlLog = new TristateConfigPanel(panel, ID_LOG, wxT("Log Directory"));

	#if wxUSE_WAVE
	#else
		lblSoundConnection->Enable(false);
		m_txtSoundConnection->Enable(false);
		cmdSoundConnection->Enable(false);
		m_cmdSoundConnectionPreview->Enable(false);
		lblSoundJoin->Enable(false);
		m_txtSoundJoin->Enable(false);
		cmdSoundJoin->Enable(false);
		m_cmdSoundJoinPreview->Enable(false);
	#endif

	m_chkPublicListEnabled = new wxCheckBox(panel, -1, wxT("&Public List"));
	m_lblPublicListAuthentication = new wxStaticText(panel, -1, wxT("&Authentication:"));
	m_txtPublicListAuthentication = new wxTextCtrl(panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD);
	FixBorder(m_txtPublicListAuthentication);
	m_lblPublicListComment = new wxStaticText(panel, -1, wxT("&Comment:"));
	m_txtPublicListComment = new wxTextCtrl(panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	FixBorder(m_txtPublicListComment);

	m_chkHTTPProxyEnabled = new wxCheckBox(panel, -1, wxT("&HTTP Proxy"));
	m_lblHTTPProxyHostname = new wxStaticText(panel, -1, wxT("Hostname:"));
	m_txtHTTPProxyHostname = new wxTextCtrl(panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	FixBorder(m_txtHTTPProxyHostname);
	m_lblHTTPProxyPort = new wxStaticText(panel, -1, wxT("Port:"));
	m_txtHTTPProxyPort = new wxTextCtrl(panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(48, m_txtHTTPProxyHostname->GetSize().y), 0);
	FixBorder(m_txtHTTPProxyPort);
	m_lblHTTPProxyUsername = new wxStaticText(panel, -1, wxT("Username:"));
	m_txtHTTPProxyUsername = new wxTextCtrl(panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(64,-1), 0);
	FixBorder(m_txtHTTPProxyUsername);
	m_lblHTTPProxyPassword = new wxStaticText(panel, -1, wxT("Password:"));
	m_txtHTTPProxyPassword = new wxTextCtrl(panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(64,-1), wxTE_PASSWORD);
	FixBorder(m_txtHTTPProxyPassword);

	wxBoxSizer *szrAll = new wxBoxSizer(wxHORIZONTAL);
	{

		wxBoxSizer *szrLeft = new wxBoxSizer(wxVERTICAL);
		{

			wxBoxSizer *szrLeftTop = new wxBoxSizer(wxHORIZONTAL);
			{

				wxFlexGridSizer *szrLeftTopLeft = new wxFlexGridSizer(2, 8, 8);
				szrLeftTopLeft->AddGrowableCol(1);
				{

					szrLeftTopLeft->Add(lblServerName, 0, wxALIGN_CENTER_VERTICAL);
					szrLeftTopLeft->Add(m_txtServerName, 0, wxEXPAND);

					szrLeftTopLeft->Add(lblHostname, 0, wxALIGN_CENTER_VERTICAL);
					szrLeftTopLeft->Add(m_txtHostname, 0, wxEXPAND);

					szrLeftTopLeft->Add(lblUserPassword, 0, wxALIGN_CENTER_VERTICAL);
					szrLeftTopLeft->Add(m_txtUserPassword, 0, wxEXPAND);

					szrLeftTopLeft->Add(lblAdminPassword, 0, wxALIGN_CENTER_VERTICAL);
					szrLeftTopLeft->Add(m_txtAdminPassword, 0, wxEXPAND);

					szrLeftTopLeft->Add(lblSoundConnection, 0, wxALIGN_CENTER_VERTICAL);
					wxBoxSizer *szrSoundConnection = new wxBoxSizer(wxHORIZONTAL);
					{
						szrSoundConnection->Add(m_txtSoundConnection, 1, wxEXPAND, 0);
						szrSoundConnection->Add(cmdSoundConnection, 0, 0, 0);
						szrSoundConnection->Add(m_cmdSoundConnectionPreview, 0, 0, 0);
					}
					szrLeftTopLeft->Add(szrSoundConnection, 0, wxEXPAND);

					szrLeftTopLeft->Add(lblSoundJoin, 0, wxALIGN_CENTER_VERTICAL);
					wxBoxSizer *szrSoundJoin = new wxBoxSizer(wxHORIZONTAL);
					{
						szrSoundJoin->Add(m_txtSoundJoin, 1, wxEXPAND, 0);
						szrSoundJoin->Add(cmdSoundJoin, 0, 0, 0);
						szrSoundJoin->Add(m_cmdSoundJoinPreview, 0, 0, 0);
					}
					szrLeftTopLeft->Add(szrSoundJoin, 0, wxEXPAND);
				
				}
				szrLeftTop->Add(szrLeftTopLeft, 3, wxRIGHT | wxBOTTOM, 8);

				wxBoxSizer *szrLeftTopRight = new wxBoxSizer(wxVERTICAL);
				{

					wxFlexGridSizer *szrLeftTopRightTop = new wxFlexGridSizer(2, 8, 8);
					szrLeftTopRightTop->AddGrowableCol(1);
					{

							szrLeftTopRightTop->Add(lblListenPort, 0, wxALIGN_CENTER_VERTICAL);
							szrLeftTopRightTop->Add(m_txtListenPort, 0, wxEXPAND);
							
							szrLeftTopRightTop->Add(lblMaxUsers, 0, wxALIGN_CENTER_VERTICAL);
							szrLeftTopRightTop->Add(m_txtMaxUsers, 0, wxEXPAND);

							szrLeftTopRightTop->Add(lblMaxUsersIP, 0, wxALIGN_CENTER_VERTICAL);
							szrLeftTopRightTop->Add(m_txtMaxUsersIP, 0, wxEXPAND);

					}
					szrLeftTopRight->Add(szrLeftTopRightTop, 0, wxEXPAND | wxBOTTOM, 8);
				
					szrLeftTopRight->Add(m_pnlLog, 0, wxEXPAND, 0);

				}
				szrLeftTop->Add(szrLeftTopRight, 2, 0, 0);

			}
			szrLeft->Add(szrLeftTop, 0, wxEXPAND, 0);

			wxBoxSizer *szrLeftBottom = new wxBoxSizer(wxHORIZONTAL);
			{

				wxStaticBox *fraLeftPublic = new wxStaticBox(panel, -1, wxString(wxT(' '), 22));
				wxBoxSizer *szrLeftPublic = new StaticCheckBoxSizer(fraLeftPublic, m_chkPublicListEnabled, wxHORIZONTAL);
				{

					wxFlexGridSizer *szrLeftPublicInner = new wxFlexGridSizer(2, 8, 8);
					szrLeftPublicInner->AddGrowableCol(1);
					{

						szrLeftPublicInner->Add(m_lblPublicListAuthentication, 0, wxALIGN_CENTER_VERTICAL);
						szrLeftPublicInner->Add(m_txtPublicListAuthentication, 0, wxEXPAND);
						
						szrLeftPublicInner->Add(m_lblPublicListComment, 0, wxALIGN_CENTER_VERTICAL);
						szrLeftPublicInner->Add(m_txtPublicListComment, 0, wxEXPAND);

					}
					szrLeftPublic->Add(szrLeftPublicInner, 1, wxEXPAND, 0);
				
				}
				szrLeftBottom->Add(szrLeftPublic, 3, wxRIGHT, 8);

				wxStaticBox *fraLeftProxy = new wxStaticBox(panel, -1, wxString(wxT(' '), 24));
				wxBoxSizer *szrLeftProxy = new StaticCheckBoxSizer(fraLeftProxy, m_chkHTTPProxyEnabled, wxVERTICAL);
				{

					wxFlexGridSizer *szrRow1 = new wxFlexGridSizer(4, 8, 8);
					szrRow1->AddGrowableCol(1);
					{
						szrRow1->Add(m_lblHTTPProxyHostname, 0, wxALIGN_CENTER_VERTICAL);
						szrRow1->Add(m_txtHTTPProxyHostname, 0, wxEXPAND);
						szrRow1->Add(m_lblHTTPProxyPort, 0, wxALIGN_CENTER_VERTICAL);
						szrRow1->Add(m_txtHTTPProxyPort, 0, 0);
					}
					szrLeftProxy->Add(szrRow1, 0, wxEXPAND | wxBOTTOM, 8);

					wxFlexGridSizer *szrRow2 = new wxFlexGridSizer(4, 8, 8);
					szrRow2->AddGrowableCol(1);
					szrRow2->AddGrowableCol(3);
					{
						szrRow2->Add(m_lblHTTPProxyUsername, 0, wxALIGN_CENTER_VERTICAL);
						szrRow2->Add(m_txtHTTPProxyUsername, 0, wxEXPAND);
						szrRow2->Add(m_lblHTTPProxyPassword, 0, wxALIGN_CENTER_VERTICAL);
						szrRow2->Add(m_txtHTTPProxyPassword, 0, wxEXPAND);
					}
					szrLeftProxy->Add(szrRow2, 0, wxEXPAND, 0);

				}
				szrLeftBottom->Add(szrLeftProxy, 4, 0, 0);

			}
			szrLeft->Add(szrLeftBottom, 0, wxEXPAND, 0);

			wxBoxSizer *szrLeftFill = new wxBoxSizer(wxHORIZONTAL);
			{
			}
			szrLeft->Add(szrLeftFill, 1, wxEXPAND, 0);

		}
		szrAll->Add(szrLeft, 1, wxLEFT | wxTOP | wxEXPAND, 8);

		wxBoxSizer *szrRight = new wxBoxSizer(wxVERTICAL);
		{
			szrRight->Add(m_cmdOK, 0, wxTOP | wxBOTTOM | wxEXPAND, 8);
			szrRight->Add(cmdCancel, 0, wxBOTTOM | wxEXPAND, 8);
			szrRight->Add(m_cmdApply, 0, wxBOTTOM | wxEXPAND, 8);
			szrRight->Add(cmdReset, 0, wxBOTTOM | wxEXPAND, 8);
			szrRight->Add(m_lblNextUpdateLabel, 0, wxEXPAND, 0);
			szrRight->Add(m_lblNextUpdate, 0, wxBOTTOM | wxEXPAND, 8);
		}
		szrAll->Add(szrRight, 0, wxALL | wxEXPAND, 8);

	}

	LoadSettings();

	m_tmrNextUpdate = new wxTimer(this, ID_TIMER);
	m_tmrNextUpdate->Start(500);
	wxTimerEvent tmr_event;
	OnTimer(tmr_event);
	
	panel->SetAutoLayout(TRUE);
	panel->SetSizer(szrAll);
	szrAll->SetSizeHints(this);

	FitInside();
	CentreOnParent();
	m_cmdOK->SetDefault();
	ShowModal();

}

ServerUIFrameConfig::~ServerUIFrameConfig()
{
	delete m_tmrNextUpdate;
}

void ServerUIFrameConfig::OnOK(wxCommandEvent &event)
{
	if (SaveSettings())
	{
		m_server->ResetPublicListUpdate(5, true);
		event.Skip();
	}
}

void ServerUIFrameConfig::OnReset(wxCommandEvent &event)
{
	if (wxMessageBox(wxT("Warning: This will reset the server configuration to default values. This operation is not undoable.\n\nAre you sure you want to reset to defaults?"), wxT("Reset Confirmation"), wxOK|wxCANCEL|wxICON_QUESTION, this) == wxOK)
	{
		ServerConfig &config = m_server->GetConfig();
		if (config.ResetToDefaults())
		{
			LoadSettings();
		}
		else
		{
			wxMessageBox(wxT("Unable to reset to defaults"), wxT("Error"), wxOK|wxICON_ERROR, this);
		}
	}
}

void ServerUIFrameConfig::OnChangeText(wxCommandEvent &event)
{
	m_cmdApply->Enable(true);
}

void ServerUIFrameConfig::OnChangeCheck(wxCommandEvent &event)
{
	m_cmdApply->Enable(true);
	m_lblPublicListAuthentication->Enable(m_chkPublicListEnabled->IsChecked());
	m_txtPublicListAuthentication->Enable(m_chkPublicListEnabled->IsChecked());
	m_lblPublicListComment->Enable(m_chkPublicListEnabled->IsChecked());
	m_txtPublicListComment->Enable(m_chkPublicListEnabled->IsChecked());
	m_lblHTTPProxyHostname->Enable(m_chkHTTPProxyEnabled->IsChecked());
	m_txtHTTPProxyHostname->Enable(m_chkHTTPProxyEnabled->IsChecked());
	m_lblHTTPProxyPort->Enable(m_chkHTTPProxyEnabled->IsChecked());
	m_txtHTTPProxyPort->Enable(m_chkHTTPProxyEnabled->IsChecked());
	m_lblHTTPProxyUsername->Enable(m_chkHTTPProxyEnabled->IsChecked());
	m_txtHTTPProxyUsername->Enable(m_chkHTTPProxyEnabled->IsChecked());
	m_lblHTTPProxyPassword->Enable(m_chkHTTPProxyEnabled->IsChecked());
	m_txtHTTPProxyPassword->Enable(m_chkHTTPProxyEnabled->IsChecked());
}

void ServerUIFrameConfig::OnTimer(wxTimerEvent &event)
{
	wxLongLong_t next_tick = m_server->GetNextPublicListUpdateTick();
	wxLongLong_t now = GetMillisecondTicks();
	if (next_tick == 0)
	{
		m_lblNextUpdate->SetLabel(wxT("N/A"));
	}
	else
	{
		wxLongLong_t time_left = next_tick - now;
		if (time_left < 0) time_left = 0;
		m_lblNextUpdate->SetLabel(SecondsToMMSS((long)(time_left/1000)));
	}
}

void ServerUIFrameConfig::OnSoundText(wxCommandEvent &event)
{
	
	wxTextCtrl *txt;
	wxBitmapButton *cmd;
	
	if (event.GetId() == ID_SOUND_CONNECTION)
	{
		txt = m_txtSoundConnection;
		cmd = m_cmdSoundConnectionPreview;
	}
	else
	{
		txt = m_txtSoundJoin;
		cmd = m_cmdSoundJoinPreview;
	}

	cmd->Enable(txt->IsEnabled() && txt->GetValue().Length());

	OnChangeText(event);

}

void ServerUIFrameConfig::OnBrowse(wxCommandEvent &event)
{
	wxTextCtrl *txt =
		(event.GetId()==ID_BROWSE_SOUND_CONNECTION) ?
		m_txtSoundConnection : m_txtSoundJoin;
	wxFileName fn(txt->GetValue());
	wxFileDialog dlg(this, wxT("Select Sound File"), fn.GetPath(wxPATH_GET_VOLUME|wxPATH_GET_SEPARATOR), fn.GetFullName(), wxT("Wave Files (*.wav)|*.wav|All Files (*.*)|*.*"), wxOPEN|wxHIDE_READONLY|wxFILE_MUST_EXIST, wxDefaultPosition);
	if (dlg.ShowModal() == wxID_OK)
	{
		txt->SetValue(dlg.GetPath());
	}
}

void ServerUIFrameConfig::OnPreview(wxCommandEvent &event)
{

	#if wxUSE_WAVE

		wxTextCtrl *txt =
			(event.GetId()==ID_PREVIEW_SOUND_CONNECTION) ?
			m_txtSoundConnection : m_txtSoundJoin;

		wxString filename = txt->GetValue();

		if (wxFileName(filename).FileExists())
		{
			m_wave.Create(filename, false);
			if (m_wave.IsOk() && m_wave.Play())
			{
				return;
			}
		}

		wxMessageBox(wxT("Error playing wave file: ") + filename, wxT("Unable to play wave file"), wxICON_ERROR);

	#else

		wxMessageBox(wxT("Wave file support not available"), wxT("Unable to play wave file"), wxICON_ERROR);

	#endif

}

void ServerUIFrameConfig::LoadSettings()
{

	ServerConfig &config = m_server->GetConfig();

	m_txtListenPort->SetValue(wxString()<<config.GetListenPort());
	m_txtUserPassword->SetValue(config.GetUserPassword(false));
	m_txtAdminPassword->SetValue(config.GetAdminPassword(false));
	m_txtMaxUsers->SetValue(wxString()<<config.GetMaxUsers());
	m_txtMaxUsersIP->SetValue(wxString()<<config.GetMaxUsersIP());
	m_txtSoundConnection->SetValue(config.GetSoundConnection());
	m_txtSoundJoin->SetValue(config.GetSoundJoin());
	m_txtServerName->SetValue(config.GetServerName());
	m_txtHostname->SetValue(config.GetHostname());

	m_pnlLog->SetMode(config.GetLogDirType());
	if (config.GetLogDirType() == Config::tsmCustom)
	{
		m_pnlLog->SetPath(config.GetActualLogDir());
	}

	m_chkPublicListEnabled->SetValue(config.GetPublicListEnabled());
	m_txtPublicListAuthentication->SetValue(config.GetPublicListAuthentication(false));
	m_txtPublicListComment->SetValue(config.GetPublicListComment());
	
	m_chkHTTPProxyEnabled->SetValue(config.GetHTTPProxyEnabled());
	m_txtHTTPProxyHostname->SetValue(config.GetHTTPProxyHostname());
	m_txtHTTPProxyPort->SetValue(wxString()<<config.GetHTTPProxyPort());
	m_txtHTTPProxyUsername->SetValue(config.GetHTTPProxyUsername());
	m_txtHTTPProxyPassword->SetValue(config.GetHTTPProxyPassword(false));
	
	wxCommandEvent evt;
	OnChangeCheck(evt);
	m_cmdApply->Enable(false);
	m_cmdOK->SetFocus();

}

void ServerUIFrameConfig::ReportError(const wxString &error_message, wxTextCtrl *txt)
{
	wxMessageBox(error_message, wxT("Error"), wxOK|wxICON_ERROR, this);
	if (txt)
	{
		txt->SetFocus();
		txt->SetSelection(-1, -1);
	}
}

void ServerUIFrameConfig::ReportError(const wxString &error_message, wxCheckBox *chk)
{
	wxMessageBox(error_message, wxT("Error"), wxOK|wxICON_ERROR, this);
	if (chk)
	{
		chk->SetFocus();
	}
}

bool ServerUIFrameConfig::SaveSettings()
{
	ServerConfig &config = m_server->GetConfig();
	long x;
	if (!m_txtListenPort->GetValue().ToLong(&x) || !config.SetListenPort(x))
	{
		ReportError(wxT("Invalid listen port"), m_txtListenPort);
		return false;
	}
	if (!config.SetUserPassword(m_txtUserPassword->GetValue()))
	{
		ReportError(wxT("Invalid user password"), m_txtUserPassword);
		return false;
	}
	if (!config.SetAdminPassword(m_txtAdminPassword->GetValue()))
	{
		ReportError(wxT("Invalid admin password"), m_txtAdminPassword);
		return false;
	}
	if (!m_txtMaxUsers->GetValue().ToLong(&x) || !config.SetMaxUsers(x))
	{
		ReportError(wxT("Invalid max users"), m_txtMaxUsers);
		return false;
	}
	if (!m_txtMaxUsersIP->GetValue().ToLong(&x) || !config.SetMaxUsersIP(x))
	{
		ReportError(wxT("Invalid max users per IP"), m_txtMaxUsersIP);
		return false;
	}
	if (!config.SetServerName(m_txtServerName->GetValue()))
	{
		ReportError(wxT("Invalid server name"), m_txtServerName);
		return false;
	}
	if (!config.SetHostname(m_txtHostname->GetValue()))
	{
		ReportError(wxT("Invalid hostname"), m_txtHostname);
		return false;
	}
	if (!config.SetSoundConnection(m_txtSoundConnection->GetValue()))
	{
		ReportError(wxT("Invalid connection sound"), m_txtSoundConnection);
		return false;
	}
	if (!config.SetSoundJoin(m_txtSoundJoin->GetValue()))
	{
		ReportError(wxT("Invalid join sound"), m_txtSoundJoin);
		return false;
	}
	if (!config.SetPublicListEnabled(m_chkPublicListEnabled->GetValue()))
	{
		ReportError(wxT("Invalid public list enabled"), m_chkPublicListEnabled);
		return false;
	}
	if (!config.SetPublicListAuthentication(m_txtPublicListAuthentication->GetValue()))
	{
		ReportError(wxT("Invalid public list authentication"), m_txtPublicListAuthentication);
		return false;
	}
	if (!config.SetPublicListComment(m_txtPublicListComment->GetValue()))
	{
		ReportError(wxT("Invalid public list comment"), m_txtPublicListComment);
		return false;
	}
	if (!config.SetHTTPProxyEnabled(m_chkHTTPProxyEnabled->GetValue()))
	{
		ReportError(wxT("Invalid HTTP proxy enabled"), m_chkHTTPProxyEnabled);
		return false;
	}
	if (!config.SetHTTPProxyHostname(m_txtHTTPProxyHostname->GetValue()))
	{
		ReportError(wxT("Invalid XXX"), m_txtHTTPProxyHostname);
		return false;
	}
	if (!m_txtHTTPProxyPort->GetValue().ToLong(&x) || !config.SetHTTPProxyPort(x))
	{
		ReportError(wxT("Invalid HTTP proxy port"), m_txtHTTPProxyPort);
		return false;
	}
	if (!config.SetHTTPProxyUsername(m_txtHTTPProxyUsername->GetValue()))
	{
		ReportError(wxT("Invalid HTTP proxy username"), m_txtHTTPProxyUsername);
		return false;
	}
	if (!config.SetHTTPProxyPassword(m_txtHTTPProxyPassword->GetValue()))
	{
		ReportError(wxT("Invalid HTTP proxy password"), m_txtHTTPProxyPassword);
		return false;
	}
	bool bLogChanged = (config.GetLogDirType() != m_pnlLog->GetMode());
	if (!bLogChanged && m_pnlLog->GetMode() == Config::tsmCustom)
	{
		bLogChanged = config.GetActualLogDir() != m_pnlLog->GetPath();
	}
	if (bLogChanged)
	{
		if (!config.SetLogDir(m_pnlLog->GetMode(), m_pnlLog->GetPath()))
		{
			ReportError(wxT("Invalid log directory"), (wxTextCtrl*)NULL);
			return false;
		}
		m_server->InitLog();
	}
	config.Flush();
	m_cmdApply->Enable(false);
	return true;
}
