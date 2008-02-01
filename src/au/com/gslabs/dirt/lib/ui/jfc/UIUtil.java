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
	
	private static MacDockBouncer dockBouncer = null;
	private static WindowManager windowManager = null;
	
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
				loadNativeLibrary();
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
				Class<?> fileMgr = Class.forName("com.apple.eio.FileManager");
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
	
	public static Rectangle getDefaultWindowBounds(final Window window, final java.awt.geom.Point2D.Double normalisedScreenPos)
	{
		
		final Rectangle rectMax = GraphicsEnvironment.getLocalGraphicsEnvironment().getMaximumWindowBounds();
		
		final Dimension preferredSize = window.getPreferredSize();
		final int width = Math.min(preferredSize.width, rectMax.width);
		final int height = Math.min(preferredSize.height, rectMax.height);
		
		final Rectangle rect = new Rectangle(width, height);
		rect.x = rectMax.x + (int)( (rectMax.width-width)*normalisedScreenPos.x );
		rect.y = rectMax.y + (int)( (rectMax.height-height)*normalisedScreenPos.y );
		
		// if the gap on the bottom is bigger than the gap on the right
		if (normalisedScreenPos.x >= 0.8 && normalisedScreenPos.y >= 0.8)
		{
			final int gapBottom = (rectMax.y+rectMax.height)-(rect.y+rect.height);
			final int gapRight = (rectMax.x+rectMax.width)-(rect.x+rect.width);
			if (gapBottom > gapRight)
			{
				// make the bottom gap match the right gap
				rect.y = rectMax.y+rectMax.height-rect.height - gapRight;
			}
		}
		
		return rect;
		
	}
	
	public static void setWindowBoundsWithCascade(final Window window, final Rectangle bounds)
	{
		
		final Rectangle rect = new Rectangle(bounds);
		
		final Frame[] frames = getWindowManager().getOrderedValidFrames(true);
		
		boolean conflictFound;
		do
		{
			
			conflictFound = false;
			for (Frame frame : frames)
			{
				if (frame.getLocation().equals(rect.getLocation()))
				{
					conflictFound = true;
					break;
				}
			}
			
			if (conflictFound)
			{
				rect.x += 20;
				rect.y += 20;
			}
			
		}
		while (conflictFound);
		
		window.setBounds(rect);
		
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
	
	static void loadNativeLibrary() throws java.io.IOException, IllegalAccessException, NoSuchFieldException
	{
		if (FileUtil.isWin())
		{
			System.loadLibrary("jawt");
			FileUtil.loadLibrary("lib/win32/dirt_lib_ui_jfc.dll");
		}
		if (FileUtil.isLinux())
		{
			FileUtil.loadLibrary("lib/linux_"+System.getProperty("os.arch")+"/libdirt_lib_ui_jfc.so");
		}
		if (FileUtil.isMac())
		{
			FileUtil.loadLibrary("lib/mac/libDirtJNI.jnilib");
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
					loadNativeLibrary();
					Win32 win32 = new Win32();
					win32.alert(frame);
				}
			}
			else if (FileUtil.isLinux())
			{
				if (!frame.isFocused())
				{
					loadNativeLibrary();
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
			System.err.println("Alert() not available");
			System.err.println(e);
			e.printStackTrace();
		}
	}
	
	/**
	 * Sets the opacity (1.0 => fully opaque, 0.0 => fully transparent) of the given Frame.
	 */
	// original src: http://elliotth.blogspot.com/2007/08/transparent-java-windows-on-x11.html
	public static void setFrameAlpha(JFrame frame, double alpha)
	{
		try
		{
			Field peerField = Component.class.getDeclaredField("peer");
			peerField.setAccessible(true);
			Object peer = peerField.get(frame);
			if (peer == null)
			{
				frame.addNotify();
				peer = peerField.get(frame);
			}
			if (peer == null)
			{
				throw new IllegalArgumentException("Frame does not have a peer");
			}
			
			if (FileUtil.isMac())
			{
				if (System.getProperty("os.version").startsWith("10.3") ||
					System.getProperty("os.version").startsWith("10.4"))
				{
					Class<?> cWindowClass = Class.forName("apple.awt.CWindow");
					if (cWindowClass.isInstance(peer))
					{
						// ((apple.awt.CWindow) peer).setAlpha(alpha);
						Method setAlphaMethod = cWindowClass.getMethod("setAlpha", float.class);
						setAlphaMethod.invoke(peer, (float) alpha);
					}
				}
				else
				{
					frame.getRootPane().putClientProperty("Window.alpha", alpha);
				}
			}
			else if (FileUtil.isWin())
			{
				loadNativeLibrary();
				Win32 win32 = new Win32();
				win32.setWindowAlpha(frame, alpha);
			}
			else
			{
				loadNativeLibrary();
				Linux linux = new Linux();
				linux.setWindowAlpha(frame, alpha);
			}
		}
		catch (Throwable th)
		{
			System.err.println("Failed to apply frame alpha:");
			th.printStackTrace();
		}
	}
	
	public static void setVisibleWithoutFocus(final JFrame frame)
	{
		
		// ref: http://bugs.sun.com/bugdatabase/view_bug.do?bug_id=6187066
		// this method is full of hacks to fake something the JFC UI doesn't let us do
		
		final JMenuBar oldMenu = frame.getJMenuBar();
		frame.setJMenuBar(null);
		final boolean oldResizable = frame.isResizable();
		frame.setResizable(false);
		if (FileUtil.isMac())
		{
			setFrameAlpha(frame, 0.0);
			final Rectangle oldBounds = frame.getBounds();
			frame.setBounds(10000,10000,0,0);
		}
		frame.setFocusableWindowState(false);
		frame.toBack();
		frame.setVisible(true);
		frame.addWindowListener(new WindowAdapter()
			{
				public void windowOpened(WindowEvent e)
				{
					
					if (!frame.getFocusableWindowState())
					{
						final Window focusedFrame = getActiveWindow();
						if (focusedFrame != null)
						{
							focusedFrame.toFront();
						}
						frame.setFocusableWindowState(true);
					}
					
					frame.removeWindowListener(this);
					frame.setResizable(oldResizable);
					frame.setJMenuBar(oldMenu);
					
					if (FileUtil.isMac())
					{
						frame.setBounds(oldBounds);
						setFrameAlpha(frame, 1.0);
					}
					
				}
			});
		
	}
	
	public static void stealFocus(Window window)
	{
		window.setFocusableWindowState(true);
		if (FileUtil.isWin())
		{
			try
			{
				loadNativeLibrary();
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
			System.setProperty("apple.laf.useScreenMenuBar", "true");
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
