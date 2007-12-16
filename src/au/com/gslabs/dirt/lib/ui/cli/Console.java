package au.com.gslabs.dirt.lib.ui.cli;

import java.io.*;
import java.util.List;
import au.com.gslabs.dirt.lib.ui.common.*;
import au.com.gslabs.dirt.lib.util.TextModifierParser;
import au.com.gslabs.dirt.lib.thread.InterruptibleInputStream;

public class Console
{
	
	protected jline.ConsoleReader console;
	protected boolean bailingOut;
	protected InterruptibleInputStream input;
	protected boolean passwordMode;
	protected Completor completor;
	protected HistoryFilter historyFilter;
	
	public Console()
	{
		bailingOut = false;
		passwordMode = false;
		completor = null;
		historyFilter = null;
		try
		{
			input = new InterruptibleInputStream(new FileInputStream(FileDescriptor.in));
			console = new jline.ConsoleReader(input, new PrintWriter(System.out));
			console.setHistory(new MyHistory());
		}
		catch (IOException ex)
		{
			throw new RuntimeException(ex);
		}
	}
	
	public void setPasswordMode(boolean passwordMode)
	{
		if (this.passwordMode != passwordMode)
		{
			this.passwordMode = passwordMode;
			setText("");
			input.interrupt();
		}
	}
	
	public void setText(String text)
	{
		clearCurrentLine();
		final jline.CursorBuffer buffer = console.getCursorBuffer();
		buffer.clearBuffer();
		buffer.write(text);
		try
		{
			console.drawLine();
			console.flushConsole();
		}
		catch (IOException ex)
		{
			throw new RuntimeException(ex);
		}
	}
	
	public class MyCompletor implements jline.Completor
	{
		@SuppressWarnings("unchecked") // jline.Completor interface isn't generic
		public int complete(String buffer, int cursor, List candidates)
		{
			if (completor != null)
			{
				return completor.complete(buffer, cursor, candidates);
			}
			return -1; // default to no tab completion
		}
	}
	
	public void setCompletor(Completor completor)
	{
		this.completor = completor;
	}
	
	protected class MyHistory extends jline.History
	{
		@Override
		public void addToHistory(String buffer)
		{
			boolean canAdd = true; // default to true
			if (historyFilter != null)
			{
				canAdd = historyFilter.canAddToHistory(buffer);
			}
			if (canAdd)
			{
				super.addToHistory(buffer);
			}
		}
	}
	
	public void setHistoryFilter(HistoryFilter historyFilter)
	{
		this.historyFilter = historyFilter;
	}
	
	public void clearScreen()
	{
		try
		{
			console.clearScreen();
		}
		catch (IOException ex)
		{
		}
	}
	
	public void close()
	{
		bailingOut = true;
		try
		{
			console.getInput().close();
		}
		catch (IOException ex)
		{
			System.exit(1);
		}
	}
	
	private static final char ESC = (char)27;

	protected void wipeLastLine()
	{
		try
		{
			if (jline.Terminal.getTerminal().isANSISupported())
			{
				console.printString(ESC + "[1A"); // move up one line
				console.printString(ESC + "[K"); //clear line, from cursor position to end
				console.flushConsole();
			}
		}
		catch (IOException ex)
		{
		}
	}
	
	protected void clearCurrentLine()
	{
		try
		{
			if (jline.Terminal.getTerminal().isANSISupported())
			{
				int bufferLength = console.getCursorBuffer().length();
				if (bufferLength > 0)
				{
					console.printString(ESC + "["+bufferLength+"D"); // move cursor left by bufferLength chars
					console.printString(ESC + "[K"); //clear line, from cursor position to end
					console.flushConsole();
				}
			}
		}
		catch (IOException ex)
		{
		}
	}
	
	public synchronized void println(String message)
	{
		try
		{
			
			clearCurrentLine();
			
			String text = TextModifierParser.parse(
				message, TextModifierParser.OutputFormat.PLAIN);
			console.printString(text + "\n");
			
			console.drawLine();
			console.flushConsole();
			
		}
		catch (IOException ex)
		{
			System.out.println(message);
		}
	}
	
	public final class UserInput
	{
		public final String text;
		public final boolean isPassword;
		UserInput(String text, boolean isPassword)
		{
			this.text = text;
			this.isPassword = isPassword;
		}
	}
	
	public UserInput getNextUserInput()
	{
		
		while (!bailingOut)
		{
			final Character mask = this.passwordMode ? new Character('*') : null;
			final String line;
			try
			{
				line = console.readLine(mask);
			}
			catch (IOException ex)
			{
				return null;
			}
			if (line == null)
			{
				if (!input.wasInterrupt())
				{
					bailingOut = true;
				}
			}
			else if (this.passwordMode)
			{
				wipeLastLine();
				setPasswordMode(false);
				return new UserInput(line, true);
			}
			else if (line.length() > 0)
			{
				wipeLastLine();
				return new UserInput(line, false);
			}
		}
		
		return null;
		
	}
	
}