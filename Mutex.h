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
