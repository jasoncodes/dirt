#ifndef SwitchBarParentGeneric_H_
#define SwitchBarParentGeneric_H_

class SwitchBarChild;

class SwitchBarParent : public wxFrame
{

	friend class SwitchBarChild;

public:
	SwitchBarParent(
		wxWindow *parent, const wxWindowID id, const wxString& title,
		const wxPoint& pos, const wxSize& size, const long style,
		const wxString& name = wxFrameNameStr);

	virtual ~SwitchBarParent();

	inline SwitchBar* GetSwitchBar() { return m_switchbar; }
	inline wxWindow* GetMDIChildWindow() { return m_client_area; }

	void FocusCanvas(SwitchBarCanvas *canvas);

	wxWindow* GetActiveChildWindow() const;
	SwitchBarChild* GetActiveChild() const;
	
protected:
	void OnSize(wxSizeEvent& event);
	void OnSwitchBar(wxCommandEvent& event);
	void OnSwitchBarMiddleClick(wxCommandEvent& event);
	void OnSwitchBarMenu(wxCommandEvent& event);
	void OnSwitchBarMenuItem(wxCommandEvent& event);

	void OnWindowMinimize(wxCommandEvent& event);
	void OnWindowClose(wxCommandEvent& event);
	void OnWindowNext(wxCommandEvent& event);
	void OnWindowPrev(wxCommandEvent& event);

protected:
	SwitchBarChild* NewWindow(SwitchBarCanvas *canvas, bool focus);
	void NextChild(bool bPrevious);
	SwitchBarCanvas *GetActiveCanvas() const;
	void CloseCanvas(SwitchBarCanvas *canvas);
	
	size_t GetAcceleratorCount() const { return m_accelerator_count; }
	wxAcceleratorEntry *GetAccelerators() const { return m_accelerators; }

	inline wxMenu *GetWindowMenu() { return NULL; }

	void AddVisibleCanvas(SwitchBarCanvas *canvas);
	void RemoveVisibleCanvas(SwitchBarCanvas *canvas);
	void SelectLastVisibleWindow();

private:
	DECLARE_EVENT_TABLE()
	DECLARE_NO_COPY_CLASS(SwitchBarParent)

protected:
	SwitchBar *m_switchbar;
	wxPanel *m_client_area;
	SwitchBarCanvasArray m_visible_windows;
	int m_switchbar_popup_button_index;
	SwitchBarCanvas *m_switchbar_popup_canvas;
	wxAcceleratorEntry *m_accelerators;
	size_t m_accelerator_count;

};

#endif
