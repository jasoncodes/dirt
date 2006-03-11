package au.com.gslabs.dirt.lib.util;

import java.io.*;
import java.net.URL;

public class FileUtil
{

	private FileUtil()
	{
	}
	
	public static boolean isMac()
	{
		return System.getProperty("os.name").toLowerCase().startsWith("mac os");
	}

	public static boolean isWin()
	{
		return System.getProperty("os.name").toLowerCase().startsWith("windows");
	}
	
	public static boolean isLinux()
	{
		return System.getProperty("os.name").toLowerCase().startsWith("linux");
	}
	
	private static String temp_dir = null;

	protected static ClassLoader getClassLoader()
	{
		return new FileUtil().getClass().getClassLoader();
	}
	
	public static InputStream getResourceAsStream(String path)
	{
		return getClassLoader().getResourceAsStream(path);
	}
	
	public static URL getResource(String path)
	{
		return getClassLoader().getResource(path);
	}
	
	public static File getNewTempFile() throws IOException
	{
		File f = null;
		while (f == null || !f.createNewFile())
		{
			f = new File(getTempDir(), TextUtil.generateRandomChars(8)+".tmp");
		}
		return f;
	}
	
	public static String getJavaBinaryPath()
	{
		String path = System.getProperty("java.home") + File.separator + "bin" + File.separator + "java";
		if (isWin())
		{
			path += ".exe";
		}
		return File.exists(path) ? path : null;
	}

	public static String getTempDir()
	{
		try
		{
			initTempDir();
		}
		catch (IOException ex)
		{
			System.err.println(ex);
			System.exit(1);
		}
		return temp_dir;
	}
	
	protected static class RemoveTemporaryDirectoryShutdownHook extends Thread
	{
		public void start()
		{
			try
			{
				Runtime.getRuntime().exec(new String[] { getJavaBinaryPath(), "-jar", getJarSystemPath(), "--shutdown-cleanup", getTempDir() });
			}
			catch (Exception ex)
			{
			}
		}
	}

	// to be called first in Main()	
	public static void ShutdownCleanupCheck(String[] args)
	{
		if (args.length == 2 && args[0].equals("--shutdown-cleanup"))
		{
			for (int i = 500; i < 2500; i += 500)
			{
				try
				{
					Thread.sleep(i);
				}
				catch (Exception ex)
				{
				}
				try
				{
					if (deleteDir(new File(args[1])))
					{
						System.exit(0);
					}
				}
				catch(Exception ex)
				{
				}
			}
			// don't want execution to contiune onto main app
			System.exit(0);
		}
	}

	// Deletes all files and subdirectories under dir.
	// Returns true if all deletions were successful.
	// If a deletion fails, the method stops attempting to delete and returns false.
	public static boolean deleteDir(File dir)
	{
	
		if (dir.isDirectory())
		{
			String[] children = dir.list();
			for (int i=0; i<children.length; i++)
			{
				boolean success = deleteDir(new File(dir, children[i]));
				if (!success)
				{
					return false;
				}
			}
		}
	
		// The directory is now empty so delete it
		return dir.delete();
		
	}
	
	public static String getJarSystemPath()
	{
		String jar_path = FileUtil.class.getProtectionDomain().getCodeSource().getLocation().getPath();
		int i = jar_path.indexOf("!");
		if (i > -1)
		{
			jar_path = jar_path.substring(0, i); 
		}
		try
		{
			jar_path = java.net.URLDecoder.decode(jar_path, "UTF-8");
		}
		catch (UnsupportedEncodingException ex)
		{
			jar_path = jar_path.replaceAll("%20", " ");
			jar_path = jar_path.replaceAll("\\+", " ");
		}
		if (jar_path.startsWith("file:"))
		{
			jar_path = jar_path.substring(5);
		}
		if (isWin())
		{
			if (!jar_path.startsWith("//") && jar_path.startsWith("/"))
			{
				jar_path = jar_path.substring(1);
			}
			jar_path = jar_path.replace('/', '\\');
		}
		return jar_path;	
	}
	
	protected static void initTempDir() throws IOException
	{
		if (temp_dir == null)
		{
			File temp = File.createTempFile("dirtchat_", "");
			temp.delete();
			temp.mkdirs();
			temp.deleteOnExit();
			temp_dir = temp.getAbsolutePath();
			Runtime.getRuntime().addShutdownHook(new RemoveTemporaryDirectoryShutdownHook());
		}
	}
	
	public static void loadLibrary(String name) throws IOException, IllegalAccessException, NoSuchFieldException
	{
		File foo = new File(getTempDir(), new File(name).getName() + "foo");
		foo.deleteOnExit();
		File temp = new File(getTempDir(), new File(name).getName());
		if (!temp.exists())
		{
			temp.deleteOnExit();
			InputStream in = getResourceAsStream(name);
			if (in == null) throw new IOException(name + " not found");
			FileOutputStream out = new FileOutputStream(temp);
			copy(in, out);
			in.close();
			out.close();
			// Reset the "sys_paths" field of the ClassLoader to null.
			// Reset it to null so that whenever "System.loadLibrary" is called, it will be reconstructed with the changed value.
			Class clazz = ClassLoader.class;
			java.lang.reflect.Field field = clazz.getDeclaredField("sys_paths");
			boolean accessible = field.isAccessible();
			if (!accessible)
			{
				field.setAccessible(true);
			}
			Object original = field.get(clazz);
			field.set(clazz, null);
			System.setProperty("java.library.path", getTempDir());
		}
		String short_name = temp.getName();
		int i = short_name.lastIndexOf(".");
		short_name = short_name.substring(0, i);
		try
		{
			System.loadLibrary(short_name);
			return;
		}
		catch (Throwable t)
		{
		}
		try
		{
			System.load(temp.getAbsolutePath());
			return;
		}
		catch (Throwable t)
		{
			t.printStackTrace();
		}
		System.loadLibrary(short_name);
	}
	
	public static void copy(InputStream in, OutputStream out) throws IOException
	{
		int read = 0;
		byte[] buf = new byte[1024];
		while(in.available() > 0)
		{
			read = in.read(buf);
			if(read <= 0)
			{
				break;
			}
			out.write(buf, 0, read);
		}
	}

}
