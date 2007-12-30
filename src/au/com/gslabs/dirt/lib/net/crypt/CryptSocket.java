package au.com.gslabs.dirt.lib.net.crypt;

import java.io.*;
import java.util.*;
import au.com.gslabs.dirt.lib.thread.*;
import au.com.gslabs.dirt.lib.net.socket.*;
import au.com.gslabs.dirt.lib.util.*;
import au.com.gslabs.dirt.lib.crypt.*;

public class CryptSocket
{
	
	protected static final String transformationPublic = "RSA/ECB/OAEPWithSHA-1AndMGF1Padding";
	protected static final String transformationBlock = "AES/ECB/NoPadding";
	protected static final int sendBlockSize = 4096;
	protected static final int maxBlockKeyAgeBytes = 1024 * 128; // 128 KiB
	protected static final int maxBlockKeyAgeSeconds = 300; // 5 minutes
	
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
	protected boolean socketOwned;
	protected InputThread threadInput;
	protected OutputThread threadOutput;
	protected boolean active;
	protected Crypt crypt;
	protected KeyPair keypair;
	protected ByteBuffer remotePublicKey;
	protected ByteBuffer remoteBlockKey;
	protected ByteBuffer localBlockKey;
	protected Queue<ByteBuffer> bufferSendUser;
	protected ByteBuffer bufferSendRaw;
	protected ZlibDeflater deflater;
	protected ZlibInflator inflator;
	protected long blockKeyTTL_Bytes;
	protected long blockKeyExpire_Time;
	
	protected void clearState()
	{
		socket = null;
		socketOwned = false;
		threadInput = null;
		threadOutput = null;
		active = false;
		crypt = null;
		keypair = null;
		remotePublicKey = null;
		remoteBlockKey = null;
		localBlockKey = null;
		bufferSendUser = null;
		bufferSendRaw = null;
		deflater = null;
		inflator = null;
		blockKeyTTL_Bytes = 0;
		blockKeyExpire_Time = 0;
	}
	
	protected enum MessageType
	{
		
		PUBLIC_KEY(0),
		BLOCK_KEY(1);
		
		private int value;
		
		MessageType(int value)
		{
			this.value = value;
		}
		
		public int getValue()
		{
			return this.value;
		}
		
	}
	
	protected class InputThread extends Thread
	{
		
		public String connectHost;
		public int connectPort;
		
		public InputThread()
		{
			super("CryptSocket.InputThread");
		}
		
		public void run()
		{
			try
			{
				
				if (connectHost != null)
				{
					socket = SocketFactory.getInstance().createSocket();
					socketOwned = true;
					socket.connect(connectHost, connectPort);
				}
				
				if (isInterrupted()) return;
				
				deflater = crypt.createZlibDeflater(9);
				inflator = crypt.createZlibInflator();
					
				bufferSendUser = new LinkedList<ByteBuffer>();
				bufferSendRaw = new ByteBuffer();
				
				threadOutput = new OutputThread();
				threadOutput.setDaemon(true);
				threadOutput.start();
				
				keypair = crypt.generateKeyPair(transformationPublic, 1024);
				sendPublicKey();
				
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
					processPacket(new ByteBuffer(data));
				}
				
			}
			catch (IOException ex)
			{
				onException(ex);
			}
			catch (RuntimeException ex)
			{
				IOException ex2 = new IOException(ex.toString());
				ex2.initCause(ex);
				onException(ex2);
			}
		}
	}
	
	protected class OutputThread extends Thread
	{
		
		public OutputThread()
		{
			super("CryptSocket.OutputThread");
		}
		
		public void run()
		{
			try
			{
				OutputStream output = socket.getOutputStream();
				while (!isInterrupted())
				{
					
					byte[] block;
					
					synchronized (threadOutput)
					{
						if (bufferSendRaw.length() == 0)
						{
							try
							{
								threadOutput.wait();
							}
							catch (InterruptedException iex)
							{
								return;
							}
						}
						if (bufferSendRaw.length() > sendBlockSize)
						{
							block = bufferSendRaw.extract(0, sendBlockSize).getBytes();
							bufferSendRaw = bufferSendRaw.extract(sendBlockSize);
						}
						else
						{
							block = bufferSendRaw.getBytes();
							bufferSendRaw.clear();
						}
					}
					
					if (!isInterrupted())
					{
						output.write(block);
					}
					
				}
			}
			catch (IOException ex)
			{
				onException(ex);
			}
		}
		
	}
	
	protected void outputRaw(ByteBuffer data)
	{
		synchronized (threadOutput)
		{
			bufferSendRaw.append(ByteConvert.fromU16(data.length()));
			bufferSendRaw.append(data);
			threadOutput.notify();
		}
	}
	
	protected void sendPublicKey()
	{
		ByteBuffer key = keypair.getPublicKey();
		ByteBuffer data = new ByteBuffer(key.length() + 4);
		data.append(ByteConvert.fromU16(0));
		data.append(ByteConvert.fromU16(MessageType.PUBLIC_KEY.getValue()));
		data.append(key);
		outputRaw(data);
	}
	
	protected void sendBlockKey() throws CryptException
	{
		ByteBuffer data = new ByteBuffer(localBlockKey.length() + 4);
		data.append(ByteConvert.fromU16(0));
		data.append(ByteConvert.fromU16(MessageType.BLOCK_KEY.getValue()));
		data.append(crypt.encryptAsymmetric(transformationPublic, remotePublicKey, localBlockKey));
		outputRaw(data);
	}
	
	protected ByteBuffer padToLocalBlockKeySize(ByteBuffer data) throws CryptException
	{
		int blockSize = localBlockKey.length();
		int bytesInPartialBlock = data.length() % blockSize;
		if (bytesInPartialBlock > 0)
		{
			int paddingRequired = blockSize - bytesInPartialBlock;
			ByteBuffer padded = new ByteBuffer(data.length() + paddingRequired);
			padded.append(data);
			padded.append(crypt.generateRandomBytes(paddingRequired));
			assert((padded.length()%blockSize) == 0);
			return padded;
		}
		else
		{
			return data;
		}
	}
	
	protected void sendBufferedUserData() throws CryptException
	{
		boolean again = true;
		while (again)
		{
			ByteBuffer entry = null;
			synchronized (this)
			{
				entry = bufferSendUser.poll();
			}
			if (entry != null)
			{
				outputUserData(entry);
			}
			again = (entry != null);
		}
	}
	
	protected void outputUserData(ByteBuffer userData) throws CryptException
	{
		
		blockKeyTTL_Bytes -= userData.length();
		if (blockKeyTTL_Bytes <= 0 || System.currentTimeMillis() >= blockKeyExpire_Time)
		{
			newBlockKey();
		}
		
		ByteBuffer compressedData = deflater.deflate(userData, true);
		ByteBuffer payload =
			crypt.encryptBlockCipher(
				transformationBlock, localBlockKey,
				padToLocalBlockKeySize(compressedData));
		ByteBuffer data = new ByteBuffer(payload.length() + 2);
		data.append(ByteConvert.fromU16(compressedData.length()));
		data.append(payload);
		
		outputRaw(data);
		
	}
	
	public boolean isPendingSend()
	{
		synchronized (this)
		{
			return
				(bufferSendUser != null && bufferSendUser.size() > 0) ||
				(bufferSendRaw != null && bufferSendRaw.length() > 0);
		}
	}
	
	public void send(ByteBuffer userData)
	{
		synchronized (this)
		{
			if (localBlockKey == null)
			{
				bufferSendUser.add(userData);
			}
			else
			{
				try
				{
					outputUserData(userData);
				}
				catch (CryptException ex)
				{
					onException(ex);
				}
			}
		}
	}
	
	protected void newBlockKey() throws CryptException
	{
		blockKeyTTL_Bytes = maxBlockKeyAgeBytes;
		blockKeyExpire_Time = System.currentTimeMillis() + maxBlockKeyAgeSeconds * 1000;
		localBlockKey = crypt.generateRandomBytes(32);
		sendBlockKey();
	}
	
	protected void processPacket(final ByteBuffer data) throws IOException
	{
		if (data.length() < 4)
		{
			throw new IOException("Packet too short");
		}
		
		int lenContent = ByteConvert.toU16(data.extract(0, 2).getBytes());
		
		if (lenContent != 0)
		{
			
			// content data
			ByteBuffer payload = crypt.decryptBlockCipher(transformationBlock, remoteBlockKey, data.extract(2));
			final ByteBuffer content = inflator.inflate(payload.extract(0, lenContent));
			
			listeners.dispatchEvent(new EventSource<CryptListener>()
				{
					public void dispatchEvent(CryptListener l)
					{
						l.cryptMessage(content);
					}
				});
			
		}
		else
		{
			
			// non-content packet
			int messageType = ByteConvert.toU16(data.extract(2, 2).getBytes());
			
			if (messageType == MessageType.PUBLIC_KEY.getValue())
			{
				remotePublicKey = data.extract(4);
				newBlockKey();
				sendBufferedUserData();
			}
			else if (messageType == MessageType.BLOCK_KEY.getValue())
			{
				remoteBlockKey = crypt.decryptAsymmetric(transformationPublic, keypair.getPrivateKey(), data.extract(4));
			}
			else
			{
				throw new IOException("Unknown message type #" + messageType);
			}
			
		}
		
	}
	
	public void connect(String host, int port)
	{
		close();
		synchronized (this)
		{
			active = true;
			socket = null;
			socketOwned = false;
			crypt = CryptFactory.getInstance();
			threadInput = new InputThread();
			threadInput.connectHost = host;
			threadInput.connectPort = port;
			threadInput.setDaemon(true);
			threadInput.start();
		}
	}
	
	public void attach(Socket socket, boolean takeOwnership)
	{
		close();
		synchronized (this)
		{
			this.active = true;
			this.socket = socket;
			this.socketOwned = takeOwnership;
			this.crypt = CryptFactory.getInstance();
			this.threadInput = new InputThread();
			this.threadInput.connectHost = null;
			this.threadInput.connectPort = -1;
			this.threadInput.setDaemon(true);
			this.threadInput.start();
		}
	}
	
	protected void onException(final IOException ex)
	{
		
		boolean okayToTriggerEvent = false;
		synchronized (this)
		{
			if (active)
			{
				active = false;
				okayToTriggerEvent = true;
			}
		}
		
		close();
		
		if (okayToTriggerEvent)
		{
			listeners.dispatchEvent(new EventSource<CryptListener>()
				{
					public void dispatchEvent(CryptListener l)
					{
						if (ex instanceof java.io.EOFException)
						{
							l.cryptClosed();
						}
						else
						{
							l.cryptError(ex);
						}
					}
				});
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
					if (socketOwned)
					{
						socket.close();
					}
				}
				catch (IOException ex)
				{
				}
				socket = null;
				socketOwned = false;
			}
		}
		
		closeThread(threadInput);
		closeThread(threadOutput);
		
		clearState();
		
	}
	
	protected static void closeThread(Thread thread)
	{
		if (thread != null)
		{
			thread.interrupt();
			try
			{
				thread.join();
			}
			catch (InterruptedException ex)
			{
			}
		}
	}
	
	public String getPeerName()
	{
		return socket.getPeerName();
	}
	
}
