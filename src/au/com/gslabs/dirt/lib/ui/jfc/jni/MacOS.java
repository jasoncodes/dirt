package au.com.gslabs.dirt.lib.ui.jfc.jni;

import java.awt.Frame;
import java.awt.*;
import java.awt.image.*;
import java.util.*;
import javax.swing.ImageIcon;
import java.awt.color.*;

public class MacOS
{

	public MacOS()
	{
	}
	
	public native int requestAttention(boolean critical);
	public native void cancelAttention(int requestID);
	
	private native void setDockTile(int[] pixels, int width, int height, String label);
	
	public void setDockIcon(ImageIcon icon, String label)
	{
		
		Image image = icon.getImage();
		
		int width = icon.getIconWidth();
		int height = icon.getIconHeight();
		
		// Allocate storage for the image data.
		int pixels[] = new int[ width * height ];
		
		// Create an object to copy the image pixel data into our array.
		PixelGrabber pg = new PixelGrabber(image, 0, 0, width, height, pixels, 0, width);
		
		// Copy the pixels to the array.
		try
		{
			pg.grabPixels();
			
			// Check for error using bit values in the ImageObserver class.
			if ( ( pg.getStatus() & ImageObserver.ABORT ) != 0 )
			{
				return;
			}
			
			// pass the pixel array off to the JNI code to set the dock icon
			setDockTile(pixels, width, height, label);
			
		}
		catch ( InterruptedException e )
		{
		}
		
	}
	
}
