package au.com.gslabs.dirt.lib.crypt;

import java.io.IOException;

public class CryptException extends IOException
{
	
	public CryptException(String message, Throwable cause)
	{
		super(message);
		initCause(cause);
	}
	
}
