#ifndef ClientUIMDICanvas_H_
#define ClientUIMDICanvas_H_

#include "SwitchBarCanvas.h"
#include "LogControl.h"
#include "InputControl.h"
#include "ClientUIMDITransferPanel.h"
#include <wx/sashwin.h>

class NickListControl;
class Client;

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

	ClientUIMDICanvas(SwitchBarParent *parent, const wxString &title, CanvasType type);
	virtual ~ClientUIMDICanvas();

	virtual bool IsClosable() { return m_type != ChannelCanvas; }

	virtual CanvasType GetType() { return m_type; }

	LogControl* GetLog() { return m_txtLog; }
	NickListControl* GetNickList() { return m_lstNickList; }
	ClientUIMDITransferPanel* GetTransferPanel() { return m_pnlTransfer; }
	Client* GetClient();

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

	virtual void OnAttach();
	virtual void OnActivate();
	virtual void OnClose();

	void ResizeChildren();
	void DoGotFocus();

protected:
	CanvasType m_type;

	LogControl *m_txtLog;
	InputControl *m_txtInput;

	wxSashWindow *m_sash;
	NickListControl *m_lstNickList;

	ClientUIMDITransferPanel *m_pnlTransfer;

private:
	DECLARE_EVENT_TABLE()

};

#endif
