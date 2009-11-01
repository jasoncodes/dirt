package au.com.gslabs.dirt.ui.jfc.client.prefs;

import java.awt.event.*;
import javax.swing.*;
import au.com.gslabs.dirt.lib.ui.jfc.UIUtil;
import au.com.gslabs.dirt.lib.util.FileUtil;

public abstract class PreferencesPanel extends JPanel
{
	
	private final String name;
	private final ImageIcon icon;
	
	private boolean eventsEnabled = true;
	
	public abstract void load();
	public abstract boolean save();
	
	protected PreferencesPanel(String name, String iconName)
	{
		this(name, new ImageIcon(FileUtil.getResource("res/icons/preferences-mac-"+iconName+".png")));
		setBorder(BorderFactory.createEmptyBorder(6,6,6,6));
	}
	
	protected PreferencesPanel(String name, ImageIcon icon)
	{
		this.name = name;
		this.icon = icon;
		setOpaque(false);
	}
	
	public String getName()
	{
		return name;
	}
	
	public ImageIcon getIcon()
	{
		return icon;
	}
	
	protected void setEventsEnabled(boolean eventsEnabled)
	{
		this.eventsEnabled = eventsEnabled;
	}
	
	public PreferencesFrame getPreferencesFrame()
	{
		return (PreferencesFrame)UIUtil.getTopLevelWindow(this);
	}
	
	public Preferences getPreferences()
	{
		return getPreferencesFrame().getPreferences();
	}
	
	private boolean isAutoSave()
	{
		return getPreferencesFrame().isAutoSave();
	}
	
	private void onAutoSaveEvent()
	{
		if (eventsEnabled && isAutoSave())
		{
			save();
		}
	}
	
	protected void trapChangeEvent(AbstractButton btn)
	{
		btn.addActionListener(new ActionListener()
			{
				public void actionPerformed(ActionEvent e)
				{
					onAutoSaveEvent();
				}
			});
	}
	
	protected void trapChangeEvent(JComboBox cmb)
	{
		cmb.addActionListener(new ActionListener()
			{
				public void actionPerformed(ActionEvent e)
				{
					onAutoSaveEvent();
				}
			});
	}
	
	protected void trapChangeEvent(JTextField txt)
	{
		txt.addFocusListener(new FocusListener()
			{
				public void focusGained(FocusEvent e)
				{
				}
				public void focusLost(FocusEvent e)
				{
					onAutoSaveEvent();
				}
			});
	}
	
}
