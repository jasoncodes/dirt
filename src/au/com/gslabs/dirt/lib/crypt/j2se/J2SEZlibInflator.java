package au.com.gslabs.dirt.lib.crypt.j2se;

import au.com.gslabs.dirt.lib.util.ByteBuffer;
import com.jcraft.jzlib.*;
import au.com.gslabs.dirt.lib.crypt.*;

public class J2SEZlibInflator extends JZlibBase implements ZlibInflator
{
	
	public J2SEZlibInflator() throws CryptException
	{
		checkReturnCode("inflateInit", stream.inflateInit());
	}
	
	public ByteBuffer inflate(ByteBuffer data) throws CryptException
	{
		
		ByteBuffer output = new ByteBuffer();
		
		setInput(data);
		while (haveInput())
		{
			prepareOutputBuffer();
			checkReturnCode("inflate", stream.inflate(JZlib.Z_NO_FLUSH));
			appendToOutput(output);
		}
		
		while (outputBufferWasFull())
		{
			prepareOutputBuffer();
			int retVal = stream.inflate(JZlib.Z_NO_FLUSH);
			appendToOutput(output);
			if (retVal == JZlib.Z_STREAM_END)
			{
				break;
			}
			if (retVal == JZlib.Z_BUF_ERROR && !outputBufferWasFull())
			{
				break;
			}
			checkReturnCode("inflate", retVal);
		}
		
		return output;
		
	}
	
}
