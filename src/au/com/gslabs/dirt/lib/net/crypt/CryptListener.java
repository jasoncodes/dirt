package au.com.gslabs.dirt.lib.net.crypt;

import java.io.IOException;
import au.com.gslabs.dirt.lib.util.ByteBuffer;
import au.com.gslabs.dirt.lib.crypt.CryptException;

public interface CryptListener
{
	public void cryptError(IOException ex);
	public void cryptConnected();
	public void cryptClosed();
	public void cryptMessage(ByteBuffer data);
}
