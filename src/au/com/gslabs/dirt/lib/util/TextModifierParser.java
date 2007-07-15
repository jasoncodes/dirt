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
	
	public static String parse(String text, OutputFormat outputFormat)
	{
		TextModifierParser parser = new TextModifierParser(outputFormat);
		return parser.parse(text);
	}
	
	class Tag
	{
		public String name;
		public boolean active;
		public Tag(String name)
		{
			this.name = name;
			this.active = false;
		}
	}
	
	class TagEntry
	{
		public Tag tag;
		public String attribs;
		public TagEntry(Tag tag, String attribs)
		{
			this.tag = tag;
			this.attribs = attribs;
		}
	}
	
	protected OutputFormat outputFormat;
	
	protected StringBuffer result;
	protected int colour_pos;
	protected boolean had_comma;
	protected boolean last_was_comma;
	protected int[] colour_number = new int[2];
	protected boolean[] colour_number_valid = new boolean[2];
	
	protected Tag tagFont, tagSpan, tagBold, tagUnderline;
	protected Stack<TagEntry> tag_stack;
	protected ArrayList<TagEntry> reverse_tags;
	boolean reverse_mode;
	
	public TextModifierParser(OutputFormat outputFormat)
	{
		this.outputFormat = outputFormat;
		reset();
	}
	
	public void reset()
	{
		result = new StringBuffer();
		resetColour();
		tag_stack = new Stack<TagEntry>();
		tagFont = new Tag("span");
		tagSpan = new Tag("span");
		tagBold = new Tag("b");
		tagUnderline = new Tag("u");
		reverse_tags = new ArrayList<TagEntry>();
		reverse_mode = false;
	}
	
	protected void resetColour()
	{
		colour_pos = 0;
		had_comma = false;
		last_was_comma = false;
		colour_number[0] = 0;
		colour_number[1] = 0;
		colour_number_valid[0] = false;
		colour_number_valid[1] = false;
	}
	
	protected void tagHelperEnd(Tag tag)
	{
		if (outputFormat == OutputFormat.XHTML)
		{
			result.append("</"+tag.name+">");
		}
	}
	
	protected void tagHelperStart(Tag tag, String attribs)
	{
		if (outputFormat == OutputFormat.XHTML)
		{
			result.append("<"+tag.name+" "+attribs+">");
		}
	}
	
	protected String tagEnd(Tag tag)
	{
		String last_attribs = "";
		if (tag.active)
		{
			Stack<TagEntry> undo_stack = new Stack<TagEntry>();
			while (true)
			{
				TagEntry entry = tag_stack.pop();
				if (entry.tag.name.equals(tag.name))
				{
					last_attribs = entry.attribs;
					break;
				}
				undo_stack.push(entry);
				tagHelperEnd(entry.tag);
			}
			tagHelperEnd(tag);
			tag.active = false;
			while (!undo_stack.empty())
			{
				TagEntry entry = undo_stack.pop();
				tag_stack.push(entry);
				tagHelperStart(entry.tag, entry.attribs);
			}
		}
		return last_attribs;
	}
	
	protected String tagStart(Tag tag, String attribs)
	{
		String last_attribs = tagEnd(tag);
		tag.active = true;
		tag_stack.push(new TagEntry(tag, attribs));
		tagHelperStart(tag, attribs);
		return last_attribs;
	}
	
	protected void tagToggle(Tag tag)
	{
		if (tag.active)
		{
			tagEnd(tag);
		}
		else
		{
			tagStart(tag, "");
		}
	}
	
	protected void cleanupStack()
	{
		while (!tag_stack.empty())
		{
			TagEntry entry = tag_stack.pop();
			tagHelperEnd(entry.tag);
		}
	}
	
	protected void tagColourStart(Tag tag, String attribs)
	{
		if (reverse_mode)
		{
			reverse_tags.add(new TagEntry(tag, attribs));
		}
		else
		{
			tagStart(tag, attribs);
		}
	}
	
	protected void tagColourEnd(Tag tag)
	{
		if (reverse_mode)
		{
			reverse_tags.add(new TagEntry(tag, ""));
		}
		else
		{
			tagEnd(tag);
		}
	}
	
	protected void colourCodeEnd()
	{
		if (colour_pos == 1 && !colour_number_valid[0])
		{
			tagColourEnd(tagSpan);
			tagColourEnd(tagFont);
		}
		else if (colour_number_valid[0])
		{
			tagColourStart(tagFont, "class=\"fg"+Integer.toHexString(colour_number[0])+"\"");
			if (colour_number_valid[1])
			{
				tagColourStart(tagSpan, "class=\"bg"+Integer.toHexString(colour_number[1])+"\"");
			}
		}
		colour_number_valid[0] = false;
		colour_number_valid[1] = false;
		colour_pos = 0;
	}
	
	protected void tagReverse()
	{
		
		reverse_mode = !reverse_mode;
		
		if (reverse_mode)
		{
			
			String font_attribs = tagStart(tagFont, "");
			String span_attribs = tagStart(tagSpan, "class=\"reverse\"");
			reverse_tags.clear();
			
			if (font_attribs.length() > 0)
			{
				reverse_tags.add(new TagEntry(tagFont, font_attribs));
			}
			if (span_attribs.length() > 0)
			{
				reverse_tags.add(new TagEntry(tagSpan, span_attribs));
			}
			
		}
		else
		{
			
			tagEnd(tagSpan);
			tagEnd(tagFont);
			
			for (TagEntry entry : reverse_tags)
			{
				if (entry.attribs.length() > 0)
				{
					tagStart(entry.tag, entry.attribs);
				}
				else
				{
					tagEnd(entry.tag);
				}
			}
			
		}
		
	}
	
	public String parse(String text)
	{
		
		reset();
		
		if (outputFormat == OutputFormat.XHTML)
		{
			text = TextUtil.stringToHTMLString(text);
			text = TextUtil.convertUrlsToLinks(text);
		}
		
		result.ensureCapacity(text.length() * 3);
		
		for (int i = 0; i < text.length(); ++i)
		{
			
			char c = text.charAt(i);
			
			if (colour_pos == 0)
			{
				resetColour();
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
				tagToggle(tagBold);
			}
			else if (c == TextModifier.ORIGINAL.getChar())
			{
				colourCodeEnd();
				cleanupStack();
			}
			else if (c == TextModifier.REVERSE.getChar())
			{
				colourCodeEnd();
				tagReverse();
			}
			else if (c == TextModifier.UNDERLINE.getChar())
			{
				colourCodeEnd();
				tagToggle(tagUnderline);
			}
			else if (c == TextModifier.COLOUR.getChar())
			{
				resetColour();
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
						if (!colour_number_valid[x])
						{
							colour_number[x] = 0;
							colour_number_valid[x] = true;
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
		cleanupStack();
		
		return result.toString();
		
	}
	
}