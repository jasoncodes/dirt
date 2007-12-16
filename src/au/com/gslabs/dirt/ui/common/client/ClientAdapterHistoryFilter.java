package au.com.gslabs.dirt.ui.common.client;

import au.com.gslabs.dirt.lib.ui.common.HistoryFilter;
import au.com.gslabs.dirt.core.client.*;

public class ClientAdapterHistoryFilter implements HistoryFilter
{
	
	public DefaultClientAdapter clientAdapter;
	
	public ClientAdapterHistoryFilter(DefaultClientAdapter clientAdapter)
	{
		this.clientAdapter = clientAdapter;
	}
	
	public boolean canAddToHistory(String buffer)
	{
		return this.clientAdapter.isConsoleInputHistorySafe(buffer);
	}
	
}
