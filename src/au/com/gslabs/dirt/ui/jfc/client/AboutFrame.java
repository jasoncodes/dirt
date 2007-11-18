package au.com.gslabs.dirt.ui.jfc.client;

import java.awt.Dimension;
import javax.swing.*;
import javax.swing.event.*;
import net.roydesign.ui.*;
import net.roydesign.mac.MRJAdapter;
import java.util.ResourceBundle;
import au.com.gslabs.dirt.lib.ui.jfc.UIUtil;

public class AboutFrame extends StandardMacAboutFrame
{
	
	final static ResourceBundle resbundle = ResourceBundle.getBundle("res/strings");

	public AboutFrame()
	{
		
		super(
			resbundle.getString("title"),
			resbundle.getString("version") + " " + resbundle.getString("sourceDate"));
		setApplicationIcon(UIUtil.loadImageIcon(64));
		setBuildVersion(null);
		setCopyright(resbundle.getString("copyright"));
		
		if (MRJAdapter.isSwingUsingScreenMenuBar())
		{
			setJMenuBar(new MainMenuBar());
		}
		UIUtil.setIcon(this);
		UIUtil.addExitOnCloseHandler(this);
		
	}
	
}
