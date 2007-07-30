package au.com.gslabs.dirt.lib.util.jni;

import java.io.UnsupportedEncodingException;

public class Linux
{

	public Linux()
	{
	}
	
	protected native int getuid();
	protected native byte[] getPwUid_Gecos(int uid);
	protected native byte[] getPwUid_Name(int uid);
	
	public String getMyFullName() throws UnsupportedEncodingException
	{
		
		int uid = getuid();
		String gecos = new String(getPwUid_Gecos(uid), "ISO-8859-1");
		String name = new String(getPwUid_Name(uid), "ISO-8859-1");
		
		if (gecos.length() < 1)
		{
			gecos = "&";
		}
		
		int i = gecos.indexOf(',');
		if (i > 0)
		{
			gecos = gecos.substring(0, i);
		}
		
		return gecos.replace("&", name);
		
	}
	
}
