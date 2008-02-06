package au.com.gslabs.dirt.ui.jfc.client;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.event.*;
import au.com.gslabs.dirt.lib.ui.jfc.*;
import au.com.gslabs.dirt.lib.util.FileUtil;
import java.util.ResourceBundle;
import au.com.gslabs.dirt.ui.jfc.client.prefs.Preferences;

public class ClientFrame extends JFrame
{
	
	private final ResourceBundle resbundle = ResourceBundle.getBundle("res/strings");
	
	protected final ClientPanel panel;
	
	public ClientFrame(final ClientPanel panel)
	{
		
		super("");
		this.panel = panel;
		
		setDefaultCloseOperation(WindowConstants.DISPOSE_ON_CLOSE);
		UIUtil.setIcon(this);
		setJMenuBar(new MainMenuBar());
		
		panel.addClientPanelListener(new ClientPanelHandler());
		
		getContentPane().add(panel.getComponent());
		
		addWindowListener(new WindowAdapter()
			{
				
				@Override
				public void windowActivated(WindowEvent e)
				{
					panel.getComponent().requestFocusInWindow();
				}
				
				@Override
				public void windowClosed(WindowEvent event)
				{
					panel.cleanup();
				}
				
			});
		
		addComponentListener(new ComponentAdapter()
			{
				
				ActionRateLimiter limiter = new ActionRateLimiter(new ActionListener()
					{
						public void actionPerformed(ActionEvent e)
						{
							if ((getExtendedState() & JFrame.ICONIFIED) == 0)
							{
								Preferences.getInstance().setPanelBounds(
									panel.getPanelPreferenceKeys(), "panelBounds", getBounds());
							}
						}
					}, 1000);
					
				@Override
				public void componentMoved(ComponentEvent e)
				{
					limiter.trigger();
				}
				
				@Override
				public void componentResized(ComponentEvent e)
				{
					limiter.trigger();
				}
				
			});
		
		Rectangle bounds = UIUtil.getDefaultWindowBounds(this, panel.getDefaultNormalisedScreenPosition());
		bounds = Preferences.getInstance().getPanelBounds(panel.getPanelPreferenceKeys(), "panelBounds", bounds);
		UIUtil.setWindowBoundsWithCascade(this, bounds);
		
		updateWindowTitle();
		
	}
	
	private class ClientPanelHandler implements ClientPanelListener
	{
		
		public void panelTitleChanged(ClientPanel panel)
		{
			updateWindowTitle();
			getRootPane().putClientProperty("windowModified", Boolean.valueOf(panel.isDirty()));
		}
		
		public void panelRequestsAttention(ClientPanel panel)
		{
			doAlert();
		}
		
		public boolean linkClicked(ClientPanel panel, java.net.URL url)
		{
			return false;
		}
		
		public void panelShouldFocus(ClientPanel panel)
		{
			UIUtil.stealFocus(ClientFrame.this);
		}
		
	}
	
	protected void doAlert()
	{
		
		final Preferences prefs = Preferences.getInstance();
		UIUtil.alert(
			ClientFrame.this,
			prefs.getNotificationDockBounce().intValue(),
			prefs.getNotificationDockFlash().intValue(),
			prefs.getNotificationDockShowUnreadCount());
		
		if (Preferences.getInstance().getNotificationSoundEnabled())
		{
			java.awt.Toolkit.getDefaultToolkit().beep();
		}
		
	}
	
	protected void updateWindowTitle()
	{
		
		String title = panel.getTitle();
		
		if (title.length() == 0 || !FileUtil.isMac())
		{
			if (title.length() > 0)
			{
				title += " - ";
			}
			title += resbundle.getString("title");
		}
		
		final String suffix = getTitleSuffix();
		if (suffix != null) title += suffix;
		
		setTitle(title);
		
	}
	
	protected String getTitleSuffix()
	{
		return null;
	}
	
}
