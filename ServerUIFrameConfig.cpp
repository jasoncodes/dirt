#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: ServerUIFrameConfig.cpp,v 1.24 2003-03-05 01:05:14 jason Exp $)

#include "ServerUIFrameConfig.h"

class StaticCheckBoxSizerEventHandler : public wxEvtHandler
{

public:
	StaticCheckBoxSizerEventHandler(wxStaticBox *box, wxCheckBox *chk)
		: wxEvtHandler(), m_box(box), m_chk(chk)
	{
		Connect(wxID_ANY, wxID_ANY, wxEVT_PAINT, (wxObjectEventFunction)(wxEventFunction)(wxPaintEventFunction)&StaticCheckBoxSizerEventHandler::OnPaint);
	}

	virtual ~StaticCheckBoxSizerEventHandler()
	{
	}

protected:
	void OnPaint(wxPaintEvent &event)
	{
		m_chk->Refresh();
		event.Skip();
	}

protected:
	wxStaticBox *m_box;
	wxCheckBox *m_chk;

};

class StaticCheckBoxSizer : public wxStaticBoxSizer
{


public:
	StaticCheckBoxSizer(wxStaticBox *box, wxCheckBox *chk, int orient)
		: wxStaticBoxSizer(box, orient), m_box(box), m_chk(chk)
	{
		m_evt = new StaticCheckBoxSizerEventHandler(box, chk);
		m_box->PushEventHandler(m_evt);
	};

	virtual ~StaticCheckBoxSizer()
	{
		delete m_evt;
	}

    virtual void RecalcSizes()
	{
		wxStaticBoxSizer::RecalcSizes();
		if (m_chk)
		{
			wxPoint pos = m_box->GetPosition();
			#ifdef __WXGTK__
				if (m_box->GetLabel().Length())
				{
					pos.y -= 8;
				}
				else
				{
					pos.y -= 12;
				}
			#endif
			#ifdef __WXMSW__
				pos.x += 12;
			#else
				pos.x += 8;
			#endif
			m_chk->Move(pos);
			m_box->Lower();
			m_chk->Raise();
			m_chk->SetSize(m_chk->GetBestSize());
		}
	}

protected:
	wxStaticBox *m_box;
	wxCheckBox *m_chk;
	wxEvtHandler *m_evt;

};

enum
{
	ID_RESET = 1,
	ID_TIMER
};

BEGIN_EVENT_TABLE(ServerUIFrameConfig, wxDialog)
	EVT_BUTTON(wxID_OK, ServerUIFrameConfig::OnOK)
	EVT_BUTTON(wxID_APPLY, ServerUIFrameConfig::OnOK)
	EVT_BUTTON(ID_RESET, ServerUIFrameConfig::OnReset)
	EVT_TEXT(wxID_ANY, ServerUIFrameConfig::OnChange)
	EVT_CHECKBOX(wxID_ANY, ServerUIFrameConfig::OnChange)
	EVT_TIMER(ID_TIMER, ServerUIFrameConfig::OnTimer)
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

	wxStaticText *m_lblNextUpdateLabel = new wxStaticText(panel, -1, wxT("Next update:"));
	m_lblNextUpdate = new wxStaticText(panel, -1, wxT("N/A"));

	wxStaticText *lblListenPort = new wxStaticText(panel, -1, wxT("Listen &Port:"));
	m_txtListenPort = new wxTextCtrl(panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	FixBorder(m_txtListenPort);
	wxStaticText *lblUserPassword = new wxStaticText(panel, -1, wxT("&User Password:"));
	m_txtUserPassword = new wxTextCtrl(panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(250, m_txtListenPort->GetSize().y), wxTE_PASSWORD);
	FixBorder(m_txtUserPassword);
	wxStaticText *lblAdminPassword = new wxStaticText(panel, -1, wxT("A&dmin Password:"));
	m_txtAdminPassword = new wxTextCtrl(panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD);
	FixBorder(m_txtAdminPassword);
	wxStaticText *lblMaxUsers = new wxStaticText(panel, -1, wxT("Max &Users:"));
	m_txtMaxUsers = new wxTextCtrl(panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	FixBorder(m_txtMaxUsers);
	wxStaticText *lblMaxUsersIP = new wxStaticText(panel, -1, wxT("Max Users per &IP:"));
	m_txtMaxUsersIP = new wxTextCtrl(panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	FixBorder(m_txtMaxUsersIP);
	wxStaticText *lblServerName = new wxStaticText(panel, -1, wxT("&Server Name:"));
	m_txtServerName = new wxTextCtrl(panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	FixBorder(m_txtServerName);
	wxStaticText *lblHostname = new wxStaticText(panel, -1, wxT("&Hostname:"));
	m_txtHostname = new wxTextCtrl(panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	FixBorder(m_txtHostname);

	m_chkPublicListEnabled = new wxCheckBox(panel, -1, wxT("&Public List"));
	wxStaticText *lblPublicListAuthentication = new wxStaticText(panel, -1, wxT("&Authentication:"));
	m_txtPublicListAuthentication = new wxTextCtrl(panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD);
	FixBorder(m_txtPublicListAuthentication);
	wxStaticText *lblPublicListComment = new wxStaticText(panel, -1, wxT("&Comment:"));
	m_txtPublicListComment = new wxTextCtrl(panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	FixBorder(m_txtPublicListComment);

	m_chkHTTPProxyEnabled = new wxCheckBox(panel, -1, wxT("&HTTP Proxy"));
	wxStaticText *lblHTTPProxyHostname = new wxStaticText(panel, -1, wxT("Hostname:"));
	m_txtHTTPProxyHostname = new wxTextCtrl(panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	FixBorder(m_txtHTTPProxyHostname);
	wxStaticText *lblHTTPProxyPort = new wxStaticText(panel, -1, wxT("Port:"));
	m_txtHTTPProxyPort = new wxTextCtrl(panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(48, m_txtHTTPProxyHostname->GetSize().y), 0);
	FixBorder(m_txtHTTPProxyPort);
	wxStaticText *lblHTTPProxyUsername = new wxStaticText(panel, -1, wxT("Username:"));
	m_txtHTTPProxyUsername = new wxTextCtrl(panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	FixBorder(m_txtHTTPProxyUsername);
	wxStaticText *lblHTTPProxyPassword = new wxStaticText(panel, -1, wxT("Password:"));
	m_txtHTTPProxyPassword = new wxTextCtrl(panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD);
	FixBorder(m_txtHTTPProxyPassword);

	wxBoxSizer *szrAll = new wxBoxSizer(wxHORIZONTAL);
	{

		wxBoxSizer *szrLeft = new wxBoxSizer(wxVERTICAL);
		{

			wxBoxSizer *szrLeftTop = new wxBoxSizer(wxHORIZONTAL);
			{

				wxBoxSizer *szrLabels = new wxBoxSizer(wxVERTICAL);
				{
					szrLabels->Add(lblListenPort, 1, wxBOTTOM, 8);
					szrLabels->Add(lblUserPassword, 1, wxBOTTOM, 8);
					szrLabels->Add(lblAdminPassword, 1, wxBOTTOM, 8);
					szrLabels->Add(lblMaxUsers, 1, wxBOTTOM, 8);
					szrLabels->Add(lblMaxUsersIP, 1, wxBOTTOM, 8);
					szrLabels->Add(lblServerName, 1, wxBOTTOM, 8);
					szrLabels->Add(lblHostname, 1, wxBOTTOM, 8);
				}
				szrLeftTop->Add(szrLabels, 0, wxEXPAND | wxRIGHT, 8);

				wxBoxSizer *szrTextBoxes = new wxBoxSizer(wxVERTICAL);
				{
					szrTextBoxes->Add(m_txtListenPort, 1, wxBOTTOM | wxEXPAND, 8);
					szrTextBoxes->Add(m_txtUserPassword, 1, wxBOTTOM | wxEXPAND, 8);
					szrTextBoxes->Add(m_txtAdminPassword, 1, wxBOTTOM | wxEXPAND, 8);
					szrTextBoxes->Add(m_txtMaxUsers, 1, wxBOTTOM | wxEXPAND, 8);
					szrTextBoxes->Add(m_txtMaxUsersIP, 1, wxBOTTOM | wxEXPAND, 8);
					szrTextBoxes->Add(m_txtServerName, 1, wxBOTTOM | wxEXPAND, 8);
					szrTextBoxes->Add(m_txtHostname, 1, wxBOTTOM | wxEXPAND, 8);
				}
				szrLeftTop->Add(szrTextBoxes, 1, wxEXPAND, 0);

			}
			szrLeft->Add(szrLeftTop, 0, wxEXPAND, 0);

			wxStaticBox *fraLeftPublic = new wxStaticBox(panel, -1, wxString(wxT(' '), 22));
			wxBoxSizer *szrLeftPublic = new StaticCheckBoxSizer(fraLeftPublic, m_chkPublicListEnabled, wxHORIZONTAL);
			{

				wxBoxSizer *szrLabels = new wxBoxSizer(wxVERTICAL);
				{
					szrLabels->Add(lblPublicListAuthentication, 1, wxBOTTOM, 4);
					szrLabels->Add(lblPublicListComment, 1, wxTOP, 4);
				}
				szrLeftPublic->Add(szrLabels, 0, wxEXPAND | wxRIGHT, 8);

				wxBoxSizer *szrTextBoxes = new wxBoxSizer(wxVERTICAL);
				{
					szrTextBoxes->Add(m_txtPublicListAuthentication, 1, wxBOTTOM | wxEXPAND, 4);
					szrTextBoxes->Add(m_txtPublicListComment, 1, wxTOP | wxEXPAND, 4);
				}
				szrLeftPublic->Add(szrTextBoxes, 1, wxEXPAND, 0);
			
			}
			szrLeft->Add(szrLeftPublic, 0, wxEXPAND | wxBOTTOM, 8);

			wxStaticBox *fraLeftProxy = new wxStaticBox(panel, -1, wxString(wxT(' '), 24));
			wxBoxSizer *szrLeftProxy = new StaticCheckBoxSizer(fraLeftProxy, m_chkHTTPProxyEnabled, wxVERTICAL);
			{

				wxBoxSizer *szrRow1 = new wxBoxSizer(wxHORIZONTAL);
				{
					szrRow1->Add(lblHTTPProxyHostname, 0, wxRIGHT, 8);
					szrRow1->Add(m_txtHTTPProxyHostname, 1, wxRIGHT, 8);
					szrRow1->Add(lblHTTPProxyPort, 0, wxRIGHT, 8);
					szrRow1->Add(m_txtHTTPProxyPort, 0, 0, 0);
				}
				szrLeftProxy->Add(szrRow1, 0, wxEXPAND | wxBOTTOM, 8);

				wxBoxSizer *szrRow2 = new wxBoxSizer(wxHORIZONTAL);
				{
					szrRow2->Add(lblHTTPProxyUsername, 0, wxRIGHT, 8);
					szrRow2->Add(m_txtHTTPProxyUsername, 1, wxRIGHT, 8);
					szrRow2->Add(lblHTTPProxyPassword, 0, wxRIGHT, 8);
					szrRow2->Add(m_txtHTTPProxyPassword, 1, 0, 0);
				}
				szrLeftProxy->Add(szrRow2, 0, wxEXPAND, 0);

			}
			szrLeft->Add(szrLeftProxy, 0, wxEXPAND, 0);

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
	szrAll->SetSizeHints( this );

	FitInside();
	CentreOnParent();
	cmdOK->SetDefault();
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

void ServerUIFrameConfig::LoadSettings()
{
	ServerConfig *config = m_server->GetConfig();
	m_txtListenPort->SetValue(wxString()<<config->GetListenPort());
	m_txtUserPassword->SetValue(config->GetUserPassword(false));
	m_txtAdminPassword->SetValue(config->GetAdminPassword(false));
	m_txtMaxUsers->SetValue(wxString()<<config->GetMaxUsers());
	m_txtMaxUsersIP->SetValue(wxString()<<config->GetMaxUsersIP());
	m_txtServerName->SetValue(config->GetServerName());
	m_txtHostname->SetValue(config->GetHostname());
	m_chkPublicListEnabled->SetValue(config->GetPublicListEnabled());
	m_txtPublicListAuthentication->SetValue(config->GetPublicListAuthentication(false));
	m_txtPublicListComment->SetValue(config->GetPublicListComment());
	m_chkHTTPProxyEnabled->SetValue(config->GetHTTPProxyEnabled());
	m_txtHTTPProxyHostname->SetValue(config->GetHTTPProxyHostname());
	m_txtHTTPProxyPort->SetValue(wxString()<<config->GetHTTPProxyPort());
	m_txtHTTPProxyUsername->SetValue(config->GetHTTPProxyUsername());
	m_txtHTTPProxyPassword->SetValue(config->GetHTTPProxyPassword(false));
	m_cmdApply->Enable(false);
}

void ServerUIFrameConfig::ReportError(const wxString &error_message, wxTextCtrl *txt)
{
	wxMessageBox(error_message, wxT("Error"), wxOK|wxICON_ERROR, this);
	txt->SetFocus();
	txt->SetSelection(-1, -1);
}

void ServerUIFrameConfig::ReportError(const wxString &error_message, wxCheckBox *chk)
{
	wxMessageBox(error_message, wxT("Error"), wxOK|wxICON_ERROR, this);
	chk->SetFocus();
}

bool ServerUIFrameConfig::SaveSettings()
{
	ServerConfig *config = m_server->GetConfig();
	long x;
	if (!m_txtListenPort->GetValue().ToLong(&x) || !config->SetListenPort(x))
	{
		ReportError(wxT("Invalid listen port"), m_txtListenPort);
		return false;
	}
	if (!config->SetUserPassword(m_txtUserPassword->GetValue()))
	{
		ReportError(wxT("Invalid user password"), m_txtUserPassword);
		return false;
	}
	if (!config->SetAdminPassword(m_txtAdminPassword->GetValue()))
	{
		ReportError(wxT("Invalid admin password"), m_txtAdminPassword);
		return false;
	}
	if (!m_txtMaxUsers->GetValue().ToLong(&x) || !config->SetMaxUsers(x))
	{
		ReportError(wxT("Invalid max users"), m_txtMaxUsers);
		return false;
	}
	if (!m_txtMaxUsersIP->GetValue().ToLong(&x) || !config->SetMaxUsersIP(x))
	{
		ReportError(wxT("Invalid max users per IP"), m_txtMaxUsersIP);
		return false;
	}
	if (!config->SetServerName(m_txtServerName->GetValue()))
	{
		ReportError(wxT("Invalid server name"), m_txtServerName);
		return false;
	}
	if (!config->SetHostname(m_txtHostname->GetValue()))
	{
		ReportError(wxT("Invalid hostname"), m_txtHostname);
		return false;
	}
	if (!config->SetPublicListEnabled(m_chkPublicListEnabled->GetValue()))
	{
		ReportError(wxT("Invalid public list enabled"), m_chkPublicListEnabled);
		return false;
	}
	if (!config->SetPublicListAuthentication(m_txtPublicListAuthentication->GetValue()))
	{
		ReportError(wxT("Invalid public list authentication"), m_txtPublicListAuthentication);
		return false;
	}
	if (!config->SetPublicListComment(m_txtPublicListComment->GetValue()))
	{
		ReportError(wxT("Invalid public list comment"), m_txtPublicListComment);
		return false;
	}
	if (!config->SetHTTPProxyEnabled(m_chkHTTPProxyEnabled->GetValue()))
	{
		ReportError(wxT("Invalid HTTP proxy enabled"), m_chkHTTPProxyEnabled);
		return false;
	}
	if (!config->SetHTTPProxyHostname(m_txtHTTPProxyHostname->GetValue()))
	{
		ReportError(wxT("Invalid XXX"), m_txtHTTPProxyHostname);
		return false;
	}
	if (!m_txtHTTPProxyPort->GetValue().ToLong(&x) || !config->SetHTTPProxyPort(x))
	{
		ReportError(wxT("Invalid HTTP proxy port"), m_txtHTTPProxyPort);
		return false;
	}
	if (!config->SetHTTPProxyUsername(m_txtHTTPProxyUsername->GetValue()))
	{
		ReportError(wxT("Invalid HTTP proxy username"), m_txtHTTPProxyUsername);
		return false;
	}
	if (!config->SetHTTPProxyPassword(m_txtHTTPProxyPassword->GetValue()))
	{
		ReportError(wxT("Invalid HTTP proxy password"), m_txtHTTPProxyPassword);
		return false;
	}
	config->Flush();
	m_cmdApply->Enable(false);
	return true;
}
