#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: NickListControl.cpp,v 1.18 2004-03-15 07:05:27 jason Exp $)

#include "NickListControl.h"
#include "util.h"

#ifdef __WXMSW__
#include <windows.h>
#include <wx/msw/winundef.h>
#endif

#ifdef __WXGTK__

#include <gdk/gdk.h>
#include <gtk/gtk.h>

static gint gtk_nicklist_button_press_callback(GtkWidget *widget, GdkEventButton *gdk_event, NickListControl *list)
{
	if (gdk_event->button == 3)
	{
		int sel = list->GtkGetIndex(widget);
		list->SetSelection(sel);
	}
	return FALSE;
}

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
#ifdef __WXGTK__
		int index = GetNickIndex(nick);
		wxASSERT(index > -1);
		GList *child = g_list_nth(m_list->children, index);
		GtkObject *obj = GTK_OBJECT(child->data);
		gtk_signal_connect_after(
			obj, "button_press_event",
			(GtkSignalFunc)gtk_nicklist_button_press_callback,
			(gpointer)this);
#endif
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
	return (index > -1) ? GetNick(index) : wxString(wxT(""));
}

int NickListControl::GetCount()
{
	return wxListBox::GetCount();
}

void NickListControl::RenameNick(const wxString &old_nick, const wxString &new_nick)
{
	bool is_away = GetAway(old_nick);
	Remove(old_nick);
	Add(new_nick);
	SetAway(new_nick, is_away);
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
