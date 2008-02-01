package au.com.gslabs.dirt.ui.jfc.client.prefs;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import au.com.gslabs.dirt.lib.util.FileUtil;
import au.com.gslabs.dirt.lib.ui.jfc.UIUtil;
import au.com.gslabs.dirt.ui.jfc.client.MainMenuBar;
import net.roydesign.mac.MRJAdapter;
import au.com.gslabs.dirt.ui.jfc.client.prefs.panels.*;

public class PreferencesFrame extends JFrame
{
	
	private final boolean autoSave = FileUtil.isMac();
	private final Preferences prefs = Preferences.getInstance();
	
	private JTabbedPane tabPane;
	
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
	
	public Preferences getPreferences()
	{
		return prefs;
	}
	
	public boolean isAutoSave()
	{
		return autoSave;
	}
	
	private void createMainPanel()
	{
		
		tabPane = new JTabbedPane();
		getContentPane().add(tabPane, BorderLayout.CENTER);
		
		addPanel(new GeneralPreferencesPanel());
		addPanel(new NotificationPreferencesPanel());
		
	}
	
	
	private void addPanel(PreferencesPanel panel)
	{
		tabPane.addTab(panel.getName(), panel);
	}
	
	private PreferencesPanel[] getPanels()
	{
		final PreferencesPanel[] panels = new PreferencesPanel[tabPane.getTabCount()];
		for (int i = 0; i < tabPane.getTabCount(); ++i)
		{
			panels[i] = (PreferencesPanel)tabPane.getComponentAt(i);
		}
		return panels;
	}
	
	private void load()
	{
		for (PreferencesPanel panel : getPanels())
		{
			panel.load();
		}
	}
	
	private boolean save()
	{
		for (PreferencesPanel panel : getPanels())
		{
			if (!panel.save())
			{
				// todo: focus relevant tab
				return false;
			}
		}
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
