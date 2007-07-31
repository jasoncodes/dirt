package au.com.gslabs.dirt.ui.jfc.client;

import java.util.Locale;
import java.util.ResourceBundle;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import org.jdesktop.jdic.tray.*;
import au.com.gslabs.dirt.lib.util.*;
import au.com.gslabs.dirt.lib.ui.jfc.*;
import au.com.gslabs.dirt.core.client.*;

// this needs MRJAdapter support

public class MainFrame extends JFrame
{

	protected ResourceBundle resbundle;
	protected Action newAction, openAction, closeAction, saveAction, saveAsAction,
		undoAction, cutAction, copyAction, pasteAction, clearAction, selectAllAction;
	static final JMenuBar mainMenuBar = new JMenuBar();	
	protected JMenu fileMenu, editMenu; 
	protected Client client;
	
	public static void init()
	{
		UIUtil.initSwing();
		new MainFrame();
	}
	
	LogPane txtLog;
	InputArea txtInput;
	JPasswordField txtPassword;
	
	private MainFrame()
	{
		
		super("");
		resbundle = ResourceBundle.getBundle("strings", Locale.getDefault());
		setTitle(resbundle.getString("title"));
		UIUtil.setIcon(this);
		
		this.addWindowListener(new WindowAdapter()
		{
			
			public void windowClosing(WindowEvent event)
			{
				onClose();
			}
			
			public void windowActivated(WindowEvent e)
			{
				txtInput.requestFocusInWindow();
			}
			
		});
		
		getContentPane().setLayout(new BorderLayout());
		
		txtInput = new InputArea();
		txtInput.addInputListener(new InputArea.InputListener()
			{
				public void inputPerformed(InputArea.InputEvent e)
				{
					txtInput_Input(e.getLines());
				}
			});
		
		txtPassword = new JPasswordField();
		txtPassword.addActionListener(new ActionListener()
			{
				public void actionPerformed(ActionEvent e)
				{
					String password = new String(txtPassword.getPassword());
					txtPassword.setText("");
					txtPassword_Input(password);
				}
			});
		
		getContentPane().add(txtInput, BorderLayout.SOUTH);
		
		txtLog = new LogPane();
		getContentPane().add(txtLog, BorderLayout.CENTER);
		
		txtLog.addLinkListener(new LogPane.LinkListener()
			{
				public void linkClicked(LogPane.LinkEvent e)
				{
					txtLog_LinkClick(e.getURL());
				}
			});
		
		client = new Client();
		client.addClientListener(new ClientAdapter(), new JFCInvoker());
		
		WindowAdapter wa = new WindowAdapter()
			{
				public void windowDeactivated(WindowEvent e)
				{
					txtLog.clearRedLine();
				}
				public void windowLostFocus(WindowEvent e)
				{
					txtLog.clearRedLine();
				}
			};
		addWindowListener(wa);
		addWindowFocusListener(wa);
		
		setSize(850, 330);
		setVisible(true);
		txtInput.requestFocusInWindow();
		
	}
	
	protected void setPasswordMode(boolean passwordMode)
	{
		JComponent txtOld = passwordMode ? txtInput : txtPassword;
		JComponent txtNew = passwordMode ? txtPassword : txtInput;
		txtPassword.setPreferredSize(txtInput.getPreferredSize());
		txtPassword.setText("");
		getContentPane().remove(txtOld);
		getContentPane().add(txtNew, BorderLayout.SOUTH);
		getContentPane().validate();
		if (UIUtil.getActiveWindow() == this)
		{
			txtNew.requestFocusInWindow();
		}
	}
	
	protected enum SupportedCommand
	{
		CLEAR,
		MINTOTRAY
	}
	
	protected class ClientAdapter extends EnumClientAdapter<SupportedCommand>
	{
		
		public ClientAdapter()
		{
			super(SupportedCommand.class);
		}
		
		protected void clientConsoleOutput(Client source, String context, String className, String message)
		{
			if (UIUtil.getActiveWindow() != MainFrame.this)
			{
				txtLog.setRedLine();
				UIUtil.alert(MainFrame.this);
			}
			txtLog.appendTextLine(getOutputPrefix() + message, className);
		}
		
		protected boolean clientPreprocessConsoleInput(Client source, String context, SupportedCommand cmd, String params)
		{
			
			switch (cmd)
			{
				
				case CLEAR:
					txtLog.clearText();
					return true;
				
				case MINTOTRAY:
					doMinToTray();
					return true;
				
				/*
				case TEST:
					for (int i = 0; i < 10; i++)
					{
						StringBuilder sb = new StringBuilder();
						for (int j = 0; j < 7; ++j)
						{
							sb.append("Line ");
							sb.append(i);
							sb.append(' ');
						}
						String tmp = sb.toString().trim();
						txtLog.appendTextLine(tmp);
					}
					txtLog.appendTextLine("Testing 1 2 3. http://dirt.gslabs.com.au/.");
					char ctrl_b = '\u0002';
					char ctrl_c = '\u0003';
					char ctrl_r = '\u0016';
					char ctrl_u = '\u001f';
					txtLog.appendTextLine("this " + ctrl_b + "is" + ctrl_b + " " + ctrl_u + "a " + ctrl_c + "9,1test" + ctrl_c + " line");
					txtLog.appendXHTMLLine("alpha <span style=\"color: green\">beta</span> <span style=\"background: yellow;\">delta</span> gamma -- green white black yellow");
					txtLog.appendXHTMLLine("alpha <span style=\"background-color: yellow\">beta</span> <span style=\"color: green\">delta</span> gamma -- black yellow green white");
					txtLog.appendXHTMLLine("alpha <span style=\"color: green\">beta <span style=\"background: yellow\">delta</span></span> gamma -- green white green yellow");
					txtLog.appendXHTMLLine("<span style=\"background: #e0e0e0\"><span style=\"color: #000080\">these words should be on a single line</span></span>");
					txtLog.appendTextLine(ctrl_c + "9,1green black " + ctrl_c + "4red black");
					txtLog.appendTextLine(ctrl_c + "9,1green black" + ctrl_c + " black white");
					txtLog.appendTextLine(ctrl_c + "3,green");
					txtLog.appendXHTMLLine("no <span style=\"background: yellow\"></span>colour<span style=\"background: #e0e0e0\"></span> on <b></b>this <span style=\"color: red\"></span>line");
					txtLog.appendXHTMLLine("a single 'x' with yellow bg --&gt; <span style=\"background: yellow\">x</span> &lt;--");
					txtLog.appendTextLine(ctrl_c + "2,15blue-grey " + ctrl_r + "reverse" + ctrl_r + " blue-grey " + ctrl_c + "4red-grey " + ctrl_r + "rev" + ctrl_c + ctrl_c + "2erse" + ctrl_r + " blue-white " + ctrl_c + "black-white " + ctrl_r + "reverse");
					txtLog.appendTextLine("Should have two spaces between letters: " + ctrl_c + "1t " + ctrl_c + "1 " + ctrl_c + "1e " + ctrl_c + " " + ctrl_c + "1s  t !");
					txtLog.appendTextLine("Space Test: 1 2  3   4    . exclamation line up -> !");
					return true;
				*/
					
				default:
					return false;
				
			}
			
		}
		
		public void clientNeedNickname(Client source, String defaultNick)
		{
			String prompt = "/nick " + defaultNick;
			txtInput.setText(prompt, 6, prompt.length());
		}
		
		public void clientNeedAuthentication(Client source, String prompt)
		{
			super.clientNeedAuthentication(source, prompt);
			setPasswordMode(true);
		}
		
		public void clientStateChanged(Client source)
		{
			super.clientStateChanged(source);
			setPasswordMode(false);
		}
		
	}
	
	protected void txtLog_LinkClick(java.net.URL url)
	{
		UIUtil.openURL(url.toString());
	}
	
	protected void txtInput_Input(String[] lines)
	{
		txtLog.clearRedLine();
		client.processConsoleInput(null, lines);
	}
	
	protected void txtPassword_Input(String password)
	{
		txtLog.clearRedLine();
		client.authenticate(null, password);
	}
	
	protected void cmdPopupQuit_Click()
	{
		System.exit(0);
	}
	
	protected void doMinToTray()
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
		
		org.jdesktop.jdic.tray.SystemTray tray =
			org.jdesktop.jdic.tray.SystemTray.getDefaultSystemTray();
		org.jdesktop.jdic.tray.TrayIcon ti;
		
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
		
		ti = new org.jdesktop.jdic.tray.TrayIcon(
			UIUtil.loadImageIcon(true),
			"Dirt Secure Chat", menu);
		
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

}
