package au.com.gslabs.dirt.core.client;

import java.util.Date;
import java.util.Map;
import java.util.HashMap;
import au.com.gslabs.dirt.lib.util.Duration;
import au.com.gslabs.dirt.core.client.enums.UserStatus;

public class Contact
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
	
	public Duration getAwayDuration()
	{
		return (awayTimeLocal != null) ?
			new Duration(awayTimeLocal, new Date()) :
			null;
	}
	
}