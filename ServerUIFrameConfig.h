/*
    Copyright 2002, 2003 General Software Laboratories
    
    
    This file is part of Dirt Secure Chat.

    Dirt Secure Chat is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Dirt Secure Chat is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Dirt Secure Chat; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


#ifndef ServerUIFrameConfig_H_
#define ServerUIFrameConfig_H_

class ServerUIFrame;
class Server;
class TristateConfigPanel;

#include <wx/sound.h>

class ServerUIFrameConfig : public wxDialog
{

public:
	ServerUIFrameConfig(ServerUIFrame *parent, Server *server);
	virtual ~ServerUIFrameConfig();

protected:
	void OnOK(wxCommandEvent &event);
	void OnReset(wxCommandEvent &event);
	void OnChangeText(wxCommandEvent &event);
	void OnChangeCheck(wxCommandEvent &event);
	void OnTimer(wxTimerEvent &event);
	void OnSoundText(wxCommandEvent &event);
	void OnBrowse(wxCommandEvent &event);
	void OnPreview(wxCommandEvent &event);

protected:
	virtual void LoadSettings();
	virtual bool SaveSettings();
	virtual void ReportError(const wxString &error_message, wxTextCtrl *txt);
	virtual void ReportError(const wxString &error_message, wxCheckBox *chk);

protected:
	Server *m_server;

	#if wxUSE_SOUND
		wxSound m_sound;
	#endif

	wxTextCtrl *m_txtListenPort;
	wxTextCtrl *m_txtUserPassword;
	wxTextCtrl *m_txtAdminPassword;
	wxTextCtrl *m_txtMaxUsers;
	wxTextCtrl *m_txtMaxUsersIP;
	wxTextCtrl *m_txtServerName;
	wxTextCtrl *m_txtHostname;
	wxTextCtrl *m_txtSoundConnection;
	wxTextCtrl *m_txtSoundJoin;

	wxBitmapButton *m_cmdSoundConnectionPreview, *m_cmdSoundJoinPreview;

	TristateConfigPanel *m_pnlLog;

	wxCheckBox *m_chkLogPublicMessages;

	wxCheckBox *m_chkPublicListEnabled;
	wxStaticText *m_lblPublicListAuthentication;
	wxTextCtrl *m_txtPublicListAuthentication;
	wxStaticText *m_lblPublicListComment;
	wxTextCtrl *m_txtPublicListComment;

	wxCheckBox *m_chkHTTPProxyEnabled;
	wxStaticText *m_lblHTTPProxyHostname;
	wxTextCtrl *m_txtHTTPProxyHostname;
	wxStaticText *m_lblHTTPProxyPort;
	wxTextCtrl *m_txtHTTPProxyPort;
	wxStaticText *m_lblHTTPProxyUsername;
	wxTextCtrl *m_txtHTTPProxyUsername;
	wxStaticText *m_lblHTTPProxyPassword;
	wxTextCtrl *m_txtHTTPProxyPassword;

	wxButton *m_cmdOK;
	wxButton *m_cmdApply;

	wxTimer *m_tmrNextUpdate;
	wxStaticText *m_lblNextUpdate;

private:
	DECLARE_EVENT_TABLE()
	DECLARE_NO_COPY_CLASS(ServerUIFrameConfig)

};

#endif
