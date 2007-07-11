package au.com.gslabs.dirt.core.client;

import java.util.*;
import au.com.gslabs.dirt.core.client.enums.*;
import au.com.gslabs.dirt.lib.util.*;
import au.com.gslabs.dirt.lib.thread.*;

public class Client
{
	
	protected final EventHandlers<ClientListener> listeners;
	
	public void addClientListener(ClientListener l)
	{
		listeners.add(l);
	}
	
	public void removeClientListener(ClientListener l)
	{
		listeners.remove(l);
	}
	
	public Client(Invoker eventInvoker)
	{
		this.listeners = new EventHandlers<ClientListener>(eventInvoker);
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
		HEXDUMP
	}
	
	protected boolean processConsoleCommand(final String context, final ConsoleCommand cmd, final String params)
	{
		
		switch (cmd)
		{
			
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
