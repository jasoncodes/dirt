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


#ifndef ClientUIMDIConfigDialog_H_
#define ClientUIMDIConfigDialog_H_

class ClientUIMDIFrame;
class ClientConfig;
class TristateConfigPanel;
class DestNetworkPanel;
class DestPortsPanel;
class CryptSocketProxySettings;
class HotKeyControl;
class FontControl;

class ClientUIMDIConfigDialog : public wxDialog
{

public:
	ClientUIMDIConfigDialog(ClientUIMDIFrame *parent);
	virtual ~ClientUIMDIConfigDialog();

protected:
	void OnOK(wxCommandEvent &event);
	void OnProxy(wxCommandEvent &event);

protected:
	virtual void LoadSettings();
	virtual bool SaveSettings();
	bool ErrMsg(const wxString &msg);

protected:
	ClientConfig *m_config;
	CryptSocketProxySettings *m_proxy_settings;

	wxStaticBox *m_fraNickname;
	wxTextCtrl *m_txtNickname;

	TristateConfigPanel *m_pnlLog;

	wxStaticBox *m_fraHotKey;
	HotKeyControl *m_HotKey[2];

	wxStaticBox *m_fraFont;
	FontControl *m_font;

	wxStaticBox *m_fraNotification;
	wxCheckBox *m_chkTaskbarNotification;
	wxCheckBox *m_chkFileTransferStatus;
	wxComboBox *m_cmbSystemTrayIcon;
	TristateConfigPanel *m_pnlSound;
	TristateConfigPanel *m_pnlBrowser;

	wxCheckBox *m_chkProxy;
	wxStaticText *m_lblProtocol;
	wxComboBox *m_cmbProtocol;
	wxStaticText *m_lblHostname, *m_lblPort, *m_lblUsername, *m_lblPassword;
	wxTextCtrl *m_txtHostname, *m_txtPort, *m_txtUsername, *m_txtPassword;
	wxStaticBox *m_fraProxyTypes;
	wxCheckBox *m_chkTypeServer, *m_chkTypeDCCConnect, *m_chkTypeDCCListen;
	DestNetworkPanel *m_pnlDestNetwork;
	DestPortsPanel *m_pnlDestPorts;

private:
	DECLARE_EVENT_TABLE()
	DECLARE_NO_COPY_CLASS(ClientUIMDIConfigDialog)

};

#endif
