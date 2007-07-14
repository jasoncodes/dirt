package au.com.gslabs.dirt.lib.net;

public class URL
{
	
	protected String protocol;
	protected String hostname;
	protected int port;
	protected String path;
	
	protected int defaultPort;
	
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
	
	public String getHostname()
	{
		return this.hostname;
	}
	
	public int getPort()
	{
		return this.port;
	}
	
}
