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
		
		add(mnuFile);
		if (mnuEdit.getItemCount() > 0)
		{
			add(mnuEdit);
		}
		add(mnuHelp);
		
	}
	
	static WeakReference<Window> lastFocusedMenuWindow = null;
	
	protected boolean canCloseActiveWindow()
	{
		
		Window w = UIUtil.getActiveWindow();
		
		if (w != null)
		{
			lastFocusedMenuWindow = new WeakReference<Window>(w);
		}
		else
		{
			w = lastFocusedMenuWindow.get();
		}
		
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
