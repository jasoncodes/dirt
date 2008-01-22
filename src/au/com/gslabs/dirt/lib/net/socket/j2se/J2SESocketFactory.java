package au.com.gslabs.dirt.lib.net.socket.j2se;

import au.com.gslabs.dirt.lib.net.socket.*;

public class J2SESocketFactory extends SocketFactory
{
	
	public Socket createSocket()
	{
		return new J2SESocket();
	}
	
	public ServerSocket createServerSocket()
	{
		return new J2SEServerSocket();
	}
	
	public String getHostName()
	{
		try
		{
			return java.net.InetAddress.getLocalHost().getHostName();
		}
		catch (java.net.UnknownHostException ex)
		{
			return null;
		}
	}
	
}
