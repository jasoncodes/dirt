package au.com.gslabs.dirt.lib.ui.jfc;

import java.io.IOException;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.event.*;
import javax.swing.text.*;
import javax.swing.text.html.*;
import au.com.gslabs.dirt.lib.util.*;
import au.com.gslabs.dirt.lib.ui.jfc.XHTMLEditorKit;
import java.net.URL;
import java.util.*;

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
	protected int redLine;
	protected Overlay overlay;
	protected boolean linkActivatedWhileFocused = false;
	
	public LogPane()
	{
		
		super();
		
		lastIsAtEnd = true;
		redLine = -1;
		overlay = new Overlay();
		
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
		getVerticalScrollBar().setUnitIncrement(4);
		
		addEventListeners();
		
		listeners = new ArrayList<LinkListener>();
		
	}
	
	protected enum OverlayArrowPulseState
	{
		DRIVE,
		HOLD,
		DECAY
	}
	
	protected class Overlay
	{
		
		protected int refcount = 0;
		protected boolean arrow_visible = false;
		protected float arrow_pulse_pos = 0f;
		protected OverlayArrowPulseState arrow_pulse_state;
		protected javax.swing.Timer arrow_timer = new javax.swing.Timer(0, new ArrowTimerListener());
		
		static final int arrow_delay_drive = 50;
		static final int arrow_delay_hold = 500;
		static final int arrow_delay_decay = 50;
		
		static final float arrow_cR1 = 0.6f;
		static final float arrow_cG1 = 0.6f;
		static final float arrow_cB1 = 0.6f;
		static final float arrow_a1 = 0.3f;
		
		static final float arrow_cR2 = 0.9f;
		static final float arrow_cG2 = 0.9f;
		static final float arrow_cB2 = 0.2f;
		static final float arrow_a2 = 0.8f;
		
		static final float arrow_drive_increment = 0.075f;
		static final float arrow_drive_multiplier = 1.200f;
		
		static final float arrow_decay_increment = -0.020f;
		static final float arrow_decay_multiplier = 0.980f;
		
		protected class ArrowTimerListener implements ActionListener
		{
			public void actionPerformed(ActionEvent e)
			{
				switch (arrow_pulse_state)
				{
					
					case DRIVE:
						if (arrow_pulse_pos < 1f)
						{
							arrow_pulse_pos += arrow_drive_increment;
							arrow_pulse_pos *= arrow_drive_multiplier;
						}
						if (arrow_pulse_pos >= 1f)
						{
							arrow_pulse_pos = 1f;
							arrow_pulse_state = OverlayArrowPulseState.HOLD;
							arrow_timer.setDelay(arrow_delay_hold);
						}
						break;
						
					case HOLD:
						arrow_pulse_state = OverlayArrowPulseState.DECAY;
						arrow_timer.setDelay(arrow_delay_decay);
						break;
						
					case DECAY:
						if (arrow_pulse_pos > 0f)
						{
							arrow_pulse_pos += arrow_decay_increment;
							arrow_pulse_pos *= arrow_decay_multiplier;
						}
						if (arrow_pulse_pos <= 0f)
						{
							arrow_pulse_pos = 0f;
							arrow_timer.stop();
						}
						break;
						
				}
				repaintArrowRegion();
			}
		}
		
		public void setArrowVisible(boolean newValue)
		{
			if (newValue != arrow_visible)
			{
				arrow_visible = newValue;
				updateRefCount(newValue);
				repaintArrowRegion();
			}
		}
		
		public void pulseArrow()
		{
			if (arrow_visible)
			{
				arrow_pulse_state = OverlayArrowPulseState.DRIVE;
				arrow_timer.setDelay(arrow_delay_drive);
				arrow_timer.start();
			}
		}
		
		protected void updateRefCount(boolean newValue)
		{
			refcount += newValue ? 1 : -1;
			if (refcount == 0)
			{
				getViewport().setScrollMode(JViewport.BLIT_SCROLL_MODE);
			}
			else if (refcount == 1)
			{
				getViewport().setScrollMode(JViewport.SIMPLE_SCROLL_MODE);
			}
		}
		
		public void paint(Graphics g)
		{
			if (arrow_visible)
			{
				drawArrow(g);
			}
		}
		
		protected Polygon getPositionedArrow(boolean includeScrollOffset)
		{
			Rectangle rectView = getViewport().getViewRect();
			Polygon p = createArrow();
			p.translate(rectView.width-12, rectView.height-12);
			if (includeScrollOffset)
			{
				p.translate(rectView.x, rectView.y);
			}
			return p;
		}
		
		protected final float blend(float v1, float v2, float pos)
		{
			return (v2-v1)*pos + v1;
		}
		
		protected void drawArrow(Graphics g)
		{
			
			Graphics2D g2 = (Graphics2D)g;
			g2.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);
			
			Polygon p = getPositionedArrow(true);
			
			final float cR = blend(arrow_cR1, arrow_cR2, arrow_pulse_pos);
			final float cG = blend(arrow_cG1, arrow_cG2, arrow_pulse_pos);
			final float cB = blend(arrow_cB1, arrow_cB2, arrow_pulse_pos);
			final float a = blend(arrow_a1, arrow_a2, arrow_pulse_pos);
			
			g.setColor(new Color(cR, cG, cB, a));
			g.fillPolygon(p);
			g.setColor(new Color(cR*0.75f, cG*0.75f, cB*0.75f, a));
			g.drawPolygon(p);
			
		}
		
		protected void repaintArrowRegion()
		{
			Rectangle rect = getPositionedArrow(false).getBounds();
			rect.grow(8,8);
			LogPane.this.repaint(rect);
		}
		
		public void repaint()
		{
			if (arrow_visible)
			{
				repaintArrowRegion();
			}
		}
		
		protected Polygon createArrow()
		{
			
			/*
			   54321012345
			7     *****   
			6     *   *   
			5     *   *   
			4     *   *   
			3  ****   ****
			2   *       * 
			1     *   *   
			0       *    X <-- origin (0,0)
			*/
			final int[] xpoints = new int[] { +0, -5, -2, -2, +2, +2, +5, 0 };
			final int[] ypoints = new int[] { +0, -3, -3, -7, -7, -3, -3, 0 };
			final int scaleX = 2;
			final int scaleY = 3;
			
			for (int i = 0; i < xpoints.length; ++i)
			{
				xpoints[i] *= scaleX;
				ypoints[i] *= scaleY;
			}
			
			Polygon p = new Polygon(xpoints, ypoints, 7);
			p.translate(scaleX*-5, 0);
			return p;
			
		}
		
	}
	
	public void addTestData()
	{
		
		for (int i = 0; i < 10; i++)
		{
			StringBuilder sb = new StringBuilder();
			for (int j = 0; j < 7; ++j)
			{
				sb.append("Line ");
				sb.append(i);
				sb.append(' ');
			}
			String tmp = sb.toString().trim();
			appendTextLine(tmp);
		}
		appendTextLine("Testing 1 2 3. http://dirt.gslabs.com.au/.");
		char ctrl_b = '\u0002';
		char ctrl_c = '\u0003';
		char ctrl_r = '\u0016';
		char ctrl_u = '\u001f';
		appendTextLine("this " + ctrl_b + "is" + ctrl_b + " " + ctrl_u + "a " + ctrl_c + "9,1test" + ctrl_c + " line");
		appendXHTMLLine("alpha <span style=\"color: green\">beta</span> <span style=\"background: yellow;\">delta</span> gamma -- green white black yellow");
		appendXHTMLLine("alpha <span style=\"background-color: yellow\">beta</span> <span style=\"color: green\">delta</span> gamma -- black yellow green white");
		appendXHTMLLine("alpha <span style=\"color: green\">beta <span style=\"background: yellow\">delta</span></span> gamma -- green white green yellow");
		appendXHTMLLine("<span style=\"background: #e0e0e0\"><span style=\"color: #000080\">these words should be on a single line</span></span>");
		appendTextLine(ctrl_c + "9,1green black " + ctrl_c + "4red black");
		appendTextLine(ctrl_c + "9,1green black" + ctrl_c + " black white");
		appendTextLine(ctrl_c + "3,green");
		appendXHTMLLine("no <span style=\"background: yellow\"></span>colour<span style=\"background: #e0e0e0\"></span> on <b></b>this <span style=\"color: red\"></span>line");
		appendXHTMLLine("a single 'x' with yellow bg --&gt; <span style=\"background: yellow\">x</span> &lt;--");
		appendTextLine(ctrl_c + "2,15blue-grey " + ctrl_r + "reverse" + ctrl_r + " blue-grey " + ctrl_c + "4red-grey " + ctrl_r + "rev" + ctrl_c + ctrl_c + "2erse" + ctrl_r + " blue-white " + ctrl_c + "black-white " + ctrl_r + "reverse");
		appendTextLine("Should have two spaces between letters: " + ctrl_c + "1t " + ctrl_c + "1 " + ctrl_c + "1e " + ctrl_c + " " + ctrl_c + "1s  t !");
		appendTextLine("Space Test: 1 2  3   4    . exclamation line up -> !");
		appendTextLine("\n\t\u00038,0 \u2584\u2588\u2588\u2588\u2588\u2588\u2584\n\t\u00038,0\u2588\u2588\u00031,1  \u00038,0\u2588\u00031,1  \u00038,0\u2588\u2588\n\t\u00038,0\u2588\u2588\u2588\u2588\u2588\u2588\u2588\u2588\u2588\n\t\u00038,0\u2588\u00038,4\u2580\u00038,0\u2588\u2588\u2588\u2588\u2588\u00038,4\u2580\u00038,0\u2588\n\t\u00038,0 \u2580\u00038,4\u2584\u2584\u2584\u2584\u2584\u00038,0\u2580");
		appendTextLine("Hello World  \u039A\u03B1\u03BB\u03B7\u03BC\u03AD\u03C1\u03B1 \u03BA\u03CC\u03C3\u03BC\u03B5  \u3053\u3093\u306B\u3061\u306F \u4E16\u754C");
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
		
		@Override
		public Dimension getPreferredSize()
		{
			Dimension d = super.getPreferredSize();
			d.width = 0;
			d.height = Math.max(1, d.height);
			return d;
		}
		
		@Override
		public void paint(Graphics g)
		{
			
			super.paint(g);
			if (this != editor) return;
			
			if (redLine >= 0)
			{
				int[] offsets = getLineViewOffsets();
				if (redLine < offsets.length)
				{
					g.setColor(Color.RED);
					g.drawLine(0, offsets[redLine], getWidth(), offsets[redLine]);
				}
			}
			
			overlay.paint(g);
			
		}
		
	}
	
	public int[] getLineViewOffsets()
	{
		try
		{
			
			ArrayList<Integer> offsets = new ArrayList<Integer>();
			HTMLDocument doc = (HTMLDocument)editor.getDocument();
			for (int offset = 0; offset < doc.getLength(); offset = doc.getParagraphElement(offset).getEndOffset() + 1)
			{
				Rectangle rect = editor.modelToView(offset);
				if (rect.height > 0)
				{
					int y = rect.y - 1;
					offsets.add(y);
				}
			}
			
			int[] retVal = new int[offsets.size()];
			for (int i = 0; i < offsets.size(); ++i)
			{
				retVal[i] = offsets.get(i);
			}
			return retVal;
			
		}
		catch (javax.swing.text.BadLocationException ex)
		{
			throw new RuntimeException(ex);
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
		
		if (FileUtil.isMac())
		{
			tagWrap_Pre +=
				"<link rel=\"stylesheet\" href=\"" +
				FileUtil.getResource("res/styles/logpane/platform-mac.css").toString() +
				"\" type=\"text/css\"/>\n";
		}
		
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
								if (lastIsAtEnd)
								{
									overlay.setArrowVisible(false);
								}
							}
						});
				}
			});
		
		this.addComponentListener(new ComponentAdapter()
			{
				@Override
				public void componentResized(ComponentEvent e)
				{
					if (lastIsAtEnd)
					{
						SwingUtilities.invokeLater(new DoScroll(-1));
					}
					overlay.repaint();
				}
			});
		
		new TopLevelWindowEventProxy(this).addWindowListener(new WindowAdapter()
			{
				@Override
				public void windowActivated(WindowEvent e)
				{
					javax.swing.Timer t = new javax.swing.Timer(250, new ActionListener()
						{
							public void actionPerformed(ActionEvent e)
							{
								linkActivatedWhileFocused = true;
							}
						});
					t.setRepeats(false);
					t.start();
				}
				@Override
				public void windowDeactivated(WindowEvent e)
				{
					linkActivatedWhileFocused = false;
				}
			});
		
		editor.addHyperlinkListener(new HyperlinkListener()
			{
				public void hyperlinkUpdate(HyperlinkEvent evt)
				{
					if (evt.getEventType() == HyperlinkEvent.EventType.ACTIVATED)
					{
						if (linkActivatedWhileFocused)
						{
							raiseLinkEvent(evt.getURL());
						}
						linkActivatedWhileFocused = true;
					}
				}
			});
			
		editor.addMouseListener(new MouseAdapter()
			{
				@Override
				public void mouseClicked(MouseEvent e)
				{
					editor.transferFocus();
				}
			});
			
		editor.addMouseMotionListener(new MouseMotionListener()
			{
				public void mouseDragged(MouseEvent e)
				{
				}
				public void mouseMoved(MouseEvent e)
				{
					linkActivatedWhileFocused = true;
				}
			});
		
		editor.addKeyListener(new KeyAdapter()
			{
				@Override
				public void keyTyped(KeyEvent e)
				{
					redispatchToNextFocus(e);
					e.consume();
				}
				@Override
				public void keyPressed(KeyEvent e)
				{
					linkActivatedWhileFocused = true;
					if (e.getModifiers() == Toolkit.getDefaultToolkit().getMenuShortcutKeyMask())
					{
						if (e.getKeyCode() == KeyEvent.VK_V)
						{
							redispatchToNextFocus(e);
							e.consume();
						}
					}
				}
			});
		
	}
	
	protected void redispatchToNextFocus(AWTEvent e)
	{
		final Container focusRoot = getFocusCycleRootAncestor();
		final Component nextFocus = focusRoot.getFocusTraversalPolicy().getComponentAfter(focusRoot, (Component)e.getSource());
		nextFocus.requestFocus();
		KeyboardFocusManager.getCurrentKeyboardFocusManager().redispatchEvent(nextFocus, e);
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
		redLine = -1;
		editor.setText(wrapInXHTMLTags(""));
		lastIsAtEnd = true;
	}
	
	public void setRedLine()
	{
		if (redLine == -1)
		{
			redLine = getLineViewOffsets().length;
			repaint();
		}
	}
	
	public void clearRedLine()
	{
		if (redLine != -1)
		{
			redLine = -1;
			repaint();
		}
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
		System.err.println("Content that caused exception:\n" + new ByteBuffer(content).toHexString(true));
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
	
	public void showAndPulseArrow()
	{
		overlay.setArrowVisible(true);
		overlay.pulseArrow();
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
		appendTextLine(text, null, true);
	}
	
	public void appendTextLine(String text, String className)
	{
		boolean detectURLs = "debug".compareToIgnoreCase(className) != 0;
		appendTextLine(text, className, detectURLs);
	}
	
	public void appendTextLine(String text, String className, boolean detectURLs)
	{
		String html = TextModifierParser.parse(
			text, TextModifierParser.OutputFormat.XHTML, detectURLs);
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
