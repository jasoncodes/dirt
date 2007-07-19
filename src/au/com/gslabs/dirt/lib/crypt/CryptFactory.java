package au.com.gslabs.dirt.lib.crypt;

public class CryptFactory
{
	public static Crypt getInstance()
	{
		return new au.com.gslabs.dirt.lib.crypt.j2se.J2SECrypt();
	}
}
