package au.com.gslabs.dirt.lib.util;

import java.util.Random;
import java.util.ArrayList;
import java.text.MessageFormat;

public final class TextUtil
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
		
		boolean lastWasBlankChar = false;
		int len = string.length();
		char c;

		for (int i = 0; i < len; i++)
		{
			c = string.charAt(i);
			if (c == ' ')
			{
				// blanks get special treatment:
				// since continuous whitespace in HTML collapses to one space
				// we need to insert non-breaking spaces to pad it out
				// unfortunately if we add just non-breaking spaces,
				// we'll lose the ability to word wrap. A nice compromise
				// is to replace every second whitespace character with an &nbsp;
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
				
				// HTML Special Chars
				switch (c)
				{
					case '"':
						sb.append("&quot;");
						break;
					case '&':
						sb.append("&amp;");
						break;
					case '<':
						sb.append("&lt;");
						break;
					case '>':
						sb.append("&gt;");
						break;
					case '\n':
						sb.append("<br />");
						break;
					default:
						sb.append(c);
						break;
				}
				
			}
		}
		
		return sb.toString();
		
	}
	
	public static String convertUrlsToLinks(String html)
	{
		URLExtractor extractor = new URLExtractor(html);
		StringBuffer sb = new StringBuffer(html.length());
		for (URLExtractor.Token token : extractor.getTokens())
		{
			if (token.getUrl() != null)
			{
				String href = TextModifierParser.parse(
					token.getUrl(), TextModifierParser.OutputFormat.PLAIN);
				sb.append("<a href=\"");
				sb.append(href);
				sb.append("\">");
				sb.append(token.getText());
				sb.append("</a>");
			}
			else
			{
				sb.append(token.getText());
			}
		}
		return sb.toString();
	}
	
	protected static java.text.SimpleDateFormat sdfDateTime = new java.text.SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
	
	public static String formatDateTime(java.util.Date d)
	{
		return sdfDateTime.format(d.getTime());
	}
	
}
