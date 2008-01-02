package au.com.gslabs.dirt.ui.jfc.client;

import java.util.ResourceBundle;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import org.jdesktop.jdic.tray.*;
import au.com.gslabs.dirt.lib.util.*;
import au.com.gslabs.dirt.lib.ui.jfc.*;
import au.com.gslabs.dirt.core.client.*;
import au.com.gslabs.dirt.core.client.console.*;
import au.com.gslabs.dirt.ui.common.client.ContactNickCompletor;
import au.com.gslabs.dirt.core.client.enums.*;

public class MainApplet extends JApplet
{
	
	protected Client client;
	protected ConsoleClientAdapter clientAdapter;
	ContactListModel contacts;
	
	protected final ResourceBundle resbundle;
	JLabel lblTitle;
	LogPane txtLog;
	InputArea txtInput;
	JPasswordField txtPassword;
	JList lstContacts;
	JComponent activeInputControl;
	
	public MainApplet()
	{
		
		resbundle = ResourceBundle.getBundle("res/strings");
		UIUtil.initSwing(resbundle.getString("name"));
		
		lblTitle = new JLabel();
		lblTitle.setText(resbundle.getString("title"));
		getContentPane().add(lblTitle, BorderLayout.NORTH);
		
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
		activeInputControl = txtInput;
		
		client = new Client();
		clientAdapter = new ClientAdapter();
		client.addClientListener(clientAdapter, new JFCInvoker());
		
		updateWindowTitle();
		txtInput.setCompletor(new ContactNickCompletor(client));
		activeInputControl.requestFocusInWindow();
		
	}
	
	protected void setPasswordMode(boolean passwordMode)
	{
		JComponent txtOld = passwordMode ? txtInput : txtPassword;
		JComponent txtNew = passwordMode ? txtPassword : txtInput;
		txtPassword.setPreferredSize(txtInput.getPreferredSize());
		txtPassword.setText("");
		getContentPane().remove(txtOld);
		getContentPane().add(txtNew, BorderLayout.SOUTH);
		activeInputControl = txtNew;
		getContentPane().validate();
		txtNew.requestFocusInWindow();
	}
	
	protected enum SupportedCommand
	{
		CLEAR
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
		protected void clientConsoleOutput(Client source, String context, String className, boolean suppressAlert, String message)
		{
			if (!suppressAlert)
			{
				txtLog.setRedLine();
			}
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
			setPasswordMode(false);
			updateWindowTitle();
			getRootPane().putClientProperty(
				"windowModified",
				(client.isConnected() && client.getNickname() != null) ?
					Boolean.TRUE :
					Boolean.FALSE);
		}
		
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
			
		}
		else
		{
			title =
				resbundle.getString("title") + " " +
				resbundle.getString("version") + " " +
				resbundle.getString("sourceDate");
		}
		
		lblTitle.setText(title);
		
	}
	
	protected void txtLog_LinkClick(java.net.URL url)
	{
		getAppletContext().showDocument(url, "_blank");
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
	
	public void destroy()
	{
		if (client.isConnected())
		{
			client.disconnect(null, 2000);
		}
	}
	
}
