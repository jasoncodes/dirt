/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class au_com_gslabs_dirt_lib_util_jni_Linux */

#ifndef _Included_au_com_gslabs_dirt_lib_util_jni_Linux
#define _Included_au_com_gslabs_dirt_lib_util_jni_Linux
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     au_com_gslabs_dirt_lib_util_jni_Linux
 * Method:    getuid
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_au_com_gslabs_dirt_lib_util_jni_Linux_getuid
  (JNIEnv *, jobject);

/*
 * Class:     au_com_gslabs_dirt_lib_util_jni_Linux
 * Method:    getPwUid_Gecos
 * Signature: (I)[B
 */
JNIEXPORT jbyteArray JNICALL Java_au_com_gslabs_dirt_lib_util_jni_Linux_getPwUid_1Gecos
  (JNIEnv *, jobject, jint);

/*
 * Class:     au_com_gslabs_dirt_lib_util_jni_Linux
 * Method:    getPwUid_Name
 * Signature: (I)[B
 */
JNIEXPORT jbyteArray JNICALL Java_au_com_gslabs_dirt_lib_util_jni_Linux_getPwUid_1Name
  (JNIEnv *, jobject, jint);

#ifdef __cplusplus
}
#endif
#endif