package au.com.gslabs.dirt.lib.ui.jfc;

import java.io.File;
import java.lang.reflect.*;
import au.com.gslabs.dirt.lib.ui.jfc.jni.*;
import java.awt.*;
import javax.swing.*;
import au.com.gslabs.dirt.util.FileUtil;
import java.net.URL;

public class UIUtil
{
	
	private static final String JNILib_Win32 = "lib/jni/win32/dirt_lib_ui_jfc.dll";
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
				FileUtil.loadLibrary(JNILibWin32);
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
				Class c = Class.forName("org.jdesktop.jdic.misc.Alerter");
				Method new_instance = c.getMethod("newInstance", (Class[])null);
				Object alerter = new_instance.invoke(null, (Object[])null);
				Method alert = c.getMethod("alert", new Class[] { Class.forName("java.awt.Frame") });
				alert.invoke(alerter, new Object[] { frame });
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
				FileUtil.loadLibrary(JNILibWin32);
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
    		UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
		}
		catch (Exception ex)
		{
		}
		
		System.setProperty("apple.laf.useScreenMenuBar", "true");
		
	}
	
}