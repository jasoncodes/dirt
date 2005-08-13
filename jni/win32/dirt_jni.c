#include <jni.h>
#include <jawt.h>
#include <jawt_md.h>
#include "au_com_gslabs_dirt_jni_Win32.h"

static HWND GetFrameHWND(JNIEnv *env, jobject frame)
{ 
	JAWT awt;
	JAWT_DrawingSurface* ds;
	JAWT_DrawingSurfaceInfo* dsi;
	JAWT_Win32DrawingSurfaceInfo* dsi_win32;
	jint lock;
	jint result;
	HWND hWnd;
	awt.version = JAWT_VERSION_1_4;
	result = JAWT_GetAWT(env, &awt);
	ds = awt.GetDrawingSurface(env, frame);
	lock = ds->Lock(ds);
	dsi = ds->GetDrawingSurfaceInfo(ds);
	dsi_win32 = (JAWT_Win32DrawingSurfaceInfo*)dsi->platformInfo;
	hWnd = dsi_win32->hwnd;
	ds->FreeDrawingSurfaceInfo(dsi);
	ds->Unlock(ds);
	awt.FreeDrawingSurface(ds);
	return hWnd;
}

JNIEXPORT void JNICALL Java_au_com_gslabs_dirt_jni_Win32_FlashWindow
  (JNIEnv *env, jobject canvas, jobject frame)
{
	FlashWindow(GetFrameHWND(env, frame), TRUE);
}

JNIEXPORT void JNICALL Java_au_com_gslabs_dirt_jni_Win32_ForceForegroundWindow
  (JNIEnv *env, jobject canvas, jobject frame)
{

	HWND hWnd = GetFrameHWND(env, frame);
	
	if (hWnd != GetForegroundWindow())
	{
		
		DWORD ThreadID1 = GetWindowThreadProcessId(GetForegroundWindow(), 0);
		DWORD ThreadID2 = GetWindowThreadProcessId(hWnd, 0);
		int flag = (ThreadID1 != ThreadID2);

		if (flag)
		{
			AttachThreadInput(ThreadID1, ThreadID2, TRUE);
		}
		
		SetForegroundWindow(hWnd);
		SetWindowPos(hWnd, HWND_TOP, 0, 0, 0, 0, SWP_ASYNCWINDOWPOS | SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
		
		if (flag)
		{
			AttachThreadInput(ThreadID1, ThreadID2, FALSE);
		}

		if (IsIconic(hWnd))
		{
			ShowWindow(hWnd, SW_RESTORE);
		}
		else
		{
			ShowWindow(hWnd, SW_SHOW);
		}

		SetWindowPos(hWnd, HWND_TOP, 0, 0, 0, 0, SWP_ASYNCWINDOWPOS | SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);

	}
	
}
