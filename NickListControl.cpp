#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: NickListControl.cpp,v 1.15 2003-05-14 16:10:36 jason Exp $)

#include "NickListControl.h"
#include "util.h"

#ifdef __WXMSW__
	#include <windows.h>
	#include <wx/msw/winundef.h>
#endif

const wxString NickListControl::AwayPostfix = wxT(" (Away)");

BEGIN_EVENT_TABLE(NickListControl, wxListBox)
	EVT_RIGHT_UP(NickListControl::OnRightUp)
	EVT_MOTION(NickListControl::OnMotion)
END_EVENT_TABLE()

NickListControl::NickListControl(wxWindow *parent, int id)
	: wxListBox(
		parent, id,
		wxDefaultPosition, wxDefaultSize,
		0, NULL,
		wxLB_SINGLE | wxLB_SORT | wxLB_HSCROLL | wxLB_NEEDED_SB | wxSUNKEN_BORDER)
{
	FixBorder(this);
	SetCursor(*wxSTANDARD_CURSOR);
}

NickListControl::~NickListControl()
{
}

void NickListControl::Add(const wxString &nick)
{
	if (GetNickIndex(nick) == -1)
	{
		Append(nick);
	}
}

void NickListControl::Remove(const wxString &nick)
{
	int index = GetNickIndex(nick);
	if (index > -1)
	{
		Delete(index);
	}
}

bool NickListControl::GetAway(const wxString &nick)
{
	int index = GetNickIndex(nick);
	if (index > -1)
	{
		wxString nick = wxListBox::GetString(index);
		return RightEq(nick, AwayPostfix);
	}
	else
	{
		return false;
	}
}

bool NickListControl::SetAway(const wxString &nick, bool away)
{
	int index = GetNickIndex(nick);
	if (index > -1)
	{
		wxListBox::SetString(index, GetNick(index) + (away?AwayPostfix:(const wxString)wxEmptyString));
		return true;
	}
	else
	{
		return false;
	}
}

int NickListControl::GetNickIndex(const wxString &nick)
{
	for (int i = 0; i < GetCount(); ++i)
	{
		if (GetNick(i).CmpNoCase(nick) == 0)
		{
			return i;
		}
	}
	return -1;
}

void NickListControl::Clear()
{
	wxListBox::Clear();
}

wxString NickListControl::GetNick(int index)
{
	wxString nick = wxListBox::GetString(index);
	if (RightEq(nick, AwayPostfix))
	{
		nick = nick.Left(nick.Length() - AwayPostfix.Length());
	}
	return nick;
}

int NickListControl::GetSelectedIndex()
{
	return wxListBox::GetSelection();
}

wxString NickListControl::GetSelectedNick()
{
	int index = GetSelectedIndex();
	return (index > -1) ? GetNick(index) : wxT("");
}

int NickListControl::GetCount()
{
	return wxListBox::GetCount();
}

int NickListControl::HitTest(const wxPoint &pt)
{
	#ifdef __WXMSW__

		HWND hWnd = (HWND)GetHandle();
		LPARAM lParam = MAKELPARAM(pt.x, pt.y);
		LRESULT lResult = ::SendMessage(hWnd, LB_ITEMFROMPOINT, 0, lParam);
		int index = LOWORD(lResult);
		bool inside = (HIWORD(lResult) == 0);
		if (inside)
		{
			return index;
		}
		else
		{
			return -1;
		}

	#else

		return -2; // Not available on this platform
		
	#endif

}

void NickListControl::OnMotion(wxMouseEvent &event)
{

	if (event.LeftIsDown() || event.RightIsDown())
	{

		int index = HitTest(event.GetPosition());
		
		if (index > -1 && GetSelection() != index)
		{
			SetSelection(index);
		}

	}

}

void NickListControl::OnRightUp(wxMouseEvent &event)
{

	int index = HitTest(event.GetPosition());

	if (index > -1)
	{
		SetSelection(index);
	}
	else if (index != -2) // -2 is a special return val for "not implemented"
	{
		return; // if not right clicking over anything, return
	}

	if (GetSelectedIndex() > -1)
	{
		wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED);
		evt.SetId(GetId());
		evt.SetInt(GetSelectedIndex());
		evt.SetString(GetNick(GetSelectedIndex()));
		ProcessEvent(evt);
	}

}
