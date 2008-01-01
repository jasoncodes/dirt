package au.com.gslabs.dirt.lib.ui.jfc;

import java.awt.*;
import java.awt.event.*;
import java.util.*;
import java.lang.ref.WeakReference;
import au.com.gslabs.dirt.lib.util.FileUtil;

public class WindowManager
{
	
	protected final class FrameData
	{
		
		public final WeakReference<Frame> frame;
		public long focusPriority;
		
		public FrameData(Frame frame)
		{
			this.frame = new WeakReference<Frame>(frame);
		}
		
	}
	
	protected long lastFocusPriority = 0;
	protected WeakHashMap<Frame,FrameData> frameDatas = new WeakHashMap<Frame,FrameData>();
	
	WindowManager(final Toolkit toolkit)
	{
		
		// add our AWT window change event hook
		toolkit.addAWTEventListener(new AWTEventListener()
			{
				public void eventDispatched(AWTEvent e)
				{
					switch (e.getID())
					{
						case WindowEvent.WINDOW_CLOSED:
							onWindowClosed();
							break;
						case WindowEvent.WINDOW_GAINED_FOCUS:
							onWindowFocused((Window)e.getSource());
							break;
					}
				}
			}, AWTEvent.WINDOW_EVENT_MASK);
		
		// initialise state with a best guess
		for (Frame f : Frame.getFrames())
		{
			updateWindowLastFocused(f);
		}
		updateWindowLastFocused(UIUtil.getActiveWindow());
		
	}
	
	protected FrameData getFrameData(final Frame f)
	{
		FrameData data = frameDatas.get(f);
		if (data == null)
		{
			data = new FrameData(f);
			frameDatas.put(f, data);
		}
		return data;
	}
	
	public void updateWindowLastFocused(final Window w)
	{
		if (UIUtil.isValidWindow(w) && w instanceof Frame)
		{
			final Frame f = (Frame)w;
			final FrameData data = getFrameData(f);
			data.focusPriority = ++lastFocusPriority;
		}
	}
	
	protected void onWindowClosed()
	{
		// on platforms other than Mac, processes don't stay open without windows
		if (!FileUtil.isMac() && UIUtil.getFirstValidFrame() == null)
		{
			System.exit(0);
		}
	}
	
	protected void onWindowFocused(final Window w)
	{
		updateWindowLastFocused(w);
	}
	
	public Frame[] getOrderedValidFrames(final boolean mostRecentFirst)
	{
		final long sortOrder = mostRecentFirst ? -1 : 1;
		final SortedMap<Long,Frame> list = new TreeMap<Long,Frame>();
		for (final FrameData data : frameDatas.values())
		{
			final Frame f = data.frame.get();
			if (UIUtil.isValidWindow(f))
			{
				list.put(sortOrder*data.focusPriority, f);
			}
		}
		return list.values().toArray(new Frame[0]);
	}
	
	public void cycleThroughWindows(boolean backwards)
	{
		final Window currentFocus = UIUtil.getActiveWindow();
		boolean haveFocused = false;
		for (Frame f : getOrderedValidFrames(!backwards))
		{
			updateWindowLastFocused(f);
			if (currentFocus != f && !haveFocused)
			{
				UIUtil.stealFocus(f);
				haveFocused = true;
			}
		}
	}
	
}