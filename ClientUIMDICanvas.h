#ifndef ClientUIMDICanvas_H_
#define ClientUIMDICanvas_H_

#include "SwitchBarCanvas.h"
#include <wx/sashwin.h>

class NickListControl;
class LogControl;
class InputControl;
class Client;
class ClientUIMDIFrame;
class ClientUIMDITransferPanel;
class LogWriter;

enum CanvasType
{
	ChannelCanvas,
	QueryCanvas,
	TransferSendCanvas,
	TransferReceiveCanvas
};

class ClientUIMDICanvas : public SwitchBarCanvas
{

public:

	ClientUIMDICanvas(ClientUIMDIFrame *parent, const wxString &title, CanvasType type);
	virtual ~ClientUIMDICanvas();

	virtual bool IsClosable() { return m_type != ChannelCanvas; }

	virtual CanvasType GetType() { return m_type; }

	LogControl* GetLog() { return m_txtLog; }
	NickListControl* GetNickList() { return m_lstNickList; }
	InputControl* GetInput() { return m_txtInput; }
	ClientUIMDITransferPanel* GetTransferPanel() { return m_pnlTransfer; }
	Client* GetClient();
	bool GetPasswordMode() const;
	void SetPasswordMode(bool value);
	LogWriter* GetLogWriter() const { return m_log; }
	bool GetLogWriterWarningShown() const { return m_log_warning_shown; }
	void SetLogWriterWarningShown(bool log_warning_shown) { m_log_warning_shown = log_warning_shown; }

	void ProcessInput(const wxString &text);

	void ResizeChildren();
	void LogControlTest();

protected:
	void OnSize(wxSizeEvent &event);
	void OnFocus(wxFocusEvent &event);
	void OnInputEnter(wxCommandEvent &event);
	void OnLinkClicked(wxCommandEvent &event);
	void OnSashDragged(wxSashEvent &event);
	void OnNickListDblClick(wxCommandEvent &event);
	void OnNickListMenu(wxCommandEvent &event);
	void OnNickListMenuItem(wxCommandEvent &event);
	void OnPasswordEnter(wxCommandEvent &event);

	virtual void OnAttach();
	virtual void OnDetach();
	virtual void OnActivate();
	virtual void OnClose();
	virtual bool OnPopupMenu(wxMenu &menu);
	virtual bool OnPopupMenuItem(wxCommandEvent &event);

	void DoGotFocus();

protected:
	CanvasType m_type;

	LogControl *m_txtLog;
	InputControl *m_txtInput;
	wxTextCtrl *m_txtPassword;

	wxSashWindow *m_sash;
	NickListControl *m_lstNickList;

	ClientUIMDITransferPanel *m_pnlTransfer;

	LogWriter *m_log;
	bool m_log_warning_shown;

private:
	DECLARE_EVENT_TABLE()

};

#endif
