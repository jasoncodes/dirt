package au.com.gslabs.dirt.ui.jfc.client;

import au.com.gslabs.dirt.lib.util.FileUtil;
import au.com.gslabs.dirt.lib.ui.jfc.*;
import javax.swing.*;
import javax.swing.event.*;
import java.awt.*;
import java.awt.event.*;
import net.roydesign.app.*;
import net.roydesign.ui.*;
import java.lang.ref.WeakReference;

public class MainMenuBar extends JScreenMenuBar
{
	
	protected Application app;
	
	protected PreferencesJMenuItem preferences;
	
	public MainMenuBar()
	{
		
		this.app = Application.getInstance();
		
		final JScreenMenu mnuFile = new JScreenMenu("File");
		final JScreenMenu mnuEdit = new JScreenMenu("Edit");
		final JScreenMenu mnuWindow = new JScreenMenu("Window");
		final JScreenMenu mnuHelp = new JScreenMenu("Help");

		final JScreenMenuItem mnuNew = new JScreenMenuItem("New Window");
		mnuNew.addActionListener(new ActionListener()
			{
				public void actionPerformed(ActionEvent e)
				{
					new MainFrame().setVisible(true);
				}
			});
		mnuNew.setAccelerator(KeyStroke.getKeyStroke(
				KeyEvent.VK_N,
				Toolkit.getDefaultToolkit().getMenuShortcutKeyMask()
			));
		mnuFile.add(mnuNew);
		
		final JScreenMenuItem mnuClose = new JScreenMenuItem("Close Window");
		mnuClose.setAccelerator(KeyStroke.getKeyStroke(
				KeyEvent.VK_W,
				Toolkit.getDefaultToolkit().getMenuShortcutKeyMask()
			));
		mnuClose.addActionListener(new ActionListener()
			{
				public void actionPerformed(ActionEvent e)
				{
					if (canCloseActiveWindow())
					{
						Toolkit tk = Toolkit.getDefaultToolkit();
						EventQueue q = tk.getSystemEventQueue();
						q.postEvent(new WindowEvent(UIUtil.getActiveWindow(), WindowEvent.WINDOW_CLOSING));
					}
				}
			});
		mnuFile.add(mnuClose);
		
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
					mnuClose.setEnabled(canCloseActiveWindow());
				}
			});
		
		final QuitJMenuItem mnuQuit = app.getQuitJMenuItem();
		mnuQuit.addActionListener(new ActionListener()
			{
				public void actionPerformed(ActionEvent e)
				{
					System.exit(0);
				}
			});
		if (!mnuQuit.isAutomaticallyPresent())
		{
			mnuFile.add(mnuQuit);
		}
		
		final PreferencesJMenuItem mnuPreferences = app.getPreferencesJMenuItem();
		mnuPreferences.addActionListener(new ActionListener()
			{
				public void actionPerformed(ActionEvent e)
				{
					new PreferencesFrame().setVisible(true);
				}
			});
		if (!mnuPreferences.isAutomaticallyPresent())
		{
			mnuEdit.add(mnuPreferences);
		}
		
		final JScreenMenuItem mnuMinimize = new JScreenMenuItem("Minimize");
		mnuMinimize.setAccelerator(KeyStroke.getKeyStroke(
				KeyEvent.VK_M,
				Toolkit.getDefaultToolkit().getMenuShortcutKeyMask()
			));
		mnuMinimize.addActionListener(new ActionListener()
			{
				public void actionPerformed(ActionEvent e)
				{
					if (canMinimizeActiveWindow())
					{
						Window w = UIUtil.getActiveWindow();
						if (w instanceof Frame)
						{
							((Frame)w).setState(Frame.ICONIFIED);
						}
					}
				}
			});
		if (FileUtil.isMac())
		{
			mnuWindow.add(mnuMinimize);
		}
		
		mnuWindow.addMenuListener(new MenuListener()
			{
				public void menuCanceled(MenuEvent e)
				{
				}
				public void menuDeselected(MenuEvent e)
				{
				}
				public void menuSelected(MenuEvent e)
				{
					mnuMinimize.setEnabled(canMinimizeActiveWindow());
				}
			});
		
		final AboutJMenuItem mnuAbout = app.getAboutJMenuItem();
		mnuAbout.addActionListener(new ActionListener()
			{
				public void actionPerformed(ActionEvent e)
				{
					new AboutFrame().setVisible(true);
				}
			});
		if (!mnuAbout.isAutomaticallyPresent())
		{
			mnuHelp.add(mnuAbout);
		}
		
		add(mnuFile);
		if (mnuEdit.getItemCount() > 0)
		{
			add(mnuEdit);
		}
		if (mnuWindow.getItemCount() > 0)
		{
			add(mnuWindow);
		}
		add(mnuHelp);
		
	}
	
	protected boolean canCloseActiveWindow()
	{
		return app.getFramelessJMenuBar() != this;
	}
	
	protected boolean canMinimizeActiveWindow()
	{
		if (app.getFramelessJMenuBar() != this)
		{
			return (UIUtil.getActiveWindow() instanceof Frame);
		}
		return false;
	}
	
}
