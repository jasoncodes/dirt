package au.com.gslabs.dirt.lib.util;

import java.io.UnsupportedEncodingException;

public class ByteBuffer
{
	
	protected byte[] data;
	
	public ByteBuffer()
	{
		data = new byte[0];
	}
	
	public ByteBuffer(String text)
	{
		try
		{
			data = text.getBytes("UTF-8");
		}
		catch (UnsupportedEncodingException ex)
		{
			throw new RuntimeException("UTF-8 Conversion Error", ex);
		}
	}
	
	public String toString()
	{
		try
		{
			return new String(data, "UTF-8");
		}
		catch (UnsupportedEncodingException ex)
		{
			throw new RuntimeException("UTF-8 Conversion Error", ex);
		}
	}
	
}