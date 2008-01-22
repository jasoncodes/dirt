package au.com.gslabs.dirt.lib.ui.jfc;

import java.awt.*;
import java.awt.event.*;
import java.util.*;

public class TopLevelWindowEventProxy
{
	
	private Window currentTopLevel = null;
	private ArrayList<WindowListener> windowListeners = new ArrayList<WindowListener>();
	
	public TopLevelWindowEventProxy(final Component c)
	{
		
		c.addHierarchyListener(new HierarchyListener()
			{
				public void hierarchyChanged(final HierarchyEvent e)
				{
					final Window newTopLevel = UIUtil.getTopLevelWindow(c);
					if (currentTopLevel != newTopLevel)
					{
						removeListeners(currentTopLevel);
						addListeners(newTopLevel);
						currentTopLevel = newTopLevel;
					}
				}
			});
		
	}
	
	private void removeListeners(Window w)
	{
		if (w != null)
		{
			for (WindowListener l : windowListeners)
			{
				w.removeWindowListener(l);
			}
		}
	}
	
	private void addListeners(Window w)
	{
		if (w != null)
		{
			for (WindowListener l : windowListeners)
			{
				w.addWindowListener(l);
			}
		}
	}
	
	public void addWindowListener(WindowListener l)
	{
		
		windowListeners.add(l);
		
		if (currentTopLevel != null)
		{
			currentTopLevel.addWindowListener(l);
		}
		
	}
	
	public void removeWindowListener(WindowListener l)
	{
		
		int idx = windowListeners.indexOf(l);
		windowListeners.remove(idx);
		
		if (currentTopLevel != null)
		{
			currentTopLevel.removeWindowListener(l);
		}
		
	}
	
}
