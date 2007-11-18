package au.com.gslabs.dirt.lib.net.socket.j2se;

import java.net.*;
import java.io.*;

class J2SESocket implements au.com.gslabs.dirt.lib.net.socket.Socket
{
	
	protected Socket socket = null;
	
	public J2SESocket()
	{
	}
	
	public void connect(String host, int port) throws IOException
	{
		
		// prefer AAAA records when available
		try
		{
			System.setProperty("java.net.preferIPv6Addresses", "true");
		}
		catch (SecurityException ex)
		{
		}
		
		// get a list of all the addresses for the hostname supplied
		final InetAddress[] addresses;
		if (host != null)
		{
			addresses = InetAddress.getAllByName(host);
		}
		else
		{
			// Can't trust the JRE to get both IPv6 and IPv4 addresses for localhost
			addresses = new InetAddress[2];
			try
			{
				addresses[0] = InetAddress.getByName("::1");
			}
			catch (Exception ex)
			{
			}
			addresses[1] = InetAddress.getByName("127.0.0.1");
		}
		IOException ex = null;
		
		// try connecting to each address until we get a connection
		for (InetAddress addr : addresses)
		{
			if (addr != null)
			{
				ex = null;
				try
				{
					socket = new Socket();
					socket.connect(new InetSocketAddress(addr, port));
					break;
				}
				catch (IOException thisEx)
				{
					ex = thisEx;
				}
			}
		}
		
		// the last try failed, raise an error
		if (ex != null)
		{
			throw ex;
		}
		
	}
	
	public void close() throws IOException
	{
		if (socket != null)
		{
			socket.close();
			socket = null;
		}
	}
	
	public InputStream getInputStream() throws IOException
	{
		return socket.getInputStream();
	}
	
	public OutputStream getOutputStream() throws IOException
	{
		return socket.getOutputStream();
	}
	
	public String getPeerName()
	{
		
		String name = socket.getInetAddress().getHostAddress();
		
		// make IPv6 addresses all pretty looking
		if (name.indexOf("::") < 0)
		{
			int i = name.indexOf(":0:");
			if (i > -1)
			{
				int j = i;
				while (j+3 < name.length() &&
				       name.charAt(j+0) == ':' &&
				       name.charAt(j+1) == '0' &&
				       name.charAt(j+2) == ':')
				{
					j += 2;
				}
				name = name.substring(0, i) + ":" + name.substring(j);
			}
			if (name.startsWith("0::"))
			{
				name = name.substring(1);
			}
			if (name.endsWith("::0"))
			{
				name = name.substring(0, name.length()-1);
			}
		}
		
		if (name.indexOf(":") > -1)
		{
			name = "["+name+"]";
		}
		name += ":" + socket.getPort();
		
		return name;
		
	}
	
}