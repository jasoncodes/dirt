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


#ifndef SwitchBarMDI_H_
#define SwitchBarMDI_H_

#include "SwitchBar.h"
#include "SwitchBarCanvas.h"

#ifndef NATIVE_MDI
	#ifdef __WXMSW__
		#define NATIVE_MDI 1
	#else
		#define NATIVE_MDI 0
	#endif
#endif

#if NATIVE_MDI

	#include "SwitchBarChild.h"
	#include "SwitchBarParent.h"

#else

	#include "SwitchBarChildGeneric.h"
	#include "SwitchBarParentGeneric.h"

#endif

#endif
