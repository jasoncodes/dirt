package au.com.gslabs.dirt.ui.jfc.client;

import java.util.*;
import javax.swing.*;
import javax.swing.border.*;
import java.awt.*;
import java.awt.event.*;
import au.com.gslabs.dirt.lib.ui.jfc.*;

public abstract class BaseClientPanel extends JPanel implements ClientPanel
{
	
	private final String context;
	private final ArrayList<ClientPanelListener> listeners = new ArrayList<ClientPanelListener>();
	private boolean isFocused = false;
	
	public BaseClientPanel(String context)
	{
		
		this.context = context;
		
		new ContainerFocusEventProxy(this).addFocusListener(new FocusListener()
			{
				public void focusLost(FocusEvent e)
				{
					isFocused = false;
				}
				public void focusGained(FocusEvent e)
				{
					isFocused = true;
				}
			});
		
	}
	
	protected boolean isFocused()
	{
		return isFocused;
	}
	
	public void addClientPanelListener(ClientPanelListener l)
	{
		listeners.add(l);
	}
	
	public void removeClientPanelListener(ClientPanelListener l)
	{
		int idx = listeners.indexOf(l);
		listeners.remove(idx);
	}
	
	public Component getComponent()
	{
		return this;
	}
	
	public String getContext()
	{
		return context;
	}
	
	public Color getBorderColor()
	{
		return new EtchedBorder().getShadowColor(this);
	}
	
	public void requestAttention()
	{
		if (!isFocused)
		{
			for (ClientPanelListener l : listeners)
			{
				l.panelRequestsAttention(this);
			}
		}
	}
	
	public void requestAttentionAfterDelay(final long delayMilliseconds)
	{
		new Thread(new Runnable() {
			public void run()
			{
				try
				{
					Thread.sleep(delayMilliseconds);
				}
				catch (InterruptedException ex)
				{
				}
				requestAttention();
			}
		}).start();
	}
	
	public void requestFocus()
	{
		for (ClientPanelListener l : listeners)
		{
			l.panelShouldFocus(this);
		}
		super.requestFocus();
	}
	
	protected void notifyLinkClicked(final java.net.URL url)
	{
		for (ClientPanelListener l : listeners)
		{
			if (l.linkClicked(this, url))
			{
				return;
			}
		}
		UIUtil.openURL(url.toString());
	}
	
	public void notifyTitleChanged()
	{
		for (ClientPanelListener l : listeners)
		{
			l.panelTitleChanged(this);
		}
	}
	
}
