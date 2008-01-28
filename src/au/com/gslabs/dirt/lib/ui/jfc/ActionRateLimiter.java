package au.com.gslabs.dirt.lib.ui.jfc;

import javax.swing.Timer;
import java.awt.event.ActionListener;
import java.awt.event.ActionEvent;

public class ActionRateLimiter
{
	
	private final Timer timer;
	private final ActionListener listener;
	
	public ActionRateLimiter(ActionListener listener, int delay)
	{
		this.listener = listener;
		timer = new Timer(delay, new ActionHandler());
		timer.setRepeats(false);
		timer.setCoalesce(true);
	}
	
	public void trigger()
	{
		if (!timer.isRunning())
		{
			timer.restart();
		}
	}
	
	private void raiseEvent()
	{
		if (listener != null)
		{
			listener.actionPerformed(new ActionEvent(this, 0, null));
		}
	}
	
	private class ActionHandler implements ActionListener
	{
		public void actionPerformed(ActionEvent e)
		{
			raiseEvent();
		}
	}
	
}
