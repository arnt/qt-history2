/****************************************************************************
** $Id: //depot/qt/main/src/tools/qgeneric.h#9 $
**
** Macros for pasting tokens; utilized by our generic classes
**
** Created : 920529
**
** Copyright (C) 1992-1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QGENERIC_H
#define QGENERIC_H

#include "qglobal.h"

#if defined(_CC_MPW_)
#include <generic.h>		/* MPW C++ has a non-standard preprocessor */
#else

#if defined(_OS_LINUX_)
#include <g++/generic.h>
#elsiif defined(UNIX)		// TODO what Windows compilers include it?
#include <generic.h>
#endif

#define USE_STDC_MACROS		/* comment out this line for K&R style cpp */

#if defined(USE_STDC_MACROS)

// Standard token-pasting macros for ANSI C preprocessors

#if !defined(name2)
#define name2(a,b)		_name2_aux(a,b)
#endif
#if !defined(_name2_aux)
#define _name2_aux(a,b)		a##b
#endif
#if !defined(name3)
#define name3(a,b,c)		_name3_aux(a,b,c)
#endif
#if !defined(_name3_aux)
#define _name3_aux(a,b,c)	a##b##c
#endif
#if !defined(name4)
#define name4(a,b,c,d)		_name4_aux(a,b,c,d)
#endif
#if !defined(_name4_aux)
#define _name4_aux(a,b,c,d)	a##b##c##d
#endif

#else

// Token-pasting macros for outdated K&R C preprocessors

#if !defined(name2)
#define name2(a,b)		a/**/b
#endif
#if !defined(name3)
#define name3(a,b,c)		a/**/b/**/c
#endif
#if !defined(name4)
#define name4(a,b,c,d)		a/**/b/**/c/**/d
#endif

#endif

#if !defined(declare)
#define declare(a,t)		name2(a,declare)(t)
#endif
#if !defined(implement)
#define implement(a,t)		name2(a,implement)(t)
#endif
#if !defined(declare2)
#define declare2(a,t1,t2)	name2(a,declare2)(t1,t2)
#endif
#if !defined(implement2)
#define implement2(a,t1,t2)	name2(a,implement2)(t1,t2)
#endif

#endif

#endif // QGENERIC_H

