package au.com.gslabs.dirt.lib.thread;

public interface EventSource<L extends Object>
{
	public void dispatchEvent(L listener);
}
