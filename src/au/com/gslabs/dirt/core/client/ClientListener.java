package au.com.gslabs.dirt.core.client;

import java.util.ArrayList;
import java.util.Date;
import java.util.HashMap;
import au.com.gslabs.dirt.lib.util.ByteBuffer;
import au.com.gslabs.dirt.core.client.enums.*;

public interface ClientListener extends java.util.EventListener
{
	
	public String[] getClientSupportedCommands(Client source);
	public boolean clientPreprocessConsoleInput(Client source, String context, String cmd, String params);
	public String getClientExtraVersionInfo(Client source);
	public void clientNotification(Client source, String context, NotificationSeverity severity, String type, String message);
	public void clientStateChanged(Client source);
	public void clientChatMessage(Client source, String context, String nick, String text, MessageDirection direction, ChatMessageType type, ChatMessageVisibility visibility);
	public void clientCTCP(Client source, String context, String nick, String type, ByteBuffer data, MessageDirection direction, CTCPStage stage);
	public void clientUserListReceived(Client source, ArrayList<String> nicklist);
	public void clientUserJoin(Client source, String nick, String details);
	public void clientUserPart(Client source, String nick, String details, String message);
	public void clientUserNick(Client source, String nick, String new_nick);
	public void clientUserStatus(Client source, String nick, String message, Date away_time, long away_duration_secs, long previous_away_duration_secs, String previous_away_message);
	public void clientUserWhois(Client source, String context, HashMap<String,ByteBuffer> details);
	
}