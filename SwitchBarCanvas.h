#ifndef _SwitchBarCanvas_H
#define _SwitchBarCanvas_H

class SwitchBarParent;
class SwitchBarChild;

class SwitchBarCanvas : public wxPanel
{

	friend SwitchBarParent;

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

protected:
	bool saved_state_maximized;
	wxRect saved_state_rect;
	bool saved_state_valid;
	SwitchBarParent *m_parent;

private:
	DECLARE_EVENT_TABLE()

};

#endif
