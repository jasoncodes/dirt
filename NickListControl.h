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


#ifndef NickListControl_H_
#define NickListControl_H_

/**
 * EVT_MENU for right click
 * EVT_LISTBOX_DCLICK for double click
 *
 * Get the nick index via event.GetInt() and
 * use NickListControl::GetSelectedNick() to get the nick
 */
class NickListControl : public wxListBox
{

	static const wxString AwayPostfix;

public:

	NickListControl(wxWindow *parent, int id);
	virtual ~NickListControl();

	virtual void Add(const wxString &nick);
	virtual void Remove(const wxString &nick);
	virtual bool GetAway(const wxString &nick);
	virtual bool SetAway(const wxString &nick, bool away);
	virtual void Clear();
	virtual wxString GetNick(int index);
	virtual int GetCount();
	virtual int GetNickIndex(const wxString &nick);
	virtual int GetSelectedIndex();
	virtual wxString GetSelectedNick();
	virtual int HitTest(const wxPoint &pt);
	virtual void RenameNick(const wxString &old_nick, const wxString &new_nick);

protected:
	void OnRightUp(wxMouseEvent &event);
	void OnMotion(wxMouseEvent &event);

private:
	DECLARE_EVENT_TABLE()
	DECLARE_NO_COPY_CLASS(NickListControl)

};

#endif
