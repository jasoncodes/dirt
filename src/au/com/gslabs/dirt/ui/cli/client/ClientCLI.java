package au.com.gslabs.dirt.ui.cli.client;

import au.com.gslabs.dirt.core.client.*;
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
	
	protected Console console;
	protected Client client;
	protected ClientAdapter clientAdapter;
	
	protected enum SupportedCommand
	{
		ABOUT,
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
		protected void clientConsoleOutput(Client source, String context, String className, boolean suppressAlert, String message)
		{
			output(message);
		}
		
		@Override
		protected boolean clientPreprocessConsoleInput(Client source, String context, SupportedCommand cmd, String params)
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
		console.println(DefaultClientAdapter.getOutputPrefix() + line);
	}
	
	private ClientCLI()
	{
		
		console = new Console();
		
		client = new Client();
		clientAdapter = new ClientAdapter();
		client.addClientListener(clientAdapter, new SameThreadInvoker());
		
		console.setCompletor(new ContactNickCompletor(client));
		console.setHistoryFilter(new ClientAdapterHistoryFilter(clientAdapter));
		
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
