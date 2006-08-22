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

#include "peermanager.h"
#include "connection.h"
#include "client.h"

#include <QtNetwork>

static const qint32 BroadcastInterval = 2000;
static const unsigned broadcastPort = 45000;

PeerManager::PeerManager(Client *client)
    : QObject(client), client(client)
{
    QStringList envVariables;
    envVariables << "USERNAME.*" << "USER.*" << "USERDOMAIN.*"
                 << "HOSTNAME.*" << "DOMAINNAME.*";

    QStringList environment = QProcess::systemEnvironment();
    foreach (QString string, envVariables) {
        int index = environment.indexOf(QRegExp(string));
        if (index != -1) {
            QStringList stringList = environment.at(index).split("=");
            if (stringList.size() == 2) {
                username = stringList.at(1).toUtf8();
                break;
            }
        }
    }

    if (username.isEmpty())
        username = "unknown";

    updateAddresses();
    serverPort = 0;

    broadcastSocket.bind(QHostAddress::Any, broadcastPort);
    connect(&broadcastSocket, SIGNAL(readyRead()), this, SLOT(readBroadcastDatagram()));

    broadcastTimer.setInterval(BroadcastInterval);
    connect(&broadcastTimer, SIGNAL(timeout()), this, SLOT(sendBroadcastDatagram()));
}

void PeerManager::setServerPort(int port)
{
    serverPort = port;
}

QByteArray PeerManager::userName() const
{
    return username;
}

void PeerManager::startBroadcasting()
{
    broadcastTimer.start();
}

void PeerManager::sendBroadcastDatagram()
{
    QByteArray datagram(username);
    datagram.append(QLatin1Char('@').toLatin1());
    datagram.append(QByteArray::number(serverPort));

    bool validBroadcastAddresses = true;
    foreach (QHostAddress ba, broadcastAddresses) {
        if (broadcastSocket.writeDatagram(datagram, ba, broadcastPort) == -1)
            validBroadcastAddresses = false;
    }

    if (!validBroadcastAddresses)
        updateAddresses();
}

void PeerManager::readBroadcastDatagram()
{
    while (broadcastSocket.hasPendingDatagrams()) {
        QHostAddress senderIp;
        quint16 senderPort;
        QByteArray datagram;
        datagram.resize(broadcastSocket.pendingDatagramSize());
        if (broadcastSocket.readDatagram(datagram.data(), datagram.size(), &senderIp, &senderPort) == -1)
            continue;
        bool isPackageFromMe = false;
        foreach (QHostAddress ip, ipAddresses) {
            if (ip == senderIp) {
                isPackageFromMe = true;
                break;
            }
        }

        if (!isPackageFromMe && !client->hasConnection(senderIp)) {
            QList<QByteArray> list = datagram.split(QLatin1Char('@').toLatin1());
            if (list.size() != 2)
                continue; // not a valid package (user@port)

            Connection *connection = new Connection(this);
            connection->setName(list.at(0));
            emit newConnection(connection);
            connection->connectToHost(senderIp, list.at(1).toInt());
        }
    }
}

void PeerManager::updateAddresses()
{
    broadcastAddresses.clear();
    ipAddresses.clear();
    foreach (QNetworkInterface networkInterface, QNetworkInterface::allInterfaces()) {
        foreach (QNetworkAddressEntry addressEntry, networkInterface.addressEntries()) {
            QHostAddress broadcastAddress = addressEntry.broadcast();
            if (broadcastAddress != QHostAddress::Null) {
                broadcastAddresses << broadcastAddress;
                ipAddresses << addressEntry.ip();
            }
        }
    }
}
