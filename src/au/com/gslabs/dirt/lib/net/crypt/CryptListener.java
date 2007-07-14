package au.com.gslabs.dirt.lib.net.crypt;

public interface CryptListener
{
	public void cryptError(java.io.IOException ex);
	public void cryptConnected();
}
