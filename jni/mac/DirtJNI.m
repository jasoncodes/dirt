#import <Cocoa/Cocoa.h>
#include "au_com_gslabs_dirt_lib_ui_jfc_jni_MacOS.h"
#include <jawt_md.h>
#include "DirtJNI.h"

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

#if defined(__USE_LEOPARD_API)

void Dock_setContentImage(CGImageRef imageRef)
{
	
	NSAutoreleasePool *autoreleasepool = [[NSAutoreleasePool alloc] init];

	NSRect rect;
	rect.origin.x = 0.0f;
	rect.origin.y = 0.0f;
	rect.size.width = CGImageGetWidth(imageRef);
	rect.size.height = CGImageGetHeight(imageRef);
	
	NSImage* image = [[NSImage alloc] initWithSize:rect.size];
    [image lockFocus];
    CGContextDrawImage([[NSGraphicsContext currentContext] graphicsPort], *(CGRect*)&rect, imageRef);
    [image unlockFocus];
	
	NSImageView *dockImageView = [[NSImageView alloc] initWithFrame: rect];
	[dockImageView setImage: image];
	
	NSDockTile *dockTile = [NSApp dockTile];
	[dockTile setContentView: dockImageView];
	[dockTile display];
	
	[image release];
	
	[autoreleasepool release];
	
}

#endif

JNIEXPORT jint JNICALL Java_au_com_gslabs_dirt_lib_ui_jfc_jni_MacOS_requestAttention(JNIEnv *env, jobject this, jboolean critical)
{
	NSAutoreleasePool *autoreleasepool = [[NSAutoreleasePool alloc] init];
	int requestID = [NSApp requestUserAttention: critical?NSCriticalRequest:NSInformationalRequest];
	[autoreleasepool release];
	return requestID;
}

JNIEXPORT void JNICALL Java_au_com_gslabs_dirt_lib_ui_jfc_jni_MacOS_cancelAttention(JNIEnv *env, jobject this, jint requestID)
{
	NSAutoreleasePool *autoreleasepool = [[NSAutoreleasePool alloc] init];
	[NSApp cancelUserAttentionRequest: requestID];
	[autoreleasepool release];
}
