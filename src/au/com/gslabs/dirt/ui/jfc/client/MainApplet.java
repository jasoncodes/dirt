package au.com.gslabs.dirt.ui.jfc.client;

import java.util.ResourceBundle;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import au.com.gslabs.dirt.lib.ui.jfc.UIUtil;

public class MainApplet extends JApplet
{
	
	protected final ResourceBundle resbundle = ResourceBundle.getBundle("res/strings");
	protected final MainPanel panel;
	protected final JLabel lblTitle;
	
	public MainApplet()
	{
		
		UIUtil.initSwing(resbundle.getString("name"));
		
		panel = new MainPanel();
		panel.setClientExtraVersionInfo("Applet");
		panel.addMainPanelListener(new MainPanelHandler());
		
		lblTitle = new JLabel();
		lblTitle.setText(resbundle.getString("title"));
		lblTitle.setBorder(BorderFactory.createMatteBorder(0,0,1,0, panel.getBorderColor()));
		
		JPanel container = new JPanel();
		container.setLayout(new BorderLayout());
		container.setBorder(BorderFactory.createLineBorder(panel.getBorderColor()));
		container.add(panel, BorderLayout.CENTER);
		container.add(lblTitle, BorderLayout.NORTH);
		getContentPane().add(container);
		
		addFocusListener(new FocusListener()
			{
				
				public void focusGained(FocusEvent e)
				{
					panel.requestFocusInWindow();
				}
				
				public void focusLost(FocusEvent e)
				{
				}
				
			});
		
		updateWindowTitle();
		
	}
	
	protected class MainPanelHandler implements MainPanelListener
	{
		
		public void clientStateChanged(MainPanel panel)
		{
			updateWindowTitle();
			getRootPane().putClientProperty("windowModified", Boolean.valueOf(panel.isDirty()));
		}
		
		public void panelRequestsAttention(MainPanel panel)
		{
			// we could Toolkit.getDefaultToolkit().beep(); but that'll probably annoy users
		}
		
		public boolean linkClicked(MainPanel panel, java.net.URL url)
		{
			getAppletContext().showDocument(url, "_blank");
			return true;
		}
		
	}
	
	protected void updateWindowTitle()
	{
		
		String title = panel.getTitle();
		
		if (title.length() == 0)
		{
			title = getAppletInfo();
		}
		
		lblTitle.setText(title);
		
	}
	
	@Override
	public String getAppletInfo()
	{
		return
			resbundle.getString("title") + " " +
			resbundle.getString("version") + " " +
			resbundle.getString("sourceDate");
	}
	
	@Override
	public String[][] getParameterInfo()
	{
		return new String[][]
			{
				{ "url", "String", "URL the client should connect to (AUTO to detect)" }
			};
	}
	
	@Override
	public void start()
	{
		String url = getParameter("url");
		if (url.equals("AUTO"))
		{
			url = getDocumentBase().getHost();
		}
		if (url != null && url.length() > 0)
		{
			panel.getClient().connect(null, url);
		}
	}
	
	@Override
	public void stop()
	{
		panel.cleanup();
	}
	
}
