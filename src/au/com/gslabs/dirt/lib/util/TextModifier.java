package au.com.gslabs.dirt.lib.util;

public enum TextModifier
{
	
	BOLD      ('\u0002'),
	COLOUR    ('\u0003'),
	ORIGINAL  ('\u000f'),
	REVERSE   ('\u0016'),
	UNDERLINE ('\u001f');
	
	private final char charValue;
	
	TextModifier(char charValue)
	{
		this.charValue = charValue;
	}
	
	public char getChar()
	{
		return charValue;
	}
	
}