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
#ifndef TRACKERCLIENT_H
#define TRACKERCLIENT_H

#include <QByteArray>
#include <QHostAddress>
#include <QHttp>
#include <QList>
#include <QObject>

#include "metainfo.h"
#include "torrentclient.h"

class TorrentClient;

class TrackerClient : public QObject
{
    Q_OBJECT
public:
    TrackerClient(TorrentClient *downloader, QObject *parent = 0);

    void start(const MetaInfo &info);
    void stop();

    QList<TorrentPeer> peerList() const;
    int leechCount() const;
    int seedCount() const;
    qint64 uploadCount() const;
    qint64 downloadCount() const;
    void setUploadCount(qint64 bytes);
    void setDownloadCount(qint64 bytes);

signals:
    void connectionError(QHttp::Error error);

    void failure(const QString &reason);
    void warning(const QString &message);
    void leechCountUpdated(int leechers);
    void seedCountUpdated(int seeders);
    void peerListUpdated(const QList<TorrentPeer> &peerList);

    void uploadCountUpdated(qint64 newUploadCount);
    void downloadCountUpdated(qint64 newDownloadCount);
    
    void stopped();

protected:
    void timerEvent(QTimerEvent *event);

private slots:
    void fetchPeerList();
    void httpRequestDone(bool error);
    void addUploaded(qint64 uploadedBytes);
    void addDownloaded(qint64 downloadedBytes);

private:
    TorrentClient *torrentDownloader;

    int requestInterval;
    int requestIntervalTimer;
    QHttp http;
    MetaInfo metaInfo;
    QByteArray trackerId;
    int seeders;
    int leechers;
    QList<TorrentPeer> peers;
    qint64 uploadedBytes;
    qint64 downloadedBytes;
    qint64 length;
    
    bool firstTrackerRequest;
    bool lastTrackerRequest;
};

#endif
