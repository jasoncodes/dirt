package au.com.gslabs.dirt.lib.ui.jfc.jni;

import java.awt.Frame;

public class Linux
{

	public Linux()
	{
	}

	public void alert(final Frame frame)
	{
		setDemandsAttention(frame);
	}

	private native void setDemandsAttention(Frame frame);

}
