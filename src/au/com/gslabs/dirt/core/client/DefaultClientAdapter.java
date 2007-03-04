package au.com.gslabs.dirt.core.client;

import java.util.ArrayList;
import java.util.Date;
import java.util.HashMap;
import au.com.gslabs.dirt.lib.util.*;
import au.com.gslabs.dirt.core.client.enums.*;

public abstract class DefaultClientAdapter implements ClientListener
{
	
	public abstract void clientConsoleOutput(Client source, String context, String className, String message);
	
	public String[] getClientSupportedCommands(Client source)
	{
		return new String[0];
	}
	
	public boolean clientPreprocessConsoleInput(Client source, String context, String cmd, String params)
	{
		return false;
	}
	
	public String getClientExtraVersionInfo(Client source)
	{
		return null;
	}
	
	public void clientNotification(Client source, String context, NotificationSeverity severity, String type, String message)
	{
		StringBuilder buff = new StringBuilder();
		buff.append("*** ");
		switch (severity)
		{
			case DEBUG:
				buff.append("Debug: ");
				break;
			case ERROR:
				buff.append("Error: ");
				break;
		}
		if (type != null && type.length() > 0)
		{
			buff.append(type);
			buff.append(": ");
		}
		buff.append(message);
		clientConsoleOutput(source, context, severity.toString().toLowerCase(), buff.toString());
	}
	
	public void clientStateChanged(Client source)
	{
	}
	
	public void clientChatMessage(Client source, String context, String nick, String text, MessageDirection direction, ChatMessageType type, ChatMessageVisibility visibility)
	{
		
		String className;
		switch (type)
		{
			case ACTION:
				className = "chat-action";
				break;
			default:
				className = "chat-message";
				break;
		}
		
		String sep = text.startsWith("'s") ? "" : " ";
		
		switch (direction)
		{
			
			case INBOUND:
				switch (visibility)
				{
					case PRIVATE:
						if (type == ChatMessageType.ACTION)
						{
							clientConsoleOutput(source, context, className, "* *" + nick + "*" + sep + text);
						}
						else
						{
							clientConsoleOutput(source, context, className, "*" + nick + "* " + text);
						}
						break;
					case PUBLIC:
						if (type == ChatMessageType.ACTION)
						{
							clientConsoleOutput(source, context, className, "* " + nick + sep + text);
						}
						else
						{
							clientConsoleOutput(source, context, className, "<" + nick + "> " + text);
						}
						break;
					default:
						throw new RuntimeException("Unknown visibility");
				}
				break;
				
			case OUTBOUND:
				switch (visibility)
				{
					case PRIVATE:
						if (type == ChatMessageType.ACTION)
						{
							clientConsoleOutput(source, context, className, "-> *" + nick + "* * " + source.getNickname() + sep + text);
						}
						else
						{
							clientConsoleOutput(source, context, className, "-> *" + nick + "* " + text);
						}
						break;
					case PUBLIC:
						// public msgs will come back via INBOUND
						// no need to output them here
						break;
					default:
						throw new RuntimeException("Unknown visibility");
				}
				break;
			
			default:
				throw new RuntimeException("Unknown direction");
		
		}
	}
	
	public void clientCTCP(Client source, String context, String nick, String type, ByteBuffer data, MessageDirection direction, CTCPStage stage)
	{
		
		if (direction == MessageDirection.OUTBOUND && stage == CTCPStage.RESPONSE)
		{
			// don't care about displaying these
			return;
		}
		
		StringBuilder buff = new StringBuilder();
		if (direction == MessageDirection.OUTBOUND)
		{
			buff.append("-> ");
		}
		buff.append("[");
		buff.append(nick);
		if (direction == MessageDirection.OUTBOUND)
		{
			buff.append("]");
		}
		if (type != null && type.length() > 0)
		{
			buff.append(' ');
			buff.append(type);
			buff.append(TextModifier.ORIGINAL);
		}
		if (direction != MessageDirection.OUTBOUND)
		{
			if (stage == CTCPStage.RESPONSE)
			{
				buff.append(" reply");
			}
			buff.append("]");
		}
		if (data != null && data.length() > 0)
		{
			if (stage == CTCPStage.RESPONSE)
			{
				buff.append(":");
			}
			buff.append(' ');
			buff.append(data.toString());
		}
		
		clientConsoleOutput(source, context, "ctcp", buff.toString());
		
	}
	
	public void clientUserListReceived(Client source, ArrayList<String> nicklist)
	{
		throw new RuntimeException("todo");
	}
	
	public void clientUserJoin(Client source, String nick, String details)
	{
		throw new RuntimeException("todo");
	}
	
	public void clientUserPart(Client source, String nick, String details, String message)
	{
		throw new RuntimeException("todo");
	}
	
	public void clientUserNick(Client source, String nick, String new_nick)
	{
		throw new RuntimeException("todo");
	}
	
	public void clientUserStatus(Client source, String nick, String message, Date away_time, long away_duration_secs, long previous_away_duration_secs, String previous_away_message)
	{
		throw new RuntimeException("todo");
	}
	
	public void clientUserWhois(Client source, String context, HashMap<String,ByteBuffer> details)
	{
		throw new RuntimeException("todo");
	}
	
}
