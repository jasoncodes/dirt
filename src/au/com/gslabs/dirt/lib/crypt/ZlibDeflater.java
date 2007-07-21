package au.com.gslabs.dirt.lib.crypt;

import au.com.gslabs.dirt.lib.util.ByteBuffer;

public interface ZlibDeflater
{
	public ByteBuffer deflate(ByteBuffer data, boolean flush) throws CryptException;
}
