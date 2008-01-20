package au.com.gslabs.dirt.ui.jfc.client;

import au.com.gslabs.dirt.core.client.Client;

public interface ChatPanel extends ClientPanel
{
	
	public boolean clientConsoleOutput(Client source, String context, String className, boolean suppressAlert, String message);
	public void clearText();
	public void outputTestData();
	
}
