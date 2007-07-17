package au.com.gslabs.dirt.lib.ui.jfc;

import javax.swing.*;
import javax.swing.event.*;
import java.awt.event.*;
import javax.swing.border.*;
import java.awt.Toolkit;
import java.util.ArrayList;
import java.util.EventListener;
import java.util.EventObject;
import au.com.gslabs.dirt.lib.util.TextUtil;
import au.com.gslabs.dirt.lib.util.TextModifier;
import au.com.gslabs.dirt.lib.util.FileUtil;

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
	protected ArrayList<String> history;
	protected int history_pos;
	protected String edit_text;
	protected int edit_row;
	
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
		history = new ArrayList<String>();
		history_pos = 0;
		edit_text = "";
		edit_row = -1;
		
		listeners = new ArrayList<InputListener>();
		addEventListeners();
		
		if (FileUtil.isMac())
		{
			Border border = BorderFactory.createEmptyBorder(0, 0, 0, 14);
			setBorder(new CompoundBorder(getBorder(), border));
		}
		
	}
	
	protected void addToHistory(String line)
	{
		if (history.size() < 1 || !history.get(history.size()-1).equals(line))
		{
			if (line.length() > 0)
			{
				history.add(line);
			}
		}
		history_pos = history.size();
		edit_text = "";
		edit_row = -1;
	}
	
	protected void removeLastHistoryEntry()
	{
		if (history.size() > 0)
		{
			history.remove(history.size() - 1);
		}
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
					if (e.getKeyChar() == 13 || e.getKeyChar() == 10)
					{
						inputCompleted(e.isControlDown());
					}
					if (e.getModifiers() == Toolkit.getDefaultToolkit().getMenuShortcutKeyMask())
					{
						for (TextModifier mod : TextModifier.class.getEnumConstants())
						{
							if (KeyEvent.getKeyText(e.getKeyCode()).toUpperCase().equals(""+Character.toUpperCase(mod.getKeyChar())))
							{
								e.consume();
								processModifierKey(mod);
								break;
							}
						}
					}
					if (e.getKeyCode() == KeyEvent.VK_UP)
					{
						onHistoryUp();
						e.consume();
					}
					if (e.getKeyCode() == KeyEvent.VK_DOWN)
					{
						onHistoryDown();
						e.consume();
					}
				}
			});
	}
	
	protected boolean hasChangedHistoryItem()
	{
		if (history_pos >= 0 && history_pos < history.size())
		{
			return !history.get(history_pos).equals(txt.getText());
		}
		else
		{
			return false;
		}
	}
	
	protected void onHistoryUp()
	{
		if (history_pos > 0)
		{
			if (hasChangedHistoryItem())
			{
				edit_text = txt.getText();
				edit_row = history_pos;
				if (edit_text.length() == 0)
				{
					history_pos = edit_row = history.size();
				}
			}
			history_pos--;
			displayHistoryItem();
		}
		else
		{
			if (hasChangedHistoryItem())
			{
				if (txt.getText().length() == 0)
				{
					history_pos = edit_row = history.size();
					return;
				}
			}
			alert();
		}
	}
	
	protected void onHistoryDown()
	{
		if ((history_pos+1) < history.size())
		{
			if (hasChangedHistoryItem())
			{
				edit_text = txt.getText();
				edit_row = history_pos;
				if (edit_text.length() == 0)
				{
					history_pos = edit_row = history.size();
					return;
				}
			}
			history_pos++;
			displayHistoryItem();
		}
		else
		{
			if (history_pos+1 == edit_row)
			{
				if (history_pos == history.size())
				{
					txt.setText("");
				}
				else
				{
					txt.setText(edit_text);
				}
				setInsertionPointEnd();
				history_pos = history.size();
			}
		}
	}
	
	protected void setInsertionPointEnd()
	{
		int len = txt.getText().length();
		txt.setSelectionStart(len);
		txt.setSelectionEnd(len);
	}
	
	protected void displayHistoryItem()
	{
		if (edit_row == history_pos)
		{
			txt.setText(edit_text);
		}
		else
		{
			txt.setText(history.get(history_pos));
		}
		setInsertionPointEnd();
	}
	
	protected void alert()
	{
		// do nothing
	}
	
	protected void processModifierKey(TextModifier mod)
	{
		if (txt.getSelectedText() == null)
		{
			txt.replaceSelection(""+mod.getChar());
		}
		else
		{
			txt.replaceSelection(mod.getChar()+txt.getSelectedText()+mod.getChar());
		}
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
		
		for (int i = 0; i < lines.length; ++i)
		{
			addToHistory(lines[i]);
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