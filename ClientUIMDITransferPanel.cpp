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
}

ClientUIMDITransferPanel::~ClientUIMDITransferPanel()
{
}

void ClientUIMDITransferPanel::OnPaint(wxPaintEvent &event)
{
	wxPaintDC dc(this);
	dc.DrawText("This is ClientUIMDITransferPanel", 32, 32);
}
