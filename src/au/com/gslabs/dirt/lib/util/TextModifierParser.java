package au.com.gslabs.dirt.lib.util;

public class TextModifierParser
{
	
	public enum OutputFormat
	{
		TEXT,
		XHTML
	}
	
	public static String parseText(String text, OutputFormat outputFormat)
	{
		
		switch (outputFormat)
		{
			
			case XHTML:
				String html = TextUtil.stringToHTMLString(text);
				html = TextUtil.convertUrlsToLinks(html);
				return html; //todo
			
			case TEXT:
				return text; //todo
			
			default:
				throw new IllegalArgumentException("Unknown OutputFormat");
			
		}
		
	}
	
}