package au.com.gslabs.dirt.ui.jfc.client.prefs;

import java.awt.event.*;
import javax.swing.*;
import au.com.gslabs.dirt.lib.ui.jfc.UIUtil;

public abstract class PreferencesPanel extends JPanel
{
	
	public abstract String getName();
	public abstract void load();
	public abstract boolean save();
	
	protected PreferencesPanel()
	{
		setOpaque(false);
	}
	
	public PreferencesFrame getPreferencesFrame()
	{
		return (PreferencesFrame)UIUtil.getTopLevelWindow(this);
	}
	
	public Preferences getPreferences()
	{
		return getPreferencesFrame().getPreferences();
	}
	
	protected boolean isAutoSave()
	{
		return getPreferencesFrame().isAutoSave();
	}
	
	protected void trapChangeEvent(AbstractButton btn)
	{
		btn.addActionListener(new ActionListener()
			{
				public void actionPerformed(ActionEvent e)
				{
					if (isAutoSave()) save();
				}
			});
	}
	
	protected void trapChangeEvent(JComboBox cmb)
	{
		cmb.addActionListener(new ActionListener()
			{
				public void actionPerformed(ActionEvent e)
				{
					if (isAutoSave()) save();
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
					if (isAutoSave()) save();
				}
			});
	}
	
}
