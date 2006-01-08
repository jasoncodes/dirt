package au.com.gslabs.dirt.util;

import java.util.Random;

public class TextUtil
{
	
	private TextUtil()
	{
	}
	
	protected static Random random = null;
	
	public static String generateRandomChars(int count)
	{
		if (random == null)
		{
			random = new Random();
		}
		StringBuilder sb = new StringBuilder(count);
		for (int i = 0; i < count; ++i)
		{
			int c = random.nextInt(26*2+10);
			char ch;
			if (c < 10)
			{
				ch = (char)('0'+c);
			}
			else if (c < 10+26)
			{
				ch = (char)('A'+(c-10));
			}
			else
			{
				ch = (char)('a'+(c-10-26));
			}
			sb.append(ch);
		}
		return sb.toString();
	}
	
	public static String stringToHTMLString(String string)
	{
		StringBuffer sb = new StringBuffer(string.length());
		// true if last char was blank
		boolean lastWasBlankChar = false;
		int len = string.length();
		char c;

		for (int i = 0; i < len; i++)
		{
			c = string.charAt(i);
			if (c == ' ')
			{
				// blank gets extra work,
				// this solves the problem you get if you replace all
				// blanks with &nbsp;, if you do that you loss 
				// word breaking
				if (lastWasBlankChar)
				{
					lastWasBlankChar = false;
					sb.append("&nbsp;");
				}
				else
				{
					lastWasBlankChar = true;
					sb.append(' ');
				}
			}
			else
			{
				lastWasBlankChar = false;
				//
				// HTML Special Chars
				if (c == '"')
				{
					sb.append("&quot;");
				}
				else if (c == '&')
				{
					sb.append("&amp;");
				}
				else if (c == '<')
				{
					sb.append("&lt;");
				}
				else if (c == '>')
				{
					sb.append("&gt;");
				}
				else if (c == '\n')
				{
					// Handle Newline
					sb.append("&lt;br/&gt;");
				}
				else
				{
					int ci = 0xffff & c;
					if (ci < 160 )
					{
						// nothing special only 7 Bit
						sb.append(c);
					}
					else
					{
						// Not 7 Bit use the unicode system
						sb.append("&#");
						sb.append(new Integer(ci).toString());
						sb.append(';');
					}
				}
			}
		}
		return sb.toString();
	}
	
}