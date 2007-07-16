package au.com.gslabs.dirt.lib.util;

public enum TextModifier
{
	
	BOLD      ('\u0002', 'B'),
	COLOUR    ('\u0003', 'K'),
	ORIGINAL  ('\u000f', 'O'),
	REVERSE   ('\u0016', 'R'),
	UNDERLINE ('\u001f', 'U');
	
	private final char charValue;
	private final char keyChar;
	
	TextModifier(char charValue, char keyChar)
	{
		this.charValue = charValue;
		this.keyChar = keyChar;
	}
	
	public char getChar()
	{
		return charValue;
	}
	
	public char getKeyChar()
	{
		return keyChar;
	}
	
}