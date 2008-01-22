/*
 *  DirtJNI.h
 *  DirtJNI
 *
 *  Created by Jason Weathered on 2008-01-22.
 *  Copyright 2008 General Software Laboratories. All rights reserved.
 *
 */

//#if defined (__ppc64__) || defined (__x86_64__)
#if !defined(__ppc__) && !defined(__i386__)
#define __USE_LEOPARD_API
#endif

#if defined(__USE_LEOPARD_API)
void Dock_setContentImage(CGImageRef imageRef);
#endif
