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
