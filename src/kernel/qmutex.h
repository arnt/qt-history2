/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qmutex.h#9 $
**
** Definition of QMutex class
**
** Created : 961223
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QMUTEX_H
#define QMUTEX_H

#ifndef QT_H
#include "qglobal.h"
#include "qwindowdefs.h"
#endif // QT_H

#error "QMutexED_* is not defined anywhere."

#if defined(QMutexED_POSIX)
typedef int QMutexID;
#elif defined(QMutex_WIN32)
typedef HANDLE QMutexID;
#endif


class Q_EXPORT QMutex
{
public:
    QMutex( bool lock = FALSE );
    QMutex( const QMutex & ) {} // ### is this right?
   ~QMutex();

    QMutex &operator=( const QMutex & );

    void lock();
    void unlock();
    bool tryLock();

private:
    QMutexID	mutex;
};


#endif // QMUTEX_H
