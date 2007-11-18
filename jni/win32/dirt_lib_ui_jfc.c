#define _WIN32_WINNT 0x5000
#include <windows.h>
#include <security.h>
#include <lmaccess.h>

#include <jni.h>
#include <jawt.h>
#include <jawt_md.h>

#include "au_com_gslabs_dirt_lib_ui_jfc_jni_Win32.h"

static HWND GetWindowHWND(JNIEnv *env, jobject window)
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
	ds = awt.GetDrawingSurface(env, window);
	lock = ds->Lock(ds);
	dsi = ds->GetDrawingSurfaceInfo(ds);
	dsi_win32 = (JAWT_Win32DrawingSurfaceInfo*)dsi->platformInfo;
	hWnd = dsi_win32->hwnd;
	ds->FreeDrawingSurfaceInfo(dsi);
	ds->Unlock(ds);
	awt.FreeDrawingSurface(ds);
	return hWnd;
}

JNIEXPORT void JNICALL Java_au_com_gslabs_dirt_lib_ui_jfc_jni_Win32_FlashWindow
  (JNIEnv *env, jobject canvas, jobject window)
{
	FlashWindow(GetWindowHWND(env, window), TRUE);
}

JNIEXPORT void JNICALL Java_au_com_gslabs_dirt_lib_ui_jfc_jni_Win32_ForceForegroundWindow
  (JNIEnv *env, jobject canvas, jobject window)
{

	HWND hWnd = GetWindowHWND(env, window);
	
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

JNIEXPORT void JNICALL Java_au_com_gslabs_dirt_lib_ui_jfc_jni_Win32_SetIcon
  (JNIEnv *env, jobject canvas, jobject window, jstring filename)
{
	int pxBig = GetSystemMetrics(SM_CXICON);
	int pxSmall = GetSystemMetrics(SM_CXSMICON);
	HWND hWnd = GetWindowHWND(env, window);
	const char *path = (*env)->GetStringUTFChars(env, filename, 0);
	HANDLE icoBig = LoadImage(NULL, path, IMAGE_ICON, pxBig, pxBig, LR_LOADFROMFILE);
	HANDLE icoSmall = LoadImage(NULL, path, IMAGE_ICON, pxSmall, pxSmall, LR_LOADFROMFILE);
	(*env)->ReleaseStringUTFChars(env, filename, path);
	SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)icoBig);
	SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)icoSmall);
}

NET_API_STATUS WINAPI (*netapibufferfree)(PVOID);

int loadAPI()
{
	HANDLE h = LoadLibrary("netapi32.dll");
	if (!h)
		return FALSE;
	if (!(netapibufferfree = (void *) GetProcAddress (h, "NetApiBufferFree")))
		return FALSE;
	return TRUE;
}

JNIEXPORT jstring JNICALL Java_au_com_gslabs_dirt_lib_ui_jfc_jni_Win32_getMyFullName
  (JNIEnv *env, jobject obj)
{
	
	short buff[4096];
	DWORD len = sizeof(buff)/sizeof(buff[0]);
	if (GetUserNameExW(NameDisplay, buff, &len))
	{
		return (*env)->NewString(env, buff, len);
	}
	
	len = sizeof(buff)/sizeof(buff[0]);
	if (loadAPI() && GetUserNameW(buff, &len))
	{
		USER_INFO_3 *buffer;
		DWORD rc = NetUserGetInfo(NULL, buff, 3, (void*)&buffer);
		if (rc == ERROR_SUCCESS)
		{
			jstring result = (*env)->NewString(env, buffer->usri3_full_name, lstrlenW(buffer->usri3_full_name));
			netapibufferfree(buffer);
			return result;
		}
	}
	
	return 0;
	
}
