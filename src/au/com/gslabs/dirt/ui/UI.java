package au.com.gslabs.dirt.ui;

public class UI
{

	private UI()
	{
	}

	public static void startClient(String[] args) throws Throwable
	{
		
		boolean cli = false;
		boolean gui = false;
		
		for (String arg : args)
		{
			if (arg.equals("--console"))
			{
				cli = true;
			}
			if (arg.equals("--gui"))
			{
				gui = true;
			}
		}
		
		if (gui)
		{
			au.com.gslabs.dirt.ui.jfc.client.ClientJFC.init();
		}
		else if (cli)
		{
			au.com.gslabs.dirt.ui.cli.client.ClientCLI.init();
		}
		else
		{
			try
			{
				au.com.gslabs.dirt.ui.jfc.client.ClientJFC.init();
			}
			catch (Throwable ex)
			{
				au.com.gslabs.dirt.ui.cli.client.ClientCLI.init();
			}
		}
		
	}

}
