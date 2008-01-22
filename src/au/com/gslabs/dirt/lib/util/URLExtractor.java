package au.com.gslabs.dirt.lib.util;

import java.util.ArrayList;
import java.util.regex.*;

public class URLExtractor
{
	
	public class Token
	{
		private String text;
		private String url;
		public Token(String text, String url)
		{
			this.text = text;
			this.url = url;
		}
		public String getText()
		{
			return this.text;
		}
		public String getUrl()
		{
			return this.url;
		}
	}
	
	ArrayList<Token> tokens = null;
	
	public URLExtractor(String text)
	{
		tokenize(text);
	}
	
	public Token[] getTokens()
	{
		return tokens.toArray(new Token[0]);
	}
	
	public static void main(String[] args)
	{
		
		String html = "<blockquote cite=\"http://example.com/\">\nThis is a&nbsp;test http://www.google.com.au/.\nTest string with an email address foo@example.tld and an URL <sourceforge.net>.\n</blockquote>\n";
		URLExtractor extractor = new URLExtractor(html);
		
		final char ESC = (char)27;
		System.out.print(ESC+"[0m"+ESC+"[1m");
		for (Token token : extractor.getTokens())
		{
			if (token.getUrl() != null)
			{
				System.out.print(ESC+"[34m"+ESC+"[4m");
			}
			System.out.print(token.getText());
			if (token.getUrl() != null)
			{
				System.out.print(ESC+"[0m"+ESC+"[7m");
				System.out.print("<");
				System.out.print(token.getUrl());
				System.out.print(">");
				System.out.print(ESC+"[0m"+ESC+"[1m");
			}
		}
		System.out.print(ESC+"[0m");
		System.out.println();
	}
	
	public static boolean looksLikeAnEmailAddress(String part)
	{
		
		if (part.indexOf(':') > -1 || part.indexOf('/') > -1)
		{
			return false;
		}
		
		int i = part.indexOf('@');
		if (i > 0)
		{
			if (part.indexOf('.', i+1) > -1)
			{
				return true;
			}
		}
		
		return false;
		
	}
	
	private static String removeBefore(
		String src,
		StringBuilder beforeOut, String beforeToMatch)
	{
		if (src.startsWith(beforeToMatch))
		{
			beforeOut.append(beforeToMatch);
			src = src.substring(beforeToMatch.length());
		}
		return src;
	}
	
	private static String removeAfter(
		String src,
		StringBuilder afterOut, String afterToMatch)
	{
		if (src.endsWith(afterToMatch))
		{
			afterOut.insert(0, afterToMatch);
			src = src.substring(0, src.length() - afterToMatch.length());
		}
		return src;
	}
	
	private static String removeBeforeAndAfter(
		String src,
		StringBuilder beforeOut, String beforeToMatch,
		StringBuilder afterOut, String afterToMatch)
	{
		if (src.startsWith(beforeToMatch) && src.endsWith(afterToMatch) &&
		    src.length() >= beforeToMatch.length() + afterToMatch.length())
		{
			src = removeBefore(src, beforeOut, beforeToMatch);
			src = removeAfter(src, afterOut, afterToMatch);
		}
		return src;
	}
	
	private void addToken(String text, String url)
	{
		Token lastToken = tokens.size() > 0 ? tokens.get(tokens.size()-1) : null;
		if (url == null && lastToken != null && lastToken.url == null)
		{
			lastToken.text += text;
		}
		else
		{
			tokens.add(new Token(text, url));
		}
	}
	
	static final Pattern partDelims = Pattern.compile("(\\p{Space}|\"|<|>|&quot;|&lt;|&gt;)+");
	static final Pattern suffixToIgnore = Pattern.compile("[.,?!]+$");
	static final Pattern domainEnd = Pattern.compile("\\.(com|net|org|gov|mil|co)(\\.[a-z]+)?(:[0-9]+)?(/|$)");
	
	private void tokenize(String text)
	{
		
		// break the string up based on delimiter characters
		// the delimiters remain in the result as their own fragments
		ArrayList<String> parts = new ArrayList<String>();
		if (text.length() > 0)
		{
			Matcher partMatcher = partDelims.matcher(text);
			int lastEnd = 0;
			while (partMatcher.find())
			{
				parts.add(text.substring(lastEnd, partMatcher.start()));
				parts.add(text.substring(partMatcher.start(), partMatcher.end()));
				lastEnd = partMatcher.end();
			}
			if (lastEnd < text.length())
			{
				parts.add(text.substring(lastEnd, text.length()));
			}
		}
		
		tokens = new ArrayList<Token>();
		
		// process our string fragments
		for (String part : parts)
		{
			
			StringBuilder before = new StringBuilder();
			StringBuilder after = new StringBuilder();
			String url = null;
			
			// remove any trailing characters we don't want to match against
			Matcher ignoreMatcher = suffixToIgnore.matcher(part);
			if (ignoreMatcher.find())
			{
				part = ignoreMatcher.replaceFirst("");
				after.insert(0, ignoreMatcher.group());
			}
			
			// remove any other character combos that may be wrapping our part
			part = removeBeforeAndAfter(part, before, "(", after, ")");
			part = removeBefore(part, before, "'");
			part = removeAfter(part, after, "'");
			part = removeAfter(part, after, "'s");
			
			// see if what we have left looks like a URL
			String partLower = part.toLowerCase();
			if (partLower.startsWith("news:") && partLower.length() > 5)
			{
				url = part;
			}
			else if (part.startsWith("\\\\") && part.length() > 2)
			{
				url = part;
			}
			else if (partLower.startsWith("mailto:") || (part.indexOf("://") > -1))
			{
				url = part;
			}
			else if (partLower.startsWith("www") && (part.length() > 4) && (part.indexOf('.') > -1) && (part.indexOf('.') < part.length()-1))
			{
				url = "http://" + part;
			}
			else if (partLower.startsWith("ftp.") && (part.indexOf('.', 4) > 0))
			{
				url = "ftp://" + part;
			}
			else if (looksLikeAnEmailAddress(part))
			{
				url = "mailto:" + part;
			}
			else if (domainEnd.matcher(partLower).find())
			{
				url = "http://" + part;
			}
			
			// add the result to the output
			if (before.length() > 0)
			{
				addToken(before.toString(), null);
			}
			addToken(part, url);
			if (after.length() > 0)
			{
				addToken(after.toString(), null);
			}
			
		}
		
	}
	
}
