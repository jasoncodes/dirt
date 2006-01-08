package au.com.gslabs.ui.jfc;

import java.io.IOException;
import javax.swing.*;
import javax.swing.text.*;
import javax.swing.text.html.*;
import au.com.gslabs.dirt.util.*;
import net.sf.pw.ui.XHTMLEditorKit;

public class LogPane extends JEditorPane
{
	
	protected XHTMLEditorKit kit;
	
	public LogPane()
	{
		kit = new XHTMLEditorKit();
		setEditorKit(kit);
		setEditable(false);
	}
	
	public void moveToEnd()
	{
		setCaretPosition(getDocument().getLength());
	}
	
	public void appendXHTML(String xhtml)
	{
		moveToEnd();
		HTMLDocument doc = (HTMLDocument)getDocument();
		try
		{
			kit.insertHTML(doc, 0, xhtml, 0, 0, HTML.Tag.BODY);
		}
		catch (IOException ex)
		{
			throw new RuntimeException("Error appending to LogPane", ex);
		}
		catch (BadLocationException ex)
		{
			throw new RuntimeException("Error appending to LogPane", ex);
		}
	}
	
	public void appendText(String text)
	{
		appendXHTML(TextUtil.stringToHTMLString(text));
	}
	
}