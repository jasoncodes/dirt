#ifndef Mutex_H_
#define Mutex_H_

// This mutex class is for use in syncronizing multiple processes
// only and has not been tested with multiple threads in one process
// For multiple threads in a single process, please use wxThread.

class MutexPrivate;

class Mutex
{

public:
	Mutex(const wxString &path);
	~Mutex();

	bool Lock();
	bool Unlock();
	bool IsLocked();
	size_t GetLockDepth();

protected:
	wxCriticalSection m_section;
	size_t m_depth;
	MutexPrivate *m_priv;

private:
	DECLARE_NO_COPY_CLASS(Mutex)

};

#endif
