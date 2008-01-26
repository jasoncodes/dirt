package au.com.gslabs.dirt.ui.jfc.client;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import au.com.gslabs.dirt.lib.ui.jfc.UIUtil;
import au.com.gslabs.dirt.lib.util.FileUtil;
import javax.swing.SwingUtilities;
import net.roydesign.app.Application;
import java.awt.Window;
import java.awt.Frame;
import javax.swing.JFrame;
import java.util.ResourceBundle;

public class ClientJFC extends Application
{
	
	public static void init() throws Throwable
	{
		SwingUtilities.invokeAndWait(new Runnable()
			{
				public void run()
				{
					UIUtil.initSwing(getAppName());
					new ClientJFC();
				}
			});
	}
	
	private ClientJFC()
	{
		
		setName(getAppName());
		setFramelessJMenuBar(new MainMenuBar());
		
		addOpenApplicationListener(new ActionListener()
			{
				public void actionPerformed(ActionEvent e)
				{
					SwingUtilities.invokeLater(new ReopenApplicationHelper());
				}
			});
		addReopenApplicationListener(new ActionListener()
			{
				public void actionPerformed(ActionEvent e)
				{
					SwingUtilities.invokeLater(new ReopenApplicationHelper());
				}
			});
		
		if (!FileUtil.isMac())
		{
			new MainFrame().setVisible(true);
		}
		
	}
	
	public static final String getAppName()
	{
		return ResourceBundle.getBundle("res/strings").getString("name");
	}
	
	public static void showPreferences()
	{
		for (Frame f : UIUtil.getWindowManager().getOrderedValidFrames(true))
		{
			if (f instanceof PreferencesFrame)
			{
				UIUtil.stealFocus(f);
				return;
			}
		}
		new PreferencesFrame().setVisible(true);
	}
	
	public class ReopenApplicationHelper implements Runnable
	{
		
		public void run()
		{
			
			if (!UIUtil.isValidWindow(UIUtil.getActiveWindow()))
			{
				
				Window selected = UIUtil.getFirstValidFrame();
				
				if (selected == null)
				{
					selected = new MainFrame();
				}
				
				selected.setVisible(true);
				if (selected instanceof Frame)
				{
					((Frame)selected).setState(Frame.NORMAL);
				}
				UIUtil.stealFocus(selected);
				
			}
			
		}
		
	}
	
}
