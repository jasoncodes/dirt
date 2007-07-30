#include <jni.h>
#include "au_com_gslabs_dirt_lib_util_jni_Linux.h"

#include <string.h>
#include <sys/types.h>
#include <pwd.h>

JNIEXPORT jint JNICALL Java_au_com_gslabs_dirt_lib_util_jni_Linux_getuid(JNIEnv *env, jobject obj)
{
	return getuid();
}

jbyteArray doGetPwUid(JNIEnv *env, jint uid, int mode)
{
	struct passwd *who = getpwuid(uid);
	if (who)
	{
		char *data = mode ? who->pw_gecos : who->pw_name;
		jbyteArray jb = (*env)->NewByteArray(env, strlen(data));
		(*env)->SetByteArrayRegion(env, jb, 0, strlen(data), (jbyte*)data);
		return jb;
	}
	else
	{
		return NULL;
	}
}

JNIEXPORT jbyteArray JNICALL Java_au_com_gslabs_dirt_lib_util_jni_Linux_getPwUid_1Gecos(JNIEnv *env, jobject obj, jint uid)
{
	return doGetPwUid(env, uid, 1);
}

JNIEXPORT jbyteArray JNICALL Java_au_com_gslabs_dirt_lib_util_jni_Linux_getPwUid_1Name(JNIEnv *env, jobject obj, jint uid)
{
	return doGetPwUid(env, uid, 0);
}
