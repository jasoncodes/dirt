package au.com.gslabs.dirt.ui.jfc;

import java.io.IOException;
import javax.swing.*;
import javax.swing.text.*;
import javax.swing.text.html.*;
import au.com.gslabs.dirt.util.*;
import au.com.gslabs.dirt.lib.ui.jfc.XHTMLEditorKit;
import java.net.URL;

public class LogPane extends JEditorPane
{
	
	protected XHTMLEditorKit kit;
	
	protected static String wrapInXHTMLTags(String data)
	{
		URL stylesheet = FileUtil.getResource("res/styles/logpane.css");
		return
			"<?xml version=\"1.0\" encoding=\"iso-8859-1\" ?>\n" +
			"<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.1//EN\" \n" +
			"\"http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd\">\n" +
			"<html xmlns=\"http://www.w3.org/1999/xhtml\" xml:lang=\"en\">\n" +
			"<head>\n" +
			"<title/>\n" +
			"<link rel=\"stylesheet\" href=\""+stylesheet+"\" type=\"text/css\"/>\n"+
			"</head>\n" +
			"<body>\n" +
			data +
			"</body>\n"+
			"</html>\n";			
	}

	public LogPane()
	{
		kit = new XHTMLEditorKit();
		setEditorKit(kit);
		setEditable(false);
		System.out.println(wrapInXHTMLTags(""));
	}
	
	public void moveToEnd()
	{
		setCaretPosition(getDocument().getLength());
	}
	
	public void appendXHTMLFragment(String xhtml)
	{
		moveToEnd();
		HTMLDocument doc = (HTMLDocument)getDocument();
		try
		{
			String data = wrapInXHTMLTags("<p>"+xhtml+"</p>");
			kit.insertHTML(doc, doc.getEndPosition().getOffset()-1, data, 0, 0, HTML.Tag.P);
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
		appendXHTMLFragment(TextUtil.stringToHTMLString(text));
	}
	
}