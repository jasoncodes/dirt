package au.com.gslabs.dirt.lib.ui.jfc.jni;

import java.awt.Frame;

public class Win32
{

	public Win32()
	{
	}

	public void alert(final Window window)
	{
		FlashWindow(window);
	}
	
	public void stealFocus(final Window window)
	{
		ForceForegroundWindow(window);
	}

	public void setIcon(window Window frame, final String filename)
	{
		SetIcon(window, filename);
	}

	protected native void FlashWindow(Window window);
	protected native void ForceForegroundWindow(Window window);
	protected native void SetIcon(Window window, String filename);
	public native String getMyFullName();

}
