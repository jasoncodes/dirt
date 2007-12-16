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
	
	public class ReopenApplicationHelper implements Runnable
	{
		
		public void run()
		{
			
			if (!UIUtil.isValidWindow(UIUtil.getActiveWindow()))
			{
				
				Frame selected = null;
				for (Frame f : Frame.getFrames())
				{
					if (UIUtil.isValidWindow(f))
					{
						selected = f;
						break;
					}
				}
				
				if (selected == null)
				{
					selected = new MainFrame();
				}
				
				selected.setVisible(true);
				selected.setState(Frame.NORMAL);
				UIUtil.stealFocus(selected);
				
			}
			
		}
		
	}
	
}
