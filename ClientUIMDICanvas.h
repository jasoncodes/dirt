#ifndef _ClientUIMDICanvas_H
#define _ClientUIMDICanvas_H

#include "SwitchBarCanvas.h"
#include "LogControl.h"
#include "InputControl.h"

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

	LogControl* GetLog() { return m_txtLog; }

	virtual CanvasType GetType() { return m_type; }

	void LogControlTest();

protected:
	void OnSize(wxSizeEvent& event);
	void OnFocus(wxFocusEvent &event);
	void OnInputEnter(wxCommandEvent& event);
	void OnLinkClicked(wxCommandEvent& event);
	virtual void OnAttach();
	virtual void OnActivate();

	void ResizeChildren();
	void DoGotFocus();

protected:
	CanvasType m_type;

	LogControl *m_txtLog;
	InputControl *m_txtInput;

	wxListBox *m_lstNickList;

private:
	DECLARE_EVENT_TABLE()

};

#endif
