#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: LanListFrame.cpp,v 1.4 2003-04-30 07:38:48 jason Exp $)

#include "LanListFrame.h"
#include "util.h"
#include "BroadcastSocket.h"

#include "res/dirt.xpm"

struct LanListItemData
{
	wxString ip;
	wxUint16 port;
	wxLongLong_t last_update_tick;
	wxString name;
};

enum
{
	ID_LIST = 1,
	ID_TIMER,
	ID_BROADCAST
};

static const int ping_interval = 2000;
static const int timeout_interval = 5000;

BEGIN_EVENT_TABLE(LanListFrame, wxFrame)
	EVT_CLOSE(LanListFrame::OnClose)
	EVT_SIZE(LanListFrame::OnSize)
	EVT_TIMER(ID_TIMER, LanListFrame::OnTimer)
	EVT_BROADCAST_SOCKET(ID_BROADCAST, LanListFrame::OnBroadcast)
	EVT_LIST_ITEM_ACTIVATED(ID_LIST, LanListFrame::OnItemActivate)
END_EVENT_TABLE()

LanListFrame::LanListFrame()
	: wxFrame(NULL, wxID_ANY, AppTitle(wxT("LAN List")), wxDefaultPosition, wxSize(600, 300), wxDEFAULT_FRAME_STYLE | wxNO_FULL_REPAINT_ON_RESIZE | wxCLIP_CHILDREN | wxTAB_TRAVERSAL, wxT("Dirt"))
{

	SetIcon(wxIcon(dirt_xpm));

	wxPanel *panel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNO_FULL_REPAINT_ON_RESIZE | wxCLIP_CHILDREN | wxTAB_TRAVERSAL);
	m_lstServers = new wxListCtrl(panel, ID_LIST, wxPoint(0, 0), wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL | wxNO_BORDER);

	m_lstServers->InsertColumn(0, wxT("Server Name"), wxLIST_FORMAT_LEFT,  128);
	m_lstServers->InsertColumn(1, wxT("IP"),          wxLIST_FORMAT_LEFT,  140);
	m_lstServers->InsertColumn(2, wxT("Latency"),     wxLIST_FORMAT_RIGHT, 56);
	m_lstServers->InsertColumn(3, wxT("Users"),       wxLIST_FORMAT_LEFT,  160);
	m_lstServers->InsertColumn(4, wxT("Avg Ping"),    wxLIST_FORMAT_RIGHT, 64);
	m_lstServers->InsertColumn(5, wxT("Version"),     wxLIST_FORMAT_LEFT,  128);
	m_lstServers->InsertColumn(6, wxT("Uptime"),      wxLIST_FORMAT_RIGHT, 64);
	m_lstServers->InsertColumn(7, wxT("Idle Time"),   wxLIST_FORMAT_RIGHT, 64);
	m_lstServers->InsertColumn(8, wxT("Comment"),     wxLIST_FORMAT_LEFT,  192);

	m_bcast = new BroadcastSocket;
	m_bcast->SetEventHandler(this, ID_BROADCAST);

	m_tmr = new wxTimer(this, ID_TIMER);
	m_tmr->Start(ping_interval);

	DoPing();

	CenterOnScreen();
	RestoreWindowState(this, &m_config, wxT("LAN List"), true, false);

}

LanListFrame::~LanListFrame()
{
	delete m_tmr;
	delete m_bcast;
}

void LanListFrame::OnClose(wxCloseEvent &event)
{
	Cleanup();
	SaveWindowState(this, &m_config, wxT("LAN List"));
	event.Skip();
}

void LanListFrame::OnSize(wxSizeEvent &event)
{
	m_lstServers->SetSize(GetClientSize());
	event.Skip();
}

void LanListFrame::OnTimer(wxTimerEvent &event)
{
	DoPing();
	wxLongLong_t now = GetMillisecondTicks();
	for (int i = m_lstServers->GetItemCount() - 1; i >= 0; --i)
	{
		LanListItemData *data = (LanListItemData*)(void*)m_lstServers->GetItemData(i);
		if (!data || now > data->last_update_tick + timeout_interval)
		{
			delete data;
			m_lstServers->DeleteItem(i);
		}
	}
}

void LanListFrame::OnBroadcast(BroadcastSocketEvent &event)
{
	wxString context, cmd;
	ByteBuffer data;
	if (DecodeMessage(event.GetData(), context, cmd, data))
	{
		cmd.MakeUpper();
		if (cmd == wxT("PONG"))
		{
			if (data.Length() > 9)
			{
				const byte *ptr = data.LockRead();
				ByteBuffer hashmap_data(ptr+9, data.Length()-9);
				wxLongLong_t timestamp = BytesToUint64(ptr, 8);
				byte sep_byte = ptr[8];
				data.Unlock();
				wxLongLong_t now = GetMillisecondTicks();
				wxLongLong_t latency = now - timestamp;
				StringHashMap server_info = UnpackStringHashMap(hashmap_data);
				if (sep_byte == 0 && latency >= 0 && server_info.size() > 0)
				{
					ProcessPong(event.GetIP(), event.GetPort(), now, (off_t)latency, server_info);
				}
			}
		}
	}
}

void LanListFrame::DoPing()
{
	m_bcast->SendAll(11626, EncodeMessage(wxEmptyString, wxT("PING"), Uint64ToBytes(GetMillisecondTicks())));
}

long LanListFrame::GetEntry(const wxString &ip, wxUint16 port, bool create_if_not_exist)
{
	for (int i = 0; i < m_lstServers->GetItemCount(); ++i)
	{
		LanListItemData *data = (LanListItemData*)(void*)m_lstServers->GetItemData(i);
		if (data && data->ip == ip && data->port == port)
		{
			return i;
		}
	}
	if (create_if_not_exist)
	{
		wxListItem item;
		item.SetId(m_lstServers->GetItemCount());
		LanListItemData *data = new LanListItemData;
		data->ip = ip;
		data->port = port;
		item.SetData(data);
		item.SetMask(wxLIST_MASK_DATA);
		return m_lstServers->InsertItem(item);
	}
	return -1;
}

static wxString GetAndKill(StringHashMap &hash, const wxString &key)
{
	wxString value = hash[key];
	hash.erase(key);
	return value;
}

static long ToLong(const wxString &text, long default_value = 0)
{
	long x;
	return text.ToLong(&x) ? x : default_value;
}

void LanListFrame::ProcessPong(const wxString &ip, wxUint16 port, wxLongLong_t last_update_tick, off_t latency, const StringHashMap &server_info)
{
	long index = GetEntry(ip, port, true);
	wxASSERT(index > -1);
	LanListItemData *data = (LanListItemData*)(void*)m_lstServers->GetItemData(index);
	StringHashMap info(server_info);
	data->last_update_tick = last_update_tick;
	data->name = GetAndKill(info, wxT("name"));
	m_lstServers->SetItem(index, 0, data->name);
	m_lstServers->SetItem(index, 1, wxString() << ip << wxT(':') << (int)port);
	m_lstServers->SetItem(index, 2, AddCommas(latency) + wxT(" ms"));
	wxString users;
	users
		<< GetAndKill(info, wxT("usercount")) << wxT(" (")
		<< GetAndKill(info, wxT("away")) << wxT(" away, ")
		<< GetAndKill(info, wxT("peakusers")) << wxT(" peak, ")
		<< GetAndKill(info, wxT("maxusers")) << wxT(" max)");
	m_lstServers->SetItem(index, 3, users);
	m_lstServers->SetItem(index, 4, AddCommas((off_t)ToLong(GetAndKill(info, wxT("avgping")))) + wxT(" ms"));
	m_lstServers->SetItem(index, 5, GetAndKill(info, wxT("version")));
	m_lstServers->SetItem(index, 6, SecondsToMMSS(ToLong(GetAndKill(info, wxT("idletime")))));
	m_lstServers->SetItem(index, 7, SecondsToMMSS(ToLong(GetAndKill(info, wxT("uptime")))));
	m_lstServers->SetItem(index, 8, GetAndKill(info, wxT("comment")));
	info.erase(wxT("iplist"));
	info.erase(wxT("hostname"));
	//for (StringHashMap::iterator i = info.begin(); i != info.end(); ++i)
	//{
	//	OutputDebugString(i->first + wxT(" "));
	//}
	//OutputDebugString(wxT("\n"));
	Sort();
}

static int wxCALLBACK LanListItemCompareFunc(long item1, long item2, long sortData)
{
	LanListItemData *data1 = (LanListItemData*)(void*)item1;
	LanListItemData *data2 = (LanListItemData*)(void*)item2;
	int retval = data1->name.CmpNoCase(data2->name);
	if (retval == 0)
	{
		retval = data1->ip.Cmp(data2->ip);
	}
	return retval;
}

void LanListFrame::Sort()
{
	m_lstServers->SortItems(&LanListItemCompareFunc, 0);
}

void LanListFrame::Cleanup()
{
	for (int i = 0; i < m_lstServers->GetItemCount(); ++i)
	{
		LanListItemData *data = (LanListItemData*)(void*)m_lstServers->GetItemData(i);
		delete data;
	}
	m_lstServers->DeleteAllItems();
}

void LanListFrame::OnItemActivate(wxListEvent &event)
{
	int i = event.GetIndex();
	if (i >= 0 && i < m_lstServers->GetItemCount())
	{
		LanListItemData *data = (LanListItemData*)(void*)m_lstServers->GetItemData(i);
		if (data)
		{
			wxString param;
			param << wxT("--host=") << data->ip << wxT(':') << (int)data->port;
			const wxChar *argv[3];
			wxString self = GetSelf();
			argv[0] = self.c_str();
			argv[1] = param.c_str();
			argv[2] = NULL;
			if (::wxExecute((wxChar**)argv))
			{
				Destroy();
			}
		}
	}
}
