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
	void OnClient(wxCommandEvent &event);

protected:
	wxBitmap *m_bmp;

private:
	DECLARE_EVENT_TABLE()

};

#endif
