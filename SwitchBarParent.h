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


#ifndef SwitchBarParent_H_
#define SwitchBarParent_H_

enum
{
	ID_SWITCHBARPARENT_FIRST = 11000,
	ID_WINDOW_WINDOWS = ID_SWITCHBARPARENT_FIRST,
	ID_SWITCHBAR = ID_WINDOW_WINDOWS + 1000,
	ID_UPDATEWINDOWMENUTIMER,
	ID_WINDOW_MINIMIZE,
	ID_WINDOW_CLOSE,
	ID_WINDOW_CASCADE,
	ID_WINDOW_TILE,
	ID_WINDOW_ARRANGE,
	ID_WINDOW_NEXT,
	ID_WINDOW_PREV,
	ID_SWITCHBAR_RESTORE,
	ID_SWITCHBAR_MINIMIZE,
	ID_SWITCHBAR_CLOSE,
	ID_SWITCHBARPARENT_LAST
};

class SwitchBar;
class SwitchBarChild;
class SwitchBarCanvas;

class SwitchBarParent : public wxMDIParentFrame
{

	friend class SwitchBarChild;
	friend class SwitchBarCanvas;

public:
	SwitchBarParent(
		wxWindow *parent, const wxWindowID id, const wxString& title,
		const wxPoint& pos, const wxSize& size, const long style,
		const wxString& name = wxFrameNameStr);

	virtual ~SwitchBarParent();

	inline SwitchBar* GetSwitchBar() { return m_switchbar; }

	void FocusCanvas(SwitchBarCanvas *canvas);

protected:
	void OnSize(wxSizeEvent& event);
	void OnSwitchBar(wxCommandEvent& event);
	void OnSwitchBarMiddleClick(wxCommandEvent& event);
	void OnSwitchBarMenu(wxCommandEvent& event);
	void OnSwitchBarMenuItem(wxCommandEvent& event);

	void OnWindowMinimize(wxCommandEvent& event);
	void OnWindowClose(wxCommandEvent& event);
	void OnWindowCascade(wxCommandEvent& event);
	void OnWindowTile(wxCommandEvent& event);
	void OnWindowNext(wxCommandEvent& event);
	void OnWindowPrev(wxCommandEvent& event);
	void OnWindowWindows(wxCommandEvent& event);

	void OnUpdateWindowMenuTimer(wxTimerEvent &event);
	void OnUpdateWindowMenuIdle(wxIdleEvent &event);

protected:
	SwitchBarChild* NewWindow(SwitchBarCanvas *canvas, bool focus);
	void NextChild(bool bPrevious);
	void DoUpdateWindowMenu();
	void UpdateCheckMenuItem(int id, const wxString &label, bool enabled, bool checked);
	SwitchBarCanvas *GetActiveCanvas() const;
	void CloseCanvas(SwitchBarCanvas *canvas);
	size_t GetAcceleratorCount() const { return m_accelerator_count; }
	wxAcceleratorEntry *GetAccelerators() const { return m_accelerators; }

	void UpdateWindowMenu();

	inline wxMenu *GetWindowMenu() { return m_mnuWindow; }

private:
	DECLARE_EVENT_TABLE()
	DECLARE_NO_COPY_CLASS(SwitchBarParent)

protected:
	SwitchBar *m_switchbar;
	wxTimer *m_tmrUpdateWindowMenu;
	wxMenu *m_mnuWindow;
	int m_num_window_menus;
	int m_switchbar_popup_button_index;
	SwitchBarCanvas *m_switchbar_popup_canvas;
	wxAcceleratorEntry *m_accelerators;
	size_t m_accelerator_count;

};

#endif
