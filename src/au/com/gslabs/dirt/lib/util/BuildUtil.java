package au.com.gslabs.dirt.lib.util;

/* This class is called during the build process by ant */

import java.io.*;
import java.util.*;
import org.w3c.dom.*;
import org.xml.sax.InputSource;
import au.com.gslabs.dirt.lib.xml.LocalDTD;
import javax.xml.parsers.*;
import javax.xml.transform.*;
import javax.xml.transform.stream.StreamResult;
import javax.xml.transform.dom.DOMSource;

public class BuildUtil
{
	
	public static void main(String[] args) throws Exception
	{
		if (args.length == 3 && args[0].equals("--append-source-date-to-version"))
		{
			
			Date sourceDate = getSourceDate(args[1]);
			String sourceDateString = TextUtil.formatDateTime(sourceDate, false, true);
			
			Properties strings = new Properties();
			strings.load(new FileInputStream(args[2]));
			strings.setProperty("sourceDate", sourceDateString);
			strings.store(new FileOutputStream(args[2]), null);
		}
		else if (args.length == 1 && args[0].equals("--print-arch"))
		{
			System.out.println(System.getProperty("os.arch"));
		}
		else if (args.length == 1 && args[0].equals("--print-javahome"))
		{
			System.out.println(System.getProperty("java.home"));
		}
		else if (args.length == 2 && args[0].equals("--fix-mac-bundle-plist"))
		{
			fixMacBundlePlist(new File(args[1]));
		}
		else
		{
			System.err.println("Invalid args");
			System.exit(1);
		}
	}
	
	public static class JavaSourceFilenameFilter implements FilenameFilter
	{
		public boolean accept(File dir, String name)
		{
			return name.endsWith(".java");
		}
	}
	
	public static Date getSourceDate(String basePath)
	{
		if (basePath.length() > 0 && basePath.charAt(basePath.length()-1) != File.separatorChar)
		{
			basePath += File.separatorChar;
		}
		FileIterator iterator = new FileIterator(basePath, new JavaSourceFilenameFilter());
		long latestModified = 0;
		String filename;
		while ((filename = iterator.next()) != null)
		{
			long thisModified = new File(basePath+filename).lastModified();
			latestModified = Math.max(latestModified, thisModified);
		}
		return new Date(latestModified);
	}
	
	public static Node plistDictLookup(final Node dict, final String key)
	{
		for (Node child = dict.getFirstChild(); child != null; child = child.getNextSibling())
		{
			if (child.getNodeName().equals("key"))
			{
				if (key.equals(child.getFirstChild().getNodeValue()))
				{
					return child.getNextSibling();
				}
			}
		}
		return null;
	}
	
	public static Node plistCreateStringArray(final Document document, final String[] values)
	{
		final Node array = document.createElement("array");
		for (String value : values)
		{
			final Node entry = document.createElement("string");
			entry.appendChild(document.createTextNode(value));
			array.appendChild(entry);
		}
		return array;
	}
	
	/**
	 * Unfortunately non-validating parsers don't always strip whitespace when asked
	 */
	public static void stripWhitespace(final Node node)
	{
		for (Node child = node.getFirstChild(); child != null; child = child.getNextSibling())
		{
			while (child instanceof Text && ((Text)child).isElementContentWhitespace())
			{
				final Node next = child.getNextSibling();
				child.getParentNode().removeChild(child);
				if (next == null) break;
				child = next;
			}
			stripWhitespace(child);
		}
	}
	
	public static void fixMacBundlePlist(File file) throws Exception
	{
		
		final Document document;
		
		{
			final DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
			factory.setValidating(true);
			factory.setIgnoringComments(true);
			factory.setIgnoringElementContentWhitespace(true);
			final DocumentBuilder builder = factory.newDocumentBuilder();
			builder.setEntityResolver(new LocalDTD());
			InputSource source = new InputSource();
			source.setCharacterStream(new InputStreamReader(new FileInputStream(file)));
			document = builder.parse(source);
			stripWhitespace(document.getDocumentElement());
		}
		
		final Node nodeJava = plistDictLookup(document.getDocumentElement().getFirstChild(), "Java");
		if (plistDictLookup(nodeJava, "JVMArchs") == null)
		{
			
			final Node keyJVMArchs = document.createElement("key");
			keyJVMArchs.appendChild(document.createTextNode("JVMArchs"));
			nodeJava.appendChild(keyJVMArchs);
			nodeJava.appendChild(plistCreateStringArray(document, new String[] { "x86_64", "x86", "ppc64", "ppc" } ));
			
		}
	
		if (true)
		{
			// <key>LSMinimumSystemVersionByArchitecture</key>
			// <dict>
			//     <key>x86_64</key>
			//     <string>10.6.0</string>
			// </dict>
			final Node keyMinSysVer = document.createElement("key");
			keyMinSysVer.appendChild(document.createTextNode("LSMinimumSystemVersionByArchitecture"));
			document.getDocumentElement().getFirstChild().appendChild(keyMinSysVer);
			final Node valMinSysVer = document.createElement("dict");
			document.getDocumentElement().getFirstChild().appendChild(valMinSysVer);
			final Node key_x86_64 = document.createElement("key");
			key_x86_64.appendChild(document.createTextNode("x86_64"));
			valMinSysVer.appendChild(key_x86_64);
			final Node val_x86_64 = document.createElement("string");
			val_x86_64.appendChild(document.createTextNode("10.6.0"));
			valMinSysVer.appendChild(val_x86_64);
		}
		
		{
			final Source source = new DOMSource(document);
			final Result result = new StreamResult(file);
			final Transformer transformer = TransformerFactory.newInstance().newTransformer();
			transformer.setOutputProperty(OutputKeys.DOCTYPE_SYSTEM, "http://www.apple.com/DTDs/PropertyList-1.0.dtd");
			transformer.setOutputProperty(OutputKeys.INDENT, "yes");
			transformer.transform(source, result);
		}
		
	}
	
}
