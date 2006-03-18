package au.com.gslabs.dirt.ui.jfc.client;

import java.util.Locale;
import java.util.ResourceBundle;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import org.jdesktop.jdic.tray.*;
import au.com.gslabs.dirt.lib.util.*;
import au.com.gslabs.dirt.lib.ui.jfc.*;

// this whole file is currently very messy
// when I get around to implementing OS X specifics
// and then abstracting them it will get much cleaner
// this will be done using MRJAdapter
////import com.apple.eawt.*;

public class Client extends JFrame
{

	protected ResourceBundle resbundle;
	protected AboutBox aboutBox;
	protected PrefPane prefs;
	////private Application fApplication = Application.getApplication();
	protected Action newAction, openAction, closeAction, saveAction, saveAsAction,
		undoAction, cutAction, copyAction, pasteAction, clearAction, selectAllAction;
	static final JMenuBar mainMenuBar = new JMenuBar();	
	protected JMenu fileMenu, editMenu; 
	
	public static void init()
	{
		UIUtil.initSwing();
		new Client();
	}
	
	LogPane txtLog;
	InputArea txtInput;
	
	private Client()
	{
		
		super("");
		resbundle = ResourceBundle.getBundle("strings", Locale.getDefault());
		setTitle(resbundle.getString("title"));
		UIUtil.setIcon(this);
		
		createActions();
		addMenus();
		
		this.addWindowListener(new WindowAdapter()
		{
			public void windowClosing(WindowEvent event)
			{
				onClose();
			}
		});	
		
		
		getContentPane().setLayout(new BorderLayout());
		
		JPanel pnl = new JPanel();
		
		pnl.setLayout(new GridLayout(2, 1));
		
		JButton cmdTestAlert = new JButton("Test Alert");
		cmdTestAlert.addActionListener(new ActionListener()
		{
			public void actionPerformed(ActionEvent newEvent)
			{
				cmdTestAlert_Click();
			}
		});
		pnl.add(cmdTestAlert);
		
		JButton cmdTestTray = new JButton("Test Tray");
		cmdTestTray.addActionListener(new ActionListener()
		{
			public void actionPerformed(ActionEvent newEvent)
			{
				cmdTestTray_Click();
			}
		});
		pnl.add(cmdTestTray);
		
		getContentPane().add(pnl, BorderLayout.NORTH);
		
		txtLog = new LogPane();
		getContentPane().add(txtLog, BorderLayout.CENTER);
		
		txtInput = new InputArea();
		getContentPane().add(txtInput, BorderLayout.SOUTH);
		
		txtInput.addInputListener(new InputArea.InputListener()
			{
				public void inputPerformed(InputArea.InputEvent e)
				{
					txtInput_Input(e.getLines());
				}
			});
		txtLog.addLinkListener(new LogPane.LinkListener()
			{
				public void linkClicked(LogPane.LinkEvent e)
				{
					txtLog_LinkClick(e.getURL());
				}
			});
		
		for (int i = 0; i < 3; ++i)
		{
			txtLog.appendText("Line " + i);
		}
		
		/**
		fApplication.setEnabledPreferencesMenu(true);
		fApplication.addApplicationListener(new com.apple.eawt.ApplicationAdapter() {
			public void handleAbout(ApplicationEvent e) {
				if (aboutBox == null) {
					aboutBox = new AboutBox();
				}
				about(e);
				e.setHandled(true);
			}
			public void handleOpenApplication(ApplicationEvent e) {
			}
			public void handleOpenFile(ApplicationEvent e) {
			}
			public void handlePreferences(ApplicationEvent e) {
				if (prefs == null) {
					prefs = new PrefPane();
				}
				preferences(e);
			}
			public void handlePrintFile(ApplicationEvent e) {
			}
			public void handleQuit(ApplicationEvent e) {
				quit(e);
			}
		});
		**/
		
		setSize(350, 250);
		setVisible(true);
	}
	
	protected void txtLog_LinkClick(java.net.URL url)
	{
		txtLog.appendText("Link clicked: " + url, "info");
	}
	
	protected void txtInput_Input(String[] lines)
	{
		for (String line : lines)
		{
			if (!line.startsWith("/"))
			{
				throw new IllegalArgumentException("Expected input to start with slash");
			}
			String cmd, params;
			int i = line.indexOf(" ");
			if (i < 0)
			{
				cmd = line.substring(1);
				params = "";
			}
			else
			{
				cmd = line.substring(1, i);
				params = line.substring(i+1);
			}
			cmd = cmd.toUpperCase().trim();
			params = params.trim();
			if (cmd.equals("SAY"))
			{
				txtLog.appendText(params);
			}
			else if (cmd.equals("XHTML"))
			{
				try
				{
					txtLog.appendXHTMLFragment(params, null);
				}
				catch (Exception ex)
				{
					Throwable t = ex.getCause();
					if (t != null)
					{
						while (t != null)
						{
							txtLog.appendText(t.toString(), "error");
							t = t.getCause();
						}
					}
					else
					{
						txtLog.appendText(ex.toString(), "error");
					}
				}
			}
			else if (cmd.equals("CLEAR"))
			{
				txtLog.clearText();
			}
			else if (cmd.equals("HELP"))
			{
				txtLog.appendText("Supported commands: CLEAR HELP SAY XHTML", "info");
			}
			else
			{
				txtLog.appendText("Unknown command: " + cmd, "error");
			}
		}
	}
	
	protected void cmdPopupQuit_Click()
	{
		System.exit(0);
	}
	
	protected void cmdTestAlert_Click()
	{
		new Thread(
			new Runnable()
			{
				public void run()
				{
					try
					{
						Thread.currentThread().sleep(1000);
					}
					catch (Exception ex)
					{
					}
					UIUtil.alert(Client.this);
				}
			}).start();
	}
	
	protected void cmdTestTray_Click()
	{
	
		try
		{
			if (FileUtil.isWin())
			{
				FileUtil.loadLibrary("lib/win32/tray.dll");
			}
			if (FileUtil.isLinux())
			{
				FileUtil.loadLibrary("lib/linux_x86/libtray.so");
			}
		}
		catch (Exception e)
		{
			System.err.println("Error loading native tray library"); 
			return;
		}	
		
		SystemTray tray = SystemTray.getDefaultSystemTray();
		TrayIcon ti;
		
		JPopupMenu menu;
		JMenu  submenu;
		JMenuItem menuItem;
		JRadioButtonMenuItem rbMenuItem;
		JCheckBoxMenuItem cbMenuItem;
		
		if (Integer.parseInt(System.getProperty("java.version").substring(2,3)) >=5)
		{
			System.setProperty("javax.swing.adjustPopupLocationToFit", "false");
		}
		
		menu = new JPopupMenu("A Menu");
		
		// "Quit" menu item
		menu.addSeparator();
		menuItem = new JMenuItem("Quit");
		menuItem.addActionListener(new ActionListener()
			{
				public void actionPerformed(ActionEvent evt)
				{
					cmdPopupQuit_Click();
				}
			});
		menu.add(menuItem);
		
		ti = new TrayIcon(UIUtil.loadImageIcon(true), "Dirt Secure Chat", menu);
		
		ti.setIconAutoSize(true);
		ti.addActionListener(new ActionListener()
			{
				long firstWhen = 0;
				public void actionPerformed(ActionEvent e)
				{
					if (firstWhen != 0 && (e.getWhen() - firstWhen) <= 700)
					{
						firstWhen = 0;
						Popup_DblClick();
					}
					else
					{
						firstWhen = e.getWhen();
					}
				}
			});
		
		tray.addTrayIcon(ti);
	
	}

	protected void Popup_DblClick()
	{
		setVisible(!isVisible());
		if (isVisible())
		{	
			UIUtil.stealFocus(this);
		}
	}
		
	protected void onClose()
	{
		setVisible(false);
		if (!FileUtil.isMac())
		{
			dispose();
			System.exit(0);
		}
	}
	
	/**
	public void about(ApplicationEvent e)
	{
		aboutBox.setResizable(false);
		aboutBox.setVisible(true);
	}
	
	public void preferences(ApplicationEvent e)
	{
		prefs.setResizable(false);
		prefs.setVisible(true);
	}
	
	public void quit(ApplicationEvent e) {	
		System.exit(0);
	}
	**/
	
	public void createActions() {
		int shortcutKeyMask = Toolkit.getDefaultToolkit().getMenuShortcutKeyMask();
		
		//Create actions that can be used by menus, buttons, toolbars, etc.
		newAction = new newActionClass( resbundle.getString("newItem"),
										KeyStroke.getKeyStroke(KeyEvent.VK_N, shortcutKeyMask) );
		openAction = new openActionClass( resbundle.getString("openItem"),
										  KeyStroke.getKeyStroke(KeyEvent.VK_O, shortcutKeyMask) );
		closeAction = new closeActionClass( resbundle.getString("closeItem"),
											KeyStroke.getKeyStroke(KeyEvent.VK_W, shortcutKeyMask) );
		saveAction = new saveActionClass( resbundle.getString("saveItem"),
										  KeyStroke.getKeyStroke(KeyEvent.VK_S, shortcutKeyMask) );
		saveAsAction = new saveAsActionClass( resbundle.getString("saveAsItem") );
		
		undoAction = new undoActionClass( resbundle.getString("undoItem"),
										  KeyStroke.getKeyStroke(KeyEvent.VK_Z, shortcutKeyMask) );
		cutAction = new cutActionClass( resbundle.getString("cutItem"),
										KeyStroke.getKeyStroke(KeyEvent.VK_X, shortcutKeyMask) );
		copyAction = new copyActionClass( resbundle.getString("copyItem"),
										  KeyStroke.getKeyStroke(KeyEvent.VK_C, shortcutKeyMask) );
		pasteAction = new pasteActionClass( resbundle.getString("pasteItem"),
											KeyStroke.getKeyStroke(KeyEvent.VK_V, shortcutKeyMask) );
		clearAction = new clearActionClass( resbundle.getString("clearItem") );
		selectAllAction = new selectAllActionClass( resbundle.getString("selectAllItem"),
													KeyStroke.getKeyStroke(KeyEvent.VK_A, shortcutKeyMask) );
	}
	
	public void addMenus() {
		
		fileMenu = new JMenu(resbundle.getString("fileMenu"));
		fileMenu.add(new JMenuItem(newAction));
		fileMenu.add(new JMenuItem(openAction));
		fileMenu.add(new JMenuItem(closeAction));
		fileMenu.add(new JMenuItem(saveAction));
		fileMenu.add(new JMenuItem(saveAsAction));
		mainMenuBar.add(fileMenu);
		
		editMenu = new JMenu(resbundle.getString("editMenu"));
		editMenu.add(new JMenuItem(undoAction));
		editMenu.addSeparator();
		editMenu.add(new JMenuItem(cutAction));
		editMenu.add(new JMenuItem(copyAction));
		editMenu.add(new JMenuItem(pasteAction));
		editMenu.add(new JMenuItem(clearAction));
		editMenu.addSeparator();
		editMenu.add(new JMenuItem(selectAllAction));
		mainMenuBar.add(editMenu);
		
		setJMenuBar (mainMenuBar);
	}
		
	public class newActionClass extends AbstractAction {
		public newActionClass(String text, KeyStroke shortcut) {
			super(text);
			putValue(ACCELERATOR_KEY, shortcut);
		}
		public void actionPerformed(ActionEvent e) {
			System.out.println("New...");
		}
	}
	
	public class openActionClass extends AbstractAction {
		public openActionClass(String text, KeyStroke shortcut) {
			super(text);
			putValue(ACCELERATOR_KEY, shortcut);
		}
		public void actionPerformed(ActionEvent e) {
			System.out.println("Open...");
		}
	}
	
	public class closeActionClass extends AbstractAction {
		public closeActionClass(String text, KeyStroke shortcut) {
			super(text);
			putValue(ACCELERATOR_KEY, shortcut);
		}
		public void actionPerformed(ActionEvent e) {
			dispose();
		}
	}
	
	public class saveActionClass extends AbstractAction {
		public saveActionClass(String text, KeyStroke shortcut) {
			super(text);
			putValue(ACCELERATOR_KEY, shortcut);
		}
		public void actionPerformed(ActionEvent e) {
			System.out.println("Save...");
		}
	}
	
	public class saveAsActionClass extends AbstractAction {
		public saveAsActionClass(String text) {
			super(text);
		}
		public void actionPerformed(ActionEvent e) {
			System.out.println("Save As...");
		}
	}
	
	public class undoActionClass extends AbstractAction {
		public undoActionClass(String text, KeyStroke shortcut) {
			super(text);
			putValue(ACCELERATOR_KEY, shortcut);
		}
		public void actionPerformed(ActionEvent e) {
			System.out.println("Undo...");
		}
	}
	
	public class cutActionClass extends AbstractAction {
		public cutActionClass(String text, KeyStroke shortcut) {
			super(text);
			putValue(ACCELERATOR_KEY, shortcut);
		}
		public void actionPerformed(ActionEvent e) {
			System.out.println("Cut...");
		}
	}
	
	public class copyActionClass extends AbstractAction {
		public copyActionClass(String text, KeyStroke shortcut) {
			super(text);
			putValue(ACCELERATOR_KEY, shortcut);
		}
		public void actionPerformed(ActionEvent e) {
			System.out.println("Copy...");
		}
	}
	
	public class pasteActionClass extends AbstractAction {
		public pasteActionClass(String text, KeyStroke shortcut) {
			super(text);
			putValue(ACCELERATOR_KEY, shortcut);
		}
		public void actionPerformed(ActionEvent e) {
			System.out.println("Paste...");
		}
	}
	
	public class clearActionClass extends AbstractAction {
		public clearActionClass(String text) {
			super(text);
		}
		public void actionPerformed(ActionEvent e) {
			System.out.println("Clear...");
		}
	}
	
	public class selectAllActionClass extends AbstractAction {
		public selectAllActionClass(String text, KeyStroke shortcut) {
			super(text);
			putValue(ACCELERATOR_KEY, shortcut);
		}
		public void actionPerformed(ActionEvent e) {
			System.out.println("Select All...");
		}
	}
	
}
