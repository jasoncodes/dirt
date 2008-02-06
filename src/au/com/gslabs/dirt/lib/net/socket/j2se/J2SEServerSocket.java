package au.com.gslabs.dirt.lib.net.socket.j2se;

import java.io.*;
import au.com.gslabs.dirt.lib.net.socket.*;

class J2SEServerSocket implements au.com.gslabs.dirt.lib.net.socket.ServerSocket
{
	
	java.net.ServerSocket socket = null;
	
	private void init() throws IOException
	{
		if (socket == null)
		{
			socket = new java.net.ServerSocket();
		}
	}
	
	public void setReuseAddress(boolean on) throws IOException
	{
		init();
		socket.setReuseAddress(on);
	}
	
	public void bind(int port) throws IOException
	{
		init();
		socket.bind(new java.net.InetSocketAddress(port));
	}
	
	public int getLocalPort()
	{
		return socket != null ? socket.getLocalPort() : -1;
	}
	
	public Socket accept() throws IOException
	{
		init();
		return new J2SESocket(socket.accept());
	}
	
	public void close() throws IOException
	{
		init();
		socket.close();
	}
	
}
