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
#include "peerwireclient.h"

#include <QtCore/QListIterator>
#include <QtCore/QSet>
#include <QtCore/QTimerEvent>

static const int ClientTimeout = 60 * 1000;
static const int ConnectTimeout = 60 * 1000;
static const int RateControlTimerDelay = 2000;
static const int MinimalHeaderSize = 48;
static const int FullHeaderSize = 68;
static const char ProtocolId[] = "BitTorrent protocol";
static const char ProtocolIdSize = 19;

// Reads a 32bit unsigned int from data in network order.
static inline quint32 fromNetworkData(const char *data)
{
    const unsigned char *udata = (const unsigned char *)data;
    return (quint32(udata[0]) << 24)
        | (quint32(udata[1]) << 16)
        | (quint32(udata[2]) << 8)
        | (quint32(udata[3]));
}

// Writes a 32bit unsigned int from num to data in network order.
static inline void toNetworkData(quint32 num, char *data)
{
    unsigned char *udata = (unsigned char *)data;
    udata[3] = (num & 0xff);
    udata[2] = (num & 0xff00) >> 8;
    udata[1] = (num & 0xff0000) >> 16;
    udata[0] = (num & 0xff000000) >> 24;
}

// Constructs an unconnected PeerWire client and starts the connect timer.
PeerWireClient::PeerWireClient(QObject *parent)
    : QTcpSocket(parent), pwState(ChokingPeer | ChokedByPeer),
      receivedHandShake(false), gotPeerId(false), sentHandShake(false),
      nextPacketLength(-1), invalidateTimeout(false)
{
    memset(uploadSpeedData, 0, sizeof(uploadSpeedData));
    memset(downloadSpeedData, 0, sizeof(downloadSpeedData));

    transferSpeedTimer = startTimer(RateControlTimerDelay);
    timeoutTimer = startTimer(ConnectTimeout);
}

// Registers the peer ID and SHA1 sum of the torrent, and initiates
// the handshake.
void PeerWireClient::initialize(const QByteArray &infoHash, const QByteArray &peerId,
                                int pieceCount)
{
    this->peerIdString = peerId;
    this->infoHash = infoHash;
    peerPieces.resize(pieceCount);
    if (!sentHandShake)
	sendHandShake();
}

QSet<int> PeerWireClient::availablePieces() const
{
    QSet<int> tmp;
    for (int i = 0; i < peerPieces.size(); ++i) {
       if (peerPieces.testBit(i))
           tmp << i;
    }
    return tmp;
}

// Sends a "choke" message, asking the peer to stop requesting blocks.
void PeerWireClient::chokePeer()
{
    const char message[] = {0, 0, 0, 1, 0};
    writeToBuffer(message, sizeof(message));
    pwState |= ChokingPeer;

    // After receiving a choke message, the peer will assume all
    // pending requests are lost.
    pendingBlocks.clear();
}

// Sends an "unchoke" message, allowing the peer to start/resume
// requesting blocks.
void PeerWireClient::unchokePeer()
{
    const char message[] = {0, 0, 0, 1, 1};
    writeToBuffer(message, sizeof(message));
    pwState &= ~ChokingPeer;
}

// Sends an "interested" message, informing the peer that it has got
// pieces that we'd like to download.
void PeerWireClient::sendInterested()
{
    const char message[] = {0, 0, 0, 1, 2};
    writeToBuffer(message, sizeof(message));
    pwState |= InterestedInPeer;
}

// Sends a "not interested" message, informing the peer that it does
// not have any pieces that we'd like to download.
void PeerWireClient::sendNotInterested()
{
    const char message[] = {0, 0, 0, 1, 3};
    writeToBuffer(message, sizeof(message));
    pwState &= ~InterestedInPeer;
}

// Sends a piece notification / a "have" message, informing the peer
// that we have just downloaded a new piece.
void PeerWireClient::sendPieceNotification(int piece)
{
    if (!sentHandShake)
	sendHandShake();

    char message[] = {0, 0, 0, 5, 4, 0, 0, 0, 0};
    toNetworkData(piece, &message[5]);
    writeToBuffer(message, sizeof(message));
}

// Sends the complete list of pieces that we have downloaded.
void PeerWireClient::sendPieceList(const QSet<int> &bitField)
{
    // The bitfield message may only be sent immediately after the
    // handshaking sequence is completed, and before any other
    // messages are sent.
    if (!sentHandShake)
	sendHandShake();

    // Don't send the bit field unless we have pieces.
    if (bitField.size() == 0)
	return;

    int size = peerPieces.size() / 8;
    if (peerPieces.size() % 8)
	++size;
    QByteArray bits(size, '\0');
    foreach (int pieceIndex, bitField) {
	quint32 byte = quint32(pieceIndex) / 8;
	quint32 bit = quint32(pieceIndex) % 8;
	bits[byte] = uchar(bits.at(byte)) | (1 << (7 - bit));
    }

    char message[] = {0, 0, 0, 1, 5};
    toNetworkData(1 + bits.size(), &message[0]);
    writeToBuffer(message, sizeof(message));
    writeToBuffer(bits);
}

// Sends a request for a block.
void PeerWireClient::requestBlock(int piece, int offset, int length)
{
    char message[] = {0, 0, 0, 1, 6};
    toNetworkData(13, &message[0]);
    writeToBuffer(message, sizeof(message));

    char numbers[4 * 3];
    toNetworkData(piece, &numbers[0]);
    toNetworkData(offset, &numbers[4]);
    toNetworkData(length, &numbers[8]);
    writeToBuffer(numbers, sizeof(numbers));
}

// Cancels a request for a block.
void PeerWireClient::cancelRequest(int piece, int offset, int length)
{
    char message[] = {0, 0, 0, 1, 8};
    toNetworkData(13, &message[0]);
    writeToBuffer(message, sizeof(message));

    char numbers[4 * 3];
    toNetworkData(piece, &numbers[0]);
    toNetworkData(offset, &numbers[4]);
    toNetworkData(length, &numbers[8]);
    writeToBuffer(numbers, sizeof(numbers));
}

// Sends a block to the peer.
void PeerWireClient::sendBlock(int piece, int offset, const QByteArray &data)
{
    QByteArray block;

    char message[] = {0, 0, 0, 1, 7};
    toNetworkData(9 + data.size(), &message[0]);
    block += QByteArray(message, sizeof(message));

    char numbers[4 * 2];
    toNetworkData(piece, &numbers[0]);
    toNetworkData(offset, &numbers[4]);
    block += QByteArray(numbers, sizeof(numbers));
    block += data;

    pendingBlocks << block;
}

// Returns the number of bytes waiting to be written.
int PeerWireClient::bufferedBytesToWrite() const
{
    int toWrite = outgoingBuffer.size();
    QListIterator<QByteArray> it(pendingBlocks);
    while (it.hasNext())
	toWrite += it.next().size();
    return toWrite;
}

// Attempts to write 'bytes' bytes to the socket from the buffer.
// This is used by RateController, which precisely controls how much
// each client can write.
int PeerWireClient::acceptBytesToWrite(int bytes)
{
    int totalWritten = 0;
    do {
	if (outgoingBuffer.size() < (bytes - totalWritten))
	    outgoingBuffer += pendingBlocks.takeFirst();
	qint64 written = write(outgoingBuffer.constData(), qMin(bytes - totalWritten, outgoingBuffer.size()));
	totalWritten += written;
	uploadSpeedData[0] += written;
	outgoingBuffer.remove(0, written);
    } while (totalWritten < bytes && (!outgoingBuffer.isEmpty() || !pendingBlocks.isEmpty()));

    return totalWritten;
}

// Attempts to read at most 'bytes' bytes from the socket.
int PeerWireClient::acceptBytesToRead(int bytes)
{
    QByteArray chunk = read(bytes);

    downloadSpeedData[0] += chunk.size();

    incomingBuffer.append(chunk);
    processIncomingData();
    emit bytesReceived(chunk.size());
    return chunk.size();
}

// Returns the average number of bytes per second this client is
// downloading.
qint64 PeerWireClient::downloadSpeed() const
{
    qint64 sum = 0;
    for (unsigned int i = 0; i < sizeof(downloadSpeedData) / sizeof(qint64); ++i)
	sum += downloadSpeedData[i];
    return sum / (8 * 2);
}

// Returns the average number of bytes per second this client is
// uploading.
qint64 PeerWireClient::uploadSpeed() const
{
    qint64 sum = 0;
    for (unsigned int i = 0; i < sizeof(uploadSpeedData) / sizeof(qint64); ++i)
	sum += uploadSpeedData[i];
    return sum / (8 * 2);
}

void PeerWireClient::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == transferSpeedTimer) {
	// Rotate the upload / download records.
	for (int i = 6; i >= 0; --i) {
	    uploadSpeedData[i + i] = uploadSpeedData[i];
	    downloadSpeedData[i + 1] = uploadSpeedData[i];
	}
	uploadSpeedData[0] = 0;
	downloadSpeedData[0] = 0;
    } else if (event->timerId() == timeoutTimer) {
	// Disconnect if we timed out; otherwise the timeout is
	// restarted.
	if (invalidateTimeout) {
	    invalidateTimeout = false;
	} else {
	    closeConnection();
	}
    }
    QTcpSocket::timerEvent(event);
}

// Sends the handshake to the peer.
void PeerWireClient::sendHandShake()
{
    sentHandShake = true;

    // Restart the timeout
    if (timeoutTimer)
	killTimer(timeoutTimer);
    timeoutTimer = startTimer(ClientTimeout);

    // Write the 68 byte PeerWire handshake.
    writeToBuffer(&ProtocolIdSize, 1);
    writeToBuffer(ProtocolId, ProtocolIdSize);
    writeToBuffer(QByteArray(8, '\0'));
    writeToBuffer(infoHash);
    writeToBuffer(peerIdString);
}

void PeerWireClient::processIncomingData()
{
    invalidateTimeout = true;

    if (!receivedHandShake) {
	// Check that we received enough data
	if (incomingBuffer.size() < MinimalHeaderSize)
	    return;

	// Sanity check the protocol ID
	QByteArray id = readFromBuffer(ProtocolIdSize + 1);
	if (id.at(0) != ProtocolIdSize || !id.mid(1).startsWith(ProtocolId)) {
	    abort();
	    return;
	}

	// Discard 8 reserved bytes, then read the info hash and peer ID
	(void) readFromBuffer(8);

	// Read infoHash
	QByteArray peerInfoHash = readFromBuffer(20);
	if (peerInfoHash != infoHash) {
	    abort();
	    return;
	}

	// Send handshake
	if (!sentHandShake)
	    sendHandShake();
	receivedHandShake = true;
    }

    // Handle delayed peer id arrival
    if (!gotPeerId) {
	if (incomingBuffer.size() < 20)
	    return;
	gotPeerId = true;
	readFromBuffer(20);
    }

    do {
	// Find the packet length
	if (nextPacketLength == -1) {
	    if (incomingBuffer.size() < 4)
		return;

	    char tmp[4];
	    readFromBuffer(tmp, sizeof(tmp));
	    nextPacketLength = fromNetworkData(tmp);

	    if (nextPacketLength < 0 || nextPacketLength > 200000) {
		// Prevent DoS
		abort();
		return;
	    }
	}
    
	// KeepAlive
	if (nextPacketLength == 0) {
	    nextPacketLength = -1;
	    continue;
	}

	// Wait with parsing until the whole packet has been received
	if (incomingBuffer.size() < nextPacketLength)
	    return;

	// Read the packet
	QByteArray packet = readFromBuffer(nextPacketLength);
	if (packet.size() != nextPacketLength) {
	    abort();
	    return;
	}

	switch (packet.at(0)) {
	case ChokePacket:
	    // We have been choked.
	    pwState |= ChokedByPeer;
	    emit choked();
	    break;
	case UnchokePacket: 
	    // We have been unchoked.
	    pwState &= ~ChokedByPeer;
	    emit unchoked();
	    break;
	case InterestedPacket: 
	    // The peer is interested in downloading.
	    pwState |= PeerIsInterested;
	    emit interested();
	    break;
	case NotInterestedPacket: 
	    // The peer is not interested in downloading.
	    pwState &= ~PeerIsInterested;
	    emit notInterested();
	    break;
	case HavePacket: {
	    // The peer has a new piece available.
	    quint32 index = fromNetworkData(&packet.data()[1]);
	    peerPieces.setBit(int(index));
	    emit piecesAvailable(availablePieces());
	    break;
	}
	case BitFieldPacket:
	    // The peer has the following pieces available.
	    for (int i = 1; i < packet.size(); ++i) {
		for (int bit = 0; bit < 8; ++bit) {
		    if (packet.at(i) & (1 << (7 - bit)))
			peerPieces.setBit(int(((i - 1) * 8) + bit));
		}
	    }
	    emit piecesAvailable(availablePieces());
	    break;
	case RequestPacket: {
	    // The peer requests a block.
	    quint32 index = fromNetworkData(&packet.data()[1]);
	    quint32 begin = fromNetworkData(&packet.data()[5]);
	    quint32 length = fromNetworkData(&packet.data()[9]);
	    emit blockRequested(int(index), int(begin), int(length));
	    break;
	}
	case PiecePacket: {
	    // The peer sends a block.
	    quint32 index = fromNetworkData(&packet.data()[1]);
	    quint32 begin = fromNetworkData(&packet.data()[5]);
	    emit blockReceived(int(index), int(begin), packet.mid(9));
	    break;
	}
	case CancelPacket: {
	    // The peer cancels a block request.
	    quint32 index = fromNetworkData(&packet.data()[1]);
	    quint32 begin = fromNetworkData(&packet.data()[5]);
	    quint32 length = fromNetworkData(&packet.data()[9]);
	    emit requestCanceled(int(index), int(begin), int(length));
	    break;
	}
	default:
	    // Unsupported packet type; just ignore it.
	    break;
	}
	nextPacketLength = -1;
    } while (incomingBuffer.size() > 0);
}

void PeerWireClient::closeConnection()
{
    abort();
}

void PeerWireClient::writeToBuffer(const char *data, int size)
{
    writeToBuffer(QByteArray(data, size));
}

void PeerWireClient::writeToBuffer(const QByteArray &data)
{
    outgoingBuffer += data;
}

int PeerWireClient::readFromBuffer(char *data, int size)
{
    int n = qMin(size, incomingBuffer.size());
    memcpy(data, incomingBuffer.constData(), n);
    incomingBuffer.remove(0, n);
    return n;
}

QByteArray PeerWireClient::readFromBuffer(int size)
{
    int n = qMin(size, incomingBuffer.size());
    QByteArray tmp = incomingBuffer.left(n);
    incomingBuffer.remove(0, n);
    return tmp;
}
