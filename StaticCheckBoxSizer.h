#ifndef StaticCheckBoxSizer_H_
#define StaticCheckBoxSizer_H_

class StaticCheckBoxSizer : public wxStaticBoxSizer
{

public:
	StaticCheckBoxSizer(wxStaticBox *box, wxCheckBox *chk, int orient);
	virtual ~StaticCheckBoxSizer();
	virtual void RecalcSizes();

protected:
	wxStaticBox *m_box;
	wxCheckBox *m_chk;
	wxEvtHandler *m_evt;

};

#endif
