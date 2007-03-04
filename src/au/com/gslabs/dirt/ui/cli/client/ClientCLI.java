package au.com.gslabs.dirt.ui.cli.client;

import au.com.gslabs.dirt.core.client.*;
import jline.*;
import java.io.*;
import java.util.*;

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
	
	protected class ClientAdapter extends DefaultClientAdapter
	{
		
		java.text.SimpleDateFormat sdf = new java.text.SimpleDateFormat("[HH:mm] ");
		
		public void clientConsoleOutput(Client source, String context, String className, String message)
		{
			java.util.Calendar c = java.util.Calendar.getInstance();
			output(sdf.format(c.getTime()) + message);
		}
		
		public String[] getClientSupportedCommands(Client source)
		{
			return new String[]
				{
					"EXIT",
					"CLEAR"
				};
		}
		
		public boolean clientPreprocessConsoleInput(Client source, String context, String cmd, String params)
		{
			if (cmd.equals("EXIT"))
			{
				bailingOut = true;
				try
				{
					console.getInput().close();
					return true;
				}
				catch (IOException ex)
				{
					return false;
				}
			}
			else if (cmd.equals("CLEAR"))
			{
				try
				{
					console.clearScreen();
					return true;
				}
				catch (IOException ex)
				{
					return false;
				}
			}
			else
			{
				return false;
			}
		}
		
		public String getClientExtraVersionInfo(Client source)
		{
			return "Console";
		}
		
	}
	
	public ClientCLI()
	{
		
		try
		{
			
			Terminal.getTerminal().initializeTerminal();
			console = new ConsoleReader();
			
			client = new Client();
			client.addClientListener(new ClientAdapter());
			
			output("*** Dirt Secure Chat Client " + au.com.gslabs.dirt.Dirt.VERSION);
			
			completor = new SimpleCompletor(new String[0]);
			console.addCompletor(completor);
			
			completor.addCandidateString("Foo");
			completor.addCandidateString("Bar");
			
			bailingOut = false;
			
			String line;
			while ((line = console.readLine("")) != null)
			{
				if (!line.startsWith("/"))
				{
					line = "/SAY " + line;
				}
				client.processConsoleInput(null, line);
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
	
	protected void output(String message)
	{
		try
		{
			console.printString(message + "\n");
			console.flushConsole();
		}
		catch (IOException ex)
		{
			System.out.println(message);
		}
	}
	
}