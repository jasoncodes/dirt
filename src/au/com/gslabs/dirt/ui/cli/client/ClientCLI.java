package au.com.gslabs.dirt.ui.cli.client;

import au.com.gslabs.dirt.core.client.*;
import jline.*;
import java.io.*;
import java.util.*;
import au.com.gslabs.dirt.lib.util.TextModifierParser;
import au.com.gslabs.dirt.lib.thread.SameThreadInvoker;
import au.com.gslabs.dirt.lib.thread.InterruptibleInputStream;
import au.com.gslabs.dirt.ui.common.client.*;

public class ClientCLI
{
	
	public static void init()
	{
		new ClientCLI();
	}
	
	protected Client client;
	protected ConsoleReader console;
	protected PrintWriter out;
	protected boolean bailingOut;
	protected InterruptibleInputStream input;
	protected boolean passwordMode;
	
	protected enum SupportedCommand
	{
		CLEAR,
		EXIT
	}
	
	protected class ClientAdapter extends EnumClientAdapter<SupportedCommand>
	{
		
		public ClientAdapter()
		{
			super(SupportedCommand.class);
		}
		
		@Override
		protected void clientConsoleOutput(Client source, String context, String className, String message)
		{
			output(message);
		}
		
		@Override
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
		
		@Override
		public String getClientExtraVersionInfo(Client source)
		{
			return "Console";
		}
		
		@Override
		public void clientNeedNickname(Client source, String defaultNick)
		{
			String prompt = "/nick " + defaultNick;
			setText(prompt);
		}
		
		@Override
		public void clientNeedAuthentication(Client source, String prompt)
		{
			super.clientNeedAuthentication(source, prompt);
			setPasswordMode(true);
		}
		
		@Override
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
		@Override
		public void addToHistory(String buffer)
		{
			if (client.isConsoleInputHistorySafe(buffer))
			{
				super.addToHistory(buffer);
			}
		}
	}
	
	public class MyCompletor implements jline.Completor
	{
		
		protected ContactNickCompletor completor;
		
		public MyCompletor(ContactNickCompletor completor)
		{
			this.completor = completor;
		}
		
		@SuppressWarnings("unchecked") // jline.Completor interface isn't generic
		public int complete(String buffer, int cursor, List candidates)
		{
			return this.completor.complete(buffer, cursor, candidates);
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
			console.addCompletor(new MyCompletor(new ContactNickCompletor(client)));
			
			output("*** Dirt Secure Chat Client " + au.com.gslabs.dirt.Dirt.VERSION);
			
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
	
	protected synchronized void output(String message)
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
