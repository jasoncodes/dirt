#ifndef SwitchBarParent_H_
#define SwitchBarParent_H_

class SwitchBar;
class SwitchBarChild;
class SwitchBarCanvas;

class SwitchBarParent : public wxMDIParentFrame
{

	friend class SwitchBarChild;
	friend class SwitchBarCanvas;

public:
	SwitchBarParent(
		wxWindow *parent, const wxWindowID id, const wxString& title,
		const wxPoint& pos, const wxSize& size, const long style,
		const wxString& name = wxFrameNameStr);

	virtual ~SwitchBarParent();

	inline SwitchBar* GetSwitchBar() { return m_switchbar; }

	void FocusCanvas(SwitchBarCanvas *canvas);

protected:
	void OnSize(wxSizeEvent& event);
	void OnClose(wxCloseEvent& event);
	void OnSwitchBar(wxCommandEvent& event);
	void OnSwitchBarMenu(wxCommandEvent& event);

	void OnWindowMinimize(wxCommandEvent& event);
	void OnWindowClose(wxCommandEvent& event);
	void OnWindowCascade(wxCommandEvent& event);
	void OnWindowTile(wxCommandEvent& event);
	void OnWindowNext(wxCommandEvent& event);
	void OnWindowPrev(wxCommandEvent& event);
	void OnWindowWindows(wxCommandEvent& event);

	void OnSwitchBarRestore(wxCommandEvent& event);
	void OnSwitchBarMinimize(wxCommandEvent& event);
	void OnSwitchBarClose(wxCommandEvent& event);

	void OnUpdateWindowMenuTimer(wxTimerEvent &event);
	void OnUpdateWindowMenuIdle(wxIdleEvent &event);

protected:
	SwitchBarChild* NewWindow(SwitchBarCanvas *canvas, bool focus);
	void NextChild(bool bPrevious);
	void DoUpdateWindowMenu();
	void UpdateCheckMenuItem(int id, const wxString &label, bool enabled, bool checked);
	SwitchBarCanvas *GetActiveCanvas();
	size_t GetAcceleratorCount() const { return m_accelerator_count; }
	wxAcceleratorEntry *GetAccelerators() const { return m_accelerators; }

	virtual SwitchBarChild* OnCreateNewChild(wxPoint pos, wxSize size, bool bMaximized, SwitchBarCanvas *canvas);

	void UpdateWindowMenu();

	inline wxMenu *GetWindowMenu() { return mnuWindow; }

private:
	DECLARE_EVENT_TABLE()

protected:
	SwitchBar *m_switchbar;
	wxTimer *tmrUpdateWindowMenu;
	wxMenu *mnuWindow;
	int num_window_menus;
	int switchbar_popup_button_index;
	SwitchBarCanvas *switchbar_popup_canvas;
	wxAcceleratorEntry *m_accelerators;
	size_t m_accelerator_count;

};

#endif
