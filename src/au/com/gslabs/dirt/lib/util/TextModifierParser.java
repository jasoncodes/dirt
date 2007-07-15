package au.com.gslabs.dirt.lib.util;

import java.util.Stack;
import java.util.ArrayList;

public class TextModifierParser
{
	
	public enum OutputFormat
	{
		PLAIN,
		XHTML
	}
	
	protected static final boolean useInlineStyles = true;
	protected static final String[] html_colour_codes = new String[]
		{
			"#ffffff", "#000000", "#000080", "#008000",
			"#ff0000", "#800000", "#800080", "#ff8000",
			"#ffff00", "#00ff00", "#008080", "#00ffff",
			"#0000ff", "#ff00ff", "#808080", "#c0c0c0;"
		};
	
	public static String parse(String text, OutputFormat outputFormat)
	{
		TextModifierParser parser = new TextModifierParser(outputFormat);
		return parser.parse(text);
	}
	
	protected OutputFormat outputFormat;
	protected StringBuffer result;
	protected ArrayList<Entry> active;
	
	public TextModifierParser(OutputFormat outputFormat)
	{
		this.outputFormat = outputFormat;
		reset();
	}
	
	public void reset()
	{
		result = new StringBuffer();
		resetColourParser();
		active = new ArrayList<Entry>();
	}
	
	class Entry
	{
		
		public TextModifier tag;
		public int[] params;
		public boolean supressed;
		
		public boolean equals(Object obj)
		{
			Entry other = (Entry)obj;
			if (this.tag != other.tag ||
			    this.params.length != other.params.length)
			{
				return false;
			}
			for (int i = 0; i < this.params.length; ++i)
			{
				if (this.params[i] != other.params[i])
				{
					return false;
				}
			}
			return true;
		}
		
	}
	
	protected int tagFind(TextModifier tag)
	{
		for (int i = active.size()-1; i >= 0; --i)
		{
			if (active.get(i).tag == tag)
			{
				return i;
			}
		}
		return -1;
	}
	
	protected void tagSet(TextModifier tag)
	{
		
		Entry newEntry = new Entry();
		newEntry.tag = tag;
		newEntry.params = (tag == TextModifier.COLOUR) ? colour_number.clone() : new int[0];
		newEntry.supressed = (tag == TextModifier.COLOUR) && (tagFind(TextModifier.REVERSE) > -1);
		
		int idxExisting = tagFind(tag);
		if (idxExisting > -1 && !active.get(idxExisting).equals(newEntry))
		{
			tagClear(tag);
			idxExisting = -1;
		}
		
		if (idxExisting < 0)
		{
			outputTag(newEntry, true);
			active.add(newEntry);
		}
		
	}
	
	protected void tagClearAll()
	{
		for (int idx = active.size()-1; idx >= 0; --idx)
		{
			outputTag(active.get(idx), false);
		}
		active.clear();
	}
	
	protected void tagClear(TextModifier tag)
	{
		int idxToClear = tagFind(tag);
		if (idxToClear > -1)
		{
			// roll back in reverse order up to and including the one we want to kill
			for (int idx = active.size()-1; idx >= idxToClear; --idx)
			{
				outputTag(active.get(idx), false);
			}
			// roll forward the ones we had to roll back
			for (int idx = idxToClear+1; idx < active.size(); ++idx)
			{
				Entry entry = active.get(idx);
				if (tag == TextModifier.REVERSE && entry.supressed)
				{
					entry.supressed = false;
				}
				outputTag(entry, true);
			}
			// remove the one from the list
			active.remove(idxToClear);
		}
	}
	
	protected void tagToggle(TextModifier tag)
	{
		if (tagFind(tag) > -1)
		{
			tagClear(tag);
		}
		else
		{
			tagSet(tag);
		}
	}
	
	protected void outputTag(Entry entry, boolean opening)
	{
		if (entry.supressed) return;
		final String tagName;
		switch (entry.tag)
		{
			case BOLD:
				tagName = "strong";
				break;
			case UNDERLINE:
				tagName = "u";
				break;
			case COLOUR:
				tagName = "span";
				break;
			case REVERSE:
				tagName = "span";
				break;
			default:
				throw new RuntimeException("Unknown tag: " + entry.tag);
		}
		if (entry.tag == TextModifier.COLOUR)
		{
			if (useInlineStyles)
			{
				String attribs = "style=\"";
				if (entry.params[0] > -1)
				{
					attribs += "color: "+html_colour_codes[entry.params[0]&0xf]+";";
					if (entry.params[1] > -1)
					{
						attribs += " background-color: "+html_colour_codes[entry.params[1]&0xf]+";";
					}
				}
				attribs += "\"";
				outputTag(tagName, opening, attribs);
			}
			else
			{
				String[] classPrefixes = new String[] { "fg", "bg" };
				for (int i = 0; i < classPrefixes.length; ++i)
				{
					if (entry.params[i] > -1)
					{
						outputTag(tagName, opening, "class=\""+classPrefixes[i]+Integer.toHexString(entry.params[i]&0xf)+"\"");
					}
				}
			}
		}
		else if (entry.tag == TextModifier.REVERSE)
		{
			String attribs;
			if (useInlineStyles)
			{
				attribs = "style=\"background: black; color: white;\"";
			}
			else
			{
				attribs = "class=\"reverse\"";
			}
			outputTag(tagName, opening, attribs);
		}
		else
		{
			outputTag(tagName, opening, null);
		}
	}
	
	protected void outputTag(String tagName, boolean opening, String attributes)
	{
		result.append('<');
		if (!opening)
		{
			result.append('/');
		}
		result.append(tagName);
		if (opening && attributes != null && attributes.length() > 0)
		{
			result.append(' ');
			result.append(attributes);
		}
		result.append('>');
	}
	
	protected int colour_pos;
	protected boolean had_comma;
	protected boolean last_was_comma;
	protected int[] colour_number = new int[2];

	protected void resetColourParser()
	{
		colour_pos = 0;
		colour_number[0] = -1;
		colour_number[1] = -1;
		had_comma = false;
		last_was_comma = false;
	}
	
	protected void colourCodeEnd()
	{
		if (colour_pos == 1 && colour_number[0] < 0)
		{
			tagClear(TextModifier.COLOUR);
		}
		else if (colour_number[0] > -1)
		{
			tagSet(TextModifier.COLOUR);
		}
		colour_pos = 0;
		colour_number[0] = -1;
		colour_number[1] = -1;
	}
	
	public String parse(String text)
	{
		
		if (outputFormat == OutputFormat.XHTML)
		{
			text = TextUtil.stringToHTMLString(text);
			text = TextUtil.convertUrlsToLinks(text);
		}
		
		result.ensureCapacity(result.length() + text.length() * 3);
		
		for (int i = 0; i < text.length(); ++i)
		{
			
			char c = text.charAt(i);
			
			if (colour_pos == 0)
			{
				resetColourParser();
			}
			
			if ( colour_pos > 0 && // is inside a colour code and
			    ( (c == ',' && had_comma) || // is either a 2nd comma
			      (c != ',' && !Character.isDigit(c)) ) // or a non-valid character
			   )
			{
				colourCodeEnd();
				had_comma = false;
				if (last_was_comma)
				{
					result.append(',');
				}
			}
			
			if (c == TextModifier.BOLD.getChar())
			{
				colourCodeEnd();
				tagToggle(TextModifier.BOLD);
			}
			else if (c == TextModifier.ORIGINAL.getChar())
			{
				colourCodeEnd();
				tagClearAll();
			}
			else if (c == TextModifier.REVERSE.getChar())
			{
				colourCodeEnd();
				tagToggle(TextModifier.REVERSE);
			}
			else if (c == TextModifier.UNDERLINE.getChar())
			{
				colourCodeEnd();
				tagToggle(TextModifier.UNDERLINE);
			}
			else if (c == TextModifier.COLOUR.getChar())
			{
				int idx = tagFind(TextModifier.COLOUR);
				if (idx > -1)
				{
					colour_number[1] = active.get(idx).params[1];
				}
				tagClear(TextModifier.COLOUR);
				colour_pos = 1;
			}
			else if (Character.isDigit(c))
			{
				if (colour_pos > 0)
				{
					colour_pos++;
					if (colour_pos > 3)
					{
						colourCodeEnd();
					}
					else
					{
						int x = had_comma ? 1 : 0;
						if (colour_number[x] < 0)
						{
							colour_number[x] = 0;
						}
						colour_number[x] *= 10;
						colour_number[x] += (c - '0');
					}
				}
				if (colour_pos == 0)
				{
					result.append(c);
				}
			}
			else if (c == ',')
			{
				if (colour_pos > 0)
				{
					if (had_comma)
					{
						colourCodeEnd();
						result.append(c);
						result.append(c);
					}
					else
					{
						colour_pos = 1;
						had_comma = true;
					}
				}
				else
				{
					result.append(c);
				}
			}
			else
			{
				colourCodeEnd();
				result.append(c);
			}
			
			last_was_comma = (c == ',');
			
		}
		
		colourCodeEnd();
		tagClearAll();
		
		System.out.println(result);
		return result.toString();
		
	}
	
}