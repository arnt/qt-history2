/*-*-mode:c++;c-basic-offset:4-*-*/
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

#include <QDebug>
#include <QTimer>
#include <QTimerEvent>
#include <QUrl>

static const int ClientTimeout = 60 * 1000;
static const int ConnectTimeout = 60 * 1000;
static const int MinimalHeaderSize = 48;
static const int FullHeaderSize = 68;
static const char ProtocolId[] = "BitTorrent protocol";
static const char ProtocolIdSize = 19;

static inline quint32 fromNetworkData(const char *data)
{
    const unsigned char *udata = (const unsigned char *)data;
    return (quint32(udata[0]) << 24)
        | (quint32(udata[1]) << 16)
        | (quint32(udata[2]) << 8)
        | (quint32(udata[3]));
}

static inline void toNetworkData(quint32 num, char *data)
{
    unsigned char *udata = (unsigned char *)data;
    udata[3] = (num & 0xff);
    udata[2] = (num & 0xff00) >> 8;
    udata[1] = (num & 0xff0000) >> 16;
    udata[0] = (num & 0xff000000) >> 24;
}

PeerWireClient::PeerWireClient(QObject *parent)
    : QTcpSocket(parent), pwState(ChokingPeer | ChokedByPeer),
      receivedHandShake(false), gotPeerId(false), sentHandShake(false),
      nextPacketLength(-1), timeoutTimer(0)
{
    connect(&timeoutTimer, SIGNAL(timeout()), this, SLOT(closeConnection()));

    memset(downloadSpeedData, 0, sizeof(downloadSpeedData));
    downloadSpeedTimer = 0;
    downloaded = 0;
    uploaded = 0;

    timeoutTimer.start(ConnectTimeout);
}

PeerWireClient::~PeerWireClient()
{
}

void PeerWireClient::initialize(const QByteArray &infoHash, const QByteArray &peerId)
{
    this->peerIdString = peerId;
    this->infoHash = infoHash;
    if (!sentHandShake)
	sendHandShake();
}

PeerWireClient::PeerWireState PeerWireClient::peerWireState() const
{
    return pwState;
}

QSet<int> PeerWireClient::availablePieces() const
{
    return peerPieces;
}

QHostAddress PeerWireClient::lastPeerAddress() const
{
    return lastAddress;
}

quint16 PeerWireClient::lastPeerPort() const
{
    return lastPort;
}

qint64 PeerWireClient::downloadSpeed() const
{
    qint64 sum = 0;
    for (unsigned int i = 0; i < sizeof(downloadSpeedData) / sizeof(qint64); ++i)
	sum += downloadSpeedData[i];
    return sum / (sizeof(downloadSpeedData) / sizeof(qint64));
}

qint64 PeerWireClient::downloadCount() const
{
    return downloaded;
}

qint64 PeerWireClient::uploadCount() const
{
    return uploaded;
}

int PeerWireClient::acceptBytesToRead(int bytes)
{
    QByteArray chunk = read(bytes);
    incomingBuffer.append(chunk);
    emit bytesReceived(chunk.size());
    QMetaObject::invokeMethod(this, "processIncomingData", Qt::QueuedConnection);
    return chunk.size();
}

int PeerWireClient::bufferedBytesToWrite() const
{
    return outgoingBuffer.size();
}

int PeerWireClient::acceptBytesToWrite(int bytes)
{
    int written = write(outgoingBuffer.constData(), qMin(bytes, outgoingBuffer.size()));
    outgoingBuffer.remove(0, written);
    return written;
}

void PeerWireClient::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == downloadSpeedTimer) {
	memmove(downloadSpeedData + 1, downloadSpeedData, 
		sizeof(downloadSpeedData) - sizeof(qint64));
	downloadSpeedData[0] = 0;

	qint64 sum = 0;
	for (unsigned int i = 0; i < sizeof(downloadSpeedData) / sizeof(qint64); ++i)
	    sum += downloadSpeedData[i];

	if (sum == 0) {
	    killTimer(downloadSpeedTimer);
	    downloadSpeedTimer = 0;
	}
    }
    QTcpSocket::timerEvent(event);
}

void PeerWireClient::sendHandShake()
{
    if (sentHandShake)
        return;
                          
    timeoutTimer.start(ClientTimeout);

    // Save the peer address and port if we need to reconnect
    lastAddress = peerAddress();
    lastPort = peerPort();
    memset(downloadSpeedData, 0, sizeof(downloadSpeedData));

    // The header is 68 bytes long
    putChar(ProtocolIdSize); // 1 byte
    writeToBuffer(ProtocolId, ProtocolIdSize); // 19 bytes
    writeToBuffer(QByteArray(8, '\0')); // 8 bytes
    writeToBuffer(infoHash); // 20 bytes
    writeToBuffer(peerIdString); // 20 bytes

    sentHandShake = true;
}

void PeerWireClient::processIncomingData()
{
    timeoutTimer.start(ClientTimeout);

    if (!receivedHandShake) {
	// Check that we received enough data
	if (bytesInBuffer() < MinimalHeaderSize)
	    return;

	// Sanity check the protocol ID
	QByteArray id = readFromBuffer(ProtocolIdSize + 1);
	if (id.at(0) != ProtocolIdSize || !id.mid(1).startsWith(ProtocolId)) {
	    disconnectFromHost();
	    return;
	}

	// Discard 8 reserved bytes, then read the info hash and peer ID
	(void) readFromBuffer(8);

	// Read infoHash
	QByteArray peerInfoHash = readFromBuffer(20);
	if (peerInfoHash != infoHash) {
	    disconnectFromHost();
	    return;
	}

	// Send handshake
	if (!sentHandShake)
	    sendHandShake();
	receivedHandShake = true;
    }

    // Handle delayed peer id arrival
    if (!gotPeerId) {
	if (bytesInBuffer() < 20)
	    return;
	gotPeerId = true;
	remotePeerIdString = readFromBuffer(20);
    }

    do {
	// Find the packet length
	if (nextPacketLength == -1) {
	    if (bytesInBuffer() < 4)
		return;

	    char tmp[4];
	    readFromBuffer(tmp, sizeof(tmp));
	    nextPacketLength = fromNetworkData(tmp);

	    if (nextPacketLength < 0 || nextPacketLength > 200000) {
		// Prevent DoS
		disconnectFromHost();
		return;
	    }
	}
    
	// KeepAlive
	if (nextPacketLength == 0) {
	    nextPacketLength = -1;
	    continue;
	}

	// Wait with parsing until the whole packet has been received
	if (bytesInBuffer() < nextPacketLength)
	    return;

	// Read the packet
	QByteArray packet = readFromBuffer(nextPacketLength);
	if (packet.size() != nextPacketLength) {
	    disconnectFromHost();
	    return;
	}

	switch (packet.at(0)) {
	case ChokePacket:
	    pwState |= ChokedByPeer;
	    emit choked();
	    break;
	case UnchokePacket: 
	    pwState &= ~ChokedByPeer;
	    emit unchoked();
	    break;
	case InterestedPacket: 
	    pwState |= PeerIsInterested;
	    emit interested();
	    break;
	case NotInterestedPacket: 
	    pwState &= ~PeerIsInterested;
	    emit notInterested();
	    break;
	case HavePacket: {
	    quint32 index = fromNetworkData(&packet.data()[1]);
	    peerPieces << int(index);
	    emit piecesAvailable(peerPieces);
	    break;
	}
	case BitFieldPacket:
	    for (int i = 1; i < packet.size(); ++i) {
		for (int bit = 0; bit < 8; ++bit) {
		    if (packet.at(i) & (1 << bit))
			peerPieces << int(((i - 1) * 8) + bit);
		}
	    }
	    emit piecesAvailable(peerPieces);
	    break;
	case RequestPacket: {
	    quint32 index = fromNetworkData(&packet.data()[1]);
	    quint32 begin = fromNetworkData(&packet.data()[5]);
	    quint32 length = fromNetworkData(&packet.data()[9]);
	    emit blockRequest(int(index), int(begin), int(length));
	    break;
	}
	case PiecePacket: {
	    quint32 index = fromNetworkData(&packet.data()[1]);
	    quint32 begin = fromNetworkData(&packet.data()[5]);
	    emit blockReceived(int(index), int(begin), packet.mid(9));

	    if (!downloadSpeedTimer)
		downloadSpeedTimer = startTimer(1000);

	    downloadSpeedData[0] += packet.size() - 9;
	    downloaded += packet.size() - 9;
	    break;
	}
	case CancelPacket: {
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
    } while (bytesInBuffer() > 0);
}

void PeerWireClient::updateBytesWritten(qint64 written)
{
    Q_UNUSED(written);
    timeoutTimer.start(ClientTimeout);
}

void PeerWireClient::closeConnection()
{
    disconnectFromHost();
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

int PeerWireClient::bytesInBuffer() const
{
    return incomingBuffer.size();
}

void PeerWireClient::chokePeer()
{
    const char message[] = {0, 0, 0, 1, 0};
    writeToBuffer(message, sizeof(message));
    pwState |= ChokingPeer;
}

void PeerWireClient::unchokePeer()
{
    const char message[] = {0, 0, 0, 1, 1};
    writeToBuffer(message, sizeof(message));
    pwState &= ~ChokingPeer;
}

void PeerWireClient::sendInterested()
{
    const char message[] = {0, 0, 0, 1, 2};
    writeToBuffer(message, sizeof(message));
    pwState |= InterestedInPeer;
}

void PeerWireClient::sendNotInterested()
{
    const char message[] = {0, 0, 0, 1, 3};
    writeToBuffer(message, sizeof(message));
    pwState &= ~InterestedInPeer;
}

void PeerWireClient::sendKeepAlive()
{
    const char message[] = {0, 0, 0, 0};
    writeToBuffer(message, sizeof(message));
}

void PeerWireClient::sendPieceNotification(int piece)
{
    if (!sentHandShake)
	sendHandShake();

    char message[] = {0, 0, 0, 5, 4, 0, 0, 0, 0};
    toNetworkData(piece, &message[5]);
    writeToBuffer(message, sizeof(message));
}

void PeerWireClient::sendPieceList(const QSet<int> &bitField, int pieceCount)
{
    // The bitfield message may only be sent immediately after the
    // handshaking sequence is completed, and before any other
    // messages are sent.
    if (!sentHandShake)
	sendHandShake();

    // Don't send the bit field unless we have pieces.
    if (bitField.size() == 0)
	return;

    quint32 lastNonZeroByte = 0;
    int size = pieceCount / 8;
    if (pieceCount % 8)
	++size;
    QByteArray bits(size, '\0');
    foreach (int pieceIndex, bitField) {
	quint32 byte = quint32(pieceIndex) / 8;
	quint32 bit = quint32(pieceIndex) % 8;
	//bits[byte] = bits.at(byte) | (1 << (7 - bit));
	bits[byte] = uchar(bits[byte] | (1 << bit));
	if (byte > lastNonZeroByte)
	    lastNonZeroByte = byte;
    }

    char message[] = {0, 0, 0, 1, 5};
    toNetworkData(1 + bits.size(), &message[0]);
    writeToBuffer(message, sizeof(message));
    writeToBuffer(bits);
    flush();
}

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

void PeerWireClient::sendBlock(int piece, int offset, const QByteArray &data)
{
    char message[] = {0, 0, 0, 1, 7};
    toNetworkData(9 + data.size(), &message[0]);
    outgoingBuffer += QByteArray(message, sizeof(message));

    char numbers[4 * 2];
    toNetworkData(piece, &numbers[0]);
    toNetworkData(offset, &numbers[4]);
    writeToBuffer(numbers, sizeof(numbers));
    writeToBuffer(data);
    uploaded += data.size();
}
