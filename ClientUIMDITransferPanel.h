#ifndef _ClientUIMDITransferPanel_H
#define _ClientUIMDITransferPanel_H

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

protected:
	void OnSize(wxSizeEvent &event);

protected:
	wxStaticText *AddRow(const wxString &caption, const wxString &value = wxEmptyString);

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

private:
	DECLARE_EVENT_TABLE()

};



#endif
