package au.com.gslabs.dirt.ui.jfc.client.prefs.panels;

import javax.swing.*;
import au.com.gslabs.dirt.ui.jfc.client.prefs.*;
import au.com.gslabs.dirt.lib.ui.jfc.SpringUtilities;
import au.com.gslabs.dirt.lib.util.FileUtil;

public class NotificationPreferencesPanel extends PreferencesPanel
{
	
	private static final int PADDING = 6;
	private static final int INIT_LOCATION = 6;
	
	private JComboBox cmbNotificationDockBounce;
	private JComboBox cmbNotificationDockFlash;
	private JCheckBox chkNotificationDockShowUnreadCount;
	private JCheckBox chkNotificationSound;
	
	public NotificationPreferencesPanel()
	{
		
		super("Notifications", "notifications");
		
		setLayout(new SpringLayout());
		
		cmbNotificationDockBounce = new JComboBox(Preferences.NotificationIterations.class.getEnumConstants());
		cmbNotificationDockFlash = new JComboBox(Preferences.NotificationIterations.class.getEnumConstants());
		chkNotificationDockShowUnreadCount = new JCheckBox("Show Unread Message Count");
		chkNotificationDockShowUnreadCount.setOpaque(false);
		chkNotificationSound = new JCheckBox("Play Notification Sound");
		chkNotificationSound.setOpaque(false);
		
		trapChangeEvent(cmbNotificationDockBounce);
		trapChangeEvent(cmbNotificationDockFlash);
		trapChangeEvent(chkNotificationDockShowUnreadCount);
		trapChangeEvent(chkNotificationSound);
		
		add(new JLabel("Notification Flash:"));
		add(cmbNotificationDockFlash);
		if (FileUtil.isMac())
		{
			add(new JLabel("Dock Icon Bounce:"));
			add(cmbNotificationDockBounce);
			add(new JLabel());
			add(chkNotificationDockShowUnreadCount);
		}
		add(new JLabel());
		add(chkNotificationSound);
		
		SpringUtilities.makeCompactGrid(
			this, getComponentCount()/2, 2, 
			INIT_LOCATION, INIT_LOCATION, 
			PADDING, PADDING);
		
	}
	
	public void load()
	{
		setEventsEnabled(false);
		cmbNotificationDockBounce.setSelectedItem(getPreferences().getNotificationDockBounce());
		cmbNotificationDockFlash.setSelectedItem(getPreferences().getNotificationDockFlash());
		chkNotificationSound.setSelected(getPreferences().getNotificationSoundEnabled());
		chkNotificationDockShowUnreadCount.setSelected(getPreferences().getNotificationDockShowUnreadCount());
		setEventsEnabled(true);
	}
	
	public boolean save()
	{
		getPreferences().setNotificationDockBounce((Preferences.NotificationIterations)cmbNotificationDockBounce.getSelectedItem());
		getPreferences().setNotificationDockFlash((Preferences.NotificationIterations)cmbNotificationDockFlash.getSelectedItem());
		getPreferences().setNotificationDockShowUnreadCount(chkNotificationDockShowUnreadCount.isSelected());
		getPreferences().setNotificationSoundEnabled(chkNotificationSound.isSelected());
		return true;
	}
	
}
