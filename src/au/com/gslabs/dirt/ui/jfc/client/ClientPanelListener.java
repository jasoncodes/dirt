package au.com.gslabs.dirt.ui.jfc.client;

public interface ClientPanelListener
{
	
	public void panelTitleChanged(ClientPanel panel);
	
	public void panelRequestsAttention(ClientPanel panel);
	
	/**
	 * @return Return true if this listener has handled the link.
	 *         If all listeners return false, the default URL handler be used
	 */
	public boolean linkClicked(ClientPanel panel, java.net.URL url);
	
	public void panelShouldFocus(ClientPanel panel);
	
}
