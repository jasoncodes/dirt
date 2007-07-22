package au.com.gslabs.dirt.lib.crypt.j2se;

import au.com.gslabs.dirt.lib.crypt.*;
import au.com.gslabs.dirt.lib.util.ByteBuffer;
import java.security.KeyPairGenerator;
import java.security.KeyFactory;
import java.security.Key;
import javax.crypto.Cipher;
import java.security.spec.X509EncodedKeySpec;
import java.security.spec.PKCS8EncodedKeySpec;
import java.security.GeneralSecurityException;
import java.security.SecureRandom;
import javax.crypto.spec.SecretKeySpec;

public class J2SECrypt implements Crypt
{
	
	protected static String getCipherFromTransformation(String transformation)
	{
		int i = transformation.indexOf("/");
		if (i > -1)
		{
			return transformation.substring(0, i);
		}
		else
		{
			return transformation;
		}
	}
	
	public KeyPair generateKeyPair(String transformation, int keySize) throws CryptException
	{
		String cipher = getCipherFromTransformation(transformation);
		KeyPairGenerator gen;
		try
		{
			gen = KeyPairGenerator.getInstance(cipher);
		}
		catch (GeneralSecurityException ex)
		{
			throw new CryptException("Error generating key pair", ex);
		}
		gen.initialize(keySize);
		java.security.KeyPair pair = gen.genKeyPair();
		return new KeyPair(
				cipher,
				new ByteBuffer(pair.getPublic().getEncoded()),
				new ByteBuffer(pair.getPrivate().getEncoded())
			);
	}
	
	public static Key getKeyFromBuffer(String cipher, ByteBuffer data, boolean isPublic) throws CryptException
	{
		try
		{
			KeyFactory keyFac = KeyFactory.getInstance(cipher);
			if (isPublic)
			{
				X509EncodedKeySpec spec = new X509EncodedKeySpec(data.getBytes());
				return keyFac.generatePublic(spec);
			}
			else
			{
				PKCS8EncodedKeySpec spec = new PKCS8EncodedKeySpec(data.getBytes());
				return keyFac.generatePrivate(spec);
			}
		}
		catch (GeneralSecurityException ex)
		{
			throw new CryptException("Error loading key", ex);
		}
	}
	
	public ByteBuffer encryptAsymmetric(String transformation, ByteBuffer publicKey, ByteBuffer data) throws CryptException
	{
		try
		{
			String cipher = getCipherFromTransformation(transformation);
			Cipher c = Cipher.getInstance(transformation);
			c.init(Cipher.ENCRYPT_MODE, getKeyFromBuffer(cipher, publicKey, true));
			return new ByteBuffer(c.doFinal(data.getBytes()));
		}
		catch (GeneralSecurityException ex)
		{
			throw new CryptException("Error encrypting with public key", ex);
		}
	}
	
	public ByteBuffer decryptAsymmetric(String transformation, ByteBuffer privateKey, ByteBuffer data) throws CryptException
	{
		try
		{
			String cipher = getCipherFromTransformation(transformation);
			Cipher c = Cipher.getInstance(transformation);
			c.init(Cipher.DECRYPT_MODE, getKeyFromBuffer(cipher, privateKey, false));
			return new ByteBuffer(c.doFinal(data.getBytes()));
		}
		catch (GeneralSecurityException ex)
		{
			throw new CryptException("Error decrypting with private key", ex);
		}
	}
	
	protected SecureRandom sr = null;
	
	public ByteBuffer generateRandomBytes(int count) throws CryptException
	{
		try
		{
			if (sr == null)
			{
				sr = SecureRandom.getInstance("SHA1PRNG");
			}
			byte[] data = new byte[count];
			sr.nextBytes(data);
			return new ByteBuffer(data);
		}
		catch (GeneralSecurityException ex)
		{
			throw new CryptException("Error generating random number", ex);
		}
	}
	
	public ByteBuffer encryptBlockCipher(String transformation, ByteBuffer sharedKey, ByteBuffer data) throws CryptException
	{
		try
		{
			String cipher = getCipherFromTransformation(transformation);
			SecretKeySpec spec = new SecretKeySpec(sharedKey.getBytes(), cipher);
			Cipher c = Cipher.getInstance(transformation);
			c.init(Cipher.ENCRYPT_MODE, spec);
			return new ByteBuffer(c.doFinal(data.getBytes()));
		}
		catch (GeneralSecurityException ex)
		{
			throw new CryptException("Error encrypting with block cipher", ex);
		}
	}
	
	public ByteBuffer decryptBlockCipher(String transformation, ByteBuffer sharedKey, ByteBuffer data) throws CryptException
	{
		try
		{
			String cipher = getCipherFromTransformation(transformation);
			SecretKeySpec spec = new SecretKeySpec(sharedKey.getBytes(), cipher);
			Cipher c = Cipher.getInstance(transformation);
			c.init(Cipher.DECRYPT_MODE, spec);
			return new ByteBuffer(c.doFinal(data.getBytes()));
		}
		catch (GeneralSecurityException ex)
		{
			throw new CryptException("Error encrypting with block cipher", ex);
		}
	}
	
	public ZlibDeflater createZlibDeflater(int level) throws CryptException
	{
		return new J2SEZlibDeflater(level);
	}
	
	public ZlibInflator createZlibInflator() throws CryptException
	{
		return new J2SEZlibInflator();
	}
	
}

