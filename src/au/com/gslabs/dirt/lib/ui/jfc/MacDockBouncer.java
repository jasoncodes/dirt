package au.com.gslabs.dirt.lib.ui.jfc;

import java.lang.ref.WeakReference;
import java.lang.reflect.*;
import javax.swing.*;
import au.com.gslabs.dirt.lib.util.FileUtil;
import au.com.gslabs.dirt.lib.ui.jfc.jni.MacOS;
import java.awt.*;
import java.awt.event.*;
import java.util.*;
import net.roydesign.app.Application;

class MacDockBouncer extends Thread
{
	
	private final ImageIcon[] icons;
	private final MacOS mac;
	
	private class FocusMonitor
	{
		final public Frame frame;
		public long bounceStopTick;
		public int countAlert = 0, countFlash = 0;
		public boolean hasBeenFocused;
		final private WindowAdapter listener;
		public FocusMonitor(Frame frame)
		{
			this.frame = frame;
			this.hasBeenFocused = false;
			this.listener = new WindowAdapter()
				{
					public void windowActivated(WindowEvent e)
					{
						done();
					}
					public void windowGainedFocus(WindowEvent e)
					{
						done();
					}
					public void windowClosing(WindowEvent e)
					{
						done();
					}
				};
			this.frame.addWindowListener(listener);
			this.frame.addWindowFocusListener(listener);
		}
		public void done()
		{
			this.frame.removeWindowListener(listener);
			this.frame.removeWindowFocusListener(listener);
			this.hasBeenFocused = true;
			lastFocusedAlertFrame = new WeakReference<Frame>(this.frame);
			MacDockBouncer.this.interrupt();
		}
	}
	
	private Map<Frame,FocusMonitor> monitors;
	private WeakReference<Frame> lastFocusedAlertFrame;
	
	public MacDockBouncer() throws Exception
	{
		
		super("MacDockBouncer");
		setDaemon(true);
		
		monitors = new HashMap<Frame,FocusMonitor>();
		
		UIUtil.loadNativeLibrary();
		mac = new MacOS();
		
		icons = new ImageIcon[2];
		icons[0] = new ImageIcon(FileUtil.getResource("res/icons/dirt.png"));
		icons[1] = new ImageIcon(FileUtil.getResource("res/icons/dirt_highlight.png"));
		
		Runtime.getRuntime().addShutdownHook(new Thread()
			{
				public void run()
				{
					mac.setDockIcon(icons[0], null);
				}
			});
			
		Application.getInstance().addReopenApplicationListener(new ActionListener()
			{
				public void actionPerformed(ActionEvent e)
				{
					if (lastFocusedAlertFrame == null || UIUtil.getActiveWindow() != lastFocusedAlertFrame.get())
					{
						final Frame[] alertingFrames = getAlertingFrames();
						if (alertingFrames.length > 0)
						{
							UIUtil.stealFocus(alertingFrames[0]);
						}
					}
					lastFocusedAlertFrame = null;
					
				}
			});
		
		start();
		
	}
	
	private Integer requestAttention()
	{
		try
		{
			return Integer.valueOf(mac.requestAttention(false));
		}
		catch (Exception ex)
		{
			System.err.println("Error requesting attention:");
			ex.printStackTrace();
			return null;
		}
	}
	
	private void cancelAttention(Integer requestID)
	{
		try
		{
			mac.cancelAttention(requestID);
		}
		catch (Exception ex)
		{
			System.err.println("Error cancelling attention:");
			ex.printStackTrace();
		}
	}
	
	public void addFrame(Frame frame, int bounceCount, int flashCount, boolean enableAlertCount)
	{
		synchronized (this)
		{
			FocusMonitor monitor = monitors.get(frame);
			if (monitor == null)
			{
				monitor = new FocusMonitor(frame);
				monitors.put(frame, monitor);
			}
			monitor.countAlert = enableAlertCount ? (monitor.countAlert + 1) : 0;
			monitor.countFlash = flashCount;
			monitor.bounceStopTick = bounceCount > 0 ? System.currentTimeMillis() + 1000 * bounceCount : bounceCount;
			notify();
		}
	}
	
	public Frame[] getAlertingFrames()
	{
		return monitors.keySet().toArray(new Frame[0]);
	}
	
	public int getAlertCount()
	{
		int count = 0;
		for (FocusMonitor monitor : monitors.values())
		{
			count += monitor.countAlert;
		}
		return count;
	}
	
	private Integer requestID = null;
	private int currentIcon = 0;
	
	private void reset()
	{
		if (requestID != null)
		{
			cancelAttention(requestID);
			requestID = null;
		}
		currentIcon = 0;
		mac.setDockIcon(icons[0], null);
	}
	
	protected void updateIconState()
	{
		
		currentIcon = ++currentIcon % 2;
		
		// transitioning from 0 to 1 means we are going active
		// lets check if we can do that
		if (currentIcon == 1) 
		{
			boolean canActive = false;
			for (FocusMonitor monitor : monitors.values())
			{
				canActive |= (monitor.countFlash != 0);
				if (monitor.countFlash > 0)
				{
					monitor.countFlash--;
				}
			}
			if (!canActive)
			{
				currentIcon = 0;
			}
		}
	}
	
	protected boolean shouldBeBouncing()
	{
		for (FocusMonitor monitor : monitors.values())
		{
			if (monitor.bounceStopTick < 0 || monitor.bounceStopTick > System.currentTimeMillis())
			{
				return true;
			}
		}
		return false;
	}
	
	public void run()
	{
		while (true)
		{
			
			boolean active;
			
			synchronized (this)
			{
				
				// wait for a frame to request an alert if there's none active
				// the loop is to call reset() a few times after gaining focus to make sure the dock icon resets
				final int tryCount = 4;
				final int tryLength = 250;
				for (int i = 0; i < tryCount && monitors.size() < 1; ++i)
				{
					reset();
					try
					{
						if (i < tryCount-1)
						{
							wait(tryLength);
						}
						else
						{
							wait();
						}
					}
					catch (InterruptedException ex)
					{
					}
				}
				
				// remove any frames that have been focused since we last checked
				ArrayList<Frame> keysToRemove = new ArrayList<Frame>();
				for (FocusMonitor monitor : monitors.values())
				{
					if (monitor.hasBeenFocused || UIUtil.getActiveWindow() == monitor.frame)
					{
						monitor.done();
						keysToRemove.add(monitor.frame);
					}
				}
				for (Frame key : keysToRemove)
				{
					monitors.remove(key);
				}
				
				// if there's any frames left, we're still in alert state
				active = monitors.size() > 0;
				
			}
			
			if (active)
			{
				
				updateIconState();
				final int badgeCount = getAlertCount();
				final String badgeStr = badgeCount > 0 ? String.valueOf(badgeCount) : "";
				mac.setDockIcon(icons[currentIcon], badgeStr);
				
				if (shouldBeBouncing())
				{
					if (requestID == null)
					{
						requestID = requestAttention();
					}
				}
				else
				{
					if (requestID != null)
					{
						cancelAttention(requestID);
						requestID = null;
					}
				}
				
				try
				{
					sleep(750);
				}
				catch (InterruptedException ex)
				{
				}
				
			}
			
		}
	}
	
}
