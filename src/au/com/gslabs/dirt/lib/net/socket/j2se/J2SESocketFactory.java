package au.com.gslabs.dirt.lib.net.socket.j2se;

import au.com.gslabs.dirt.lib.net.socket.*;
import au.com.gslabs.dirt.lib.util.EnumerationIterator;
import java.util.ArrayList;
import java.util.SortedSet;
import java.util.Comparator;
import java.net.InetAddress;
import java.net.NetworkInterface;
import java.net.UnknownHostException;

public class J2SESocketFactory extends SocketFactory
{
	
	public J2SESocketFactory()
	{
		
		// prefer AAAA records when available
		try
		{
			System.setProperty("java.net.preferIPv6Addresses", "true");
		}
		catch (SecurityException ex)
		{
		}
		
	}
	
	public Socket createSocket()
	{
		return new J2SESocket();
	}
	
	public ServerSocket createServerSocket()
	{
		return new J2SEServerSocket();
	}
	
	public String getHostName()
	{
		try
		{
			return java.net.InetAddress.getLocalHost().getHostName();
		}
		catch (java.net.UnknownHostException ex)
		{
			return null;
		}
	}
	
	protected class InetAddressComparator implements Comparator<InetAddress>
	{
		
		private ArrayList<Comparable<? extends Comparable>> getComparableProperties(InetAddress addr)
		{
			ArrayList<Comparable<? extends Comparable>> props = new ArrayList<Comparable<? extends Comparable>>();
			props.add(addr.isLinkLocalAddress()); // prefer non-link-local, then..
			props.add(addr.isSiteLocalAddress()); // prefer non-site-local, then..
			props.add(!(addr instanceof java.net.Inet6Address)); // prefer IPv6
			return props;
		}
		
		@SuppressWarnings("unchecked")
		private int doCompare(Comparable<? extends Comparable> c1, Comparable<? extends Comparable> c2)
		{
			return ((Comparable)c1).compareTo(c2);
		}
		
		public int compare(InetAddress addr1, InetAddress addr2)
		{
			
			final ArrayList<Comparable<? extends Comparable>> props1 = getComparableProperties(addr1);
			final ArrayList<Comparable<? extends Comparable>> props2 = getComparableProperties(addr2);
			
			for (int i = 0; i < props1.size(); ++i)
			{
				final int cmp = doCompare(props1.get(i), props2.get(i));
				if (cmp != 0)
				{
					return cmp;
				}
			}
			
			return addr1.getHostAddress().compareTo(addr2.getHostAddress());
			
		}
		
	}
	
	protected static boolean isUsableAddress(final InetAddress addr)
	{
		return !addr.isLoopbackAddress() && !addr.isAnyLocalAddress() && !addr.isMulticastAddress();
	}
	
	public String[] getHostAddresses()
	{
		try
		{
			
			final SortedSet<InetAddress> addresses = new java.util.TreeSet<InetAddress>(new InetAddressComparator());
			for (final NetworkInterface iface : new EnumerationIterator<NetworkInterface>(NetworkInterface.getNetworkInterfaces()))
			{
				for (final InetAddress addr : new EnumerationIterator<InetAddress>(iface.getInetAddresses()))
				{
					if (isUsableAddress(addr))
					{
						addresses.add(addr);
					}
				}
			}
			
			String[] strs = new String[addresses.size()];
			int idxStr = 0;
			for (InetAddress addr : addresses)
			{
				strs[idxStr++] = addr.getHostAddress();
			}
			return strs;
			
		}
		catch (java.net.SocketException ex)
		{
			return null;
		}
	}
	
}
