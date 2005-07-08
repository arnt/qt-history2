// -*-mode:c++;c-basic-offset:4-*-
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
**
****************************************************************************/
#include "filemanager.h"
#include "metainfo.h"
#include "torrentclient.h"
#include "trackerclient.h"
#include "peerwireclient.h"
#include "ratecontroller.h"

#include <QtCore/QtCore>
#include <QtNetwork/QTcpServer>

extern "C" {
#include "3rdparty/sha1.h"
}

static const int ServerMinPort = 6881;
static const int ServerMaxPort = /* 6889 */ 7000;
static const int BlockSize = 16384;
static const int MaxBlocksInProgress = 8;
static const int MaxConnectionPerPeer = 1;
static const int MaxConnections = 30;
static const int RateControlWindowLength = 10;
static const int RateControlTimerDelay = 1000;
static const int MinimumTimeBeforeRevisit = 30;

struct TorrentPiece {
    int index;
    int length;
    QBitArray completedBlocks;
    QBitArray requestedBlocks;
    bool inProgress;
};

class PeerWireServer : public QTcpServer
{
    Q_OBJECT
public:
    inline PeerWireServer(QObject *parent = 0) : QTcpServer(parent) {}

signals:
    void newPeerWireClient(PeerWireClient *client);

protected:
    inline void incomingConnection(int socketDescriptor)
    {
	PeerWireClient *client = new PeerWireClient(this);
	client->setSocketDescriptor(socketDescriptor);
	emit newPeerWireClient(client);
    }
};

class TorrentClientPrivate
{
public:
    TorrentClientPrivate(TorrentClient *qq);

    // State / error
    void setError(TorrentClient::Error error);
    void setState(TorrentClient::State state);
    TorrentClient::Error error;
    TorrentClient::State state;
    QString errorString;
    QString stateString;

    // Where to save data
    QString destinationFolder;
    MetaInfo metaInfo;

    // Announce tracker and file manager
    QByteArray peerId;
    QByteArray infoHash;
    TrackerClient trackerClient;
    FileManager fileManager;
    PeerWireServer peerWireServer;

    // Connections
    int maxConnections;
    QSet<PeerWireClient *> outgoingClients;
    QSet<PeerWireClient *> incomingClients;
    QList<TorrentPeer *> peers;
    int seeds;
    int leeches;
    bool schedulerCalled;
    void callScheduler();
    bool connectingToClients;
    void callPeerConnector();

    // Pieces
    QMap<int, PeerWireClient *> readIds;
    QMap<PeerWireClient *, TorrentPiece *> payloads;
    QMultiMap<PeerWireClient *, int> pieceLocations;
    QMap<int, TorrentPiece *> pendingPieces;
    QSet<int> completedPieces;
    QSet<int> incompletePieces;
    int pieceCount;

    // Progress
    int lastProgressValue;
    qint64 downloadedBytes;
    qint64 uploadedBytes;
    int downloadRate[RateControlWindowLength];
    int uploadRate[RateControlWindowLength];
    int transferRateTimer;

    TorrentClient *q;
};

TorrentClientPrivate::TorrentClientPrivate(TorrentClient *qq)
    : trackerClient(qq), q(qq)
{
    error = TorrentClient::UnknownError;
    state = TorrentClient::Idle;
    errorString = QT_TRANSLATE_NOOP(TorrentClient, "Unknown error");
    stateString = QT_TRANSLATE_NOOP(TorrentClient, "Idle");
    maxConnections = MaxConnections;
    seeds = 0;
    leeches = 0;
    schedulerCalled = false;
    connectingToClients = false;
    lastProgressValue = -1;
    pieceCount = 0;
    downloadedBytes = 0;
    uploadedBytes = 0;
    memset(downloadRate, 0, sizeof(downloadRate));
    memset(uploadRate, 0, sizeof(uploadRate));
    transferRateTimer = 0;
}

void TorrentClientPrivate::setError(TorrentClient::Error errorCode)
{
    this->error = errorCode;
    switch (error) {
    case TorrentClient::UnknownError:
	errorString = QT_TRANSLATE_NOOP(TorrentClient, "Unknown error");
	break;
    case TorrentClient::TorrentParseError:
	errorString = QT_TRANSLATE_NOOP(TorrentClient, "Invalid torrent data");
	break;
    case TorrentClient::InvalidTrackerError:
	errorString = QT_TRANSLATE_NOOP(TorrentClient, "Unable to connect to tracker");
	break;
    case TorrentClient::FileError:
	errorString = QT_TRANSLATE_NOOP(TorrentClient, "File error");
	break;
    case TorrentClient::ServerError:
	errorString = QT_TRANSLATE_NOOP(TorrentClient, "Unable to initialize server");
	break;
    }
    emit q->error(errorCode);
}

void TorrentClientPrivate::setState(TorrentClient::State state)
{
    this->state = state;
    switch (state) {
    case TorrentClient::Idle:
	stateString = QT_TRANSLATE_NOOP(TorrentClient, "Idle");
	break;
    case TorrentClient::Paused:
	stateString = QT_TRANSLATE_NOOP(TorrentClient, "Paused");
	break;
    case TorrentClient::Stopping:
	stateString = QT_TRANSLATE_NOOP(TorrentClient, "Stopping");
	break;
    case TorrentClient::Preparing:
	stateString = QT_TRANSLATE_NOOP(TorrentClient, "Preparing");
	break;
    case TorrentClient::Searching:
	stateString = QT_TRANSLATE_NOOP(TorrentClient, "Searching");
	break;
    case TorrentClient::Connecting:
	stateString = QT_TRANSLATE_NOOP(TorrentClient, "Connecting");
	break;
    case TorrentClient::Downloading:
	stateString = QT_TRANSLATE_NOOP(TorrentClient, "Downloading");
	break;
    case TorrentClient::Seeding:
	stateString = QT_TRANSLATE_NOOP(TorrentClient, "Seeding");
	break;
    }
    emit q->stateChanged(state);
}

void TorrentClientPrivate::callScheduler()
{
    if (!schedulerCalled) {
	schedulerCalled = true;
        QMetaObject::invokeMethod(q, "schedulePayloads", Qt::QueuedConnection);
    }
}

void TorrentClientPrivate::callPeerConnector()
{
    if (!connectingToClients) {
	connectingToClients = true;
        QMetaObject::invokeMethod(q, "connectToPeers", Qt::QueuedConnection);
    }
}

TorrentClient::TorrentClient(QObject *parent)
    : QObject(parent), d(new TorrentClientPrivate(this))
{
    // Generate peer id    
    time_t startupTime = QDateTime::currentDateTime().toTime_t();
    d->peerId = "-QB1000-" + QByteArray::number(int(startupTime), 16);
    d->peerId += QByteArray(20 - d->peerId.size(), '-');

    // Connect the file manager
    connect(&d->fileManager, SIGNAL(dataRead(int, int, int, const QByteArray &)),
	    this, SLOT(sendToPeer(int, int, int, const QByteArray &)));
    connect(&d->fileManager, SIGNAL(verificationProgress(int)),
	    this, SLOT(updateProgress(int)));
    connect(&d->fileManager, SIGNAL(verificationDone()),
	    this, SLOT(fullVerificationDone()));
    connect(&d->fileManager, SIGNAL(pieceVerified(int, bool)),
	    this, SLOT(pieceVerified(int, bool)));

    // Connect the tracker client
    connect(&d->trackerClient, SIGNAL(peerListUpdated(const QList<TorrentPeer> &)),
	    this, SLOT(addToPeerList(const QList<TorrentPeer> &)));
    connect(&d->trackerClient, SIGNAL(seedCountUpdated(int)),
            this, SLOT(trackerInfoUpdated()));
    connect(&d->trackerClient, SIGNAL(leechCountUpdated(int)),
            this, SLOT(trackerInfoUpdated()));
    connect(&d->trackerClient, SIGNAL(stopped()),
	    this, SIGNAL(stopped()));
}

TorrentClient::~TorrentClient()
{
    qDeleteAll(d->peers);
    qDeleteAll(d->pendingPieces);
    delete d;
}

bool TorrentClient::setTorrent(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly) || !setTorrent(file.readAll())) {
	d->setError(TorrentParseError);
	return false;
    }
    return true;
}

bool TorrentClient::setTorrent(const QByteArray &torrentData)
{
    if (!d->metaInfo.parse(torrentData)) {
	d->setError(TorrentParseError);
	return false;
    }

    // Calculate SHA1 hash of the "info" section in the torrent
    QByteArray block = d->metaInfo.infoValue();
    SHA1Context sha;
    SHA1Reset(&sha);
    SHA1Input(&sha, (const unsigned char *)block.constData(), block.size());
    SHA1Result(&sha);
    QByteArray sha1Sum(20, ' ');
    unsigned char *digest = (unsigned char *)sha.Message_Digest;
    for (int i = 0; i < 5; ++i) {
#if Q_BYTE_ORDER == Q_BIG_ENDIAN
        sha1Sum[i * 4 + 3] = digest[i * 4 + 3];
        sha1Sum[i * 4 + 2] = digest[i * 4 + 2];
        sha1Sum[i * 4 + 1] = digest[i * 4 + 1];
        sha1Sum[i * 4 + 0] = digest[i * 4 + 0];
#else
        sha1Sum[i * 4 + 0] = digest[i * 4 + 3];
        sha1Sum[i * 4 + 1] = digest[i * 4 + 2];
        sha1Sum[i * 4 + 2] = digest[i * 4 + 1];
        sha1Sum[i * 4 + 3] = digest[i * 4 + 0];
#endif
    }

    d->infoHash = sha1Sum;
    return true;
}

MetaInfo TorrentClient::metaInfo() const
{
    return d->metaInfo;
}

void TorrentClient::setMaxConnections(int connections)
{
    d->maxConnections = connections;
}

int TorrentClient::maxConnections() const
{
    return d->maxConnections;
}

void TorrentClient::setDestinationFolder(const QString &directory)
{
    d->destinationFolder = directory;
}

QString TorrentClient::destinationFolder() const
{
    return d->destinationFolder;
}

void TorrentClient::setDumpedState(const QByteArray &dumpedState)
{
    // Recover partially completed pieces
    QDataStream stream(dumpedState);
    while (!stream.atEnd()) {
	int index;
	int length;
	QBitArray bits;
	stream >> index >> length >> bits;
	if (stream.status() != QDataStream::Ok)
	    break;

	// ### it should be enough the read the QBitArray, but the
	// ### size is sometimes wrong then.
        int size = length / BlockSize;
        if (length % BlockSize)
            ++size;
	QBitArray completed(size);
	for (int i = 0; i < bits.size(); ++i) {
	    if (bits.testBit(i))
		completed.setBit(i);
	}

	TorrentPiece *piece = new TorrentPiece;
	piece->index = index;
	piece->length = length;
	piece->completedBlocks = completed;
	piece->requestedBlocks.resize(completed.size());
	piece->inProgress = false;
	d->pendingPieces[index] = piece;
    }
}

QByteArray TorrentClient::dumpedState() const
{
    QByteArray partials;
    QDataStream stream(&partials, QIODevice::WriteOnly);

    // Save the state of all partially downloaded pieces into a format
    // suitable for storing in settings.
    QMap<int, TorrentPiece *>::ConstIterator it = d->pendingPieces.constBegin();
    while (it != d->pendingPieces.constEnd()) {
	TorrentPiece *piece = it.value();
	if (blocksLeftForPiece(piece) > 0 && blocksLeftForPiece(piece) < piece->completedBlocks.size()) {
	    stream << piece->index;
	    stream << piece->length;
	    stream << piece->completedBlocks;
	}
	++it;
    }

    return partials;
}

qint64 TorrentClient::progress() const
{
    return d->lastProgressValue;
}

void TorrentClient::setDownloadedBytes(qint64 bytes)
{
    d->downloadedBytes = bytes;
}

qint64 TorrentClient::downloadedBytes() const
{
    return d->downloadedBytes;
}

void TorrentClient::setUploadedBytes(qint64 bytes)
{
    d->uploadedBytes = bytes;
}

qint64 TorrentClient::uploadedBytes() const
{
    return d->uploadedBytes;
}

int TorrentClient::visitedPeerCount() const
{
    int tmp = 0;    
    foreach (TorrentPeer *peer, d->peers) {
        if (peer->lastVisited)
            ++tmp;
    }
    return tmp;
}

int TorrentClient::connectedPeerCount() const
{
    int tmp = 0;
    foreach (PeerWireClient *client, d->incomingClients + d->outgoingClients) {
        if (client->state() == QAbstractSocket::ConnectedState)
            ++tmp;
    }
    return tmp;
}

int TorrentClient::seedCount() const
{
    return d->seeds;
}

int TorrentClient::leechCount() const
{
    return d->leeches;
}

TorrentClient::State TorrentClient::state() const
{
    return d->state;
}

QString TorrentClient::stateString() const
{
    return d->stateString;
}

TorrentClient::Error TorrentClient::error() const
{
    return d->error;
}

QString TorrentClient::errorString() const
{
    return d->errorString;
}

QByteArray TorrentClient::peerId() const
{
    return d->peerId;
}

QByteArray TorrentClient::infoHash() const
{
    return d->infoHash;
}

quint16 TorrentClient::serverPort() const
{
    return d->peerWireServer.serverPort();
}

void TorrentClient::start()
{
    if (d->state != Idle)
	return;
    
    // Initialize the file manager
    d->setState(Preparing);
    d->fileManager.setMetaInfo(d->metaInfo);    
    d->fileManager.setDestinationFolder(d->destinationFolder);    
    d->fileManager.start(QThread::LowestPriority);    
    d->fileManager.startDataVerification();
}

void TorrentClient::stop()
{
    if (d->state == Stopping)
        return;
    
    State oldState = d->state;
    d->setState(Stopping);

    if (d->transferRateTimer) {
	killTimer(d->transferRateTimer);
	d->transferRateTimer = 0;
    }

    if (d->peerWireServer.isListening())
	d->peerWireServer.close();
    
    d->maxConnections = 0;

    foreach (PeerWireClient *client, d->incomingClients + d->outgoingClients)
	client->abort();
    d->incomingClients.clear();
    d->outgoingClients.clear();
    
    if (oldState > Preparing) {
	d->trackerClient.stop();
    } else {
	d->setState(Idle);
	emit stopped();
    }
}

void TorrentClient::setPaused(bool paused)
{
    if (paused) {
	// Abort all connections, and set the max number of
	// connections to 0. Keep the list of peers, so we can quickly
	// resume later.
	d->setState(Paused);
	d->maxConnections = 0;
	foreach (PeerWireClient *client, d->incomingClients)
	    client->abort();
	d->incomingClients.clear();
	foreach (PeerWireClient *client, d->outgoingClients)
	    client->abort();
	d->outgoingClients.clear();
    } else {
	// Restore the max number of connections, and start the peer
	// connector. We should also quickly start receiving incoming
	// connections.
	d->setState(d->completedPieces.size() == d->fileManager.pieceCount()
		    ? Seeding : Searching);
	d->maxConnections = MaxConnections;
	d->callPeerConnector();
    }
}

void TorrentClient::timerEvent(QTimerEvent *event)
{
    if (event->timerId() != d->transferRateTimer) {
	QObject::timerEvent(event);
	return;
    }

    // Calculate average download rate
    qint64 downloadBytesPerSecond = 0;
    for (int i = 0; i < RateControlWindowLength; ++i)
	downloadBytesPerSecond += d->downloadRate[i];
    downloadBytesPerSecond /= qint64(RateControlWindowLength);
    for (int i = RateControlWindowLength - 2; i >= 0; --i)
	d->downloadRate[i + 1] = d->downloadRate[i];
    d->downloadRate[0] = 0;
    emit downloadRateUpdated(int(downloadBytesPerSecond));

    // Calculate average upload rate
    qint64 uploadBytesPerSecond = 0;
    for (int i = 0; i < RateControlWindowLength; ++i)
	uploadBytesPerSecond += d->uploadRate[i];
    uploadBytesPerSecond /= qint64(RateControlWindowLength);
    for (int i = RateControlWindowLength - 2; i >= 0; --i)
	d->uploadRate[i + 1] = d->uploadRate[i];
    d->uploadRate[0] = 0;
    emit uploadRateUpdated(int(uploadBytesPerSecond));

    // Stop the timer if there is no activity.
    if (downloadBytesPerSecond == 0 && uploadBytesPerSecond == 0) {
	killTimer(d->transferRateTimer);
	d->transferRateTimer = 0;
    }
}

void TorrentClient::sendToPeer(int readId, int pieceIndex, int begin, const QByteArray &data)
{
    // Send the requested block to the peer if the client connection
    // still exists; otherwise do nothing. This slot is called by the
    // file manager after it's read a block of data.
    PeerWireClient *client = d->readIds.value(readId, 0);
    if (client)
        client->sendBlock(pieceIndex, begin, data);
    d->readIds.remove(readId);
}

void TorrentClient::fullVerificationDone()
{
    // Update our list of completed and incomplete pieces.
    d->completedPieces = d->fileManager.completedPieces();
    d->pieceCount = d->fileManager.pieceCount();
    for (int i = 0; i < d->fileManager.pieceCount(); ++i) {
	if (!d->completedPieces.contains(i))
	    d->incompletePieces.insert(i);
    }

    updateProgress();

    // If the checksums show that what the dumped state thought was
    // partial was in fact complete, then we trust the checksums.
    QMap<int, TorrentPiece *>::Iterator it = d->pendingPieces.begin();
    while (it != d->pendingPieces.end()) {
	if (d->completedPieces.contains(it.key()))
	    it = d->pendingPieces.erase(it);
	else
            ++it;
    }

    // Set up the peer wire server
    d->peerWireServer.setMaxPendingConnections(d->maxConnections);
    connect(&d->peerWireServer, SIGNAL(newPeerWireClient(PeerWireClient*)),
	    this, SLOT(setupIncomingConnection(PeerWireClient*)));
    for (int i = ServerMinPort; i <= ServerMaxPort; ++i) {
	if (d->peerWireServer.listen(QHostAddress::Any, i))
	    break;
    }
    if (!d->peerWireServer.isListening()) {
	d->setError(ServerError);
	return;
    }

    // Start the tracker client
    d->trackerClient.setUploadCount(d->uploadedBytes);
    d->trackerClient.setDownloadCount(d->downloadedBytes);
    d->trackerClient.start(d->metaInfo);

    d->setState(d->completedPieces.size() == d->pieceCount
		? Seeding : Searching);
}

void TorrentClient::pieceVerified(int pieceIndex, bool ok)
{
    TorrentPiece *piece = d->pendingPieces.value(pieceIndex);
   
    if (!ok) {
	piece->inProgress = false;
	piece->completedBlocks.fill(false);
	piece->requestedBlocks.fill(false);
        return;
    }

    // Remove this piece from all payloads
    QMap<PeerWireClient *, TorrentPiece *>::Iterator it = d->payloads.begin();
    while (it != d->payloads.end()) {
	if (it.value()->index == pieceIndex)
	    it = d->payloads.erase(it);
        else
	    ++it;
    }

    // Update the peer list so we know who's still interesting.
    foreach (TorrentPeer *peer, d->peers) {
	if (!peer->interesting)
	    continue;
	bool interesting = false;
	for (int i = 0; i < d->pieceCount; ++i) {
	    if (peer->pieces.testBit(i) && d->incompletePieces.contains(i)) {
		interesting = true;
		break;
	    }
	}
	peer->interesting = false;
    }

    delete piece;
    d->pendingPieces.remove(pieceIndex);
    d->completedPieces << pieceIndex;
    d->incompletePieces.remove(pieceIndex);

    foreach (PeerWireClient *client, d->incomingClients + d->outgoingClients) {
	if (client->state() == QAbstractSocket::ConnectedState
	    && !client->availablePieces().contains(pieceIndex)) {
	    client->sendPieceNotification(pieceIndex);
	}
    }
   
    if (d->completedPieces.size() == d->pieceCount) {
	if (d->state != Seeding) {
            foreach (PeerWireClient *client, d->outgoingClients)
                client->disconnectFromHost();                
            d->setState(Seeding);
            d->trackerClient.start(d->metaInfo);
        }
    } else {
	d->callScheduler();
    }

    updateProgress();
}

void TorrentClient::handleFileError()
{
    qDebug() << d->fileManager.errorString();
    emit error(FileError);
    stop();
}

void TorrentClient::connectToPeers()
{
    d->connectingToClients = false;

    if (d->state == Stopping || d->state == Idle || d->outgoingClients.size() >= d->maxConnections)
	return;

    if (d->state == Searching)
	d->setState(Connecting);

    // Find the list of peers we are not currently connected to, where
    // the more interesting peers are listed more than once.
    QList<TorrentPeer *> weighedPeers = weighedFreePeers();
    if (weighedPeers.isEmpty()) {
        // If none are available now, try again in 5 seconds.
        QTimer::singleShot(5000, this, SLOT(connectToPeers()));
    }

    // Start as many connections as we can
    while (!weighedPeers.isEmpty() && (d->outgoingClients.size() < d->maxConnections)) {
        PeerWireClient *client = new PeerWireClient(this);
	RateController::instance()->addClient(client);

	connect(client, SIGNAL(connected()), 
		this, SLOT(setupOutgoingConnection()));
	connect(client, SIGNAL(disconnected()), 
		this, SLOT(removeClient()));
	connect(client, SIGNAL(error(SocketError)),
		this, SLOT(handleSocketError(SocketError)));
	connect(client, SIGNAL(piecesAvailable(const QSet<int> &)), 
		this, SLOT(peerPiecesAvailable(const QSet<int> &)));
	connect(client, SIGNAL(blockRequest(int, int, int)),
		this, SLOT(peerRequestsBlock(int, int, int)));
	connect(client, SIGNAL(blockReceived(int, int, const QByteArray &)),
		this, SLOT(blockReceived(int, int, const QByteArray &)));
	connect(client, SIGNAL(choked()), this, SLOT(peerChoked()));
	connect(client, SIGNAL(unchoked()), this, SLOT(peerUnchoked()));
	connect(client, SIGNAL(interested()), this, SLOT(peerInterested()));
	connect(client, SIGNAL(notInterested()), this, SLOT(peerNotInterested()));
	connect(client, SIGNAL(bytesWritten(qint64)), this, SLOT(peerWireBytesWritten(qint64)));
	connect(client, SIGNAL(bytesReceived(qint64)), this, SLOT(peerWireBytesReceived(qint64)));

        d->outgoingClients << client;

	// Pick a random peer from the list of weighed peers.	
	TorrentPeer *peer = weighedPeers.takeAt(rand() % weighedPeers.size());
	weighedPeers.removeAll(peer);
	peer->connectStart = QDateTime::currentDateTime().toTime_t();
	peer->lastVisited = peer->connectStart;

	// Connect to the peer.
	client->connectToHost(peer->address, peer->port);
    }
}

QList<TorrentPeer *> TorrentClient::weighedFreePeers() const
{
    QList<TorrentPeer *> weighedPeers;

    // Generate a list of peers that we want to connect to.
    uint now = QDateTime::currentDateTime().toTime_t();
    QList<TorrentPeer *> freePeers;
    QMap<QString, int> connectionsPerPeer;
    foreach (TorrentPeer *peer, d->peers) {
	bool busy = false;
	foreach (PeerWireClient *client, d->incomingClients + d->outgoingClients) {
	    if (client->state() == PeerWireClient::ConnectedState
		&& client->peerAddress() == peer->address
		&& client->peerPort() == peer->port) {
		if (++connectionsPerPeer[peer->address.toString()] >= MaxConnectionPerPeer) {
		    busy = true;
		    break;
		}
	    }
	}
	if (!busy && (now - peer->lastVisited) > uint(MinimumTimeBeforeRevisit))
	    freePeers << peer;
    }

    // Nothing to connect to
    if (freePeers.isEmpty())
	return weighedPeers;

    // Assign points based on connection speed and pieces available.
    QList<QPair<int, TorrentPeer *> > points;
    foreach (TorrentPeer *peer, freePeers) {
	int tmp = 0;
	if (peer->interesting) {
	    tmp += peer->numCompletedPieces;
	    if (d->state == Seeding)
		tmp = d->pieceCount - tmp;
	    if (!peer->connectStart) // An unknown peer is as interesting as a seed
		tmp += d->pieceCount;

	    // 1/5 of the total score for each second below 5 it takes to
	    // connect.
	    if (peer->connectTime < 5)
		tmp += (d->pieceCount / 10) * (5 - peer->connectTime);
	}
	points << QPair<int, TorrentPeer *>(tmp, peer);
    }
    qSort(points);

    // Minimize the list so the point difference is never more than 1.
    typedef QPair<int,TorrentPeer*> PointPair;
    QMultiMap<int, TorrentPeer *> pointMap;
    int lowestScore = 0;
    int lastIndex = 0;
    foreach (PointPair point, points) {
	if (point.first > lowestScore) {
	    lowestScore = point.first;
	    ++lastIndex;
	}
	pointMap.insert(lastIndex, point.second);
    }

    // Now make up a list of peers where the ones with more points are
    // listed many times.
    QMultiMap<int, TorrentPeer *>::ConstIterator it = pointMap.constBegin();
    while (it != pointMap.constEnd()) {
	for (int i = 0; i < it.key() + 1; ++i)
	    weighedPeers << it.value();
	++it;
    }

    return weighedPeers;
}

void TorrentClient::handleSocketError(QAbstractSocket::SocketError socketError)
{
    if (socketError != QAbstractSocket::RemoteHostClosedError)
	removeClient();
}

void TorrentClient::setupIncomingConnection(PeerWireClient *client)
{
    // Disconnect if there are too many connections
    if (d->incomingClients.size() >= d->maxConnections) {
	client->abort();
	return;
    }

    // Add this client to our lists
    d->incomingClients << client;
    RateController::instance()->addClient(client);

    // Initialize this client
    initializeConnection(client);

    // Call the scheduler ### but we don't know what the peer has yet
    emit peerInfoUpdated();
}

void TorrentClient::setupOutgoingConnection()
{
    PeerWireClient *client = qobject_cast<PeerWireClient *>(sender());
    foreach (TorrentPeer *peer, d->peers) {
        if (peer->port == client->peerPort() && peer->address == client->peerAddress()) {
	    peer->connectTime = peer->lastVisited - peer->connectStart;
            emit peerInfoUpdated();
	    break;
        }
    }

    // Send handshake and piece list
    client->initialize(d->infoHash, d->peerId);
    client->sendPieceList(d->completedPieces, d->pieceCount);
}

void TorrentClient::initializeConnection(PeerWireClient *client)
{
    connect(client, SIGNAL(disconnected()),
	    this, SLOT(removeClient()));
    connect(client, SIGNAL(error(SocketError)),
	    this, SLOT(handleSocketError(SocketError)));
    connect(client, SIGNAL(piecesAvailable(const QSet<int> &)), 
	    this, SLOT(peerPiecesAvailable(const QSet<int> &)));
    connect(client, SIGNAL(blockRequest(int, int, int)),
	    this, SLOT(peerRequestsBlock(int, int, int)));
    connect(client, SIGNAL(blockReceived(int, int, const QByteArray &)),
	    this, SLOT(blockReceived(int, int, const QByteArray &)));
    connect(client, SIGNAL(choked()), this, SLOT(peerChoked()));
    connect(client, SIGNAL(unchoked()), this, SLOT(peerUnchoked()));
    connect(client, SIGNAL(interested()), this, SLOT(peerInterested()));
    connect(client, SIGNAL(notInterested()), this, SLOT(peerNotInterested()));
    connect(client, SIGNAL(bytesWritten(qint64)), this, SLOT(peerWireBytesWritten(qint64)));
    connect(client, SIGNAL(bytesReceived(qint64)), this, SLOT(peerWireBytesReceived(qint64)));

    client->initialize(d->infoHash, d->peerId);
    client->sendPieceList(d->completedPieces, d->pieceCount);
    emit peerInfoUpdated();
}

void TorrentClient::removeClient()
{
    PeerWireClient *client = qobject_cast<PeerWireClient *>(sender());
    if (client->error() == QAbstractSocket::RemoteHostClosedError) {
	if (client->downloadSpeed() > 0) {
	    // Attempt to reconnect to peers that we have recently
	    // been able to download from.
	    client->connectToHost(client->lastPeerAddress(), client->lastPeerPort());
	    return;
	}
    }
    
    RateController::instance()->removeClient(client);
    d->incomingClients.remove(client);
    d->outgoingClients.remove(client);
    TorrentPiece *piece = d->payloads.value(client);
    if (piece) {
        piece->inProgress = false;
	piece->requestedBlocks.fill(false);
    }
    d->payloads.remove(client);

    QMap<int, PeerWireClient *>::Iterator it = d->readIds.begin();
    while (it != d->readIds.end()) {
        if (it.value() == client)
            it = d->readIds.erase(it);
        else
            ++it;
    }

    d->pieceLocations.remove(client);
    client->deleteLater();

    d->callPeerConnector();
    emit peerInfoUpdated();
}

void TorrentClient::peerPiecesAvailable(const QSet<int> &pieces)
{
    PeerWireClient *client = qobject_cast<PeerWireClient *>(sender());

    // Find the peer in our list of announced peers. If it's there,
    // then we can use the piece list into to gather statistics that
    // help us decide what peers to connect to.
    TorrentPeer *peer = 0;
    QList<TorrentPeer *>::Iterator it = d->peers.begin();
    while (it != d->peers.end()) {
	if ((*it)->address == client->peerAddress() && (*it)->port == client->peerPort()) {
	    peer = *it;
	    break;
	}
	++it;
    }

    // If the peer is a seed, and we are in seeding mode, then the
    // peer is uninteresting.
    if (pieces.count() == d->pieceCount) {
	if (peer)
	    peer->seed = true;
	if (d->state == Seeding) {
	    client->disconnectFromHost();
	    return;
	} else {
	    if (peer)
		peer->interesting = true;
	    if ((client->peerWireState() & PeerWireClient::InterestedInPeer) == 0)
		client->sendInterested();
	    d->callScheduler();
	    return;
	}
    }

    // Update our list of available pieces.
    if (peer) {
	peer->pieces.fill(false);
	peer->numCompletedPieces = 0;
	foreach (int pieceIndex, pieces) {
	    if (pieceIndex >= peer->pieces.size() || pieceIndex < 0)
		continue;
	    peer->pieces.setBit(pieceIndex);
	    ++peer->numCompletedPieces;
	}
    }
    
    // Check for interesting pieces, and tell the peer whether we are
    // interested or not.
    bool interested = false;
    foreach (int pieceIndex, pieces) {
	if (!d->completedPieces.contains(pieceIndex)) {
	    interested = true;
	    if ((client->peerWireState() & PeerWireClient::InterestedInPeer) == 0) {
		if (peer)
		    peer->interesting = true;
		client->sendInterested();
	    }

	    TorrentPiece *piece = d->payloads.value(client);
	    if (!piece || !piece->inProgress || blocksInProgressForPiece(piece) == 0)
		d->callScheduler();
	    break;
	}
    }
    if (!interested && (client->peerWireState() & PeerWireClient::InterestedInPeer)) {
	if (peer)
	    peer->interesting = false;
	if (d->outgoingClients.contains(client)) {
	    client->disconnectFromHost();
	} else {
	    client->sendNotInterested();
	}
    }
}

void TorrentClient::peerRequestsBlock(int pieceIndex, int begin, int length)
{
    // Silently ignore requests for pieces we don't have.
    if (!d->completedPieces.contains(pieceIndex))
	return;

    // Request the block from the file manager
    d->readIds.insert(d->fileManager.read(pieceIndex, begin, length),
		      qobject_cast<PeerWireClient *>(sender()));
}

void TorrentClient::blockReceived(int pieceIndex, int begin, const QByteArray &data)
{
    PeerWireClient *client = qobject_cast<PeerWireClient *>(sender());
    if (data.size() == 0) {
	client->disconnectFromHost();
	return;
    }

    // If we are in endgame mode, cancel all duplicate requests for
    // this block.
    bool endgameMode = d->lastProgressValue > 95;
    if (endgameMode) {
	QMap<PeerWireClient *, TorrentPiece *>::Iterator it = d->payloads.begin();
	while (it != d->payloads.end()) {
	    if (it.key() != client && it.value()->index == pieceIndex)
		it.key()->cancelRequest(pieceIndex, begin, data.size());
	    ++it;
	}
    }
    
    int blockBit = begin / BlockSize;
    TorrentPiece *piece = d->pendingPieces.value(pieceIndex);
    if (!piece || piece->completedBlocks.testBit(blockBit)) {
	// discard blocks that we already have
	return;
    }

    if (d->state != Downloading)
	d->setState(Downloading);

    // Store this block
    d->fileManager.write(pieceIndex, begin, data);
    piece->completedBlocks.setBit(blockBit);
    piece->requestedBlocks.clearBit(blockBit);

    if (blocksLeftForPiece(piece) == 0 || blocksInProgressForPiece(piece) == 0) {
        if (blocksLeftForPiece(piece) == 0) {
            // Ask the file manager to verify the newly downloaded piece
            d->fileManager.verifyPiece(piece->index);    
        }

        // Remove this piece from all payloads
        QMap<PeerWireClient *, TorrentPiece *>::Iterator it = d->payloads.begin();
        while (it != d->payloads.end()) {
            if (!it.value() || it.value()->index == piece->index)
                it = d->payloads.erase(it);
            else
                ++it;
        }

        // Schedule more blocks for downloading
	d->callScheduler();
    } else {
	// Fill up the pipeline.
	requestBlocks(client, piece);
    }
}

void TorrentClient::peerWireBytesWritten(qint64 size)
{
    if (!d->transferRateTimer)
	d->transferRateTimer = startTimer(RateControlTimerDelay);

    
    d->uploadRate[0] += size;
    //for (int i = 0; i < RateControlWindowLength/2; ++i)
    // d->uploadRate[i] += size / (RateControlWindowLength/2);
    d->uploadedBytes += size;
    emit dataSent(size);
}

void TorrentClient::peerWireBytesReceived(qint64 size)
{
    if (!d->transferRateTimer)
	d->transferRateTimer = startTimer(RateControlTimerDelay);

    d->downloadRate[0] += size;

    //    for (int i = 0; i < RateControlWindowLength/2; ++i)
    //	d->downloadRate[i] += size / (RateControlWindowLength/2);
    d->downloadedBytes += size;
    emit dataSent(size);
}

int TorrentClient::blocksInProgressForPiece(const TorrentPiece *piece) const
{
    int blocksInProgress = 0;
    for (int i = 0; i < piece->requestedBlocks.size(); ++i) {
	if (piece->requestedBlocks.testBit(i))
	    ++blocksInProgress;
    }
    return blocksInProgress;
}

int TorrentClient::blocksLeftForPiece(const TorrentPiece *piece) const
{
    int blocksLeft = 0;
    for (int i = 0; i < piece->completedBlocks.size(); ++i) {
	if (!piece->completedBlocks.testBit(i))
	    ++blocksLeft;
    }
    return blocksLeft;
}

void TorrentClient::schedulePayloads()
{
    d->schedulerCalled = false;

    if (d->state == Stopping || d->state == Idle)
	return;

    if (d->completedPieces.size() == d->pieceCount)
	return;

    // Check what each client is doing, and assign payloads to those
    // who are either idle or done.
    foreach (PeerWireClient *client, d->incomingClients + d->outgoingClients)
	schedulePieceForClient(client);
}

void TorrentClient::schedulePieceForClient(PeerWireClient *client)
{
    if (client->state() != QTcpSocket::ConnectedState)
	return;
        
    // Choked by peer; try again later
    if (client->peerWireState() & PeerWireClient::ChokedByPeer)
	return;

    // Skip all clients that have requested blocks and are waiting for
    // the response.
    TorrentPiece *pieceInProgress = d->payloads.value(client);
    if (pieceInProgress && pieceInProgress->inProgress && blocksInProgressForPiece(pieceInProgress) > 0)
	return;

    bool endgameMode = d->lastProgressValue > 95;

    TorrentPiece *piece = pieceInProgress;
    if (!piece) {
	QSet<int> incompletePiecesAvailableToClient = d->incompletePieces;
	// Unless we are in endgame mode, we remove all pieces that
	// are marked as being in progress (i.e., pieces that clients
	// are already waiting for).
	if (!endgameMode) {
	    QMap<int, TorrentPiece *>::ConstIterator it = d->pendingPieces.constBegin();
	    while (it != d->pendingPieces.constEnd()) {
		if (it.value()->inProgress)
		    incompletePiecesAvailableToClient.remove(it.key());
		++it;
	    }

	    incompletePiecesAvailableToClient &= client->availablePieces();
	}

	// If no more pieces are available for download, disconnect.
	if (incompletePiecesAvailableToClient.isEmpty()) {
	    if (d->outgoingClients.contains(client))
		client->disconnectFromHost();
	    return;
	}

	// Check to see if any of the partially completed pieces
	// can be recovered.
	QMap<int, TorrentPiece *>::ConstIterator it = d->pendingPieces.constBegin();
	while (it != d->pendingPieces.constEnd()) {
	    TorrentPiece *tmp = it.value();
	    if (incompletePiecesAvailableToClient.contains(it.key())) {
		if (endgameMode || !tmp->inProgress) {
		    piece = tmp;
		    break;
		}
	    }
	    ++it;
	}

	if (!piece) {
	    // Either pick one of the least available pieces, or just
	    // pick a random one.
	    int pieceIndex = 0;
	    if (!endgameMode && (rand() & 4) == 0) {
		QMap<int, int> occurrances;
		foreach (PeerWireClient *peer, d->incomingClients + d->outgoingClients) {
		    foreach (int availablePieceIndex, peer->availablePieces())
			++occurrances[availablePieceIndex];
		}
		QMap<int, int>::ConstIterator it = occurrances.constBegin();
		int numOccurrances = 99999;
		QList<int> piecesReadyForDownload;
		while (it != occurrances.constEnd()) {
		    if (it.value() <= numOccurrances
			&& incompletePiecesAvailableToClient.contains(it.key())) {
			if (it.value() < numOccurrances)
			    piecesReadyForDownload.clear();
			piecesReadyForDownload.append(it.key());
			numOccurrances = it.value();
		    }
		    ++it;
		}
		pieceIndex = piecesReadyForDownload.at(rand() % piecesReadyForDownload.size());
	    } else {
		pieceIndex = incompletePiecesAvailableToClient.values()
                             .at(rand() % incompletePiecesAvailableToClient.size());
	    }

	    // Create a new Piece and fill in all initial properties.           
	    piece = new TorrentPiece;
	    piece->index = pieceIndex;
	    piece->length = d->fileManager.pieceLengthAt(pieceIndex);
	    int numBlocks = piece->length / BlockSize;
	    if (piece->length % BlockSize)
		++numBlocks;
	    piece->completedBlocks.resize(numBlocks);
	    piece->requestedBlocks.resize(numBlocks);
	    d->pendingPieces.insert(pieceIndex, piece);
	}
    }

    piece->inProgress = true;
    d->payloads.insert(client, piece);
    requestBlocks(client, piece);
}

void TorrentClient::requestBlocks(PeerWireClient *client, TorrentPiece *piece)
{
    // Generate the list of incomplete blocks, and randomize it
    QVector<int> bits;
    for (int i = 0; i < piece->completedBlocks.size(); ++i) {
	if (!piece->completedBlocks.testBit(i) && !piece->requestedBlocks.testBit(i))
	    bits << i;
    }

    bool endgameMode = d->lastProgressValue > 95;

    // Nothing more to request
    if (bits.size() == 0) {
	if (!endgameMode)
	    return;
	bits.clear();
	for (int i = 0; i < piece->completedBlocks.size(); ++i) {
	    if (!piece->completedBlocks.testBit(i))
		bits << i;
	}
    }

    // Randomize the list
    for (int i = 0; i < bits.size(); ++i) {
	int a = rand() % bits.size();
	int b = rand() % bits.size();
	int tmp = bits[a];
	bits[a] = bits[b];
	bits[b] = tmp;
    }

    int blocksInProgress = endgameMode ? 0 : blocksInProgressForPiece(piece);

    // Request all blocks in random order
    for (int i = 0; i < qMin(MaxBlocksInProgress - blocksInProgress, bits.size()); ++i) {
	int blockSize = BlockSize;
	if ((piece->length % BlockSize) && bits.at(i) == piece->completedBlocks.size() - 1)
	    blockSize = piece->length % BlockSize;
        client->requestBlock(piece->index, bits.at(i) * BlockSize, blockSize);
	piece->requestedBlocks.setBit(bits.at(i));
    }
}

void TorrentClient::peerChoked()
{
    PeerWireClient *client = qobject_cast<PeerWireClient *>(sender());
    if (!client)
	return;

    TorrentPiece *piece = d->payloads.value(client);
    if (piece)
	piece->requestedBlocks.fill(false);
}

void TorrentClient::peerUnchoked()
{
    PeerWireClient *client = qobject_cast<PeerWireClient *>(sender());
    if (!client)
	return;
   
    // We got unchoked
    TorrentPiece *piece = d->payloads.value(client);
    if (!piece) {
	d->callScheduler();
	return;
    }

    if (d->state != Seeding)
	requestBlocks(client, piece);
}

void TorrentClient::peerInterested()
{
    PeerWireClient *client = qobject_cast<PeerWireClient *>(sender());
    client->unchokePeer();
}

void TorrentClient::peerNotInterested()
{
    // ### is there anything we can do here?
}

void TorrentClient::addToPeerList(const QList<TorrentPeer> &peerList)
{
    int newPeers = 0;
    foreach (TorrentPeer peer, peerList) {
	bool known = false;
	foreach (TorrentPeer *knownPeer, d->peers) {
	    if (knownPeer->port == peer.port
		&& knownPeer->address == peer.address) {
		known = true;
		break;
	    }
	}
	if (!known) {
	    ++newPeers;
	    TorrentPeer *newPeer = new TorrentPeer;
	    *newPeer = peer;
	    newPeer->interesting = true;
            newPeer->seed = false;
            newPeer->lastVisited = 0;
            newPeer->connectStart = 0;
            newPeer->connectTime = 999999;
	    newPeer->pieces.resize(d->pieceCount);
	    newPeer->numCompletedPieces = 0;
	    d->peers << newPeer;
	}
    }

    if (newPeers)
        emit peerInfoUpdated();

    if (d->outgoingClients.size() < d->maxConnections && d->state != Paused)
	d->callPeerConnector();
}

void TorrentClient::trackerInfoUpdated()
{
    d->seeds = d->trackerClient.seedCount();
    d->leeches = d->trackerClient.leechCount();
    emit peerInfoUpdated();
}

void TorrentClient::trackerStopped()
{
    d->setState(Idle);
    emit stopped();
}

void TorrentClient::updateProgress(int progress)
{
    if (progress == -1) {
        int newProgress = (d->completedPieces.size() * 100) / d->pieceCount;
        if (d->lastProgressValue != newProgress) {
            d->lastProgressValue = newProgress;
            emit progressUpdated(newProgress);
        }
    } else if (d->lastProgressValue != progress) {
        d->lastProgressValue = progress;
        emit progressUpdated(progress);
    }
}

#include "torrentclient.moc"
