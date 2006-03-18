package au.com.gslabs.dirt.lib.ui.jfc;

import au.com.gslabs.dirt.lib.xml.LocalDTD;
import java.io.*; 
import javax.swing.text.*;
import javax.swing.text.html.*;
import javax.xml.parsers.*;
import org.xml.sax.*;
import org.xml.sax.helpers.DefaultHandler;
 
public class XHTMLEditorKit extends HTMLEditorKit
{
	
	protected XHTMLParser parser = null;
	
	protected Parser getParser()
	{
		if (parser == null)
		{
			parser = new XHTMLParser();
		}
		return parser;
	}
	
	private class XHTMLParser extends Parser
	{
		
		protected SAXParser parser = null;
		
		public void parse(Reader reader, ParserCallback callback, boolean ignoreCharSet) throws IOException
		{
			
			try
			{
				
				if (parser == null)
				{
					SAXParserFactory factory = SAXParserFactory.newInstance();
					factory.setValidating(true);
					parser = factory.newSAXParser();
				}
			
				XHTMLSaxHandler handler = new XHTMLSaxHandler(callback);
				InputSource source = new InputSource(reader);
				parser.parse(source, handler);
				
			}
			catch (Exception inner)
			{
				IOException ex = new IOException("Error parsing XHTML");
				ex.initCause(inner);
				throw ex;
			}
			
		}
		
	}
 
	private class XHTMLSaxHandler extends DefaultHandler
	{
		
		private final ParserCallback callback;
		
		public XHTMLSaxHandler(ParserCallback callback)
		{
			this.callback = callback;
		}
		
		public void endElement(String uri, String name, String qName) throws SAXException
		{
			callback.handleEndTag(HTML.getTag(qName), -1);
		}
		
		public void startElement(String uri, String name, String qName, Attributes atts) throws SAXException
		{
			SimpleAttributeSet attributeSet = convertAttributes(atts);
			callback.handleStartTag(HTML.getTag(qName), attributeSet, -1);
		}
		
		private SimpleAttributeSet convertAttributes(Attributes atts)
		{
			SimpleAttributeSet attributeSet = new SimpleAttributeSet();
			
			for (int i = 0; i < atts.getLength(); ++i)
			{
				HTML.Attribute attribute = HTML.getAttributeKey(atts.getQName(i));
				if (attribute != null)
				{
					attributeSet.addAttribute(attribute, atts.getValue(i));
				}
			}
			return attributeSet;
		}
		
		public void characters(char[] ch, int start, int length) throws SAXException
		{
			char[] tmp = new char[length];
			System.arraycopy(ch, start, tmp, 0, length);
			callback.handleText(tmp, -1);
		}
		
		public void error(SAXParseException e) throws SAXException
		{
			callback.handleError(e.getMessage(), -1);
		}
		
		public void fatalError(SAXParseException e) throws SAXException
		{
			callback.handleError(e.getMessage(), -1);
		}
		
		protected EntityResolver resolver = null;
		
		public InputSource resolveEntity(String publicId, String systemId) throws SAXException, IOException
		{
			if (resolver == null)
			{
				resolver = new LocalDTD();
			}
			return resolver.resolveEntity(publicId, systemId);
		}
		
	}
}
