/****************************************************************************
**
** Copyright (C) 2004-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "connectionmanager.h"

#include <QByteArray>
#include <QDateTime>

static const char ClientID[] = "-QT-"QT_VERSION_STR"-";
static const int MaxConnections = 250;

Q_GLOBAL_STATIC(ConnectionManager, connectionManager)

ConnectionManager *ConnectionManager::instance()
{
    return connectionManager();
}

bool ConnectionManager::canAddConnection() const
{
    return (connections.size() < MaxConnections);
}

void ConnectionManager::addConnection(PeerWireClient *client)
{
    connections << client;
}

void ConnectionManager::removeConnection(PeerWireClient *client)
{
    connections.remove(client);
}

int ConnectionManager::maxConnections() const
{
    return MaxConnections;
}

QByteArray ConnectionManager::clientId() const
{
    if (id.isEmpty()) {
        // Generate peer id
        int startupTime = int(QDateTime::currentDateTime().toTime_t());
        id = ClientID + QByteArray::number(startupTime, 16);
        id += QByteArray(20 - id.size(), '-');
    }
    return id;
}
