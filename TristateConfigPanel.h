#ifndef TristateConfigPanel_H_
#define TristateConfigPanel_H_

#include "ConfigFile.h"

class TristateConfigPanel : public wxPanel
{

public:
	TristateConfigPanel(wxWindow *parent, wxWindowID id = -1,
		const wxString &caption = wxT("Tristate Config Panel"), const wxString &filespec = wxT(""),
		const wxPoint& pos = wxDefaultPosition);
	virtual ~TristateConfigPanel();

	virtual Config::TristateMode GetMode() const;
	virtual wxString GetPath() const;

	virtual void SetMode(Config::TristateMode mode);
	virtual void SetPath(const wxString &path);

	virtual bool Enable(bool enabled);
	virtual void Enable(Config::TristateMode mode, bool enabled);

protected:
	void OnSize(wxSizeEvent &event);
	void OnRadio(wxCommandEvent &event);
	void OnText(wxCommandEvent &event);
	void OnBrowse(wxCommandEvent &event);

protected:
	virtual void SendChangeEvent();

protected:
	wxRadioBox *m_boxRadio;
	wxPanel *m_pnlPath;
	wxTextCtrl *m_txt;
	wxButton *m_cmdBrowse;
	wxString m_filespec;

private:
	DECLARE_EVENT_TABLE()

};

#endif
