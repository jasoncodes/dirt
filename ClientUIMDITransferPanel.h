#ifndef ClientUIMDITransferPanel_H_
#define ClientUIMDITransferPanel_H_

class ClientUIMDICanvas;

class ClientUIMDITransferPanel : public wxPanel
{

public:

	ClientUIMDITransferPanel(
		ClientUIMDICanvas *canvas, wxWindowID id,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxTAB_TRAVERSAL | wxNO_BORDER | wxCLIP_CHILDREN);

	virtual ~ClientUIMDITransferPanel();

	wxString GetNickname();
	wxString GetFilename();
	off_t GetFileSize();
	long GetTime();
	long GetTimeleft();
	long GetCPS();
	off_t GetFileSent();
	wxString GetStatus();

	void SetNickname(const wxString &nickname);
	void SetFilename(const wxString &filename);
	void SetFileSize(off_t bytes);
	void SetTime(long seconds);
	void SetTimeleft(long seconds);
	void SetCPS(long cps);
	void SetFileSent(off_t bytes);
	void SetStatus(const wxString &status);

	wxString GetShortFilename();

protected:
	void OnSize(wxSizeEvent &event);

protected:
	wxStaticText *AddRow(const wxString &caption, const wxString &value = wxEmptyString);
	bool IsSend();
	wxString GetTypeString() { return IsSend() ? "Send" : "Get"; }
	void UpdateCaption();
	void UpdateProgress();

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
	wxString m_nickname;
	wxString m_filename;
	off_t m_filesize;
	long m_time;
	long m_timeleft;
	long m_cps;
	off_t m_filesent;
	wxString m_status;

private:
	DECLARE_EVENT_TABLE()

};



#endif
