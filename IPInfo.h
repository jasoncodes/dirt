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
