package au.com.gslabs.dirt.ui.jfc.client.prefs.panels;

import au.com.gslabs.dirt.ui.jfc.client.prefs.*;
import javax.swing.*;
import au.com.gslabs.dirt.lib.ui.jfc.SpringUtilities;

public class GeneralPreferencesPanel extends PreferencesPanel
{
	
	private static final int PADDING = 6;
	private static final int INIT_LOCATION = 6;
	
	private JTextField txtNickname;
	private JTextField txtDefaultServer;
	private JCheckBox chkLogChat;
	private JCheckBox chkLastReadMarkerClearOnBlur;
	private JCheckBox chkLastReadMarkerClearOnInput;
	
	public GeneralPreferencesPanel()
	{
		
		setLayout(new SpringLayout());
		
		txtNickname = new JTextField();
		txtDefaultServer = new JTextField();
		chkLogChat = new JCheckBox("Log Chat Messages");
		chkLastReadMarkerClearOnBlur = new JCheckBox("Clear Last Read Marker On Focus Lost");
		chkLastReadMarkerClearOnInput = new JCheckBox("Clear Last Read Marker On Input");
		
		trapChangeEvent(txtNickname);
		trapChangeEvent(txtDefaultServer);
		trapChangeEvent(chkLogChat);
		trapChangeEvent(chkLastReadMarkerClearOnBlur);
		trapChangeEvent(chkLastReadMarkerClearOnInput);
		
		add(new JLabel("Preferred Nickname:"));
		add(txtNickname);
		add(new JLabel("Default Server:"));
		add(txtDefaultServer);
		//add(new JLabel());
		//add(chkLogChat);
		
		add(new JLabel());
		add(chkLastReadMarkerClearOnBlur);
		add(new JLabel());
		add(chkLastReadMarkerClearOnInput);
		
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
		txtNickname.setText(getPreferences().getNickname());
		txtDefaultServer.setText(getPreferences().getDefaultServer());
		chkLogChat.setSelected(getPreferences().getLogChatEnabled());
		chkLastReadMarkerClearOnBlur.setSelected(getPreferences().getLastReadMarkerClearOnBlur());
		chkLastReadMarkerClearOnInput.setSelected(getPreferences().getLastReadMarkerClearOnInput());
	}
	
	public boolean save()
	{
		getPreferences().setNickname(txtNickname.getText());
		getPreferences().setDefaultServer(txtDefaultServer.getText());
		getPreferences().setLogChatEnabled(chkLogChat.isSelected());
		getPreferences().setLastReadMarkerClearOnBlur(chkLastReadMarkerClearOnBlur.isSelected());
		getPreferences().setLastReadMarkerClearOnInput(chkLastReadMarkerClearOnInput.isSelected());
		return true;
	}
	
}
