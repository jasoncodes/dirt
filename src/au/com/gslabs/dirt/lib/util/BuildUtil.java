package au.com.gslabs.dirt.lib.util;

/* This class is called during the build process by ant */

import java.io.*;
import java.util.*;

public class BuildUtil
{
	
	public static void main(String[] args) throws IOException
	{
		if (args.length == 3 && args[0].equals("--append-source-date-to-version"))
		{
			
			Date sourceDate = getSourceDate(args[1]);
			String sourceDateString = TextUtil.formatDateTime(sourceDate, false, true);
			
			Properties strings = new Properties();
			strings.load(new FileInputStream(args[2]));
			strings.setProperty("sourceDate", sourceDateString);
			strings.store(new FileOutputStream(args[2]), null);
		}
		else if (args.length == 1 && args[0].equals("--print-arch"))
		{
			System.out.println(System.getProperty("os.arch"));
		}
		else if (args.length == 1 && args[0].equals("--print-javahome"))
		{
			System.out.println(System.getProperty("java.home"));
		}
		else
		{
			System.err.println("Invalid args");
			System.exit(1);
		}
	}
	
	public static class JavaSourceFilenameFilter implements FilenameFilter
	{
		public boolean accept(File dir, String name)
		{
			return name.endsWith(".java");
		}
	}
	
	public static Date getSourceDate(String basePath)
	{
		if (basePath.length() > 0 && basePath.charAt(basePath.length()-1) != File.separatorChar)
		{
			basePath += File.separatorChar;
		}
		FileIterator iterator = new FileIterator(basePath, new JavaSourceFilenameFilter());
		long latestModified = 0;
		String filename;
		while ((filename = iterator.next()) != null)
		{
			long thisModified = new File(basePath+filename).lastModified();
			latestModified = Math.max(latestModified, thisModified);
		}
		return new Date(latestModified);
	}
	
}