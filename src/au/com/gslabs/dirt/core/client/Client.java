package au.com.gslabs.dirt.core.client;

import java.util.*;
import au.com.gslabs.dirt.core.client.enums.*;
import au.com.gslabs.dirt.lib.util.*;

public class Client
{
	
	protected ArrayList<ClientListener> listeners = new ArrayList<ClientListener>();
	
	public void addClientListener(ClientListener l)
	{
		listeners.add(l);
	}
	
	public void removeClientListener(ClientListener l)
	{
		listeners.remove(listeners.indexOf(l));
	}
	
	public void processConsoleInput(String context, String[] lines)
	{
		for (String line : lines)
		{
			processConsoleInput(context, line);
		}
	}
	
	public void processConsoleInput(String context, String line)
	{
		
		if (!line.startsWith("/"))
		{
			throw new IllegalArgumentException("Expected input to start with slash");
		}
		String cmd, params;
		int idx = line.indexOf(" ");
		if (idx < 0)
		{
			cmd = line.substring(1);
			params = "";
		}
		else
		{
			cmd = line.substring(1, idx);
			params = line.substring(idx+1);
		}
		String org_cmd = cmd;
		cmd = cmd.toUpperCase().trim();
		
		for (ClientListener l : listeners)
		{
			if (l.clientPreprocessConsoleInput(this, context, cmd, params))
			{
				return;
			}
		}
		
		if (cmd.length() > 3 && cmd.substring(0, 3).equals("ME'"))
		{
			processConsoleInput(context, "/me " + org_cmd.substring(2) + " " + params);
			return;
		}
		
		ConsoleCommand cmdEnum;
		
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
	
	protected boolean processConsoleCommand(String context, ConsoleCommand cmd, String params)
	{
		
		switch (cmd)
		{
			
			case SAY:
			case ME:
				ChatMessageType type = (cmd == ConsoleCommand.ME) ? ChatMessageType.ACTION : ChatMessageType.TEXT;
				for (ClientListener l : listeners)
				{
					// test stub
					l.clientChatMessage(this, context, getNickname(), params, MessageDirection.OUTBOUND, type, ChatMessageVisibility.PUBLIC);
					l.clientChatMessage(this, context, getNickname(), params, MessageDirection.INBOUND, type, ChatMessageVisibility.PUBLIC);
				}
				return true;
				
			case MY:
				processConsoleInput(context, "/me 's " + params);
				return true;
				
			case HEXDUMP:
				notification(context, NotificationSeverity.DEBUG, cmd.toString(), new ByteBuffer(params).toHexString());
				return true;
				
			case HELP:
				SortedSet<String> cmds = new TreeSet<String>();
			
				for (ConsoleCommand entry : ConsoleCommand.class.getEnumConstants())
				{
					cmds.add(entry.toString());
				}
			
				for (ClientListener l : listeners)
				{
					for (String entry : l.getClientSupportedCommands(this))
					{
						cmds.add(entry);
					}
				}
			
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
	
	protected void notification(String context, NotificationSeverity severity, String type, String message)
	{
		for (ClientListener l : listeners)
		{
			l.clientNotification(this, context, severity, type, message);
		}
	}
	
	public String getNickname()
	{
		// test stub
		return System.getProperty("user.name");
	}
	
}
