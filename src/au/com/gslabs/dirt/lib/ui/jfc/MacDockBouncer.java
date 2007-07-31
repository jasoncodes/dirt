package au.com.gslabs.dirt.lib.ui.jfc;

import java.lang.reflect.*;
import javax.swing.*;
import au.com.gslabs.dirt.lib.util.FileUtil;
import au.com.gslabs.dirt.lib.ui.jfc.jni.MacOS;
import java.awt.*;
import java.awt.event.*;
import java.util.*;

class MacDockBouncer extends Thread
{
	
	private static final String JNILib_MacOS = "lib/mac/libDirtJNI.jnilib";
	
	protected ImageIcon[] icons;
	protected MacOS mac;
	
	final Object app;
	final Method request_attention;
	final Object[] request_params;
	final Method cancel_attention;
	
	protected class FocusMonitor
	{
		final public Frame frame;
		public boolean hasBeenFocused;
		final protected WindowAdapter listener;
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
					public void windowClosed(WindowEvent e)
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
			MacDockBouncer.this.interrupt();
		}
	}
	
	protected Map<Frame,FocusMonitor> monitors;
	
	public MacDockBouncer() throws Exception
	{
		
		super("MacDockBouncer");
		setDaemon(true);
		
		monitors = new HashMap<Frame,FocusMonitor>();
		
		FileUtil.loadLibrary(JNILib_MacOS);
		mac = new MacOS();
		
		icons = new ImageIcon[2];
		icons[0] = new ImageIcon(FileUtil.getResource("res/icons/dirt.png"));
		icons[1] = new ImageIcon(FileUtil.getResource("res/icons/dirt_highlight.png"));
		
		final Class c = Class.forName("com.apple.cocoa.application.NSApplication");
		final Method shared_app = c.getMethod("sharedApplication", (Class[])null);
		this.app = shared_app.invoke(null, (Object[])null);
		final Class[] int_param = new Class[] { Integer.TYPE };
		this.request_attention = c.getMethod("requestUserAttention", int_param);
		final Field f = c.getField("UserAttentionRequestInformational");
		this.request_params = new Object[] { f.getInt(null) };
		this.cancel_attention = c.getMethod("cancelUserAttentionRequest", int_param);
		
		Runtime.getRuntime().addShutdownHook(new Thread()
			{
				public void run()
				{
					mac.setDockIcon(icons[0]);
				}
			});
		
		start();
		
	}
	
	protected Integer requestAttention()
	{
		try
		{
			return (Integer)request_attention.invoke(app, request_params);
		}
		catch (Exception ex)
		{
			return null;
		}
	}
	
	protected void cancelAttention(Integer requestID)
	{
		try
		{
			Object[] cancel_params = new Object[] { requestID };
			cancel_attention.invoke(app, cancel_params);
		}
		catch (Exception ex)
		{
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
	
	protected Integer requestID = null;
	protected int currentIcon = 0;
	protected long bounceStopTick = 0;
	
	protected void reset()
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
