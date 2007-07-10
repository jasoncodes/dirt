package au.com.gslabs.dirt.lib.thread;

import java.util.ArrayList;

public class EventHandlers<L extends Object>
{
	
	protected ArrayList<L> listeners;
	protected Invoker invoker;
	
	public EventHandlers(Invoker invoker)
	{
		this.dispatcher = dispatcher;
	}
	
	public void add(L listener)
	{
		listeners.add(listeners);
	}
	
	public void remove(L listener)
	{
		listeners.remove(listeners.indexOf(listener));
	}
	
	protected class Dispatcher implements Runnable
	{
		public EventSource<L> source;
		public void run()
		{
			for (L listener : listeners)
			{
				source.dispatchEvent(listener);
			}
		}
	}
	
	public void dispatchEvent(EventSource<L> source)
	{
		Dispatcher d = new Dispatcher();
		d.source = source;
		invoker.invoke(d);
	}
	
}
