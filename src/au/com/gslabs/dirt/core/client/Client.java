package au.com.gslabs.dirt.core.client;

import java.util.*;
import au.com.gslabs.dirt.core.client.enums.*;

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
	
	protected static final String SUPPORTED_COMMANDS = "SAY ME MY HELP";
	
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
		
		if (cmd.equals("SAY") || cmd.equals("ME"))
		{
			ChatMessageType type = cmd.equals("ME") ? ChatMessageType.ACTION : ChatMessageType.TEXT;
			for (ClientListener l : listeners)
			{
				// test stub
				l.clientChatMessage(this, context, getNickname(), params, MessageDirection.OUTBOUND, type, ChatMessageVisibility.PUBLIC);
				l.clientChatMessage(this, context, getNickname(), params, MessageDirection.INBOUND, type, ChatMessageVisibility.PUBLIC);
			}
		}
		else if (cmd.equals("MY"))
		{
			processConsoleInput(context, "/me 's " + params);
		}
		else if (cmd.length() > 3 && cmd.substring(0, 3).equals("ME'"))
		{
			processConsoleInput(context, "/me " + org_cmd.substring(2) + " " + params);
		}
		else if (cmd.equals("HELP"))
		{
			
			SortedSet<String> cmds = new TreeSet<String>();
			for (String entry : SUPPORTED_COMMANDS.split(" "))
			{
				cmds.add(entry);
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
			notification(context, NotificationSeverity.INFO, cmd, buff.toString());
			
		}
		else
		{
			notification(context, NotificationSeverity.ERROR, cmd, "Unknown command");
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
