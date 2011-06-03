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


#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: HotKeyControl.cpp,v 1.4 2004-06-19 02:04:36 jason Exp $)

#include "HotKeyControl.h"

#ifdef __WXMSW__
#include <windows.h>
#include <wx/msw/winundef.h>
#endif

BEGIN_EVENT_TABLE(HotKeyControl, wxTextCtrl)
	EVT_KEY_DOWN(HotKeyControl::OnKeyDown)
	EVT_KEY_UP(HotKeyControl::OnKeyUp)
	EVT_KILL_FOCUS(HotKeyControl::OnKillFocus)
END_EVENT_TABLE()

HotKeyControl::HotKeyControl(wxWindow *parent, int id, wxPoint pos, wxSize size)
	: wxTextCtrl(parent, id, wxEmptyString, pos, size)
{
	m_win_left = m_win_right = false;
	SetEditable(false);
	SetValue(0, 0);
}

HotKeyControl::~HotKeyControl()
{
}

int HotKeyControl::GetKeyCode() const
{
	return m_keycode;
}

wxUint8 HotKeyControl::GetModifiers() const
{
	return m_modifiers;
}

bool HotKeyControl::AcceptsFocus() const
{
    return wxControl::AcceptsFocus();
}

void HotKeyControl::OnKillFocus(wxFocusEvent &event)
{
	if (!m_keycode)
	{
		SetValue(0, 0);
	}
	event.Skip();
}

#if !wxUSE_HOTKEY
enum wxHotkeyModifier
{
    wxMOD_NONE = 0,
    wxMOD_ALT = 1,
    wxMOD_CONTROL = 2,
    wxMOD_SHIFT = 4,
    wxMOD_WIN = 8
};
#endif

wxString HotKeyControl::HotKeyToString(int keycode, wxUint8 modifiers)
{

	wxString str;

	if (modifiers & wxMOD_WIN)
	{
		str << wxT("Win + ");
	}
	if (modifiers & wxMOD_CONTROL)
	{
		str << wxT("Ctrl + ");
	}
	if (modifiers & wxMOD_ALT)
	{
		str << wxT("Alt + ");
	}
	if (modifiers & wxMOD_SHIFT)
	{
		str << wxT("Shift + ");
	}

	if (keycode == 0)
	{
		if (!str.Length())
		{
			str << wxT("None");
		}
	}
	else if (keycode >= 'A' && keycode <= 'Z')
	{
		str << wxChar(wxT('A')+(keycode-'A'));
	}
	else if (keycode >= '0' && keycode <= '9')
	{
		str << wxChar(wxT('0')+(keycode-'0'));
	}
	else if (keycode >= WXK_F1 && keycode <= WXK_F24)
	{
		str << wxT('F') << (keycode-WXK_F1+1);
	}
	else if (keycode >= WXK_NUMPAD0 && keycode <= WXK_NUMPAD9)
	{
		str << wxT("Num") << wxChar(wxT('0')+(keycode-WXK_NUMPAD0));
	}
	else
	{
		switch (keycode)
		{
			case WXK_BACK:
				str << wxT("Back");
				break;
			case WXK_TAB:
				str << wxT("Tab");
				break;
			case WXK_RETURN:
				str << wxT("Enter");
				break;
			case WXK_ESCAPE:
				str << wxT("Esc");
				break;
			case WXK_SPACE:
				str << wxT("Space");
				break;
			case WXK_DELETE:
				str << wxT("Del");
				break;
			case WXK_CANCEL:
				str << wxT("Cancel");
				break;
			case WXK_CLEAR:
				str << wxT("Clear");
				break;
			case WXK_SHIFT:
				str << wxT("Shift");
				break;
			case WXK_ALT:
				str << wxT("Alt");
				break;
			case WXK_CONTROL:
				str << wxT("Ctrl");
				break;
			case WXK_MENU:
				str << wxT("Menu");
				break;
			case WXK_PAUSE:
				str << wxT("Pause");
				break;
			case WXK_CAPITAL:
				str << wxT("Caps");
				break;
			case WXK_PAGEUP:
				str << wxT("PgUp");
				break;
			case WXK_PAGEDOWN:
				str << wxT("PgDn");
				break;
			case WXK_END:
				str << wxT("End");
				break;
			case WXK_HOME:
				str << wxT("Home");
				break;
			case WXK_LEFT:
				str << wxT("Left");
				break;
			case WXK_UP:
				str << wxT("Up");
				break;
			case WXK_RIGHT:
				str << wxT("Right");
				break;
			case WXK_DOWN:
				str << wxT("Down");
				break;
			case WXK_SELECT:
				str << wxT("Select");
				break;
			case WXK_PRINT:
				str << wxT("Print");
				break;
			case WXK_EXECUTE:
				str << wxT("Execute");
				break;
			case WXK_SNAPSHOT:
				str << wxT("Snapshot");
				break;
			case WXK_INSERT:
				str << wxT("Insert");
				break;
			case WXK_HELP:
				str << wxT("Help");
				break;
			case WXK_MULTIPLY:
				str << wxT("*");
				break;
			case WXK_ADD:
				str << wxT("+");
				break;
			case WXK_SEPARATOR:
				str << wxT("Separator");
				break;
			case WXK_SUBTRACT:
				str << wxT("-");
				break;
			case WXK_DECIMAL:
				str << wxT(".");
				break;
			case WXK_DIVIDE:
				str << wxT("/");
				break;
			case WXK_NUMLOCK:
				str << wxT("Num");
				break;
			case WXK_SCROLL:
				str << wxT("Scroll");
				break;
			case WXK_NUMPAD_SPACE:
				str << wxT("NumSpace");
				break;
			case WXK_NUMPAD_TAB:
				str << wxT("NumTab");
				break;
			case WXK_NUMPAD_ENTER:
				str << wxT("NumEnter");
				break;
			case WXK_NUMPAD_F1:
				str << wxT("NumF1");
				break;
			case WXK_NUMPAD_F2:
				str << wxT("NumF2");
				break;
			case WXK_NUMPAD_F3:
				str << wxT("NumF3");
				break;
			case WXK_NUMPAD_F4:
				str << wxT("NumF4");
				break;
			case WXK_NUMPAD_HOME:
				str << wxT("NumHome");
				break;
			case WXK_NUMPAD_LEFT:
				str << wxT("NumLeft");
				break;
			case WXK_NUMPAD_UP:
				str << wxT("NumUp");
				break;
			case WXK_NUMPAD_RIGHT:
				str << wxT("NumRight");
				break;
			case WXK_NUMPAD_DOWN:
				str << wxT("NumDown");
				break;
			case WXK_NUMPAD_PAGEUP:
				str << wxT("NumPgUp");
				break;
			case WXK_NUMPAD_PAGEDOWN:
				str << wxT("NumPgDn");
				break;
			case WXK_NUMPAD_END:
				str << wxT("NumEnd");
				break;
			case WXK_NUMPAD_BEGIN:
				str << wxT("NumBegin");
				break;
			case WXK_NUMPAD_INSERT:
				str << wxT("NumIns");
				break;
			case WXK_NUMPAD_DELETE:
				str << wxT("NumDel");
				break;
			case WXK_NUMPAD_EQUAL:
				str << wxT("Num=");
				break;
			case WXK_NUMPAD_MULTIPLY:
				str << wxT("Num*");
				break;
			case WXK_NUMPAD_ADD:
				str << wxT("Num+");
				break;
			case WXK_NUMPAD_SEPARATOR:
				str << wxT("NumSep");
				break;
			case WXK_NUMPAD_SUBTRACT:
				str << wxT("Num-");
				break;
			case WXK_NUMPAD_DECIMAL:
				str << wxT("Num.");
				break;
			case WXK_NUMPAD_DIVIDE:
				str << wxT("Num/");
				break;
#if wxCHECK_VERSION(2,5,0)
			case WXK_WINDOWS_LEFT:
				str << wxT("WinLeft");
				break;
			case WXK_WINDOWS_RIGHT:
				str << wxT("WinRight");
				break;
			case WXK_WINDOWS_MENU:
				str << wxT("WinMenu");
				break;
#endif
			default:
				if (wxIsprint(keycode))
				{
					str << (wxChar)keycode;
				}
				else
				{
					str << wxT("#") << keycode;
				}
		}
	}

	return str;

}

void HotKeyControl::SetValue(int keycode, wxUint8 modifiers)
{

	switch (modifiers)
	{

		case wxMOD_WIN:
		case wxMOD_WIN|wxMOD_SHIFT:
		case wxMOD_CONTROL|wxMOD_SHIFT:
		case wxMOD_CONTROL|wxMOD_ALT:
		case wxMOD_CONTROL|wxMOD_ALT|wxMOD_SHIFT:
			m_keycode = keycode;
			m_modifiers = modifiers;
			break;

		default:
			m_keycode = keycode;
			m_modifiers = keycode ? wxMOD_CONTROL|wxMOD_ALT : modifiers;

	}

	wxTextCtrl::SetValue(HotKeyToString(m_keycode, m_modifiers));
	SetInsertionPointEnd();

}

void HotKeyControl::OnKeyDown(wxKeyEvent &event)
{

	switch (event.GetKeyCode())
	{

		case WXK_TAB:
			event.Skip();
			return;

#if wxCHECK_VERSION(2,5,0)
		case WXK_WINDOWS_LEFT:
			m_win_left = true;
			break;

		case WXK_WINDOWS_RIGHT:
			m_win_right = true;
			break;
#endif

		default:
			break;

	}

#ifdef __WXMSW__
	bool win_left = (GetKeyState(VK_LWIN) & 0x8000) != 0;
	bool win_right = (GetKeyState(VK_RWIN) & 0x8000) != 0;
#else
	bool win_left = m_win_left;
	bool win_right = m_win_right;
#endif

	wxUint8 mods = 0;

	if (event.ControlDown())
	{
		mods |= wxMOD_CONTROL;
	}
	if (event.AltDown())
	{
		mods |= wxMOD_ALT;
	}
	if (event.ShiftDown())
	{
		mods |= wxMOD_SHIFT;
	}
	if (event.MetaDown() || win_left || win_right)
	{
		mods |= wxMOD_WIN;
	}

	int keycode = event.GetKeyCode();

	switch (keycode)
	{

		case WXK_CAPITAL:
		case WXK_NUMLOCK:
		case WXK_SCROLL:
			return;

		case WXK_SHIFT:
			if (mods == wxMOD_SHIFT)
			{
				return;
			}
			else
			{
				keycode = 0;
			}
			break;

		case WXK_ALT:
		case WXK_CONTROL:
		case WXK_MENU:
#if wxCHECK_VERSION(2,5,0)
		case WXK_WINDOWS_LEFT:
		case WXK_WINDOWS_RIGHT:
		case WXK_WINDOWS_MENU:
#endif
			keycode = 0;
			break;

	}

	if (!mods)
	{
		switch (event.GetKeyCode())
		{
			case 0:
			case WXK_ESCAPE:
			case WXK_DELETE:
			case WXK_BACK:
				SetValue(0, 0);
				return;
		}
	}

	SetValue(keycode, mods);

}

void HotKeyControl::OnKeyUp(wxKeyEvent &event)
{

	switch (event.GetKeyCode())
	{

#if wxCHECK_VERSION(2,5,0)
		case WXK_WINDOWS_LEFT:
			m_win_left = false;
			break;

		case WXK_WINDOWS_RIGHT:
			m_win_right = false;
			break;
#endif

		default:
			break;

	}

	if (m_keycode == 0)
	{
		SetValue(0, 0);
	}

}
