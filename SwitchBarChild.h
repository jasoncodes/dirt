#ifndef SwitchBarChild_H_
#define SwitchBarChild_H_

class SwitchBarParent;
class SwitchBarCanvas;

class SwitchBarChild : public wxMDIChildFrame
{

public:
	SwitchBarChild(SwitchBarParent *parent, const wxPoint& pos, const wxSize& size, const long style, SwitchBarCanvas *canvas);
	virtual ~SwitchBarChild();

	inline SwitchBarCanvas* GetCanvas() { return m_canvas; }

protected:
	void OnActivate(wxActivateEvent& event);
	void OnSize(wxSizeEvent& event);
	void OnMove(wxMoveEvent& event);
	void OnClose(wxCloseEvent& event);

protected:
	SwitchBarCanvas *m_canvas;
	SwitchBarParent *m_parent;

private:
	DECLARE_EVENT_TABLE()

};

#endif
