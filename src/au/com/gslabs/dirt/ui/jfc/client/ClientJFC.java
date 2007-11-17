package au.com.gslabs.dirt.ui.jfc.client;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import au.com.gslabs.dirt.lib.ui.jfc.UIUtil;
import au.com.gslabs.dirt.lib.util.FileUtil;
import javax.swing.SwingUtilities;
import net.roydesign.app.Application;
import java.awt.Window;
import javax.swing.JFrame;

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
		
		addOpenApplicationListener(new ActionListener()
			{
				public void actionPerformed(ActionEvent e)
				{
					new MainFrame().setVisible(true);
				}
			});
		addReopenApplicationListener(new ActionListener()
			{
				public void actionPerformed(ActionEvent e)
				{
					Window w = UIUtil.getActiveWindow();
					if (w == null || (w instanceof JFrame) && getFramelessJMenuBar() == ((JFrame)w).getJMenuBar())
					{
						new MainFrame().setVisible(true);
					}
				}
			});
		
		if (!FileUtil.isMac())
		{
			new MainFrame().setVisible(true);
		}
		
	}
	
}
