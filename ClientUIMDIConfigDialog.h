#ifndef ClientUIMDIConfigDialog_H_
#define ClientUIMDIConfigDialog_H_

class ClientUIMDIFrame;
class ClientConfig;
class TristateConfigPanel;
class DestNetworkPanel;
class DestPortsPanel;

class ClientUIMDIConfigDialog : public wxDialog
{

public:
	ClientUIMDIConfigDialog(ClientUIMDIFrame *parent);
	virtual ~ClientUIMDIConfigDialog();

protected:
	void OnOK(wxCommandEvent &event);

protected:
	virtual void LoadSettings();
	virtual bool SaveSettings();

protected:
	ClientConfig *m_config;

	wxCheckBox *m_chkProxy;
	wxStaticText *m_lblProtocol;
	wxComboBox *m_cmbProtocol;
	wxStaticText *m_lblHostname, *m_lblPort, *m_lblUsername, *m_lblPassword;
	wxTextCtrl *m_txtHostname, *m_txtPort, *m_txtUsername, *m_txtPassword;
	wxStaticBox *m_fraProxyTypes;

	wxCheckBox *m_chkTypeServer, *m_chkTypeDCCConnect, *m_chkTypeDCCListen;

	DestNetworkPanel *m_pnlDestNetwork;
	DestPortsPanel *m_pnlDestPorts;

	TristateConfigPanel *m_pnlLog;
	TristateConfigPanel *m_pnlSound;

private:
	DECLARE_EVENT_TABLE()

};

#endif
