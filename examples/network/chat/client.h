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

#ifndef CLIENT_H
#define CLIENT_H

#include <QtCore/QObject>
#include <QtCore/QHash>
#include <QtNetwork/QAbstractSocket>
#include <QtNetwork/QTcpServer>
#include <QtNetwork/QHostAddress>

class PeerManager;
class Connection;

class Server : public QTcpServer
{
    Q_OBJECT
public:
    Server(QObject *parent = 0);

signals:
    void newConnection(Connection *);

protected:
    void incomingConnection(int socketDescriptor);
};

class Client : public QObject
{
    Q_OBJECT
public:
    Client();

    void sendMessage(const QString &message);
    QString nickName() const;
    bool hasConnection(const QHostAddress &senderIp) const;

signals:
    void newMessage(const QString &from, const QString &message);
    void newParticipant(const QString &nick);
    void participantLeft(const QString &nick);

private slots:
    void newConnection(Connection *connection);
    void connectionError(QAbstractSocket::SocketError socketError);
    void disconnected();
    void readyForUse();

private:
    PeerManager *peerManager;
    Server server;
    QHash<QHostAddress, Connection *> peers;

    void removeConnection(Connection *connection);
};

#endif // CLIENT_H

