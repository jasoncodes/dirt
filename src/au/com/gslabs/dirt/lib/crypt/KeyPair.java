package au.com.gslabs.dirt.lib.crypt;

import au.com.gslabs.dirt.lib.util.ByteBuffer;

public final class KeyPair
{
	
	protected String cipher;
	protected final ByteBuffer publicKey;
	protected final ByteBuffer privateKey;
	
	public KeyPair(String cipher, ByteBuffer publicKey, ByteBuffer privateKey)
	{
		this.publicKey = publicKey;
		this.privateKey = privateKey;
	}
	
	public ByteBuffer getPublicKey()
	{
		return this.publicKey;
	}
	
	public ByteBuffer getPrivateKey()
	{
		return this.privateKey;
	}
	
}
