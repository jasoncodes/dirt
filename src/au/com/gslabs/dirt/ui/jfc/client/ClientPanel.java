package au.com.gslabs.dirt.ui.jfc.client;

import au.com.gslabs.dirt.core.client.Client;
import au.com.gslabs.dirt.core.client.console.ConsoleClientAdapter;
import java.awt.Component;
import java.awt.Dimension;
import java.awt.Rectangle;

public interface ClientPanel
{
	
	public Component getComponent();
	public Client getClient();
	public String getContext();
	public ConsoleClientAdapter getConsoleClientAdapter();
	public String getTitle();
	public void cleanup();
	public void detach();
	public boolean isDirty();
	public String[] getPanelPreferenceKeys();
	public Dimension getPreferredSize();
	public java.awt.geom.Point2D.Double getDefaultNormalisedScreenPosition();
	
	public void addClientPanelListener(ClientPanelListener l);
	public void removeClientPanelListener(ClientPanelListener l);
	
}

