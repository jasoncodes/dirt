package au.com.gslabs.dirt.ui.jfc.client;

import java.util.ResourceBundle;
import java.awt.event.*;
import javax.swing.*;
import org.jdesktop.jdic.tray.*;
import au.com.gslabs.dirt.lib.util.FileUtil;
import au.com.gslabs.dirt.lib.ui.jfc.*;
import au.com.gslabs.dirt.core.client.*;
import au.com.gslabs.dirt.core.client.console.*;

public class MainFrame extends JFrame
{
	
	protected final ResourceBundle resbundle = ResourceBundle.getBundle("res/strings");
	protected final MainPanel panel;
	protected boolean isDND;
	
	MainFrame()
	{
		
		super("");
		setDefaultCloseOperation(WindowConstants.DO_NOTHING_ON_CLOSE);
		setTitle(resbundle.getString("title"));
		UIUtil.setIcon(this);
		setJMenuBar(new MainMenuBar());
		
		isDND = false;
		
		panel = new MainPanel();
		panel.getConsoleClientAdapter().addConsoleCommandListener(new CommandAdapter());
		panel.addMainPanelListener(new MainPanelHandler());
		getContentPane().add(panel);
		
		addWindowListener(new WindowAdapter()
			{
				
				@Override
				public void windowActivated(WindowEvent e)
				{
					panel.requestFocusInWindow();
				}
				
				@Override
				public void windowClosing(WindowEvent event)
				{
					onClosing();
				}
				
			});
		
		updateWindowTitle();
		UIUtil.setDefaultWindowBounds(this, 850, 330, MainFrame.class);
		
	}
	
	protected enum SupportedCommand
	{
		EXIT,
		DND,
		BACK
	}
	
	protected class CommandAdapter extends EnumConsoleCommandAdapter<SupportedCommand>
	{
		
		public CommandAdapter()
		{
			super(SupportedCommand.class);
		}
		
		protected boolean processConsoleInput(ConsoleClientAdapter adapter, Client source, String context, SupportedCommand cmd, String params)
		{
			
			switch (cmd)
			{
				
				case EXIT:
					adapter.processConsoleInput(source, context, "/QUIT " + params);
					System.exit(0);
					return true;
				
				case DND:
					isDND = !isDND || params.length() > 0;
					if (params.length() > 0)
					{
						source.setAway(context, params);
					}
					updateWindowTitle();
					return true;
					
				case BACK:
					isDND = false;
					updateWindowTitle();
					return false;
				
				default:
					return false;
				
			}
			
		}
		
	}
	
	protected class MainPanelHandler implements MainPanelListener
	{
		
		public void clientStateChanged(MainPanel panel)
		{
			updateWindowTitle();
			getRootPane().putClientProperty("windowModified", Boolean.valueOf(panel.isDirty()));
		}
		
		public void panelRequestsAttention(MainPanel panel)
		{
			if (!isDND)
			{
				UIUtil.alert(MainFrame.this);
			}
		}
		
		public boolean linkClicked(MainPanel panel, java.net.URL url)
		{
			return false;
		}
		
	}
	
	protected void updateWindowTitle()
	{
		
		String title = panel.getTitle();
		
		if (title.length() == 0 || !FileUtil.isMac())
		{
			if (title.length() > 0)
			{
				title += " - ";
			}
			title += resbundle.getString("title");
		}
		
		if (panel.getClient().isConnected() && isDND)
		{
			title += " | DND";
		}
		
		setTitle(title);
		
	}
	
	protected void cmdPopupQuit_Click()
	{
		panel.getConsoleClientAdapter().processConsoleInput(panel.getClient(), null, "/exit");
	}
	
	protected void onClosing()
	{
		
		if (panel.isDirty())
		{
			
			final JTextField txtQuitMessage = new JTextField();
			txtQuitMessage.setText("Closing");
			final String title = "Confirm Disconnect";
			final Object[] message = {
					"Closing this window will disconnect from " + panel.getClient().getServerName() + ".",
					new JLabel(),
					"Quit Message:",
					txtQuitMessage
				};
			final String[] options = { "Disconnect", "Cancel" };
			
			int result = JOptionPane.showOptionDialog(
				this, message, title,
				JOptionPane.YES_NO_CANCEL_OPTION, JOptionPane.WARNING_MESSAGE, null,
				options, options[0]);
			
			// if anything but Disconnect (the first option) is chosen
			// (i.e. cancel button or confirm window closed),
			// the user wants to cancel the operation
			if (result != 0)
			{
				return; 
			}
			
			panel.getClient().quit(null, txtQuitMessage.getText());
			
		}
		
		panel.cleanup();
		dispose();
		
	}
	
	// this code needs to be possibly split off into it's own class in dirt.lib.ui.jfc
	// and updated to initially hide the MainFrame, restore it on clicking, provide
	// status updates in the tooltip, flash when something happens, etc
	/*
	protected void doMinToTray()
	{
	
		try
		{
			if (FileUtil.isWin())
			{
				FileUtil.loadLibrary("lib/win32/tray.dll");
			}
			if (FileUtil.isLinux())
			{
				FileUtil.loadLibrary("lib/linux_x86/libtray.so");
			}
		}
		catch (Exception e)
		{
			System.err.println("Error loading native tray library"); 
			return;
		}	
		
		org.jdesktop.jdic.tray.SystemTray tray =
			org.jdesktop.jdic.tray.SystemTray.getDefaultSystemTray();
		org.jdesktop.jdic.tray.TrayIcon ti;
		
		JPopupMenu menu;
		JMenu  submenu;
		JMenuItem menuItem;
		JRadioButtonMenuItem rbMenuItem;
		JCheckBoxMenuItem cbMenuItem;
		
		if (Integer.parseInt(System.getProperty("java.version").substring(2,3)) >=5)
		{
			System.setProperty("javax.swing.adjustPopupLocationToFit", "false");
		}
		
		menu = new JPopupMenu("A Menu");
		
		// "Quit" menu item
		menu.addSeparator();
		menuItem = new JMenuItem("Quit");
		menuItem.addActionListener(new ActionListener()
			{
				public void actionPerformed(ActionEvent evt)
				{
					cmdPopupQuit_Click();
				}
			});
		menu.add(menuItem);
		
		ti = new org.jdesktop.jdic.tray.TrayIcon(
			UIUtil.loadImageIcon(true),
			"Dirt Secure Chat", menu);
		
		ti.setIconAutoSize(true);
		ti.addActionListener(new ActionListener()
			{
				long firstWhen = 0;
				public void actionPerformed(ActionEvent e)
				{
					if (firstWhen != 0 && (e.getWhen() - firstWhen) <= 700)
					{
						firstWhen = 0;
						Popup_DblClick();
					}
					else
					{
						firstWhen = e.getWhen();
					}
				}
			});
		
		tray.addTrayIcon(ti);
	
	}

	protected void Popup_DblClick()
	{
		setVisible(!isVisible());
		if (isVisible())
		{	
			UIUtil.stealFocus(this);
		}
	}
	*/
	
}
