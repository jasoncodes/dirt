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
	void OnSwitchBarMiddleClick(wxCommandEvent& event);
	void OnSwitchBarMenu(wxCommandEvent& event);
	void OnSwitchBarMenuItem(wxCommandEvent& event);

	void OnWindowMinimize(wxCommandEvent& event);
	void OnWindowClose(wxCommandEvent& event);
	void OnWindowCascade(wxCommandEvent& event);
	void OnWindowTile(wxCommandEvent& event);
	void OnWindowNext(wxCommandEvent& event);
	void OnWindowPrev(wxCommandEvent& event);
	void OnWindowWindows(wxCommandEvent& event);

	void OnUpdateWindowMenuTimer(wxTimerEvent &event);
	void OnUpdateWindowMenuIdle(wxIdleEvent &event);

protected:
	SwitchBarChild* NewWindow(SwitchBarCanvas *canvas, bool focus);
	void NextChild(bool bPrevious);
	void DoUpdateWindowMenu();
	void UpdateCheckMenuItem(int id, const wxString &label, bool enabled, bool checked);
	SwitchBarCanvas *GetActiveCanvas();
	void CloseCanvas(SwitchBarCanvas *canvas);
	size_t GetAcceleratorCount() const { return m_accelerator_count; }
	wxAcceleratorEntry *GetAccelerators() const { return m_accelerators; }

	virtual SwitchBarChild* OnCreateNewChild(wxPoint pos, wxSize size, bool bMaximized, SwitchBarCanvas *canvas);

	void UpdateWindowMenu();

	inline wxMenu *GetWindowMenu() { return m_mnuWindow; }

private:
	DECLARE_EVENT_TABLE()
    DECLARE_NO_COPY_CLASS(SwitchBarParent)

protected:
	SwitchBar *m_switchbar;
	wxTimer *m_tmrUpdateWindowMenu;
	wxMenu *m_mnuWindow;
	int m_num_window_menus;
	int m_switchbar_popup_button_index;
	SwitchBarCanvas *m_switchbar_popup_canvas;
	wxAcceleratorEntry *m_accelerators;
	size_t m_accelerator_count;

};

#endif
