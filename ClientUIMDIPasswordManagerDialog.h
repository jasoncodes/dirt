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


#ifndef ClientUIMDIPasswords_H_
#define ClientUIMDIPasswords_H_

class ClientUIMDIFrame;
class Client;
class ClientConfig;

#include "ByteBuffer.h"

class ClientUIMDIPasswordManagerDialog : public wxDialog
{

public:
	ClientUIMDIPasswordManagerDialog(ClientUIMDIFrame *parent);
	virtual ~ClientUIMDIPasswordManagerDialog();

protected:
	void OnList(wxCommandEvent &event);
	void OnListDblClick(wxCommandEvent &event);
	void OnOK(wxCommandEvent &event);
	void OnAdd(wxCommandEvent &event);
	void OnRemove(wxCommandEvent &event);

protected:
	void LoadSettings();
	bool SaveSettings();

protected:
	Client *m_client;
	ClientConfig *m_config;

	wxListBox *m_lstPasswords;
	ByteBufferArray m_passwords;

	wxButton *m_cmdAdd;
	wxButton *m_cmdRemove;

private:
	DECLARE_EVENT_TABLE()
	DECLARE_NO_COPY_CLASS(ClientUIMDIPasswordManagerDialog)

};

#endif
