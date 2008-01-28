package au.com.gslabs.dirt.lib.ui.jfc;

import javax.swing.*;
import javax.swing.text.Document;
import javax.swing.undo.*;
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
import au.com.gslabs.dirt.lib.ui.common.Completor;

public class InputArea extends JScrollPane
{
	
	public interface InputListener extends EventListener
	{
		public void inputPerformed(InputArea source, String[] lines);
		public void inputCompletionCandidates(InputArea source, String[] candidates);
		public boolean inputCanAddToHistory(InputArea source, String line);
	}
	
	private final JTextArea txt;
	private final UndoManager undo;
	private final ArrayList<InputListener> listeners;
	private final ArrayList<String> history;
	private String defaultCommand;
	private String commandPrefix;
	private int history_pos;
	private String edit_text;
	private Completor completor;
	
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
		
		listeners = new ArrayList<InputListener>();
		addEventListeners();
		
		undo = new UndoManager();
		initUndo();
		
		if (FileUtil.isMac())
		{
			Border border = BorderFactory.createEmptyBorder(0, 0, 0, 14);
			setBorder(new CompoundBorder(getBorder(), border));
		}
		
	}
	
	private void initUndo()
	{
		
		final Document doc = txt.getDocument();
		doc.addUndoableEditListener(new UndoableEditListener()
			{
				public void undoableEditHappened(UndoableEditEvent evt)
				{
					undo.addEdit(evt.getEdit());
				}
			});
			
		txt.getInputMap().put(
			KeyStroke.getKeyStroke(
				KeyEvent.VK_Z,
				Toolkit.getDefaultToolkit().getMenuShortcutKeyMask()),
			"Undo");
		txt.getInputMap().put(
			KeyStroke.getKeyStroke(
				KeyEvent.VK_Z,
				Toolkit.getDefaultToolkit().getMenuShortcutKeyMask() | InputEvent.SHIFT_MASK),
			"Redo");
			
		txt.getActionMap().put("Undo", new AbstractAction("Undo")
			{
				public void actionPerformed(ActionEvent evt)
				{
					try
					{
						if (undo.canUndo())
						{
							undo.undo();
						}
					}
					catch (CannotUndoException e)
					{
					}
				}
			});
		txt.getActionMap().put("Redo", new AbstractAction("Redo")
			{
				public void actionPerformed(ActionEvent evt)
				{
					try
					{
						if (undo.canRedo())
						{
							undo.redo();
						}
					}
					catch (CannotRedoException e)
					{
					}
				}
			});
		
	}
	
	private void addToHistory(String line)
	{
		if (history.size() < 1 || !history.get(history.size()-1).equals(line))
		{
			if (line.length() > 0)
			{
				for (TextModifier mod : TextModifier.class.getEnumConstants())
				{
					line = line.replace(mod.getChar(), mod.getSafeInputChar());
				}
				history.add(line);
			}
		}
		history_pos = history.size();
		edit_text = "";
	}
	
	private void removeLastHistoryEntry()
	{
		if (history.size() > 0)
		{
			history.remove(history.size() - 1);
		}
	}
	
	public void setEnabled(boolean newValue)
	{
		super.setEnabled(newValue);
		txt.setEnabled(newValue);
		getHorizontalScrollBar().setEnabled(newValue);
		getVerticalScrollBar().setEnabled(newValue);
		getViewport().getView().setEnabled(newValue);
	}
	
	public boolean requestFocusInWindow()
	{
		return txt.requestFocusInWindow();
	}
	
	private void addEventListeners()
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
				@Override
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
					if (e.getKeyCode() == KeyEvent.VK_BACK_SPACE)
					{
						e.consume();
						// is there anything to backspace?
						if (txt.getSelectionEnd() > 0)
						{
							// trigger the backspace action
							// this feels so hacky, is there a better way?
							txt.getActions()[1].actionPerformed(new ActionEvent(e.getSource(), 0, null));
						}
					}
					if (e.getKeyCode() == KeyEvent.VK_TAB)
					{
						doTabCompletion();
						e.consume();
					}
				}
			});
	}
	
	private void doTabCompletion()
	{
		
		if (txt.getSelectionStart() != txt.getSelectionEnd())
		{
			alert();
			return;
		}
		
		final ArrayList<String> candidates = new ArrayList<String>();
		final int idxStart;
		final int idxEnd = txt.getSelectionStart();
		
		if (completor != null)
		{
			idxStart = completor.complete(txt.getText(), idxEnd, candidates);
		}
		else
		{
			idxStart = -1;
		}
		
		if (candidates.size() < 1 || idxStart < 0)
		{
			alert();
			return;
		}
		
		int commonCandidateChars = 0;
		boolean commonDone = false;
		while (!commonDone)
		{
			int c = -1;
			for (String candidate : candidates)
			{
				if (candidate.length() <= commonCandidateChars)
				{
					commonDone = true;
					break;
				}
				if (c < 0)
				{
					c = candidate.charAt(commonCandidateChars);
				}
				else if (c != candidate.charAt(commonCandidateChars))
				{
					commonDone = true;
					break;
				}
			}
			if (!commonDone)
			{
				commonCandidateChars++;
			}
		}
		
		final String src = txt.getText();
		final String dst =
			src.substring(0, idxStart) +
			candidates.get(0).substring(0, commonCandidateChars) +
			src.substring(idxEnd);
		final int idxInsertionPoint = idxStart + commonCandidateChars;
		txt.setText(dst);
		txt.setSelectionStart(idxInsertionPoint);
		txt.setSelectionEnd(idxInsertionPoint);
		
		if (candidates.size() > 1)
		{
			for (int i = listeners.size()-1; i >= 0; --i)
			{
				listeners.get(i).inputCompletionCandidates(this, candidates.toArray(new String[0]));
			}
		}
		
	}
	
	private void onHistoryUp()
	{
		if (history_pos > 0)
		{
			if (history_pos == history.size())
			{
				edit_text = txt.getText();
			}
			history_pos--;
			displayHistoryItem();
		}
		else
		{
			alert();
		}
	}
	
	private void onHistoryDown()
	{
		if ((history_pos+1) <= history.size())
		{
			history_pos++;
			displayHistoryItem();
		}
		else
		{
			alert();
		}
	}
	
	private void displayHistoryItem()
	{
		if (history_pos == history.size())
		{
			txt.setText(edit_text);
		}
		else
		{
			txt.setText(history.get(history_pos));
		}
		setInsertionPointEnd();
	}
	
	private void setInsertionPointEnd()
	{
		int len = txt.getText().length();
		txt.setSelectionStart(len);
		txt.setSelectionEnd(len);
	}
	
	public void setText(String text, int selStart, int selEnd)
	{
		txt.setText(text);
		txt.setSelectionStart(selStart);
		txt.setSelectionEnd(selEnd);
		history_pos = history.size();
	}
	
	public void setCompletor(Completor completor)
	{
		this.completor = completor;
	}
	
	private void alert()
	{
		Toolkit.getDefaultToolkit().beep();
	}
	
	private void processModifierKey(TextModifier mod)
	{
		if (txt.getSelectedText() == null)
		{
			txt.replaceSelection(""+mod.getSafeInputChar());
		}
		else
		{
			int start = txt.getSelectionStart();
			txt.replaceSelection(mod.getSafeInputChar()+txt.getSelectedText()+mod.getSafeInputChar());
			if (mod == TextModifier.COLOUR)
			{
				txt.setSelectionStart(start+1);
				txt.setSelectionEnd(start+1);
			}
		}
	}
	
	private void onChange()
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
	
	private void inputCompleted(boolean forceDefaultCommand)
	{
		
		String text = txt.getText();
		
		for (TextModifier mod : TextModifier.class.getEnumConstants())
		{
			text = text.replace(mod.getSafeInputChar(), mod.getChar());
		}
		
		ArrayList<String> lines = TextUtil.split(text, '\n');
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
	
	private void processInput(String[] lines, boolean forceDefaultCommand)
	{
		
		boolean again = lines.length > 1;
		while (again)
		{
			int minTabDepth = Integer.MAX_VALUE;
			int minSpaceDepth = Integer.MAX_VALUE;
			for (int i = 0; i < lines.length; ++i)
			{
				if (lines[i].length() > 0)
				{
					int thisTabDepth = 0;
					int thisSpaceDepth = 0;
					for (int j = 0; j < lines[i].length(); ++j)
					{
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
				}
			}
			int minDepth = Math.max(minTabDepth, minSpaceDepth);
			if (minDepth > 0)
			{
				for (int i = 0; i < lines.length; ++i)
				{
					if (lines[i].length() > 0)
					{
						lines[i] = lines[i].substring(minDepth);
					}
				}
			}
			again = minDepth > 0;
		}
		
		for (int i = 0; i < lines.length; ++i)
		{
			if (canAddToHistory(lines[i]))
			{
				addToHistory(lines[i]);
			}
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
		
		for (int i = listeners.size()-1; i >= 0; --i)
		{
			listeners.get(i).inputPerformed(this, lines);
		}
		
	}
	
	private boolean canAddToHistory(String line)
	{
		boolean safe = true;
		for (int i = listeners.size()-1; i >= 0; --i)
		{
			safe &= listeners.get(i).inputCanAddToHistory(this, line);
		}
		return safe;
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
