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


#ifndef ClientUIMDICanvas_H_
#define ClientUIMDICanvas_H_

#include "SwitchBarMDI.h"
#include <wx/sashwin.h>

class NickListControl;
class LogControl;
class InputControl;
class Client;
class ClientUIMDIFrame;
class ClientUIMDITransferPanel;
class LogWriter;
class FileDropEvent;

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
	wxTextCtrl* GetPassword() { return m_txtPassword; }
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
	void SendFiles(const wxString &nickname, const wxArrayString &filenames);
	void InitLog();
	bool SetFont(const wxFont &font);

	virtual void OnFocusCheck();

protected:
	void OnSize(wxSizeEvent &event);
	void OnFocus(wxFocusEvent &event);
	void OnInputEnter(wxCommandEvent &event);
	void OnLinkClicked(wxCommandEvent &event);
	void OnSashDragged(wxSashEvent &event);
	void OnFileDrop(FileDropEvent &event);
	void OnNickListDblClick(wxCommandEvent &event);
	void OnNickListMenu(wxCommandEvent &event);
	void OnNickListMenuItem(wxCommandEvent &event);
	void OnPasswordEnter(wxCommandEvent &event);
	void OnMouseWheel(wxMouseEvent &event);
	void OnKeyDown(wxKeyEvent &event);

	virtual void OnAttach();
	virtual void OnDetach();
	virtual void OnActivate();
	virtual void OnClose();
	virtual bool OnPopupMenu(wxMenu &menu);
	virtual bool OnPopupMenuItem(wxCommandEvent &event);

	void DoGotFocus();
	void ListenToInput(wxWindow *wnd);

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
	DECLARE_NO_COPY_CLASS(ClientUIMDICanvas)

};

#endif
