#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif

#include "LogViewerFrame.h"
#include "LogReader.h"
#include "util.h"
#include <wx/progdlg.h>
#include <wx/dir.h>
#include "Dirt.h"
#include "LogControl.h"
#include <wx/filename.h>

DECLARE_APP(DirtApp)

#ifdef __WXMSW__
	#include <windows.h>
	#include <wx/msw/winundef.h>
#endif

#include "res/dirt.xpm"

class LogViewerTreeItemData : public wxTreeItemData
{

public:
	LogViewerTreeItemData(const wxString& filename)
		: m_filename(filename)
	{
	}

	inline wxString GetFilename()
	{
		return m_filename;
	}

protected:
	wxString m_filename;

};

enum
{
	ID_SASH1 = 1,
	ID_SASH2,
	ID_TREECTRL,
	ID_LOG,
	ID_CTRL_F,
	ID_DELETE
};

BEGIN_EVENT_TABLE(LogViewerFrame, wxFrame)
	EVT_CLOSE(LogViewerFrame::OnClose)
	EVT_SIZE(LogViewerFrame::OnSize)
	EVT_SASH_DRAGGED(ID_SASH1, LogViewerFrame::OnSashDragged)
	EVT_SASH_DRAGGED(ID_SASH2, LogViewerFrame::OnSashDragged)
	EVT_TREE_SEL_CHANGED(ID_TREECTRL, LogViewerFrame::OnTreeSelChanged)
	EVT_TREE_ITEM_RIGHT_CLICK(ID_TREECTRL, LogViewerFrame::OnItemRightClick)
	EVT_MENU(ID_CTRL_F, LogViewerFrame::OnCtrlF)
	EVT_MENU(ID_DELETE, LogViewerFrame::OnDelete)
	EVT_BUTTON(ID_LOG, LogViewerFrame::OnLinkClicked)
END_EVENT_TABLE()

LogViewerFrame::LogViewerFrame()
	: wxFrame(NULL, -1, AppTitle(wxT("Log Viewer")), wxDefaultPosition, wxSize(720, 480), wxDEFAULT_FRAME_STYLE | wxNO_FULL_REPAINT_ON_RESIZE | wxCLIP_CHILDREN | wxTAB_TRAVERSAL)
{

	m_is_busy = false;

	SetIcon(wxIcon(dirt_xpm));

	wxPanel *panel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNO_FULL_REPAINT_ON_RESIZE | wxCLIP_CHILDREN | wxTAB_TRAVERSAL | wxNO_BORDER);

	m_sash1 = new wxSashWindow(panel, ID_SASH1, wxDefaultPosition, wxSize(225,100), wxSW_3DSASH | wxNO_FULL_REPAINT_ON_RESIZE | wxCLIP_CHILDREN | wxNO_BORDER);
	m_sash2 = new wxSashWindow(m_sash1, ID_SASH2, wxDefaultPosition, wxSize(100, 200), wxSW_3DSASH | wxNO_FULL_REPAINT_ON_RESIZE | wxCLIP_CHILDREN | wxNO_BORDER);
	m_sash1->SetSashVisible(wxSASH_RIGHT, true);
	m_sash2->SetSashVisible(wxSASH_BOTTOM, true);
	
	m_log = new LogControl(panel, ID_LOG);
	
	m_tree = new wxTreeCtrl(m_sash1, ID_TREECTRL, wxDefaultPosition, wxDefaultSize, wxTR_DEFAULT_STYLE | wxTR_HIDE_ROOT);
	m_tree->AddRoot(wxEmptyString);
	
	wxFileName::Mkdir(GetLogDirectory(), 0700, wxPATH_MKDIR_FULL);

	m_dir = new wxGenericDirCtrl(m_sash2, wxID_ANY, wxDirDialogDefaultFolderStr, wxDefaultPosition, wxDefaultSize, wxDIRCTRL_3D_INTERNAL | wxDIRCTRL_DIR_ONLY | wxNO_FULL_REPAINT_ON_RESIZE | wxCLIP_CHILDREN);
 	Connect(m_dir->GetTreeCtrl()->GetId(), wxEVT_COMMAND_TREE_SEL_CHANGED,
		(wxObjectEventFunction)(wxEventFunction)(wxTreeEventFunction)&LogViewerFrame::OnDirSelChange);

	m_dir->ExpandPath(GetLogDirectory());

	wxAcceleratorEntry entries[2];
	entries[0].Set(wxACCEL_CTRL, 'F', ID_CTRL_F);
	entries[1].Set(0, WXK_DELETE, ID_DELETE);
	wxAcceleratorTable accel(2, entries);
	SetAcceleratorTable(accel);

	SetSizeHints(300,200);
	CentreOnScreen();

	RestoreWindowState(this, &m_config, wxT("Log Viewer"), true, true);
	int sash1pos = m_sash1->GetSize().x;
	int sash2pos = m_sash2->GetSize().y;
	m_config.Read(wxT("/Log Viewer/WindowState/Sash1"), &sash1pos, sash1pos);
	m_config.Read(wxT("/Log Viewer/WindowState/Sash2"), &sash2pos, sash2pos);
	m_sash1->SetSize(sash1pos, -1);
	m_sash2->SetSize(-1, sash2pos);
	ResizeChildren();

}

LogViewerFrame::~LogViewerFrame()
{
}

wxString LogViewerFrame::GetLogDirectory()
{
	Config cfg(wxT("Client"));
	return cfg.GetActualLogDir();
}

void LogViewerFrame::OnClose(wxCloseEvent &event)
{
	SaveWindowState(this, &m_config, wxT("Log Viewer"));
	int sash1pos = m_sash1->GetSize().x;
	int sash2pos = m_sash2->GetSize().y;
	m_config.Write(wxT("/Log Viewer/WindowState/Sash1"), sash1pos);
	m_config.Write(wxT("/Log Viewer/WindowState/Sash2"), sash2pos);
	event.Skip();
}

void LogViewerFrame::OnSize(wxSizeEvent &event)
{
	ResizeChildren();
	event.Skip();
}

void LogViewerFrame::OnLinkClicked(wxCommandEvent& event)
{
	OpenBrowser(this, event.GetString());
}

void LogViewerFrame::OnSashDragged(wxSashEvent &event)
{
	if (event.GetDragStatus() == wxSASH_STATUS_OK)
	{
		if (event.GetId() == ID_SASH1)
		{
			m_sash1->SetSize(event.GetDragRect());
		}
		else if (event.GetId() == ID_SASH2)
		{
			m_sash2->SetSize(event.GetDragRect());
		}
		ResizeChildren();
	}
}

void LogViewerFrame::ResizeChildren()
{

	wxSize size = GetClientSize();

	int x = m_sash1->GetSize().x;
	int y = m_sash2->GetSize().y;

	static const int min_x1 = 128;
	static const int min_x2 = 128;
	static const int min_y1 = 64;
	static const int min_y2 = 64;

	m_sash1->SetMinimumSizeX(min_x1);
	m_sash1->SetMaximumSizeX(size.x-min_x2);
	m_sash2->SetMinimumSizeY(min_y1);
	m_sash2->SetMaximumSizeY(size.y-min_y2);

	x = wxMin(x, size.x-min_x2);
	x = wxMax(x, min_x1);
	y = wxMin(y, size.y-min_y2);
	y = wxMax(y, min_y1);

	int margin_x = m_sash1->GetEdgeMargin(wxSASH_RIGHT);
	int margin_y = m_sash2->GetEdgeMargin(wxSASH_BOTTOM);

	m_sash1->SetSize(0, 0, x, size.y);
	m_sash2->SetSize(0, 0, x-margin_x, y);
	m_dir->SetSize(0, 0, x-margin_x, y-margin_y);
	m_tree->SetSize(0, y, x-margin_x, size.y-y-margin_y);
	m_log->SetSize(x, 0, size.x-x, size.y);

}

void LogViewerFrame::OnDirSelChange(wxTreeEvent &event)
{
	wxString temp = m_dir->GetPath();
	PopulateTree(temp);
}

void LogViewerFrame::OnTreeSelChanged(wxTreeEvent &event)
{

	if (m_is_busy) return;

	m_log->Clear();
	
	wxString filename = GetItemFilename(m_tree->GetSelection());
	
	if (filename.Length())
	{
		ViewLogFile(wxFileName(m_dir->GetPath(), filename).GetFullPath());
	}
	else
	{
		m_last_filename.Empty();
	}

}

wxString LogViewerFrame::GetItemFilename(const wxTreeItemId& id) const
{
	LogViewerTreeItemData *data = id.IsOk()?((LogViewerTreeItemData*)(m_tree->GetItemData(id))):0;
	return data?data->GetFilename():wxString();
}

void LogViewerFrame::EnsureItemSelected(const wxTreeItemId& idParent, const wxString &filename, long cookie)
{

	if (GetItemFilename(m_tree->GetSelection()) == filename)
	{
		return;
	}

	wxTreeItemId id;

	if (cookie == -1)
	{
		id = m_tree->GetFirstChild(idParent, cookie);
	}
	else
	{
		id = m_tree->GetNextChild(idParent, cookie);
	}

	if (!id.IsOk())
	{
		return;
	}

	if (GetItemFilename(id) == filename)
	{
		m_tree->SelectItem(id);
		return;
	}

	if (m_tree->ItemHasChildren(id))
	{
		EnsureItemSelected(id, filename, -1);
	}

	if (GetItemFilename(m_tree->GetSelection()) == filename)
	{
		return;
	}

	EnsureItemSelected(idParent, filename, cookie);

}

void LogViewerFrame::ViewLogFile(const wxString &filename)
{

	if (m_last_filename == filename) return;

	LogReader reader(filename);

	if (reader.Ok())
	{

		wxFileName fn(filename);
		
		m_last_filename = fn.GetFullPath();

		m_is_busy = true;

		wxString filepath = wxFileName(filename).GetPath(wxPATH_GET_VOLUME);
		if (m_dir->GetPath() != filepath)
		{
			m_dir->ExpandPath(filepath);
		}

		m_last_filename = fn.GetFullPath();

		EnsureItemSelected(m_tree->GetRootItem(), wxFileName(filename).GetFullName());

		m_is_busy = false;

		m_log->Freeze();

		wxLongLong_t next_update = 0;

		wxProgressDialog dlg(wxT("Loading..."), wxFileName(filename).GetFullName(), reader.GetLength(), this, wxPD_APP_MODAL | wxPD_AUTO_HIDE | wxPD_CAN_ABORT | wxPD_ELAPSED_TIME);

		bool had_text = false;
		
		while (!reader.Eof())
		{

			switch (reader.GetNext())
			{
				case letText:
					m_log->AddTextLine(reader.GetText(), reader.GetTextColour(), tmmParse, reader.GetTextConvertURLs());
					had_text = true;
					break;
				case letSeparator:
					if (had_text)
					{
						m_log->AddSeparator();
					}
					break;
				default:
					break;
			}

			if (next_update < GetMillisecondTicks())
			{
				if (!dlg.Update(reader.GetPosition()))
				{
					m_log->AddSeparator();
					m_log->AddTextLine(wxT("Cancelled"), *wxRED);
					break;
				}
				next_update = GetMillisecondTicks() + 100;
			}

		}

		m_log->Thaw();

	}
	else
	{
		m_last_filename.Empty();
		wxMessageBox(wxT("Unable to open log file"), wxT("Dirt Secure Chat"), wxOK|wxICON_ERROR, this);
	}

}

wxArrayString LogViewerFrame::GetLogFilenames(const wxString &dirname)
{

	wxArrayString filenames;
	wxDir dir(dirname);

	if (dir.IsOpened())
	{

		wxString filename;

		bool more_files = dir.GetFirst(&filename);
		while (more_files)
		{
			if (RightEq(filename, wxT(".dirtlog")) && wxFileName(dirname, filename).FileExists())
			{
				filenames.Add(filename);
			}
			more_files = dir.GetNext(&filename);
		}

		filenames.Sort();

	}
	
	return filenames;

}

void LogViewerFrame::PopulateTree(const wxString &dirname)
{

	m_last_filename.Empty();

	wxBusyCursor busy;

	m_tree->DeleteChildren(m_tree->GetRootItem());
	m_log->Clear();

	wxArrayString filenames = GetLogFilenames(dirname);

	if (filenames.GetCount())
	{

		wxArrayString PrefixArray;
		wxArrayString OtherFileArray;

		for (size_t i = 0; i < filenames.GetCount(); i++)
		{

			wxString prefix, suffix;
			wxDateTime date;

			if (LogReader::ParseFilename(filenames.Item(i), prefix, date, suffix))
			{
				if (PrefixArray.GetCount() == 0 ||
					PrefixArray[PrefixArray.GetCount()-1] != prefix)
				{
					PrefixArray.Add(prefix);
				}
			}
			else
			{
				OtherFileArray.Add(filenames.Item(i));
			}

		}

		PrefixArray.Sort();
		OtherFileArray.Sort();

		wxDateTime CurrentDate;
		wxTreeItemId id2;

		for (size_t i = 0; i < PrefixArray.GetCount(); i++)
		{

			wxTreeItemId id =
				m_tree->AppendItem(m_tree->GetRootItem(), PrefixArray.Item(i));
			
			for (size_t j = 0; j < filenames.GetCount(); j++)
			{

				wxString prefix, suffix;
				wxDateTime date;
				
				if (LogReader::ParseFilename(filenames.Item(j), prefix, date, suffix))
				{

					if (prefix == PrefixArray.Item(i))
					{

						if (!CurrentDate.IsValid() || date != CurrentDate)
						{
							id2 = m_tree->AppendItem(id, FormatISODateTime(date));
							CurrentDate = date;
						}

						if (suffix.length() > 0)
						{
							m_tree->AppendItem(
								id2, suffix, -1, -1,
								new LogViewerTreeItemData(filenames.Item(j)));
						}
						else
						{
							m_tree->SetItemData(
								id2, new LogViewerTreeItemData(filenames.Item(j)));
						}


					}

				}

			}

		}

		if (OtherFileArray.GetCount() > 0)
		{
			
			wxTreeItemId id = m_tree->AppendItem(m_tree->GetRootItem(), wxT("(Other)"));
			
			for (size_t i = 0; i < OtherFileArray.GetCount(); i++)
			{
				m_tree->AppendItem(
					id, wxFileName(OtherFileArray.Item(i)).GetName(),
					-1, -1, new LogViewerTreeItemData(OtherFileArray.Item(i)));
			}

		}

	}

	m_tree->Refresh();

}

void LogViewerFrame::OnCtrlF(wxCommandEvent &event)
{
	m_log->ShowFindDialog(true);
}

void LogViewerFrame::OnItemRightClick(wxTreeEvent &event)
{
	wxTreeItemId id = event.GetItem();
	if (id.IsOk())
	{
		if (m_tree->GetItemData(id) != NULL)
		{
			wxMenu menu;
			menu.Append(ID_DELETE, wxT("&Delete"));
			PopupMenu(&menu, ScreenToClient(m_tree->ClientToScreen(event.GetPoint())));
		}
	}
}

void LogViewerFrame::OnDelete(wxCommandEvent &event)
{

	wxTreeItemId id = m_tree->GetSelection();

	wxString filename = GetItemFilename(id);

	if (filename.Length())
	{

		filename = wxFileName(m_dir->GetPath(), filename).GetFullPath();

		bool success;

		if (wxMessageBox(wxT("Are you sure you want to delete ") + filename, wxT("Dirt Secure Chat"), wxOK | wxCANCEL | wxICON_QUESTION, this) != wxOK)
		{
			return;
		}

		{

			wxBusyCursor busy;

			#ifdef __WXMSW__
				SHFILEOPSTRUCT FileOp;
				FileOp.hwnd = (HWND)GetHandle();
				FileOp.wFunc = FO_DELETE;
				wxChar pFromBuff[MAX_PATH*2];
				memset(pFromBuff, 0, MAX_PATH*2*(sizeof (wxChar)));
				wxStrcpy(pFromBuff, filename.c_str());
				FileOp.pFrom = pFromBuff;
				FileOp.pTo = NULL;
				FileOp.fFlags = FOF_ALLOWUNDO|FOF_NOERRORUI|FOF_NOCONFIRMATION;
				success = (SHFileOperation(&FileOp) == 0);
			#else
				success = wxRemoveFile(filename);
			#endif

		}
		
		if (success)
		{
		
			m_log->Clear();
			m_last_filename.Empty();

			if (m_tree->GetChildrenCount(id))
			{
				delete m_tree->GetItemData(id);
				m_tree->SetItemData(id, NULL);
			}
			else
			{
				wxTreeItemId parent = m_tree->GetItemParent(id);
				m_tree->Delete(id);
				if (parent.IsOk() && !m_tree->GetChildrenCount(parent) && !m_tree->GetItemData(parent))
				{
					wxTreeItemId grand_parent = m_tree->GetItemParent(parent);
					m_tree->Delete(parent);
					if (grand_parent.IsOk() && !m_tree->GetChildrenCount(grand_parent) && !m_tree->GetItemData(grand_parent))
					{
						m_tree->Delete(grand_parent);
					}
				}
			}

		}
		else
		{
			wxMessageBox(wxT("Error deleting ") + filename, wxT("Dirt Secure Chat"), wxOK | wxICON_ERROR, this);
		}

	}

}
