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
	
	// generic
	private JTabbedPane tabPane;
	
	// mac unified
	private JPanel cardPanel;
	private CardLayout cardLayout;
	private ToggleButtonToolBar toolbar;
	private ButtonGroup toolbarGroup;
	
	public PreferencesFrame()
	{
		
		super("Dirt Preferences");
		if (MRJAdapter.isSwingUsingScreenMenuBar())
		{
			setJMenuBar(new MainMenuBar());
		}
		if (isMacUnified())
		{
			getRootPane().putClientProperty("apple.awt.brushMetalLook", Boolean.TRUE);
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
	
	public static boolean isMacUnified()
	{
		return
			FileUtil.isMac() &&
			System.getProperty("os.version").startsWith("10.") &&
			System.getProperty("os.version").compareTo("10.5") > 0;
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
		
		if (isMacUnified())
		{
			
			toolbarGroup = new ButtonGroup();
			
			toolbar = new ToggleButtonToolBar();
			toolbar.setFloatable(false);
			getContentPane().add(toolbar, BorderLayout.NORTH);
			
			cardPanel = new JPanel();
			cardPanel.setBackground(SystemColor.control);
			cardPanel.setBorder(BorderFactory.createMatteBorder(1,0,0,0, new Color(0f,0f,0f,0.6f)));
			cardLayout = new CardLayout();
			cardPanel.setLayout(cardLayout);
			getContentPane().add(cardPanel, BorderLayout.CENTER);
			
		}
		else
		{
			
			tabPane = new JTabbedPane();
			tabPane.setBorder(BorderFactory.createEmptyBorder(8,8,0,8));
			getContentPane().add(tabPane, BorderLayout.CENTER);
			
		}
		
		addPanel(new GeneralPreferencesPanel());
		addPanel(new NotificationPreferencesPanel());
		
	}
	
	
	private void addPanel(PreferencesPanel panel)
	{
		
		if (isMacUnified())
		{
			
			cardPanel.add(panel, panel.getName());
			ToolbarAction action = new ToolbarAction(panel.getName(), panel.getIcon());
			JToggleButton btn = toolbar.addToggleButton(action);
			toolbarGroup.add(btn);
			btn.setFocusable(false);
			if (toolbarGroup.getSelection() == null)
			{
			    btn.setSelected(true);
			}
			
		}
		else
		{
			
			tabPane.addTab(panel.getName(), panel);
			
		}
		
	}
	
	private PreferencesPanel[] getPanels()
	{
		if (isMacUnified())
		{
			final PreferencesPanel[] panels = new PreferencesPanel[cardPanel.getComponentCount()];
			int i = 0;
			for (Component c : cardPanel.getComponents())
			{
				panels[i++] = (PreferencesPanel)c;
			}
			return panels;
		}
		else
		{
			final PreferencesPanel[] panels = new PreferencesPanel[tabPane.getTabCount()];
			for (int i = 0; i < tabPane.getTabCount(); ++i)
			{
				panels[i] = (PreferencesPanel)tabPane.getComponentAt(i);
			}
			return panels;
		}
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
		pnlButtons.setBorder(BorderFactory.createEmptyBorder(6,6,6,6));
		
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
	
	protected class ToolbarAction extends AbstractAction
	{
		final String name;
		public ToolbarAction(String name, Icon icon)
		{
			super(name, icon);
			this.name = name;
		}
		public void actionPerformed(ActionEvent e)
		{
			cardLayout.show(cardPanel, name);
		}
	}
	
	protected class ToggleButtonToolBar extends JToolBar
	{
		
		public ToggleButtonToolBar()
		{
			super();
		}
		
		JToggleButton addToggleButton(Action a)
		{
			JToggleButton tb = new JToggleButton(
					(String)a.getValue(Action.NAME),
					(Icon)a.getValue(Action.SMALL_ICON)
				);
			tb.setMargin(new Insets(1,1,1,1));
			tb.setText(null);
			tb.setEnabled(a.isEnabled());
			tb.setToolTipText((String)a.getValue(Action.SHORT_DESCRIPTION));
			tb.setAction(a);
			tb.setVerticalTextPosition(BOTTOM);
			tb.setHorizontalTextPosition(CENTER);
			add(tb);
			return tb;
		}
		
	}
	
}
