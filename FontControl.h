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


#ifndef FontControl_H_
#define FontControl_H_

class FontControl : public wxPanel
{

public:
	FontControl(
		wxWindow *parent, int id,
		wxPoint pos = wxDefaultPosition, wxSize size = wxDefaultSize);
	virtual ~FontControl();

	virtual wxFont GetFont() const;
	virtual bool SetFont(const wxFont &font);

protected:
	void OnBrowse(wxCommandEvent &event);

protected:
	wxFontData m_data;
	wxPanel *m_pnlPreview;

private:
	DECLARE_EVENT_TABLE()

};

#endif
