package au.com.gslabs.dirt.core.client;

import java.util.ArrayList;

public abstract class EnumClientAdapter<SupportedCommand extends Enum<SupportedCommand>> extends DefaultClientAdapter
{
	
	protected Class<SupportedCommand> enumType;
	
	protected EnumClientAdapter(Class<SupportedCommand> enumType)
	{
		this.enumType = enumType;
	}
	
	public final String[] getClientSupportedCommands(Client source)
	{
		SupportedCommand[] cmds = enumType.getEnumConstants();
		ArrayList<String> names = new ArrayList<String>(cmds.length);
		for (SupportedCommand cmd : cmds)
		{
			names.add(cmd.toString());
		}
		return names.toArray(new String[0]);
	}
	
	protected abstract boolean clientPreprocessConsoleInput(Client source, String context, SupportedCommand cmd, String params);

	public final boolean clientPreprocessConsoleInput(Client source, String context, String cmd, String params)
	{
		
		SupportedCommand cmdEnum;
		
		try
		{
			cmdEnum = Enum.valueOf(enumType, cmd);
		}
		catch (Exception ex)
		{
			return super.clientPreprocessConsoleInput(source, context, cmd, params);
		}
		
		if (clientPreprocessConsoleInput(source, context, cmdEnum, params))
		{
			return true;
		}
		else
		{
			return super.clientPreprocessConsoleInput(source, context, cmd, params);
		}
		
	}
	
}