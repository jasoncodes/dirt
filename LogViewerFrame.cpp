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
	ID_CTRL_F,
	ID_EXPORT
};

BEGIN_EVENT_TABLE(LogViewerFrame, wxFrame)
	EVT_SIZE(LogViewerFrame::OnSize)
	EVT_SASH_DRAGGED(ID_SASH1, LogViewerFrame::OnSashDragged)
	EVT_SASH_DRAGGED(ID_SASH2, LogViewerFrame::OnSashDragged)
	EVT_TREE_SEL_CHANGED(ID_TREECTRL, LogViewerFrame::OnTreeSelChanged)
	EVT_TREE_ITEM_RIGHT_CLICK(ID_TREECTRL, LogViewerFrame::OnItemRightClick)
	EVT_MENU(ID_CTRL_F, LogViewerFrame::OnCtrlF)
END_EVENT_TABLE()

LogViewerFrame::LogViewerFrame()
	: wxFrame(NULL, -1, AppTitle(wxT("Log Viewer")), wxDefaultPosition, wxSize(720, 480), wxDEFAULT_FRAME_STYLE | wxNO_FULL_REPAINT_ON_RESIZE | wxCLIP_CHILDREN)
{

	SetIcon(wxIcon(dirt_xpm));

	wxPanel *panel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNO_FULL_REPAINT_ON_RESIZE | wxCLIP_CHILDREN);

	m_sash1 = new wxSashWindow(panel, ID_SASH1, wxDefaultPosition, wxSize(225,100), wxSW_3DSASH | wxNO_FULL_REPAINT_ON_RESIZE | wxCLIP_CHILDREN);
	m_sash2 = new wxSashWindow(m_sash1, ID_SASH2, wxDefaultPosition, wxSize(100, 200), wxSW_3DSASH | wxNO_FULL_REPAINT_ON_RESIZE | wxCLIP_CHILDREN);
	m_sash1->SetSashVisible(wxSASH_RIGHT, true);
	m_sash2->SetSashVisible(wxSASH_BOTTOM, true);
	
	m_log = new LogControl(panel, wxID_ANY);
	
	m_tree = new wxTreeCtrl(m_sash1, ID_TREECTRL, wxDefaultPosition, wxDefaultSize, wxTR_DEFAULT_STYLE | wxTR_HIDE_ROOT);
	m_tree->AddRoot(wxEmptyString);
	
	wxFileName::Mkdir(LogReader::GetDefaultLogDirectory(), 0700, wxPATH_MKDIR_FULL);

	m_dir = new wxGenericDirCtrl(m_sash2, wxID_ANY, wxDirDialogDefaultFolderStr, wxDefaultPosition, wxDefaultSize, wxDIRCTRL_3D_INTERNAL | wxDIRCTRL_DIR_ONLY | wxNO_FULL_REPAINT_ON_RESIZE | wxCLIP_CHILDREN);
 	Connect(m_dir->GetTreeCtrl()->GetId(), wxEVT_COMMAND_TREE_SEL_CHANGED,
		(wxObjectEventFunction)(wxEventFunction)(wxTreeEventFunction)&LogViewerFrame::OnDirSelChange);

	m_dir->ExpandPath(LogReader::GetDefaultLogDirectory());

	wxAcceleratorEntry entries[1];
	entries[0].Set(wxACCEL_CTRL, 'F', ID_CTRL_F);
	wxAcceleratorTable accel(1, entries);
	SetAcceleratorTable(accel);

	SetSizeHints(300,200);
	CentreOnScreen();
	Maximize();
	Show();

}

LogViewerFrame::~LogViewerFrame()
{
}

void LogViewerFrame::OnSize(wxSizeEvent &event)
{
	ResizeChildren();
	event.Skip();
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

	m_sash1->SetMinimumSizeX(128);
	m_sash1->SetMaximumSizeX(size.x-128);
	m_sash2->SetMinimumSizeY(64);
	m_sash2->SetMaximumSizeY(size.y-64);
	
	m_sash1->SetSize(0,0,x, size.y);
	m_sash2->SetSize(0,0,x-3, y);
	m_dir->SetSize(0,0,x-3, y-3);
	m_tree->SetSize(0,y,x-3,size.y-y);
	m_log->SetSize(x, 0, size.x-x, size.y);

}

void LogViewerFrame::OnDirSelChange(wxTreeEvent &event)
{
	wxString temp = m_dir->GetPath();
	PopulateTree(temp);
}

void LogViewerFrame::OnTreeSelChanged(wxTreeEvent &event)
{

	m_log->Clear();
	
	wxString filename = GetItemFilename(m_tree->GetSelection());
	
	if (filename.Length())
	{
		ViewLogFile(wxFileName(m_dir->GetPath(), filename).GetFullPath());
	}
	else
	{
		last_filename.Empty();
	}

}

wxString LogViewerFrame::GetItemFilename(const wxTreeItemId& id) const
{
	LogViewerTreeItemData *data = (LogViewerTreeItemData*)(m_tree->GetItemData(id));
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

	if (last_filename == filename) return;

	LogReader reader(filename);

	if (reader.Ok())
	{

		last_filename = filename;

		wxString filepath = wxFileName(filename).GetPath(wxPATH_GET_VOLUME);
		if (m_dir->GetPath() != filepath)
		{
			m_dir->ExpandPath(filepath);
		}

		last_filename = filename;

		EnsureItemSelected(m_tree->GetRootItem(), wxFileName(filename).GetFullName());

		m_log->Freeze();

		wxLongLong_t next_update = 0;

		wxProgressDialog dlg(wxT("Loading..."), filename, reader.GetLength(), this, wxPD_APP_MODAL | wxPD_AUTO_HIDE | wxPD_CAN_ABORT | wxPD_ELAPSED_TIME);
		
		while (!reader.Eof())
		{

			switch (reader.GetNext())
			{
				case letText:
					m_log->AddTextLine(reader.GetText(), reader.GetTextColour(), tmmParse, reader.GetTextConvertURLs());
					break;
				case letSeparator:
					//m_log->AddSeparator(); // not implemented yet
					break;
			}

			if (next_update < GetMillisecondTicks())
			{
				if (!dlg.Update(reader.GetPosition()))
				{
					//m_log->AddSeparator(); // not implemented yet
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
			if (RightEq(filename, wxT(".dirtlog")))
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

	last_filename = wxEmptyString;

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

void LogViewerFrame::OnItemRightClick(wxTreeEvent &event)
{
	wxTreeItemId id = event.GetItem();
	if (id.IsOk())
	{
		if (m_tree->GetItemData(id) != NULL)
		{
			wxMenu menu;
			menu.Append(ID_EXPORT, wxT("&Export..."));
			PopupMenu(&menu, ScreenToClient(m_tree->ClientToScreen(event.GetPoint())));
		}
	}
}

void LogViewerFrame::OnCtrlF(wxCommandEvent &event)
{
	m_log->ShowFindDialog(true);
}
