#ifndef Splash_H_
#define Splash_H_

class Splash : public wxFrame
{

public:
	Splash();
	virtual ~Splash();

protected:
	void OnErase(wxEraseEvent &event);
	void OnPaint(wxPaintEvent &event);
	void OnButton(wxCommandEvent &event);

protected:
	wxBitmap *m_bmp;
	bool m_button_clicked;

private:
	DECLARE_EVENT_TABLE()

};

#endif
