#ifndef Splash_H_
#define Splash_H_

class Splash : public wxFrame
{

public:
	Splash();
	virtual ~Splash();

protected:
	void OnButton(wxCommandEvent &event);

protected:
	bool m_button_clicked;

private:
	DECLARE_EVENT_TABLE()

};

#endif
