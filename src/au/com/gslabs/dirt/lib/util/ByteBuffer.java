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
	
	public String toHexString()
	{
		StringBuffer buff = new StringBuffer(data.length*3);
		for (int i = 0; i < data.length; i++)
		{
			byte nibbleLo = (byte)( data[i] & 0x0F );
			byte hibbleHi = (byte)( (data[i]>>4) & 0x0F );
			buff.append(
				(i>0?" ":"") + 
				Integer.toHexString(hibbleHi) +
				Integer.toHexString(nibbleLo));
		}
		return buff.toString();
	}
	
	public int length()
	{
		return data.length;
	}
	
}