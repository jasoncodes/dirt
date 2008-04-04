package au.com.gslabs.dirt.ui.cli.client;

import au.com.gslabs.dirt.core.client.*;
import au.com.gslabs.dirt.core.client.console.*;
import au.com.gslabs.dirt.lib.thread.SameThreadInvoker;
import au.com.gslabs.dirt.lib.ui.cli.Console;
import au.com.gslabs.dirt.ui.common.client.*;
import java.util.ResourceBundle;

public class ClientCLI
{
	
	public static void init()
	{
		new ClientCLI();
	}
	
	private Console console;
	private Client client;
	private ConsoleClientAdapter clientAdapter;
	
	private enum SupportedCommand
	{
		ABOUT,
		CLEAR,
		EXIT
	}
	
	private class CommandAdapter extends EnumConsoleCommandAdapter<SupportedCommand>
	{
		
		@Override
		protected boolean processConsoleInput(ConsoleClientAdapter adapter, Client source, String context, SupportedCommand cmd, String params)
		{
			
			switch (cmd)
			{
				
				case ABOUT:
					final ResourceBundle resbundle = ResourceBundle.getBundle("res/strings");
					String msg = 
						"*** " +
						resbundle.getString("title") + " " +
						resbundle.getString("version") + " " +
						resbundle.getString("sourceDate");
					output(msg);
					return true;
				
				case CLEAR:
					console.clearScreen();
					return true;
				
				case EXIT:
					console.close();
					return true;
					
				default:
					return false;
				
			}
			
		}
		
	}
	
	private class ClientAdapter extends ConsoleClientAdapter
	{
		
		public ClientAdapter()
		{
			addConsoleCommandListener(new CommandAdapter());
		}
		
		@Override
		protected String getContextForNickname(String nickname, boolean okayToCreate)
		{
			return null; // we don't support multiple contexts
		}
		
		@Override
		protected String getNicknameForContext(String context)
		{
			return null; // we don't support multiple contexts
		}
		
		@Override
		protected void clientConsoleOutput(Client source, String context, String className, boolean suppressAlert, String message)
		{
			output(message);
		}
		
		@Override
		public String getClientExtraVersionInfo(Client source)
		{
			return "Console";
		}
		
		@Override
		public void clientNeedNickname(Client source, String defaultNick, boolean defaultNickOkay)
		{
			if (defaultNickOkay)
			{
				source.setNickname(null, defaultNick);
				return;
			}
			String prompt = "/nick " + defaultNick;
			console.setText(prompt);
		}
		
		@Override
		public void clientNeedAuthentication(Client source, String prompt)
		{
			super.clientNeedAuthentication(source, prompt);
			console.setPasswordMode(true);
		}
		
		@Override
		public void clientStateChanged(Client source)
		{
			super.clientStateChanged(source);
			if (!source.isConnected())
			{
				console.setPasswordMode(false);
			}
		}
		
	}
	
	public void output(String line)
	{
		console.setPasswordMode(false);
		console.println(ConsoleClientAdapter.getOutputPrefix() + line);
	}
	
	private ClientCLI()
	{
		
		console = new Console();
		
		client = new Client();
		clientAdapter = new ClientAdapter();
		client.addClientListener(clientAdapter, new SameThreadInvoker());
		
		console.setCompletor(new ContactNickCompletor(client));
		console.setHistoryFilter(new ConsoleClientAdapterHistoryFilter(clientAdapter));
		
		output("*** " + ResourceBundle.getBundle("res/strings").getString("title"));
		
		Console.UserInput input;
		while ((input = console.getNextUserInput()) != null)
		{
			if (input.isPassword)
			{
				client.authenticate(null, input.text, false);
			}
			else
			{
				String line = input.text;
				if (!line.startsWith("/"))
				{
					line = "/SAY " + line;
				}
				clientAdapter.processConsoleInput(client, null, line);
			}
		}
		
	}
	
}
