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

void Dock_setContentImage(CGImageRef imageRef, CFStringRef label_ref)
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
	
	NSString *label = (NSString*)label_ref; // toll free bridging
	    
#if defined(__USE_LEOPARD_API)
	
	NSImageView *dockImageView = [[NSImageView alloc] initWithFrame: rect];
	[dockImageView setImage: image];
	
	NSDockTile *dockTile = [NSApp dockTile];
	[dockTile setContentView: dockImageView];
	[dockTile setBadgeLabel: label];
	[dockTile display];
	
	[dockImageView release];
	
#else
	
	if (!label || ![label length])
	{
		[NSApp setApplicationIconImage: image];
	}
	else
	{
			
		NSImage *iconImageBuffer = [image copy];
		
		NSDictionary *attributes = [[NSDictionary alloc] initWithObjectsAndKeys:
									[NSFont fontWithName:@"Helvetica-Bold" size:26], NSFontAttributeName,
									[NSColor whiteColor], NSForegroundColorAttributeName, nil];
		
		NSSize numSize = [label sizeWithAttributes:attributes];
		NSSize iconSize = [iconImageBuffer size];
		
		[iconImageBuffer lockFocus];
		[iconImageBuffer drawAtPoint:NSMakePoint(0, 0)
						 fromRect:NSMakeRect(0, 0, iconSize.width, iconSize.height) 
						 operation:NSCompositeSourceOver 
						 fraction:1.0f];
		float width = (numSize.width > numSize.height) ? numSize.width : numSize.height;
		float height = numSize.height;
		width += 16;
		height += 16;
		NSRect circleRect = NSMakeRect(iconSize.width - width, iconSize.height - height, width, height);
		NSBezierPath *bp = [NSBezierPath bezierPathWithOvalInRect:circleRect];
		[[NSColor colorWithCalibratedRed:0.8f green:0.0f blue:0.0f alpha:1.0f] set];
		[bp fill];
		[label drawAtPoint:NSMakePoint(NSMidX(circleRect) - numSize.width / 2.0f, 
										   NSMidY(circleRect) - numSize.height / 2.0f + 2.0f) 
										   withAttributes:attributes];
		
		[iconImageBuffer unlockFocus];
		
		[NSApp setApplicationIconImage: iconImageBuffer];
		
		[iconImageBuffer release];
		[attributes release];
	
	}
	
#endif
	
	[image release];
	
	[autoreleasepool release];
	
}

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
