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
#include "bencodeparser.h"
#include "torrentclient.h"
#include "trackerclient.h"

#include <QDebug>
#include <QEvent>
#include <QFile>
#include <QTimer>
#include <QUrl>

static inline quint32 qntoh(quint32 source)
{
    return 0
        | ((source & 0x000000ff) << 24)
        | ((source & 0x0000ff00) << 8)
        | ((source & 0x00ff0000) >> 8)
        | ((source & 0xff000000) >> 24);
}

TrackerClient::TrackerClient(TorrentClient *downloader, QObject *parent)
    : QObject(parent), torrentDownloader(downloader)
{
    length = 0;
    uploadedBytes = 0;
    downloadedBytes = 0;
    seeders = 0;
    leechers = 0;
    requestInterval = 5 * 60;
    requestIntervalTimer = -1;
    firstTrackerRequest = true;
    lastTrackerRequest = false;

    connect(&http, SIGNAL(done(bool)), this, SLOT(httpRequestDone(bool)));
}

void TrackerClient::start(const MetaInfo &info)
{
    metaInfo = info;
    QTimer::singleShot(0, this, SLOT(fetchPeerList()));

    if (metaInfo.fileForm() == MetaInfo::SingleFileForm) {
	length = metaInfo.singleFile().length;
    } else {
	QList<MetaInfoMultiFile> files = metaInfo.multiFiles();
	for (int i = 0; i < files.size(); ++i)
	    length += files.at(i).length;
    }
}

void TrackerClient::stop()
{
    lastTrackerRequest = true;
    http.abort();
    fetchPeerList();
}

int TrackerClient::leechCount() const
{
    return leechers;
}

int TrackerClient::seedCount() const
{
    return seeders;
}

qint64 TrackerClient::uploadCount() const
{
    return uploadedBytes;
}

qint64 TrackerClient::downloadCount() const
{
    return downloadedBytes;
}

void TrackerClient::setUploadCount(qint64 bytes)
{
    uploadedBytes = bytes;
}

void TrackerClient::setDownloadCount(qint64 bytes)
{
    downloadedBytes = bytes;
}

void TrackerClient::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == requestIntervalTimer) {
	if (http.state() == QHttp::Unconnected)
	    fetchPeerList();
    }
}

void TrackerClient::fetchPeerList()
{
    // Prepare connection details
    QUrl url(metaInfo.announceUrl());

    QByteArray infoHash = torrentDownloader->infoHash();
    unsigned int *messageDigest = (unsigned int *)infoHash.constData();

    // Percent encode the hash
    QByteArray encodedSum;
    const char hex[] = "0123456789ABCDEF";
    unsigned char chars[4];

    for (int i = 0; i < 5; ++i) {
	unsigned int digest = qntoh(messageDigest[i]);
	chars[0] = (digest & 0xff000000) >> 24;
	chars[1] = (digest & 0x00ff0000) >> 16;
	chars[2] = (digest & 0x0000ff00) >> 8;
	chars[3] = (digest & 0x000000ff);

	for (int j = 0; j < 4; ++j) {
	    unsigned char c = chars[j];
	    if ((c >= '0' && c <= '9')
		|| (c >= 'a' && c <= 'z')
		|| (c >= 'A' && c <= 'Z'))
		encodedSum += c;
	    else switch (c) {
	    case '$': case '-': case '_': case '.':
	    case '+': case '!': case '*': case '\'':
	    case '(': case ')':
		encodedSum += c;
		break;
	    default: {
		encodedSum += "%";
		encodedSum += hex[(c & 0xf0) >> 4];
		encodedSum += hex[c & 0xf];
		break;
	    }
	    }
	}
    }

    QByteArray query;
    query += url.path().toLatin1();
    query += "?";
    query += "info_hash=" + encodedSum;
    query += "&peer_id=" + torrentDownloader->peerId();
    query += "&port=" + QByteArray::number(torrentDownloader->serverPort());
    query += "&uploaded=" + QByteArray::number(uploadedBytes);
    query += "&downloaded=" + QByteArray::number(downloadedBytes);
    query += "&left="+ QByteArray::number(qMax<int>(0, length - downloadedBytes));
    query += "&compact=1";
    if (firstTrackerRequest) {
	firstTrackerRequest = false;
	query += "&event=started";
    } else if (lastTrackerRequest) {
	query += "&event=stopped";
    } else if (torrentDownloader->state() == TorrentClient::Seeding) {
	query += "&event=completed";
   }
    if (!trackerId.isEmpty())
	query += "&trackerid=" + trackerId;
    
    http.setHost(url.host(), url.port() == -1 ? 80 : url.port());
    if (!url.userName().isEmpty())
	http.setUser(url.userName(), url.password());
 
    http.get(query);
}

void TrackerClient::httpRequestDone(bool error)
{
    if (lastTrackerRequest) {
	emit stopped();
	return;
    }

    if (error) {
	emit connectionError(http.error());
	return;
    }

    QByteArray response = http.readAll();
    http.abort();

    BencodeParser parser;
    if (!parser.parse(response)) {
	qWarning("Error parsing bencode response from tracker: %s",
		 qPrintable(parser.errorString()));
	http.abort();
	return;
    }

    QMap<QByteArray, QVariant> dict = parser.dictionary();

    if (dict.contains("failure reason")) {
	// no other items are present
	emit failure(QString::fromUtf8(dict.value("failure reason").toByteArray()));
	return;
    }

    if (dict.contains("warning message")) {
	// continue processing
	emit warning(QString::fromUtf8(dict.value("warning message").toByteArray()));
    }

    if (dict.contains("tracker id")) {
	// store it
        trackerId = dict.value("tracker id").toByteArray();
    }

    if (dict.contains("complete")) {
	// store it
        int tmp = dict.value("complete").toInt();
	if (tmp != seeders) {
            seeders = tmp;
	    emit seedCountUpdated(tmp);
        }
    }

    if (dict.contains("incomplete")) {
	// store it
        int tmp = dict.value("incomplete").toInt();
	if (tmp != leechers) {
            leechers = tmp;
	    emit leechCountUpdated(tmp);
        }
    }

    if (dict.contains("interval")) {
	// Mandatory item
	if (requestIntervalTimer != -1)
	    killTimer(requestIntervalTimer);
	requestIntervalTimer = startTimer(dict.value("interval").toInt() * 1000);
    }

    if (dict.contains("peers")) {
	// store it
	peers.clear();
	QVariant peerEntry = dict.value("peers");
	if (peerEntry.type() == QVariant::List) {
	    QList<QVariant> peerTmp = peerEntry.toList();
	    for (int i = 0; i < peerTmp.size(); ++i) {
		TorrentPeer tmp;
		QMap<QByteArray, QVariant> peer = qVariantValue<QMap<QByteArray, QVariant> >(peerTmp.at(i));
		tmp.id = QString::fromUtf8(peer.value("peer id").toByteArray());
		tmp.address.setAddress(QString::fromUtf8(peer.value("ip").toByteArray()));
		tmp.port = peer.value("port").toInt();
		peers << tmp;
	    }
	} else {
	    QByteArray peerTmp = peerEntry.toByteArray();
	    for (int i = 0; i < peerTmp.size(); i += 6) {
		TorrentPeer tmp;
                uchar *data = (uchar *)peerTmp.constData() + i;
		tmp.port = (int(data[4]) << 8) + data[5];
		uint ipAddress = uint(data[0]) << 24;
		ipAddress += uint(data[1]) << 16;
		ipAddress += uint(data[2]) << 8;
		ipAddress += uint(data[3]);
		tmp.address.setAddress(ipAddress);
		peers << tmp;
	    }
	}
	emit peerListUpdated(peers);
    }
}

void TrackerClient::addUploaded(qint64 uploadedBytes)
{
    this->uploadedBytes += uploadedBytes;
}

void TrackerClient::addDownloaded(qint64 downloadedBytes)
{
    this->downloadedBytes += downloadedBytes;
}

