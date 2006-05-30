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

#ifndef TRACKERCLIENT_H
#define TRACKERCLIENT_H

#include <QByteArray>
#include <QList>
#include <QObject>
#include <QHostAddress>
#include <QHttp>

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

    inline qint64 uploadCount() const { return uploadedBytes; }
    inline qint64 downloadCount() const { return downloadedBytes; }
    inline void setUploadCount(qint64 bytes) { uploadedBytes = bytes; }
    inline void setDownloadCount(qint64 bytes) { downloadedBytes = bytes; }

signals:
    void connectionError(QHttp::Error error);

    void failure(const QString &reason);
    void warning(const QString &message);
    void peerListUpdated(const QList<TorrentPeer> &peerList);

    void uploadCountUpdated(qint64 newUploadCount);
    void downloadCountUpdated(qint64 newDownloadCount);
    
    void stopped();

protected:
    void timerEvent(QTimerEvent *event);

private slots:
    void fetchPeerList();
    void httpRequestDone(bool error);

private:
    TorrentClient *torrentDownloader;

    int requestInterval;
    int requestIntervalTimer;
    QHttp http;
    MetaInfo metaInfo;
    QByteArray trackerId;
    QList<TorrentPeer> peers;
    qint64 uploadedBytes;
    qint64 downloadedBytes;
    qint64 length;
    
    bool firstTrackerRequest;
    bool lastTrackerRequest;
};

#endif
