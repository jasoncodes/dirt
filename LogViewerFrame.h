#ifndef DefaultFrame_H_
#define DefaultFrame_H_

#include <wx/sashwin.h>
#include <wx/dirctrl.h>
#include <wx/treectrl.h>
#include "ConfigFile.h"

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

	ConfigFile m_config;
	bool m_is_busy;

private:
	DECLARE_EVENT_TABLE()
	DECLARE_NO_COPY_CLASS(LogViewerFrame)

};

#endif
