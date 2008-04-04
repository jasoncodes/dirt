package au.com.gslabs.dirt.core.client.console;

import java.util.ArrayList;
import au.com.gslabs.dirt.core.client.Client;
import java.lang.reflect.ParameterizedType;

public abstract class EnumConsoleCommandAdapter<SupportedCommand extends Enum<SupportedCommand>> implements ConsoleCommandListener
{
	
	private final Class<SupportedCommand> enumType;
	
	@SuppressWarnings("unchecked") // cast from non-generic Type to Class<SupportedCommand>
	protected EnumConsoleCommandAdapter()
	{
		final ParameterizedType thisType = (ParameterizedType)getClass().getGenericSuperclass();
		this.enumType = (Class<SupportedCommand>)thisType.getActualTypeArguments()[0];
	}
	
	public final String[] getSupportedCommands(ConsoleClientAdapter adapter, Client source)
	{
		final SupportedCommand[] cmds = enumType.getEnumConstants();
		final ArrayList<String> names = new ArrayList<String>(cmds.length);
		for (final SupportedCommand cmd : cmds)
		{
			names.add(cmd.toString());
		}
		return names.toArray(new String[0]);
	}
	
	protected abstract boolean processConsoleInput(ConsoleClientAdapter adapter, Client source, String context, SupportedCommand cmd, String params);

	public final boolean processConsoleInput(ConsoleClientAdapter adapter, Client source, String context, String cmd, String params)
	{
		
		final SupportedCommand cmdEnum;
		
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
