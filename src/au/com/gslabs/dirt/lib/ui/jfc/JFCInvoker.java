package au.com.gslabs.dirt.lib.ui.jfc;

import au.com.gslabs.dirt.lib.thread.Invoker;
import javax.swing.SwingUtilities;

public class JFCInvoker implements Invoker
{
	
	public void invoke(Runnable r, boolean synchronous)
	{
		if (synchronous)
		{
			if (SwingUtilities.isEventDispatchThread())
			{
				r.run();
			}
			else
			{
				try
				{
					SwingUtilities.invokeAndWait(r);
				}
				catch (Exception ex)
				{
					throw new RuntimeException(ex);
				}
			}
		}
		else
		{
			SwingUtilities.invokeLater(r);
		}
	}
	
}
