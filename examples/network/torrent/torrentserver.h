/****************************************************************************
**
** Copyright (C) 2004-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef TORRENTSERVER_H
#define TORRENTSERVER_H

#include <QList>
#include <QTcpServer>

class TorrentClient;

class TorrentServer : public QTcpServer
{
    Q_OBJECT

public:
    inline TorrentServer() {}
    static TorrentServer *instance();

    void addClient(TorrentClient *client);
    void removeClient(TorrentClient *client);

protected:
    void incomingConnection(int socketDescriptor);

private slots:
    void removeClient();
    void processInfoHash(const QByteArray &infoHash);

private:
    QList<TorrentClient *> clients;
};

#endif
