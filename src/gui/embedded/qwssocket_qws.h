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

#ifndef QWSSOCKET_QWS_H
#define QWSSOCKET_QWS_H

#include <Qt/qconfig.h>

#ifndef QT_NO_SXV
#define QWS_SOCK_BASE QUnixSocket
#include "qunixsocket.h"
#include "qunixsocketserver.h"
#include <QMutex>
#else
#define QWS_SOCK_BASE QTcpSocket
#include <QtNetwork/qtcpsocket.h>
#include <QtNetwork/qtcpserver.h>
#endif

QT_MODULE(Gui)

#ifndef QT_NO_QWS_MULTIPROCESS

#ifndef QT_NO_SXV
class QWSSocket : public QUnixSocket
#else
class QWSSocket : public QTcpSocket
#endif
{
    Q_OBJECT
public:
    explicit QWSSocket(QObject *parent=0);
    ~QWSSocket();

    bool connectToLocalFile(const QString &file);

#ifndef QT_NO_SXV
    QString errorString();
Q_SIGNALS:
    void connected();
    void disconnected();
    void error(QAbstractSocket::SocketError);
private Q_SLOTS:
    void forwardStateChange(SocketState);
#endif

private:
    Q_DISABLE_COPY(QWSSocket)
};


#ifndef QT_NO_SXV
class QWSServerSocket : public QUnixSocketServer
#else
class QWSServerSocket : public QTcpServer
#endif
{
    Q_OBJECT
public:
    QWSServerSocket(const QString& file, QObject *parent=0);
    ~QWSServerSocket();

#ifndef QT_NO_SXV
    QWSSocket *nextPendingConnection();
Q_SIGNALS:
    void newConnection();
protected:
    void incomingConnection(int socketDescriptor);
private:
    QMutex ssmx;
    QList<int> inboundConnections;
#endif

private:
    Q_DISABLE_COPY(QWSServerSocket)

    void init(const QString &file);
};

#endif // QT_NO_QWS_MULTIPROCESS

#endif // QWSSOCKET_QWS_H
