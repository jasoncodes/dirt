#include <jni.h>
#include <jawt.h>
#include <jawt_md.h>
#include "au_com_gslabs_dirt_jni_Win32.h"

JNIEXPORT void JNICALL Java_au_com_gslabs_dirt_jni_Win32_FlashWindow
  (JNIEnv *env, jobject canvas, jobject frame) {    
    //(JAWT_Win32DrawingSurfaceInfo)
    JAWT awt;
    JAWT_DrawingSurface* ds;
    JAWT_DrawingSurfaceInfo* dsi;
    JAWT_Win32DrawingSurfaceInfo* dsi_win32;
    jint lock;
    jint result;
  
    BOOL retorno;
    
    awt.version = JAWT_VERSION_1_4;
    result = JAWT_GetAWT(env, &awt);
  
    ds = awt.GetDrawingSurface(env, frame);
    lock = ds->Lock(ds);
    dsi = ds->GetDrawingSurfaceInfo(ds);
    dsi_win32 = (JAWT_Win32DrawingSurfaceInfo*)dsi->platformInfo;
  
    retorno = FlashWindow(dsi_win32->hwnd, TRUE);
    ds->FreeDrawingSurfaceInfo(dsi);
    ds->Unlock(ds);
    awt.FreeDrawingSurface(ds);
}

