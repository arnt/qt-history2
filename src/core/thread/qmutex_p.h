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

#ifndef QMUTEX_P_H
#define QMUTEX_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of qmutex_unix.cpp and qmutex_win.cpp.  This header file may change
// from version to version without notice, or even be removed.
//
// We mean it.
//

#ifndef QT_H
#endif // QT_H

#ifdef Q_OS_UNIX
struct QMutexPrivate
{
    bool recursive;

    pthread_t owner;
    unsigned int count;

    pthread_mutex_t mutex;
    pthread_mutex_t mutex2;
};
#endif

#ifdef Q_OS_WIN32
class QMutexPrivate {
public:
    bool recursive;

    unsigned long owner;
    unsigned int count;
    QAtomic waiters;

    HANDLE event;
};
#endif

#endif // QMUTEX_P_H
