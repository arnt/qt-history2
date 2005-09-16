/****************************************************************************
**
** Copyright (C) 2004-2005 Trolltech AS. All rights reserved.
**
** This file is part of the example classes of the Qt Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#include "connectionmanager.h"
#include "peerwireclient.h"
#include "ratecontroller.h"
#include "torrentclient.h"
#include "torrentserver.h"

Q_GLOBAL_STATIC(TorrentServer, torrentServer)

TorrentServer *TorrentServer::instance()
{
    return torrentServer();
}

void TorrentServer::addClient(TorrentClient *client)
{
    clients << client;
}

void TorrentServer::removeClient(TorrentClient *client)
{
    clients.removeAll(client);
}

void TorrentServer::incomingConnection(int socketDescriptor)
{
    PeerWireClient *client = new PeerWireClient(ConnectionManager::instance()->clientId(), this);
    if (clients.isEmpty())
        client->abort();

    if (ConnectionManager::instance()->canAddConnection()) {
        if (client->setSocketDescriptor(socketDescriptor)) {
            connect(client, SIGNAL(infoHashReceived(const QByteArray &)),
                    this, SLOT(processInfoHash(const QByteArray &)));
            connect(client, SIGNAL(error(QAbstractSocket::SocketError)),
                    this, SLOT(removeClient()));
            RateController::instance()->addSocket(client);
            ConnectionManager::instance()->addConnection(client);
            if (clients.size() == 1) {
                client->disconnect(client, 0, this, 0);
                clients.first()->setupIncomingConnection(client);
            }
            return;
        }
    }

    delete client;
}

void TorrentServer::removeClient()
{
    PeerWireClient *peer = qobject_cast<PeerWireClient *>(sender());
    peer->deleteLater();
    RateController::instance()->removeSocket(peer);
    ConnectionManager::instance()->removeConnection(peer);
}

void TorrentServer::processInfoHash(const QByteArray &infoHash)
{
    PeerWireClient *peer = qobject_cast<PeerWireClient *>(sender());
    foreach (TorrentClient *client, clients) {
        if (client->state() >= TorrentClient::Searching && client->infoHash() == infoHash) {
            peer->disconnect(peer, 0, this, 0);
            client->setupIncomingConnection(peer);
            return;
        }
    }
    removeClient();
}
