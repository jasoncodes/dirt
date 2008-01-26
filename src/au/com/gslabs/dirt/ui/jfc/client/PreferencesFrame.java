package au.com.gslabs.dirt.ui.jfc.client;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import au.com.gslabs.dirt.lib.util.FileUtil;
import au.com.gslabs.dirt.lib.ui.jfc.UIUtil;
import au.com.gslabs.dirt.lib.ui.jfc.SpringUtilities;
import net.roydesign.mac.MRJAdapter;

public class PreferencesFrame extends JFrame
{
	
	private static final int PADDING = 6;
	private static final int INIT_LOCATION = 6;
	
	private final boolean autoSave = FileUtil.isMac();
	private final Preferences prefs = Preferences.getInstance();
	
	private JTextField txtNickname;
	private JTextField txtDefaultServer;
	private JCheckBox chkLogChat;
	private JCheckBox chkNotificationSound;
	
	public PreferencesFrame()
	{
		
		super("Preferences");
		if (MRJAdapter.isSwingUsingScreenMenuBar())
		{
			setJMenuBar(new MainMenuBar());
		}
		UIUtil.setIcon(this);
		setDefaultCloseOperation(WindowConstants.DO_NOTHING_ON_CLOSE);
		setLocationRelativeTo(null);
		
		addWindowListener(new WindowAdapter()
			{
				@Override
				public void windowClosing(WindowEvent e)
				{
					if (!autoSave || save())
					{
						dispose();
					}
				}
				@Override
				public void windowDeactivated(WindowEvent e)
				{
					if (autoSave) save();
				}
			});
		
		createMainPanel();
		createButtonPanel();
		
		load();
		
		pack();
		
	}
	
	private void trapChangeEvent(AbstractButton btn)
	{
		btn.addActionListener(new ActionListener()
			{
				public void actionPerformed(ActionEvent e)
				{
					if (autoSave) save();
				}
			});
	}
	
	private void trapChangeEvent(JTextField txt)
	{
		txt.addFocusListener(new FocusListener()
			{
				public void focusGained(FocusEvent e)
				{
				}
				public void focusLost(FocusEvent e)
				{
					if (autoSave) save();
				}
			});
	}
	
	private void createMainPanel()
	{
		
		JPanel mainPanel = new JPanel();
		mainPanel.setLayout(new SpringLayout());
		
		txtNickname = new JTextField();
		txtDefaultServer = new JTextField();
		chkLogChat = new JCheckBox("Log Chat Messages");
		chkNotificationSound = new JCheckBox("Notification Sounds");
		
		trapChangeEvent(txtNickname);
		trapChangeEvent(txtDefaultServer);
		trapChangeEvent(chkLogChat);
		trapChangeEvent(chkNotificationSound);
		
		mainPanel.add(new JLabel("Preferred Nickname:"));
		mainPanel.add(txtNickname);
		mainPanel.add(new JLabel("Default Server:"));
		mainPanel.add(txtDefaultServer);
		//mainPanel.add(new JLabel());
		//mainPanel.add(chkLogChat);
		mainPanel.add(new JLabel());
		mainPanel.add(chkNotificationSound);
		
		SpringUtilities.makeCompactGrid(
			mainPanel, mainPanel.getComponentCount()/2, 2, 
			INIT_LOCATION, INIT_LOCATION, 
			PADDING, PADDING);
		
		getContentPane().add(mainPanel, BorderLayout.CENTER);
		
	}
	
	private void load()
	{
		txtNickname.setText(prefs.getNickname());
		txtDefaultServer.setText(prefs.getDefaultServer());
		chkLogChat.setSelected(prefs.getLogChatEnabled());
		chkNotificationSound.setSelected(prefs.getNotificationSoundEnabled());
	}
	
	private boolean save()
	{
		prefs.setNickname(txtNickname.getText());
		prefs.setDefaultServer(txtDefaultServer.getText());
		prefs.setLogChatEnabled(chkLogChat.isSelected());
		prefs.setNotificationSoundEnabled(chkNotificationSound.isSelected());
		return true;
	}
	
	private void createButtonPanel()
	{
		
		JPanel pnlButtons = new JPanel();
		pnlButtons.setLayout(new FlowLayout(FlowLayout.RIGHT));
		
		JButton btnSave = new JButton("Save");
		btnSave.addActionListener(new ActionListener()
			{
				public void actionPerformed(ActionEvent e)
				{
					if (save())
					{
						dispose();
					}
				}
			});
		
		final JButton btnClose = new JButton("Cancel");
		btnClose.addActionListener(new ActionListener()
			{
				public void actionPerformed(ActionEvent e)
				{
					dispose();
				}
			});
		
		if (!autoSave)
		{
			pnlButtons.add(btnSave);
			pnlButtons.add(btnClose);
		}
		
		if (pnlButtons.getComponentCount() > 0)
		{
			getContentPane().add(pnlButtons, BorderLayout.SOUTH);
		}
		
	}
	
}
