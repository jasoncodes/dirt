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
