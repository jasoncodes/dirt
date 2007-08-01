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
	
	protected Map<String,Contact> contacts;
	protected CryptSocket socket;
	protected boolean connected;
	protected String nickname_current;
	protected String nickname_last;
	protected ByteBuffer authseed;
	protected String server_name;
	
	public Client()
	{
		contacts = new TreeMap<String,Contact>();
		nickname_last = getDefaultNickname();
		socket = new CryptSocket();
		socket.addCryptListener(new CryptListener()
			{
				public void cryptError(java.io.IOException ex)
				{
					onConnectionError(ex);
				}
				public void cryptConnected()
				{
					onConnected();
				}
				public void cryptClosed()
				{
					onDisconnected();
				}
				public void cryptMessage(ByteBuffer data)
				{
					processServerMessage(data);
				}
			}, new SameThreadInvoker());
		resetState();
	}
	
	protected String getUserDetails()
	{
		
		String username = System.getProperty("user.name");
		String hostname = au.com.gslabs.dirt.lib.net.socket.SocketFactory.getInstance().getHostName();
		String userDetails = username + "@" + hostname;
		
		String fullName = FileUtil.getMyFullName();
		if (fullName != null && !fullName.equalsIgnoreCase(username))
		{
			userDetails += " \"" + fullName + "\"";
		}
		
		String osInfo =
			System.getProperty("os.name") + " " + System.getProperty("os.version") +
			" (" + System.getProperty("os.arch") + ")";
		userDetails += " on " + osInfo;
		
		return userDetails;
		
	}
	
	public static String getDefaultNickname()
	{
		String name = FileUtil.getMyFullName();
		if (name == null || name.length() < 1)
		{
			name = System.getProperty("user.name");
		}
		return TextUtil.splitQuotedHeadTail(name)[0];
	}
	
	protected Contact getContact(String nickname, boolean createOrReset)
	{
		Contact contact = contacts.get(nickname);
		if (createOrReset)
		{
			if (contact == null)
			{
				contact = new Contact();
				contact.nickname = nickname;
				contacts.put(nickname, contact);
			}
			else
			{
				contact.reset();
				contact.nickname = nickname;
			}
		}
		return contact;
	}
	
	public Contact getContact(String nickname)
	{
		return getContact(nickname, false);
	}
	
	public Map<String,Contact> getContacts()
	{
		return Collections.unmodifiableMap(contacts);
	}
	
	protected String getUserAgent()
	{
		
		final HashSet<String> extras = new HashSet<String>();
		listeners.dispatchEvent(new EventSource<ClientListener>()
			{
				public void dispatchEvent(ClientListener l)
				{
					String extra = l.getClientExtraVersionInfo(Client.this);
					if (extra != null && extra.length() > 0)
					{
						extras.add(extra);
					}
				}
			}, true);
		
		String version = au.com.gslabs.dirt.Dirt.VERSION;
		version += " " + TextUtil.formatDateTime(FileUtil.getJarBuildDate(), false);
		
		if (extras.size() > 0)
		{
			version += " (";
			boolean pastFirst = false;
			for (String extra : extras)
			{
				if (pastFirst)
				{
					version += ", ";
				}
				version += extra;
				pastFirst = true;
			}
			version += ")";
		}
		
		return version;
		
	}
	
	protected void onConnected()
	{
		connected = true;
		notification(null, NotificationSeverity.INFO, null, "Connected to " + socket.getPeerName());
		sendToServer(null, "USERDETAILS", new ByteBuffer(getUserDetails()));
		sendToServer(null, "USERAGENT", new ByteBuffer(getUserAgent()));
		raiseStateChanged();
	}
	
	protected void onDisconnected()
	{
		resetState();
		notification(null, NotificationSeverity.ERROR, null, "Connection lost");
		raiseStateChanged();
	}
	
	public void disconnect(String context)
	{
		if (ensureConnected(context, "DISCONNECT"))
		{
			resetState();
			notification(null, NotificationSeverity.INFO, null, "Disconnected");
			raiseStateChanged();
		}
	}
	
	public void connect(String context, URL url)
	{
		resetState();
		notification(context, NotificationSeverity.INFO, null, "Connecting to " + url);
		if (!url.getProtocol().equals("dirt"))
		{
			notification(context, NotificationSeverity.ERROR, "CONNECT", "Unknown protocol: " + url.getProtocol());
		}
		else
		{
			socket.connect(url.getHostname(), url.getPort());
		}
	}
	
	protected void resetState()
	{
		
		connected = false;
		nickname_current = null;
		authseed = null;
		server_name = null;
		socket.close();
		
		final ArrayList<Contact> modifiedContacts = new ArrayList<Contact>(this.contacts.size());
		for (Contact contact : Client.this.contacts.values())
		{
			if (contact.status != UserStatus.OFFLINE)
			{
				contact.status = UserStatus.OFFLINE;
				modifiedContacts.add(contact);
			}
		}
		
		listeners.dispatchEvent(new EventSource<ClientListener>()
			{
				public void dispatchEvent(ClientListener l)
				{
					for (Contact contact : modifiedContacts)
					{
						l.clientContactUpdated(Client.this, contact);
					}
				}
			});
		
	}
	
	protected void raiseStateChanged()
	{
		listeners.dispatchEvent(new EventSource<ClientListener>()
			{
				public void dispatchEvent(ClientListener l)
				{
					l.clientStateChanged(Client.this);
				}
			});
	}
	
	protected void onConnectionError(Exception ex)
	{
		System.err.println("Connection error:");
		ex.printStackTrace();
		resetState();
		String msg = "Connection error: " + ex.toString();
		Throwable t = ex.getCause();
		while (t != null)
		{
			msg += ", " + t.toString();
			t = t.getCause();
		}
		notification(null, NotificationSeverity.ERROR, null, msg);
		raiseStateChanged();
	}
	
	public boolean isConnected()
	{
		return connected;
	}
	
	protected void processServerMessage(ByteBuffer data)
	{
		
		ArrayList<ByteBuffer> tokens = ByteConvert.tokenizeNull(data, 3);
		if (tokens.size() != 3)
		{
			throw new IllegalArgumentException("Not enough tokens in server message");
		}
		
		String context = tokens.get(0).toString();
		String cmdString = tokens.get(1).toString();
		ByteBuffer params = tokens.get(2);
		
		final ServerCommand cmd;
		try
		{
			cmd = ServerCommand.valueOf(cmdString);
		}
		catch (Exception ex)
		{
			notification(context, NotificationSeverity.ERROR, null, "Server message type unknown: " + cmdString);
			dumpMessageData(context, cmdString, params);
			return;
		}
		
		if (!processServerMessage(context, cmd, params))
		{
			notification(context, NotificationSeverity.ERROR, null, "Server message type not implemented: " + cmd);
			dumpMessageData(context, cmdString, params);
		}
		
	}
	
	protected enum ServerCommand
	{
		PING,
		INFO,
		PUBMSG,
		PUBACTION,
		PRIVMSG,
		PRIVACTION,
		PRIVMSGOK,
		PRIVACTIONOK,
		AUTHOK,
		IPLIST,
		IPSELF,
		ERROR,
		AUTHSEED,
		AUTH,
		AUTHBAD,
		JOIN,
		PART,
		NICKLIST,
		SERVERNAME,
		AWAY,
		BACK,
		WHOIS,
		NICK
	}
	
	protected boolean processServerMessage(final String context, final ServerCommand cmd, final ByteBuffer params)
	{
		
		switch (cmd)
		{
			
			case IPLIST:
			case IPSELF:
				// do not care about these messages (yet)
				return true;
			
			case AUTHSEED:
				this.authseed = params;
				return true;
				
			case AUTH:
			case AUTHBAD:
				{
					listeners.dispatchEvent(new EventSource<ClientListener>()
						{
							public void dispatchEvent(ClientListener l)
							{
								l.clientNeedAuthentication(Client.this, params.toString());
							}
						});
				}
				return true;
			
			case JOIN:
				{
					ArrayList<ByteBuffer> tokens = ByteConvert.tokenizeNull(params);
					if (ensureMinTokenCount(context, cmd.toString(), tokens, 2))
					{
						final Contact contact = getContact(tokens.get(0).toString(), true);
						contact.status = UserStatus.ONLINE;
						contact.hostname = tokens.get(1).toString();
						contact.joinTimeLocal = new Date();
						listeners.dispatchEvent(new EventSource<ClientListener>()
							{
								public void dispatchEvent(ClientListener l)
								{
									l.clientContactUpdated(Client.this, contact);
									l.clientUserJoin(Client.this, contact.nickname, contact.hostname);
								}
							});
					}
				}
				return true;
				
			case PART:
				{
					ArrayList<ByteBuffer> tokens = ByteConvert.tokenizeNull(params);
					if (ensureMinTokenCount(context, cmd.toString(), tokens, 3))
					{
						final Contact contact = getContact(tokens.get(0).toString());
						contact.status = UserStatus.OFFLINE;
						contact.hostname = tokens.get(1).toString();
						contact.partMessage = tokens.get(2).toString();
						contact.partTimeLocal = new Date();
						listeners.dispatchEvent(new EventSource<ClientListener>()
							{
								public void dispatchEvent(ClientListener l)
								{
									l.clientContactUpdated(Client.this, contact);
									l.clientUserPart(Client.this, contact.nickname, contact.hostname, contact.partMessage);
								}
							});
					}
				}
				return true;
				
			case AWAY:
			case BACK:
				{
					UserStatus status = (cmd == ServerCommand.AWAY) ? UserStatus.AWAY : UserStatus.ONLINE;
					ArrayList<ByteBuffer> tokens = ByteConvert.tokenizeNull(params);
					if (ensureMinTokenCount(context, cmd.toString(), tokens, 4))
					{
						
						String nickname = tokens.get(0).toString();
						String message = tokens.get(1).toString();
						Date awayTimeServer = new Date(Long.parseLong(tokens.get(2).toString())*1000);
						Date awayTimeLocal = new Date(System.currentTimeMillis()-Long.parseLong(tokens.get(3).toString())*1000);
						
						final Contact contact = getContact(nickname);
						final Duration previous_duration;
						final String previous_message;
						if (contact.status == UserStatus.AWAY)
						{
							previous_duration = contact.getAwayDuration();
							previous_message = contact.awayMessage;
						}
						else
						{
							previous_duration = null;
							previous_message = null;
						}
						contact.status = status;
						contact.awayMessage = message;
						contact.awayTimeServer = awayTimeServer;
						contact.awayTimeLocal = awayTimeLocal;
						
						listeners.dispatchEvent(new EventSource<ClientListener>()
							{
								public void dispatchEvent(ClientListener l)
								{
									l.clientContactUpdated(Client.this, contact);
									l.clientUserStatus(Client.this, contact.nickname, contact.status, contact.awayMessage, contact.awayTimeServer, contact.getAwayDuration(), previous_duration, previous_message);
								}
							});
						
					}
				}
				return true;
				
			case SERVERNAME:
				{
					ArrayList<ByteBuffer> tokens = ByteConvert.tokenizeNull(params);
					this.server_name = (tokens.size() > 0) ? tokens.get(0).toString() : null;
					raiseStateChanged();
				}
				return true;
				
			case NICKLIST:
				{
					ArrayList<ByteBuffer> tokens = ByteConvert.tokenizeNull(params);
					ArrayList<String> nicks = new ArrayList<String>(tokens.size());
					for (ByteBuffer nickBytes : ByteConvert.tokenizeNull(params))
					{
						if (nickBytes.length() > 0)
						{
							String nick = nickBytes.toString();
							Contact contact = getContact(nick, true);
							contact.status = UserStatus.ONLINE;
							nicks.add(nick);
						}
					}
					final String[] nickArray = (String[])nicks.toArray(new String[0]);
					listeners.dispatchEvent(new EventSource<ClientListener>()
						{
							public void dispatchEvent(ClientListener l)
							{
								for (Contact contact : Client.this.contacts.values())
								{
									l.clientContactUpdated(Client.this, contact);
								}
								l.clientUserListReceived(Client.this, nickArray);
							}
						});
				}
				return true;
				
			case NICK:
				{
					ArrayList<ByteBuffer> tokens = ByteConvert.tokenizeNull(params);
					if (ensureMinTokenCount(context, cmd.toString(), tokens, 1))
					{
						if (tokens.size() < 2 || tokens.get(1).length() < 1)
						{
							this.nickname_current = tokens.get(0).toString();
						}
						else
						{
							final String nick_old = tokens.get(0).toString();
							final String nick_new = tokens.get(1).toString();
							final Contact contact = getContact(nick_old);
							contacts.remove(nick_old);
							contact.nickname = nick_new;
							contacts.put(nick_new, contact);
							listeners.dispatchEvent(new EventSource<ClientListener>()
								{
									public void dispatchEvent(ClientListener l)
									{
										l.clientContactUpdated(Client.this, contact);
										l.clientUserNick(Client.this, nick_old, nick_new);
									}
								});
						}
					}
				}
				return true;
				
			case WHOIS:
				{
					Map<String,String> data = ByteConvert.toMap(params);
					final String nick = data.get("NICK");
					final Contact contact = getContact(nick);
					contact.parseWhoisResponse(data);
					listeners.dispatchEvent(new EventSource<ClientListener>()
						{
							public void dispatchEvent(ClientListener l)
							{
								l.clientContactUpdated(Client.this, contact);
								l.clientUserWhois(Client.this, context, nick);
							}
						});
				}
				return true;
			
			case AUTHOK:
				{
					if (params.length() > 0)
					{
						notification(context, NotificationSeverity.INFO, null, params.toString());
					}
					raiseStateChanged();
					listeners.dispatchEvent(new EventSource<ClientListener>()
						{
							public void dispatchEvent(ClientListener l)
							{
								l.clientNeedNickname(Client.this, nickname_last);
							}
						});
				}
				return true;
				
			case INFO:
				{
					notification(context, NotificationSeverity.INFO, null, params.toString());
				}
				return true;
			
			case PING:
				{
					sendToServer(context, "PONG", params);
				}
				return true;
				
			case PUBMSG:
			case PUBACTION:
			case PRIVMSG:
			case PRIVACTION:
				{
					final ChatMessageType type =
						(cmd == ServerCommand.PUBACTION || cmd == ServerCommand.PRIVACTION) ?
							ChatMessageType.ACTION :
							ChatMessageType.TEXT;
					final ChatMessageVisibility visibility =
						(cmd == ServerCommand.PRIVMSG || cmd == ServerCommand.PRIVACTION) ?
							ChatMessageVisibility.PRIVATE :
							ChatMessageVisibility.PUBLIC;
					final ArrayList<ByteBuffer> tokens = ByteConvert.tokenizeNull(params, 2);
					listeners.dispatchEvent(new EventSource<ClientListener>()
						{
							public void dispatchEvent(ClientListener l)
							{
								l.clientChatMessage(Client.this, context, tokens.get(0).toString(), tokens.get(1).toString(), MessageDirection.INBOUND, type, visibility);
							}
						});
				}
				return true;
				
			case PRIVMSGOK:
			case PRIVACTIONOK:
				{
					final ChatMessageType type =
						(cmd == ServerCommand.PRIVACTIONOK) ?
							ChatMessageType.ACTION :
							ChatMessageType.TEXT;
					final ArrayList<ByteBuffer> tokens = ByteConvert.tokenizeNull(params, 2);
					listeners.dispatchEvent(new EventSource<ClientListener>()
						{
							public void dispatchEvent(ClientListener l)
							{
								l.clientChatMessage(Client.this, context, tokens.get(0).toString(), tokens.get(1).toString(), MessageDirection.OUTBOUND, type, ChatMessageVisibility.PRIVATE);
							}
						});
					
				}
				return true;
				
			case ERROR:
				{
					final ArrayList<ByteBuffer> tokens = ByteConvert.tokenizeNull(params, 2);
					notification(context, NotificationSeverity.ERROR, tokens.get(0).toString(), tokens.get(1).toString());
					if (tokens.size() > 1 && tokens.get(0).toString().equalsIgnoreCase("NICK"))
					{
						listeners.dispatchEvent(new EventSource<ClientListener>()
							{
								public void dispatchEvent(ClientListener l)
								{
									l.clientNeedNickname(Client.this, nickname_last);
								}
							});
					}
				}
				return true;
			
			default:
				return false;
				
		}
		
	}
	
	protected void dumpMessageData(String context, String cmdString, ByteBuffer params)
	{
		notification(context, NotificationSeverity.DEBUG, null,
			"Message data:\n\tContext: " + context + "\n\tCommand: "+cmdString+
			"\n\tParams:\n\t\t"+params.toHexString(true).replace("\n","\n\t\t"));
	}
	
	protected void sendToServer(String context, String command, ByteBuffer params)
	{
		if (context == null) context = "";
		if (command == null) command = "";
		if (params == null) params = new ByteBuffer();
		if (context.indexOf(0) > -1 || command.indexOf(0) > -1)
		{
			throw new IllegalArgumentException("Context and command cannot contain null characters");
		}
		ByteBuffer data = new ByteBuffer(context.length()+command.length()+params.length()+2);
		data.append(context);
		data.append((byte)0x00);
		data.append(command);
		data.append((byte)0x00);
		data.append(params);
		socket.send(data);
	}
	
	public void processConsoleInput(String context, String[] lines)
	{
		for (String line : lines)
		{
			processConsoleInput(context, line);
		}
	}
	
	public void authenticate(String context, String authentication, boolean forAdmin)
	{
		String cmd = forAdmin ? "OPER" : "AUTH";
		if (ensureConnected(context, cmd))
		{
			try
			{
				au.com.gslabs.dirt.lib.crypt.Crypt crypt =
					au.com.gslabs.dirt.lib.crypt.CryptFactory.getInstance();
				ByteBuffer digest = crypt.generateMacDigest("MD5MAC", authseed, new ByteBuffer(authentication));
				sendToServer(context, cmd, digest);
			}
			catch (Exception ex)
			{
				onConnectionError(ex);
			}
		}
	}
	
	public boolean isConsoleInputHistorySafe(String line)
	{
		return !(TextUtil.splitQuotedHeadTail(line)[0].equalsIgnoreCase("/oper"));
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
	
	protected boolean ensureConnected(String context, ConsoleCommand cmd)
	{
		return ensureConnected(context, (cmd != null && cmd != ConsoleCommand.SAY) ? cmd.toString() : null);
	}
	
	protected boolean ensureConnected(String context, String cmdString)
	{
		if (isConnected())
		{
			return true;
		}
		else
		{
			notification(context, NotificationSeverity.ERROR, cmdString, "Not connected");
			return false;
		}
	}
	
	protected boolean ensureMinTokenCount(String context, String cmdString, ArrayList<ByteBuffer> tokens, int countMin)
	{
		if (tokens != null && tokens.size() >= countMin)
		{
			return true;
		}
		else
		{
			notification(context, NotificationSeverity.ERROR, null, "Invalid "+cmdString+" response from server");
			return false;
		}
	}
	
	protected enum ConsoleCommand
	{
		SAY,
		ME,
		MY,
		HELP,
		CONNECT,
		RAW,
		NICK,
		MSG,
		MSGME,
		OPER,
		QUIT,
		AWAY,
		BACK,
		WHO,
		WHOIS,
		DISCONNECT
	}
	
	protected boolean processConsoleCommand(final String context, final ConsoleCommand cmd, final String params)
	{
		
		switch (cmd)
		{
			
			case RAW:
				if (ensureConnected(context, cmd))
				{
					int i = params.indexOf(' ');
					if (i > 0)
					{
						sendToServer(context, params.substring(0, i), new ByteBuffer(params.substring(i+1)));
					}
					else
					{
						sendToServer(context, params, null);
					}
				}
				return true;
				
			case NICK:
				if (ensureConnected(context, cmd))
				{
					if (params.length() > 0)
					{
						setNickname(context, params);
					}
					else
					{
						listeners.dispatchEvent(new EventSource<ClientListener>()
							{
								public void dispatchEvent(ClientListener l)
								{
									l.clientUserNick(Client.this, getNickname(), getNickname());
								}
							});
					}
				}
				return true;
				
			case AWAY:
			case BACK:
				if (ensureConnected(context, cmd))
				{
					ByteBuffer serverParams = new ByteBuffer(
						(cmd == ConsoleCommand.AWAY) ?
							params :
							"");
					String serverCmd = params.length() > 0 ? "AWAY" : "BACK";
					sendToServer(context, serverCmd, serverParams);
				}
				return true;
				
			case WHO:
				if (ensureConnected(context, cmd))
				{
					String list = "";
					for (Contact contact : contacts.values())
					{
						if (!contact.nickname.equals(getNickname()) && contact.status != UserStatus.OFFLINE)
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
					notification(context, NotificationSeverity.INFO, null, "Chatting with: " + list);
				}
				return true;
			
			case QUIT:
				if (ensureConnected(context, cmd))
				{
					sendToServer(context, "QUIT", new ByteBuffer(params));
				}
				return true;
				
			case WHOIS:
				if (ensureConnected(context, cmd))
				{
					sendToServer(context, "WHOIS", new ByteBuffer(params));
				}
				return true;
				
			case OPER:
				if (ensureConnected(context, cmd))
				{
					authenticate(context, params, true);
				}
				return true;
			
			case DISCONNECT:
				disconnect(context);
				return true;
			
			case CONNECT:
				connect(context, new URL(params, "dirt", 11626));
				return true;
				
			case SAY:
			case ME:
				if (!isConnected())
				{
					notification(context, NotificationSeverity.ERROR, cmd.toString(), "Not connected");
				}
				else
				{
					final ChatMessageType type = (cmd == ConsoleCommand.ME) ? ChatMessageType.ACTION : ChatMessageType.TEXT;
					String srvCmd = (cmd == ConsoleCommand.ME) ? "PUBACTION" : "PUBMSG";
					sendToServer(context, srvCmd, new ByteBuffer(params));
					listeners.dispatchEvent(new EventSource<ClientListener>()
						{
							public void dispatchEvent(ClientListener l)
							{
								l.clientChatMessage(Client.this, context, getNickname(), params, MessageDirection.OUTBOUND, type, ChatMessageVisibility.PUBLIC);
							}
						});
				}
				return true;
				
			case MSG:
			case MSGME:
				if (!isConnected())
				{
					notification(context, NotificationSeverity.ERROR, cmd.toString(), "Not connected");
				}
				else
				{
					String[] tokens = TextUtil.splitQuotedHeadTail(params);
					if (tokens.length < 2 || tokens[0].length() < 1 || tokens[1].length() < 1)
					{
						notification(context, NotificationSeverity.ERROR, cmd.toString(), "Insufficient parameters");
					}
					else
					{
						final ChatMessageType type = (cmd == ConsoleCommand.MSGME) ? ChatMessageType.ACTION : ChatMessageType.TEXT;
						String srvCmd = (cmd == ConsoleCommand.MSGME) ? "PRIVACTION" : "PRIVMSG";
						ByteBuffer data = new ByteBuffer(params.length());
						data.append(tokens[0]);
						data.append((byte)0);
						data.append(tokens[1]);
						sendToServer(context, srvCmd, data);
					}
				}
				return true;
				
			case MY:
				processConsoleInput(context, "/me 's " + params);
				return true;
				
			case HELP:
				{
					
					final SortedSet<String> cmds = new TreeSet<String>(String.CASE_INSENSITIVE_ORDER);
					
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
					
				}
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
		return nickname_current;
	}
	
	public void setNickname(String context, String nickname)
	{
		nickname_last = nickname;
		sendToServer(context, "NICK", new ByteBuffer(nickname));
	}
	
}
