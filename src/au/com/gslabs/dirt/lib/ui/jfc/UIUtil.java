package au.com.gslabs.dirt.lib.ui.jfc;

import java.io.File;
import au.com.gslabs.dirt.lib.ui.jfc.jni.*;
import java.awt.*;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.lang.reflect.*;
import javax.swing.*;
import au.com.gslabs.dirt.lib.util.FileUtil;
import net.roydesign.app.Application;

public class UIUtil
{
	
	protected static MacDockBouncer dockBouncer = null;
	protected static WindowManager windowManager = null;
	
	private UIUtil()
	{
	}
	
	public static Window getActiveWindow()
	{
		return java.awt.KeyboardFocusManager.getCurrentKeyboardFocusManager().getActiveWindow();
	}
	
	public static Window getTopLevelWindow(Component c)
	{
		while (c.getParent() != null)
		{
			c = c.getParent();
		}
		return (c instanceof Window) ? (Window)c : null;
	}
	
	public static void setWin32Icon(Frame frame, String path)
	{
		if (FileUtil.isWin())
		{
			try
			{
				loadLibrary();
				File temp = FileUtil.getNewTempFile();
				java.io.InputStream in = FileUtil.getResourceAsStream(path);
				java.io.FileOutputStream out = new java.io.FileOutputStream(temp);
				FileUtil.copy(in, out);
				in.close();
				out.close();
				frame.pack(); // ensure this frame is realised
				Win32 win32 = new Win32();
				win32.setIcon(frame, temp.getAbsolutePath());
			}
			catch (Throwable ex)
			{
				ex.printStackTrace();
			}
		}
	}
	
	public static void setIcon(Frame frame)
	{
		frame.setIconImage(loadImageIcon(false).getImage());
		setWin32Icon(frame, "res/icons/dirt.ico");
	}
	
	public static ImageIcon loadImageIcon(int size)
	{
		ImageIcon icon = new ImageIcon(FileUtil.getResource("res/icons/dirt.png"));
		Image image = icon.getImage().getScaledInstance(size, size, Image.SCALE_SMOOTH);
		return new ImageIcon(image);
	}
	
	public static ImageIcon loadImageIcon(boolean small)
	{
		String path;
		if (FileUtil.isWin())
		{
			path = small ? "res/icons/dirt16.png" : "res/icons/dirt32.png";
		}
		else if (FileUtil.isMac())
		{
			path = "res/icons/dirt.png";
		}
		else
		{
			path = "res/icons/dirt2.png";
		}
		return new ImageIcon(FileUtil.getResource(path));
	}
	
	public static void openURL(String url)
	{
		try
		{
			if (FileUtil.isMac())
			{
				Class fileMgr = Class.forName("com.apple.eio.FileManager");
				Method openURL = fileMgr.getDeclaredMethod("openURL", new Class[] {String.class});
				openURL.invoke(null, new Object[] {url});
			}
			else if (FileUtil.isWin())
			{
				Runtime.getRuntime().exec("rundll32 url.dll,FileProtocolHandler " + url);
			}
			else
			{
				
				String[] browsers = { "firefox", "konqueror", "mozilla", "netscape" };
				String browser = null;
				for (int count = 0; count < browsers.length && browser == null; count++)
				{
					if (Runtime.getRuntime().exec(new String[] {"which", browsers[count]}).waitFor() == 0)
					{
						browser = browsers[count];
					}
				}
				
				if (browser == null)
				{
					throw new Exception("Could not find web browser");
				}
				else
				{
					Runtime.getRuntime().exec(new String[] {browser, url});
				}
				
			}
		}
		catch (Exception e)
		{
			JOptionPane.showMessageDialog(null, "Error opening URL:\n" + e.getLocalizedMessage());
		}
	}
	
	/**
	 * Returns window bounds suitable for the specified width and height
	 */
	public static Rectangle getDefaultWindowBounds(int width, int height)
	{
		
		Rectangle rectMax = GraphicsEnvironment.getLocalGraphicsEnvironment().getMaximumWindowBounds();
		
		width = Math.min(width, rectMax.width);
		height = Math.min(height, rectMax.height);
		
		Rectangle rect = new Rectangle(width, height);
		rect.x = rectMax.x + (rectMax.width-width)*7/8;
		rect.y = rectMax.y + (rectMax.height-height)*9/10;
		
		// if the gap on the bottom is bigger than the gap on the right
		int gapBottom = (rectMax.y+rectMax.height)-(rect.y+rect.height);
		int gapRight = (rectMax.x+rectMax.width)-(rect.x+rect.width);
		if (gapBottom > gapRight)
		{
			// make the bottom gap match the right gap
			rect.y = rectMax.y+rectMax.height-rect.height - gapRight;
		}
		
		return rect;
		
	}
	
	public static Frame getFirstValidFrame()
	{
		
		Frame selected;
		
		final Window activeWindow = UIUtil.getActiveWindow();
		if (UIUtil.isValidWindow(activeWindow) && activeWindow instanceof Frame)
		{
			selected = (Frame)activeWindow;
		}
		else
		{
			selected = null;
			for (Frame f : getWindowManager().getOrderedValidFrames(true))
			{
				selected = f;
				break;
			}
		}
		
		return selected;
		
	}
	
	public static WindowManager getWindowManager()
	{
		return windowManager;
	}
	
	public static void setDefaultWindowBounds(final JFrame frame, final int width, final int height, final Class cascadeClass)
	{
		
		Rectangle bounds = getDefaultWindowBounds(width, height);
		
		if (cascadeClass != null)
		{
			for (final Frame f : getWindowManager().getOrderedValidFrames(true))
			{
				if (f != frame && cascadeClass.isInstance(f))
				{
					// found a window to cascade from
					final Rectangle srcBounds = f.getBounds();
					bounds.x = srcBounds.x + 20;
					bounds.y = srcBounds.y + 20;
					break;
				}
			}
		}
		
		frame.setBounds(bounds);
		
	}
	
	protected static void loadLibrary() throws java.io.IOException, IllegalAccessException, NoSuchFieldException
	{
		if (FileUtil.isWin())
		{
			FileUtil.loadLibrary("lib/win32/dirt_lib_ui_jfc.dll");
		}
		if (FileUtil.isLinux())
		{
			FileUtil.loadLibrary("lib/linux_"+System.getProperty("os.arch")+"/libdirt_lib_ui_jfc.so");
		}
	}
	
	public static void alert(Frame frame)
	{
		try
		{
			if (FileUtil.isWin())
			{
				if (!frame.isFocused())
				{
					loadLibrary();
					Win32 win32 = new Win32();
					win32.alert(frame);
				}
			}
			else if (FileUtil.isLinux())
			{
				if (!frame.isFocused())
				{
					loadLibrary();
					Linux linux = new Linux();
					linux.alert(frame);
				}
			}
			else if (FileUtil.isMac())
			{
				synchronized (FileUtil.class)
				{
					if (dockBouncer == null)
					{
						dockBouncer = new MacDockBouncer();
					}
				}
				dockBouncer.addFrame(frame);
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
	
	public static void stealFocus(Window window)
	{
		if (FileUtil.isWin())
		{
			try
			{
				loadLibrary();
				Win32 win32 = new Win32();
				win32.stealFocus(window);
			}
			catch (Exception ex)
			{
				ex.printStackTrace();
			}
		}
		else
		{
			window.toFront();
			window.requestFocus();
		}
	}
	
	public static boolean isValidWindow(Window w)
	{
		if (w == null || !w.isDisplayable())
		{
			return false;
		}
		if (w instanceof JFrame)
		{
			JMenuBar menuBar = ((JFrame)w).getJMenuBar();
			if (menuBar != null && menuBar == Application.getInstance().getFramelessJMenuBar())
			{
				return false;
			}
		}
		return true;
	}
	
	public static void initSwing(String appName)
	{
		
		// set L&F to platform default
		try
		{
			UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
		}
		catch (Throwable t)
		{
			// don't really care if this fails
		}
		
		// enable dynamic layouts if not enabled by default
		Toolkit toolkit = Toolkit.getDefaultToolkit();
		if (Boolean.TRUE.equals(toolkit.getDesktopProperty("awt.dynamicLayoutSupported")))
		{
			toolkit.setDynamicLayout(true);
		}
		
		System.setProperty("apple.laf.useScreenMenuBar", "true");
		
		// Set a sane WM_CLASS on X11
		// <http://bugs.sun.com/bugdatabase/view_bug.do?bug_id=6528430>
		try
		{
			java.lang.reflect.Field awtAppClassNameField =
				toolkit.getClass().getDeclaredField("awtAppClassName");
			awtAppClassNameField.setAccessible(true);
			awtAppClassNameField.set(toolkit, appName);
		}
		catch (Throwable t)
		{
			// don't really care if this fails
		}
		
		// initialise our window manager
		windowManager = new WindowManager(toolkit);
		
	}
	
}