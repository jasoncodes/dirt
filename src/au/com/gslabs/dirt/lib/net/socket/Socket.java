package au.com.gslabs.dirt.lib.net.socket;

import java.io.*;

public interface Socket
{
	public void connect(String host, int port) throws IOException;
	public void close() throws IOException;
	public InputStream getInputStream() throws IOException;
	public OutputStream getOutputStream() throws IOException;
	public String getPeerName();
}
