package au.com.gslabs.dirt.lib.thread;

public interface Invoker
{

	/**
	 * @param synchronous
	 *            If true, the Runnable object <strong><em>must</em></strong> complete before returning.
	 *            Otherwise the Runnable object <em>should</em> be ran asynchronously.
	 */
	public void invoke(Runnable r, boolean synchronous);

}
