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


#ifndef CPSCalc_H_
#define CPSCalc_H_

#include <wx/dynarray.h>
WX_DECLARE_OBJARRAY(wxLongLong_t, HistoryArray);

class CPSCalc
{

public:
	CPSCalc();
	virtual ~CPSCalc();

	void Reset();

	// this func must be called every 1000 ms
	wxLongLong_t Update(wxLongLong_t pos);

	wxLongLong_t GetCPS() { return m_cps; }
	bool IsCPSValid() { return m_history_filled > 1; }

protected:
	HistoryArray m_history;
	int m_history_filled;
	wxLongLong_t m_cps;
	wxLongLong_t m_pos;

};

#endif
