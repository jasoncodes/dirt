package au.com.gslabs.dirt.core.client.console;

import au.com.gslabs.dirt.core.client.Client;

public interface ConsoleCommandListener
{
	public String[] getSupportedCommands(ConsoleClientAdapter adapter, Client source);
	public boolean processConsoleInput(ConsoleClientAdapter adapter, Client source, String context, String cmd, String params);
}
