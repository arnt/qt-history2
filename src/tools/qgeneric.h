/****************************************************************************
** $Id: //depot/qt/main/src/tools/qgeneric.h#3 $
**
** Macros for pasting tokens; utilized by our generic classes
**
** Author  : Haavard Nord
** Created : 920529
**
** Copyright (C) 1992-1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QGENERIC_H
#define QGENERIC_H


#if defined(_CC_MPW_)

#include <generic.h>		/* MPW C++ has a non-standard preprocessor */

#else

#define USE_STDC_MACROS		/* comment out this line for K&R style cpp */

#if defined(USE_STDC_MACROS)

// Standard token-pasting macros for ANSI C preprocessors

#define name2(a,b)		_name2_aux(a,b)
#define _name2_aux(a,b)		a##b
#define name3(a,b,c)		_name3_aux(a,b,c)
#define _name3_aux(a,b,c)	a##b##c
#define name4(a,b,c,d)		_name4_aux(a,b,c,d)
#define _name4_aux(a,b,c,d)	a##b##c##d

#else 

// Token-pasting macros for outdated K&R C preprocessors

#define name2(a,b)		a/**/b
#define name3(a,b,c)		a/**/b/**/c
#define name4(a,b,c,d)		a/**/b/**/c/**/d

#endif

#define declare(a,t)		name2(a,declare)(t)
#define implement(a,t)		name2(a,implement)(t)
#define declare2(a,t1,t2)	name2(a,declare2)(t1,t2)
#define implement2(a,t1,t2)	name2(a,implement2)(t1,t2)

#endif

#endif // QGENERIC_H

