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
