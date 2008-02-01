package au.com.gslabs.dirt.ui.jfc.client.prefs.panels;

import au.com.gslabs.dirt.ui.jfc.client.prefs.*;
import javax.swing.*;
import au.com.gslabs.dirt.lib.ui.jfc.SpringUtilities;

public class NotificationPreferencesPanel extends PreferencesPanel
{
	
	private static final int PADDING = 6;
	private static final int INIT_LOCATION = 6;
	
	private JCheckBox chkNotificationSound;
	private JComboBox cmbNotificationDockBounce;
	
	public NotificationPreferencesPanel()
	{
		
		super("Notifications", "notifications");
		
		setLayout(new SpringLayout());
		
		chkNotificationSound = new JCheckBox("Notification Sounds");
		cmbNotificationDockBounce = new JComboBox(Preferences.NotificationDockBounce.class.getEnumConstants());
		
		trapChangeEvent(chkNotificationSound);
		trapChangeEvent(cmbNotificationDockBounce);
		
		add(new JLabel());
		add(chkNotificationSound);
		//todo//add(new JLabel("Dock Bounce:"));
		//todo//add(cmbNotificationDockBounce);
		
		SpringUtilities.makeCompactGrid(
			this, getComponentCount()/2, 2, 
			INIT_LOCATION, INIT_LOCATION, 
			PADDING, PADDING);
		
	}
	
	public void load()
	{
		chkNotificationSound.setSelected(getPreferences().getNotificationSoundEnabled());
		cmbNotificationDockBounce.setSelectedItem(getPreferences().getNotificationDockBounce());
	}
	
	public boolean save()
	{
		getPreferences().setNotificationSoundEnabled(chkNotificationSound.isSelected());
		getPreferences().setNotificationDockBounce((Preferences.NotificationDockBounce)cmbNotificationDockBounce.getSelectedItem());
		return true;
	}
	
}
