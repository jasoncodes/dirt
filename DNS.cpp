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
RCS_ID($Id: DNS.cpp,v 1.19 2004-05-30 09:45:07 jason Exp $)

#include "DNS.h"
#include "IPInfo.h"
#include <wx/module.h>
#include <wx/thread.h>
#include "util.h"

#define DNS_DEBUG 0

#if DNS_DEBUG
#define DNSDebugMsg(x) DebugMsg(x)
#else
#define DNSDebugMsg(x)
#endif

class DNSThread;

// event

const wxEventType wxEVT_DNS = wxNewEventType();

// queue structure

struct DNSQueueEntry
{
	wxString question;
	bool is_reverse;
	bool in_progress;
	DNS *owner;
	void *userdata;
};

#include <wx/list.h>
WX_DECLARE_LIST(DNSQueueEntry, DNSQueue);
#include <wx/listimpl.cpp>
WX_DEFINE_LIST(DNSQueue);

// global variables

static wxCriticalSection *s_DNS_section = NULL;
static DNSQueue *s_DNS_queue = NULL;
static wxMutex *s_DNS_condition_mutex = NULL;
static wxCondition *s_DNS_condition = NULL;
static wxMutex *s_DNS_startup_condition_mutex = NULL;
static wxCondition *s_DNS_startup_condition = NULL;
static DNSThread *s_DNS_thread = NULL;
static bool s_DNS_shutdown = false;
static bool s_DNS_going_to_signal = false;

// helpers

// deletes the entry referred to by the node
// and the node itself from the queue
// returns a node that is safe to enumerate from
// (can be and probably will be the first node)
// return value will be NULL if no entries left
static DNSQueue::Node *DeleteQueueEntryNode(DNSQueue::Node *node)
{
	delete node->GetData();
	delete node;
	// the docs seem to suggest there's no way to continue
	// on after deleting an entry mid-enum, so start at the top
	node = s_DNS_queue->GetFirst();
	return node;
}

// thread

class DNSThread : public wxThread
{

	friend class DNS;

protected:
	DNSThread()
		: wxThread(wxTHREAD_JOINABLE)
	{
	}

	virtual ExitCode Entry()
	{

		DNSDebugMsg(wxT("thread: locking main mutex"));
		// lock the condition mutex all ready for notifications from main thread
		s_DNS_condition_mutex->Lock();

		DNSDebugMsg(wxT("thread: broadcasting ready to go msg"));
		// let the main thread know we are up and running
		// and the normal condition is ready to be notified on
		s_DNS_startup_condition->Broadcast();

		DNSDebugMsg(wxT("thread: here we go"));
		// variables used to send events, NULL normally
		wxEvtHandler *handler = NULL;
		wxEvent *event = NULL;

		while (!TestDestroy() && !s_DNS_shutdown)
		{
			
			s_DNS_section->Enter();
			bool is_empty = (s_DNS_queue->GetFirst() == NULL);
			s_DNS_section->Leave();

			if (s_DNS_going_to_signal || is_empty)
			{
				DNSDebugMsg(wxT("thread: calling wait"));
				s_DNS_condition->Wait();
				DNSDebugMsg(wxT("thread: wait returned"));
			}

			// if we are shutting down, bail out
			if (TestDestroy() || s_DNS_shutdown)
			{
				return NULL;
			}

			s_DNS_section->Enter();
			
			if (!s_DNS_going_to_signal)
			{
				// after a wait, we need to set up the condition again
				//DNSDebugMsg(wxT("thread: re-locking main mutex"));
				//s_DNS_condition_mutex->Lock();
				//DNSDebugMsg(wxT("thread: main mutex re-locked"));
				DNSDebugMsg(wxT("thread: destroying condition"));
				delete s_DNS_condition;
				delete s_DNS_condition_mutex;
				DNSDebugMsg(wxT("thread: creating new condtion"));
				s_DNS_condition_mutex = new wxMutex;
				s_DNS_condition = new wxCondition(*s_DNS_condition_mutex);
				DNSDebugMsg(wxT("thread: locking condition mutex"));
				s_DNS_condition_mutex->Lock();
				DNSDebugMsg(wxT("thread: condition all ready to go again"));
				s_DNS_going_to_signal = false;
			}

			// locate the first entry that isn't being processed
			// this code should make it safe to run multiple lookup
			// threads if required in the future
			DNSQueue::Node *node = s_DNS_queue->GetFirst();
			while ((node != NULL) && (node->GetData()->in_progress))
			{
				node = node->GetNext();
			}

			DNSQueueEntry *entry = node ? node->GetData() : NULL;

			// if we have something to do, do it :)
			if (entry != NULL)
			{

				// grab the lookup information
				// we are grabbing a copy because it's not safe
				// to use the entry pointer outside of the lock
				// (i.e. during the DNS lookup)
				wxString question = entry->question;
				bool is_reverse = entry->is_reverse;

				// mark this entry as in progress
				entry->in_progress = true;
				
				s_DNS_section->Leave();
				
				bool success; // not implemented
				wxString hostname; // not implemented
				wxIPV4address addr; // not implemented
				wxUint32 ip; // not implemented
				if (is_reverse)
				{
					ip = GetIPV4Address(question);
					if (!IsValidIPV4Address(ip))
					{
						success = false;
						hostname = wxEmptyString;
					}
					else
					{
						ip = wxUINT32_SWAP_ON_LE(ip);
						addr.Hostname(question);
						hostname = addr.Hostname();
						success = hostname.Length() > 0;
						if (!success)
						{
							hostname = question;
						}
					}
				}
				else
				{
					DNSDebugMsg(wxT("thread: looking up ") + question);
					hostname = question;
					success = addr.Hostname(hostname);
					ip = success ? GetIPV4Address(addr) : wxUint32(-1);
					DNSDebugMsg(wxT("thread: lookup complete: ") + GetIPV4AddressString(ip));
				}
				
				// bail out if we are shutting down
				if (TestDestroy() || s_DNS_shutdown)
				{
					return NULL;
				}
				
				s_DNS_section->Enter();
				
				if (entry->owner)
				{
					handler = entry->owner->m_handler;
					event = new DNSEvent(
						entry->owner->m_id, entry->owner,
						success, hostname, addr, ip,
						entry->userdata);
				}

				DeleteQueueEntryNode(node);

			}

			s_DNS_section->Leave();

			// queue the event outside of our lock to
			// prevent any possible deadlocking
			if (handler)
			{
				if (event)
				{
					DNSDebugMsg(wxT("sending dns result event"));
					handler->AddPendingEvent(*event);
					DNSDebugMsg(wxT("sent dns result event"));
					delete event;
					event = NULL;
				}
				handler = NULL;
			}

		}

		return NULL;

	}

protected:
	DECLARE_NO_COPY_CLASS(DNSThread)

};

// module

class DNSModule: public wxModule
{

	DECLARE_DYNAMIC_CLASS(DNSModule)

public:
	DNSModule()
	{
	}

	bool OnInit()
	{
		s_DNS_section = new wxCriticalSection;
		s_DNS_queue = NULL;
		s_DNS_condition_mutex = NULL;
		s_DNS_condition = NULL;
		s_DNS_startup_condition_mutex = NULL;
		s_DNS_startup_condition = NULL;
		s_DNS_thread = NULL;
		s_DNS_shutdown = false;
		return true;
	}

	void OnExit()
	{
		if (s_DNS_thread)
		{
			s_DNS_shutdown = true;
			s_DNS_condition->Broadcast();
			s_DNS_thread->Delete();
		}
		delete s_DNS_thread;
		delete s_DNS_condition;
		delete s_DNS_condition_mutex;
		delete s_DNS_startup_condition;
		delete s_DNS_startup_condition_mutex;
		delete s_DNS_queue;
		delete s_DNS_section;
	}

};

IMPLEMENT_DYNAMIC_CLASS(DNSModule, wxModule)

// DNS Interface

DNS::DNS()
{
	m_handler = NULL;
	m_id = wxID_ANY;
}

DNS::~DNS()
{
	Cancel();
}

void DNS::SetEventHandler(wxEvtHandler *handler, wxEventType id)
{
	m_handler = handler;
	m_id = id;
}

void DNS::Cancel()
{

	s_DNS_section->Enter();

	// if we aren't initialized yet, no cleanup needed
	if (s_DNS_thread)
	{

		DNSQueue::Node *node = s_DNS_queue->GetFirst();

		while (node)
		{

			DNSQueueEntry *entry = node->GetData();

			if (entry->owner == this)
			{
				if (entry->in_progress)
				{
					// let the thread know we don't want the response
					entry->owner = NULL;
					entry->userdata = NULL;
					// and then move on
					node = node->GetNext();
				}
				else
				{
					// the entry can be deleted safely now and we
					// can avoid the lookup all together
					// this call delets both the node and the entry
					entry = NULL;
					node = DeleteQueueEntryNode(node);
				}
			}
			else
			{
				// not ours, move on
				node = node->GetNext();
			}

		}

	}

	s_DNS_section->Leave();

}

void DNS::Lookup(const wxString &question, bool is_reverse, void *userdata)
{

	DNSDebugMsg(wxT("main: lookup called with " + question));

	DNSDebugMsg(wxT("main: grabbing mutex"));
	s_DNS_section->Enter();
	DNSDebugMsg(wxT("main: got mutex"));

	// thread startup code
	if (!s_DNS_thread)
	{

		// due to a bug in wx, we make sure the first socket call
		// occurs on the main thread, otherwise bad things happen
		{
			wxIPV4address addr;
			addr.Hostname(wxT("127.0.0.1"));
		}

		DNSDebugMsg(wxT("main: first call, creating DNS thread"));
		// create everything
		s_DNS_queue = new DNSQueue;
		s_DNS_condition_mutex = new wxMutex;
		s_DNS_condition = new wxCondition(*s_DNS_condition_mutex);
		s_DNS_startup_condition_mutex = new wxMutex;
		s_DNS_startup_condition = new wxCondition(*s_DNS_startup_condition_mutex);
		s_DNS_startup_condition_mutex->Lock();
		s_DNS_thread = new DNSThread;
		s_DNS_going_to_signal = true;
		
		DNSDebugMsg(wxT("main: starting the thread"));
		// start the thread
		if (s_DNS_thread->Create() != wxTHREAD_NO_ERROR)
		{
			wxLogFatalError(wxT("Error creating DNS thread"));
			return;
		}
		if (s_DNS_thread->Run() != wxTHREAD_NO_ERROR)
		{
			wxLogFatalError(wxT("Error starting DNS thread"));
			return;
		}

		DNSDebugMsg(wxT("main: waiting for the thread"));
		// wait for thread to be ready
		s_DNS_startup_condition->Wait();

		DNSDebugMsg(wxT("main: thread is ready"));

	}

	s_DNS_going_to_signal = (s_DNS_queue->GetFirst() == NULL);

	DNSDebugMsg(wxT("main: appending dns entry"));
	// append the new entry
	DNSQueueEntry *entry = new DNSQueueEntry;
	entry->question = question;
	entry->is_reverse = is_reverse;
	entry->in_progress = false;
	entry->owner = this;
	entry->userdata = userdata;
	s_DNS_queue->Append(entry);

	s_DNS_section->Leave();
	DNSDebugMsg(wxT("main: released mutex"));

	// wake the thread up if this is the first entry in the queue
	if (s_DNS_going_to_signal)
	{
		DNSDebugMsg(wxT("main: first entry in queue, locking mutex"));
		s_DNS_condition_mutex->Lock();
		DNSDebugMsg(wxT("main: signalling"));
		s_DNS_condition->Signal();
		DNSDebugMsg(wxT("main: unlocking mutex"));
		s_DNS_condition_mutex->Unlock();
	}

}
