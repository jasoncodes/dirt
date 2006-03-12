package au.com.gslabs.dirt.lib.util;

import java.util.Random;
import java.util.ArrayList;
import java.text.MessageFormat;

public class TextUtil
{
	
	private TextUtil()
	{
	}
	
	protected static Random random = null;
	
	public static ArrayList<String> split(String str, char delim)
	{

		ArrayList<String> tokens = new ArrayList<String>();

		StringBuilder token = new StringBuilder();

		for (int i = 0; i < str.length(); ++i)
		{
			if (str.charAt(i) == delim)
			{
				tokens.add(token.toString());
				token = new StringBuilder();
			}
			else
			{
				token.append(str.charAt(i));
			}
		}

		tokens.add(token.toString());

		return tokens;

	}
	
	public static String replace(String text, String repl, String with)
	{
		return replace(text, repl, with, -1);
	}

	public static String replace(String text, String repl, String with, int max)
	{
		
		if (text == null)
		{
			return null;
		}
		
		StringBuilder sb = new StringBuilder(text.length()+with.length());
		
		int start = 0, end = 0;
		while ((end = text.indexOf(repl, start)) >= 0)
		{
			sb.append(text.substring(start, end));
			sb.append(with);
			start = end + repl.length();
			if (--max == 0) break;
		}
		sb.append(text.substring(start));
		
		return sb.toString();
		
	}
	
	public static String join(String[] array, String sep)
	{
		
		int count = 0;
		for (String str : array)
		{
			count += str.length() + sep.length();
		}
		
		StringBuilder sb = new StringBuilder(count);
		
		for (int i = 0; i < array.length; ++i)
		{
			if (i > 0)
			{
				sb.append(sep);
			}
			sb.append(array[i]);
		}
		
		return sb.toString();
		
	}
	
	public static String format(String format, Object... params)
	{
		MessageFormat mf = new MessageFormat(format);
		return mf.format(params);
	}
	
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
