#ifndef ClientUIMDITransferPanel_H_
#define ClientUIMDITransferPanel_H_

#include "FileTransfer.h"

class ClientUIMDICanvas;
class wxFileName;

class ClientUIMDITransferPanel : public wxPanel
{

public:

	ClientUIMDITransferPanel(
		ClientUIMDICanvas *canvas, wxWindowID id,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxTAB_TRAVERSAL);

	virtual ~ClientUIMDITransferPanel();

	void Update(const FileTransfer &transfer);
	void OnClose();

	long GetTransferId();
	FileTransferState GetState();
	wxString GetNickname();
	wxString GetFilename();
	wxLongLong_t GetFileSize();
	long GetTime();
	long GetTimeleft();
	long GetCPS();
	wxLongLong_t GetFileSent();
	wxString GetStatus();

	void SetTransferId(long id);
	void SetState(FileTransferState state);
	void SetNickname(const wxString &nickname);
	void SetFilename(const wxString &filename);
	void SetFileSize(wxLongLong_t bytes);
	void SetTime(long seconds);
	void SetTimeleft(long seconds);
	void SetCPS(long cps);
	void SetFileSent(wxLongLong_t bytes);
	void SetStatus(const wxString &status);

	wxString GetShortFilename();

	virtual bool OnPopupMenu(wxMenu &menu);
	virtual bool OnPopupMenuItem(wxCommandEvent &event);

protected:
	void OnSize(wxSizeEvent &event);

protected:
	wxStaticText *AddRow(const wxString &caption, const wxString &value = wxEmptyString);
	bool IsSend();
	wxString GetTypeString() { return IsSend() ? wxT("Send") : wxT("Get"); }
	void UpdateCaption();
	void UpdateProgress();
	void OpenFile();
	void OpenFolder();
	wxFileName GetFilenameObject();

protected:
	ClientUIMDICanvas *m_canvas;
	wxPanel *m_pnlLeft, *m_pnlRight;
	wxSizer *m_szrLeft, *m_szrRight;
	wxStaticText *m_lblType;
	wxStaticText *m_lblNickname;
	wxStaticText *m_lblFilename;
	wxStaticText *m_lblSize;
	wxStaticText *m_lblTime;
	wxStaticText *m_lblLeft;
	wxStaticText *m_lblCPS;
	wxStaticText *m_lblSent;
	wxStaticText *m_lblStatus;
	wxGauge *m_gauge;

protected:
	long m_transferid;
	FileTransferState m_state;
	wxString m_nickname;
	wxString m_filename;
	wxLongLong_t m_filesize;
	long m_time;
	long m_timeleft;
	long m_cps;
	wxLongLong_t m_filesent;
	wxString m_status;
	bool m_open_file_when_complete;
	bool m_open_folder_when_complete;

private:
	DECLARE_EVENT_TABLE()

};



#endif
