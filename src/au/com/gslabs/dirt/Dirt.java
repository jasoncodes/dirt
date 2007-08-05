package au.com.gslabs.dirt;

import au.com.gslabs.dirt.lib.util.FileUtil;
import au.com.gslabs.dirt.ui.UI;

public class Dirt
{
	
	public static void main(String[] args)
	{
		try
		{
			FileUtil.ShutdownCleanupCheck(args);
			UI.startClient(args);
		}
		catch (Throwable t)
		{
			t.printStackTrace();
			System.exit(1);
		}
	}
	
}
