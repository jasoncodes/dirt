package au.com.gslabs.dirt.ui.jfc;

import javax.swing.*;
import javax.swing.event.*;
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
	
	public enum DefaultCommandMode
	{
		Never,
		Always,
		SingleLine
	}
	
	JTextArea txt;
	protected DefaultCommandMode defaultCommandMode;
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
		defaultCommandMode = DefaultCommandMode.Always;
		defaultCommand = "SAY";
		commandPrefix = "/";
		listeners = new ArrayList<InputListener>();
		addEventListeners();
	}
	
	protected void addEventListeners()
	{
		txt.getDocument().addDocumentListener(new DocumentListener()
			{
				public void changedUpdate(DocumentEvent e)
				{
					OnChange();
				}
				public void insertUpdate(DocumentEvent e)
				{
					OnChange();
				}
				public void removeUpdate(DocumentEvent e)
				{
					OnChange();
				}
			});	
	}
	
	protected void OnChange()
	{
		if (txt.getText().indexOf("\n") > -1)
		{
			SwingUtilities.invokeLater(new OnChangeHandler());
		}
	}
	
	protected class OnChangeHandler implements Runnable
	{
		public void run()
		{

			ArrayList<String> lines = TextUtil.split(txt.getText(), '\n');

			if (lines.size() > 1)
			{
				
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
				processInput(tmp);
				
			}
			
		}

	}
	
	protected void processInput(String[] lines)
	{
		
		if (commandPrefix != null && defaultCommand != null)
		{
			
			boolean ok = false;
			switch (defaultCommandMode)
			{
				case Always:
					ok = true;
					break;
				case SingleLine:
					ok = lines.length == 1;
					break;
				case Never:
					ok = false;
					break;
				default:
					throw new IllegalArgumentException("Unexpected default command mode");
			}
			
			if (ok)
			{
				
				for (int i = 0; i < lines.length; ++i)
				{
					if (!lines[i].startsWith(commandPrefix))
					{
						lines[i] = commandPrefix + defaultCommand + " " + lines[i];
					}
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
	
	public DefaultCommandMode getDefaultCommandMode()
	{
		return this.defaultCommandMode;
	}
	
	public void setDefaultCommandMode(DefaultCommandMode defaultCommandMode)
	{
		this.defaultCommandMode = defaultCommandMode;
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