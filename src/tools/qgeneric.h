/****************************************************************************
** $Id: //depot/qt/main/src/tools/qgeneric.h#13 $
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

#define Q_DECLARE(a,t)		a##declare(t)

#if defined(MOC_CONNECTIONLIST_DECLARED) && !defined(declare)
#define declare(a,t)		a##declare(t)
#endif

#endif // QGENERIC_H

