package au.com.gslabs.dirt.lib.util;

public enum TextModifier
{
	
	BOLD      ('\u0002', '\u2441', 'B'),
	COLOUR    ('\u0003', '\u2440', 'K'),
	ORIGINAL  ('\u000f', '\u2444', 'O'),
	REVERSE   ('\u0016', '\u2442', 'R'),
	UNDERLINE ('\u001f', '\u2443', 'U');
	
	private final char charValue;
	private final char safeInputChar;
	private final char keyChar;
	
	TextModifier(char charValue, char safeInputChar, char keyChar)
	{
		this.charValue = charValue;
		this.safeInputChar = safeInputChar;
		this.keyChar = keyChar;
	}
	
	public char getChar()
	{
		return this.charValue;
	}
	
	public char getSafeInputChar()
	{
		return this.safeInputChar;
	}
	
	public char getKeyChar()
	{
		return this.keyChar;
	}
	
	public String getName()
	{
		return super.toString();
	}
	
	public String toString()
	{
		return ""+getChar();
	}
	
}
