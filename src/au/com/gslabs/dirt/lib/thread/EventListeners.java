package au.com.gslabs.dirt.lib.thread;

import java.util.ArrayList;

public class EventListeners<L extends Object>
{
	
	protected final ArrayList<L> listeners = new ArrayList<L>();
	protected final ArrayList<Invoker> invokers = new ArrayList<Invoker>();
	
	public EventListeners()
	{
	}
	
	public void add(L listener, Invoker invoker)
	{
		listeners.add(listener);
		invokers.add(invoker);
	}
	
	public void remove(L listener)
	{
		int idx = listeners.indexOf(listener);
		listeners.remove(idx);
		invokers.remove(idx);
	}
	
	protected class Dispatcher implements Runnable
	{
		
		protected L listener;
		protected EventSource<L> source;
		
		public Dispatcher(L listener, EventSource<L> source)
		{
			this.listener = listener;
			this.source = source;
		}
		
		public void run()
		{
			source.dispatchEvent(listener);
		}
		
	}
	
	public void dispatchEvent(EventSource<L> source, boolean synchronous)
	{
		for (int idx = listeners.size()-1; idx >= 0; --idx)
		{
			invokers.get(idx).invoke(new Dispatcher(listeners.get(idx), source), synchronous);
		}
	}
	
	public void dispatchEvent(EventSource<L> source)
	{
		dispatchEvent(source, false);
	}
	
}
