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

#ifndef PEERMANAGER_H
#define PEERMANAGER_H

#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QTimer>
#include <QtCore/QByteArray>
#include <QtNetwork/QUdpSocket>

class Connection;
class Client;

class PeerManager : public QObject
{
    Q_OBJECT
public:
    PeerManager(Client *client);

    void setServerPort(int port);
    QByteArray userName() const;
    void startBroadcasting();

signals:
    void newConnection(Connection *);

private slots:
    void sendBroadcastDatagram();
    void readBroadcastDatagram();

private:
    Client *client;
    QList<QHostAddress> broadcastAddresses;
    QList<QHostAddress> ipAddresses;
    QUdpSocket broadcastSocket;
    QTimer broadcastTimer;
    QByteArray username;
    int serverPort;

    void updateAddresses();
};

#endif // PEERMANAGER_H

