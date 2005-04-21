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

#ifndef QTCPSERVER_H
#define QTCPSERVER_H

#include <QtCore/qobject.h>
#include <QtNetwork/qabstractsocket.h>
#include <QtNetwork/qhostaddress.h>

class QTcpServerPrivate;
class QTcpSocket;

class Q_NETWORK_EXPORT QTcpServer : public QObject
{
    Q_OBJECT
public:
    explicit QTcpServer(QObject *parent = 0);
    virtual ~QTcpServer();

    bool listen(const QHostAddress &address = QHostAddress::Any, quint16 port = 0);
    void close();

    bool isListening() const;

    void setMaxPendingConnections(int numConnections);
    int maxPendingConnections() const;

    quint16 serverPort() const;
    QHostAddress serverAddress() const;

    int socketDescriptor() const;
    bool setSocketDescriptor(int socketDescriptor);

    bool waitForNewConnection(int msec = 0, bool *timedOut = 0);
    virtual bool hasPendingConnections() const;
    virtual QTcpSocket *nextPendingConnection();

    QAbstractSocket::SocketError serverError() const;
    QString errorString() const;

protected:
    virtual void incomingConnection(int handle);

signals:
    void newConnection();

private:
    Q_PRIVATE_SLOT(d_func(), void processIncomingConnection(int))
    Q_DISABLE_COPY(QTcpServer)
    Q_DECLARE_PRIVATE(QTcpServer)
};

#endif // QTCPSERVER_H
