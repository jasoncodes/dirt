#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: ClientTimers.cpp,v 1.3 2004-04-25 18:41:55 jason Exp $)

#include "ClientTimers.h"
#include "util.h"

#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY(ClientTimerArray);

const wxEventType wxEVT_CLIENT_TIMERS = wxNewEventType();

enum
{
	ID_TIMER = 1
};

BEGIN_EVENT_TABLE(ClientTimers, wxEvtHandler)
	EVT_TIMER(ID_TIMER, ClientTimers::OnTimer)
END_EVENT_TABLE()

ClientTimers::ClientTimers(wxEvtHandler *handler, int id)
{
	m_handler = handler;
	m_id = id;
	m_timer = new wxTimer(this, ID_TIMER);
}

ClientTimers::~ClientTimers()
{
	delete m_timer;
}

size_t ClientTimers::GetCount() const
{
	return m_timers.GetCount();
}

int ClientTimers::Find(const wxString &name) const
{
	for (size_t i = 0; i < m_timers.GetCount(); ++i)
	{
		if (Item(i)->GetName() == name)
		{
			return i;
		}
	}
	return -1;
}

const ClientTimer* ClientTimers::Item(const size_t index) const
{
	if (index < m_timers.GetCount())
	{
		return &m_timers.Item(index);
	}
	return NULL;
}

const ClientTimer* ClientTimers::Item(const wxString &name) const
{
	int index = Find(name);
	if (index > -1)
	{
		return Item(index);
	}
	return NULL;
}

bool ClientTimers::Add(const wxString &name, const wxString &context, const wxString &commands, const wxDateTime &start_time, const wxLongLong_t interval, const wxLongLong_t times)
{
	
	if (!name.Length() || !commands.Length() || !(interval > 0) || !(times > 0 || times == -1))
	{
		return false;
	}
	
	int index = Find(name);

	ClientTimer *tmr;
	bool is_new;
	
	if (index > -1)
	{
		tmr = &m_timers.Item(index);
		is_new = false;
	}
	else
	{
		tmr = new ClientTimer;
		is_new = true;
	}

	tmr->m_name = name;
	tmr->m_context = context;
	tmr->m_commands = commands;
	tmr->m_interval = interval;
	tmr->m_times_remaining = times;

	if (start_time.IsValid() && start_time > wxDateTime::Now())
	{
		tmr->m_start_time = start_time;
		tmr->m_next_tick = 0;
	}
	else
	{
		tmr->m_start_time = wxInvalidDateTime;
		tmr->m_next_tick = GetMillisecondTicks() + interval;
	}

	if (is_new)
	{
		m_timers.Add(*tmr);
		delete tmr;
	}

	if (!m_timer->IsRunning())
	{
		m_timer->Start(250);
	}

	return true;

}

bool ClientTimers::Remove(const wxString &name)
{
	int index = Find(name);
	if (index > -1)
	{
		return Remove(index);
	}
	return false;
}

bool ClientTimers::Remove(const size_t index)
{
	if (index < m_timers.GetCount())
	{
		m_timers.RemoveAt(index);
		if (!m_timers.GetCount())
		{
			m_timer->Stop();
		}
		return true;
	}
	return false;
}

bool ClientTimers::Clear()
{
	if (m_timers.GetCount())
	{
		m_timers.Clear();
		m_timer->Stop();
		return true;
	}
	return false;
}

void ClientTimers::OnTimer(wxTimerEvent &WXUNUSED(event))
{
	bool remove_any = false;
	wxLongLong_t now = GetMillisecondTicks();
	wxDateTime time_now = wxDateTime::Now();
	for (size_t i = 0; i < m_timers.GetCount(); ++i)
	{
		ClientTimer &tmr = m_timers.Item(i);
		if (tmr.m_times_remaining > 0 || tmr.m_times_remaining == -1)
		{

			bool is_time = tmr.m_start_time.IsValid() ?
				(time_now >= tmr.m_start_time) :
				(now >= tmr.m_next_tick);

			if (is_time)
			{
				ClientTimersEvent evt(m_id, this, tmr);
				m_handler->AddPendingEvent(evt);
				tmr.m_start_time = wxInvalidDateTime;
				if (tmr.m_times_remaining > 0)
				{
					tmr.m_times_remaining--;
					if (tmr.m_times_remaining > 0)
					{
						tmr.m_next_tick = now + tmr.m_interval;
					}
					else
					{
						remove_any = true;
					}
				}
				else if (tmr.m_times_remaining == -1)
				{
					tmr.m_next_tick = now + tmr.m_interval;
				}
				else
				{
					wxFAIL_MSG(wxT("uh oh.. ClientTimers are broken."));
				}
			}
		}
	}
	if (remove_any)
	{
		for (size_t i = m_timers.GetCount(); i > 0; --i)
		{
			ClientTimer &tmr = m_timers.Item(i-1);
			if (tmr.m_times_remaining == 0)
			{
				m_timers.RemoveAt(i-1);
			}
		}
	}
	if (!m_timers.GetCount())
	{
		m_timer->Stop();
	}
}
