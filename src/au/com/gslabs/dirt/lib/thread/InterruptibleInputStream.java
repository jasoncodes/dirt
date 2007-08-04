package au.com.gslabs.dirt.lib.thread;

import java.util.concurrent.*;
import java.util.*;
import java.io.*;

public class InterruptibleInputStream extends InputStream
{
	
	protected InputStream in;
	protected ReadThread thread;
	BlockingQueue<Integer> queue;
	protected Exception ex;
	protected int lastRead;
	
	public InterruptibleInputStream(InputStream in)
	{
		this.in = in;
		this.queue = new LinkedBlockingQueue<Integer>(1);
		this.ex = null;
		thread = new ReadThread();
		thread.start();
		lastRead = -4;
	}
	
	public int read() throws IOException
	{
		if (this.ex != null)
		{
			IOException ex2 = new IOException("Error reading from input stream");
			ex2.initCause(this.ex);
			throw ex2;
		}
		
		try
		{
			lastRead = queue.take().intValue();
		}
		catch (InterruptedException ex)
		{
			lastRead = -3;
		}
		return (lastRead < 0) ? -1 : lastRead;
	}
	
	protected class ReadThread extends Thread
	{
		ReadThread()
		{
			super("InterruptibleInputStream$ReadThread");
			setDaemon(true);
		}
		public void run()
		{
			try
			{
				while (true)
				{
					queue.put(in.read());
				}
			}
			catch (Exception ex)
			{
				InterruptibleInputStream.this.ex = ex;
			}
		}
	}
	
	public boolean wasInterrupt()
	{
		return lastRead == -2 || lastRead == -3;
	}
	
	public void interrupt()
	{
		try
		{
			queue.put(-2);
		}
		catch (InterruptedException ex)
		{
		}
	}
	
}
