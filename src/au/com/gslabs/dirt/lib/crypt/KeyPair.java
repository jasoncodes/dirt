package au.com.gslabs.dirt.lib.crypt;

import au.com.gslabs.dirt.lib.util.ByteBuffer;

public final class KeyPair
{
	
	private String cipher;
	private final ByteBuffer publicKey;
	private final ByteBuffer privateKey;
	
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
