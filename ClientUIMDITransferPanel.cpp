#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif

#include "ClientUIMDITransferPanel.h"
#include "ClientUIMDICanvas.h"

BEGIN_EVENT_TABLE(ClientUIMDITransferPanel, wxPanel)
	EVT_PAINT(ClientUIMDITransferPanel::OnPaint)
END_EVENT_TABLE()

ClientUIMDITransferPanel::ClientUIMDITransferPanel(
	ClientUIMDICanvas *canvas, wxWindowID id,
	const wxPoint& pos, const wxSize& size,
	long style)
	: wxPanel(canvas, id, pos, size, style)
{
	m_canvas = canvas;
	m_gauge = new wxGauge(
		this, -1, 100,
		wxPoint(32,64), wxSize(256, 24),
		wxGA_SMOOTH | wxNO_BORDER);
	m_gauge->SetValue(50);
}

ClientUIMDITransferPanel::~ClientUIMDITransferPanel()
{
}

void ClientUIMDITransferPanel::OnPaint(wxPaintEvent &event)
{
	wxPaintDC dc(this);
	dc.DrawText("This is ClientUIMDITransferPanel", 32, 32);
}
