package au.com.gslabs.dirt;

import java.util.Locale;
import java.util.ResourceBundle;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import java.net.URL;
import java.net.URLClassLoader;
import java.io.File;
import java.lang.reflect.*;
import au.com.gslabs.dirt.jni.*;
import org.jdesktop.jdic.tray.*;

////import com.apple.eawt.*;

public class Dirt extends JFrame
{

	private Font font = new Font("serif", Font.ITALIC+Font.BOLD, 36);
	protected ResourceBundle resbundle;
	protected AboutBox aboutBox;
	protected PrefPane prefs;
	////private Application fApplication = Application.getApplication();
	protected Action newAction, openAction, closeAction, saveAction, saveAsAction,
		undoAction, cutAction, copyAction, pasteAction, clearAction, selectAllAction;
	static final JMenuBar mainMenuBar = new JMenuBar();	
	protected JMenu fileMenu, editMenu; 
	
	public Dirt() {
		
		super("");
		// The ResourceBundle below contains all of the strings used in this
		// application.  ResourceBundles are useful for localizing applications.
		// New localities can be added by adding additional properties files.
		resbundle = ResourceBundle.getBundle ("strings", Locale.getDefault());
		setTitle(resbundle.getString("frameConstructor"));
		setIconImage(loadImageIcon(false).getImage());
		
		createActions();
		addMenus();
		
		this.addWindowListener(new WindowAdapter()
		{
			public void windowClosing(WindowEvent event)
			{
				onClose();
			}
		});	
		
		getContentPane().setLayout(new GridLayout(2, 1));
		
		JButton cmdTestAlert = new JButton("Test Alert");
		cmdTestAlert.addActionListener(new ActionListener()
		{
			public void actionPerformed(ActionEvent newEvent)
			{
				cmdTestAlert_Click();
			}
		});
		this.getContentPane().add(cmdTestAlert);
		
		JButton cmdTestTray = new JButton("Test Tray");
		cmdTestTray.addActionListener(new ActionListener()
		{
			public void actionPerformed(ActionEvent newEvent)
			{
				cmdTestTray_Click();
			}
		});
		this.getContentPane().add(cmdTestTray);
	
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
		
		setSize(310, 150);
		setVisible(true);
	}
	
	protected void cmdPopupQuit_Click()
	{
		System.exit(0);
	}
	
	protected ImageIcon loadImageIcon(boolean small)
	{
		ImageIcon icon;
		if (Util.isWin())
		{
			icon = new ImageIcon(getClass().getClassLoader().getResource("res/dirt32.png"));
			if (small)
			{
			//	icon = new ImageIcon(icon.getImage().getScaledInstance(16, -1, Image.SCALE_SMOOTH));
			}
		}
		else
		{
			icon = new ImageIcon(getClass().getClassLoader().getResource("res/dirt2.png"));
		}
		return icon;
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
					Alert();
				}
			}).start();
	}
	
	protected void cmdTestTray_Click()
	{
	
		try
		{
		if (Util.isWin())
		{
		Util.loadLibrary("lib/win32/tray.dll");
		}
		if (Util.isLinux())
		{
		Util.loadLibrary("lib/linux_x86/libtray.so");
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
		
		ti = new TrayIcon(loadImageIcon(true), "Dirt Secure Chat", menu);
		
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
		if (isVisible() && Util.isWin())
		{	
			try
			{
				Util.loadLibrary("lib/win32/dirt_jni.dll");
				Win32 win32 = new Win32();
				win32.stealFocus(this);
			}
			catch (Exception ex)
			{
				ex.printStackTrace();
			}
		}
	}
		
	protected void Alert()
	{
		try
		{
			if (Util.isWin())
			{
				if (!isFocused())
				{
					Util.loadLibrary("lib/win32/dirt_jni.dll");
					Win32 win32 = new Win32();
					win32.alert(this);
				}
			}
			else if (Util.isLinux())
			{
				if (!isFocused())
				{
					Util.loadLibrary("lib/linux_x86/libdirt_jni.so");
					Linux linux = new Linux();
					linux.alert(this);
				}
			}
			else if (Util.isMac())
			{
				Class c = Class.forName("org.jdesktop.jdic.misc.Alerter");
				Method new_instance = c.getMethod("newInstance", (Class[])null);
				Object alerter = new_instance.invoke(null, (Object[])null);
				Method alert = c.getMethod("alert", new Class[] { Class.forName("java.awt.Frame") });
				alert.invoke(alerter, new Object[] { this });
			}
			else
			{
				throw new Exception("Unsupported OS");
			}
		}
		catch (Exception e)
		{
			// todo: provide alternate alert (window caption hacking?)
			System.err.println("Alert() not available");
			System.err.println(e);
			e.printStackTrace();
		}
	}
	
	protected void onClose()
	{
		setVisible(false);
		if (!Util.isMac())
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
			System.out.println("Close...");
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
	
	public static void main(String args[])
	{
	
		Util.ShutdownCleanupCheck(args);

		try
		{
    		UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
		}
		catch (Exception ex)
		{
		}
		
		System.setProperty("apple.laf.useScreenMenuBar", "true");
		
		new Dirt();
		
	}
	
}
