package au.com.gslabs.dirt.lib.ui.jfc;

import javax.swing.*;
import javax.swing.event.*;
import java.awt.event.*;
import au.com.gslabs.dirt.lib.util.TextUtil;
import java.util.ArrayList;
import java.util.EventListener;
import java.util.EventObject;

public class InputArea extends JScrollPane
{
	
	public class InputEvent extends EventObject
	{
		protected String[] lines;
		public InputEvent(InputArea source, String[] lines)
		{
			super(source);
			this.lines = lines;
		}
		public String[] getLines()
		{
			return lines;
		}
	}
	
	public interface InputListener extends EventListener
	{
		public void inputPerformed(InputEvent e);
	}
	
	JTextArea txt;
	protected String defaultCommand;
	protected String commandPrefix;
	protected ArrayList<InputListener> listeners;
	
	public InputArea()
	{
		super(
			JScrollPane.VERTICAL_SCROLLBAR_NEVER,
			JScrollPane.HORIZONTAL_SCROLLBAR_NEVER);
		txt = new JTextArea(1, 0);
		txt.setRows(1);
		setViewportView(txt);
		defaultCommand = "SAY";
		commandPrefix = "/";
		listeners = new ArrayList<InputListener>();
		addEventListeners();
	}
	
	public boolean requestFocusInWindow()
	{
		return txt.requestFocusInWindow();
	}
	
	protected void addEventListeners()
	{
		txt.getDocument().addDocumentListener(new DocumentListener()
			{
				
				public void changedUpdate(DocumentEvent e)
				{
					onChange();
				}
				public void insertUpdate(DocumentEvent e)
				{
					onChange();
				}
				public void removeUpdate(DocumentEvent e)
				{
					onChange();
				}
				
			});
		txt.addKeyListener(new KeyAdapter()
			{
				public void keyPressed(KeyEvent e)
				{
					if ((e.getKeyChar() == 13 || e.getKeyChar() == 10))
					{
						inputCompleted(e.isControlDown());
					}
				}
			});
	}
	
	protected void onChange()
	{
		if (txt.getText().indexOf("\n") > -1)
		{
			SwingUtilities.invokeLater(new Runnable()
				{
					public void run()
					{
						inputCompleted(true);
					}
				});
		}
	}
	
	protected void inputCompleted(boolean forceDefaultCommand)
	{
		
		ArrayList<String> lines = TextUtil.split(txt.getText(), '\n');
		txt.setText(null);
		
		while (lines.size() > 0 && lines.get(0).trim().length() == 0)
		{
			lines.remove(0);
		}
		
		while (lines.size() > 0 && lines.get(lines.size()-1).trim().length() == 0)
		{
			lines.remove(lines.size()-1);
		}
		
		String[] tmp = null;
		tmp = lines.toArray(new String[0]);
		processInput(tmp, forceDefaultCommand);
		
	}
	
	protected void processInput(String[] lines, boolean forceDefaultCommand)
	{
		
		boolean again = lines.length > 1;
		while (again)
		{
			int minTabDepth = Integer.MAX_VALUE;
			int minSpaceDepth = Integer.MAX_VALUE;
			for (int i = 0; i < lines.length; ++i)
			{
				int thisTabDepth = 0;
				int thisSpaceDepth = 0;
				System.out.println(lines[i]);
				for (int j = 0; j < lines[i].length(); ++j)
				{
					System.out.print((lines[i].charAt(j) == '\t') + " " + (lines[i].charAt(j) == ' ') + " ");
					if (lines[i].charAt(j) == '\t' && thisTabDepth == j)
					{
						thisTabDepth++;
					}
					if (lines[i].charAt(j) == ' ' && thisSpaceDepth == j)
					{
						thisSpaceDepth++;
					}
				}
				minTabDepth = Math.min(minTabDepth, thisTabDepth);
				minSpaceDepth = Math.min(minSpaceDepth, thisSpaceDepth);
				System.out.println(minTabDepth + " " + minSpaceDepth);
			}
			int minDepth = Math.max(minTabDepth, minSpaceDepth);
			System.out.println(minDepth);
			if (minDepth > 0)
			{
				for (int i = 0; i < lines.length; ++i)
				{
					lines[i] = lines[i].substring(minDepth);
				}
			}
			again = minDepth > 0;
		}
		
		if (commandPrefix != null && defaultCommand != null)
		{
			for (int i = 0; i < lines.length; ++i)
			{
				if (!lines[i].startsWith(commandPrefix) || forceDefaultCommand)
				{
					lines[i] = commandPrefix + defaultCommand + " " + lines[i];
				}
			}
		}
		
		InputEvent e = new InputEvent(this, lines);
		for (int i = listeners.size()-1; i >= 0; --i)
		{
			listeners.get(i).inputPerformed(e);
		}
		
	}
	
	public String getDefaultCommand()
	{
		return this.defaultCommand;
	}
	
	public void setDefaultCommand(String defaultCommand)
	{
		this.defaultCommand = defaultCommand;
	}
	
	public String getCommandPrefix()
	{
		return this.commandPrefix;
	}
	
	public void setCommandPrefix(String commandPrefix)
	{
		this.commandPrefix = commandPrefix;
	}
	
	public void addInputListener(InputListener l)
	{
		listeners.add(l);
	}
	
	public void removeInputListener(InputListener l)
	{
		listeners.remove(l);
	}
	
}