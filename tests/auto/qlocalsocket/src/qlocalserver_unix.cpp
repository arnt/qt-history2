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

#include "qlocalserver.h"
#include "qlocalserver_p.h"
#include "qlocalsocket_p.h"
#include "qlocalsocket.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <qdebug.h>
#include <qfile.h>
#include <qdir.h>
#include <qdatetime.h>

void QLocalServerPrivate::setError(const QString &function)
{
    if (errno == EAGAIN)
        return;

    closeServer();
    switch (errno) {
    case EACCES:
        errorString = QLocalServer::tr("%1: Permission denied").arg(function);
        error = QLocalServer::PermissionDeniedError;
        break;
    case ELOOP:
    case ENOENT:
    case ENAMETOOLONG:
    case EROFS:
    case ENOTDIR:
        errorString = QLocalServer::tr("%1: Name error").arg(function);
        error = QLocalServer::NameError;
        break;
    case EADDRINUSE:
        errorString = QLocalServer::tr("%1: Address in use").arg(function);
        error = QLocalServer::AddressInUseError;
        break;

    default:
        errorString = QLocalServer::tr("%1: Unknown error %2")
                      .arg(function).arg(errno);
        error = QLocalServer::UnknownError;
#if defined QLOCALSERVER_DEBUG
        qWarning() << errorString << "serverNamePath:" << serverNamePath;
#endif
    }
}

void QLocalServerPrivate::init()
{
}

bool QLocalServerPrivate::closeServer()
{
    bool success = true;

    if (-1 == listenSocket)
        success = false;
    else
        QT_CLOSE(listenSocket);
    listenSocket = -1;

    if (socketNotifier)
        socketNotifier->deleteLater();
    socketNotifier = 0;

    success |= QFile::remove(serverNamePath);
    serverNamePath = QString();
    errorString = QString();
    error = QLocalServer::NoError;

    return success;
}

bool QLocalServerPrivate::listen(const QString &serverName)
{
    Q_Q(QLocalServer);

    // create the unix socket
    listenSocket = qSocket(PF_UNIX, SOCK_STREAM, 0);
    if (listenSocket == -1) {
        setError(QLatin1String("QLocalServer::listen"));
        return false;
    }

    // Construct our unix address
    serverNamePath = QDir::tempPath() + QLatin1Char('/') + serverName;
    struct ::sockaddr_un addr;
    addr.sun_family = PF_UNIX;
    ::memcpy(addr.sun_path, serverNamePath.toLatin1().data(),
             serverNamePath.toLatin1().size() + 1);
    unlink(addr.sun_path);

    // bind
    if(-1 == qBind(listenSocket, (sockaddr *)&addr, sizeof(sockaddr_un))) {
        setError(QLatin1String("QLocalServer::listen"));
        QT_CLOSE(listenSocket);
        listenSocket = -1;
        serverNamePath = QString();
        return false;
    }

    // listen for connections
    if (qListen(listenSocket, 10) == -1) {
        setError(QLatin1String("QLocalServer::listen"));
        QT_CLOSE(listenSocket);
        listenSocket = -1;
        serverNamePath = QString();
        return false;
    }
    if (!socketNotifier) {
        socketNotifier = new QSocketNotifier(listenSocket, QSocketNotifier::Read, q);
        q->connect(socketNotifier, SIGNAL(activated(int)),
                   q, SLOT(_q_socketActivated()));
    }
    socketNotifier->setEnabled(maxPendingConnections > 0);
    return true;
}

/*!
    \internal
    We have received a notification that we can read off the listen socket
    accept the new socket.
 */
void QLocalServerPrivate::_q_socketActivated()
{
    Q_Q(QLocalServer);
    if (-1 == listenSocket)
        return;

    ::sockaddr_un addr;
    QT_SOCKLEN_T length = sizeof(sockaddr_un);
    int connectedSocket = qAccept(listenSocket, (sockaddr *) &addr, &length);
    if(-1 == connectedSocket) {
        setError(QLatin1String("QLocalSocket::listen"));
    } else {
        socketNotifier->setEnabled(pendingConnections.size() <= maxPendingConnections);
        q->incomingConnection(connectedSocket);
    }
}

void QLocalServerPrivate::waitForNewConnection(int msec, bool *timedOut)
{
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(listenSocket, &readfds);

    timeval timeout;
    timeout.tv_sec = msec / 1000;
    timeout.tv_usec = (msec % 1000) * 1000;

    // timeout can't be 0 or else select will return an error
    if (msec == -1)
        timeout.tv_usec = 1000;

    int result = -1;
    // on Linux timeout will be updated by select, but _not_ on other systems
    QTime timer;
    timer.start();
    while (pendingConnections.isEmpty() && (msec == -1 || timer.elapsed() < msec)) {
        result = select(listenSocket + 1, &readfds, 0, 0, &timeout);
        if (-1 == result) {
            setError(QLatin1String("QLocalServer::waitForNewConnection"));
            break;
        }
        if (result > 0)
            _q_socketActivated();
    }
    if (timedOut)
        *timedOut = (result == 0);
}

