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
	virtual bool IsAttached();

	virtual void OnAttach() {}
	virtual void OnDetach() {}
	virtual void OnActivate() {}
	virtual void OnClose() {}
	virtual bool OnPopupMenu(wxMenu &WXUNUSED(menu)) { return true; }
	virtual bool OnPopupMenuItem(wxCommandEvent &WXUNUSED(event)) { return true; }

	SwitchBar* GetSwitchBar();

protected:
	bool m_saved_state_maximized;
	wxRect m_saved_state_rect;
	bool m_saved_state_valid;
	SwitchBarParent *m_parent;

private:
	DECLARE_EVENT_TABLE()
	DECLARE_NO_COPY_CLASS(SwitchBarCanvas)

};

#include <wx/dynarray.h>
#ifndef WX_DEFINE_ARRAY_PTR
	#define WX_DEFINE_ARRAY_PTR WX_DEFINE_ARRAY
#endif
WX_DEFINE_ARRAY_PTR(SwitchBarCanvas*, SwitchBarCanvasArray);

#endif
