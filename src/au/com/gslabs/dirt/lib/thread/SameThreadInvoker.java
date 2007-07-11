package au.com.gslabs.dirt.lib.thread;

import au.com.gslabs.dirt.lib.thread.Invoker;

public class SameThreadInvoker implements Invoker
{
	
	public void invoke(Runnable r, boolean synchronous)
	{
		// this implementation always runs synchronously
		r.run();
	}
	
}
