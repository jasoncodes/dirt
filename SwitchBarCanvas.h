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


#ifndef SwitchBarCanvas_H_
#define SwitchBarCanvas_H_

class SwitchBarParent;
class SwitchBar;

class SwitchBarCanvas : public wxPanel
{

	friend class SwitchBarParent;

public:
	SwitchBarCanvas(SwitchBarParent *parent, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);
	virtual ~SwitchBarCanvas();

	virtual wxString GetTitle();
	virtual void SetTitle(const wxString &title);

	virtual wxIcon GetIcon();
	virtual void SetIcon(const wxIcon &icon);

	virtual bool IsClosable() { return true; }
	virtual bool IsAttached();

	virtual void OnAttach() {}
	virtual void OnDetach() {}
	virtual void OnActivate() {}
	virtual void OnClose() {}
	virtual bool OnPopupMenu(wxMenu &WXUNUSED(menu)) { return true; }
	virtual bool OnPopupMenuItem(wxCommandEvent &WXUNUSED(event)) { return true; }

	SwitchBar* GetSwitchBar();

protected:
	bool m_saved_state_maximized;
	wxRect m_saved_state_rect;
	bool m_saved_state_valid;
	SwitchBarParent *m_parent;

private:
	DECLARE_EVENT_TABLE()
	DECLARE_NO_COPY_CLASS(SwitchBarCanvas)

};

#include <wx/dynarray.h>
#ifndef WX_DEFINE_ARRAY_PTR
	#define WX_DEFINE_ARRAY_PTR WX_DEFINE_ARRAY
#endif
WX_DEFINE_ARRAY_PTR(SwitchBarCanvas*, SwitchBarCanvasArray);

#endif
