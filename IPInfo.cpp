/*
    Copyright 2002, 2003 General Software Laboratories
    
    
    This file is part of Dirt Secure Chat.

    Dirt Secure Chat is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Dirt Secure Chat is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Dirt Secure Chat; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: IPInfo.cpp,v 1.12 2004-12-13 05:54:41 jason Exp $)

#include "IPInfo.h"
#include "util.h"

#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY(IPInfoEntryArray)

bool operator==(const IPInfoEntry &e1, const IPInfoEntry &e2)
{
	return
		(e1.IPAddress == e2.IPAddress) &&
		(e1.SubnetMask == e2.SubnetMask) &&
		(e1.NetworkAddress == e2.NetworkAddress) &&
		(e1.BroadcastAddress == e2.BroadcastAddress) &&
		(e1.InterfaceName == e2.InterfaceName);
}

bool operator==(const IPInfoEntryArray &a1, const IPInfoEntryArray &a2)
{
	bool x = a1.GetCount() == a2.GetCount();
	for (size_t i = 0; i < a1.GetCount() && x; ++i)
	{
		x &= (a1[i] == a2[i]);
	}
	return x;
}

IPInfoEntry::IPInfoEntry()
{
	IPAddress = SubnetMask = NetworkAddress = BroadcastAddress = 0;
	IPAddressString = SubnetMaskString = NetworkAddressString = BroadcastAddressString = wxString();
	InterfaceName = wxString();
	MTU = Metric = 0;
}

#if defined(__WXMSW__)

	#include <windows.h>
	#include <wx/msw/winundef.h>
	#include <iphlpapi.h>

	typedef DWORD (WINAPI *PFNGETIPADDRTABLE)(PMIB_IPADDRTABLE, PULONG, BOOL);

#elif defined(__UNIX__)

	#include <stdio.h>
	#include <stdlib.h>
	#include <unistd.h>
	#include <string.h>
	#include <errno.h>

	#include <sys/types.h>
	#include <sys/socket.h>
	#include <ifaddrs.h>
	#include <net/if.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>

#endif

IPInfoEntryArray GetIPInfo()
{

	IPInfoEntryArray entries;

	#if defined(__WXMSW__)

		HINSTANCE hIpHlpApi = LoadLibrary(wxT("iphlpapi"));

		if (hIpHlpApi)
		{

			PFNGETIPADDRTABLE pfnGetIpAddrTable =
				(PFNGETIPADDRTABLE)GetProcAddress(hIpHlpApi, "GetIpAddrTable");

			if (pfnGetIpAddrTable)
			{

				unsigned long dwSize = 0;
				pfnGetIpAddrTable(NULL, &dwSize, FALSE);
				if (dwSize > 0)
				{

					MIB_IPADDRTABLE *table = (MIB_IPADDRTABLE*)malloc(dwSize);

					if (pfnGetIpAddrTable(table, &dwSize, FALSE) == NO_ERROR)
					{
						for (size_t i = 0; i < table->dwNumEntries; ++i)
						{
							MIB_IPADDRROW *row = &table->table[i];
							if (row->dwAddr != 0 || row->dwMask != 0)
							{
								IPInfoEntry entry;
								entry.IPAddress = ntohl(row->dwAddr);
								entry.SubnetMask = ntohl(row->dwMask);
								entry.NetworkAddress = ntohl(row->dwAddr & row->dwMask);
								entry.BroadcastAddress  = ntohl((row->dwAddr & row->dwMask) | ~(row->dwMask));
								entry.IPAddressString = GetIPV4AddressString(entry.IPAddress);
								entry.SubnetMaskString = GetIPV4AddressString(entry.SubnetMask);
								entry.NetworkAddressString = GetIPV4AddressString(entry.NetworkAddress);
								entry.BroadcastAddressString = GetIPV4AddressString(entry.BroadcastAddress);
								entries.Add(entry);
							}
						}
					}

					free(table);

				}

			}

			FreeModule(hIpHlpApi);

		}

	#elif defined(__UNIX__)

		struct ifaddrs *addrs;

		if (getifaddrs(&addrs) == 0)
		{

			struct ifaddrs *n;

			for (n = addrs; n; n = n->ifa_next)
			{
			
				if (n->ifa_flags & IFF_UP &&
					n->ifa_addr &&
					n->ifa_addr->sa_family == AF_INET)
				{
				
					struct in_addr addr = ((struct sockaddr_in *)n->ifa_addr)->sin_addr;
					struct in_addr netmask = ((struct sockaddr_in *)n->ifa_netmask)->sin_addr;

					struct in_addr network;
					network.s_addr = addr.s_addr & netmask.s_addr;
					
					struct in_addr broadcast;
					if (n->ifa_flags & IFF_BROADCAST && n->ifa_broadaddr)
					{
						broadcast = ((struct sockaddr_in *)n->ifa_broadaddr)->sin_addr;
					}
					else
					{
						broadcast.s_addr = network.s_addr | ~netmask.s_addr;
					}

					IPInfoEntry entry;
					
					entry.InterfaceName = wxString(n->ifa_name, wxConvLibc);
					
					entry.IPAddress = ntohl(addr.s_addr);
					entry.SubnetMask = ntohl(netmask.s_addr);
					entry.NetworkAddress = ntohl(network.s_addr);
					entry.BroadcastAddress  = ntohl(broadcast.s_addr);
					
					entry.IPAddressString = GetIPV4AddressString(entry.IPAddress);
					entry.SubnetMaskString = GetIPV4AddressString(entry.SubnetMask);
					entry.NetworkAddressString = GetIPV4AddressString(entry.NetworkAddress);
					entry.BroadcastAddressString = GetIPV4AddressString(entry.BroadcastAddress);

					entry.MTU = 0;
					entry.Metric = 0;
					
					entries.Add(entry);
			
				}
			
			}

			freeifaddrs(addrs);
			addrs = 0;

		}
	
	#endif

	return entries;
}

wxUint32 GetIPV4Address(const wxString &ip)
{
	return inet_addr(ip.mb_str());
}

bool IsValidIPV4Address(const wxUint32 ip)
{
	return ip != INADDR_NONE;
}

wxUint32 GetIPV4Address(wxSockAddress &addr)
{
	wxCHECK_MSG(addr.Type() == wxSockAddress::IPV4, 0, wxT("Not an IPV4 address"));
	return GAddress_INET_GetHostAddress(addr.GetAddress());
}

wxString GetIPV4AddressString(wxUint32 ip)
{
	
	wxString result = wxString()
		<< ((ip >> 24) & 0xff) << wxT(".")
		<< ((ip >> 16) & 0xff) << wxT(".")
		<< ((ip >> 8)  & 0xff) << wxT(".")
		<< ((ip >> 0)  & 0xff);
	
	return result;
	
}

wxString GetIPV4AddressString(wxSockAddress &addr)
{
	return GetIPV4AddressString(GetIPV4Address(addr));
}

wxString GetIPV4String(wxSockAddress &addr, bool include_port, bool fail_if_dns_needed)
{
	wxCHECK_MSG(addr.Type() == wxSockAddress::IPV4, wxEmptyString, wxT("Not an IPV4 address"));
	wxIPV4address *ipv4 = static_cast<wxIPV4address*>(&addr);
	wxString retval;
	wxString ip = GetIPV4AddressString(*ipv4);
	if (ip == wxT("127.0.0.1"))
	{
		retval << wxT("localhost");
	}
	else if (ip == wxT("0.0.0.0"))
	{
		retval << wxT("*");
	}
	else if (LeftEq(ip, wxT("127.")))
	{
		retval << ip;
	}
	else
	{
		if (fail_if_dns_needed)
		{
			return wxEmptyString;
		}
		wxString hostname = ipv4->Hostname();
		retval << (hostname.Length() ? hostname : ip);
	}
	if (include_port)
	{
		retval << wxT(":") << (int)ipv4->Service();
	}
	return retval;
}

wxArrayString GetIPAddresses()
{

	IPInfoEntryArray entries = GetIPInfo();

	wxArrayString IPs;

	for (size_t i = 0; i < entries.GetCount(); ++i)
	{
		if (entries[i].IPAddressString != wxT("127.0.0.1"))
		{
			IPs.Add(entries[i].IPAddressString);
		}
	}

	if (IPs.GetCount() == 0)
	{
		IPs.Add(wxT("127.0.0.1"));
	}

	return IPs;

}
