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


#ifndef IPInfo_H_
#define IPInfo_H_

#include <wx/socket.h>

struct IPInfoEntry
{
public:
	wxUint32 IPAddress, SubnetMask, NetworkAddress, BroadcastAddress;
	wxString IPAddressString, SubnetMaskString, NetworkAddressString, BroadcastAddressString;
	wxString InterfaceName;
	int MTU, Metric;
public:
	IPInfoEntry();
};

#include <wx/dynarray.h>
WX_DECLARE_OBJARRAY(IPInfoEntry, IPInfoEntryArray);

bool operator==(const IPInfoEntry &e1, const IPInfoEntry &e2);
bool operator==(const IPInfoEntryArray &a1, const IPInfoEntryArray &a2);
inline bool operator!=(const IPInfoEntryArray &a1, const IPInfoEntryArray &a2) { return !(a1 == a2); }

IPInfoEntryArray GetIPInfo();

wxUint32 GetIPV4Address(const wxString &ip);
wxUint32 GetIPV4Address(wxSockAddress &addr);
bool IsValidIPV4Address(const wxUint32 ip);
wxString GetIPV4AddressString(wxUint32 ip);
wxString GetIPV4AddressString(wxSockAddress &addr);
wxString GetIPV4String(wxSockAddress &addr, bool include_port);
wxArrayString GetIPAddresses();

#endif
