package au.com.gslabs.dirt.lib.crypt;

import au.com.gslabs.dirt.lib.util.ByteBuffer;

public interface Crypt
{
	
	public KeyPair generateKeyPair(String cipher, int keySize) throws CryptException;
	public ByteBuffer encrypt(String cipher, ByteBuffer publicKey, ByteBuffer data) throws CryptException;
	public ByteBuffer decrypt(String cipher, ByteBuffer privateKey, ByteBuffer data) throws CryptException;
	
}
