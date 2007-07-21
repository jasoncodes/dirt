package au.com.gslabs.dirt.lib.crypt;

import au.com.gslabs.dirt.lib.util.ByteBuffer;

public interface ZlibInflator
{
	public ByteBuffer inflate(ByteBuffer data) throws CryptException;
}
