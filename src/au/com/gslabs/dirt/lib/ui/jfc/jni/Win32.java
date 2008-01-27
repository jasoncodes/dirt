package au.com.gslabs.dirt.lib.ui.jfc.jni;

import java.awt.Window;

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

	public void setIcon(final Window window, final String filename)
	{
		SetIcon(window, filename);
	}
	
	public void setWindowAlpha(final Window window, final double alpha)
	{
		SetWindowAlpha(window, alpha);
	}

	private native void FlashWindow(Window window);
	private native void ForceForegroundWindow(Window window);
	private native void SetIcon(Window window, String filename);
	private native void SetWindowAlpha(Window window, double alpha)

}
