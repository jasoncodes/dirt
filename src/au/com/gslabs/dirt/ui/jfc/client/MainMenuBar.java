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
	
	private final Application app;
	
	public MainMenuBar()
	{
		
		this.app = Application.getInstance();
		
		final JScreenMenu mnuFile = new JScreenMenu("File");
		mnuFile.setMnemonic('F');
		final JScreenMenu mnuEdit = new JScreenMenu("Edit");
		mnuFile.setMnemonic('E');
		final JScreenMenu mnuWindow = new JScreenMenu("Window");
		mnuFile.setMnemonic('W');
		final JScreenMenu mnuHelp = new JScreenMenu("Help");
		mnuFile.setMnemonic('H');

		final JScreenMenuItem mnuNew = new JScreenMenuItem("New Window");
		mnuNew.setMnemonic('N');
		mnuNew.setAccelerator(KeyStroke.getKeyStroke(
				KeyEvent.VK_N,
				Toolkit.getDefaultToolkit().getMenuShortcutKeyMask()
			));
		mnuNew.addActionListener(new ActionListener()
			{
				public void actionPerformed(ActionEvent e)
				{
					new MainFrame().setVisible(true);
				}
			});
		mnuFile.add(mnuNew);
		
		final JScreenMenuItem mnuClose = new JScreenMenuItem("Close Window");
		mnuClose.setMnemonic('C');
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
		mnuQuit.setMnemonic('Q');
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
		mnuPreferences.setMnemonic('P');
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
		mnuMinimize.setMnemonic('M');
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
		
		final KeyStroke keyCycleForward = KeyStroke.getKeyStroke(
			KeyEvent.VK_BACK_QUOTE,
			Toolkit.getDefaultToolkit().getMenuShortcutKeyMask());
		final KeyStroke keyCycleBackward = KeyStroke.getKeyStroke(
			KeyEvent.VK_BACK_QUOTE,
			Toolkit.getDefaultToolkit().getMenuShortcutKeyMask() | KeyEvent.SHIFT_MASK);
		
		final JScreenMenuItem mnuCycleWindows = new JScreenMenuItem("Cycle Through Windows");
		mnuCycleWindows.setMnemonic('C');
		mnuCycleWindows.setAccelerator(keyCycleForward);
		mnuCycleWindows.addActionListener(new ActionListener()
			{
				public void actionPerformed(ActionEvent e)
				{
					final boolean backwards = (e.getModifiers() & KeyEvent.SHIFT_MASK) != 0;
					UIUtil.getWindowManager().cycleThroughWindows(backwards);
				}
			});
		mnuWindow.add(mnuCycleWindows);
		
		// change the accelerator when shift is held down
		Toolkit.getDefaultToolkit().addAWTEventListener(new AWTEventListener()
			{
				public void eventDispatched(AWTEvent awtEvent)
				{
					KeyEvent e = (KeyEvent)awtEvent;
					if (e.getKeyCode() == KeyEvent.VK_SHIFT)
					{
						switch (e.getID())
						{
							case KeyEvent.KEY_PRESSED:
								mnuCycleWindows.setAccelerator(keyCycleBackward);
								break;
							case KeyEvent.KEY_RELEASED:
								mnuCycleWindows.setAccelerator(keyCycleForward);
								break;
						}
					}
				}
			}, AWTEvent.KEY_EVENT_MASK);
		
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
					mnuCycleWindows.setEnabled(UIUtil.getWindowManager().getOrderedValidFrames(true).length > 0);
				}
			});
		
		final AboutJMenuItem mnuAbout = app.getAboutJMenuItem();
		mnuAbout.setMnemonic('A');
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
	
	private boolean canCloseActiveWindow()
	{
		return app.getFramelessJMenuBar() != this;
	}
	
	private boolean canMinimizeActiveWindow()
	{
		if (app.getFramelessJMenuBar() != this)
		{
			return (UIUtil.getActiveWindow() instanceof Frame);
		}
		return false;
	}
	
}
