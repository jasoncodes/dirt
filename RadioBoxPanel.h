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


#ifndef RadioBoxPanel_H_
#define RadioBoxPanel_H_

class RadioBoxPanel : public wxPanel
{

public:
	RadioBoxPanel(wxWindow *parent, wxWindowID id = -1,
		const wxString &caption = wxT("RadioBox Panel"),
		const wxPoint& pos = wxDefaultPosition,
		size_t num = 0, const wxString *choices = 0);
	virtual ~RadioBoxPanel();

	virtual int GetSelection() const;
	virtual void SetSelection(int n);

	virtual bool Enable(bool enabled);
	virtual void Enable(int n, bool enabled);

#ifdef __WXMSW__
	virtual long MSWWindowProc(WXUINT nMsg, WXWPARAM wParam, WXLPARAM lParam);
#endif

protected:
	void OnSize(wxSizeEvent &event);
	void OnRadio(wxCommandEvent &event);

protected:
	virtual wxPanel *GetPanel() const { return m_pnl; }
	virtual void SetSizes(); // call after panel is populated and has a sizer

	virtual void OnSelectionChanged(int n) = 0;
	virtual void SendChangeEvent();

protected:
	wxRadioBox *m_boxRadio;
	wxPanel *m_pnl;
	bool m_sizes_set;

private:
	DECLARE_EVENT_TABLE()
	DECLARE_NO_COPY_CLASS(RadioBoxPanel)

};

#endif
