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

#ifndef QLOCALSERVER_P_H
#define QLOCALSERVER_P_H

#ifndef QT_NO_LOCALSERVER

#include "qlocalserver.h"
#include "private/qobject_p.h"

#ifdef Q_OS_WIN
#include <qt_windows.h>
#include <qthread.h>
#include <qqueue.h>

class QLocalServerThread : public QThread
{
    Q_OBJECT

Q_SIGNALS:
    void connected(int newSocket);
    void error();

public:
    QLocalServerThread(QObject *parent = 0);
    ~QLocalServerThread();

public:
    void setName(const QString &key);
    void run();
    void makeHandle();

    QQueue<HANDLE> handles;
    int maxPendingConnections;
    // current handle the thread is looking after
    HANDLE handle;
private:
    QString fullName;
    QString key;

};
#else
#include <QSocketNotifier>
#include "private/qnativesocketengine_p.h"
#endif

class QLocalServerPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QLocalServer)

public:
    QLocalServerPrivate() :
#ifdef Q_OS_WIN
            waiting(false),
#else
            listenSocket(-1), socketNotifier(0),
#endif
            maxPendingConnections(30), error(QLocalServer::NoError)
    {
    }

    void init();
    bool listen(const QString &name);
    bool closeServer();
    void waitForNewConnection(int msec, bool *timedOut);

#ifdef Q_OS_WIN
    void _q_openSocket(int socket);
    void _q_stoppedListening();
    void _q_error();

    QLocalServerThread thread;
    bool waiting;
#else
    void setError(const QString &function);
    void _q_socketActivated();

    int listenSocket;
    QString serverNamePath;
    QSocketNotifier *socketNotifier;
#endif

    QString serverName;
    int maxPendingConnections;
    QList<QLocalSocket*> pendingConnections;
    QString errorString;
    QLocalServer::LocalServerError error;
};

#endif // QT_NO_LOCALSERVER

#endif // QLOCALSERVER_P_H

