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


#ifndef Console_H_
#define Console_H_

class ConsoleReadThread;

class Console : public wxEvtHandler
{

public:

	Console(bool no_input = false);
	virtual ~Console();

	virtual void ExitMainLoop();
	virtual void Output(const wxString &line);

protected:
	void OnTextEnter(wxCommandEvent& event);

protected:
	virtual void OnInput(const wxString &line) = 0;
	virtual void OnEOF() = 0;

protected:
	ConsoleReadThread *m_read_thread;

private:
	DECLARE_EVENT_TABLE()
	DECLARE_NO_COPY_CLASS(Console)

};

#endif
