package au.com.gslabs.dirt.ui.jfc.client;

import java.util.Locale;
import java.util.ResourceBundle;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import org.jdesktop.jdic.tray.*;
import au.com.gslabs.dirt.lib.util.*;
import au.com.gslabs.dirt.lib.ui.jfc.*;
import au.com.gslabs.dirt.core.client.*;
import au.com.gslabs.dirt.ui.common.client.ContactNickCompletor;

// this needs MRJAdapter support

public class MainFrame extends JFrame
{

	protected ResourceBundle resbundle;
	protected Action newAction, openAction, closeAction, saveAction, saveAsAction,
		undoAction, cutAction, copyAction, pasteAction, clearAction, selectAllAction;
	static final JMenuBar mainMenuBar = new JMenuBar();	
	protected JMenu fileMenu, editMenu; 
	protected Client client;
	
	public static void init()
	{
		UIUtil.initSwing();
		new MainFrame();
	}
	
	LogPane txtLog;
	InputArea txtInput;
	JPasswordField txtPassword;
	ContactListModel contacts;
	JList lstContacts;
	
	private MainFrame()
	{
		
		super("");
		resbundle = ResourceBundle.getBundle("strings", Locale.getDefault());
		setTitle(resbundle.getString("title"));
		UIUtil.setIcon(this);
		
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
		
		contacts = new ContactListModel();
		lstContacts = new JList(contacts);
		lstContacts.setFocusable(false);
		JScrollPane scrlContacts = new JScrollPane(lstContacts);
		scrlContacts.setPreferredSize(new Dimension(160, 0));
		if (FileUtil.isMac())
		{
			scrlContacts.setVerticalScrollBarPolicy(JScrollPane.VERTICAL_SCROLLBAR_ALWAYS);
		}
		getContentPane().add(scrlContacts, BorderLayout.EAST);
		
		txtLog = new LogPane();
		getContentPane().add(txtLog, BorderLayout.CENTER);
		
		txtLog.addLinkListener(new LogPane.LinkListener()
			{
				public void linkClicked(LogPane.LinkEvent e)
				{
					txtLog_LinkClick(e.getURL());
				}
			});
		
		getContentPane().add(txtInput, BorderLayout.SOUTH);
		
		client = new Client();
		client.addClientListener(new ClientAdapter(), new JFCInvoker());
		
		txtInput.setCompletor(new ContactNickCompletor(client));
		
		WindowAdapter wa = new WindowAdapter()
			{
				
				@Override
				public void windowActivated(WindowEvent e)
				{
					txtInput.requestFocusInWindow();
				}
				@Override
				public void windowGainedFocus(WindowEvent e)
				{
					txtInput.requestFocusInWindow();
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
					onClose();
				}
				
		};
		addWindowListener(wa);
		addWindowFocusListener(wa);
		
		setSize(850, 330);
		setVisible(true);
		txtInput.requestFocusInWindow();
		
	}
	
	protected void setPasswordMode(boolean passwordMode)
	{
		JComponent txtOld = passwordMode ? txtInput : txtPassword;
		JComponent txtNew = passwordMode ? txtPassword : txtInput;
		txtPassword.setPreferredSize(txtInput.getPreferredSize());
		txtPassword.setText("");
		getContentPane().remove(txtOld);
		getContentPane().add(txtNew, BorderLayout.SOUTH);
		getContentPane().validate();
		if (UIUtil.getActiveWindow() == this)
		{
			txtNew.requestFocusInWindow();
		}
	}
	
	protected enum SupportedCommand
	{
		CLEAR,
		MINTOTRAY,
		EXIT
	}
	
	protected class ClientAdapter extends EnumClientAdapter<SupportedCommand>
	{
		
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
			if (!suppressAlert && isDisplayable() && UIUtil.getActiveWindow() != MainFrame.this)
			{
				txtLog.setRedLine();
				UIUtil.alert(MainFrame.this);
			}
			txtLog.appendTextLine(getOutputPrefix() + message, className);
		}
		
		@Override
		protected boolean clientPreprocessConsoleInput(Client source, String context, SupportedCommand cmd, String params)
		{
			
			switch (cmd)
			{
				
				case CLEAR:
					txtLog.clearText();
					return true;
				
				case MINTOTRAY:
					doMinToTray();
					return true;
					
				case EXIT:
					source.processConsoleInput(context, "/QUIT " + params);
					System.exit(0);
					return true;
				
				default:
					return false;
				
			}
			
		}
		
		@Override
		public void clientNeedNickname(Client source, String defaultNick)
		{
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
			setPasswordMode(false);
			
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
				
			}
			else
			{
				title = resbundle.getString("title");
			}
			
			setTitle(title);
			
		}
		
	}
	
	protected void txtLog_LinkClick(java.net.URL url)
	{
		UIUtil.openURL(url.toString());
	}
	
	protected void txtInput_Input(String[] lines)
	{
		txtLog.clearRedLine();
		client.processConsoleInput(null, lines);
	}
	
	protected void txtPassword_Input(String password)
	{
		txtLog.clearRedLine();
		client.authenticate(null, password, false);
	}
	
	protected void cmdPopupQuit_Click()
	{
		if (client.isConnected())
		{
			client.processConsoleInput(null, "/QUIT Closing");
		}
		System.exit(0);
	}
	
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
	
	protected void onClose()
	{
		if (client.isConnected())
		{
			client.processConsoleInput(null, "/QUIT Closing");
		}
		setVisible(false);
		dispose();
		if (!FileUtil.isMac())
		{
			System.exit(0);
		}
	}

}
