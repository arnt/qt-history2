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
#include <qobject.h>
#include <qabstractsocket.h>
#include <qhostaddress.h>

#if defined(QT_LICENSE_PROFESSIONAL)
#define QM_EXPORT_NETWORK
#else
#define QM_EXPORT_NETWORK Q_NETWORK_EXPORT
#endif

class QTcpServerPrivate;
class QTcpSocket;

class QM_EXPORT_NETWORK QTcpServer : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QTcpServer)
public:
    QTcpServer(QObject *parent = 0);
    virtual ~QTcpServer();

    bool listen(const QHostAddress &address, Q_UINT16 port);
    bool listen(Q_UINT16 port = 0);
    void close();

    bool isListening() const;

    void setMaxPendingConnections(int numConnections);
    int maxPendingConnections() const;

    Q_UINT16 serverPort() const;
    QHostAddress serverAddress() const;

    int socketDescriptor() const;
    bool setSocketDescriptor(int socketDescriptor);

    bool waitForConnection(int msec = 0, bool *timedOut = 0);
    virtual bool hasPendingConnection() const;
    virtual QTcpSocket *nextPendingConnection();

    Qt::SocketError serverError() const;
    QString errorString() const;

protected:
    virtual void incomingConnection(int handle);

signals:
    void newConnection();

private:
    Q_PRIVATE_SLOT(d, void processIncomingConnection(int))
};

#endif
