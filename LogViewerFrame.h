#ifndef DefaultFrame_H_
#define DefaultFrame_H_

#include <wx/sashwin.h>
#include <wx/dirctrl.h>
#include <wx/treectrl.h>

#include "LogControl.h"

class LogViewerFrame : public wxFrame
{

public:
	LogViewerFrame();
	virtual ~LogViewerFrame();

	void ViewLogFile(const wxString &filename);

protected:
	void OnCtrlF(wxCommandEvent &event);
	void OnSize(wxSizeEvent &event);
	void OnSashDragged(wxSashEvent &event);
	void OnTreeSelChanged(wxTreeEvent &event);
	void OnDirSelChange(wxTreeEvent &event);
	void OnItemRightClick(wxTreeEvent &event);

protected:
	virtual void ResizeChildren();
	virtual void PopulateTree(const wxString &dirname);
	static wxArrayString GetLogFilenames(const wxString &dirname);
	void EnsureItemSelected(const wxTreeItemId& idParent, const wxString &filename, long cookie = -1);
	wxString GetItemFilename(const wxTreeItemId& id) const;

protected:
	wxSashWindow *m_sash1;
	wxSashWindow *m_sash2;
	
	LogControl *m_log;
	wxTreeCtrl *m_tree;
	wxGenericDirCtrl *m_dir;

	wxString last_filename;

private:
	DECLARE_EVENT_TABLE()

};

#endif
