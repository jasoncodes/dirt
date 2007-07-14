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
		this.data = new byte[16];
		this.offset = 0;
		this.length = 0;
		this.shared = false;
	}
	
	public ByteBuffer(ByteBuffer src)
	{
		src.shared = true;
		this.data = src.data;
		this.offset = src.offset;
		this.length = src.length;
		this.shared = src.shared;
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
	
	public String toHexString()
	{
		StringBuffer buff = new StringBuffer(this.length*3);
		for (int i = 0; i < this.length; i++)
		{
			byte nibbleLo = (byte)( this.data[this.offset+i] & 0x0F );
			byte hibbleHi = (byte)( (this.data[this.offset+i]>>4) & 0x0F );
			buff.append(
				(i>0?" ":"") + 
				Integer.toHexString(hibbleHi) +
				Integer.toHexString(nibbleLo));
		}
		return buff.toString();
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
			shared = false;
		}
	}
	
	public ByteBuffer append(byte[] buff, int offset, int len)
	{
		int newLength = this.length + len;
		if (newLength > currentCapacity())
		{
			expandCapacity(newLength);
		}
		ensureOwnCopy();
		System.arraycopy(buff, offset, this.data, this.offset+this.length, len);
		this.length += len;
		return this;
	}
	
	public ByteBuffer append(ByteBuffer buff)
	{
		return append(buff.data, buff.offset, buff.length);
	}
	
	public ByteBuffer extract(int offset, int len)
	{
		return new ByteBuffer(this, offset, len);
	}
	
}