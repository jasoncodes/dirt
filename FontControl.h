#ifndef FontControl_H_
#define FontControl_H_

class FontControl : public wxPanel
{

public:
	FontControl(
		wxWindow *parent, int id,
		wxPoint pos = wxDefaultPosition, wxSize size = wxDefaultSize);
	virtual ~FontControl();

	virtual wxFont GetFont() const;
	virtual bool SetFont(const wxFont &font);

protected:
	void OnBrowse(wxCommandEvent &event);

protected:
	wxFontData m_data;
	wxPanel *m_pnlPreview;

private:
	DECLARE_EVENT_TABLE()

};

#endif
