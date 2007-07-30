package au.com.gslabs.dirt.lib.ui.jfc.jni;

import java.awt.Frame;

public class Win32
{

	public Win32()
	{
	}

	public void alert(final Frame frame)
	{
		FlashWindow(frame);
	}
	
	public void stealFocus(final Frame frame)
	{
		ForceForegroundWindow(frame);
	}

	public void setIcon(final Frame frame, final String filename)
	{
		SetIcon(frame, filename);
	}

	protected native void FlashWindow(Frame frame);
	protected native void ForceForegroundWindow(Frame frame);
	protected native void SetIcon(Frame frame, String filename);
	public native String getMyFullName();

}
