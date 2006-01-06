package au.com.gslabs.dirt;

public class Dirt
{

	public static void main(String[] args)
	{
	
		Util.ShutdownCleanupCheck(args);

		au.com.gslabs.dirt.ui.Launcher.startClient();
		
	}
	
}
