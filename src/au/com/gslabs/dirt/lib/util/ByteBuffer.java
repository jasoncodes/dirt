package au.com.gslabs.dirt.lib.util;

import java.io.UnsupportedEncodingException;

public class ByteBuffer
{
	
	protected byte[] data;
	protected int offset;
	protected int length;
	protected boolean shared;
	
	public ByteBuffer()
	{
		clear();
	}
	
	public ByteBuffer(ByteBuffer src)
	{
		src.shared = true;
		this.data = src.data;
		this.offset = src.offset;
		this.length = src.length;
		this.shared = src.shared;
	}
	
	public ByteBuffer(byte[] bytes)
	{
		clear();
		append(bytes, 0, bytes.length);
	}
	
	public ByteBuffer(ByteBuffer src, int offset, int len)
	{
		if (offset < 0 || len < 0 || src.length < offset + len)
		{
			throw new IndexOutOfBoundsException();
		}
		src.shared = true;
		this.data = src.data;
		this.offset = src.offset + offset;
		this.length = len;
		this.shared = src.shared;
		
	}
	
	public ByteBuffer(String text)
	{
		try
		{
			this.data = text.getBytes("UTF-8");
			this.offset = 0;
			this.length = data.length;
			this.shared = false;
		}
		catch (UnsupportedEncodingException ex)
		{
			throw new RuntimeException("UTF-8 Conversion Error", ex);
		}
	}
	
	public void clear()
	{
		this.data = new byte[16];
		this.offset = 0;
		this.length = 0;
		this.shared = false;
	}
	
	public String toString()
	{
		try
		{
			return new String(this.data, this.offset, this.length, "UTF-8");
		}
		catch (UnsupportedEncodingException ex)
		{
			throw new RuntimeException("UTF-8 Conversion Error", ex);
		}
	}
	
	public byte[] getBytes()
	{
		byte bytes[] = new byte[this.length];
		System.arraycopy(this.data, this.offset, bytes, 0, this.length);
		return bytes;
	}
	
	protected static String makeHexString(byte val, int width)
	{
		return makeHexString(val&0xff, width);
	}
	
	protected static String makeHexString(int val, int width)
	{
		
		String str = Integer.toHexString(val);
		
		StringBuffer buff = new StringBuffer(width);
		for (int i = 0; i < (width - str.length()); ++i)
		{
			buff.append('0');
		}
		buff.append(str);
		
		return buff.toString();
		
	}
	
	public String toHexString(boolean verbose)
	{
		if (verbose)
		{
			
			int posWidth = (int)Math.ceil(Math.log(this.length) / Math.log(16));
			if (posWidth < 4)
			{
				posWidth = 4;
			}
			else if (posWidth < 8)
			{
				posWidth = 8;
			}
			
			StringBuffer buff = new StringBuffer(this.length*5);
			for (int i = 0; i < this.length; i += 16)
			{
				if (i > 0)
				{
					buff.append("\n");
				}
				buff.append(makeHexString(i, posWidth)+ " ");
				int lineEnd = Math.min(this.length, i+16);
				for (int j = i; j < lineEnd; ++j)
				{
					if (j == i + 8)
					{
						buff.append(" ");
					}
					buff.append(" " + makeHexString(this.data[this.offset+j], 2));
				}
				if ((lineEnd % 16) > 0)
				{
					int buffNeeded = (16 - (lineEnd%16));
					for (int j = 0; j < buffNeeded; ++j)
					{
						buff.append("   ");
					}
					if (buffNeeded >= 8)
					{
						buff.append(" ");
					}
				}
				buff.append("  ");
				buff.append("|");
				for (int j = i; j < lineEnd; ++j)
				{
					byte b = this.data[this.offset+j];
					if (b >= 0x20 && b < 0x7f)
					{
						buff.append((char)b);
					}
					else
					{
						buff.append('.');
					}
				}
				buff.append("|");
			}
			
			return buff.toString();
		
		}
		else
		{
			StringBuffer buff = new StringBuffer(this.length*3);
			for (int i = 0; i < this.length; i++)
			{
				buff.append((i>0?" ":"") + makeHexString(this.data[this.offset+i], 2));
			}
			return buff.toString();
		}
	}
	
	public String toHexString()
	{
		return toHexString(false);
	}
	
	public int length()
	{
		return this.length;
	}
	
	public int currentCapacity()
	{
		return this.data.length - this.offset;
	}
	
	public void ensureCapacity(int minimumCapacity)
	{
		if (minimumCapacity > currentCapacity())
		{
			expandCapacity(minimumCapacity);
		}
	}
	
	private void expandCapacity(int minimumCapacity)
	{
		int newCapacity = (currentCapacity() + 1) * 2;
		if (minimumCapacity > newCapacity)
		{
			newCapacity = minimumCapacity;
		}
		byte newData[] = new byte[newCapacity];
		System.arraycopy(this.data, this.offset, newData, 0, this.length);
		this.offset = 0;
		this.data = newData;
		this.shared = false;
	}
	
	protected void ensureOwnCopy()
	{
		if (this.shared)
		{
			byte newData[] = new byte[currentCapacity()];
			System.arraycopy(this.data, this.offset, newData, 0, this.length);
			this.offset = 0;
			this.data = newData;
			this.shared = false;
		}
	}
	
	public void append(byte[] buff)
	{
		append(buff, 0, buff.length);
	}
	
	public void append(byte[] buff, int offset, int len)
	{
		int newLength = this.length + len;
		if (newLength > currentCapacity())
		{
			expandCapacity(newLength);
		}
		ensureOwnCopy();
		System.arraycopy(buff, offset, this.data, this.offset+this.length, len);
		this.length += len;
	}
	
	public void append(ByteBuffer buff)
	{
		append(buff.data, buff.offset, buff.length);
	}
	
	public ByteBuffer extract(int offset, int len)
	{
		return new ByteBuffer(this, offset, len);
	}
	
}
