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

#ifndef QVFBSOCKET_H
#define QVFBSOCKET_H

#include <QTcpSocket>
#include <QTcpServer>
#include <QMutex>

class QVFbSocket : public QTcpSocket
{
    Q_OBJECT
public:
    explicit QVFbSocket(QObject *parent=0);
    ~QVFbSocket();

    bool connectToLocalFile(const QString &file);

Q_SIGNALS:
    void connected();
    void disconnected();
    void error(QAbstractSocket::SocketError);

private:
    Q_DISABLE_COPY(QVFbSocket)
};

class QVFbServerSocket : public QTcpServer
{
    Q_OBJECT
public:
    QVFbServerSocket(const QString& file, QObject *parent=0);
    ~QVFbServerSocket();

    QVFbSocket *nextPendingConnection();
Q_SIGNALS:
    void newConnection();
protected:
    void incomingConnection(int socketDescriptor);
private:
    QMutex ssmx;
    QList<int> inboundConnections;
    Q_DISABLE_COPY(QVFbServerSocket)

    void init(const QString &file);
};

#endif // QVFBSOCKET_H
