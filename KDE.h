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


#ifndef KDE_H_
#define KDE_H_

#ifdef KDE_AVAILABLE

class KApplication;

bool DoesDCOPFileExist();

class KDEThread : public wxThread
{

protected:
	KDEThread();

public:
	static void Init();
	static void CleanUp();

protected:
	virtual ExitCode Entry();

protected:
	static KDEThread *s_thread;
	KApplication *m_kapp;
	static wxCriticalSection s_section;
	static wxMutex s_mutex;
	static wxCondition s_cond;

};

#endif

#endif
