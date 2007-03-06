package au.com.gslabs.dirt.core.client;

import java.util.ArrayList;
import java.util.Date;
import java.util.HashMap;
import java.util.Map;
import au.com.gslabs.dirt.lib.util.*;
import au.com.gslabs.dirt.core.client.enums.*;

public abstract class DefaultClientAdapter implements ClientListener
{
	
	protected java.text.SimpleDateFormat sdf = new java.text.SimpleDateFormat("[HH:mm] ");

	protected String getOutputPrefix()
	{
		java.util.Calendar c = java.util.Calendar.getInstance();
		return sdf.format(c.getTime());
	}
	
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
	
	public void clientUserListReceived(Client source, String[] nicklist)
	{
		StringBuilder sb = new StringBuilder();
		sb.append("*** Now chatting with: ");
		if (nicklist != null && nicklist.length > 0)
		{
			for (int i = 0; i < nicklist.length; ++i)
			{
				if (i > 0)
				{
					sb.append(", ");
				}
				sb.append(nicklist[i]);
			}
		}
		else
		{
			sb.append("(Nobody)");
		}
		clientConsoleOutput(source, null, "status", sb.toString());
	}
	
	public void clientUserJoin(Client source, String nick, String details)
	{
		StringBuilder sb = new StringBuilder();
		sb.append("*** ");
		if (nick.equals(source.getNickname()))
		{
			sb.append("Now chatting as ");
			sb.append(nick);
		}
		else
		{
			sb.append(nick);
			if (details != null && details.length() > 0)
			{
				sb.append(" (");
				sb.append(details);
				sb.append(TextModifier.ORIGINAL);
				sb.append(")");
			}
			sb.append(" has joined the chat");
		}
		clientConsoleOutput(source, null, "status", sb.toString());
	}
	
	public void clientUserPart(Client source, String nick, String details, String message)
	{
		StringBuilder sb = new StringBuilder();
		sb.append("*** ");
		sb.append(nick);
		if (details != null && details.length() > 0)
		{
			sb.append(" (");
			sb.append(details);
			sb.append(TextModifier.ORIGINAL);
			sb.append(")");
		}
		sb.append(" has left the chat");
		if (message != null && message.length() > 0)
		{
			sb.append(" (");
			sb.append(message);
			sb.append(TextModifier.ORIGINAL);
			sb.append(")");
		}
		clientConsoleOutput(source, null, "status", sb.toString());
	}
	
	public void clientUserNick(Client source, String old_nick, String new_nick)
	{
		StringBuilder sb = new StringBuilder();
		sb.append("*** ");
		if (new_nick.equals(source.getNickname()))
		{
			if (old_nick.equals(new_nick))
			{
				sb.append("Your nickname is ");
			}
			else
			{
				sb.append("You are now known as ");
			}
			sb.append(new_nick);
		}
		else
		{
			sb.append(old_nick);
			sb.append(" is now known as ");
			sb.append(new_nick);
		}
		clientConsoleOutput(source, null, "status", sb.toString());
	}
	
	public void clientUserStatus(Client source, String nick, UserStatus status, String message, Date away_start, Duration away_duration, Duration previous_away_duration, String previous_away_message)
	{
		StringBuilder sb = new StringBuilder();
		sb.append("*** ");
		sb.append(nick);
		if (status != UserStatus.RETURNED)
		{
			if (away_duration != null)
			{
				sb.append(" has been away for ");
				sb.append(away_duration.toString());
			}
			else
			{
				sb.append(" is away");
			}
			if (message != null && message.length() > 0)
			{
				sb.append(": ");
				sb.append(message);
				sb.append(TextModifier.ORIGINAL);
			}
			if (previous_away_message != null)
			{
				sb.append(" (was: ");
				sb.append(previous_away_message);
				sb.append(TextModifier.ORIGINAL);
				if (previous_away_duration != null)
				{
					sb.append(" for ");
					sb.append(previous_away_duration.toString());
				}
				sb.append(")");
			}
		}
		else
		{
			sb.append(" has returned");
			if (message != null && message.length() > 0)
			{
				sb.append(" (msg: ");
				sb.append(message);
				sb.append(TextModifier.ORIGINAL);
				sb.append(")");
			}
			if (away_duration != null)
			{
				sb.append(" (away for ");
				sb.append(away_duration.toString());
				sb.append(")");
			}
		}
		clientConsoleOutput(source, null, "status", sb.toString());
	}
	
	protected final class WhoisOutputter
	{
		private Client source;
		private String context;
		private String nickname;
		public WhoisOutputter(Client source, String context, String nickname)
		{
			this.source = source;
			this.context = context;
			this.nickname = nickname;
		}
		public void output(String text)
		{
			clientConsoleOutput(source, context, "whois", nickname + " " + text);
		}
	}
	
	public void clientUserWhois(Client source, String context, Map<String,ByteBuffer> details)
	{
		
		Map<String,ByteBuffer> data = new HashMap<String,ByteBuffer>(details);
		String nickname = data.get("NICK").toString();
		WhoisOutputter out = new WhoisOutputter(source, context, nickname);
		
		out.output("is " + data.get("DETAILS"));
		out.output("is connecting from " + data.get("HOSTNAME"));
		if (data.containsKey("ISADMIN"))
		{
			out.output("is a server administrator");
		}
		if (data.containsKey("AWAY"))
		{
			out.output("is away: " + data.get("AWAY"));
		}
		if (data.containsKey("AWAYTIMEDIFFSTRING"))
		{
			out.output("has been away for " + data.get("AWAYTIMEDIFFSTRING"));
		}
		out.output("is using " + data.get("AGENT"));
		out.output("has been idle for " + data.get("IDLESTRING") + " (" + data.get("LATENCYSTRING") + " lag)");
		out.output("signed on at " + data.get("JOINTIMESTRING"));
		
		data.remove("NICK");
		data.remove("DETAILS");
		data.remove("HOSTNAME");
		data.remove("ISADMIN");
		data.remove("AWAY");
		data.remove("AWAYTIME");
		data.remove("AWAYTIMEDIFF");
		data.remove("AWAYTIMEDIFFSTRING");
		data.remove("AGENT");
		data.remove("IDLE");
		data.remove("IDLESTRING");
		data.remove("LATENCY");
		data.remove("LATENCYSTRING");
		data.remove("JOINTIME");
		data.remove("JOINTIMESTRING");
		
		for (Map.Entry entry : data.entrySet())
		{
			out.output(entry.getKey() + " = " + entry.getValue());
		}
		
		out.output("End of /WHOIS");
		
	}
	
}
