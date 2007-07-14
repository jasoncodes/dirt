package au.com.gslabs.dirt.lib.net.socket.j2se;

import au.com.gslabs.dirt.lib.net.socket.*;

public class J2SESocketFactory extends SocketFactory
{
	
	public Socket createSocket()
	{
		return new J2SESocket();
	}
	
}