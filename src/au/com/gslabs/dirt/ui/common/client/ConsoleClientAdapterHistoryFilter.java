package au.com.gslabs.dirt.ui.common.client;

import au.com.gslabs.dirt.lib.ui.common.HistoryFilter;
import au.com.gslabs.dirt.core.client.console.*;

public class ConsoleClientAdapterHistoryFilter implements HistoryFilter
{
	
	public ConsoleClientAdapter clientAdapter;
	
	public ConsoleClientAdapterHistoryFilter(ConsoleClientAdapter clientAdapter)
	{
		this.clientAdapter = clientAdapter;
	}
	
	public boolean canAddToHistory(String buffer)
	{
		return this.clientAdapter.isConsoleInputHistorySafe(buffer);
	}
	
}
