#ifndef _ClientUIMDICanvas_H
#define _ClientUIMDICanvas_H

#include "SwitchBarCanvas.h"
#include "LogControl.h"
#include "InputControl.h"

class ClientUIMDICanvas : public SwitchBarCanvas
{

public:

	ClientUIMDICanvas(SwitchBarParent *parent, wxIcon icon, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);
	virtual ~ClientUIMDICanvas();

	virtual bool IsClosable() { return closable; }
	bool closable;

	LogControl* GetLog() { return m_txtLog; }

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

	static int s_num;

protected:
	LogControl *m_txtLog;
	InputControl *m_txtInput;

private:
	DECLARE_EVENT_TABLE()

};

#endif
