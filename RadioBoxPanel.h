#ifndef RadioBoxPanel_H_
#define RadioBoxPanel_H_

class RadioBoxPanel : public wxPanel
{

public:
	RadioBoxPanel(wxWindow *parent, wxWindowID id = -1,
		const wxString &caption = wxT("RadioBox Panel"),
		const wxPoint& pos = wxDefaultPosition,
		size_t num = 0, const wxString *choices = 0);
	virtual ~RadioBoxPanel();

	virtual int GetSelection() const;
	virtual void SetSelection(int n);

	virtual bool Enable(bool enabled);
	virtual void Enable(int n, bool enabled);

#ifdef __WXMSW__
	virtual long MSWWindowProc(WXUINT nMsg, WXWPARAM wParam, WXLPARAM lParam);
#endif

protected:
	void OnSize(wxSizeEvent &event);
	void OnRadio(wxCommandEvent &event);

protected:
	virtual wxPanel *GetPanel() const { return m_pnl; }
	virtual void SetSizes(); // call after panel is populated and has a sizer

	virtual void OnSelectionChanged(int n) = 0;
	virtual void SendChangeEvent();

protected:
	wxRadioBox *m_boxRadio;
	wxPanel *m_pnl;
	bool m_sizes_set;

private:
	DECLARE_EVENT_TABLE()
	DECLARE_NO_COPY_CLASS(RadioBoxPanel)

};

#endif
