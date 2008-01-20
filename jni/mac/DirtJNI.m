#import <Cocoa/Cocoa.h>
#include "au_com_gslabs_dirt_lib_ui_jfc_jni_MacOS.h"
#include <jawt_md.h>

// src: http://developer.apple.com/samplecode/JSheets/
// Given a Java component, return an NSWindow*
NSWindow * GetWindowFromComponent(jobject parent, JNIEnv *env) {
	JAWT awt;
	JAWT_DrawingSurface* ds;
	JAWT_DrawingSurfaceInfo* dsi;
	JAWT_MacOSXDrawingSurfaceInfo* dsi_mac;
	jboolean result;
	jint lock;
	
	// Get the AWT
	awt.version = JAWT_VERSION_1_4;
	result = JAWT_GetAWT(env, &awt);
	assert(result != JNI_FALSE);
	
	// Get the drawing surface
	ds = awt.GetDrawingSurface(env, parent);
	assert(ds != NULL);
	
	// Lock the drawing surface
	lock = ds->Lock(ds);
	assert((lock & JAWT_LOCK_ERROR) == 0);
	
	// Get the drawing surface info
	dsi = ds->GetDrawingSurfaceInfo(ds);
	
	// Get the platform-specific drawing info
	dsi_mac = (JAWT_MacOSXDrawingSurfaceInfo*)dsi->platformInfo;
	
	// Get the NSView corresponding to the component that was passed
	NSView *view = dsi_mac->cocoaViewRef;
	
	// Free the drawing surface info
	ds->FreeDrawingSurfaceInfo(dsi);
	// Unlock the drawing surface
	ds->Unlock(ds);
	
	// Free the drawing surface
	awt.FreeDrawingSurface(ds);
	
	// Get the view's parent window; this is what we need to show a sheet
	return [view window];
}
