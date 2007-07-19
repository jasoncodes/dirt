package au.com.gslabs.dirt;

import au.com.gslabs.dirt.lib.util.FileUtil;
import au.com.gslabs.dirt.ui.UI;

import au.com.gslabs.dirt.lib.util.ByteBuffer;
import au.com.gslabs.dirt.lib.crypt.*;

public class Dirt
{
	
	public static final String VERSION = "3.1.0 Alpha 0";
	
	public static void main(String[] args)
	{
		//FileUtil.ShutdownCleanupCheck(args);
		//UI.startClient(args);
		try
		{
			Crypt crypt = CryptFactory.getInstance();
			KeyPair pair = crypt.generateKeyPair("RSA", 1024);
			ByteBuffer plain = new ByteBuffer("This is a test.");
			ByteBuffer encrypted = crypt.encrypt("RSA", pair.getPublicKey(), plain);
			ByteBuffer decrypted = crypt.decrypt("RSA", pair.getPrivateKey(), encrypted);
			System.out.println(decrypted);
		}
		catch (CryptException ex)
		{
			ex.printStackTrace();
		}
	}
	
}
