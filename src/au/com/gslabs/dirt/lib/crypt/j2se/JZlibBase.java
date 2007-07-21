package au.com.gslabs.dirt.lib.crypt.j2se;

import au.com.gslabs.dirt.lib.util.ByteBuffer;
import com.jcraft.jzlib.*;
import au.com.gslabs.dirt.lib.crypt.*;

public class JZlibBase
{
	
	protected ZStream stream;
	protected byte[] bufferOutput = null;
	protected final int bufferSize = 4096;
	
	public JZlibBase()
	{
		stream = new ZStream();
	}
	
	protected void checkReturnCode(String method, int returnCode) throws CryptException
	{
		if (returnCode != JZlib.Z_OK)
		{
			String msg = "Zlib error";
			switch (returnCode)
			{
				case JZlib.Z_OK:
					msg += " Z_OK";
					break;
				case JZlib.Z_STREAM_END:
					msg += " Z_STREAM_END";
					break;
				case JZlib.Z_NEED_DICT:
					msg += " Z_NEED_DICT";
					break;
				case JZlib.Z_ERRNO:
					msg += " Z_ERRNO";
					break;
				case JZlib.Z_STREAM_ERROR:
					msg += " Z_STREAM_ERROR";
					break;
				case JZlib.Z_DATA_ERROR:
					msg += " Z_DATA_ERROR";
					break;
				case JZlib.Z_MEM_ERROR:
					msg += " Z_MEM_ERROR";
					break;
				case JZlib.Z_BUF_ERROR:
					msg += " Z_BUF_ERROR";
					break;
				case JZlib.Z_VERSION_ERROR:
					msg += " Z_VERSION_ERROR";
					break;
			}
			msg += " during "+ method;
			if (stream.msg != null)
			{
				msg += ": " + stream.msg;
			}
			throw new CryptException(msg, null);
		}
	}
	
	protected void setInput(ByteBuffer data) throws CryptException
	{
		if (haveInput())
		{
			throw new CryptException("Still have input in buffer", null);
		}
		stream.next_in = data.getBytes();
		stream.next_in_index = 0;
		stream.avail_in = data.length();
	}
	
	protected boolean haveInput()
	{
		return stream.avail_in > 0;
	}
	
	protected void prepareOutputBuffer()
	{
		if (bufferOutput == null)
		{
			bufferOutput = new byte[bufferSize];
		}
		stream.next_out = bufferOutput;
		stream.next_out_index = 0;
		stream.avail_out = bufferOutput.length;
	}
	
	protected void appendToOutput(ByteBuffer output)
	{
		output.append(bufferOutput, 0, stream.next_out_index);
		stream.next_out_index = -1;
	}
	
	protected boolean outputBufferWasFull()
	{
		return stream.avail_out == 0;
	}
	
}
