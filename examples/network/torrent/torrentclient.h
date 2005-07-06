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
#ifndef TORRENTCLIENT_H
#define TORRENTCLIENT_H

#include <QtCore/QBitArray>
#include <QtNetwork/QHostAddress>

class MetaInfo;
class PeerWireClient;
class TorrentClientPrivate;
struct TorrentPeer;
class TorrentPiece;
template<typename T> class QList;
class QTimerEvent;

struct TorrentPeer {
    QHostAddress address;
    quint16 port;
    QString id;
    bool interesting;
    bool seed;
    uint lastVisited;
    uint connectStart;
    uint connectTime;
    QBitArray pieces;
    int numCompletedPieces;

    inline bool operator==(const TorrentPeer &other)
    {
	return port == other.port
	    && address == other.address
	    && id == other.id;
    }
};

class TorrentClient : public QObject
{
    Q_OBJECT
public:
    enum State {
	Idle,
	Paused,
	Stopping,
	Preparing,
	Searching,
        Connecting,
	Downloading,
	Seeding
    };
    enum Error {
	UnknownError,
	TorrentParseError,
	InvalidTrackerError,
        FileError,
	ServerError
    };

    TorrentClient(QObject *parent = 0);
    ~TorrentClient();

    bool setTorrent(const QString &fileName);
    bool setTorrent(const QByteArray &torrentData);
    MetaInfo metaInfo() const;

    void setMaxConnections(int connections);
    int maxConnections() const;

    void setDestinationFolder(const QString &directory);
    QString destinationFolder() const;

    void setDumpedState(const QByteArray &dumpedState);
    QByteArray dumpedState() const;

    // Progress and stats for download feedback.
    qint64 progress() const;
    void setDownloadedBytes(qint64 bytes);
    qint64 downloadedBytes() const;
    void setUploadedBytes(qint64 bytes);
    qint64 uploadedBytes() const;
    int visitedPeerCount() const;
    int connectedPeerCount() const;
    int seedCount() const;
    int leechCount() const;
   
    // Accessors for the tracker
    QByteArray peerId() const;
    QByteArray infoHash() const;
    quint16 serverPort() const;

    // State and error.
    State state() const;
    QString stateString() const;
    Error error() const;
    QString errorString() const;

signals:
    void stateChanged(State state);
    void error(Error error);

    void downloadCompleted();
    void peerInfoUpdated();
    
    void dataSent(int uploadedBytes);
    void dataReceived(int downloadedBytes);
    void progressUpdated(int percentProgress);
    void downloadRateUpdated(int bytesPerSecond);
    void uploadRateUpdated(int bytesPerSecond);

    void stopped();

public slots:
    void start();
    void stop();
    void setPaused(bool paused);

protected slots:
    void timerEvent(QTimerEvent *event);

private slots:
    // File management
    void sendToPeer(int readId, int pieceIndex, int begin, const QByteArray &data);
    void fullVerificationDone();
    void pieceVerified(int pieceIndex, bool ok);
    void handleFileError();

    // Connection handling
    void connectToPeers();
    QList<TorrentPeer *> weighedFreePeers() const;
    void handleSocketError(QAbstractSocket::SocketError socketError);
    void setupIncomingConnection(PeerWireClient *client);
    void setupOutgoingConnection();
    void initializeConnection(PeerWireClient *client);
    void removeClient();
    void peerPiecesAvailable(const QSet<int> &pieces);
    void peerRequestsBlock(int pieceIndex, int begin, int length);
    void blockReceived(int pieceIndex, int begin, const QByteArray &data);
    void peerWireBytesWritten(qint64 bytes);
    void peerWireBytesReceived(qint64 bytes);
    int blocksInProgressForPiece(const TorrentPiece *piece) const;
    int blocksLeftForPiece(const TorrentPiece *piece) const;

    // Scheduling
    void schedulePayloads();
    void schedulePieceForClient(PeerWireClient *client);
    void requestBlocks(PeerWireClient *client, TorrentPiece *piece);
    void peerChoked();
    void peerUnchoked();
    void peerInterested();
    void peerNotInterested();

    // Tracker handling
    void addToPeerList(const QList<TorrentPeer> &peerList);
    void trackerInfoUpdated();
    void trackerStopped();

    // Progress
    void updateProgress(int progress = -1);

private:
    TorrentClientPrivate *d;
    friend class TorrentClientPrivate;
};

#endif
