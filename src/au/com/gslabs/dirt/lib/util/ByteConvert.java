package au.com.gslabs.dirt.lib.util;

import java.util.Map;
import java.util.HashMap;
import java.util.ArrayList;

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
	
	public static ArrayList<ByteBuffer> tokenizeNull(ByteBuffer data)
	{
		return tokenizeNull(data, -1);
	}
	
	public static ArrayList<ByteBuffer> tokenizeNull(ByteBuffer data, int maxTokens)
	{
		
		ArrayList<ByteBuffer> tokens = new ArrayList<ByteBuffer>();
		
		int startPos = 0;
		int tokensLeft = (maxTokens > 0) ? (maxTokens-1) : Integer.MAX_VALUE;
		
		while (tokensLeft-- > 0 && startPos < data.length())
		{
			int endPos = data.indexOf((byte)0x00, startPos);
			if (endPos >= 0)
			{
				tokens.add(data.extract(startPos, endPos-startPos));
				startPos = endPos + 1;
			}
			else
			{
				break;
			}
		}
		
		if (startPos < data.length())
		{
			tokens.add(data.extract(startPos));
		}
		else if (startPos == data.length())
		{
			tokens.add(new ByteBuffer());
		}
		
		return tokens;
		
	}
	
	public static Map<String,String> toMap(ByteBuffer data)
	{
		
		ArrayList<ByteBuffer> tokens = tokenizeNull(data);
		if ((tokens.size()%2) != 0)
		{
			throw new IllegalArgumentException("Map must have multiple of 2 tokens");
		}
		
		Map<String,String> map = new HashMap<String,String>(tokens.size()/2);
		
		for (int i = 0; i < tokens.size(); i += 2)
		{
			map.put(tokens.get(i).toString(), tokens.get(i+1).toString());
		}
		
		return map;
		
	}
	
}