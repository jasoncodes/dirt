#ifndef ClientUIMDIConfigDialog_H_
#define ClientUIMDIConfigDialog_H_

class ClientUIMDIFrame;
class ClientConfig;
class TristateConfigPanel;

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
	TristateConfigPanel *m_pnlLog;
	TristateConfigPanel *m_pnlSound;

private:
	DECLARE_EVENT_TABLE()

};

#endif
