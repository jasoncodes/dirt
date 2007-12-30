package au.com.gslabs.dirt.core.server;

import au.com.gslabs.dirt.lib.net.socket.*;
import au.com.gslabs.dirt.lib.net.crypt.*;
import au.com.gslabs.dirt.lib.thread.SameThreadInvoker;
import au.com.gslabs.dirt.lib.util.ByteBuffer;
import java.io.IOException;

public class Server
{
	
	protected ServerSocket listen;
	
	public Server() throws IOException
	{
		
		System.out.println("Creating listening socket");
		listen = SocketFactory.getInstance().createServerSocket();
		listen.setReuseAddress(true);
		listen.bind(11626);
		
		System.out.println("Waiting for connection");
		Socket peer = listen.accept();
		System.out.println("Got connection from " + peer.getPeerName());
		
		System.out.println("Creating crypt socket");
		CryptSocket socket = new CryptSocket();
		socket.addCryptListener(new CryptListener()
			{
				
				public void cryptError(IOException ex)
				{
					System.out.println("Socket error: " + ex);
					System.exit(1);
				}
				
				public void cryptConnected()
				{
					System.out.println("Socket connected");
				}
				
				public void cryptClosed()
				{
					System.out.println("Socket closed");
					System.exit(0);
				}
				
				public void cryptMessage(ByteBuffer data)
				{
					System.out.println("Got message:\n" + data.toHexString(true));
				}
				
			}, new SameThreadInvoker());
		
		System.out.println("Attaching socket peer");
		socket.attach(peer, true);
		
		System.out.println("Waiting forever on main thread");
		while (true)
		{
			try
			{
				Thread.sleep(10000);
			}
			catch (InterruptedException ex)
			{
			}
		}
		
	}
	
	public static void main(String[] args) throws IOException
	{
		new Server();
	}
	
}
