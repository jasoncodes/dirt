#ifndef ClientUIMDITransferResumeDialog_H_
#define ClientUIMDITransferResumeDialog_H_

class ClientUIMDITransferResumeDialog : public wxDialog
{

public:
	ClientUIMDITransferResumeDialog(wxWindow *parent, const wxString &nickname, const wxString &org_filename, const wxString &new_filename, bool can_resume);
	virtual ~ClientUIMDITransferResumeDialog();

	virtual int ShowModal();

protected:
	void OnClick(wxCommandEvent &event);

private:
	DECLARE_EVENT_TABLE();
	DECLARE_NO_COPY_CLASS(ClientUIMDITransferResumeDialog)

};

#endif
