// to handle events from this control use EVT_TEXT_ENTER()

#ifndef InputControl_H_
#define InputControl_H_

class InputControlColourPopup;

static const wxString CtrlEnterPrefix = "/say ";

class InputControl : public wxTextCtrl
{

public:
	virtual wxSize GetBestSize();
	InputControl(
		wxWindow* parent, wxWindowID id,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize);

	virtual ~InputControl();

	void OnChange(wxCommandEvent &event);
	void OnEnterPress(wxCommandEvent &event);	
	void OnKeyDown(wxKeyEvent& event);
	void OnKeyUp(wxKeyEvent& event);
	void OnChar(wxKeyEvent& event);
	void ClosePopup();

protected:
	wxArrayString history;
	size_t history_pos;
	InputControlColourPopup *popup;
	wxTextCtrl *txtBestSize;
	bool m_ctrl_down;

	void AddToHistory(const wxString &line);
	bool ModifierCheck(wxKeyEvent& event);
	void AddModifier(const wxString &modifier, bool alternate_caret_pos);
	void DisplayHistory();
	void ProcessInput();
	void RaiseEvent(const wxString &text);
	void ShowPopup();
	void MaybeClosePopup(wxKeyEvent& event);
	bool IsPrintable(const wxString &text);

private:
	DECLARE_EVENT_TABLE()

};

#endif
