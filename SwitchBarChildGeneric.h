#ifndef SwitchBarChild_H_
#define SwitchBarChild_H_

class SwitchBarParent;
class SwitchBarCanvas;

class SwitchBarChild : public wxPanel
{

public:
	SwitchBarChild(SwitchBarParent *parent);
	virtual ~SwitchBarChild();

	void SetCanvas(SwitchBarCanvas *canvas);
	inline SwitchBarCanvas* GetCanvas() { return m_canvas; }

protected:
	void OnSize(wxSizeEvent& event);

protected:
	SwitchBarParent *m_parent;
	SwitchBarCanvas *m_canvas;

private:
	DECLARE_EVENT_TABLE()
	DECLARE_NO_COPY_CLASS(SwitchBarChild)

};

#endif
