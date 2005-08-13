#include <jni.h>
#include <jawt.h>
#include <jawt_md.h>
#include "au_com_gslabs_dirt_jni_Linux.h"

JNIEXPORT void JNICALL Java_au_com_gslabs_dirt_jni_Linux_setDemandsAttention
  (JNIEnv *env, jobject canvas, jobject frame)
{
	
	JAWT awt;
	JAWT_DrawingSurface* ds;
	JAWT_DrawingSurfaceInfo* dsi;
	JAWT_X11DrawingSurfaceInfo* dsi_x11;
	jint lock;
	jint result;
	Window rootwin;
	
	static Atom demandsAttention = 0;
	static Atom wmState = 0;
	
	XEvent e;
	
	awt.version = JAWT_VERSION_1_4;
	result = JAWT_GetAWT(env, &awt);
	
	ds = awt.GetDrawingSurface(env, frame);
	lock = ds->Lock(ds);
	dsi = ds->GetDrawingSurfaceInfo(ds);
	dsi_x11 = (JAWT_X11DrawingSurfaceInfo*)dsi->platformInfo;

	if (demandsAttention == 0)
	{
		demandsAttention = XInternAtom(dsi_x11->display, "_NET_WM_STATE_DEMANDS_ATTENTION", 1);
		wmState = XInternAtom(dsi_x11->display, "_NET_WM_STATE", 1);
	}
	
    e.xclient.type = ClientMessage;
    e.xclient.message_type = wmState;
    e.xclient.display = dsi_x11->display;
    e.xclient.window = dsi_x11->drawable;
    e.xclient.format = 32;
    e.xclient.data.l[0] = 1; // true
    e.xclient.data.l[1] = demandsAttention;
    e.xclient.data.l[2] = 0l;
    e.xclient.data.l[3] = 0l;
    e.xclient.data.l[4] = 0l;
	
	rootwin = RootWindow(dsi_x11->display, DefaultScreen(dsi_x11->display));
	
	XSendEvent(dsi_x11->display, rootwin, False, (SubstructureRedirectMask | SubstructureNotifyMask), &e);
	
	ds->FreeDrawingSurfaceInfo(dsi);
    ds->Unlock(ds);
    awt.FreeDrawingSurface(ds);
	
}
