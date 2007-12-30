package au.com.gslabs.dirt.lib.net.socket;

public abstract class SocketFactory
{
	
	public static SocketFactory getInstance()
	{
		return new au.com.gslabs.dirt.lib.net.socket.j2se.J2SESocketFactory();
	}
	
	public abstract Socket createSocket();
	public abstract ServerSocket createServerSocket();
	public abstract String getHostName();
	
}
