package au.com.gslabs.dirt.lib.util;

import java.io.UnsupportedEncodingException;
import java.util.ArrayList;

public class ByteBuffer
{
	
	private byte[] data;
	private int offset;
	private int length;
	private boolean shared;
	
	public ByteBuffer()
	{
		clear();
	}
	
	public ByteBuffer(int initialCapacity)
	{
		clear(initialCapacity);
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
		
		if (len == 0)
		{
			// no point in sharing a buffer if we aren't using anything in it
			clear();
		}
		else
		{
			src.shared = true;
			this.data = src.data;
			this.offset = src.offset + offset;
			this.length = len;
			this.shared = src.shared;
		}
		
	}
	
	public ByteBuffer(String text)
	{
		try
		{
			this.data = (text != null) ? text.getBytes("UTF-8") : new byte[0];
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
		clear(16);
	}
	
	public boolean equals(Object obj)
	{
		
		if (this == obj)
		{
			return true;
		}
		
		if (!(obj instanceof ByteBuffer))
		{
			return false;
		}
		
		ByteBuffer other = (ByteBuffer)obj;
		
		if (this.data == other.data &&
			this.offset == other.offset &&
			this.length == other.length)
		{
			return true;
		}
		
		if (this.length != other.length)
		{
			return false;
		}
		
		for (int i = 0; i < this.length; ++i)
		{
			if (this.data[i+this.offset] != other.data[i+other.offset])
			{
				return false;
			}
		}
		return true;
		
	}
	
	public void clear(int initialCapacity)
	{
		this.data = new byte[initialCapacity];
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
	
	private static String makeHexString(byte val, int width)
	{
		return makeHexString(val&0xff, width);
	}
	
	private static String makeHexString(int val, int width)
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
			
			if (this.length == 0)
			{
				buff.append(makeHexString(0, posWidth)+ " ");
			}
			
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
	
	private void ensureOwnCopy()
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
	
	public void append(byte value)
	{
		byte[] buff = new byte[1];
		buff[0] = value;
		append(buff);
	}
	
	public void append(String value)
	{
		append(new ByteBuffer(value));
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
	
	public ByteBuffer extract(int offset)
	{
		return new ByteBuffer(this, offset, this.length - offset);
	}
	
	public void set(int offset, byte value)
	{
		ensureOwnCopy();
		if (offset < 0 || offset >= length)
		{
			throw new IndexOutOfBoundsException();
		}
		this.data[this.offset+offset] = value;
	}
	
	public byte get(int offset)
	{
		if (offset < 0 || offset >= length)
		{
			throw new IndexOutOfBoundsException();
		}
		return this.data[this.offset+offset];
	}
	
	public int indexOf(byte value)
	{
		return indexOf(value, 0);
	}
	
	public int indexOf(byte value, int startPos)
	{
		for (int i = startPos; i < this.length; ++i)
		{
			if (this.data[this.offset+i] == value)
			{
				return i;
			}
		}
		return -1;
	}
	
}
