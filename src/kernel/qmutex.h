/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qmutex.h#3 $
**
** Definition of QMutex class
**
** Created : 961223
**
** Copyright (C) 1996-1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QMUTEX_H
#define QMUTEX_H

#include "qglobal.h"
#include "qwindefs.h"


#if defined(QMutexED_POSIX)
typedef int QMutexID;
#elif defined(QMutex_WIN32)
typedef HANDLE QMutexID;
#endif


class QMutex
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
