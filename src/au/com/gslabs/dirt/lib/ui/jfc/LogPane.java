package au.com.gslabs.dirt.lib.ui.jfc;

import java.io.IOException;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.event.*;
import javax.swing.text.*;
import javax.swing.text.html.*;
import au.com.gslabs.dirt.lib.util.*;
import au.com.gslabs.dirt.lib.ui.jfc.XHTMLEditorKit;
import java.net.URL;
import java.util.*;
import java.awt.BorderLayout;
import java.awt.Dimension;

public class LogPane extends JScrollPane
{
	
	protected final boolean synchronousAppend = false;
	
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
		
		editor = createEditor();
		kit = (XHTMLEditorKit)editor.getEditorKit();
		
		JEditorPane fill = createEditor();
		fill.setEnabled(false);
		
		JPanel panel = new JPanel();
		panel.setLayout(new BorderLayout());
		panel.add(fill, BorderLayout.CENTER);
		panel.add(editor, BorderLayout.SOUTH);
		setViewportView(panel);
		
		if (FileUtil.isMac())
		{
			setVerticalScrollBarPolicy(JScrollPane.VERTICAL_SCROLLBAR_ALWAYS);
		}
		
		addEventListeners();

		listeners = new ArrayList<LinkListener>();

	}
	
	protected JEditorPane createEditor()
	{
		JEditorPane editor = new Editor();
		editor.setEditorKit(new XHTMLEditorKit());
		editor.setEditable(false);
		editor.setText(wrapInXHTMLTags(""));
		editor.setBorder(BorderFactory.createEmptyBorder());
		return editor;
	}
	
	protected class Editor extends JEditorPane
	{
		public Dimension getPreferredSize()
		{
			Dimension d = super.getPreferredSize();
			d.width = 0;
			d.height = Math.max(1, d.height);
			return d;
		}
	}
	
	protected static boolean tagWrap_InitDone = false;
	protected static String tagWrap_Pre;
	protected static String tagWrap_Post;
	
	protected static void tagWrapInit()
	{
		
		if (tagWrap_InitDone) return;
		
		tagWrap_Pre =
			"<?xml version=\"1.0\" encoding=\"utf-8\" ?>\n" +
			"<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.1//EN\" \n" +
			"\"http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd\">\n" +
			"<html xmlns=\"http://www.w3.org/1999/xhtml\" xml:lang=\"en\">\n" +
			"<head>" +
			"<title/>\n";
		
		tagWrap_Pre +=
			"<link rel=\"stylesheet\" href=\"" +
			FileUtil.getResource("res/styles/logpane/base.css").toString() +
			"\" type=\"text/css\"/>\n";
		
		tagWrap_Pre +=
			"<link rel=\"stylesheet\" href=\"" +
			FileUtil.getResource("res/styles/logpane/platform-mac.css").toString() +
			"\" type=\"text/css\"/>\n";
		
		tagWrap_Pre +=
			"</head>\n" +
			"<body>\n";
			
		tagWrap_Post =
			"</body>\n"+
			"</html>\n";
			
		tagWrap_InitDone = true;
		
	}
	
	protected static String wrapInXHTMLTags(String data)
	{
		tagWrapInit();
		return tagWrap_Pre + data + tagWrap_Post;
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
								lastIsAtEnd = isAtEnd();
							}
						});
				}
			});
		
		this.addComponentListener(new ComponentAdapter()
			{
				public void componentResized(ComponentEvent e)
				{
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
		
		editor.addMouseListener(new MouseAdapter()
			{
				public void mouseClicked(MouseEvent e)
				{
					editor.transferFocus();
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
						handleAppendError(ex, xhtml);
					}
				}
			}
		}
	}
	
	private int handleAppendErrorDepth = 0;
	
	private void handleAppendError(Exception ex, String content)
	{
		
		String message = "";
		Throwable t = ex.getCause();
		if (t != null)
		{
			while (t != null)
			{
				message += t.toString() + "\n";
				t = t.getCause();
			}
		}
		else
		{
			message += ex.toString() + "\n";
		}
		
		System.err.println("LogPane append failed: " + message);
		System.err.println("Content that caused exception: " + content);
		if (handleAppendErrorDepth > 0)
		{
			throw new RuntimeException(ex);
		}
		else
		{
			++handleAppendErrorDepth;
			appendTextLine(message, "error");
			--handleAppendErrorDepth;
		}
		
	}
	
	private void appendXHTMLFragment(String xhtml)
	{
		synchronized (queue)
		{
			if (synchronousAppend)
			{
				try
				{
					doAppendXHTMLFragment(xhtml);
				}
				catch (Exception ex)
				{
					handleAppendError(ex, xhtml);
				}
			}
			else
			{
				buffer.append(xhtml);
				queue.execute(new AppendXHTMLWorker());
			}
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
		String html = TextModifierParser.parse(
			text, TextModifierParser.OutputFormat.XHTML);
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