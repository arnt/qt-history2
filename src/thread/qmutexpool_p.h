/****************************************************************************
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QMUTEXPOOL_P_H
#define QMUTEXPOOL_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of QSettings. This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//
//

#ifndef QT_H
#include "qmutex.h"
#endif // QT_H
#ifdef QT_THREAD_SUPPORT

class Q_CORE_EXPORT QMutexPool
{
public:
    QMutexPool( bool recursive = false, int size = 128 );
    ~QMutexPool();

    QMutex *get( void *address );

private:
    QMutex mutex;
    QMutex **mutexes;
    int count;
    bool recurs;
};

Q_CORE_EXPORT QMutexPool *qt_global_mutexpool_func();
#define qt_global_mutexpool qt_global_mutexpool_func()

#endif // QT_THREAD_SUPPORT
#endif // QMUTEXPOOL_P_H
