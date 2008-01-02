package au.com.gslabs.dirt.core.client.console;

import java.util.ArrayList;
import au.com.gslabs.dirt.core.client.Client;

public abstract class EnumConsoleCommandAdapter<SupportedCommand extends Enum<SupportedCommand>> implements ConsoleCommandListener
{
	
	protected Class<SupportedCommand> enumType;
	
	protected EnumConsoleCommandAdapter(Class<SupportedCommand> enumType)
	{
		this.enumType = enumType;
	}
	
	public final String[] getSupportedCommands(ConsoleClientAdapter adapter, Client source)
	{
		SupportedCommand[] cmds = enumType.getEnumConstants();
		ArrayList<String> names = new ArrayList<String>(cmds.length);
		for (SupportedCommand cmd : cmds)
		{
			names.add(cmd.toString());
		}
		return names.toArray(new String[0]);
	}
	
	protected abstract boolean processConsoleInput(ConsoleClientAdapter adapter, Client source, String context, SupportedCommand cmd, String params);

	public final boolean processConsoleInput(ConsoleClientAdapter adapter, Client source, String context, String cmd, String params)
	{
		
		SupportedCommand cmdEnum;
		
		try
		{
			cmdEnum = Enum.valueOf(enumType, cmd);
		}
		catch (Exception ex)
		{
			return false;
		}
		
		return processConsoleInput(adapter, source, context, cmdEnum, params);
		
	}
	
}
