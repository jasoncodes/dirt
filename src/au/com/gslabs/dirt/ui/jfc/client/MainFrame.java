package au.com.gslabs.dirt.ui.jfc.client;

import java.util.ResourceBundle;
import java.awt.event.*;
import javax.swing.*;
import org.jdesktop.jdic.tray.*;
import au.com.gslabs.dirt.lib.util.FileUtil;
import au.com.gslabs.dirt.lib.ui.jfc.*;
import au.com.gslabs.dirt.core.client.*;
import au.com.gslabs.dirt.core.client.console.*;

public class MainFrame extends ClientFrame
{
	
	private boolean isDND;
	
	MainFrame()
	{
		
		super(new MainPanel());
		
		setDefaultCloseOperation(WindowConstants.DO_NOTHING_ON_CLOSE);
		
		isDND = false;
		
		panel.getConsoleClientAdapter().addConsoleCommandListener(new CommandAdapter());
		((MainPanel)panel).addMainPanelListener(new MainPanelHandler());
		
		addWindowListener(new WindowAdapter()
			{
				
				@Override
				public void windowClosing(WindowEvent event)
				{
					onClosing();
				}
				
			});
		
		updateWindowTitle();
		
		ActionListener gcPerformer = new ActionListener()
		{
			public void actionPerformed(ActionEvent e)
			{
				System.gc();
			}
		};
		new javax.swing.Timer(60*1000, gcPerformer).start();
		
	}
	
	private enum SupportedCommand
	{
		EXIT,
		DND,
		BACK
	}
	
	private class CommandAdapter extends EnumConsoleCommandAdapter<SupportedCommand>
	{
		
		public CommandAdapter()
		{
			super(SupportedCommand.class);
		}
		
		@Override
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
	
	private class MainPanelHandler implements MainPanelListener
	{
		
		public boolean clientPanelCreated(MainPanel panel, ClientPanel clientPanel)
		{
			final JFrame frame = new ClientFrame(clientPanel);
			UIUtil.setVisibleWithoutFocus(frame);
			return true;
		}
		
	}
	
	@Override
	protected void doAlert()
	{
		if (!isDND)
		{
			super.doAlert();
		}
	}
	
	@Override
	protected String getTitleSuffix()
	{
		if (panel.getClient().isConnected() && isDND)
		{
			return " | DND";
		}
		else
		{
			return null;
		}
	}
	
	private void onClosing()
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
			panel.cleanup();
			
		}
		
		dispose();
		
	}
	
	// this code needs to be possibly split off into it's own class in dirt.lib.ui.jfc
	// and updated to initially hide the MainFrame, restore it on clicking, provide
	// status updates in the tooltip, flash when something happens, etc
	/*
	private void cmdPopupQuit_Click()
	{
		panel.getConsoleClientAdapter().processConsoleInput(panel.getClient(), null, "/exit");
	}
	
	private void doMinToTray()
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

	private void Popup_DblClick()
	{
		setVisible(!isVisible());
		if (isVisible())
		{	
			UIUtil.stealFocus(this);
		}
	}
	*/
	
}
