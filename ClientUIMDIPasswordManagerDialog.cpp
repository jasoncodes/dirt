#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: ClientUIMDIPasswordManagerDialog.cpp,v 1.3 2003-08-06 15:30:53 jason Exp $)

#include "ClientUIMDIPasswordManagerDialog.h"
#include "ClientUIMDIFrame.h"

class ClientUIMDIPasswordManagerAddDialog : public wxDialog
{

public:
	ClientUIMDIPasswordManagerAddDialog(ClientUIMDIPasswordManagerDialog *parent, wxListBox *lst, ByteBufferArray &passwords)
		: wxDialog(parent, wxID_ANY, wxT("Add Password"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxCLIP_CHILDREN | wxNO_FULL_REPAINT_ON_RESIZE | wxTAB_TRAVERSAL),
		m_lst(lst), m_passwords(passwords)
	{

		wxPanel *panel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNO_BORDER | wxCLIP_CHILDREN | wxNO_FULL_REPAINT_ON_RESIZE | wxTAB_TRAVERSAL);
		
		wxStaticText *lblServerName = new wxStaticText(panel, wxID_ANY, wxT("Server Name:"));
		m_txtServerName = new wxTextCtrl(panel, wxID_ANY, wxEmptyString);
		FixBorder(m_txtServerName);
		wxStaticText *lblPassword = new wxStaticText(panel, wxID_ANY, wxT("Password:"));
		m_txtPassword = new wxTextCtrl(panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(128, -1), wxTE_PASSWORD);
		FixBorder(m_txtPassword);
		wxStaticText *lblConfirm = new wxStaticText(panel, wxID_ANY, wxT("Confirm:"));
		m_txtConfirm = new wxTextCtrl(panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(128, -1), wxTE_PASSWORD);
		FixBorder(m_txtConfirm);

		m_cmdOK = new wxButton(panel, wxID_OK, wxT("OK"));
		m_cmdOK->Enable(false);
		wxButton *cmdCancel = new wxButton(panel, wxID_CANCEL, wxT("Cancel"));

		wxBoxSizer *szrAll = new wxBoxSizer(wxHORIZONTAL);
		{

			wxFlexGridSizer *szrMain = new wxFlexGridSizer(2, 8, 8);
			szrMain->AddGrowableCol(1);
			{
				szrMain->Add(lblServerName, 0, wxALIGN_CENTER_VERTICAL);
				szrMain->Add(m_txtServerName, 0, wxEXPAND);
				szrMain->Add(lblPassword, 0, wxALIGN_CENTER_VERTICAL);
				szrMain->Add(m_txtPassword, 0, wxEXPAND);
				szrMain->Add(lblConfirm, 0, wxALIGN_CENTER_VERTICAL);
				szrMain->Add(m_txtConfirm, 0, wxEXPAND);
			}
			szrAll->Add(szrMain, 1, wxALL | wxEXPAND, 8);

			wxBoxSizer *szrRight = new wxBoxSizer(wxVERTICAL);
			{
				szrRight->Add(m_cmdOK, 0, wxTOP | wxBOTTOM | wxEXPAND, 8);
				szrRight->Add(cmdCancel, 0, wxBOTTOM | wxEXPAND, 8);
			}
			szrAll->Add(szrRight, 0, wxRIGHT | wxEXPAND, 8);

		}

		panel->SetAutoLayout(TRUE);
		panel->SetSizer(szrAll);
		szrAll->SetSizeHints(this);

		FitInside();
		CentreOnParent();
		m_cmdOK->SetDefault();
		CenterOnParent();

	}

	void SetServerName(const wxString &name)
	{
		m_txtServerName->SetValue(name);
		m_txtServerName->Enable(false);
		m_txtPassword->SetFocus();
		SetTitle(wxT("Change Password"));
	}

protected:
	void OnOK(wxCommandEvent &event)
	{
		if (AddPassword())
		{
			event.Skip();
		}
	}

	void OnText(wxCommandEvent &WXUNUSED(event))
	{
		m_cmdOK->Enable(
			m_txtServerName->GetValue().Length() &&
			m_txtPassword->GetValue().Length() &&
			m_txtPassword->GetValue() == m_txtConfirm->GetValue());
	}

	bool AddPassword()
	{
		int index = m_lst->FindString(m_txtServerName->GetValue());
		if (index > -1)
		{
			wxString msg;
			msg << wxT("Replace existing entry for \"")
				<< m_lst->GetString(index)
				<< wxT("\"?");
			int answer =
				wxMessageBox(msg, GetTitle(), wxOK|wxCANCEL|wxICON_QUESTION, this);
			if (answer != wxOK)
			{
				return false;
			}
			size_t password_index =
				(size_t)m_lst->GetClientData(index);
			m_passwords[password_index] = m_txtPassword->GetValue();
			m_lst->Delete(index);
			m_lst->Append(m_txtServerName->GetValue(), (void*)password_index);
			index = m_lst->FindString(m_txtServerName->GetValue());
		}
		else
		{
			m_lst->Append(m_txtServerName->GetValue(), (void*)m_passwords.GetCount());
			m_passwords.Add(m_txtPassword->GetValue());
			index = m_lst->FindString(m_txtServerName->GetValue());
		}
		for (int i = 0; i < m_lst->GetCount(); ++i)
		{
			m_lst->Deselect(i);
		}
		m_lst->SetSelection(index);
		return true;
	}

protected:
	wxListBox *m_lst;
	ByteBufferArray &m_passwords;

	wxTextCtrl *m_txtServerName;
	wxTextCtrl *m_txtPassword;
	wxTextCtrl *m_txtConfirm;

	wxButton *m_cmdOK;

private:
	DECLARE_EVENT_TABLE()
	DECLARE_NO_COPY_CLASS(ClientUIMDIPasswordManagerAddDialog)

};

BEGIN_EVENT_TABLE(ClientUIMDIPasswordManagerAddDialog, wxDialog)
	EVT_BUTTON(wxID_OK, ClientUIMDIPasswordManagerAddDialog::OnOK)
	EVT_TEXT(wxID_ANY, ClientUIMDIPasswordManagerAddDialog::OnText)
END_EVENT_TABLE()

enum
{
	ID_LIST = 1,
	ID_ADD,
	ID_REMOVE
};

BEGIN_EVENT_TABLE(ClientUIMDIPasswordManagerDialog, wxDialog)
	EVT_LISTBOX(ID_LIST, ClientUIMDIPasswordManagerDialog::OnList)
	EVT_LISTBOX_DCLICK(ID_LIST, ClientUIMDIPasswordManagerDialog::OnListDblClick)
	EVT_BUTTON(wxID_OK, ClientUIMDIPasswordManagerDialog::OnOK)
	EVT_BUTTON(ID_ADD, ClientUIMDIPasswordManagerDialog::OnAdd)
	EVT_BUTTON(ID_REMOVE, ClientUIMDIPasswordManagerDialog::OnRemove)
END_EVENT_TABLE()

ClientUIMDIPasswordManagerDialog::ClientUIMDIPasswordManagerDialog(ClientUIMDIFrame *parent)
	: wxDialog(parent, wxID_ANY, wxT("Password Manager"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxCLIP_CHILDREN | wxNO_FULL_REPAINT_ON_RESIZE | wxTAB_TRAVERSAL)
{
	
	m_client = parent->GetClient();
	m_config = &(m_client->GetConfig());
	
	wxPanel *panel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNO_BORDER | wxCLIP_CHILDREN | wxNO_FULL_REPAINT_ON_RESIZE | wxTAB_TRAVERSAL);

	m_lstPasswords = new wxListBox(panel, ID_LIST, wxDefaultPosition, wxSize(200,150), 0, NULL, wxLB_EXTENDED|wxLB_HSCROLL|wxLB_NEEDED_SB|wxLB_SORT);
	FixBorder(m_lstPasswords);

	m_cmdAdd = new wxButton(panel, ID_ADD, wxT("&Add"));
	m_cmdRemove = new wxButton(panel, ID_REMOVE, wxT("&Remove"));
	m_cmdRemove->Enable(false);

	wxButton *cmdOK = new wxButton(panel, wxID_OK, wxT("OK"));
	wxButton *cmdCancel = new wxButton(panel, wxID_CANCEL, wxT("Cancel"));

	wxBoxSizer *szrAll = new wxBoxSizer(wxHORIZONTAL);
	{

		wxBoxSizer *szrMain = new wxBoxSizer(wxVERTICAL);
		{

			szrMain->Add(m_lstPasswords, 1, wxBOTTOM|wxEXPAND, 8);

			wxBoxSizer *szrBottom = new wxBoxSizer(wxHORIZONTAL);
			{
				szrBottom->Add(m_cmdAdd, 0, wxRIGHT, 8);
				szrBottom->Add(m_cmdRemove, 0, 0, 8);
			}
			szrMain->Add(szrBottom, 0, wxALIGN_CENTRE, 8);

		}
		szrAll->Add(szrMain, 1, wxALL | wxEXPAND, 8);

		wxBoxSizer *szrRight = new wxBoxSizer(wxVERTICAL);
		{
			szrRight->Add(cmdOK, 0, wxTOP | wxBOTTOM | wxEXPAND, 8);
			szrRight->Add(cmdCancel, 0, wxBOTTOM | wxEXPAND, 8);
		}
		szrAll->Add(szrRight, 0, wxTOP | wxRIGHT | wxEXPAND, 8);

	}

	LoadSettings();

	panel->SetAutoLayout(TRUE);
	panel->SetSizer(szrAll);
	szrAll->SetSizeHints(this);

	FitInside();
	CentreOnParent();
	cmdOK->SetDefault();
	CenterOnParent();
	ShowModal();

}

ClientUIMDIPasswordManagerDialog::~ClientUIMDIPasswordManagerDialog()
{
}

void ClientUIMDIPasswordManagerDialog::OnList(wxCommandEvent &WXUNUSED(event))
{
	wxArrayInt indexes;
	m_lstPasswords->GetSelections(indexes);
	m_cmdRemove->Enable(indexes.GetCount() > 0);
}

void ClientUIMDIPasswordManagerDialog::OnListDblClick(wxCommandEvent &WXUNUSED(event))
{
	wxArrayInt indexes;
	m_lstPasswords->GetSelections(indexes);
	if (indexes.GetCount() == 1)
	{
		ClientUIMDIPasswordManagerAddDialog dlg(this, m_lstPasswords, m_passwords);
		dlg.SetServerName(m_lstPasswords->GetString(indexes[0u]));
		dlg.ShowModal();
	}
}

void ClientUIMDIPasswordManagerDialog::OnOK(wxCommandEvent &event)
{
	if (SaveSettings())
	{
		event.Skip();
	}
}

void ClientUIMDIPasswordManagerDialog::LoadSettings()
{
	m_lstPasswords->Clear();
	m_passwords.Empty();
	wxArrayString server_names = m_config->GetSavedPasswordServerNames();
	for (size_t i = 0; i < server_names.GetCount(); ++i)
	{
		m_lstPasswords->Append(server_names[i], (void*)m_passwords.GetCount());
		m_passwords.Add(m_config->GetSavedPassword(server_names[i], false));
	}
}

bool ClientUIMDIPasswordManagerDialog::SaveSettings()
{
	bool success = m_config->DeleteSavedPasswords();
	for (int i = 0; i < m_lstPasswords->GetCount(); ++i)
	{
		wxString key = m_lstPasswords->GetString(i);
		size_t j = (size_t)m_lstPasswords->GetClientData(i);
		wxString value = m_passwords[j];
		if (!m_config->SetSavedPassword(key, value))
		{
			success = false;
		}
	}
	if (!m_config->Flush())
	{
		success = false;
	}
	return success;
}

void ClientUIMDIPasswordManagerDialog::OnAdd(wxCommandEvent &event)
{
	ClientUIMDIPasswordManagerAddDialog dlg(this, m_lstPasswords, m_passwords);
	dlg.ShowModal();
	OnList(event);
}

static int ReverseSortInt(int *x, int *y)
{
	wxASSERT(x && y);
	if (*x == *y)
	{
		return 0;
	}
	return (*x < *y) ? 1 : -1;
}

void ClientUIMDIPasswordManagerDialog::OnRemove(wxCommandEvent &WXUNUSED(event))
{
	wxArrayInt indexes;
	m_lstPasswords->GetSelections(indexes);
	indexes.Sort(ReverseSortInt);
	if (indexes.GetCount())
	{
		wxString msg;
		if (indexes.GetCount() == 1)
		{
			msg << wxT("Remove saved password for \"")
				<< m_lstPasswords->GetString(indexes[0u])
				<< wxT("\"?");
		}
		else
		{
			msg << wxT("Remove ") << indexes.GetCount()
				<< wxT(" saved passwords?");
		}
		int answer =
			wxMessageBox(msg, GetTitle(), wxOK|wxCANCEL|wxICON_QUESTION, this);
		if (answer != wxOK)
		{
			return;
		}
		for (size_t i = 0; i < indexes.GetCount(); ++i)
		{
			m_lstPasswords->Delete(indexes[i]);
		}
		m_cmdRemove->Enable(false);
	}
}
