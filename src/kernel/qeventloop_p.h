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

#include "qwindowdefs.h"

class QSocketNotifier;
#ifdef Q_OS_MAC
class QMacSockNotPrivate;
#endif

#if defined(Q_OS_UNIX) || defined (Q_WS_WIN)
#include "list.h"
#endif // Q_OS_UNIX || Q_WS_WIN

#include "qobject_p.h"

#if defined(Q_OS_UNIX)
struct QSockNot
{
    QSocketNotifier *obj;
    int fd;
    fd_set *queue;
#ifdef Q_OS_MAC
    ~QSockNot();
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
#endif // Q_OS_UNIX

#if defined(Q_WS_WIN)
struct QSockNot {
    QSocketNotifier *obj;
    int fd;
};
#endif // Q_WS_WIN

class QEventLoopPrivate : public QObjectPrivate
{
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

#if defined(Q_WS_MAC)
    EventLoopTimerRef select_timer;
#endif

#if defined(Q_WS_X11)
    int xfd;
#endif // Q_WS_X11

#if defined(Q_OS_UNIX)
    int thread_pipe[2];

    // pending socket notifiers list
    QList<QSockNot *> sn_pending_list;
    // highest fd for all socket notifiers
    int sn_highest;
    // 3 socket notifier types - read, write and exception
    QSockNotType sn_vec[3];
#endif

#ifdef Q_WS_WIN
    // pending socket notifiers list
    QList<QSockNot *> sn_pending_list;
#endif // Q_WS_WIN

};

#endif // QEVENTLOOP_P_H
