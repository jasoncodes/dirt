package au.com.gslabs.dirt;

import au.com.gslabs.dirt.util.FileUtil;
import au.com.gslabs.dirt.ui.UI;

public class Dirt
{

	public static void main(String[] args)
	{
		FileUtil.ShutdownCleanupCheck(args);
		UI.startClient();
	}
                                                                               
}
