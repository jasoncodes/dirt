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
RCS_ID($Id: ClientUIMDITransferResumeDialog.cpp,v 1.4 2004-05-16 04:42:44 jason Exp $)

#ifdef __WXMSW__
	#include <windows.h>
	#include <wx/msw/winundef.h>
#endif

#include "ClientUIMDITransferResumeDialog.h"
#include <wx/filename.h>
#include <wx/artprov.h>

BEGIN_EVENT_TABLE(ClientUIMDITransferResumeDialog, wxDialog)
	EVT_BUTTON(wxID_ANY, ClientUIMDITransferResumeDialog::OnClick)
END_EVENT_TABLE()

ClientUIMDITransferResumeDialog::ClientUIMDITransferResumeDialog(wxWindow *parent, const wxString &nickname, const wxString &org_filename, const wxString &new_filename, bool can_resume)
	: wxDialog(parent, wxID_ANY,
		wxT("Resume or overwrite ") + wxFileName(org_filename).GetFullName() + wxT("?"),
		wxDefaultPosition, wxDefaultSize,
		wxDEFAULT_DIALOG_STYLE | wxCLIP_CHILDREN | wxNO_FULL_REPAINT_ON_RESIZE)
{

	wxPanel *panel = new wxPanel(
		this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
		wxTAB_TRAVERSAL | wxNO_BORDER | wxCLIP_CHILDREN | wxNO_FULL_REPAINT_ON_RESIZE);

	wxString text;
	text << wxT("Would you like to resume or overwrite\n\n");
	text << new_filename << wxT("\n\n");
	text << wxT("with ") << wxFileName(org_filename).GetFullName();
	text << wxT(" from ") << nickname << wxT("?\n\n");
	text << wxT("Note: You should only resume if the\n");
	text << wxT("existing file is an incomplete copy of\n");
	text << wxT("the remote file.\n");

	wxStaticText *lblText = new wxStaticText(panel, wxID_ANY, text);

	#ifdef __WXMSW__
		wxIcon icon;
		icon.SetHICON((WXHICON)LoadIcon(NULL, IDI_QUESTION));
		icon.SetWidth(32);
		icon.SetHeight(32);
	#else
		wxIcon icon = wxArtProvider::GetIcon(wxART_QUESTION);
	#endif

	wxStaticBitmap *bmpIcon = new wxStaticBitmap(panel, wxID_ANY, icon, wxDefaultPosition, wxDefaultSize, wxNO_BORDER);

	wxButton *cmdYes = new wxButton(panel, wxID_YES, wxT("&Resume"));
	cmdYes->Enable(can_resume);
	wxButton *cmdNo = new wxButton(panel, wxID_NO, wxT("&Overwrite"));
	wxButton *cmdCancel = new wxButton(panel, wxID_CANCEL, wxT("&Cancel"));

	wxBoxSizer *szrAll = new wxBoxSizer(wxVERTICAL);
	{

		wxBoxSizer *szrTop = new wxBoxSizer(wxHORIZONTAL);
		{
			szrTop->Add(bmpIcon, 0, wxRIGHT, 16);
			szrTop->Add(lblText, 0, 0, 0);
		}
		szrAll->Add(szrTop, 0, wxTOP | wxLEFT | wxRIGHT, 8);

		wxBoxSizer *szrButtons = new wxBoxSizer(wxHORIZONTAL);
		{
			szrButtons->Add(cmdYes, 0, wxBOTTOM, 8);
			szrButtons->Add(cmdNo, 0, wxBOTTOM | wxLEFT, 8);
			szrButtons->Add(cmdCancel, 0, wxBOTTOM | wxLEFT, 8);
		}
		szrAll->Add(szrButtons, 1, wxLEFT | wxRIGHT | wxALIGN_CENTER, 8);

	}

	panel->SetAutoLayout(TRUE);
	panel->SetSizer(szrAll);
	szrAll->SetSizeHints(this);

	CentreOnParent();
	FitInside();

}

ClientUIMDITransferResumeDialog::~ClientUIMDITransferResumeDialog()
{
}

int ClientUIMDITransferResumeDialog::ShowModal()
{
	#ifdef __WXMSW__
		MessageBeep(MB_ICONQUESTION);
	#endif
	return wxDialog::ShowModal();
}

void ClientUIMDITransferResumeDialog::OnClick(wxCommandEvent &event)
{
	EndModal(event.GetId());
}
