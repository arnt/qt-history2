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
#include "qwindowdefs.h"
#include "qptrlist.h"

class QSocketNotifier;
#ifdef Q_OS_MAC
class QMacSockNotPrivate;
#endif

#if defined(Q_OS_UNIX) || defined (Q_WS_WIN)
#include "qlist.h"
#endif // Q_OS_UNIX || Q_WS_WIN

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

    QList<QSockNot *> list;
    fd_set select_fds;
    fd_set enabled_fds;
    fd_set pending_fds;

};

struct TimerInfo {				// internal timer info
    int	     id;				// - timer identifier
    timeval  interval;				// - timer interval
    timeval  timeout;				// - when to sent event
    QObject *obj;				// - object to receive event
};
typedef QPtrList<TimerInfo> TimerList;	// list of TimerInfo structs
class QBitArray;

#endif // Q_OS_UNIX

#if defined(Q_WS_WIN)
#include <qptrvector.h>
#include <qintdict.h>
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
typedef QPtrVector<TimerInfo>  TimerVec;		// vector of TimerInfo structs
typedef QIntDict<TimerInfo> TimerDict;		// fast dict of timers

#endif // Q_WS_WIN

class QEventLoopPrivate : public QObjectPrivate
{
    Q_DECL_PUBLIC(QEventLoop);
public:
    QEventLoopPrivate()
	: QObjectPrivate()
    {
	reset();
    }

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

#if defined(Q_OS_UNIX)
    int eventloopSelect(uint, timeval *);
    int thread_pipe[2];

    // pending socket notifiers list
    QList<QSockNot *> sn_pending_list;
    // highest fd for all socket notifiers
    int sn_highest;
    // 3 socket notifier types - read, write and exception
    QSockNotType sn_vec[3];

    QBitArray *timerBitVec;			// timer bit vector
    TimerList *timerList;	  	        // timer list
    timeval *timerWait();
    void     timerInsert(const TimerInfo *);
    void     timerRepair(const timeval &);

#endif

#ifdef Q_WS_WIN
    // pending socket notifiers list
    QList<QSockNot *> sn_pending_list;
    static TimerVec  *timerVec;		// timer vector
    static TimerDict *timerDict;	// timer dict
#endif // Q_WS_WIN

};

#endif // QEVENTLOOP_P_H
