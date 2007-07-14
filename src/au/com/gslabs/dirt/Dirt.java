package au.com.gslabs.dirt;

import au.com.gslabs.dirt.lib.util.FileUtil;
import au.com.gslabs.dirt.ui.UI;

import au.com.gslabs.dirt.lib.util.ByteBuffer;
import au.com.gslabs.dirt.lib.net.crypt.*;
import au.com.gslabs.dirt.lib.thread.SameThreadInvoker;

public class Dirt
{
	
	public static final String VERSION = "3.1.0 Alpha 0";
	
	public static void main(String[] args)
	{
		FileUtil.ShutdownCleanupCheck(args);
		UI.startClient(args);
	}
	
}
