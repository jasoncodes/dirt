package au.com.gslabs.dirt.lib.ui.jfc;

import java.io.File;
import java.lang.reflect.*;
import au.com.gslabs.dirt.lib.ui.jfc.jni.*;
import java.awt.*;
import javax.swing.*;
import au.com.gslabs.dirt.lib.util.FileUtil;

public class UIUtil
{
	
	private static final String JNILib_Win32 = "lib/win32/dirt_lib_ui_jfc.dll";
	private static final String JNILib_LinuxX86 = "lib/linux_x86/libdirt_lib_ui_jfc.so";
	
	private UIUtil()
	{
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
				
				final Class c = Class.forName("com.apple.cocoa.application.NSApplication");
				final Method shared_app = c.getMethod("sharedApplication", (Class[])null);
				final Object app = shared_app.invoke(null, (Object[])null);
				
				final Class[] int_param = new Class[] { Integer.TYPE };
				final Method request_attention = c.getMethod("requestUserAttention", int_param);
				final Field f = c.getField("UserAttentionRequestInformational");
				final Object[] request_params = new Object[] { f.getInt(null) };
				final Integer requestID = (Integer)request_attention.invoke(app, request_params);
				
				final Method cancel_attention = c.getMethod("cancelUserAttentionRequest", int_param);
				final Object[] cancel_params = new Object[] { requestID };
				final Thread t = new Thread()
					{
						public void run()
						{
							try
							{
								Thread.sleep(1000);
								cancel_attention.invoke(app, cancel_params);
							}
							catch (Exception ex)
							{
							}
						}
					};
				t.start();
				
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