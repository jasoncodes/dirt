package au.com.gslabs.dirt.core.client;

import java.util.Date;
import java.util.Map;
import au.com.gslabs.dirt.core.client.enums.*;
import au.com.gslabs.dirt.lib.util.ByteBuffer;
import au.com.gslabs.dirt.lib.util.Duration;

public interface ClientListener extends java.util.EventListener
{
	
	public String getClientExtraVersionInfo(Client source);
	
	public void clientNotification(Client source, String context, NotificationSeverity severity, String type, String message);
	public void clientStateChanged(Client source);
	public void clientChatMessage(Client source, String context, String nick, ByteBuffer message, MessageDirection direction, ChatMessageType type, ChatMessageVisibility visibility);
	public void clientCTCP(Client source, String context, String nick, String type, ByteBuffer data, MessageDirection direction, CTCPStage stage);
	
	public void clientContactUpdated(Client source, Contact contact);
	public void clientUserListReceived(Client source, String[] nicklist);
	public void clientUserJoin(Client source, String nick, String details);
	public void clientUserPart(Client source, String nick, String details, String message);
	public void clientUserNick(Client source, String nick_old, String nick_new);
	public void clientUserStatus(Client source, String nick, UserStatus status, String message, Date away_start, Duration away_duration, Duration previous_away_duration, String previous_away_message);
	public void clientUserWhois(Client source, String context, String nickname);
	
	public void clientNeedNickname(Client source, String defaultNick);
	public void clientNeedAuthentication(Client source, String prompt);
	
}
