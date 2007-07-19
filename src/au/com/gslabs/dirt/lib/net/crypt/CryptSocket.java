package au.com.gslabs.dirt.lib.net.crypt;

import au.com.gslabs.dirt.lib.thread.*;
import java.io.*;
import au.com.gslabs.dirt.lib.net.socket.*;
import au.com.gslabs.dirt.lib.util.ByteBuffer;
import au.com.gslabs.dirt.lib.crypt.*;

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
	protected Crypt crypt;
	protected KeyPair keypair;
	protected ByteBuffer remotePublicKey;
	
	protected void clearState()
	{
		socket = null;
		worker = null;
		active = false;
		crypt = null;
		keypair = null;
		remotePublicKey = null;
	}
	
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
					
				keypair = crypt.generateKeyPair("RSA", 1024);
				
				listeners.dispatchEvent(new EventSource<CryptListener>()
					{
						public void dispatchEvent(CryptListener l)
						{
							//l.cryptMessage(keypair.getPublicKeyData());
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
			crypt = CryptFactory.getInstance();
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
		clearState();
	}
	
	public String getPeerName()
	{
		return socket.getPeerName();
	}
	
}
