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
			local.setPublicId(publicID);
			local.setSystemId(systemID);
			return local;
		}
		
		throw new SAXException("Unable to resolve \"" + publicID + "\", \"" + systemID + "\"");
		
	}
	
}