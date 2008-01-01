package au.com.gslabs.dirt.ui.jfc.client;

import java.util.ResourceBundle;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.border.*;
import org.jdesktop.jdic.tray.*;
import au.com.gslabs.dirt.lib.util.*;
import au.com.gslabs.dirt.lib.ui.jfc.*;
import au.com.gslabs.dirt.core.client.*;
import au.com.gslabs.dirt.ui.common.client.ContactNickCompletor;
import au.com.gslabs.dirt.core.client.enums.*;

// this needs MRJAdapter support

public class MainFrame extends JFrame
{

	protected Client client;
	protected DefaultClientAdapter clientAdapter;
	ContactListModel contacts;
	protected boolean isDND;
	
	protected ResourceBundle resbundle;
	LogPane txtLog;
	InputArea txtInput;
	JPasswordField txtPassword;
	JList lstContacts;
	JComponent activeInputControl;
	JSplitPane splitContacts;

	MainFrame()
	{
		
		super("");
		setDefaultCloseOperation(WindowConstants.DO_NOTHING_ON_CLOSE);
		resbundle = ResourceBundle.getBundle("res/strings");
		setTitle(resbundle.getString("title"));
		UIUtil.setIcon(this);
		setJMenuBar(new MainMenuBar());
		
		getContentPane().setLayout(new BorderLayout());
		
		txtInput = new InputArea();
		txtInput.addInputListener(new InputArea.InputListener()
			{
				
				public void inputPerformed(InputArea source, String[] lines)
				{
					txtInput_Input(lines);
				}
				
				public void inputCompletionCandidates(InputArea source, String[] candidates)
				{
					StringBuilder sb = new StringBuilder();
					sb.append("  ");
					for (String candidate : candidates)
					{
						sb.append("  ");
						if (candidate.indexOf(' ') > -1)
						{
							sb.append('"');
						}
						sb.append(candidate);
						if (candidate.indexOf(' ') > -1)
						{
							sb.append('"');
						}
					}
					txtLog.appendTextLine(sb.toString());
				}
				
				public boolean inputCanAddToHistory(InputArea source, String line)
				{
					return clientAdapter.isConsoleInputHistorySafe(line);
				}
				
			});
		
		txtPassword = new JPasswordField();
		txtPassword.addActionListener(new ActionListener()
			{
				public void actionPerformed(ActionEvent e)
				{
					String password = new String(txtPassword.getPassword());
					txtPassword.setText("");
					txtPassword_Input(password);
				}
			});
		txtPassword.setBorder(txtInput.getBorder());
		
		getContentPane().add(txtInput, BorderLayout.SOUTH);
		activeInputControl = txtInput;
		
		txtLog = new LogPane();
		txtLog.addLinkListener(new LogPane.LinkListener()
			{
				public void linkClicked(LogPane.LinkEvent e)
				{
					txtLog_LinkClick(e.getURL());
				}
			});
		
		contacts = new ContactListModel();
		lstContacts = new JList(contacts);
		lstContacts.setFocusable(false);
		JScrollPane scrlContacts = new JScrollPane(lstContacts);
		scrlContacts.setPreferredSize(new Dimension(160, 0)); // default width
		if (FileUtil.isMac())
		{
			scrlContacts.setVerticalScrollBarPolicy(JScrollPane.VERTICAL_SCROLLBAR_ALWAYS);
		}
		
		splitContacts = new JSplitPane(JSplitPane.HORIZONTAL_SPLIT, txtLog, scrlContacts);
		splitContacts.setResizeWeight(1); // keep the contact pane fixed width
		splitContacts.setContinuousLayout(true);
		getContentPane().add(splitContacts, BorderLayout.CENTER);
		if (FileUtil.isMac())
		{
			splitContacts.setDividerSize(3);
		}
		
		Color borderColor = new EtchedBorder().getShadowColor(splitContacts);
		Border borderLineBottom = BorderFactory.createMatteBorder(0,0,1,0, borderColor);
		Border borderLineLeft = BorderFactory.createMatteBorder(0,1,0,0, borderColor);
		Border borderLineRight = BorderFactory.createMatteBorder(0,0,0,1, borderColor);
		Border borderTextArea = BorderFactory.createEmptyBorder(1, 1, 1, FileUtil.isMac()?14:1);
		Border borderEmpty = BorderFactory.createEmptyBorder();
		splitContacts.setBorder(borderLineBottom);
		txtLog.setBorder(borderLineRight);
		scrlContacts.setBorder(FileUtil.isMac()?borderLineLeft:borderEmpty);
		txtInput.setBorder(borderTextArea);
		txtPassword.setBorder(borderTextArea);
		
		isDND = false;
		
		client = new Client();
		clientAdapter = new ClientAdapter();
		client.addClientListener(clientAdapter, new JFCInvoker());
		
		txtInput.setCompletor(new ContactNickCompletor(client));
		
		WindowAdapter wa = new WindowAdapter()
			{
				
				@Override
				public void windowActivated(WindowEvent e)
				{
					activeInputControl.requestFocusInWindow();
				}
				@Override
				public void windowGainedFocus(WindowEvent e)
				{
					activeInputControl.requestFocusInWindow();
				}
				
				@Override
				public void windowDeactivated(WindowEvent e)
				{
					txtLog.clearRedLine();
				}
				@Override
				public void windowLostFocus(WindowEvent e)
				{
					txtLog.clearRedLine();
				}
				
				@Override
				public void windowClosing(WindowEvent event)
				{
					onClosing();
				}
				
			};
		addWindowListener(wa);
		addWindowFocusListener(wa);
		
		updateWindowTitle();
		UIUtil.setDefaultWindowBounds(this, 850, 330, MainFrame.class);
		activeInputControl.requestFocusInWindow();
		
	}
	
	protected void setPasswordMode(boolean passwordMode)
	{
		JComponent txtOld = passwordMode ? txtInput : txtPassword;
		JComponent txtNew = passwordMode ? txtPassword : txtInput;
		if (activeInputControl != txtNew)
		{
			activeInputControl = txtNew;
			txtPassword.setPreferredSize(txtInput.getPreferredSize());
			txtPassword.setText("");
			txtOld.setVisible(false);
			txtNew.setVisible(true);
			getContentPane().remove(txtOld);
			getContentPane().add(txtNew, BorderLayout.SOUTH);
			getContentPane().validate();
			if (UIUtil.getActiveWindow() == this)
			{
				txtNew.requestFocusInWindow();
			}
		}
	}
	
	protected enum SupportedCommand
	{
		CLEAR,
		EXIT,
		TEST,
		DND,
		BACK
	}
	
	protected class ClientAdapter extends EnumClientAdapter<SupportedCommand>
	{
		
		@Override
		protected boolean clientPreprocessConsoleInput(Client source, String context, SupportedCommand cmd, String params)
		{
			
			switch (cmd)
			{
				
				case TEST:
					txtLog.addTestData();
					new Thread(new Runnable() {
						public void run()
						{
							try
							{
								Thread.sleep(2000);
							}
							catch (InterruptedException ex)
							{
							}
							UIUtil.alert(MainFrame.this);
						}
					}).start();
					return true;
				
				case CLEAR:
					txtLog.clearText();
					return true;
				
				case EXIT:
					processConsoleInput(source, context, "/QUIT " + params);
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
		
		public ClientAdapter()
		{
			super(SupportedCommand.class);
		}
		
		@Override
		public void clientContactUpdated(Client source, Contact contact)
		{
			contacts.updateContact(contact);
		}
		
		@Override
		public void clientUserListReceived(Client source, String[] nicklist)
		{
			// we have a nicklist, no need to let the default handler output text
		}
		
		@Override
		protected void clientConsoleOutput(Client source, String context, String className, boolean suppressAlert, String message)
		{
			if (!suppressAlert && isDisplayable())
			{
				if (UIUtil.getActiveWindow() != MainFrame.this)
				{
					txtLog.setRedLine();
					if (!isDND)
					{
						UIUtil.alert(MainFrame.this);
					}
				}
				if (!txtLog.isAtEnd())
				{
					txtLog.showAndPulseArrow();
				}
			}
			setPasswordMode(false);
			txtLog.appendTextLine(getOutputPrefix() + message, className);
		}
		
		@Override
		public void clientNeedNickname(Client source, String defaultNick, boolean defaultNickOkay)
		{
			if (defaultNickOkay)
			{
				source.setNickname(null, defaultNick);
				return;
			}
			setPasswordMode(false);
			String prompt = "/nick " + defaultNick;
			txtInput.setText(prompt, 6, prompt.length());
		}
		
		@Override
		public void clientNeedAuthentication(Client source, String prompt)
		{
			super.clientNeedAuthentication(source, prompt);
			setPasswordMode(true);
		}
		
		@Override
		public void clientStateChanged(Client source)
		{
			super.clientStateChanged(source);
			if (!source.isConnected())
			{
				setPasswordMode(false);
			}
			updateWindowTitle();
			getRootPane().putClientProperty("windowModified", Boolean.valueOf(isDirty()));
		}
		
	}
	
	public boolean isDirty()
	{
		return client.isConnected() && client.getNickname() != null;
	}
	
	protected void updateWindowTitle()
	{
		
		String title = "";
		
		if (client.isConnected())
		{
			
			String nick = client.getNickname();
			if (nick != null && nick.length() > 0)
			{
				title += nick + " on ";
			}
			else
			{
				title += "";
			}
			
			if (client.getServerName() != null)
			{
				title += client.getServerName();
				if (!client.getServerHostname().equals(client.getServerName()) &&
					!client.getServerHostname().equals("localhost"))
				{
					title += " (" + client.getServerHostname() + ")";
				}
			}
			else
			{
				title += client.getServerHostname();
			}
			
			Duration latency = client.getLatency();
			if (latency != null)
			{
				title += " (" + latency.toString(Duration.Precision.MILLISECONDS, Duration.OutputFormat.MEDIUM) + " lag)";
			}
			
			if (!FileUtil.isMac())
			{
				title += " - " + resbundle.getString("title");
			}
			
			if (isDND)
			{
				title += " | DND";
			}
			
		}
		else
		{
			title = resbundle.getString("title");
		}
		
		setTitle(title);
		
	}
	
	protected void txtLog_LinkClick(java.net.URL url)
	{
		UIUtil.openURL(url.toString());
	}
	
	protected void txtInput_Input(String[] lines)
	{
		txtLog.clearRedLine();
		clientAdapter.processConsoleInput(client, null, lines);
	}
	
	protected void txtPassword_Input(String password)
	{
		txtLog.clearRedLine();
		client.authenticate(null, password, false);
	}
	
	protected void cmdPopupQuit_Click()
	{
		clientAdapter.processConsoleInput(client, null, "/exit");
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
	
	protected void onClosing()
	{
		
		if (isDirty())
		{
			
			final JTextField txtQuitMessage = new JTextField();
			txtQuitMessage.setText("Closing");
			final String title = "Confirm Disconnect";
			final Object[] message = {
					"Closing this window will disconnect from " + client.getServerName() + ".",
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
			
			client.quit(null, txtQuitMessage.getText());
			
		}
		
		txtLog.clearText();
		setVisible(false);
		if (client.isConnected())
		{
			client.disconnect(null, 2000);
		}
		dispose();
		
	}

}
