package au.com.gslabs.dirt.ui.jfc.client.prefs.panels;

import java.awt.*;
import javax.swing.*;
import javax.swing.border.*;
import au.com.gslabs.dirt.ui.jfc.client.prefs.*;
import au.com.gslabs.dirt.lib.util.FileUtil;
import au.com.gslabs.dirt.lib.ui.jfc.UIUtil;

public class NotificationPreferencesPanel extends PreferencesPanel
{
	
	private static final int PADDING = 6;
	private static final int INIT_LOCATION = 6;
	private static final int SIZE_EM = 20;
	
	private JComboBox cmbNotificationDockBounce;
	private JComboBox cmbNotificationDockFlash;
	private JCheckBox chkNotificationDockShowUnreadCount;
	private JCheckBox chkNotificationSound;
	
	public NotificationPreferencesPanel()
	{
		
		super("Notifications", "notifications");
		
		setLayout(new GridBagLayout());
		
		GridBagConstraints gbc = new GridBagConstraints();
		gbc.fill = GridBagConstraints.NONE;
		
		Insets init = new Insets(PADDING, INIT_LOCATION+PADDING, PADDING, 0);
		Insets norm = new Insets(PADDING, PADDING, 0, PADDING);
		Insets noInsets = new Insets(0,0,0,0);
		Border noBorder = new EmptyBorder(noInsets);
		Dimension txtBoxWidthEms = UIUtil.getWidthEmsDimension( SIZE_EM );
		
		cmbNotificationDockBounce = new JComboBox(Preferences.NotificationIterations.class.getEnumConstants());
		cmbNotificationDockBounce.setMinimumSize( txtBoxWidthEms );
		cmbNotificationDockBounce.setPreferredSize( txtBoxWidthEms );
		cmbNotificationDockFlash = new JComboBox(Preferences.NotificationIterations.class.getEnumConstants());
		cmbNotificationDockFlash.setMinimumSize( txtBoxWidthEms );
		cmbNotificationDockFlash.setPreferredSize( txtBoxWidthEms );
		
		chkNotificationDockShowUnreadCount = new JCheckBox("Show Unread Message Count");
		chkNotificationDockShowUnreadCount.setOpaque(false);
		chkNotificationDockShowUnreadCount.setMargin(noInsets);
		chkNotificationDockShowUnreadCount.setBorder(noBorder);
		
		chkNotificationSound = new JCheckBox("Play Notification Sound");
		chkNotificationSound.setOpaque(false);
		chkNotificationSound.setMargin(noInsets);
		chkNotificationSound.setBorder(noBorder);
		
		trapChangeEvent(cmbNotificationDockBounce);
		trapChangeEvent(cmbNotificationDockFlash);
		trapChangeEvent(chkNotificationDockShowUnreadCount);
		trapChangeEvent(chkNotificationSound);
		
		gbc.gridx = 0;
		gbc.gridy = 0;
		gbc.insets = init;
		gbc.anchor = GridBagConstraints.EAST;
		add(new JLabel("Notification Flash:"), gbc);
		gbc.gridx = 1;
		gbc.weighty = 0.0;
		gbc.insets = norm;
		gbc.anchor = GridBagConstraints.WEST;
		add(cmbNotificationDockFlash, gbc);
		
		if (FileUtil.isMac())
		{
			gbc.gridx = 0;
			gbc.gridy = 1;
			gbc.insets = init;
			gbc.anchor = GridBagConstraints.EAST;
			add(new JLabel("Dock Icon Bounce:"), gbc);
			gbc.gridx = 1;
			gbc.insets = norm;
			gbc.anchor = GridBagConstraints.WEST;
			add(cmbNotificationDockBounce, gbc);
			
			gbc.gridx = 1;
			gbc.gridy = 2;
			gbc.weightx = 1.0;
			gbc.insets = norm;
			gbc.anchor = GridBagConstraints.WEST;
			add(chkNotificationDockShowUnreadCount, gbc);
		}
		
		gbc.gridx = 1;
		gbc.gridy = 3;
		gbc.weightx = 1.0;
		gbc.weighty = 1.0;
		gbc.insets = norm;
		gbc.anchor = GridBagConstraints.NORTHWEST;
		add(chkNotificationSound, gbc);
		
		setPreferredSize( getMinimumSize() );
		
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
