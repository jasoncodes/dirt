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


#ifndef ClientTimers_H_
#define ClientTimers_H_

#include <wx/datetime.h>
#include <wx/dynarray.h>

class ClientTimers;

class ClientTimer
{

	friend class ClientTimers;

protected:
	ClientTimer()
	{
	}

public:
	wxString GetName() const { return m_name; }
	wxString GetContext() const { return m_context; }
	wxString GetCommands() const { return m_commands; }
	wxDateTime GetStartTime() const { return m_start_time; }
	wxLongLong_t GetInterval() const { return m_interval; }
	wxLongLong_t GetNextTick() const { return m_next_tick; }
	int GetTimesRemaining() const { return m_times_remaining; }

protected:
	wxString m_name;
	wxString m_context;
	wxString m_commands;
	wxDateTime m_start_time;
	wxLongLong_t m_interval;
	wxLongLong_t m_next_tick;
	int m_times_remaining;

};

WX_DECLARE_OBJARRAY(ClientTimer, ClientTimerArray);

class ClientTimers : public wxEvtHandler
{

public:
	ClientTimers(wxEvtHandler *handler, int id = wxID_ANY);
	virtual ~ClientTimers();

	size_t GetCount() const;
	int Find(const wxString &name) const;
	const ClientTimer* Item(const size_t index) const; 
	const ClientTimer* Item(const wxString &name) const;

	bool Add(const wxString &name, const wxString &context, const wxString &commands, const wxDateTime &start_time = wxInvalidDateTime, const wxLongLong_t interval = 1, const wxLongLong_t times = 1);
	bool Remove(const wxString &name);
	bool Remove(const size_t index);

	bool Clear();

protected:
	void OnTimer(wxTimerEvent &event);

protected:
	wxEvtHandler *m_handler;
	int m_id;
	ClientTimerArray m_timers;
	wxTimer *m_timer;

private:
	DECLARE_EVENT_TABLE()
	DECLARE_NO_COPY_CLASS(ClientTimers)

};

extern const wxEventType wxEVT_CLIENT_TIMERS;

class ClientTimersEvent : public wxEvent
{

public:
	ClientTimersEvent(int id, ClientTimers *src, const ClientTimer &timer)
		: wxEvent(id, wxEVT_CLIENT_TIMERS), m_timer(timer)
	{
		SetEventObject(src);
	}

	ClientTimersEvent(const ClientTimersEvent &evt)
		: wxEvent(evt.GetId(), wxEVT_CLIENT_TIMERS), m_timer(evt.GetTimer())
	{
		SetEventObject(evt.GetEventObject());
	}

	virtual ~ClientTimersEvent()
	{
	}

	virtual const ClientTimer& GetTimer() const
	{
		return m_timer;
	}

	virtual wxEvent *Clone() const
	{
		return new ClientTimersEvent(*this);
	}

protected:
	const ClientTimer m_timer;

};

typedef void (wxEvtHandler::*ClientTimersEventFunction)(ClientTimersEvent&);

#define EVT_CLIENT_TIMERS(id, func) \
	DECLARE_EVENT_TABLE_ENTRY( \
		wxEVT_CLIENT_TIMERS, id, -1, \
		(wxObjectEventFunction) \
		(wxEventFunction) \
		(ClientTimersEventFunction) & func, \
		(wxObject *) NULL ),

#endif
