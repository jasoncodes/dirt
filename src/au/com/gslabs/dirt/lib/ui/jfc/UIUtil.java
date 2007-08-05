package au.com.gslabs.dirt.lib.ui.jfc;

import java.io.File;
import au.com.gslabs.dirt.lib.ui.jfc.jni.*;
import java.awt.*;
import java.lang.reflect.*;
import javax.swing.*;
import au.com.gslabs.dirt.lib.util.FileUtil;

public class UIUtil
{
	
	private static final String JNILib_Win32 = "lib/win32/dirt_lib_ui_jfc.dll";
	private static final String JNILib_LinuxX86 = "lib/linux_x86/libdirt_lib_ui_jfc.so";
	
	private UIUtil()
	{
	}
	
	public static Window getActiveWindow()
	{
		return java.awt.KeyboardFocusManager.getCurrentKeyboardFocusManager().getActiveWindow();
	}
	
	public static void setWin32Icon(Frame frame, String path)
	{
		if (FileUtil.isWin())
		{
			try
			{
				FileUtil.loadLibrary(JNILib_Win32);
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
		
		return rect;
	}
	
	protected static MacDockBouncer dockBouncer = null;
		
	public static void alert(Frame frame)
	{
		try
		{
			if (FileUtil.isWin())
			{
				if (!frame.isFocused())
				{
					FileUtil.loadLibrary(JNILib_Win32);
					Win32 win32 = new Win32();
					win32.alert(frame);
				}
			}
			else if (FileUtil.isLinux())
			{
				if (!frame.isFocused())
				{
					FileUtil.loadLibrary(JNILib_LinuxX86);
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
	
	public static void stealFocus(Frame frame)
	{
		if (FileUtil.isWin())
		{
			try
			{
				FileUtil.loadLibrary(JNILib_Win32);
				Win32 win32 = new Win32();
				win32.stealFocus(frame);
			}
			catch (Exception ex)
			{
				ex.printStackTrace();
			}
		}
	}
	
	public static void initSwing()
	{

		try
		{
			UIManager.setLookAndFeel( UIManager.getSystemLookAndFeelClassName() );
		}
		catch (Exception ex)
		{
		}
		
		Toolkit toolkit = Toolkit.getDefaultToolkit();
		if ( Boolean.TRUE.equals( toolkit.getDesktopProperty( "awt.dynamicLayoutSupported" ) ) )
		{
			toolkit.setDynamicLayout( true );
		}
		
		System.setProperty("apple.laf.useScreenMenuBar", "true");
		
	}
	
}