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


#ifndef RCS_H_
#define RCS_H_

// use this at the top of every CPP file like: RCS_ID($Id: RCS.h,v 1.5 2004-05-16 04:42:47 jason Exp $)
#define RCS_ID(x,y) static int rcs_id_ ## wxMAKE_UNIQUE_ASSERT_NAME = add_rcs_id( wxT( #x ), wxT( #y ) );

// call this when you want to know the calculated info
wxString GetRCSDate();
wxString GetRCSAuthor();
wxString GetProductVersion();

// don't call this directly, used by RCS_ID macro
int add_rcs_id(const wxChar *x, const wxChar *y);

#endif
