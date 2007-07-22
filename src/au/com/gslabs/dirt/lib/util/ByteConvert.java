package au.com.gslabs.dirt.lib.util;

public final class ByteConvert
{
	
	private ByteConvert()
	{
	}
	
	public static byte[] fromU16(int value)
	{
		if (value < 0)
		{
			new IllegalArgumentException("U16 must be positive");
		}
		byte[] data = new byte[2];
		data[0] = (byte)( (value >> 8) & 0xff );
		data[1] = (byte)( (value >> 0) & 0xff );
		return data;
	}
	
	public static int toU16(byte[] data)
	{
		if (data.length != 2)
		{
			throw new IllegalArgumentException("U16 must be 2 bytes");
		}
		int hi = data[0]&0xff;
		int lo = data[1]&0xff;
		return (hi<<8) + (lo<<0);
	}
	
}