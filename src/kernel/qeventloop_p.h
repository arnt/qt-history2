/****************************************************************************
**
** Definition of QEventLoop class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QEVENTLOOP_P_H
#define QEVENTLOOP_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  This header file may
// change from version to version without notice, or even be
// removed.
//
// We mean it.
//
//

#ifndef QT_H
#include "qplatformdefs.h"
#endif // QT_H

// SCO OpenServer redefines raise -> kill
#if defined(raise)
# undef raise
#endif

#include "qeventloop.h"

class QSocketNotifier;
#ifdef Q_OS_MAC
class QMacSockNotPrivate;
#endif

#include <qlist.h>

#include "qobject_p.h"

#if defined(Q_OS_UNIX)
#include <unistd.h>
struct QSockNot
{
    QSocketNotifier *obj;
    int fd;
    fd_set *queue;
#ifdef Q_OS_MAC
    QMacSockNotPrivate *mac_d;
#endif
};

class QSockNotType
{
public:
    QSockNotType();

    QList<QSockNot*> list;
    fd_set select_fds;
    fd_set enabled_fds;
    fd_set pending_fds;

};

// internal timer info
struct QTimerInfo;

// list of TimerInfo structs
typedef QList<QTimerInfo*> QTimerList;

class QBitArray;

#endif // Q_OS_UNIX

#if defined(Q_WS_WIN)
#include <qhash.h>
struct QSockNot {
    QSocketNotifier *obj;
    int fd;
};

struct TimerInfo {				// internal timer info
    uint     ind;				// - Qt timer identifier - 1
    uint     id;				// - Windows timer identifier
    bool     zero;				// - zero timing
    QObject *obj;				// - object to receive events
};
typedef QList<TimerInfo*>  TimerVec;		// vector of TimerInfo structs
typedef QHash<int,TimerInfo*> TimerDict;		// fast dict of timers

#endif // Q_WS_WIN

class Q_CORE_EXPORT QEventLoopPrivate : public QObjectPrivate
{
    Q_DECL_PUBLIC(QEventLoop);
public:
    QEventLoopPrivate();

    inline void reset() {
	looplevel = 0;
	quitcode = 0;
	quitnow = FALSE;
	exitloop = FALSE;
	shortcut = FALSE;
    }

    int looplevel;
    int quitcode;
    unsigned int quitnow  : 1;
    unsigned int exitloop : 1;
    unsigned int shortcut : 1;

#if defined(Q_WS_X11)
    int xfd;
#endif // Q_WS_X11

    // pending socket notifiers list
    QList<QSockNot*> sn_pending_list;

#if defined(Q_OS_UNIX)
    int eventloopSelect(uint, timeval *);
    int thread_pipe[2];

    // watch if time is turned back
    timeval watchtime;

    // highest fd for all socket notifiers
    int sn_highest;
    // 3 socket notifier types - read, write and exception
    QSockNotType sn_vec[3];

    QBitArray *timerBitVec;
    QTimerList *timerList;
    bool timerWait(timeval &);
    void timerInsert(QTimerInfo *);
    void timerRepair(const timeval &);
#endif

    friend class QCoreApplication;
};

#endif // QEVENTLOOP_P_H
