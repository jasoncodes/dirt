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


#ifndef TristateConfigPanel_H_
#define TristateConfigPanel_H_

#include "ConfigFile.h"
#include "RadioBoxPanel.h"
#include <wx/sound.h>

class SquareButton : public wxButton
{

public:
	SquareButton(wxWindow *window, int id, const wxString &text)
		: wxButton(window, id, text)
	{
		wxSize size = GetSize();
		SetSize(size.y, size.y);
		SetBestSize(GetSize());
	}

};

class SquareBitmapButton : public wxBitmapButton
{

public:
	SquareBitmapButton(wxWindow *window, int id, const wxBitmap& bitmap)
		: wxBitmapButton(window, id, bitmap)
	{
		wxSize size = GetSize();
		SetSize(size.y, size.y);
		SetBestSize(GetSize());
	}

};

class TristateConfigPanel : public RadioBoxPanel
{

public:
	TristateConfigPanel(wxWindow *parent, wxWindowID id = -1,
		const wxString &caption = wxT("Tristate Config Panel"),
		const wxString &filespec = wxT(""),
		const bool sound_preview = false,
		const wxPoint& pos = wxDefaultPosition);
	virtual ~TristateConfigPanel();

	virtual Config::TristateMode GetMode() const;
	virtual wxString GetPath() const;

	virtual void SetMode(Config::TristateMode mode);
	virtual void SetPath(const wxString &path);

	virtual bool Enable(bool enabled);
	virtual void Enable(Config::TristateMode mode, bool enabled);

protected:
	void OnText(wxCommandEvent &event);
	void OnBrowse(wxCommandEvent &event);
	void OnSoundPreview(wxCommandEvent &event);

protected:
	virtual void OnSelectionChanged(int n);

protected:
	wxTextCtrl *m_txt;
	wxButton *m_cmdBrowse;
	wxBitmapButton *m_cmdSoundPreview;
	wxString m_filespec;
	bool m_sound_preview;
	#if wxUSE_SOUND
		wxSound m_sound;
	#endif

private:
	DECLARE_EVENT_TABLE()
	DECLARE_NO_COPY_CLASS(TristateConfigPanel)

};

#endif
