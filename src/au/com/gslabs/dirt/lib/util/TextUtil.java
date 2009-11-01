package au.com.gslabs.dirt.lib.util;

import java.util.Random;
import java.util.ArrayList;
import java.text.MessageFormat;
import java.text.BreakIterator;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.TimeZone;

public final class TextUtil
{
	
	private TextUtil()
	{
	}
	
	private static Random random = null;
	
	public static boolean isEmpty(String str)
	{
		return str == null || str.length() <= 0;
	}
	
	public static ArrayList<String> split(String str, char delim)
	{
		return split(str, delim, -1);
	}
	
	public static ArrayList<String> split(String str, char delim, int limit)
	{
		
		ArrayList<String> tokens = new ArrayList<String>();
		
		StringBuilder token = new StringBuilder();
		
		for (int i = 0; i < str.length(); ++i)
		{
			if (str.charAt(i) == delim && ((limit < 1) || (tokens.size()+1 < limit)))
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
	
	public static String[] splitQuotedHeadTail(String text)
	{
		if (text.length() > 0 && text.charAt(0) == '"')
		{
			int i = text.indexOf('"', 1);
			while (i > -1 && i+1 < text.length())
			{
				if (text.charAt(i+1) != '"') break;
				i = text.indexOf('"', i+2);
			}
			String[] tokens = new String[2];
			if (i > -1 && i < text.length())
			{
				tokens[0] = text.substring(1, i);
				if (i+1 < text.length() && text.charAt(i+1) == ' ')
				{
					tokens[1] = text.substring(i + 2);
				}
				else
				{
					tokens[1] = text.substring(i + 1);
				}
			}
			else
			{
				tokens[0] = text.substring(1);
				tokens[1] = "";
			}
			tokens[0] = tokens[0].replace("\"\"", "\"");
			return tokens;
		}
		else
		{
			return split(text, ' ', 2).toArray(new String[0]);
		}
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
	
	public static String generateRandomAlphaNumeric(int count)
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
	
	public static String formatDateTime(Date date, boolean includeTimeZone, boolean inUTC)
	{
		SimpleDateFormat f = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss" + (includeTimeZone?" Z":""));
		if (inUTC)
		{
			TimeZone tzUTC = TimeZone.getTimeZone("UTC");
			f.setTimeZone(tzUTC);
		}
		return f.format(date);
	}
	
	public static String toTitleCase(String in)
	{
		
		StringBuffer out = new StringBuffer(in.length());
		
		BreakIterator iter = BreakIterator.getWordInstance();
		iter.setText(in);
		int start = iter.first();
		int end;
		int prevStart = 0;
		
		// Consider each word in turn
		for (end = iter.next();
		     end != BreakIterator.DONE;
		     prevStart = start, start = end, end = iter.next())
		{
			
			// Copy the separations between words
			if (start > prevStart)
			out.append(in.subSequence(prevStart, start));
			
			// Convert this word to title case
			out.append(Character.toTitleCase(in.charAt(start)));
			out.append(in.substring(start+1, end).toLowerCase());
			
		}
		
		// Copy any space at the end of the original string
		if (end > -1 && end < in.length())
		{
			out.append(in.substring(end));
		}
		
		// All done.
		return out.toString();
	
	}
	
	public static String repeat(char c, int n)
	{
		StringBuilder out = new StringBuilder(n);
		for (int i = 0; i < n; ++i)
		{
			out.append(c);
		}
		return out.toString();
	}
	
}
