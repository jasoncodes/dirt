#ifndef HotKeyControl_H_
#define HotKeyControl_H_

class HotKeyControl : public wxTextCtrl
{

public:
	HotKeyControl(wxWindow *parent, int id,
		wxPoint pos = wxDefaultPosition, wxSize size = wxDefaultSize);
	virtual ~HotKeyControl();
 
	int GetKeyCode() const;
	wxUint8 GetModifiers() const;
	void SetValue(int keycode, wxUint8 modifiers);

	static wxString HotKeyToString(int keycode, wxUint8 modifiers);

	virtual bool AcceptsFocus() const;

protected:
	void OnKeyDown(wxKeyEvent &event);
	void OnKeyUp(wxKeyEvent &event);
	void OnKillFocus(wxFocusEvent &event);

protected:
	int m_keycode;
	wxUint8 m_modifiers;
	bool m_win_left, m_win_right;

private:
	DECLARE_EVENT_TABLE()

};

#endif
