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
