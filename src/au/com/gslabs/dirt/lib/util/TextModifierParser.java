package au.com.gslabs.dirt.lib.util;

public class TextModifierParser
{
	
	public enum OutputFormat
	{
		TEXT,
		HTML
	}
	
	public static String parseText(String text, OutputFormat outputFormat)
	{
		
		// todo
		
		switch (outputFormat)
		{
			
			case HTML:
				return TextUtil.stringToHTMLString(text);
			
			case TEXT:
				return text;
			
			default:
				throw new IllegalArgumentException("Unknown OutputFormat");
			
		}
		
	}
	
}