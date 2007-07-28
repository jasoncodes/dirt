package au.com.gslabs.dirt.ui.cli.client;

import au.com.gslabs.dirt.core.client.*;
import jline.*;
import java.io.*;
import java.util.*;
import au.com.gslabs.dirt.lib.util.TextModifierParser;
import au.com.gslabs.dirt.lib.thread.SameThreadInvoker;

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
			setText("");
			console.setEchoCharacter(new Character('*'));
		}
		
		public void clientStateChanged(Client source)
		{
			super.clientStateChanged(source);
			if (console.getEchoCharacter() != null)
			{
				setText("");
				console.setEchoCharacter(null);
			}
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
	
	public ClientCLI()
	{
		
		try
		{
			
			Terminal.getTerminal().initializeTerminal();
			console = new ConsoleReader();
			
			client = new Client();
			client.addClientListener(new ClientAdapter(), new SameThreadInvoker());
			
			output("*** Dirt Secure Chat Client " + au.com.gslabs.dirt.Dirt.VERSION);
			
			//completor = new SimpleCompletor(new String[0]);
			//console.addCompletor(completor);
			//completor.addCandidateString("Foo");
			//completor.addCandidateString("Bar");
			
			bailingOut = false;
			
			String line;
			while ((line = console.readLine("")) != null)
			{
				if (console.getEchoCharacter() != null)
				{
					setText("");
					console.setEchoCharacter(null);
					client.authenticate(null, line);
				}
				else
				{
					if (line.length() > 0 && !line.startsWith("/"))
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
	
	protected void output(String message)
	{
		try
		{
			String text = TextModifierParser.parse(
				message, TextModifierParser.OutputFormat.PLAIN);
			console.printString(DefaultClientAdapter.getOutputPrefix() + text + "\n");
			console.flushConsole();
		}
		catch (IOException ex)
		{
			System.out.println(message);
		}
	}
	
}
