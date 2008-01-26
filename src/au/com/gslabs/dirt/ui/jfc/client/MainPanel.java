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

public class MainPanel extends BaseClientPanel implements ChatPanel
{
	
	private final ResourceBundle resbundle = ResourceBundle.getBundle("res/strings");
	private final Client client;
	private final ConsoleClientAdapter clientAdapter;
	private final ContactListModel contacts;
	
	private final Preferences prefs;
	private final ActionListener prefListener;
	
	private boolean cleanupDone = false;
	private LogPane txtLog;
	private InputArea txtInput;
	private JPasswordField txtPassword;
	private JList lstContacts;
	private JComponent activeInputControl;
	private JSplitPane splitContacts;
	
	private String clientExtraVersionInfo = null;
	private final ArrayList<MainPanelListener> mainListeners = new ArrayList<MainPanelListener>();
	private final WeakHashMap<String,ClientPanel> panels = new WeakHashMap<String,ClientPanel>();
	
	public MainPanel()
	{
		
		super(null);
		panels.put(getContext(), this);
		
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
					txtLog.clearRedLine();
				}
				public void focusGained(FocusEvent e)
				{
					activeInputControl.requestFocusInWindow();
				}
			});
		
		prefs = Preferences.getInstance();
		prefListener = new ActionListener()
			{
				public void actionPerformed(ActionEvent e)
				{
					if (e.getActionCommand().equals(Preferences.NICKNAME))
					{
						loadPreferredNickname();
					}
				}
			};
		prefs.addActionListener(prefListener);
		loadPreferredNickname();
	}
	
	private enum SupportedCommand
	{
		QUERY,
		CLEAR,
		CONNECT,
		TEST
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
				
				case CONNECT:
					final String defaultServer = Preferences.getInstance().getDefaultServer();
					if (params.length() == 0 && !TextUtil.isEmpty(defaultServer))
					{
						source.connect(context, defaultServer);
						return true;
					}
					return false; // use default handler
				
				case QUERY:
					String[] tokens = TextUtil.splitQuotedHeadTail(params);
					if (tokens.length < 1 || tokens[0].length() < 1)
					{
						getConsoleClientAdapter().clientNotification(
							client, context, NotificationSeverity.ERROR,
							cmd.toString(), "Insufficient parameters");
					}
					else
					{
						openQueryPanel(context, tokens[0], tokens.length>1 ? tokens[1] : null);
					}
					return true;
				
				case TEST:
					getChatPanel(context).outputTestData();
					return true;
				
				case CLEAR:
					getChatPanel(context).clearText();
					return true;
				
				default:
					return false;
				
			}
			
		}
		
	}
	
	private class ClientAdapter extends ConsoleClientAdapter
	{
		
		public ClientAdapter()
		{
			addConsoleCommandListener(new CommandAdapter());
		}
		
		@Override
		protected String getContextForNickname(String nickname, boolean okayToCreate)
		{
			final Contact contact = getClient().getContact(nickname);
			final ClientPanel panel = getQueryPanel(contact, okayToCreate);
			return (panel != null) ? panel.getContext() : null;
		}
		
		@Override
		protected String getNicknameForContext(String context)
		{
			final ClientPanel panel = panels.get(context);
			if (panel instanceof QueryPanel)
			{
				final Contact contact = ((QueryPanel)panel).getContact();
				return contact.getNickname();
			}
			else
			{
				return null;
			}
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
			final ChatPanel panel = getChatPanel(context);
			panel.clientConsoleOutput(source, context, className, suppressAlert, message);
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
			notifyTitleChanged();
		}
		
	}
	
	private void loadPreferredNickname()
	{
		String nickname = prefs.getNickname();
		boolean isGood = true;
		if (TextUtil.isEmpty(nickname))
		{
			nickname = prefs.getDefaultNickname();
			isGood = false;
		}
		client.setPreferredNickname(nickname, isGood);
	}
	
	public void openQueryPanel(final String context, final String nickname, final String message)
	{
		
		// get the existing panel or create an new panel if required
		final Contact contact = getClient().getContact(nickname);
		
		if (contact == null)
		{
			getConsoleClientAdapter().clientNotification(
				client, context, NotificationSeverity.ERROR,
				SupportedCommand.QUERY.toString(), "No such nickname: " + nickname);
		}
		else
		{
			openQueryPanel(context, contact, message);
		}
	}
	
	public void openQueryPanel(final String context, final Contact contact, final String message)
	{
		
		final QueryPanel panel = getQueryPanel(contact, true);
		
		// focus the new panel (as it's a user requested function)
		final String effectiveContext;
		if (panel != null)
		{
			effectiveContext = panel.getContext();
			panel.requestFocus();
		}
		else
		{
			effectiveContext = context;
			getConsoleClientAdapter().clientNotification(
				client, context, NotificationSeverity.WARNING,
				SupportedCommand.QUERY.toString(), "Query windows unavailable");
		}
		
		// if the user passed a message, send it to the user
		if (message != null && getClient().ensureConnected(context, SupportedCommand.QUERY.toString()))
		{
			getClient().sendChatMessage(
				effectiveContext, contact.getNickname(),
				ChatMessageType.TEXT, new ByteBuffer(message));
		}
	
	}
	
	private QueryPanel getQueryPanel(final Contact contact, final boolean okayToCreate)
	{
		
		if (contact == null)
		{
			return null;
		}
		
		// search panels for a QueryPanel with said contact
		for (final ClientPanel panel : panels.values())
		{
			if (panel instanceof QueryPanel)
			{
				final QueryPanel qp = (QueryPanel)panel;
				if (contact.equals(qp.getContact()))
				{
					// got a match
					return qp;
				}
			}
		}
		
		// no existing panel found
		if (!okayToCreate)
		{
			// not allowed to create a new panel
			return null;
		}
		else
		{
			// create one and return it
			final QueryPanel panel = new QueryPanel(MainPanel.this, createNewContext(), contact);
			panels.put(panel.getContext(), panel);
			if (notifyClientPanelCreated(panel))
			{
				return panel;
			}
			else
			{
				// no event handler accepted this new panel
				// I guess we don't support multiple contexts
				panel.cleanup();
				return null;
			}
		}
		
	}
	
	public void unregisterPanel(ClientPanel panel)
	{
		panels.put(panel.getContext(), null);
	}
	
	public void clearText()
	{
		txtLog.clearText();
	}
	
	public void outputTestData()
	{
		txtLog.addTestData();
		requestAttentionAfterDelay(2000);
	}
	
	private String createNewContext()
	{
		String context;
		do
		{
			context = TextUtil.generateRandomAlphaNumeric(8);
		} while (panels.get(context) != null);
		return context;
	}
	
	private ClientPanel getPanel(String context)
	{
		if (context == null)
		{
			context = "";
		}
		return panels.get(context);
	}
	
	private ChatPanel getChatPanel(String context)
	{
		final ClientPanel panel = getPanel(context);
		if (panel instanceof ChatPanel)
		{
			return (ChatPanel)panel;
		}
		return this;
	}
	
	private boolean notifyClientPanelCreated(final ClientPanel panel)
	{
		for (MainPanelListener l : mainListeners)
		{
			if (l.clientPanelCreated(this, panel))
			{
				return true;
			}
		}
		return false;
	}
	
	public boolean clientConsoleOutput(Client source, String context, String className, boolean suppressAlert, String message)
	{
		if (!suppressAlert && isDisplayable())
		{
			if (!isFocused())
			{
				txtLog.setRedLine();
				requestAttention();
			}
			if (!txtLog.isAtEnd())
			{
				txtLog.showAndPulseArrow();
			}
		}
		setPasswordMode(false);
		txtLog.appendTextLine(ConsoleClientAdapter.getOutputPrefix() + message, className);
		return true;
	}
	
	private void createControls()
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
					notifyLinkClicked(e.getURL());
				}
			});
		
		lstContacts = new JList(contacts);
		lstContacts.addMouseListener(new MouseAdapter()
			{
				public void mouseClicked(MouseEvent e)
				{
					if (e.getClickCount() == 2)
					{
						final Object selection[] = lstContacts.getSelectedValues();
						if (selection.length == 1 && selection[0] instanceof Contact)
						{
							final Contact contact = (Contact)selection[0];
							openQueryPanel(null, contact, null);
						}
					}
				}
			});
		
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
	
	public void addMainPanelListener(MainPanelListener l)
	{
		mainListeners.add(l);
	}
	
	public void removeMainPanelListener(MainPanelListener l)
	{
		int idx = mainListeners.indexOf(l);
		mainListeners.remove(idx);
	}
	
	public void setClientExtraVersionInfo(String value)
	{
		this.clientExtraVersionInfo = value;
	}
	
	private void txtInput_Input(String[] lines)
	{
		txtLog.clearRedLine();
		clientAdapter.processConsoleInput(client, getContext(), lines);
	}
	
	private void txtPassword_Input(String password)
	{
		txtLog.clearRedLine();
		client.authenticate(getContext(), password, false);
	}
	
	private void setPasswordMode(boolean passwordMode)
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
		if (!cleanupDone)
		{
			cleanupDone = true;
			prefs.removeActionListener(prefListener);
			client.removeClientListener(clientAdapter);
			txtLog.clearText();
			if (client.isConnected())
			{
				client.disconnect(null, 2000);
			}
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
