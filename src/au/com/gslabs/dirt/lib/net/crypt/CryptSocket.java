package au.com.gslabs.dirt.lib.net.crypt;

import au.com.gslabs.dirt.lib.thread.*;
import java.io.*;
import au.com.gslabs.dirt.lib.net.socket.*;
import au.com.gslabs.dirt.lib.util.ByteBuffer;

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
				
				DataInputStream in = new DataInputStream(socket.getInputStream());
				while (!isInterrupted())
				{
					int packetLength = in.readUnsignedShort();
					byte data[] = new byte[packetLength];
					in.readFully(data);
					processPacket(data);
				}
				
			}
			catch (IOException ex)
			{
				onException(ex);
			}
		}
	}
	
	protected void processPacket(final byte[] bytes) throws IOException
	{
		
		if (bytes.length < 2)
		{
			throw new IOException("Packet too short");
		}
		
		listeners.dispatchEvent(new EventSource<CryptListener>()
			{
				public void dispatchEvent(CryptListener l)
				{
					l.cryptMessage(new ByteBuffer(bytes));
				}
			});
		
	}
	
	public void connect(String host, int port)
	{
		close();
		synchronized (this)
		{
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
			active = false;
			if (socket != null)
			{
				try
				{
					socket.close();
				}
				catch (IOException ex)
				{
				}
				socket = null;
			}
		}
		if (worker != null)
		{
			worker.interrupt();
			try
			{
				worker.join();
			}
			catch (InterruptedException ex)
			{
			}
			worker = null;
		}
	}
	
	public String getPeerName()
	{
		return socket.getPeerName();
	}
	
}
