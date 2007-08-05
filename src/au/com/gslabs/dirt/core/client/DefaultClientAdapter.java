package au.com.gslabs.dirt.core.client;

import java.util.*;
import au.com.gslabs.dirt.lib.util.*;
import au.com.gslabs.dirt.core.client.enums.*;
import au.com.gslabs.dirt.lib.net.URL;

public abstract class DefaultClientAdapter implements ClientListener
{
	
	protected static java.text.SimpleDateFormat sdf = new java.text.SimpleDateFormat("[HH:mm] ");
	
	public static String getOutputPrefix()
	{
		java.util.Calendar c = java.util.Calendar.getInstance();
		return sdf.format(c.getTime());
	}
	
	protected abstract void clientConsoleOutput(Client source, String context, String className, boolean suppressAlert, String message);
	
	protected boolean clientPreprocessConsoleInput(Client source, String context, String cmd, String params)
	{
		return false;
	}
	
	protected String[] getClientSupportedCommands(Client source)
	{
		return new String[0];
	}
	
	public String getClientExtraVersionInfo(Client source)
	{
		return null;
	}
	
	public void clientNotification(Client source, String context, NotificationSeverity severity, String type, String message)
	{
		
		final boolean haveType = type != null && type.length() > 0 && !type.equals("CONNECT");
		
		StringBuilder buff = new StringBuilder();
		buff.append("*** ");
		switch (severity)
		{
			case DEBUG:
				buff.append("Debug");
				buff.append(haveType ? " " : ": ");
				break;
			case ERROR:
				buff.append("Error");
				buff.append(haveType ? " " : ": ");
				break;
		}
		if (haveType)
		{
			buff.append(type);
			buff.append(": ");
		}
		buff.append(message);
		
		clientConsoleOutput(source, context, severity.toString().toLowerCase(), false, buff.toString());
	}
	
	public void clientNeedNickname(Client source, String defaultNick)
	{
		clientNotification(source, null, NotificationSeverity.WARNING, "NICK", "Nickname required");
	}
	
	public void clientNeedAuthentication(Client source, String prompt)
	{
		clientNotification(source, null, NotificationSeverity.WARNING, "AUTH", prompt);
	}
	
	public void clientStateChanged(Client source)
	{
	}
	
	public void clientChatMessage(Client source, String context, String nick, ByteBuffer message, MessageDirection direction, ChatMessageType type, ChatMessageVisibility visibility)
	{
		
		final String className;
		switch (type)
		{
			case ACTION:
				className = "chat-action";
				break;
			default:
				className = "chat-message";
				break;
		}
		
		final String text = message.toString();
		
		final String sep = (
				text.startsWith("'s ") ||
				text.startsWith("'d ")
			) ? "" : " ";
		
		final boolean suppressAlert =
			(direction == MessageDirection.OUTBOUND) ||
			(nick.equals(source.getNickname()));
		
		switch (direction)
		{
			
			case INBOUND:
				switch (visibility)
				{
					case PRIVATE:
						if (type == ChatMessageType.ACTION)
						{
							clientConsoleOutput(source, context, className, suppressAlert, "* *" + nick + "*" + sep + text);
						}
						else
						{
							clientConsoleOutput(source, context, className, suppressAlert, "*" + nick + "* " + text);
						}
						break;
					case PUBLIC:
						if (type == ChatMessageType.ACTION)
						{
							clientConsoleOutput(source, context, className, suppressAlert, "* " + nick + sep + text);
						}
						else
						{
							clientConsoleOutput(source, context, className, suppressAlert, "<" + nick + "> " + text);
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
							clientConsoleOutput(source, context, className, true, "-> *" + nick + "* * " + source.getNickname() + sep + text);
						}
						else
						{
							clientConsoleOutput(source, context, className, true, "-> *" + nick + "* " + text);
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
		
		final boolean suppressAlert =
			(direction == MessageDirection.OUTBOUND) ||
			(nick.equals(source.getNickname()));
		
		clientConsoleOutput(source, context, "ctcp", suppressAlert, buff.toString());
		
	}
	
	public void clientContactUpdated(Client source, Contact contact)
	{
	}
	
	public void clientUserListReceived(Client source, String[] nicklist)
	{
		StringBuilder sb = new StringBuilder();
		sb.append("*** Chatting with: ");
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
		clientConsoleOutput(source, null, "status-userlist", false, sb.toString());
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
		clientConsoleOutput(source, null, "status-join", false, sb.toString());
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
		clientConsoleOutput(source, null, "status-part", false, sb.toString());
	}
	
	public void clientUserNick(Client source, String old_nick, String new_nick)
	{
		
		final StringBuilder sb = new StringBuilder();
		final boolean suppressAlert;
		
		sb.append("*** ");
		if (source.getNickname() == null && new_nick == null)
		{
			sb.append("You have no nickname");
			suppressAlert = false;
		}
		else if (new_nick.equals(source.getNickname()))
		{
			if (new_nick.equals(old_nick))
			{
				sb.append("Your nickname is ");
			}
			else
			{
				sb.append("You are now known as ");
			}
			sb.append(new_nick);
			suppressAlert = true;
		}
		else
		{
			sb.append(old_nick);
			sb.append(" is now known as ");
			sb.append(new_nick);
			suppressAlert = false;
		}
		
		clientConsoleOutput(source, null, "status-nick", suppressAlert, sb.toString());
		
	}
	
	public void clientUserStatus(Client source, String nick, UserStatus status, String message, Date away_start, Duration away_duration, Duration previous_away_duration, String previous_away_message)
	{
		
		final StringBuilder sb = new StringBuilder();
		boolean suppressAlert = nick.equals(source.getNickname());
		
		sb.append("*** ");
		sb.append(nick);
		if (status == UserStatus.AWAY)
		{
			if (away_duration != null && away_duration.getMilliseconds() > 500)
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
		else if (status == UserStatus.ONLINE)
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
		else
		{
			sb.append(" is " + status);
		}
		
		clientConsoleOutput(source, null, "status-"+status.toString().toLowerCase(), suppressAlert, sb.toString());
		
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
			clientConsoleOutput(source, context, "whois", false, nickname + " " + text);
		}
	}
	
	public void clientUserWhois(Client source, String context, String nickname)
	{
		
		Contact contact = source.getContact(nickname);
		WhoisOutputter out = new WhoisOutputter(source, context, nickname);
		
		if (contact.detailString != null)
		{
			out.output("is " + contact.detailString);
		}
		if (contact.hostname != null)
		{
			out.output("is connecting from " + contact.hostname);
		}
		if (contact.isAdmin)
		{
			out.output("is a server administrator");
		}
		if (contact.status == UserStatus.AWAY)
		{
			out.output("is away: " + contact.awayMessage);
			if (contact.getAwayDuration() != null)
			{
				out.output("has been away for " + contact.getAwayDuration());
			}
		}
		out.output("is using " + contact.userAgent);
		out.output(
			"has been idle for " + new Duration(contact.idleMilliseconds) +
			" (" + new Duration(contact.latencyMilliseconds).toString(Duration.Precision.MILLISECONDS, Duration.OutputFormat.MEDIUM) + " lag)");
		out.output("signed on at " + TextUtil.formatDateTime(contact.joinTimeServer, true, false));
		
		for (Map.Entry entry : contact.other.entrySet())
		{
			out.output(entry.getKey() + " = " + entry.getValue());
		}
		
		out.output("End of /WHOIS");
		
	}
	
	public boolean isConsoleInputHistorySafe(String line)
	{
		return !(TextUtil.splitQuotedHeadTail(line)[0].equalsIgnoreCase("/oper"));
	}
	
	public void processConsoleInput(Client client, String context, String[] lines)
	{
		for (String line : lines)
		{
			processConsoleInput(client, context, line);
		}
	}
	
	public void processConsoleInput(Client client, final String context, final String line)
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
		
		if (clientPreprocessConsoleInput(client, context, cmd, params))
		{
			return;
		}
		
		if (cmd.length() > 3 && cmd.substring(0, 3).equals("ME'"))
		{
			processConsoleInput(client, context, "/me " + org_cmd.substring(2) + " " + params);
			return;
		}
		
		final ConsoleCommand cmdEnum;
		
		try
		{
			cmdEnum = ConsoleCommand.valueOf(cmd);
		}
		catch (Exception ex)
		{
			clientNotification(client, context, NotificationSeverity.ERROR, cmd, "Unknown command");
			return;
		}
		
		if (!processConsoleCommand(client, context, cmdEnum, params))
		{
			clientNotification(client, context, NotificationSeverity.ERROR, cmd, "Not implemented");
		}
		
	}
	
	protected static boolean ensureConnected(Client client, String context, ConsoleCommand cmd)
	{
		return ensureConnected(client, context, (cmd != null && cmd != ConsoleCommand.SAY) ? cmd.toString() : null);
	}
	
	protected static boolean ensureConnected(Client client, String context, String cmd)
	{
		return client.ensureConnected(context, cmd);
	}
	
	protected enum ConsoleCommand
	{
		SAY,
		ME,
		MY,
		HELP,
		CONNECT,
		NICK,
		MSG,
		MSGME,
		OPER,
		QUIT,
		AWAY,
		BACK,
		WHO,
		WHOIS,
		DISCONNECT,
		RECONNECT
	}
	
	protected boolean processConsoleCommand(Client client, final String context, final ConsoleCommand cmd, final String params)
	{
		
		switch (cmd)
		{
			
			case NICK:
				if (ensureConnected(client, context, cmd))
				{
					if (params.length() > 0)
					{
						client.setNickname(context, params);
					}
					else
					{
						clientUserNick(client, client.getNickname(), client.getNickname());
					}
				}
				return true;
				
			case AWAY:
			case BACK:
				String message =
					(cmd == ConsoleCommand.AWAY && params != null && params.length() > 0) ?
						params :
						null;
				client.setAway(context, message);
				return true;
				
			case WHO:
				if (ensureConnected(client, context, cmd))
				{
					String list = "";
					for (Contact contact : client.getContacts().values())
					{
						if (!contact.nickname.equals(client.getNickname()) && contact.status != UserStatus.OFFLINE)
						{
							if (list.length() > 0)
							{
								list += ", ";
							}
							list += contact.nickname;
						}
					}
					if (list.length() < 1)
					{
						list = "(Nobody)";
					}
					clientNotification(client, context, NotificationSeverity.INFO, null, "Chatting with: " + list);
				}
				return true;
			
			case QUIT:
				client.quit(context, params);
				return true;
				
			case WHOIS:
				client.whoIs(context, params);
				return true;
				
			case OPER:
				client.authenticate(context, params, true);
				return true;
			
			case DISCONNECT:
				client.disconnect(context);
				return true;
			
			case RECONNECT:
				client.reconnect(context);
				return true;
			
			case CONNECT:
				client.connect(context, new URL(params, "dirt", 11626));
				return true;
				
			case SAY:
			case ME:
				if (ensureConnected(client, context, cmd.toString()))
				{
					final ChatMessageType type = (cmd == ConsoleCommand.ME) ? ChatMessageType.ACTION : ChatMessageType.TEXT;
					client.sendChatMessage(context, null, type, new ByteBuffer(params));
				}
				return true;
				
			case MSG:
			case MSGME:
				if (ensureConnected(client, context, cmd.toString()))
				{
					String[] tokens = TextUtil.splitQuotedHeadTail(params);
					if (tokens.length < 2 || tokens[0].length() < 1 || tokens[1].length() < 1)
					{
						clientNotification(client, context, NotificationSeverity.ERROR, cmd.toString(), "Insufficient parameters");
					}
					else
					{
						final ChatMessageType type = (cmd == ConsoleCommand.MSGME) ? ChatMessageType.ACTION : ChatMessageType.TEXT;
						client.sendChatMessage(context, tokens[0], type, new ByteBuffer(tokens[1]));
					}
				}
				return true;
				
			case MY:
				processConsoleInput(client, context, "/me 's " + params);
				return true;
				
			case HELP:
				{
					
					final SortedSet<String> cmds = new TreeSet<String>(String.CASE_INSENSITIVE_ORDER);
					
					for (ConsoleCommand entry : ConsoleCommand.class.getEnumConstants())
					{
						cmds.add(entry.toString());
					}
					
					for (String entry : getClientSupportedCommands(client))
					{
						cmds.add(entry);
					}
					
					StringBuilder buff = new StringBuilder();
					buff.append("Supported commands:");
					for (String entry : cmds)
					{
						buff.append(" ");
						buff.append(entry);
					}
					clientNotification(client, context, NotificationSeverity.INFO, cmd.toString(), buff.toString());
					
				}
				return true;
			
			default:
				return false;
				
		}
		
	}
	
}
