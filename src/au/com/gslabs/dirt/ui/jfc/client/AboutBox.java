package au.com.gslabs.dirt.ui.jfc.client;

import java.awt.*;
import java.awt.event.*;
import java.util.Locale;
import java.util.ResourceBundle;
import javax.swing.*;

public class AboutBox extends JFrame
{
	
	protected JLabel titleLabel, aboutLabel[];
	protected static int labelCount = 8;
	protected static Dimension size = new Dimension(350, 230);
	protected Font titleFont, bodyFont;
	protected ResourceBundle resbundle;
	
	public AboutBox()
	{
	
		super("");
		this.setResizable(false);
		resbundle = ResourceBundle.getBundle("strings", Locale.getDefault());
		
		this.addWindowListener(new WindowAdapter()
		{
			public void windowClosing(WindowEvent event)
			{
				onClose();
			}
		});	
		
		// Initialize useful fonts
		// todo: these null checks seem wrong, won't it either throw or return a generic font?
		titleFont = new Font("Lucida Grande", Font.BOLD, 14);
		if (titleFont == null)
		{
			titleFont = new Font("SansSerif", Font.BOLD, 14);
		}
		bodyFont  = new Font("Lucida Grande", Font.PLAIN, 10);
		if (bodyFont == null)
		{
			bodyFont = new Font("SansSerif", Font.PLAIN, 10);
		}
		
		this.getContentPane().setLayout(new BorderLayout(15, 15));
		
		aboutLabel = new JLabel[labelCount];
		aboutLabel[0] = new JLabel("");
		aboutLabel[1] = new JLabel(resbundle.getString("frameConstructor"));
		aboutLabel[1].setFont(titleFont);
		aboutLabel[2] = new JLabel(resbundle.getString("version"));
		aboutLabel[2].setFont(bodyFont);
		aboutLabel[3] = new JLabel("");
		aboutLabel[4] = new JLabel("");
		aboutLabel[5] = new JLabel("Java " + System.getProperty("java.version"));
		aboutLabel[5].setFont(bodyFont);
		aboutLabel[6] = new JLabel(resbundle.getString("copyright"));
		aboutLabel[6].setFont(bodyFont);
		aboutLabel[7] = new JLabel("");		
		
		Panel textPanel2 = new Panel(new GridLayout(labelCount, 1));
		for (int i = 0; i < labelCount; i++)
		{
			aboutLabel[i].setHorizontalAlignment(JLabel.CENTER);
			textPanel2.add(aboutLabel[i]);
		}
		this.getContentPane().add(textPanel2, BorderLayout.CENTER);
		this.pack();
		Point pos = GraphicsEnvironment.getLocalGraphicsEnvironment().getCenterPoint();
		pos.translate(-size.width / 2, -size.height / 2);
		this.setLocation(pos);
		this.setSize(size);
		
	}
	
	protected void onClose()
	{
		dispose();
	}
	
}
