#include <jni.h>
#include <jawt.h>
#include <jawt_md.h>
#include "au_com_gslabs_dirt_jni_Linux.h"

JNIEXPORT void JNICALL Java_au_com_gslabs_dirt_jni_Linux_setDemandsAttention
  (JNIEnv *env, jobject canvas, jobject frame) {    
    //(JAWT_Win32DrawingSurfaceInfo)
    JAWT awt;
    JAWT_DrawingSurface* ds;
    JAWT_DrawingSurfaceInfo* dsi;
//    JAWT_Win32DrawingSurfaceInfo* dsi_win32;
    jint lock;
    jint result;
  
//    BOOL retorno;
    
    awt.version = JAWT_VERSION_1_4;
    result = JAWT_GetAWT(env, &awt);
  
/*
    ds = awt.GetDrawingSurface(env, frame);
    lock = ds->Lock(ds);
    dsi = ds->GetDrawingSurfaceInfo(ds);
    dsi_win32 = (JAWT_Win32DrawingSurfaceInfo*)dsi->platformInfo;
  
    retorno = FlashWindow(dsi_win32->hwnd, TRUE);
    ds->FreeDrawingSurfaceInfo(dsi);
    ds->Unlock(ds);
    awt.FreeDrawingSurface(ds);
*/

puts("testing");
Display *xdisplay = qt_xdisplay();
    Window rootwin = qt_xrootwin(), winId = this->winId();

    static Atom demandsAttention = XInternAtom(xdisplay, "_NET_WM_STATE_DEMANDS_ATTENTION", 1);
    static Atom wmState = XInternAtom(xdisplay, "_NET_WM_STATE", 1);

    XEvent e;
    e.xclient.type = ClientMessage;
    e.xclient.message_type = wmState;
    e.xclient.display = xdisplay;
    e.xclient.window = winId;
    e.xclient.format = 32;
    e.xclient.data.l[1] = demandsAttention;
    e.xclient.data.l[2] = 0l;
    e.xclient.data.l[3] = 0l;
    e.xclient.data.l[4] = 0l;
    
    int yes = 1;

    if (yes) {
        e.xclient.data.l[0] = 1;
   }
    else {
        e.xclient.data.l[0] = 0;
    }
    XSendEvent(xdisplay, rootwin, False, (SubstructureRedirectMask | SubstructureNotifyMask), &e);

}

