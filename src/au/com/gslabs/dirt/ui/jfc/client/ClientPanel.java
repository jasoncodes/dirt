package au.com.gslabs.dirt.ui.jfc.client;

import au.com.gslabs.dirt.core.client.Client;
import au.com.gslabs.dirt.core.client.console.ConsoleClientAdapter;
import java.awt.Component;

public interface ClientPanel
{
	
	public Component getComponent();
	public Client getClient();
	public String getContext();
	public ConsoleClientAdapter getConsoleClientAdapter();
	public String getTitle();
	public void cleanup();
	public boolean isDirty();
	
	public void addClientPanelListener(ClientPanelListener l);
	public void removeClientPanelListener(ClientPanelListener l);
	
}

