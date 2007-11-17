package au.com.gslabs.dirt.ui.jfc.client;

import au.com.gslabs.dirt.lib.ui.jfc.UIUtil;
import javax.swing.SwingUtilities;
import net.roydesign.app.Application;

public class ClientJFC extends Application
{
	
	public static void init() throws Throwable
	{
		SwingUtilities.invokeAndWait(new Runnable()
			{
				public void run()
				{
					UIUtil.initSwing();
					new ClientJFC();
				}
			});
	}
	
	private ClientJFC()
	{
		
		setName("Dirt");
		setFramelessJMenuBar(new MainMenuBar());
		
		new MainFrame().setVisible(true);
		
	}
	
}
