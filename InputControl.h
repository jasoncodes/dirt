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


// to handle events from this control use EVT_TEXT_ENTER()

#ifndef InputControl_H_
#define InputControl_H_

class InputControlColourPopup;

static const wxString CtrlEnterPrefix = wxT("/say ");

class InputControl : public wxTextCtrl
{

public:
	InputControl(
		wxWindow* parent, wxWindowID id,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize);

	virtual ~InputControl();

	virtual wxSize DoGetBestSize() const;

	void ClosePopup();
	void RemoveLastHistoryEntry();
	void SetTabCompletionList(wxArrayString *tab_completion_list);
	virtual bool SetFont(const wxFont &font);

protected:
	void OnChange(wxCommandEvent &event);
	void OnEnterPress(wxCommandEvent &event);	
	void OnKeyDown(wxKeyEvent& event);
	void OnKeyUp(wxKeyEvent& event);
	void OnChar(wxKeyEvent& event);
	void OnIdle(wxIdleEvent &event);

protected:
	void AddToHistory(const wxString &line);
	bool ModifierCheck(wxKeyEvent& event);
	void AddModifier(const wxString &modifier, bool alternate_caret_pos);
	void DisplayHistory();
	void ProcessInput();
	void RaiseEvent(const wxString &text);
	void ShowPopup();
	void MaybeClosePopup(wxKeyEvent& event);
	bool IsPrintable(const wxString &text);
	void Alert();

protected:
	wxArrayString m_history;
	size_t m_history_pos;
	InputControlColourPopup *m_popup;
	wxTextCtrl *m_txtBestSize;
	bool m_ctrl_down;
	wxArrayString *m_tab_completion_list;
	wxString m_tab_completion_prefix;
	bool m_ignore_change;

private:
	DECLARE_EVENT_TABLE()
	DECLARE_NO_COPY_CLASS(InputControl)

};

#endif
