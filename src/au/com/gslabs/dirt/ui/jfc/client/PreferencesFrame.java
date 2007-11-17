package au.com.gslabs.dirt.ui.jfc.client;

import javax.swing.*;

public class PreferencesFrame extends JFrame
{
	
	public PreferencesFrame()
	{
		super("Preferences");
		setJMenuBar(new MainMenuBar());
		setSize(200,150);
		setDefaultCloseOperation(WindowConstants.DISPOSE_ON_CLOSE);
		add(new JLabel("Real Soon Now\u2122", SwingConstants.CENTER));
	}
	
}
