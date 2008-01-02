package au.com.gslabs.dirt.core.client;

import java.util.Date;
import java.util.Map;
import java.util.HashMap;
import java.util.Collections;
import au.com.gslabs.dirt.lib.util.Duration;
import au.com.gslabs.dirt.lib.util.TextUtil;
import au.com.gslabs.dirt.core.client.enums.UserStatus;

public class Contact implements Comparable<Contact>
{
	
	UserStatus status;
	String nickname;
	String hostname;
	String partMessage;
	Date lastWhoIs;
	Map<String,String> other;
	String detailString;
	boolean isAdmin;
	long idleMilliseconds;
	long latencyMilliseconds;
	Date joinTimeServer;
	Date joinTimeLocal;
	Date partTimeLocal;
	String userAgent;
	String awayMessage;
	Date awayTimeServer;
	Date awayTimeLocal;
	
	void reset()
	{
		this.status = UserStatus.OFFLINE;
		this.nickname = null;
		this.hostname = null;
		this.partMessage = null;
		this.lastWhoIs = null;
		this.other = null;
		this.detailString = null;
		this.isAdmin = false;
		this.idleMilliseconds = 0;
		this.latencyMilliseconds = 0;
		this.joinTimeServer = null;
		this.joinTimeLocal = null;
		this.partTimeLocal = null;
		this.userAgent = null;
		this.awayMessage = null;
		this.awayTimeServer = null;
		this.awayTimeLocal = null;
	}
	
	Contact()
	{
		reset();
	}
	
	public int compareTo(Contact other)
	{
		
		if (this == other)
		{
			return 0;
		}
		
		int thisStatusOrder = (this.status != UserStatus.OFFLINE) ? 1 : 0;
		int otherStatusOrder = (other.status != UserStatus.OFFLINE) ? 1 : 0;
		
		if (thisStatusOrder != otherStatusOrder)
		{
			return thisStatusOrder < otherStatusOrder ? -1 : 1;
		}
		
		return this.nickname.compareToIgnoreCase(other.nickname);
		
	}
	
	public boolean equals(Object other)
	{
		
		if (this == other)
		{
			return true;
		}
		
		if (!(other instanceof Contact))
		{
			return false;
		}
		
		return this.compareTo((Contact)other) == 0;
		
	}
	
	public String toString()
	{
		StringBuilder sb = new StringBuilder();
		sb.append(nickname);
		if (status != UserStatus.ONLINE)
		{
			sb.append(" (");
			sb.append(TextUtil.toTitleCase(status.toString()));
			if (status == UserStatus.AWAY && awayMessage != null && awayMessage.length() > 0)
			{
				sb.append(": ");
				sb.append(awayMessage);
			}
			sb.append(")");
		}
		return sb.toString();
	}
	
	void parseWhoisResponse(Map<String,String> data)
	{
		
		// make sure we're working with our own copy of the data
		data = new HashMap<String,String>(data);
		
		if (!nickname.equals(data.get("NICK")))
		{
			throw new RuntimeException("Expected WHOIS data for \"" + nickname + "\" not \"" + data.get("NICK")+"\"");
		}
		
		this.status = data.containsKey("AWAY") ? UserStatus.AWAY : UserStatus.ONLINE;
		this.detailString = data.get("DETAILS");
		this.hostname = data.get("HOSTNAME");
		this.isAdmin = data.containsKey("ISADMIN");
		this.idleMilliseconds = Long.parseLong(data.get("IDLE")) * 1000;
		this.latencyMilliseconds = Long.parseLong(data.get("LATENCY"));
		this.joinTimeServer = new Date(Long.parseLong(data.get("JOINTIME"))*1000);
		this.userAgent = data.get("AGENT");
		this.awayMessage = data.get("AWAY");
		this.awayTimeServer =
			data.containsKey("AWAYTIME") ?
				new Date(Long.parseLong(data.get("AWAYTIME"))*1000) :
				null;
		this.awayTimeLocal =
			data.containsKey("AWAYTIMEDIFF") ?
				new Date(System.currentTimeMillis()-Long.parseLong(data.get("AWAYTIMEDIFF"))*1000) :
				null;
		
		// save anything we we don't recognise
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
		this.other = data;
		
		this.lastWhoIs = new Date();
		
	}
	
	public String getNickname()
	{
		return nickname;
	}
	
	public UserStatus getStatus()
	{
		return status;
	}
	
	public String getAwayMessage()
	{
		return awayMessage;
	}
	
	public Duration getAwayDuration()
	{
		return (awayTimeLocal != null) ?
			new Duration(awayTimeLocal, new Date()) :
			null;
	}
	
	public Date getPartTimeLocal()
	{
		return partTimeLocal;
	}
	
	public Duration getOfflineDuration()
	{
		if (status == UserStatus.OFFLINE && partTimeLocal != null)
		{
			return new Duration(partTimeLocal, new Date());
		}
		else
		{
			return null;
		}
	}
	
	public String getPartMessage()
	{
		return this.partMessage;
	}
	
	public String getDetailString()
	{
		return this.detailString;
	}
	
	public String getHostname()
	{
		return this.hostname;
	}
	
	public boolean isAdmin()
	{
		return this.isAdmin;
	}
	
	public String getUserAgent()
	{
		return this.userAgent;
	}
	
	public Duration getIdle()
	{
		return new Duration(this.idleMilliseconds);
	}
	
	public Duration getLatency()
	{
		return new Duration(this.latencyMilliseconds);
	}
	
	public Date getJoinTimeServer()
	{
		return joinTimeServer;
	}
	
	public Map<String,String> getOther()
	{
		return Collections.unmodifiableMap(other);
	}
	
}

