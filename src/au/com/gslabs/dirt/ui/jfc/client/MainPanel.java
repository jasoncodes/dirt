package au.com.gslabs.dirt.ui.jfc.client;

import java.util.*;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.border.*;
import org.jdesktop.jdic.tray.*;
import au.com.gslabs.dirt.lib.util.*;
import au.com.gslabs.dirt.lib.ui.jfc.*;
import au.com.gslabs.dirt.core.client.*;
import au.com.gslabs.dirt.core.client.console.*;
import au.com.gslabs.dirt.ui.common.client.ContactNickCompletor;
import au.com.gslabs.dirt.core.client.enums.*;

public class MainPanel extends JPanel
{
	
	protected final ResourceBundle resbundle = ResourceBundle.getBundle("res/strings");
	protected final Client client;
	protected final ConsoleClientAdapter clientAdapter;
	protected final ContactListModel contacts;
	
	protected LogPane txtLog;
	protected InputArea txtInput;
	protected JPasswordField txtPassword;
	protected JList lstContacts;
	protected JComponent activeInputControl;
	protected JSplitPane splitContacts;
	
	protected final ArrayList<MainPanelListener> listeners = new ArrayList<MainPanelListener>();
	protected boolean isFocused = false;
	protected String clientExtraVersionInfo = null;
	
	public MainPanel()
	{
		
		setLayout(new BorderLayout());
		
		contacts = new ContactListModel();
		client = new Client();
		clientAdapter = new ClientAdapter();
		client.addClientListener(clientAdapter, new JFCInvoker());
		
		createControls();
		
		new ContainerFocusEventProxy(this).addFocusListener(new FocusListener()
			{
				public void focusLost(FocusEvent e)
				{
					isFocused = false;
					txtLog.clearRedLine();
				}
				public void focusGained(FocusEvent e)
				{
					isFocused = true;
					activeInputControl.requestFocusInWindow();
				}
			});
		
	}
	
	protected enum SupportedCommand
	{
		CLEAR,
		TEST
	}
	
	protected class CommandAdapter extends EnumConsoleCommandAdapter<SupportedCommand>
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
							if (!isFocused)
							{
								for (MainPanelListener l : listeners)
								{
									l.panelRequestsAttention(MainPanel.this);
								}
							}
						}
					}).start();
					return true;
				
				case CLEAR:
					txtLog.clearText();
					return true;
				
				default:
					return false;
				
			}
			
		}
		
	}
	
	protected class ClientAdapter extends ConsoleClientAdapter
	{
		
		public ClientAdapter()
		{
			addConsoleCommandListener(new CommandAdapter());
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
		public String getClientExtraVersionInfo(Client source)
		{
			return clientExtraVersionInfo;
		}
		
		@Override
		protected void clientConsoleOutput(Client source, String context, String className, boolean suppressAlert, String message)
		{
			if (!suppressAlert && isDisplayable())
			{
				if (!isFocused)
				{
					txtLog.setRedLine();
					for (MainPanelListener l : listeners)
					{
						l.panelRequestsAttention(MainPanel.this);
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
			for (MainPanelListener l : listeners)
			{
				l.clientStateChanged(MainPanel.this);
			}
		}
		
	}
	
	protected void createControls()
	{
		
		txtInput = new InputArea();
		txtInput.setCompletor(new ContactNickCompletor(client));
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
		
		add(txtInput, BorderLayout.SOUTH);
		activeInputControl = txtInput;
		
		txtLog = new LogPane();
		txtLog.addLinkListener(new LogPane.LinkListener()
			{
				public void linkClicked(LogPane.LinkEvent e)
				{
					txtLog_LinkClick(e.getURL());
				}
			});
		
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
		add(splitContacts, BorderLayout.CENTER);
		if (FileUtil.isMac())
		{
			splitContacts.setDividerSize(3);
		}
		
		final Color borderColor = getBorderColor();
		final Border borderLineBottom = BorderFactory.createMatteBorder(0,0,1,0, borderColor);
		final Border borderLineLeft = BorderFactory.createMatteBorder(0,1,0,0, borderColor);
		final Border borderLineRight = BorderFactory.createMatteBorder(0,0,0,1, borderColor);
		final Border borderTextArea = BorderFactory.createEmptyBorder(1, 1, 1, FileUtil.isMac()?14:1);
		final Border borderEmpty = BorderFactory.createEmptyBorder();
		splitContacts.setBorder(borderLineBottom);
		txtLog.setBorder(borderLineRight);
		scrlContacts.setBorder(FileUtil.isMac()?borderLineLeft:borderEmpty);
		txtInput.setBorder(borderTextArea);
		txtPassword.setBorder(borderTextArea);
		
	}
	
	public void setClientExtraVersionInfo(String value)
	{
		this.clientExtraVersionInfo = value;
	}
	
	public Color getBorderColor()
	{
		return new EtchedBorder().getShadowColor(splitContacts);
	}
	
	public void addMainPanelListener(MainPanelListener l)
	{
		listeners.add(l);
	}
	
	public void removeMainPanelListener(MainPanelListener l)
	{
		int idx = listeners.indexOf(l);
		listeners.remove(idx);
	}
	
	protected void txtLog_LinkClick(final java.net.URL url)
	{
		for (MainPanelListener l : listeners)
		{
			if (l.linkClicked(MainPanel.this, url))
			{
				return;
			}
		}
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
			remove(txtOld);
			add(txtNew, BorderLayout.SOUTH);
			validate();
			txtNew.requestFocusInWindow();
		}
	}
	
	public Client getClient()
	{
		return client;
	}
	
	public ConsoleClientAdapter getConsoleClientAdapter()
	{
		return clientAdapter;
	}
	
	public boolean isDirty()
	{
		return client.isConnected() && client.getNickname() != null;
	}
	
	public void cleanup()
	{
		txtLog.clearText();
		setVisible(false);
		if (client.isConnected())
		{
			client.disconnect(null, 2000);
		}
	}
	
	public String getTitle()
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
			
		}
		
		return title;
		
	}
	
}
