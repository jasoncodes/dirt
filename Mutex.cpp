#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: Mutex.cpp,v 1.3 2003-08-13 10:23:52 jason Exp $)

#include "Mutex.h"

#if defined(__WXMSW__)

#include <windows.h>

SECURITY_ATTRIBUTES* CreateSecurityAttributes()
{
	SECURITY_DESCRIPTOR *sd = new SECURITY_DESCRIPTOR;
	InitializeSecurityDescriptor(sd, SECURITY_DESCRIPTOR_REVISION);
	SetSecurityDescriptorDacl(sd, TRUE, NULL, FALSE);
	SECURITY_ATTRIBUTES *sa = new SECURITY_ATTRIBUTES;
	sa->nLength = sizeof (SECURITY_ATTRIBUTES);
	sa->bInheritHandle = FALSE;
	sa->lpSecurityDescriptor = sd;
	return sa;
}

void DestroySecurityAttributes(SECURITY_ATTRIBUTES *&sa)
{
	SECURITY_DESCRIPTOR *sd = (SECURITY_DESCRIPTOR*)sa->lpSecurityDescriptor;
	delete sd;
	delete sa;
	sa = NULL;
}

class MutexPrivate
{

	friend class Mutex;

protected:
	MutexPrivate(const wxString &path)
	{
		wxString name = ReplaceSlashes(path);
		SECURITY_ATTRIBUTES *sa = CreateSecurityAttributes();
		m_hMutex = CreateMutex(sa, FALSE, wxT("Global\\")+name);
		if (!m_hMutex)
		{
			m_hMutex = CreateMutex(sa, FALSE, name);
		}
		DestroySecurityAttributes(sa);
		wxASSERT_MSG(m_hMutex, wxT("Error creating mutex"));
	}

	~MutexPrivate()
	{
		CloseHandle(m_hMutex);
	}

	static wxString ReplaceSlashes(const wxString &path)
	{
		wxString name = path;
		name.Replace(wxT("\\"), wxT("_"));
		name.Replace(wxT("/"), wxT("_"));
		return name;
	}

	bool Lock()
	{
		if (m_hMutex)
		{
			DWORD retval = WaitForSingleObject(m_hMutex, INFINITE);
			if (retval == WAIT_OBJECT_0 || retval == WAIT_ABANDONED)
			{
				return true;
			}
			wxASSERT(retval == WAIT_TIMEOUT || retval == WAIT_FAILED);
			wxFAIL_MSG(wxT("Error locking mutex"));
		}
		return false;
	}

	bool Unlock()
	{
		if (m_hMutex)
		{
			BOOL retval = ReleaseMutex(m_hMutex);
			wxASSERT_MSG(retval != 0, wxT("Error unlocking mutex"));
			return (retval != 0);
		}
		return false;
	}

protected:
	HANDLE m_hMutex;

};

#elif defined(__UNIX__)

#include <wx/file.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h> // for S_I[RW]USR
#include <signal.h>   // for kill()
#include <errno.h>
#include <sys/file.h>

static int LockFile(int fd, bool lock)
{
	return flock(fd, lock ? LOCK_EX : LOCK_UN);
}

class MutexPrivate
{

	friend class Mutex;

protected:
	MutexPrivate(const wxString &path)
	{
		wxString name = ReplaceBackslashes(path);
		m_fd = open(name.fn_str(), O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
		wxASSERT_MSG(m_fd != -1, wxT("Error opening lock file"));
	}

	~MutexPrivate()
	{
		close(m_fd);
	}

	static wxString ReplaceBackslashes(const wxString &path)
	{
		wxString name = path;
		name.Replace(wxT("\\"), wxT("/"));
		return name;
	}

	bool Lock()
	{
		wxCHECK_MSG(m_fd != -1, false, wxT("Mutex lock file not open"));
		int retval = LockFile(m_fd, true);
		bool ok = (retval == 0);
		wxASSERT_MSG(ok, wxT("Error locking mutex"));
		return ok;
	}

	bool Unlock()
	{
		wxCHECK_MSG(m_fd != -1, false, wxT("Mutex lock file not open"));
		int retval = LockFile(m_fd, false);
		bool ok = (retval == 0);
		wxASSERT_MSG(ok, wxT("Error unlocking mutex"));
		return ok;
	}

protected:
	int m_fd;

};

#else

#error "Mutex not implemented on this platform"

#endif

Mutex::Mutex(const wxString &path)
{
	m_depth = 0;
	m_priv = new MutexPrivate(path);
}

Mutex::~Mutex()
{
	wxASSERT_MSG(!IsLocked(), wxT("Mutex destructor called while locked"));
	bool ok = true;
	while (ok && IsLocked())
	{
		ok = Unlock();
		wxASSERT_MSG(ok, wxT("Error unlocking mutex"));
	}
	delete m_priv;
}

bool Mutex::Lock()
{
	bool ok = true;
	m_section.Enter();
	if (m_depth == 0)
	{
		ok = m_priv->Lock();
		wxASSERT_MSG(ok, wxT("Error locking mutex"));
	}
	if (ok)
	{
		m_depth++;
	}
	m_section.Leave();
	return ok;
}

bool Mutex::Unlock()
{
	bool ok = true;
	m_section.Enter();
	wxASSERT_MSG(m_depth, wxT("Mutex not locked"));
	if (m_depth == 1)
	{
		ok = m_priv->Unlock();
		wxASSERT_MSG(ok, wxT("Error unlocking mutex"));
	}
	if (ok)
	{
		m_depth--;
	}
	m_section.Leave();
	return ok;
}

bool Mutex::IsLocked()
{
	return GetLockDepth() > 0;
}

size_t Mutex::GetLockDepth()
{
	size_t depth;
	m_section.Enter();
	depth = m_depth;
	m_section.Leave();
	return depth;
}
