package au.com.gslabs.dirt.ui.jfc.client;

import au.com.gslabs.dirt.lib.ui.jfc.*;
import javax.swing.*;
import javax.swing.event.*;
import java.awt.*;
import java.awt.event.*;
import net.roydesign.app.*;
import net.roydesign.ui.*;

public class MainMenuBar extends JScreenMenuBar
{
	
	protected Application app;
	
	protected PreferencesJMenuItem preferences;
	
	public MainMenuBar()
	{
		
		this.app = Application.getInstance();
		
		final JScreenMenu mnuFile = new JScreenMenu("File");
		add(mnuFile);

		final JScreenMenuItem mnuFileNew = new JScreenMenuItem("New Window");
		mnuFileNew.addActionListener(new ActionListener()
			{
				public void actionPerformed(ActionEvent e)
				{
					new MainFrame().setVisible(true);
				}
			});
		mnuFileNew.setAccelerator(KeyStroke.getKeyStroke(
				KeyEvent.VK_N,
				Toolkit.getDefaultToolkit().getMenuShortcutKeyMask()
			));
		mnuFile.add(mnuFileNew);
		
		final JScreenMenuItem mnuFileClose = new JScreenMenuItem("Close Window");
		mnuFileClose.setAccelerator(KeyStroke.getKeyStroke(
				KeyEvent.VK_W,
				Toolkit.getDefaultToolkit().getMenuShortcutKeyMask()
			));
		mnuFileClose.addActionListener(new ActionListener()
			{
				public void actionPerformed(ActionEvent e)
				{
					Window w = UIUtil.getActiveWindow();
					if (canCloseWindow(w))
					{
						Toolkit tk = Toolkit.getDefaultToolkit();
						EventQueue q = tk.getSystemEventQueue();
						q.postEvent(new WindowEvent(w, WindowEvent.WINDOW_CLOSING));
					}
				}
			});
		mnuFile.add(mnuFileClose);
		
		mnuFile.addMenuListener(new MenuListener()
			{
				public void menuCanceled(MenuEvent e)
				{
				}
				public void menuDeselected(MenuEvent e)
				{
				}
				public void menuSelected(MenuEvent e)
				{
					Window w = UIUtil.getActiveWindow();
					mnuFileClose.setEnabled(canCloseWindow(UIUtil.getActiveWindow()));
				}
			});
		
		final QuitJMenuItem mnuFileQuit = app.getQuitJMenuItem();
		mnuFileQuit.addActionListener(new ActionListener()
			{
				public void actionPerformed(ActionEvent e)
				{
					System.exit(0);
				}
			});
		if (!mnuFileQuit.isAutomaticallyPresent())
		{
			mnuFile.add(mnuFileQuit);
		}
		
		final JScreenMenu mnuHelp = new JScreenMenu("Help");
		add(mnuHelp);
		
		final AboutJMenuItem about = app.getAboutJMenuItem();
		about.addActionListener(new ActionListener()
			{
				public void actionPerformed(ActionEvent e)
				{
					new AboutFrame().setVisible(true);
				}
			});
		if (!about.isAutomaticallyPresent())
		{
			mnuHelp.add(about);
		}
		
	}
	
	protected boolean canCloseWindow(Window w)
	{
		if (w == null)
		{
			return false;
		}
		if (w instanceof JFrame)
		{
			JFrame f = (JFrame)w;
			if (app.getFramelessJMenuBar() == f.getJMenuBar())
			{
				return false;
			}
		}
		return true;
	}
	
}
