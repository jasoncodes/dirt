package au.com.gslabs.dirt.core.client;

import java.util.*;
import au.com.gslabs.dirt.core.client.enums.*;
import au.com.gslabs.dirt.lib.util.*;
import au.com.gslabs.dirt.lib.thread.*;
import au.com.gslabs.dirt.lib.net.*;
import au.com.gslabs.dirt.lib.net.crypt.*;

public class Client
{
	
	protected final EventListeners<ClientListener> listeners = new EventListeners<ClientListener>();
	
	public void addClientListener(ClientListener l, Invoker i)
	{
		listeners.add(l, i);
	}
	
	public void removeClientListener(ClientListener l)
	{
		listeners.remove(l);
	}
	
	protected CryptSocket socket;

	public Client()
	{
		socket = new CryptSocket();
		socket.addCryptListener(new CryptListener()
			{
				public void cryptError(java.io.IOException ex)
				{
					notification(null, NotificationSeverity.ERROR, "CONNECT", "Connection error: "+ex);
				}
				public void cryptConnected()
				{
					notification(null, NotificationSeverity.INFO, "CONNECT", "Connected to " + socket.getPeerName());
				}
				public void cryptMessage(ByteBuffer data)
				{
					notification(null, NotificationSeverity.DEBUG, null, "Incoming data: \n" + data.toHexString(true));
				}
			}, new SameThreadInvoker());
	}
	
	public void processConsoleInput(String context, String[] lines)
	{
		for (String line : lines)
		{
			processConsoleInput(context, line);
		}
	}
	
	public void processConsoleInput(final String context, final String line)
	{
		
		if (!line.startsWith("/"))
		{
			throw new IllegalArgumentException("Expected input to start with slash");
		}
		final String org_cmd, params;
		final int idx = line.indexOf(" ");
		if (idx < 0)
		{
			org_cmd = line.substring(1);
			params = "";
		}
		else
		{
			org_cmd = line.substring(1, idx);
			params = line.substring(idx+1);
		}
		final String cmd = org_cmd.toUpperCase().trim();
		
		class ConsoleInputPreprocessor implements EventSource<ClientListener>
		{
			public boolean done = false;
			public void dispatchEvent(ClientListener l)
			{
				done |= l.clientPreprocessConsoleInput(Client.this, context, cmd, params);
			}
		}
		ConsoleInputPreprocessor cip = new ConsoleInputPreprocessor();
		listeners.dispatchEvent(cip, true);
		if (cip.done)
		{
			return;
		}
		
		if (cmd.length() > 3 && cmd.substring(0, 3).equals("ME'"))
		{
			processConsoleInput(context, "/me " + org_cmd.substring(2) + " " + params);
			return;
		}
		
		final ConsoleCommand cmdEnum;
		
		try
		{
			cmdEnum = ConsoleCommand.valueOf(cmd);
		}
		catch (Exception ex)
		{
			notification(context, NotificationSeverity.ERROR, cmd, "Unknown command");
			return;
		}
		
		if (!processConsoleCommand(context, cmdEnum, params))
		{
			notification(context, NotificationSeverity.ERROR, cmd, "Not implemented");
		}
		
	}
	
	protected enum ConsoleCommand
	{
		SAY,
		ME,
		MY,
		HELP,
		HEXDUMP,
		CONNECT
	}
	
	protected boolean processConsoleCommand(final String context, final ConsoleCommand cmd, final String params)
	{
		
		switch (cmd)
		{
			
			case CONNECT:
				URL url = new URL(params, "dirt", 11626);
				notification(context, NotificationSeverity.INFO, "CONNECT", "Connecting to " + url);
				if (!url.getProtocol().equals("dirt"))
				{
					notification(context, NotificationSeverity.ERROR, "CONNECT", "Unknown protocol: " + url.getProtocol());
				}
				else
				{
					socket.connect(url.getHostname(), url.getPort());
				}
				return true;
				
			case SAY:
			case ME:
				final ChatMessageType type = (cmd == ConsoleCommand.ME) ? ChatMessageType.ACTION : ChatMessageType.TEXT;
				listeners.dispatchEvent(new EventSource<ClientListener>()
					{
						public void dispatchEvent(ClientListener l)
						{
							l.clientChatMessage(Client.this, context, getNickname(), params, MessageDirection.OUTBOUND, type, ChatMessageVisibility.PUBLIC);
							l.clientChatMessage(Client.this, context, getNickname(), params, MessageDirection.INBOUND, type, ChatMessageVisibility.PUBLIC);
						}
					});
				return true;
				
			case MY:
				processConsoleInput(context, "/me 's " + params);
				return true;
				
			case HEXDUMP:
				notification(context, NotificationSeverity.DEBUG, cmd.toString(), new ByteBuffer(params).toHexString());
				return true;
				
			case HELP:
				final SortedSet<String> cmds = new TreeSet<String>();
				
				for (ConsoleCommand entry : ConsoleCommand.class.getEnumConstants())
				{
					cmds.add(entry.toString());
				}
				
				listeners.dispatchEvent(new EventSource<ClientListener>()
					{
						public void dispatchEvent(ClientListener l)
						{
							for (String entry : l.getClientSupportedCommands(Client.this))
							{
								cmds.add(entry);
							}
						}
					}, true);
				
				StringBuilder buff = new StringBuilder();
				buff.append("Supported commands:");
				for (String entry : cmds)
				{
					buff.append(" ");
					buff.append(entry);
				}
				notification(context, NotificationSeverity.INFO, cmd.toString(), buff.toString());
				
				return true;
			
			default:
				return false;
				
		}
		
	}
	
	protected void notification(final String context, final NotificationSeverity severity, final String type, final String message)
	{
		listeners.dispatchEvent(new EventSource<ClientListener>()
			{
				public void dispatchEvent(ClientListener l)
				{
					l.clientNotification(Client.this, context, severity, type, message);
				}
			});
	}
	
	public String getNickname()
	{
		// test stub
		return System.getProperty("user.name");
	}
	
}
