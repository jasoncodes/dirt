#ifndef TristateConfigPanel_H_
#define TristateConfigPanel_H_

#include "ConfigFile.h"
#include "RadioBoxPanel.h"
#include <wx/wave.h>

class TristateConfigPanel : public RadioBoxPanel
{

public:
	TristateConfigPanel(wxWindow *parent, wxWindowID id = -1,
		const wxString &caption = wxT("Tristate Config Panel"),
		const wxString &filespec = wxT(""),
		const bool wave_preview = false,
		const wxPoint& pos = wxDefaultPosition);
	virtual ~TristateConfigPanel();

	virtual Config::TristateMode GetMode() const;
	virtual wxString GetPath() const;

	virtual void SetMode(Config::TristateMode mode);
	virtual void SetPath(const wxString &path);

	virtual bool Enable(bool enabled);
	virtual void Enable(Config::TristateMode mode, bool enabled);

protected:
	void OnText(wxCommandEvent &event);
	void OnBrowse(wxCommandEvent &event);
	void OnWavePreview(wxCommandEvent &event);

protected:
	virtual void OnSelectionChanged(int n);

protected:
	wxTextCtrl *m_txt;
	wxButton *m_cmdBrowse;
	wxBitmapButton *m_cmdWavePreview;
	wxString m_filespec;
	bool m_wave_preview;
	#if wxUSE_WAVE
		wxWave m_wave;
	#endif

private:
	DECLARE_EVENT_TABLE()
	DECLARE_NO_COPY_CLASS(TristateConfigPanel)

};

#endif
