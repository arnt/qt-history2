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

#ifndef QLOCALSOCKET_P_H
#define QLOCALSOCKET_P_H

#ifndef QT_NO_LOCALSOCKET

#include "qlocalsocket.h"
#include "private/qiodevice_p.h"

#ifdef Q_OS_WIN
#include <qt_windows.h>
#include "private/qwindowspipewriter_p.h"
#include <qtimer.h>
#else
#include "private/qnativesocketengine_p.h"
#include <qtcpsocket.h>
#include <qsocketnotifier.h>
#include <errno.h>

static inline int qSocket(int af, int socketype, int proto)
{
    int ret;
    while((ret = qt_socket_socket(af, socketype, proto)) == -1 && errno == EINTR){}
    return ret;
}

static inline int qBind(int fd, const sockaddr *sa, int len)
{
    int ret;
    while((ret = QT_SOCKET_BIND(fd, sa, len)) == -1 && errno == EINTR){}
    return ret;
}

static inline int qConnect(int fd, const sockaddr *sa, int len)
{
    int ret;
    while((ret = QT_SOCKET_CONNECT(fd, sa, len)) == -1 && errno == EINTR){}
    return ret;
}

static inline int qListen(int fd, int backlog)
{
    int ret;
    while((ret = qt_socket_listen(fd, backlog)) == -1 && errno == EINTR){}
    return ret;
}

static inline int qAccept(int fd, struct sockaddr *addr, QT_SOCKLEN_T *addrlen)
{
    int ret;
    while((ret = qt_socket_accept(fd, addr, addrlen)) == -1 && errno == EINTR){}
    return ret;
}

class QLocalUnixSocket : public QTcpSocket
{

public:
    QLocalUnixSocket() : QTcpSocket()
    {
    };

    void setSocketState(QAbstractSocket::SocketState state)
    {
        QAbstractSocket::setSocketState(state);
    };

    void setErrorString(const QString &string)
    {
        QAbstractSocket::setErrorString(string);
    }

    void setSocketError(QAbstractSocket::SocketError error)
    {
        QAbstractSocket::setSocketError(error);
    }

    qint64 readData(char *data, qint64 maxSize)
    {
        return QAbstractSocket::readData(data, maxSize);
    }

    qint64 writeData(const char *data, qint64 maxSize)
    {
        return QAbstractSocket::writeData(data, maxSize);
    }
};
#endif


class QLocalSocketPrivate : public QIODevicePrivate
{
    Q_DECLARE_PUBLIC(QLocalSocket)

public:
    QLocalSocketPrivate();
    void init();

#ifdef Q_OS_WIN
    void setErrorString(const QString &function);
    void _q_notified();
    QLocalSocket::LocalSocketError error;
    QTimer notifier;
    HANDLE handle;
    QWindowsPipeWriter *pipeWriter;
    bool readyRead;
#else
    QLocalUnixSocket unixSocket;
    void errorOccured(QLocalSocket::LocalSocketError, const QString &function);
    void _q_stateChanged(QAbstractSocket::SocketState newState);
    void _q_connectToSocket();
    QSocketNotifier *delayConnect;

    int connectingSocket;
    QString connectingName;
    QIODevice::OpenMode connectingOpenMode;
#endif

    QString peerName;
    QLocalSocket::LocalSocketState state;
};

#endif // QT_NO_LOCALSOCKET

#endif // QLOCALSOCKET_P_H

