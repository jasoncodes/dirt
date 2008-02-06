package au.com.gslabs.dirt.lib.util;

import java.util.*;

public class EnumerationIterator<T> implements Iterator<T>, Iterable<T>
{
	
	private final Enumeration<T> theEnum;
	
	public EnumerationIterator(Enumeration<T> theEnum)
	{
		if (theEnum == null)
		{
			throw new NullPointerException();
		}
		this.theEnum = theEnum;
	}
	
	public Iterator<T> iterator()
	{
		return this;
	}
	
	public boolean hasNext()
	{
		return theEnum.hasMoreElements();
	}
	
	public T next()
	{
		return theEnum.nextElement();
	}
	
	public void remove() throws UnsupportedOperationException
	{
		throw new UnsupportedOperationException();
	}
	
}
