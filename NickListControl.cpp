#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif

#include "NickListControl.h"
#include "util.h"

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
	for (int i = 0; i < GetCount(); ++i)
	{
		if (GetNick(i) == nick)
		{
			Delete(i);
			break;
		}
	}
}

void NickListControl::Clear()
{
	wxListBox::Clear();
}

wxString NickListControl::GetNick(int index)
{
	return GetString(index);
}

int NickListControl::GetCount()
{
	return wxListBox::GetCount();
}
