/****************************************************************************
** $Id: //depot/qt/main/src/tools/qgeneric.h#17 $
**
** Macros for pasting tokens; utilized by our generic classes
**
** Created : 920529
**
** Copyright (C) 1992-1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QGENERIC_H
#define QGENERIC_H

#include "qglobal.h"


// Try to include the system defines where it is sure to exist
#if defined(_CC_SUN_) || (defined(_CC_EDG_) && defined(_OS_IRIX_))
#include <generic.h>
#endif


// Define Qt generic macros
// At some time in the future, these will be the only #defines left here

#if defined(QT_ADD_GENERIC_MACROS)

#define Q_NAME2(a,b)		Q_NAME2_AUX(a,b)
#define Q_NAME2_AUX(a,b)	a##b
#define Q_DECLARE(a,t)		Q_NAME2(a,declare)(t)

#endif // QT_ADD_GENERIC_MACROS


// Standard token-pasting macros for ANSI C preprocessors
// We will remove these from Qt in version 2.0 or 3.0

#if !defined(declare)

#define name2(a,b)		_name2_aux(a,b)
#define _name2_aux(a,b)		a##b
#define name3(a,b,c)		_name3_aux(a,b,c)
#define _name3_aux(a,b,c)	a##b##c
#define name4(a,b,c,d)		_name4_aux(a,b,c,d)
#define _name4_aux(a,b,c,d)	a##b##c##d

#define declare(a,t)		name2(a,declare)(t)
#define implement(a,t)		name2(a,implement)(t)
#define declare2(a,t1,t2)	name2(a,declare2)(t1,t2)
#define implement2(a,t1,t2)	name2(a,implement2)(t1,t2)

#endif // !declare


#endif // QGENERIC_H
