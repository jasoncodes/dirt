package au.com.gslabs.dirt.ui.jfc.client;

import javax.swing.*;
import net.roydesign.ui.*;

public class AboutFrame extends StandardMacAboutFrame
{
	
	public AboutFrame()
	{
		
		super("Test", "1.0");
		setJMenuBar(new MainMenuBar());
		setApplicationIcon(UIManager.getIcon("OptionPane.informationIcon"));
		/*
		
			f.setBuildVersion("234");
			f.setCopyright("Copyright 2004-2007, Steve Roy, Software Design");
			f.setCredits("<html><body>MRJ Adapter<br>" +
				"<a href=\"http://homepage.mac.com/sroy/mrjadapter/\">homepage.mac.com/sroy/mrjadapter</a><br>" +
				"<br>" +
				"<b>Design &amp; Engineering</b><br>" +
				"Steve Roy<br>" +
				"<a href=\"mailto:sroy@mac.com\">sroy@mac.com</a>" +
				"</body></html>", "text/html");
			f.setHyperlinkListener(new HyperlinkListener()
				{
					public void hyperlinkUpdate(HyperlinkEvent e)
					{
						if (e.getEventType() == HyperlinkEvent.EventType.ACTIVATED)
						{
							try
							{
								MRJAdapter.openURL(e.getURL().toString());
							}
							catch (Exception ex)
							{
								ex.printStackTrace();
							}
						}
					}
				});
				
				*/
	}
	
}