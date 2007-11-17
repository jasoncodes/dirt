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
	protected boolean nickname_last_good;
	protected URL server_url_last;
	protected ByteBuffer authseed;
	protected String server_name;
	protected String server_hostname;
	protected Timer tmrPing;
	protected TimerTask tmrTaskPing;
	protected TimerTask tmrTaskNoPong;
	protected ByteBuffer ping_data;
	protected long ping_sent;
	protected long latency;
		
	private static final long initial_ping_delay = 5000;
	private static final long ping_interval = 30000;
	private static final long ping_timeout_delay = 45000;
	
	public Client()
	{
		
		contacts = new TreeMap<String,Contact>();
		nickname_last = getDefaultNickname();
		nickname_last_good = false;
		
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
		if (name != null)
		{
			name = TextUtil.splitQuotedHeadTail(name)[0];
		}
		return name;
	}
	
	protected Contact getContact(String nickname, boolean createOrReset)
	{
		if (nickname == null || nickname.length() == 0)
		{
			return null;
		}
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
		
		final ResourceBundle resbundle = ResourceBundle.getBundle("res/strings");
		String version = resbundle.getString("version") + " " + resbundle.getString("sourceDate");
		
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
	
	protected class TimerTaskPing extends TimerTask
	{
		public void run()
		{
			try
			{
				if (isConnected())
				{
					au.com.gslabs.dirt.lib.crypt.Crypt crypt =
						au.com.gslabs.dirt.lib.crypt.CryptFactory.getInstance();
					ping_data = crypt.generateRandomBytes(8);
					ping_sent = System.currentTimeMillis();
					sendToServer(null, "PING", ping_data);
					tmrTaskNoPong = new TimerTaskNoPong();
					tmrPing.schedule(tmrTaskNoPong, ping_timeout_delay);
				}
			}
			catch (Exception ex)
			{
				onConnectionError(ex);
			}
		}
	}
	
	protected void onPong(ByteBuffer data)
	{
		if (data.equals(ping_data))
		{
			if (!tmrTaskNoPong.cancel())
			{
				onConnectionError(new Exception("Error stopping TimerTaskNoPong"));
			}
			tmrTaskPing = new TimerTaskPing();
			tmrPing.schedule(tmrTaskPing, ping_interval);
			latency = System.currentTimeMillis() - ping_sent;
			raiseStateChanged();
		}
	}
	
	protected class TimerTaskNoPong extends TimerTask
	{
		public void run()
		{
			try
			{
				if (isConnected())
				{
					resetState();
					notification(null, NotificationSeverity.ERROR, null, "Ping timeout");
					raiseStateChanged();
				}
			}
			catch (Exception ex)
			{
				onConnectionError(ex);
			}
		}
	}
	
	protected Thread shutdownHook;
	
	protected class ShutdownHook extends Thread
	{
		public void run()
		{
			shutdownHook = null; // don't want to remove ourselves if we are actually shutting down
			if (isConnected())
			{
				sendToServer(null, "QUIT", new ByteBuffer("Closing"));
			}
		}
	}
	
	protected void onConnected()
	{
		
		connected = true;
		
		tmrPing = new Timer(true);
		tmrTaskPing = new TimerTaskPing();
		tmrPing.schedule(tmrTaskPing, initial_ping_delay);
		
		if (shutdownHook == null)
		{
			shutdownHook = new ShutdownHook();
		}
		Runtime.getRuntime().addShutdownHook(shutdownHook);
		
		notification(null, NotificationSeverity.INFO, null, "Connected to " + socket.getPeerName());
		sendToServer(null, "USERDETAILS", new ByteBuffer(getUserDetails()));
		sendToServer(null, "USERAGENT", new ByteBuffer(getUserAgent()));
		
		raiseStateChanged();
		
	}
	
	protected void onDisconnected()
	{
		
		Contact contact = getContact(getNickname());
		boolean clean =
			(getNickname() != null) && (contact != null) &&
			(contact.status == UserStatus.OFFLINE) &&
			(contact.getPartMessage() != null) && (contact.getPartMessage().startsWith("Quit"));
		
		resetState();
		
		if (clean)
		{
			notification(null, NotificationSeverity.INFO, null, "Disconnected");
		}
		else
		{
			notification(null, NotificationSeverity.ERROR, null, "Connection lost");
		}
		
		raiseStateChanged();
		
	}
	
	public void disconnect(String context)
	{
		disconnect(context, 0);
	}
	
	public void disconnect(String context, long pendingSendFlushWaitMilliseconds)
	{
		if (ensureConnected(context, "DISCONNECT"))
		{
			try
			{
				long stopTime = System.currentTimeMillis() + pendingSendFlushWaitMilliseconds;
				while (socket != null && socket.isPendingSend() && System.currentTimeMillis() < stopTime)
				{
					Thread.sleep(250);
				}
			}
			catch (InterruptedException ex)
			{
			}
			resetState();
			notification(null, NotificationSeverity.INFO, null, "Disconnected");
			raiseStateChanged();
		}
	}
	
	public void connect(String context, URL url)
	{
		if (url == null)
		{
			if (server_url_last == null)
			{
				notification(context, NotificationSeverity.ERROR, "RECONNECT", "No previous connection");
				return;
			}
			else
			{
				url = server_url_last;
			}
		}
		resetState();
		notification(context, NotificationSeverity.INFO, null, "Connecting to " + url);
		if (!url.getProtocol().equals("dirt"))
		{
			notification(context, NotificationSeverity.ERROR, "CONNECT", "Unknown protocol: " + url.getProtocol());
		}
		else
		{
			server_url_last = url;
			if (url.getUsername() != null)
			{
				nickname_last = url.getUsername();
			}
			server_hostname = url.getHostname();
			socket.connect(url.getHostname(), url.getPort());
		}
	}
	
	public void reconnect(String context)
	{
		connect(context, null);
	}
	
	protected void resetState()
	{
		
		connected = false;
		nickname_current = null;
		authseed = null;
		server_name = null;
		server_hostname = null;
		latency = -1;
		socket.close();
		
		if (tmrPing != null)
		{
			tmrPing.cancel();
			tmrPing = null;
		}
		
		if (shutdownHook != null)
		{
			Runtime.getRuntime().removeShutdownHook(shutdownHook);
		}
		
		final ArrayList<Contact> modifiedContacts = new ArrayList<Contact>(this.contacts.size());
		for (Contact contact : Client.this.contacts.values())
		{
			if (contact.status != UserStatus.OFFLINE)
			{
				contact.status = UserStatus.OFFLINE;
				contact.partMessage = null;
				contact.partTimeLocal = new Date();
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
	
	public String getServerName()
	{
		return server_name;
	}
	
	public String getServerHostname()
	{
		return server_hostname;
	}
	
	public Duration getLatency()
	{
		return (latency > -1) ? new Duration(latency) : null;
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
	
	protected void onNewNickname(String nick_new)
	{
		this.nickname_current = nick_new;
		this.nickname_last = nick_new;
		this.nickname_last_good = true;
		raiseStateChanged();
	}
	
	protected enum ServerCommand
	{
		PING,
		PONG,
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
									if (contact.nickname.equals(getNickname()))
									{
										l.clientStateChanged(Client.this);
									}
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
							onNewNickname(tokens.get(0).toString());
						}
						else
						{
							final String nick_old = tokens.get(0).toString();
							final String nick_new = tokens.get(1).toString();
							final Contact contact = getContact(nick_old);
							contacts.remove(nick_old);
							contact.nickname = nick_new;
							contacts.put(nick_new, contact);
							if (nick_old.equals(nickname_current))
							{
								onNewNickname(nick_new);
							}
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
								l.clientNeedNickname(Client.this, nickname_last, nickname_last_good);
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
				sendToServer(context, "PONG", params);
				return true;
			
			case PONG:
				onPong(params);
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
					ArrayList<ByteBuffer> tokens = ByteConvert.tokenizeNull(params, 2);
					final String nick = tokens.get(0).toString();
					final String message = tokens.get(1).toString();
					listeners.dispatchEvent(new EventSource<ClientListener>()
						{
							public void dispatchEvent(ClientListener l)
							{
								if (visibility == ChatMessageVisibility.PUBLIC &&
								    nick.equals(getNickname()))
								{
									// server does not double echo our own public messages
									l.clientChatMessage(Client.this, context, nick, message, MessageDirection.OUTBOUND, type, visibility);
								}
								l.clientChatMessage(Client.this, context, nick, message, MessageDirection.INBOUND, type, visibility);
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
						nickname_last_good = false;
						listeners.dispatchEvent(new EventSource<ClientListener>()
							{
								public void dispatchEvent(ClientListener l)
								{
									l.clientNeedNickname(Client.this, nickname_last, nickname_last_good);
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
	
	/**
	 * @param nickname Nick to send message to. null for a public message
	 */
	public void sendChatMessage(final String context, final String nickname, final ChatMessageType type, final ByteBuffer message)
	{
		if (ensureConnected(context, null))
		{
			if (nickname == null)
			{
				String srvCmd = (type == ChatMessageType.ACTION) ? "PUBACTION" : "PUBMSG";
				sendToServer(context, srvCmd, message);
			}
			else
			{
				String srvCmd = (type == ChatMessageType.ACTION) ? "PRIVACTION" : "PRIVMSG";
				ByteBuffer data = new ByteBuffer(nickname.length()+1+message.length());
				data.append(nickname);
				data.append((byte)0);
				data.append(message);
				sendToServer(context, srvCmd, data);
			}
		}
	}
	
	/**
	 * @param message Away message. null to cancel away
	 */
	public void setAway(String context, String message)
	{
		if (ensureConnected(context, "AWAY"))
		{
			String serverCmd = (message != null && message.length() > 0) ? "AWAY" : "BACK";
			sendToServer(context, serverCmd, new ByteBuffer(message));
		}
	}
	
	public void whoIs(String context, String nickname)
	{
		if (ensureConnected(context, "WHOIS"))
		{
			sendToServer(context, "WHOIS", new ByteBuffer(nickname));
		}
	}
	
	public void quit(String context, String message)
	{
		if (ensureConnected(context, "QUIT"))
		{
			sendToServer(context, "QUIT", new ByteBuffer(message));
		}
	}
	
	public boolean ensureConnected(String context, String cmdString)
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
