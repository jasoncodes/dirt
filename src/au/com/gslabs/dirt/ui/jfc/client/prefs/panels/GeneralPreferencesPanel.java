package au.com.gslabs.dirt.ui.jfc.client.prefs.panels;

import java.awt.*;
import javax.swing.*;
import javax.swing.border.*;
import au.com.gslabs.dirt.ui.jfc.client.prefs.*;
import au.com.gslabs.dirt.lib.ui.jfc.SpringUtilities;
import au.com.gslabs.dirt.lib.ui.jfc.UIUtil;

public class GeneralPreferencesPanel extends PreferencesPanel
{
	
	private static final int PADDING = 6;
	private static final int INIT_LOCATION = 6;
	private static final int SIZE_EM = 20;
	
	private JTextField txtNickname;
	private JTextField txtDefaultServer;
	private JCheckBox chkLogChat;
	private JCheckBox chkLastReadMarkerClearOnBlur;
	private JCheckBox chkLastReadMarkerClearOnInput;
	
	public GeneralPreferencesPanel()
	{
		
		super("General", "general");
		
		setLayout(new GridBagLayout());
		GridBagConstraints gbc = new GridBagConstraints();
		gbc.fill = GridBagConstraints.NONE;
		
		// See comment in NotificationPreferencesPanel
		Insets init = new Insets(PADDING, INIT_LOCATION+PADDING, 0, 0);
		Insets norm = new Insets(PADDING, PADDING, 0, PADDING);
		Insets noInsets = new Insets(0,0,0,0);
		Border noBorder = new EmptyBorder(noInsets);
		Dimension txtBoxWidthEms = UIUtil.getWidthEmsDimension( SIZE_EM );
		
		txtNickname = new JTextField();
		txtNickname.setMinimumSize( txtBoxWidthEms );
		txtNickname.setPreferredSize( txtBoxWidthEms );
		txtDefaultServer = new JTextField();
		txtDefaultServer.setMinimumSize( txtBoxWidthEms );
		txtDefaultServer.setPreferredSize( txtBoxWidthEms );
		
		chkLogChat = new JCheckBox("Log Chat Messages");
		chkLastReadMarkerClearOnBlur = new JCheckBox("Clear Last Read Marker On Focus Lost");
		chkLastReadMarkerClearOnBlur.setOpaque(false);
		chkLastReadMarkerClearOnBlur.setMargin(noInsets);
		chkLastReadMarkerClearOnBlur.setBorder(noBorder);
		chkLastReadMarkerClearOnInput = new JCheckBox("Clear Last Read Marker On Input");
		chkLastReadMarkerClearOnInput.setOpaque(false);
		chkLastReadMarkerClearOnInput.setMargin(noInsets);
		chkLastReadMarkerClearOnInput.setBorder(noBorder);
		
		trapChangeEvent(txtNickname);
		trapChangeEvent(txtDefaultServer);
		trapChangeEvent(chkLogChat);
		trapChangeEvent(chkLastReadMarkerClearOnBlur);
		trapChangeEvent(chkLastReadMarkerClearOnInput);
		
		int row = 0;
		
		gbc.gridx = 0;
		gbc.gridy = 0;
		gbc.insets = init;
		gbc.anchor = GridBagConstraints.EAST;
		add(Box.createGlue(), gbc);
		++row;
		
		gbc.gridx = 0;
		gbc.gridy = row;
		gbc.insets = init;
		gbc.anchor = GridBagConstraints.EAST;
		add(new JLabel("Preferred Nickname:"), gbc);
		gbc.gridx = 1;
		gbc.gridy = row;
		gbc.insets = norm;
		gbc.anchor = GridBagConstraints.WEST;
		add(txtNickname, gbc);
		++row;
		
		gbc.gridx = 0;
		gbc.gridy = row;
		gbc.insets = init;
		gbc.anchor = GridBagConstraints.EAST;
		add(new JLabel("Default Server:"), gbc);
		gbc.gridx = 1;
		gbc.gridy = row;
		gbc.insets = norm;
		gbc.anchor = GridBagConstraints.WEST;
		add(txtDefaultServer, gbc);
		++row;
		
		//todo//add(new JLabel());
		//todo//add(chkLogChat);
		
		gbc.gridx = 1;
		gbc.gridy = row;
		gbc.insets = norm;
		gbc.anchor = GridBagConstraints.WEST;
		add(chkLastReadMarkerClearOnBlur, gbc);
		++row;
		
		gbc.gridx = 1;
		gbc.gridy = row;
		gbc.insets = norm;
		gbc.anchor = GridBagConstraints.WEST;
		add(chkLastReadMarkerClearOnInput, gbc);
		++row;
		
		gbc.gridy = row;
		gbc.weightx = 1.0;
		gbc.weighty = 1.0;
		gbc.insets = norm;
		gbc.anchor = GridBagConstraints.NORTHWEST;
		add(Box.createGlue(), gbc);
		++row;
		
	}
	
	public void load()
	{
		setEventsEnabled(false);
		txtNickname.setText(getPreferences().getNickname());
		txtDefaultServer.setText(getPreferences().getDefaultServer());
		chkLogChat.setSelected(getPreferences().getLogChatEnabled());
		chkLastReadMarkerClearOnBlur.setSelected(getPreferences().getLastReadMarkerClearOnBlur());
		chkLastReadMarkerClearOnInput.setSelected(getPreferences().getLastReadMarkerClearOnInput());
		setEventsEnabled(true);
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
