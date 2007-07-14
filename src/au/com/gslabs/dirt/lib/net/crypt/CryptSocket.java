package au.com.gslabs.dirt.lib.net.crypt;

import au.com.gslabs.dirt.lib.thread.*;
import java.io.*;
import au.com.gslabs.dirt.lib.net.socket.*;

public class CryptSocket
{
	
	protected EventListeners<CryptListener> listeners = new EventListeners<CryptListener>();
	
	public void addCryptListener(CryptListener l, Invoker i)
	{
		listeners.add(l, i);
	}
	
	public void removeCryptListener(CryptListener l)
	{
		listeners.remove(l);
	}
	
	protected Socket socket;
	protected Worker worker;
	protected boolean active;
	
	
	protected class Worker extends Thread
	{
		public String connectHost;
		public int connectPort;
		public void run()
		{
			try
			{
				
				socket = SocketFactory.getInstance().createSocket();
				socket.connect(connectHost, connectPort);
				
				listeners.dispatchEvent(new EventSource<CryptListener>()
					{
						public void dispatchEvent(CryptListener l)
						{
							l.cryptConnected();
						}
					});
				
				// TODO: begin reading from stream
				
			}
			catch (IOException ex)
			{
				onException(ex);
			}
		}
	}
	
	public void connect(String host, int port)
	{
		synchronized (this)
		{
			close();
			active = true;
			socket = null;
			worker = new Worker();
			worker.connectHost = host;
			worker.connectPort = port;
			worker.start();
		}
	}
	
	protected void onException(final IOException ex)
	{
		synchronized (this)
		{
			if (active)
			{
				active = false;
				close();
				listeners.dispatchEvent(new EventSource<CryptListener>()
					{
						public void dispatchEvent(CryptListener l)
						{
							l.cryptError(ex);
						}
					});
			}
		}
	}
	
	public void close()
	{
		synchronized (this)
		{
			if (socket != null)
			{
				try
				{
					socket.close();
				}
				catch (IOException ex)
				{
					if (active)
					{
						onException(ex);
					}
				}
				socket = null;
			}
			if (worker != null)
			{
				worker.interrupt();
				worker = null;
			}
			active = false;
		}
	}
	
	public String getPeerName()
	{
		return socket.getPeerName();
	}
	
}
