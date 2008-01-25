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
					mac.setDockIcon(icons[0]);
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
	
	public void addFrame(Frame frame)
	{
		synchronized (this)
		{
			bounceStopTick = System.currentTimeMillis() + 1000;
			monitors.put(frame, new FocusMonitor(frame));
			notify();
		}
	}
	
	public Frame[] getAlertingFrames()
	{
		return monitors.keySet().toArray(new Frame[0]);
	}
	
	private Integer requestID = null;
	private int currentIcon = 0;
	private long bounceStopTick = 0;
	
	private void reset()
	{
		if (requestID != null)
		{
			cancelAttention(requestID);
			requestID = null;
		}
		bounceStopTick = 0;
		currentIcon = 0;
		mac.setDockIcon(icons[0]);
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
				
				currentIcon = ++currentIcon % 2;
				mac.setDockIcon(icons[currentIcon]);
				
				if (bounceStopTick > System.currentTimeMillis())
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
