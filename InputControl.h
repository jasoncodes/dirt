// to handle events from this control use EVT_TEXT_ENTER()

#ifndef InputControl_H_
#define InputControl_H_

class InputControlColourPopup;

static const wxString CtrlEnterPrefix = wxT("/say ");

class InputControl : public wxTextCtrl
{

public:
	virtual wxSize DoGetBestSize() const;
	InputControl(
		wxWindow* parent, wxWindowID id,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize);

	virtual ~InputControl();

	void ClosePopup();
	void RemoveLastHistoryEntry();
	void SetTabCompletionList(wxArrayString *tab_completion_list);

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

};

#endif
