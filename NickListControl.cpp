#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif

#include "NickListControl.h"
#include "util.h"

BEGIN_EVENT_TABLE(NickListControl, wxListBox)
	EVT_RIGHT_UP(NickListControl::OnRightUp)
	EVT_MOTION(NickListControl::OnMotion)
END_EVENT_TABLE()

NickListControl::NickListControl(wxWindow *parent, int id)
	: wxListBox(
		parent, id,
		wxDefaultPosition, wxDefaultSize,
		0, NULL,
		wxLB_SINGLE | wxLB_SORT | wxLB_HSCROLL | wxLB_NEEDED_SB)
{
	FixBorder(this);
}

NickListControl::~NickListControl()
{
}

void NickListControl::Add(const wxString &nick)
{
	Append(nick);
}

void NickListControl::Remove(const wxString &nick)
{
	int index = GetNickIndex(nick);
	if (index > -1)
	{
		Delete(index);
	}
}

int NickListControl::GetNickIndex(const wxString &nick)
{
	for (int i = 0; i < GetCount(); ++i)
	{
		if (GetNick(i) == nick)
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
	return wxListBox::GetString(index);
}

int NickListControl::GetSelectedIndex()
{
	return wxListBox::GetSelection();
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
		
		if (index > -1)
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
