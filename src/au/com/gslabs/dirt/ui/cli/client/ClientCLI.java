package au.com.gslabs.dirt.ui.cli.client;

import au.com.gslabs.dirt.core.client.*;
import jline.*;
import java.io.*;
import java.util.*;
import au.com.gslabs.dirt.lib.util.TextModifierParser;
import au.com.gslabs.dirt.lib.thread.SameThreadInvoker;
import java.util.concurrent.*;

public class ClientCLI
{
	
	public static void init()
	{
		new ClientCLI();
	}
	
	protected Client client;
	protected ConsoleReader console;
	protected SimpleCompletor completor;
	protected PrintWriter out;
	protected boolean bailingOut;
	protected InterruptibleInputStream input;
	protected boolean passwordMode;
	
	protected enum SupportedCommand
	{
		CLEAR,
		EXIT
	}
	
	protected class InterruptibleInputStream extends InputStream
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
		
		protected boolean wasInterrupt()
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
	
	protected class ClientAdapter extends EnumClientAdapter<SupportedCommand>
	{
		
		public ClientAdapter()
		{
			super(SupportedCommand.class);
		}
		
		protected void clientConsoleOutput(Client source, String context, String className, String message)
		{
			output(message);
		}
		
		public boolean clientPreprocessConsoleInput(Client source, String context, SupportedCommand cmd, String params)
		{
			
			switch (cmd)
			{
				
				case CLEAR:
					clearScreen();
					return true;
				
				case EXIT:
					close();
					return true;
					
				default:
					return false;
				
			}
			
		}
		
		public String getClientExtraVersionInfo(Client source)
		{
			return "Console";
		}
		
		public void clientNeedNickname(Client source, String defaultNick)
		{
			String prompt = "/nick " + defaultNick;
			setText(prompt);
		}
		
		public void clientNeedAuthentication(Client source, String prompt)
		{
			super.clientNeedAuthentication(source, prompt);
			setPasswordMode(true);
		}
		
		public void clientStateChanged(Client source)
		{
			super.clientStateChanged(source);
			setPasswordMode(false);
		}
		
	}
	
	protected void setPasswordMode(boolean passwordMode)
	{
		if (this.passwordMode != passwordMode)
		{
			this.passwordMode = passwordMode;
			setText("");
			input.interrupt();
		}
	}
	
	protected void setText(String text)
	{
		CursorBuffer buffer = console.getCursorBuffer();
		buffer.clearBuffer();
		buffer.write(text);
		try
		{
			console.drawLine();
			console.flushConsole();
		}
		catch (IOException ex)
		{
			throw new RuntimeException(ex);
		}
	}
	
	public class MyHistory extends History
	{
		public void addToHistory(String buffer)
		{
			if (client.isConsoleInputHistorySafe(buffer))
			{
				super.addToHistory(buffer);
			}
		}
	}
	
	public ClientCLI()
	{
		
		try
		{
			
			input = new InterruptibleInputStream(new FileInputStream(FileDescriptor.in));
			console = new ConsoleReader(input, new PrintWriter(System.out));
			console.setHistory(new MyHistory());
			passwordMode = false;
			
			client = new Client();
			client.addClientListener(new ClientAdapter(), new SameThreadInvoker());
			
			output("*** Dirt Secure Chat Client " + au.com.gslabs.dirt.Dirt.VERSION);
			
			//completor = new SimpleCompletor(new String[0]);
			//console.addCompletor(completor);
			//completor.addCandidateString("Foo");
			//completor.addCandidateString("Bar");
			
			bailingOut = false;
			
			while (!bailingOut)
			{
				Character mask = this.passwordMode ? new Character('*') : null;
				String line = console.readLine(mask);
				if (line == null)
				{
					if (!input.wasInterrupt())
					{
						bailingOut = true;
					}
				}
				else if (this.passwordMode)
				{
					wipeLastLine();
					setPasswordMode(false);
					client.authenticate(null, line, false);
				}
				else if (line.length() > 0)
				{
					wipeLastLine();
					if (!line.startsWith("/"))
					{
						line = "/SAY " + line;
					}
					client.processConsoleInput(null, line);
				}
			}
			
			if (!bailingOut)
			{
				client.processConsoleInput(null, "/EXIT");
			}
			
		}
		catch (Exception ex)
		{
			if (!bailingOut)
			{
				System.err.println(ex);
			}
		}
		
	}
	
	protected void clearScreen()
	{
		try
		{
			console.clearScreen();
		}
		catch (IOException ex)
		{
		}
	}
	
	protected void close()
	{
		bailingOut = true;
		try
		{
			console.getInput().close();
		}
		catch (IOException ex)
		{
			System.exit(1);
		}
	}
	
	private static final char ESC = (char)27;

	protected void wipeLastLine()
	{
		try
		{
			if (Terminal.getTerminal().isANSISupported())
			{
				console.printString(ESC + "[1A"); // move up one line
				console.printString(ESC + "[K"); //clear line, from cursor position to end
				console.flushConsole();
			}
		}
		catch (IOException ex)
		{
		}
	}
	
	protected void output(String message)
	{
		try
		{
			
			if (Terminal.getTerminal().isANSISupported())
			{
				int bufferLength = console.getCursorBuffer().length();
				if (bufferLength > 0)
				{
					console.printString(ESC + "["+bufferLength+"D"); // move cursor left by bufferLength chars
					console.printString(ESC + "[K"); //clear line, from cursor position to end
					console.flushConsole();
				}
			}
			
			String text = TextModifierParser.parse(
				message, TextModifierParser.OutputFormat.PLAIN);
			console.printString(DefaultClientAdapter.getOutputPrefix() + text + "\n");
			
			console.drawLine();
			console.flushConsole();
			
		}
		catch (IOException ex)
		{
			System.out.println(message);
		}
	}
	
}
