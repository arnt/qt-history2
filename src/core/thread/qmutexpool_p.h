/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
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

#ifndef QT_NO_THREAD

class Q_CORE_EXPORT QMutexPool
{
public:
    QMutexPool(bool recursive = false, int size = 128);
    ~QMutexPool();

    QMutex *get(const void *address);

private:
    QMutex mutex;
    QMutex **mutexes;
    int count;
    bool recurs;
};

Q_CORE_EXPORT QMutexPool *qt_global_mutexpool_func();
#define qt_global_mutexpool qt_global_mutexpool_func()

#else // QT_NO_THREAD

class QMutexPool
{
public:
    inline QMutexPool(bool = false, int = 128) {}
    ~QMutexPool() {}

    static QMutex *get(const void *) { return 0; }
};

inline Q_CORE_EXPORT QMutexPool *qt_global_mutexpool_func() { return 0; }
#define qt_global_mutexpool qt_global_mutexpool_func()

#endif // QT_NO_THREAD

#endif // QMUTEXPOOL_P_H
