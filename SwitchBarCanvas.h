#ifndef SwitchBarCanvas_H_
#define SwitchBarCanvas_H_

class SwitchBarParent;
class SwitchBar;

class SwitchBarCanvas : public wxPanel
{

	friend class SwitchBarParent;

public:
	SwitchBarCanvas(SwitchBarParent *parent, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);
	virtual ~SwitchBarCanvas();

	virtual wxString GetTitle();
	virtual void SetTitle(const wxString &title);

	virtual wxIcon GetIcon();
	virtual void SetIcon(const wxIcon &icon);

	virtual bool IsClosable() { return true; }
	virtual bool IsAttached() { return GetParent() != (wxWindow*)m_parent; }

	virtual void OnAttach() {}
	virtual void OnDetach() {}
	virtual void OnActivate() {}
	virtual void OnClose() {}

	SwitchBar* GetSwitchBar();

protected:
	bool saved_state_maximized;
	wxRect saved_state_rect;
	bool saved_state_valid;
	SwitchBarParent *m_parent;

private:
	DECLARE_EVENT_TABLE()

};

#endif
