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


#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: utilgui.cpp,v 1.4 2005-07-24 11:56:15 jason Exp $)

#include "utilgui.h"

#include <wx/mimetype.h>
#include <wx/confbase.h>
#include <wx/filename.h>

#if defined(__WXMSW__)
	#include <windows.h>
	#include <wx/msw/winundef.h>
#elif defined(__UNIX__)
	#include <unistd.h>
#endif

void FixBorder(wxWindow *wnd)
{
	#ifdef __WXMSW__
		HWND hWnd = reinterpret_cast<HWND>(wnd->GetHandle());
		DWORD dwStyle = ::GetWindowLong(hWnd, GWL_STYLE);
		dwStyle &= ~WS_BORDER;
		::SetWindowLong(hWnd, GWL_STYLE, dwStyle);
		::InvalidateRect(hWnd, NULL, FALSE);
		::SetWindowPos(hWnd, NULL, 0,0,0,0,
			SWP_FRAMECHANGED | SWP_NOACTIVATE |
			SWP_NOMOVE | SWP_NOOWNERZORDER |
			SWP_NOSIZE | SWP_NOZORDER);
	#endif
}

void ShowAbout(wxWindow *parent)
{
	wxMessageBox(wxString()
		<< wxT("Dirt Secure Chat ") << GetProductVersion() << wxT("\n")
		<< wxT("\n")
		<< wxT("Built with ") << wxVERSION_STRING << wxT("\n")
		<< wxT("\n")
		<< wxT("Last revision date: ") << GetRCSDate() << wxT(" UTC\n")
		<< wxT("Last revision author: ") << GetRCSAuthor() << wxT("\n")
		<< wxT("Compilation date: ") << FormatISODateTime(GetCompileDate()) << wxT("\n")
		<< wxT("\n")
		<< wxT("http://dirt.gslabs.com.au/"),
		wxT("About Dirt Secure Chat"),
		wxOK | wxICON_INFORMATION,
		parent);
}

#ifdef __WXMAC__
	#include "InternetConfig.h"
#endif

bool OpenBrowser(wxWindow *parent, const wxString &URL, bool show_error)
{

	wxLogNull supress_log;

	#if defined(__WXMSW__)

		return OpenFile(parent, URL, show_error);

	#elif defined(__WXMAC__)

		// http://wiki.wxwindows.org/wiki.pl?Carbon_Code_To_Open_URLs_On_WxMac

		ICInstance icInstance;

		OSType psiSignature = '\?\?\?\?';
		ConstStr255Param hint = 0;

		OSStatus error = ICStart(&icInstance, psiSignature);
		
		if ( error != noErr )
		{

			if (show_error)
			{
				wxMessageBox(
					wxString::Format(wxT("Unable to launch the web browser. Error #1/%d"), (int)error),
					wxT("Browser Problem"), wxOK | wxICON_ERROR, parent);
			}

			return false;

		}

		long length = URL.Length();
		long start = 0;
		long end = length;

		#if !TARGET_CARBON
			ICFindConfigFile(icInstance, 0, nil);
		#endif

		error = ICLaunchURL(icInstance, hint, URL.mb_str(), length, &start, &end);
		bool success = (error == noErr);

		ICStop(icInstance);

		if (!success && show_error)
		{
			wxMessageBox(
				wxString::Format(wxT("Unable to launch the web browser. Error #2/%d"), (int)error),
				wxT("Browser Problem"), wxOK | wxICON_ERROR, parent);
		}

		return success;

	#else

		const wxChar *browsers[2] = { wxT("mozilla"), wxT("netscape") };
		const size_t num_browsers = ((sizeof browsers) / (sizeof browsers[0]));

		bool success = false;

		for (int i = 0; i < num_browsers && !success; ++i)
		{
			wxString cmdline;
			cmdline << browsers[i] << wxT(' ') << URL;
			::wxBeginBusyCursor();
			long pid = ::wxExecute(cmdline, wxEXEC_ASYNC);
			::wxEndBusyCursor();
			success = (pid != 0);
		}

		if (!success)
		{

			wxFileType *ft = wxTheMimeTypesManager->GetFileTypeFromExtension(wxT("html"));
			if ( !ft )
			{
				if (show_error)
				{
					wxMessageBox(
						wxT("Unable to determine the file type for HTML."),
						wxT("Browser Problem"), wxOK | wxICON_ERROR, parent);
				}
				return false;
			}

			wxString cmd;
			bool ok = ft->GetOpenCommand(
				&cmd, wxFileType::MessageParameters(URL, wxT("")));
			delete ft;

			if (!ok)
			{
				if (show_error)
				{
					wxMessageBox(
						wxT("Unable to determine the command for running your HTML browser."),
						wxT("Browser Problem"), wxOK | wxICON_ERROR, parent);
				}
				return false;
			}

			// GetOpenCommand can prepend file:// even if it already has http://
			if (cmd.Find(wxT("http://")) != -1)
			{
				cmd.Replace(wxT("file://"), wxT(""));
			}

			ok = (wxExecute(cmd, FALSE) != 0);

			if (!ok)
			{
				if (show_error)
				{
					wxMessageBox(
						wxT("Unable to navigate to ") + URL,
						wxT("Browser Problem"), wxOK | wxICON_ERROR, parent);
				}
				return false;
			}
	
		}

		return true;

	#endif

}

void ForceForegroundWindow(wxFrame *frm)
{
	
	#if defined(__WXMSW__)
		
		HWND hWnd = (HWND)frm->GetHWND();
		
		if (hWnd != GetForegroundWindow())
		{
			
			frm->Show();
			
			DWORD ThreadID1 = GetWindowThreadProcessId(GetForegroundWindow(), 0);
			DWORD ThreadID2 = GetWindowThreadProcessId(hWnd, 0);
			bool flag = (ThreadID1 != ThreadID2);

			if (flag)
			{
				AttachThreadInput(ThreadID1, ThreadID2, TRUE);
			}
			
			SetForegroundWindow(hWnd);
			SetWindowPos(hWnd, HWND_TOP, 0, 0, 0, 0, SWP_ASYNCWINDOWPOS | SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
			
			if (flag)
			{
				AttachThreadInput(ThreadID1, ThreadID2, FALSE);
			}

			if (IsIconic(hWnd))
			{
				ShowWindow(hWnd, SW_RESTORE);
			}
			else
			{
				ShowWindow(hWnd, SW_SHOW);
			}

			SetWindowPos(hWnd, HWND_TOP, 0, 0, 0, 0, SWP_ASYNCWINDOWPOS | SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);

		}

	#else

		frm->Show(false);
		frm->Show(true);
		frm->SetFocus();

	#endif

}

void GetWindowState(const wxFrame *frm, wxRect& r, bool& maximized)
{
	
	maximized = frm->IsMaximized();

	#ifdef __WIN32__

		HWND hWnd = (HWND)frm->GetHandle();
		WINDOWPLACEMENT wp;
		memset(&wp, 0, sizeof (wp));
		wp.length = sizeof (WINDOWPLACEMENT);

		if (GetWindowPlacement(hWnd, &wp))
		{

			RECT& wr = wp.rcNormalPosition;

			r.x = wr.left;
			r.y = wr.top;
			r.width = wr.right - wr.left;
			r.height = wr.bottom - wr.top;

			maximized = (IsZoomed(hWnd) != FALSE);

		}

	#else

		r = frm->GetRect();

	#endif

}

void SetWindowState(wxFrame *frm, const wxRect &r, const bool maximized, bool show)
{

	#ifdef __WIN32__

		if (show && !frm->IsShown())
		{
			frm->Move(10000,10000);
			frm->Show();
		}

		HWND hwnd = (HWND) frm->GetHandle();
		WINDOWPLACEMENT wp;
		memset(&wp, 0, sizeof (wp));
		wp.length = sizeof (WINDOWPLACEMENT);
		wp.flags = 0;
		wp.showCmd = (show||frm->IsShown()) ? (maximized ? SW_SHOWMAXIMIZED : SW_SHOWNORMAL) : SW_HIDE;

		RECT& rc = wp.rcNormalPosition;

		rc.left = r.x;
		rc.top = r.y;
		rc.right = r.x + r.width;
		rc.bottom = r.y + r.height;

		SetWindowPlacement(hwnd, &wp);

	#else

		frm->SetSize(r);
		frm->Maximize(maximized);
		if (show)
		{
			frm->Show();
		}

	#endif

}

void SaveWindowState(const wxFrame *frm, wxConfigBase *cfg, const wxString &name)
{

	wxString path;
	if (name.Length())
	{
		path = ((name[0u] != wxT('/')) ? wxT("/") : wxT("")) + name;
	}
	path += wxT("/WindowState");

	wxString old_path = cfg->GetPath();
	cfg->SetPath(path);

	wxRect r;
	bool maximized;
	GetWindowState(frm, r, maximized);

	cfg->Write(wxT("X"), long(r.x));
	cfg->Write(wxT("Y"), long(r.y));
	cfg->Write(wxT("Width"), long(r.width));
	cfg->Write(wxT("Height"), long(r.height));
	cfg->Write(wxT("Maximized"), maximized);
	cfg->Flush();

	cfg->SetPath(old_path);

}

void RestoreWindowState(wxFrame *frm, wxConfigBase *cfg, const wxString &name, bool show, bool default_maximized)
{

	wxString path;
	if (name.Length())
	{
		path = ((name[0u] != wxT('/')) ? wxT("/") : wxT("")) + name;
	}
	path += wxT("/WindowState");

	wxString old_path = cfg->GetPath();
	cfg->SetPath(path);

	wxRect r;
	bool maximized;
	GetWindowState(frm, r, maximized);

	maximized |= default_maximized;

	cfg->Read(wxT("X"), &r.x, r.x);
	cfg->Read(wxT("Y"), &r.y, r.y);
	cfg->Read(wxT("Width"), &r.width, r.width);
	cfg->Read(wxT("Height"), &r.height, r.height);
	cfg->Read(wxT("Maximized"), &maximized, maximized);

	SetWindowState(frm, r, maximized, show);

	cfg->SetPath(old_path);

}

bool OpenFile(wxWindow *parent, const wxString &filename, bool show_error)
{

	wxLogNull supress_log;

	#ifdef __WXMSW__

		wxString file, params;

		if (filename.Find(wxT("://")) > -1 ||
			wxFileName(filename).FileExists() ||
			wxFileName(filename).DirExists())
		{
			file = filename;
			params = wxEmptyString;
		}
		else if (filename.Left(1) == wxT('"') && filename.Mid(1).Find(wxT('"')) > 0)
		{
			int i = filename.Mid(1).Find(wxT('"'));
			file = filename.Mid(1, i);
			params = filename.Mid(i+2).Strip(wxString::both);
		}
		else
		{
			int i = filename.Find(wxT(' '));
			if (i > -1)
			{
				file = filename.Left(i);
				params = filename.Mid(i+1);
			}
			else
			{
				file = filename;
				params = wxEmptyString;
			}
		}

		LPCTSTR params_api = (params.Length() > 0) ? params.c_str() : (const wxChar*)NULL;

		::wxBeginBusyCursor();
		HINSTANCE hInstance = ::ShellExecute((HWND)(parent->GetHandle()), wxEmptyString, file, params_api, NULL, SW_SHOWMAXIMIZED);
		::wxEndBusyCursor();
		bool success = ((int)hInstance > 32);
		if (!success)
		{
			::wxBeginBusyCursor();
			hInstance = ::ShellExecute((HWND)(parent->GetHandle()), wxT("open"), file, params_api, NULL, SW_SHOWMAXIMIZED);
			::wxEndBusyCursor();
			success = ((int)hInstance > 32);
		}
		if (!success)
		{
			if (show_error)
			{
				wxMessageBox(
					wxT("Unable to open ") + filename,
					wxT("Error launching external application"), wxOK | wxICON_ERROR, parent);
			}
			return false;
		}
		return true;

	#else

		wxArrayString data = SplitQuotedString(filename, wxT(" "));

		wxChar **args = new wxChar*[data.GetCount()+1];
		args[data.GetCount()] = 0;
		for (int i = 0; i < data.GetCount(); ++i)
		{
			args[i] = new wxChar[data[i].Length()+1];
			args[i][data[i].Length()] = 0;
			wxStrcpy(args[i], data[i].c_str());
		}
		bool ok = wxExecute(args) != 0;
		for (int i = 0; i < data.GetCount(); ++i)
		{
			delete args[i];
		}
		delete[] args;
		if (show_error)
		{
			wxMessageBox(wxT("Error executing command:\n\n") + filename);
		}
		return ok;

/*		
		wxString ext = wxFileName(filename).GetExt();
		wxFileType *ft = wxTheMimeTypesManager->GetFileTypeFromExtension(ext);
		if ( !ft )
		{
			if (show_error)
			{
				wxMessageBox(
					wxT("Unable to determine the file type for the extension \"") + ext + wxT("\"."),
					wxT("Error launching external application"), wxOK | wxICON_ERROR, parent);
			}
			return false;
		}

		wxString cmd;
		bool ok = ft->GetOpenCommand(
			&cmd, wxFileType::MessageParameters(filename, wxT("")));
		delete ft;

		if (!ok)
		{
			if (show_error)
			{
				wxMessageBox(
					wxT("Unable to determine the command for executing files with extension \"") + ext + wxT("\"."),
					wxT("Error launching external application"), wxOK | wxICON_ERROR, parent);
			}
			return false;
		}

		// GetOpenCommand can prepend file:// even if it already has http://
		if (cmd.Find(wxT("http://")) != -1)
		{
			cmd.Replace(wxT("file://"), wxT(""));
		}

		ok = (wxExecute(cmd, FALSE) != 0);

		if (!ok)
		{
			if (show_error)
			{
				wxMessageBox(
					wxT("Unable to execute \"") + cmd + wxT("\""),
					wxT("Error launching external application"), wxOK | wxICON_ERROR, parent);
			}
			return false;
		}
				
		return true;
*/

	#endif

}

bool OpenFolder(wxWindow *parent, const wxString &folder, bool show_error)
{
	#if defined(__WXMSW__)
		return OpenFile(parent, folder, show_error);
	#else
		return OpenBrowser(parent, wxT("file://") + folder, show_error);
	#endif
}

bool OpenExternalResource(wxWindow *parent, const wxString &name, bool show_error)
{

	if (name.Find(wxT("://")) > -1)
	{
		return OpenBrowser(parent, name, show_error);
	}

	if (wxFileName(name, wxEmptyString).DirExists())
	{
		return OpenFolder(parent, name, show_error);
	}

	if (wxFileName(name).FileExists())
	{
		return OpenFile(parent, name, show_error);
	}

	return
		OpenFile(parent, name, false) ||
		OpenBrowser(parent, name, show_error);

}

bool SetDefaultMenuItem(wxMenu &mnu, int id)
{

	#ifdef __WXMSW__

		HMENU hMenu = (HMENU)mnu.GetHMenu();
		wxMenuItem *item = mnu.FindItem(id);
		if (item)
		{
			int real_id = item->GetRealId();
			MENUITEMINFO mii;
			mii.cbSize = sizeof (MENUITEMINFO);
			mii.fMask = MIIM_STATE;
			if (GetMenuItemInfo(hMenu, real_id, FALSE, &mii))
			{
				mii.fState |= MFS_DEFAULT;
				if (SetMenuItemInfo(hMenu, real_id, FALSE, &mii))
				{
					return true;
				}
			}
		}
		return false;

	#else

		return false;

	#endif

}
