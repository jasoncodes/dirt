package au.com.gslabs.dirt.ui.jfc;

import java.io.IOException;
import javax.swing.*;
import javax.swing.text.*;
import javax.swing.text.html.*;
import au.com.gslabs.dirt.lib.util.*;
import au.com.gslabs.dirt.lib.ui.jfc.XHTMLEditorKit;
import java.net.URL;

public class LogPane extends JScrollPane
{
	
	protected JEditorPane editor;
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
		super();
		editor = new JEditorPane();
		setViewportView(editor);
		kit = new XHTMLEditorKit();
		editor.setEditorKit(kit);
		editor.setEditable(false);
	}
	
	public JEditorPane getEditor()
	{
		return editor;
	}
	
	public void moveToEnd()
	{
		editor.setCaretPosition(editor.getDocument().getLength());
	}
	
	public boolean isAtEnd()
	{
		
		JScrollBar bar = getVerticalScrollBar();
		
		if (!bar.isVisible())
		{
			return true;
		}
		
		return bar.getValue()+bar.getVisibleAmount() == bar.getMaximum();
		
	}
	
	public void appendXHTMLFragment(String xhtml)
	{
		boolean wasAtEnd = this.isAtEnd();
		int scrollPos = getVerticalScrollBar().getValue();
		try
		{
			HTMLDocument doc = (HTMLDocument)editor.getDocument();
			String data = wrapInXHTMLTags("<p>"+xhtml+"</p>");
			kit.insertHTML(doc, doc.getEndPosition().getOffset()-1, data, 1, 0, null);
		}
		catch (IOException ex)
		{
			throw new RuntimeException("Error appending to LogPane", ex);
		}
		catch (BadLocationException ex)
		{
			throw new RuntimeException("Error appending to LogPane", ex);
		}
		if (wasAtEnd)
		{
			moveToEnd();
		}
		else
		{
			getVerticalScrollBar().setValue(scrollPos);
		}
	}
	
	public void appendText(String text)
	{
		appendXHTMLFragment(TextUtil.stringToHTMLString(text));
	}
	
}