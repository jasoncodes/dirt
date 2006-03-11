package au.com.gslabs.dirt.ui;

public class UI
{

	private UI()
	{
	}

	public static void startClient()
	{
		try
		{
			au.com.gslabs.dirt.ui.jfc.client.Client.init();
		}
		catch (Exception ex1)
		{
			try
			{
				System.out.println("GUI unavailable");
			}
			catch (Exception ex2)
			{
				System.err.println("Error starting GUI:");
				ex1.printStackTrace();
				System.err.println("Error starting CLI:");
				ex2.printStackTrace();
				System.exit(1);
			}
		}
	}

}
