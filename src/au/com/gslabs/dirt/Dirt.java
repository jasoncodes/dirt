package au.com.gslabs.dirt;

public class Dirt
{

	public static void main(String[] args)
	{
	
		Util.ShutdownCleanupCheck(args);

		au.com.gslabs.dirt.ui.client.jfc.Client.init();
		
	}
	
}
