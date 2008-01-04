package au.com.gslabs.dirt.ui.jfc.client;

import java.util.ResourceBundle;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import au.com.gslabs.dirt.lib.ui.jfc.UIUtil;
//import au.com.gslabs.dirt.core.client.*;
//import au.com.gslabs.dirt.core.client.console.*;

public class MainApplet extends JApplet
{
	
	protected final ResourceBundle resbundle = ResourceBundle.getBundle("res/strings");
	protected final MainPanel panel;
	protected final JLabel lblTitle;
	
	public MainApplet()
	{
		
		UIUtil.initSwing(resbundle.getString("name"));
		
		panel = new MainPanel();
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
			Toolkit.getDefaultToolkit().beep();
		}
		
	}
	
	protected void updateWindowTitle()
	{
		
		String title = panel.getTitle();
		
		if (title.length() == 0)
		{
			title =
				resbundle.getString("title") + " " +
				resbundle.getString("version") + " " +
				resbundle.getString("sourceDate");
		}
		
		lblTitle.setText(title);
		
	}
	
	public void destroy()
	{
		panel.cleanup();
	}
	
}
