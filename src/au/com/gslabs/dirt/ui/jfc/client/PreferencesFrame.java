package au.com.gslabs.dirt.ui.jfc.client;

import javax.swing.*;
import au.com.gslabs.dirt.lib.ui.jfc.UIUtil;
import net.roydesign.mac.MRJAdapter;

public class PreferencesFrame extends JFrame
{
	
	public PreferencesFrame()
	{
		
		super("Preferences");
		
		if (MRJAdapter.isSwingUsingScreenMenuBar())
		{
			setJMenuBar(new MainMenuBar());
		}
		UIUtil.setIcon(this);
		
		setSize(200,150);
		setDefaultCloseOperation(WindowConstants.DISPOSE_ON_CLOSE);
		
		add(new JLabel("Real Soon Now\u2122", SwingConstants.CENTER));
		
	}
	
}
