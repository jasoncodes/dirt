package au.com.gslabs.dirt.ui;

public class UI
{

	private UI()
	{
	}

	public static void startClient(String[] args)
	{
		
		boolean cli = false;
		
		for (String arg : args)
		{
			if (arg.equals("--console"))
			{
				cli = true;
			}
		}
		
		if (cli)
		{
			au.com.gslabs.dirt.ui.cli.client.Client.init();
		}
		else
		{
			au.com.gslabs.dirt.ui.jfc.client.Client.init();
		}
		
	}

}
