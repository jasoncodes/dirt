#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: IPInfo.cpp,v 1.3 2003-05-27 17:02:19 jason Exp $)

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

#elif defined(__UNIX__)

	#include <stdio.h>
	#include <stdlib.h>
	#include <unistd.h>
	#include <string.h>
	#include <errno.h>

	#include <sys/socket.h>
	#include <sys/types.h>
	#include <net/if.h>
	#include <netdb.h>

	#include <sys/ioctl.h>
	#include <net/if_arp.h>
	#include <arpa/inet.h>

#else

	#include <unistd.h>
	#include <sys/socket.h>
	#include <netdb.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>

#endif

IPInfoEntryArray GetIPInfo()
{

	IPInfoEntryArray entries;

	#if defined(__WXMSW__)
		unsigned long dwSize = 0;
		GetIpAddrTable(NULL, &dwSize, FALSE);
		if (dwSize > 0)
		{
			MIB_IPADDRTABLE *table = (MIB_IPADDRTABLE*)malloc(dwSize);
			if (GetIpAddrTable(table, &dwSize, FALSE) == NO_ERROR)
			{
				for (size_t i = 0; i < table->dwNumEntries; ++i)
				{
					MIB_IPADDRROW *row = &table->table[i];
					if (row->dwAddr != 0 || row->dwMask != 0)
					{
						IPInfoEntry entry;
						entry.IPAddress = row->dwAddr;
						entry.SubnetMask = row->dwMask;
						entry.NetworkAddress = (row->dwAddr & row->dwMask);
						entry.BroadcastAddress  = (row->dwAddr & row->dwMask) | ~(row->dwMask);
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

	#elif defined(__UNIX__)

		// begin new stuff

		#define inaddrr(x) (*(struct in_addr *) &ifr->x[sizeof sa.sin_port])
		#define IFRSIZE ((int)(size * sizeof (struct ifreq)))

		unsigned char *u;
		int sockfd, size = 1;
		struct ifreq *ifr;
		struct ifconf ifc;
		struct sockaddr_in sa;

		if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP)) > 0)
		{

			ifc.ifc_len = IFRSIZE;
			ifc.ifc_req = NULL;

			do
			{

				++size;

				/* realloc buffer size until no overflow occurs  */
				ifc.ifc_req = (ifreq*)realloc(ifc.ifc_req, IFRSIZE);
				wxASSERT(ifc.ifc_req);

				ifc.ifc_len = IFRSIZE;

				int ret = ioctl(sockfd, SIOCGIFCONF, &ifc);
				wxASSERT(ret == 0);

			}
			while (IFRSIZE <= ifc.ifc_len);

			ifr = ifc.ifc_req;

			for (; (char*)ifr < (char*)ifc.ifc_req + ifc.ifc_len; ++ifr)
			{

				if (ifr->ifr_addr.sa_data == (ifr + 1)->ifr_addr.sa_data)
				{
					continue; /* duplicate, skip it */
				}

				if (ioctl(sockfd, SIOCGIFFLAGS, ifr))
				{
					continue; /* failed to get flags, skip it */
				}

				IPInfoEntry entry;
				entry.InterfaceName = wxString(ifr->ifr_name, wxConvLocal);
				entry.IPAddressString = wxString(inet_ntoa(inaddrr(ifr_addr.sa_data)), wxConvLocal);
				entry.IPAddress = inet_addr(entry.IPAddressString.mb_str());

				if (0 == ioctl(sockfd, SIOCGIFNETMASK, ifr))
				{
					entry.SubnetMaskString = wxString(inet_ntoa(inaddrr(ifr_addr.sa_data)), wxConvLocal);
					entry.SubnetMask = inet_addr(entry.SubnetMaskString.mb_str());
				}

				if (ifr->ifr_flags & IFF_BROADCAST)
				{
					if (0 == ioctl(sockfd, SIOCGIFBRDADDR, ifr))
					{
						entry.BroadcastAddressString = wxString(inet_ntoa(inaddrr(ifr_addr.sa_data)), wxConvLocal);
						entry.BroadcastAddress = inet_addr(entry.BroadcastAddressString.mb_str());
					}
				}

				if (0 == ioctl(sockfd, SIOCGIFMTU, ifr))
				{
					entry.MTU = ifr->ifr_mtu;
				}

				if (0 == ioctl(sockfd, SIOCGIFMETRIC, ifr))
				{
					entry.Metric = ifr->ifr_metric;
				}

				entries.Add(entry);

			}

		}

		close (sockfd);

		// end new stuff

	#else

		// fallback for other OS (which might not even work)
		char ac[80];
		if (gethostname(ac, sizeof(ac)) == 0)
		{

			const wxString localhost_str = wxT("127.0.0.1");
			bool found_localhost = false;
			struct hostent *phe = gethostbyname(ac);
			if (phe != 0)
			{

				for (int i = 0; phe->h_addr_list[i] != 0; ++i)
				{
					struct in_addr addr;
					memcpy(&addr, phe->h_addr_list[i], sizeof(struct in_addr));
					const char *ip = inet_ntoa(addr);
					IPInfoEntry entry;
					entry.IPAddressString = ByteBuffer((const byte*)ip, strlen(ip));
					entry.IPAddress = inet_addr(entry.IPAddressString);
					found_localhost != (entry.IPAddressString == localhost_str);
					entries.Add(entry);
				}


				if (!found_localhost)
				{
					IPInfoEntry entry;
					entry.IPAddressString = localhost_str;
					entry.IPAddress = inet_addr(entry.IPAddressString);
					entries.Add(entry);
				}

			}

		}

	#endif

	return entries;
}

wxUint32 GetIPV4Address(const wxString &ip)
{
	return inet_addr(ip.mb_str());
}

wxUint32 GetIPV4Address(wxSockAddress &addr)
{
	wxCHECK_MSG(addr.Type() == wxSockAddress::IPV4, 0, wxT("Not an IPV4 address"));
	return GAddress_INET_GetHostAddress(addr.GetAddress());
}

wxString GetIPV4AddressString(wxUint32 ip)
{
	
	ip = wxUINT32_SWAP_ON_LE(ip);
	
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

wxString GetIPV4String(wxSockAddress &addr, bool include_port)
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
		retval << ipv4->Hostname();
	}
	if (include_port)
	{
		retval << wxT(":") << (int)ipv4->Service();
	}
	return retval;
}

wxArrayString GetIPAddresses()
{

	wxArrayString IPs;

	char ac[80];
	if (gethostname(ac, sizeof(ac)) != 0)
	{
		return IPs;
	}

	struct hostent *phe = gethostbyname(ac);
	if (phe == 0)
	{
		return IPs;
	}

	for (int i = 0; phe->h_addr_list[i] != 0; ++i)
	{
		struct in_addr addr;
		memcpy(&addr, phe->h_addr_list[i], sizeof(struct in_addr));
		const char *ip = inet_ntoa(addr);
		IPs.Add(ByteBuffer((const byte*)ip, strlen(ip)));
	}

	if (IPs.GetCount() == 0)
	{
		IPs.Add(wxT("127.0.0.1"));
	}

	return IPs;
}
