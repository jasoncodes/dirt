package au.com.gslabs.dirt.lib.xml;

import java.io.InputStream;
import org.xml.sax.*;
import java.util.Hashtable;
import au.com.gslabs.dirt.lib.util.FileUtil;
import java.net.URL;

public class LocalDTD implements EntityResolver
{
	
	private Hashtable<String,String> entities = new Hashtable<String,String>();
	
	// fill the list of URLs
	public LocalDTD()
	{
		
		// The XHTML 1.0 DTDs
		this.addMapping("-//W3C//DTD XHTML 1.0 Strict//EN",
			"res/xml/xhtml/dtd/xhtml1-strict.dtd");
		this.addMapping("-//W3C//DTD XHTML 1.0 Transitional//EN",
			"res/xml/xhtml/dtd/xhtml1-transitional.dtd");
		this.addMapping("-//W3C//DTD XHTML 1.0 Frameset//EN",
			"res/xml/xhtml/dtd/xhtml1-frameset.dtd");
		
		// The XHTML 1.0 entity sets
		this.addMapping("-//W3C//ENTITIES Latin 1 for XHTML//EN",
			"res/xml/xhtml/dtd/xhtml-lat1.ent");
		this.addMapping("-//W3C//ENTITIES Symbols for XHTML//EN",
			"res/xml/xhtml/dtd/xhtml-symbol.ent");
		this.addMapping("-//W3C//ENTITIES Special for XHTML//EN",
			"res/xml/xhtml/dtd/xhtml-special.ent");
		
		// The XHTML 1.1 DTD
		this.addMapping("-//W3C//DTD XHTML 1.1//EN",
			"res/xml/xhtml/dtd/xhtml11-flat.dtd");
		
	}

	private void addMapping(String publicID, String res)
	{
		entities.put(publicID, res);
	}
	
	public InputSource resolveEntity(String publicID, String systemID) throws SAXException
	{
		
		String res = entities.get(publicID);
		if (res != null)
		{
			InputStream stream = FileUtil.getResourceAsStream(res);
			InputSource local = new InputSource(stream);
			return local;
		}
		
		throw new SAXException("Unable to resolve \"" + publicID + "\", \"" + systemID + "\"");
		
	}
	
}