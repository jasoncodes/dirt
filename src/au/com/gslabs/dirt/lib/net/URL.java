package au.com.gslabs.dirt.lib.net;

/**
 * An immutable URL class
 */
public class URL
{
	
	final protected String protocol;
	final protected String username;
	final protected String hostname;
	final protected int port;
	final protected String path;
	
	final protected int defaultPort;
	
	public URL(String url, String defaultProtocol, int defaultPort)
	{
		
		this.defaultPort = defaultPort;
		
		int i = url.indexOf("://");
		if (i > -1)
		{
			protocol = url.substring(0, i).toLowerCase();
			url = url.substring(i+3);
		}
		else
		{
			protocol = defaultProtocol;
		}
		
		i = url.indexOf("/");
		if (i > -1)
		{
			path = url.substring(i);
			url = url.substring(0, i);
		}
		else
		{
			path = "/";
		}
		
		i = url.indexOf("@");
		if (i > -1)
		{
			username = url.substring(i);
			url = url.substring(0, i);
		}
		else
		{
			username = null;
		}
		
		if (url.length() > 0 && url.charAt(0) == '[')
		{
			i = url.indexOf(':', url.indexOf(']'));
		}
		else
		{
			i = url.indexOf(":");
		}
		if (i > -1)
		{
			hostname = url.substring(0, i);
			port = Integer.valueOf(url.substring(i+1));
		}
		else
		{
			hostname = url;
			port = defaultPort;
		}
		
	}
	
	public String toString()
	{
		String url = "";
		if (protocol != null && protocol.length() > 0)
		{
			url += protocol + "://";
		}
		if (username != null)
		{
			url += username + "@";
		}
		url += hostname;
		if (port != defaultPort)
		{
			url += ":" + port;
		}
		url += path;
		return url;
	}
	
	public String getProtocol()
	{
		return this.protocol;
	}
	
	public String getUsername()
	{
		return this.username;
	}
	
	public String getHostname()
	{
		return this.hostname;
	}
	
	public int getPort()
	{
		return this.port;
	}
	
}
