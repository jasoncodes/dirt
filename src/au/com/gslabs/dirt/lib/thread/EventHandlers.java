package au.com.gslabs.dirt.lib.thread;

import java.util.ArrayList;

public class EventHandlers<L extends Object>
{
	
	protected final ArrayList<L> listeners = new ArrayList<L>();
	protected final Invoker invoker;
	
	public EventHandlers(Invoker invoker)
	{
		this.invoker = invoker;
	}
	
	public void add(L listener)
	{
		listeners.add(listener);
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
	
	public void dispatchEvent(EventSource<L> source, boolean synchronous)
	{
		Dispatcher d = new Dispatcher();
		d.source = source;
		invoker.invoke(d, synchronous);
	}
	
	public void dispatchEvent(EventSource<L> source)
	{
		dispatchEvent(source, false);
	}
	
}
