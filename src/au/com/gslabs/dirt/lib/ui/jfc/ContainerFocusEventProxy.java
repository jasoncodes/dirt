package au.com.gslabs.dirt.lib.ui.jfc;

import java.awt.*;
import java.awt.event.*;
import java.util.*;
import java.lang.ref.WeakReference;

public class ContainerFocusEventProxy
{
	
	protected final ArrayList<FocusListener> focusListeners = new ArrayList<FocusListener>();
	protected final MyListener listener = new MyListener();
	protected final WeakHashMap<Component,Object> components = new WeakHashMap<Component,Object>();
	protected final WeakReference<Component> container;
	
	public ContainerFocusEventProxy(Container c)
	{
		this.container = new WeakReference<Component>(c);
		if (c instanceof Window)
		{
			((Window)c).addWindowFocusListener(listener);
		}
		else
		{
			addComponent(c);
		}
	}
	
	protected class MyListener implements ContainerListener, FocusListener, WindowFocusListener
	{
		
		public void windowGainedFocus(WindowEvent e)
		{
			raiseFocusEvent(true);
		}
		
		public void windowLostFocus(WindowEvent e)
		{
			raiseFocusEvent(false);
		}
		
		public void focusGained(FocusEvent e)
		{
			onFocusEvent(e);
		}
		
		public void focusLost(FocusEvent e)
		{
			onFocusEvent(e);
		}
		
		public void componentAdded(ContainerEvent e)
		{
			addComponent(e.getChild());
		}
		
		public void componentRemoved(ContainerEvent e)
		{
			removeComponent(e.getChild());
		}
		
	}
	
	protected void addComponent(Component c)
	{
		
		components.put(c, null);
		
		c.addFocusListener(listener);
		if (c instanceof Container)
		{
			Container parent = (Container)c;
			parent.addContainerListener(listener);
			for (Component child : parent.getComponents())
			{
				addComponent(child);
			}
		}
		
	}
	
	protected void removeComponent(Component c)
	{
		
		components.remove(c);
		
		c.removeFocusListener(listener);
		if (c instanceof Container)
		{
			Container parent = (Container)c;
			parent.removeContainerListener(listener);
			for (Component child : parent.getComponents())
			{
				removeComponent(child);
			}
		}
		
	}
	
	protected void onFocusEvent(final FocusEvent e)
	{
		
		final boolean gained = (e.getID() == FocusEvent.FOCUS_GAINED);
		final Component compOld = gained ? e.getOppositeComponent() : (Component)e.getSource();
		final Component compNew = gained ? (Component)e.getSource() : e.getOppositeComponent();
		
		final boolean ownOld = components.containsKey(compOld);
		final boolean ownNew = components.containsKey(compNew);
		
		if (ownOld != ownNew)
		{
			raiseFocusEvent(gained);
		}
		
	}
	
	protected void raiseFocusEvent(boolean gained)
	{
		final int id = gained ? FocusEvent.FOCUS_GAINED : FocusEvent.FOCUS_LOST;
		final FocusEvent containerEvent = new FocusEvent(container.get(), id);
		for (final FocusListener listener : focusListeners)
		{
			if (gained)
			{
				listener.focusGained(containerEvent);
			}
			else
			{
				listener.focusLost(containerEvent);
			}
		}
	}
	
	public void addFocusListener(FocusListener l)
	{
		focusListeners.add(l);
	}
	
	public void removeFocusListener(FocusListener l)
	{
		int idx = focusListeners.indexOf(l);
		focusListeners.remove(idx);
	}
	
}
