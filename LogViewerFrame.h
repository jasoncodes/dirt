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


#ifndef DefaultFrame_H_
#define DefaultFrame_H_

#include <wx/sashwin.h>
#include <wx/dirctrl.h>
#include <wx/treectrl.h>
#include "Client.h"

class LogControl;

class LogViewerFrame : public wxFrame
{

public:
	LogViewerFrame();
	virtual ~LogViewerFrame();

	void ViewLogFile(const wxString &filename);

protected:
	void OnClose(wxCloseEvent &event);
	void OnCtrlF(wxCommandEvent &event);
	void OnSize(wxSizeEvent &event);
	void OnSashDragged(wxSashEvent &event);
	void OnTreeSelChanged(wxTreeEvent &event);
	void OnDirSelChange(wxTreeEvent &event);
	void OnItemRightClick(wxTreeEvent &event);
	void OnDelete(wxCommandEvent &event);
	void OnLinkClicked(wxCommandEvent &event);
	void OnConfigFileChanged(wxCommandEvent &event);

protected:
	virtual void ResizeChildren();
	virtual void PopulateTree(const wxString &dirname);
	static wxArrayString GetLogFilenames(const wxString &dirname);
	void EnsureItemSelected(const wxTreeItemId& idParent, const wxString &filename);
	void EnsureItemSelected(const wxTreeItemId& idParent, const wxString &filename, wxTreeItemIdValue cookie, bool first_call);
	wxString GetItemFilename(const wxTreeItemId& id) const;
	static wxString GetLogDirectory();

protected:
	wxSashWindow *m_sash1;
	wxSashWindow *m_sash2;
	
	LogControl *m_log;
	wxTreeCtrl *m_tree;
	wxGenericDirCtrl *m_dir;

	wxString m_last_filename;

	ClientConfig m_config;
	bool m_is_busy;

private:
	DECLARE_EVENT_TABLE()
	DECLARE_NO_COPY_CLASS(LogViewerFrame)

};

#endif
