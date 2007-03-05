package au.com.gslabs.dirt.lib.ui.jfc;

import java.io.IOException;
import java.awt.event.*;
import javax.swing.*;
import java.awt.GridBagLayout;
import java.awt.GridBagConstraints;
import javax.swing.event.*;
import javax.swing.text.*;
import javax.swing.text.html.*;
import au.com.gslabs.dirt.lib.util.*;
import au.com.gslabs.dirt.lib.ui.jfc.XHTMLEditorKit;
import java.net.URL;
import java.util.*;

public class LogPane extends JScrollPane
{
	
	public class LinkEvent extends EventObject
	{
		protected URL url;
		public LinkEvent(LogPane source, URL url)
		{
			super(source);
			this.url = url;
		}
		public URL getURL()
		{
			return this.url;
		}
	}
	
	public interface LinkListener extends EventListener
	{
		public void linkClicked(LinkEvent e);
	}

	protected JEditorPane editor;
	protected XHTMLEditorKit kit;
	protected ArrayList<LinkListener> listeners;
	protected boolean lastIsAtEnd;
	
	public LogPane()
	{
		
		super();
		
		lastIsAtEnd = true;

		editor = new JEditorPane();
		kit = new XHTMLEditorKit();
		editor.setEditorKit(kit);
		editor.setEditable(false);
		setViewportView(editor);
		setLayout(new LogPaneLayout());
		setVerticalScrollBarPolicy(JScrollPane.VERTICAL_SCROLLBAR_ALWAYS);

		addEventListeners();

		listeners = new ArrayList<LinkListener>();

	}
	
	protected class LogPaneLayout extends ScrollPaneLayout
	{

	 	public void layoutContainer(java.awt.Container parent)
		{
			super.layoutContainer(parent);
			positionContentArea();
		}
		
	}
	
	protected void positionContentArea()
	{
		FileUtil.debugMsg("LogPane", "positionContentArea");
		int pref = editor.getPreferredSize().height;
		int actual = editor.getSize().height;
		if (actual > pref)
		{
			editor.setLocation(0, actual-pref);
		}
	}

	protected static String strStylesheetURL = FileUtil.getResource("res/styles/logpane.css").toString();
	
	protected static String wrapInXHTMLTags(String data)
	{
		return
			"<?xml version=\"1.0\" encoding=\"utf-8\" ?>\n" +
			"<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.1//EN\" \n" +
			"\"http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd\">\n" +
			"<html xmlns=\"http://www.w3.org/1999/xhtml\" xml:lang=\"en\">\n" +
			"<head>" +
			"<title/>\n" +
			"<link rel=\"stylesheet\" href=\""+strStylesheetURL+"\" type=\"text/css\"/>\n"+
			"</head>\n" +
			"<body>\n" +
			data +
			"</body>\n"+
			"</html>\n";			
	}
	
	protected void addEventListeners()
	{
		
		getVerticalScrollBar().addAdjustmentListener(new AdjustmentListener()
			{
				public void adjustmentValueChanged(AdjustmentEvent evt)
				{
					SwingUtilities.invokeLater(new Runnable()
						{
							public void run()
							{
								FileUtil.debugMsg("LogPane", "adjustmentValueChanged " + lastIsAtEnd + " " + isAtEnd());
								lastIsAtEnd = isAtEnd();
							}
						});
				}
			});
		
		this.addComponentListener(new ComponentAdapter()
			{
				public void componentResized(ComponentEvent e)
				{
					FileUtil.debugMsg("LogPane", "componentResized " + lastIsAtEnd + " " + isAtEnd());
					if (lastIsAtEnd)
					{
						SwingUtilities.invokeLater(new DoScroll(-1));
					}
				}
			});
		
		editor.addHyperlinkListener(new HyperlinkListener()
			{
				public void hyperlinkUpdate(HyperlinkEvent evt)
				{
					if (evt.getEventType() == HyperlinkEvent.EventType.ACTIVATED)
					{
						raiseLinkEvent(evt.getURL());
					}
				}
			});
		
	}
	
	protected void raiseLinkEvent(URL url)
	{
		LinkEvent e = new LinkEvent(this, url);
		for (int i = listeners.size()-1; i >= 0; --i)
		{
			listeners.get(i).linkClicked(e);
		}	
	}
	
	public void clearText()
	{
		editor.setText(wrapInXHTMLTags(""));
		lastIsAtEnd = true;
	}
	
	public JEditorPane getEditor()
	{
		return editor;
	}
	
	public void moveToEnd()
	{
		
		editor.setCaretPosition(editor.getDocument().getLength());
		
		JScrollBar bar = getVerticalScrollBar();
		bar.setValue(bar.getMaximum());
		
		FileUtil.debugMsg("LogPane", "moveToEnd " + lastIsAtEnd + " " + isAtEnd() + " " + bar.getMaximum());
		
		lastIsAtEnd = true;
		
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
	
	protected class DoScroll implements Runnable
	{
		
		protected int pos;
		
		public DoScroll(int pos)
		{
			this.pos = pos;
		}
		
		public void run()
		{
			if (pos < 0) // wasAtEnd
			{
				moveToEnd();
			}
			else
			{
				getVerticalScrollBar().setValue(pos);
			}
			positionContentArea();
			FileUtil.debugMsg("LogPane", "DoScroll " + lastIsAtEnd + " " + isAtEnd());
			lastIsAtEnd = isAtEnd();
		}
		
	}
	
	StringBuilder buffer = new StringBuilder();
	WorkQueue queue = new WorkQueue(1);
	
	private class AppendXHTMLWorker implements Runnable
	{
		public void run()
		{
			String xhtml;
			synchronized (queue)
			{
				xhtml = buffer.toString();
				buffer = new StringBuilder();
			}
			if (xhtml.length() > 0)
			{
				synchronized (editor)
				{
					try
					{
						doAppendXHTMLFragment(xhtml);
					}
					catch (Exception ex)
					{
						handleAppendError(ex);
					}
				}
			}
		}
	}
	
	private void handleAppendError(Exception ex)
	{
		Throwable t = ex.getCause();
		if (t != null)
		{
			while (t != null)
			{
				appendTextLine(t.toString(), "error");
				t = t.getCause();
			}
		}
		else
		{
			appendTextLine(ex.toString(), "error");
		}
	}
	
	private void appendXHTMLFragment(String xhtml)
	{
		synchronized (queue)
		{
			buffer.append(xhtml);
			queue.execute(new AppendXHTMLWorker());
		}
	}
	
	private void doAppendXHTMLFragment(String xhtml)
	{
		
		xhtml = wrapInXHTMLTags(xhtml);
		
		int scrollPos = this.isAtEnd() ? -1 : getVerticalScrollBar().getValue();
		
		try
		{
			HTMLDocument doc = (HTMLDocument)editor.getDocument();
			kit.insertHTML(doc, doc.getEndPosition().getOffset()-1, xhtml, 1, 0, null);
		}
		catch (IOException ex)
		{
			throw new RuntimeException("Error appending to LogPane", ex);
		}
		catch (BadLocationException ex)
		{
			throw new RuntimeException("Error appending to LogPane", ex);
		}
		
		SwingUtilities.invokeLater(new DoScroll(scrollPos));
		
	}
	
	public void appendXHTMLLine(String xhtml)
	{
		appendXHTMLLine(xhtml, null);
	}
	
	public void appendXHTMLLine(String xhtml, String className)
	{
		String data;
		data = "<p";
		if (className != null && className.trim().length() > 0)
		{
			data += " class=\""+className+"\"";
		}
		data += ">"+xhtml+"</p>\n";
		appendXHTMLFragment(data);
	}
	
	public void appendTextLine(String text)
	{
		appendTextLine(text, null);
	}
	
	public void appendTextLine(String text, String className)
	{
		String html = TextModifierParser.parseText(text, TextModifierParser.OutputFormat.XHTML);
		appendXHTMLLine(html, className);
	}
	
	public void addLinkListener(LinkListener l)
	{
		listeners.add(l);
	}
	
	public void removeLinkListener(LinkListener l)
	{
		listeners.remove(l);
	}

}