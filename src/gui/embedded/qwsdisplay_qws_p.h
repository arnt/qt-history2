/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QWSDISPLAY_QWS_P_H
#define QWSDISPLAY_QWS_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qwsdisplay_qws.h"
#include "qwssocket_qws.h"
#include "qwsevent_qws.h"
#include <private/qwssharedmemory_p.h>
#include "qwscommand_qws_p.h"
#include "qwslock_p.h"

class QWSDisplay::Data
{
public:
    Data(QObject* parent, bool singleProcess = false);
    ~Data();

    void flush();

    bool queueNotEmpty();
    QWSEvent *dequeue();
    QWSEvent *peek();

    bool directServerConnection();
    void fillQueue();
#ifndef QT_NO_QWS_MULTIPROCESS
    void connectToPipe();
    void waitForConnection();
    void waitForPropertyReply();
    void waitForRegionAck(int winId);
    void waitForRegionEvents(int winId);
#endif
    void waitForCreation();
#ifndef QT_NO_COP
    void waitForQCopResponse();
#endif
#if 0
    void offsetPendingExpose(int, const QPoint &);
    void translateExpose(QWSRegionModifiedEvent *re, const QPoint &p);
#endif
    void init();
    void reinit( const QString& newAppName );
    void create(int n = 1);

    void flushCommands();
    void sendCommand(QWSCommand & cmd);
    void sendSynchronousCommand(QWSCommand & cmd);

    QWSEvent *readMore();

    int takeId();

    void setMouseFilter(void (*filter)(QWSMouseEvent*));

    //####public data members

//    QWSRegionManager *rgnMan;
    uchar *sharedRam;
#ifndef QT_NO_QWS_MULTIPROCESS
    QWSSharedMemory shm;
#endif
    int sharedRamSize;

#ifndef QT_NO_QWS_MULTIPROCESS
    static QWSLock *clientLock;

    static bool lockClient(QWSLock::LockType, int timeout = -1);
    static void unlockClient(QWSLock::LockType);
    static bool waitClient(QWSLock::LockType, int timeout = -1);
    static QWSLock* getClientLock();
#endif // QT_NO_QWS_MULTIPROCESS

private:
#ifndef QT_NO_QWS_MULTIPROCESS
    QWSSocket *csocket;
#endif
    QList<QWSEvent*> queue;

#if 0
    void debugQueue() {
            for (int i = 0; i < queue.size(); ++i) {
                QWSEvent *e = queue.at(i);
                qDebug( "   ev %d type %d sl %d rl %d", i, e->type, e->simpleLen, e->rawLen);
            }
    }
#endif

    QWSConnectedEvent* connected_event;
    QWSMouseEvent* mouse_event;
    int mouse_state;
    int mouse_winid;
//    QWSRegionModifiedEvent *region_event;
//    QWSRegionModifiedEvent *region_ack;
    QPoint region_offset;
    int region_offset_window;
#ifndef QT_NO_COP
    QWSQCopMessageEvent *qcop_response;
#endif
    QWSEvent* current_event;
    QList<int> unused_identifiers;
#ifdef QAPPLICATION_EXTRA_DEBUG
    int mouse_event_count;
#endif
    void (*mouseFilter)(QWSMouseEvent *);

    enum { VariableEvent=-1 };

};

#endif // QWSDISPLAY_QWS_P_H
