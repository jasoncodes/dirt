#ifndef _ClientUIMDITransferPanel_H
#define _ClientUIMDITransferPanel_H

class ClientUIMDICanvas;

class ClientUIMDITransferPanel : public wxPanel
{

public:

	ClientUIMDITransferPanel(
		ClientUIMDICanvas *canvas, wxWindowID id,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxTAB_TRAVERSAL | wxNO_BORDER);

	virtual ~ClientUIMDITransferPanel();

protected:
	void OnPaint(wxPaintEvent &event);

protected:
	ClientUIMDICanvas *m_canvas;

private:
	DECLARE_EVENT_TABLE()

};



#endif
