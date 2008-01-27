#define _WIN32_WINNT 0x5000
#define _WIN32_IE 0x0500

#include <windows.h>
#include <security.h>
#include <lmaccess.h>
#include <shlobj.h>

#include <jni.h>

#include "au_com_gslabs_dirt_lib_util_jni_Win32.h"

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

JNIEXPORT jstring JNICALL Java_au_com_gslabs_dirt_lib_util_jni_Win32_getMyFullName
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

JNIEXPORT jstring JNICALL Java_au_com_gslabs_dirt_lib_util_jni_Win32_getAppDataPath
  (JNIEnv *env, jobject obj)
{
	short path[MAX_PATH];
	if (SHGetFolderPathW(NULL, CSIDL_APPDATA|CSIDL_FLAG_CREATE, NULL, SHGFP_TYPE_CURRENT, path) == S_OK)
	{
		return (*env)->NewString(env, path, lstrlenW(path));
	}
	else
	{
		return NULL;
	}
}
