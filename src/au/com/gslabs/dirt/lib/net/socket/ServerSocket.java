package au.com.gslabs.dirt.lib.net.socket;

import java.io.*;

public interface ServerSocket
{
	public void setReuseAddress(boolean on) throws IOException;
	public void bind(int port) throws IOException;
	public int getLocalPort();
	public Socket accept() throws IOException;
	public void close() throws IOException;
}
