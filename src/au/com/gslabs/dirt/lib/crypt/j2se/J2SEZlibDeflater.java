package au.com.gslabs.dirt.lib.crypt.j2se;

import au.com.gslabs.dirt.lib.util.ByteBuffer;
import com.jcraft.jzlib.*;
import au.com.gslabs.dirt.lib.crypt.*;

public class J2SEZlibDeflater extends JZlibBase implements ZlibDeflater
{
	
	public J2SEZlibDeflater(int level) throws CryptException
	{
		checkReturnCode("deflateInit", stream.deflateInit(level));
	}
	
	public ByteBuffer deflate(ByteBuffer data, boolean flush) throws CryptException
	{
		
		ByteBuffer output = new ByteBuffer();
		
		int mode = flush ? JZlib.Z_SYNC_FLUSH : JZlib.Z_NO_FLUSH;
		
		setInput(data);
		while (haveInput())
		{
			prepareOutputBuffer();
			checkReturnCode("deflate", stream.deflate(mode));
			appendToOutput(output);
		}
		
		while (outputBufferWasFull())
		{
			prepareOutputBuffer();
			checkReturnCode("deflate", stream.deflate(mode));
			appendToOutput(output);
		}
		
		return output;
		
	}
	
}
