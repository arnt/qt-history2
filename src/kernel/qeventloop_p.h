#ifndef QEVENTLOOP_P_H
#define QEVENTLOOP_P_H

#if defined(QT_THREAD_SUPPORT)
#include "qmutex.h"
#endif // QT_THREAD_SUPPORT

class QEventLoopPrivate
{
public:
    QEventLoopPrivate()
#if defined(QT_THREAD_SUPPORT)
	: mutex( TRUE )
#endif // QT_THREAD_SUPPORT
    {
	reset();
    }

    void reset() {
	looplevel = 0;
	quitcode = 0;
	quitnow = FALSE;
	exitloop = FALSE;
    }

    int looplevel;
    int quitcode;
    bool quitnow;
    bool exitloop;

#if defined(QT_THREAD_SUPPORT)
    QMutex mutex;
#endif // QT_THREAD_SUPPORT

#if defined(Q_WS_X11)
    int xfd;
#endif // Q_WS_X11

#if defined(Q_OS_UNIX)
    int thread_pipe[2];
#endif
};

#endif // QEVENTLOOP_P_H
