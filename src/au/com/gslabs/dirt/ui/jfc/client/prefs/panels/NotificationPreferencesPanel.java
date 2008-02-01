package au.com.gslabs.dirt.ui.jfc.client.prefs.panels;

import au.com.gslabs.dirt.ui.jfc.client.prefs.*;
import javax.swing.*;
import au.com.gslabs.dirt.lib.ui.jfc.SpringUtilities;

public class NotificationPreferencesPanel extends PreferencesPanel
{
	
	private static final int PADDING = 6;
	private static final int INIT_LOCATION = 6;
	
	private JCheckBox chkNotificationSound;
	
	public NotificationPreferencesPanel()
	{
		
		setLayout(new SpringLayout());
		
		chkNotificationSound = new JCheckBox("Notification Sounds");
		
		trapChangeEvent(chkNotificationSound);
		
		add(new JLabel());
		add(chkNotificationSound);
		
		SpringUtilities.makeCompactGrid(
			this, getComponentCount()/2, 2, 
			INIT_LOCATION, INIT_LOCATION, 
			PADDING, PADDING);
		
	}
	
	public String getName()
	{
		return "General";
	}
	
	public void load()
	{
		chkNotificationSound.setSelected(getPreferences().getNotificationSoundEnabled());
	}
	
	public boolean save()
	{
		getPreferences().setNotificationSoundEnabled(chkNotificationSound.isSelected());
		return true;
	}
	
}
