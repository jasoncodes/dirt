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
import au.com.gslabs.dirt.lib.crypt.generic.AES;
import au.com.gslabs.dirt.lib.crypt.generic.BlockCipher;

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
		return doBlockCipher(transformation, sharedKey, data, true);
	}
	
	public ByteBuffer decryptBlockCipher(String transformation, ByteBuffer sharedKey, ByteBuffer data) throws CryptException
	{
		return doBlockCipher(transformation, sharedKey, data, false);
	}
	
	protected static ByteBuffer doBlockCipher_J2SE(String transformation, ByteBuffer sharedKey, ByteBuffer data, boolean forEncryption) throws GeneralSecurityException
	{
		String cipher = getCipherFromTransformation(transformation);
		SecretKeySpec spec = new SecretKeySpec(sharedKey.getBytes(), cipher);
		Cipher c = Cipher.getInstance(transformation);
		c.init(forEncryption?Cipher.ENCRYPT_MODE:Cipher.DECRYPT_MODE, spec);
		return new ByteBuffer(c.doFinal(data.getBytes()));
	}
	
	protected static final String transform_AES = "AES/ECB/NoPadding";
	
	protected static ByteBuffer doBlockCipher_AES(String transformation, ByteBuffer sharedKey, ByteBuffer data, boolean forEncryption) throws CryptException
	{
		if (!transformation.equals(transform_AES))
		{
			throw new CryptException("Unsupported transformation: " + transformation, null);
		}
		BlockCipher aes = new AES();
		aes.init(forEncryption, sharedKey.getBytes());
		byte[] buff = new byte[data.length()];
		int offset = 0;
		while (offset < buff.length)
		{
			aes.transformBlock(data.getBytes(), offset, buff, offset);
			offset += aes.getBlockSize();
		}
		return new ByteBuffer(buff);
	}
	
	protected static boolean cipher_BadSystemAES = false;
	
	protected ByteBuffer doBlockCipher(String transformation, ByteBuffer sharedKey, ByteBuffer data, boolean forEncryption) throws CryptException
	{
		
		if (cipher_BadSystemAES)
		{
			return doBlockCipher_AES(transformation, sharedKey, data, forEncryption);
		}
		
		try
		{
			return doBlockCipher_J2SE(transformation, sharedKey, data, forEncryption);
		}
		catch (GeneralSecurityException ex)
		{
			if (transformation.equals(transform_AES))
			{
				cipher_BadSystemAES = true;
				return doBlockCipher_AES(transformation, sharedKey, data, forEncryption);
			}
			else
			{
				throw new CryptException("Error encrypting with block cipher", ex);
			}
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

