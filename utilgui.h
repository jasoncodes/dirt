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


#ifndef UTILGUI_H_
#define UTILGUI_H_

#include "util.h"

class wxHtmlWinParser;

void FixBorder(wxWindow *wnd);
void ShowAbout(wxWindow *parent);
bool OpenBrowser(wxWindow *parent, const wxString &URL, bool show_error = true);
bool OpenFile(wxWindow *parent, const wxString &filename, bool show_error = true);
bool OpenFolder(wxWindow *parent, const wxString &folder, bool show_error = true);
bool OpenExternalResource(wxWindow *parent, const wxString &name, bool show_error = true);
void ForceForegroundWindow(wxFrame *frm);
void GetWindowState(const wxFrame *frm, wxRect& r, bool& maximized);
void SetWindowState(wxFrame *frm, const wxRect &r, const bool maximized, bool show = false);
void SaveWindowState(const wxFrame *frm, wxConfigBase *cfg, const wxString &name = wxEmptyString);
void RestoreWindowState(wxFrame *frm, wxConfigBase *cfg, const wxString &name = wxEmptyString, bool show = false, bool default_maximized = false);
bool SetDefaultMenuItem(wxMenu &mnu, int id);

#endif
