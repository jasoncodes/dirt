#include "au_com_gslabs_dirt_lib_ui_jfc_jni_MacOS.h"
#include "au_com_gslabs_dirt_lib_util_jni_MacOS.h"
#include <stdio.h>
#include <Carbon/Carbon.h>
#include <ApplicationServices/ApplicationServices.h>
#include <AddressBook/AddressBook.h>

// src: <http://developer.apple.com/samplecode/MyFirstJNIProject/index.html>

JNIEXPORT jstring JNICALL Java_au_com_gslabs_dirt_lib_util_jni_MacOS_getMyFullName
(JNIEnv *env, jobject obj) {
	ABAddressBookRef book = ABGetSharedAddressBook();
	ABPersonRef me = ABGetMe(book);
	
	
	// Get the first and last name.  For more properties, see "Person specific properties"
	// in ABGlobalsC.h
	CFMutableArrayRef names = CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);
	CFStringRef firstName = ABRecordCopyValue(me, kABFirstNameProperty);
	CFStringRef lastName = ABRecordCopyValue(me, kABLastNameProperty);
	
	if (firstName && CFStringGetLength(firstName) > 0) {
		CFArrayAppendValue(names, (const void *)firstName);
	}
	if (lastName && CFStringGetLength(lastName) > 0) {
		CFArrayAppendValue(names, (const void *)lastName);
	}
	
	// merge the first and last names into one CFString
	// An empty string (size 0) are still safely created by the JNI; no check is necessary
	CFStringRef firstAndLastName = CFStringCreateByCombiningStrings(kCFAllocatorDefault, names, CFSTR(" "));
	CFIndex strLen = CFStringGetLength(firstAndLastName);
	UniChar uniStr[strLen];
	CFRange strRange;
	strRange.location = 0;
	strRange.length = strLen;
	CFStringGetCharacters(firstAndLastName, strRange, uniStr);
	
	// return a jstring from the full name's bytes
	jstring javaName = (*env)->NewString(env, (jchar *)uniStr, (jsize)strLen);
	
	CFRelease(names);
	CFRelease(firstAndLastName);
	
	return javaName;
}

// Thanks to <http://www.mactech.com/articles/mactech/Vol.18/18.06/Jun02CoverStory/index.html>
// for putting me on the right track here. All that was needed was some endian issues fixed.

typedef union
{
	int i;
	char c[4];
} PIXEL;

// Args include the array of pixel RGBA values, and the actual image width and height.
extern void setDockTile( int * imagePixels, int width, 
						 int height ) {
	// How many bytes in each pixel? Java uses 4-byte ints.
	int kNumComponents = 4;
	
	// Input is 0xAARRGGBB where alpha has NOT been pre-multiplied
	// The bytes in the array need to be in big-endian.
	int i;
	for (i = 0; i < width*height; ++i)
	{
		unsigned int x = (unsigned int)imagePixels[i];
		PIXEL p;
		p.c[3] = (x >> 0) & 0xff;
		p.c[2] = (x >> 8) & 0xff;
		p.c[1] = (x >> 16) & 0xff;
		p.c[0] = (x >> 24) & 0xff;
		imagePixels[i] = p.i;
	}
	
	OSStatus   theError;
	
	// Several CoreGraphics variables.
	CGContextRef theContext;
	CGDataProviderRef theProvider;
	CGColorSpaceRef theColorspace;
	CGImageRef theImage;
	
	// How many bytes in each row?
	size_t bytesPerRow = width * kNumComponents;
	
	// Obtain graphics context in which to render.
	theContext = BeginCGContextForApplicationDockTile();
	
	if ( theContext != NULL ) {   
		// Use the pixels passed in as the image source.
		theProvider = CGDataProviderCreateWithData( 
													NULL, imagePixels, ( bytesPerRow * height ), NULL );
		
		theColorspace = CGColorSpaceCreateDeviceRGB();
		
		// Create the image. This is similar to creating a PixMap. 
		// - The width and height were passed as arguments. 
		// - The next two values (8 and 32) are the bits per pixel component and 
		//    total bits per pixel, respectively. 
		// - bytesPerRow was calculated above. 
		// - Use the colorspace ref obtained previously. 
		// - The alpha or transparency data is in the first byte of each pixel. 
		// - Use the data source created a few lines above.
		// - The remaining parameters are typical defaults. Consult the API docs for 
		//    more info.
		theImage = CGImageCreate( width, height, 8, 32, 
								  bytesPerRow, theColorspace, kCGImageAlphaFirst, 
								  theProvider, NULL, 0, kCGRenderingIntentDefault );
		
		CGDataProviderRelease( theProvider );
		CGColorSpaceRelease( theColorspace );
		
		// Set the created image as the tile.
		theError = SetApplicationDockTileImage( theImage );
		
		CGContextFlush( theContext );
		
		CGImageRelease( theImage );
		
		EndCGContextForApplicationDockTile( theContext );
	}
}

JNIEXPORT void JNICALL Java_au_com_gslabs_dirt_lib_ui_jfc_jni_MacOS_setDockTile( 
																				 JNIEnv *env, jobject this, jintArray pixels, jint width, 
																				 jint height ) {
	// Obtain a pointer to the array to pass to the native function.
	jint *theArray = (*env)->GetIntArrayElements( env, pixels, NULL );
	
	if ( theArray != NULL ) {
		// Call the library function.
		// Note that no adjustments are made to the primitive values.
		setDockTile( theArray, width, height );
		
		// Tell the VM we are no longer interested in the array.
		(*env)->ReleaseIntArrayElements( env, pixels, theArray, 0 );
	}
}
