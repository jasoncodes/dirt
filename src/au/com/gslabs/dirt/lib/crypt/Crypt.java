package au.com.gslabs.dirt.lib.crypt;

import au.com.gslabs.dirt.lib.util.ByteBuffer;

public interface Crypt
{
	
	public KeyPair generateKeyPair(String transformation, int keySize) throws CryptException;
	public ByteBuffer encryptAsymmetric(String transformation, ByteBuffer publicKey, ByteBuffer data) throws CryptException;
	public ByteBuffer decryptAsymmetric(String transformation, ByteBuffer privateKey, ByteBuffer data) throws CryptException;
	
	public ByteBuffer generateRandomBytes(int count) throws CryptException;
	public ByteBuffer encryptBlockCipher(String transformation, ByteBuffer sharedKey, ByteBuffer data) throws CryptException;
	public ByteBuffer decryptBlockCipher(String transformation, ByteBuffer sharedKey, ByteBuffer data) throws CryptException;
	
	public ZlibDeflater createZlibDeflater(int level) throws CryptException;
	public ZlibInflator createZlibInflator() throws CryptException;
	
}
