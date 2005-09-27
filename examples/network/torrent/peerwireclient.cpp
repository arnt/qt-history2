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

#include "peerwireclient.h"

#include <QTimerEvent>

static const int PendingRequestTimeout = 60 * 1000;
static const int ClientTimeout = 120 * 1000;
static const int ConnectTimeout = 60 * 1000;
static const int KeepAliveInterval = 30 * 1000;
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
PeerWireClient::PeerWireClient(const QByteArray &peerId, QObject *parent)
    : QTcpSocket(parent), pendingBlockSizes(0), numRequestedBlocks(0),
      pwState(ChokingPeer | ChokedByPeer), receivedHandShake(false), gotPeerId(false),
      sentHandShake(false), nextPacketLength(-1), pendingRequestTimer(0), invalidateTimeout(false),
      keepAliveTimer(0), torrentPeer(0)
{
    memset(uploadSpeedData, 0, sizeof(uploadSpeedData));
    memset(downloadSpeedData, 0, sizeof(downloadSpeedData));

    transferSpeedTimer = startTimer(RateControlTimerDelay);
    timeoutTimer = startTimer(ConnectTimeout);
    peerIdString = peerId;

    connect(this, SIGNAL(readyRead()), this, SIGNAL(readyToTransfer()));
    connect(this, SIGNAL(connected()), this, SIGNAL(readyToTransfer()));
}

// Registers the peer ID and SHA1 sum of the torrent, and initiates
// the handshake.
void PeerWireClient::initialize(const QByteArray &infoHash, int pieceCount)
{
    this->infoHash = infoHash;
    peerPieces.resize(pieceCount);
    if (!sentHandShake)
        sendHandShake();
}

void PeerWireClient::setPeer(TorrentPeer *peer)
{
    torrentPeer = peer;
}

TorrentPeer *PeerWireClient::peer() const
{
    return torrentPeer;
}

QBitArray PeerWireClient::availablePieces() const
{
    return peerPieces;
}

int PeerWireClient::incomingBlockCount() const
{
    return numRequestedBlocks;
}

// Sends a "choke" message, asking the peer to stop requesting blocks.
void PeerWireClient::chokePeer()
{
    const char message[] = {0, 0, 0, 1, 0};
    write(message, sizeof(message));
    pwState |= ChokingPeer;

    // After receiving a choke message, the peer will assume all
    // pending requests are lost.
    pendingBlocks.clear();
    pendingBlockSizes = 0;
}

// Sends an "unchoke" message, allowing the peer to start/resume
// requesting blocks.
void PeerWireClient::unchokePeer()
{
    const char message[] = {0, 0, 0, 1, 1};
    write(message, sizeof(message));
    pwState &= ~ChokingPeer;

    if (pendingRequestTimer)
        killTimer(pendingRequestTimer);
}

// Sends a "keep-alive" message to prevent the peer from closing
// the connection when there's no activity
void PeerWireClient::sendKeepAlive()
{
    const char message[] = {0, 0, 0, 0};
    write(message, sizeof(message));
}

// Sends an "interested" message, informing the peer that it has got
// pieces that we'd like to download.
void PeerWireClient::sendInterested()
{
    const char message[] = {0, 0, 0, 1, 2};
    write(message, sizeof(message));
    pwState |= InterestedInPeer;

    // After telling the peer that we're interested, we expect to get
    // unchoked within a certain timeframe; otherwise we'll drop the
    // connection.
    if (pendingRequestTimer)
        killTimer(pendingRequestTimer);
    pendingRequestTimer = startTimer(PendingRequestTimeout);
}

// Sends a "not interested" message, informing the peer that it does
// not have any pieces that we'd like to download.
void PeerWireClient::sendNotInterested()
{
    const char message[] = {0, 0, 0, 1, 3};
    write(message, sizeof(message));
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
    write(message, sizeof(message));
}

// Sends the complete list of pieces that we have downloaded.
void PeerWireClient::sendPieceList(const QBitArray &bitField)
{
    // The bitfield message may only be sent immediately after the
    // handshaking sequence is completed, and before any other
    // messages are sent.
    if (!sentHandShake)
        sendHandShake();

    // Don't send the bit field unless we have pieces.
    int bitFieldSize = bitField.size();
    if (bitFieldSize == 0)
        return;

    // ### Can't we simply use bitField.data()? ;-)
    int size = bitFieldSize / 8;
    if (bitFieldSize % 8)
        ++size;
    QByteArray bits(size, '\0');
    for (int i = 0; i < bitFieldSize; ++i) {
        if (bitField.testBit(i)) {
            quint32 byte = quint32(i) / 8;
            quint32 bit = quint32(i) % 8;
            bits[byte] = uchar(bits.at(byte)) | (1 << (7 - bit));
        }
    }

    char message[] = {0, 0, 0, 1, 5};
    toNetworkData(1 + bits.size(), &message[0]);
    write(message, sizeof(message));
    write(bits);
}

// Sends a request for a block.
void PeerWireClient::requestBlock(int piece, int offset, int length)
{
    char message[] = {0, 0, 0, 1, 6};
    toNetworkData(13, &message[0]);
    write(message, sizeof(message));

    char numbers[4 * 3];
    toNetworkData(piece, &numbers[0]);
    toNetworkData(offset, &numbers[4]);
    toNetworkData(length, &numbers[8]);
    write(numbers, sizeof(numbers));

    ++numRequestedBlocks;

    // After requesting a block, we expect the block to be sent by the
    // other peer within a certain number of seconds. Otherwise, we
    // drop the connection.
    if (pendingRequestTimer)
        killTimer(pendingRequestTimer);
    pendingRequestTimer = startTimer(PendingRequestTimeout);
}

// Cancels a request for a block.
void PeerWireClient::cancelRequest(int piece, int offset, int length)
{
    char message[] = {0, 0, 0, 1, 8};
    toNetworkData(13, &message[0]);
    write(message, sizeof(message));

    char numbers[4 * 3];
    toNetworkData(piece, &numbers[0]);
    toNetworkData(offset, &numbers[4]);
    toNetworkData(length, &numbers[8]);
    write(numbers, sizeof(numbers));

    if (numRequestedBlocks)
        --numRequestedBlocks;
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

    BlockInfo blockInfo;
    blockInfo.pieceIndex = piece;
    blockInfo.offset = offset;
    blockInfo.length = data.size();
    blockInfo.block = block;

    pendingBlocks << blockInfo;
    pendingBlockSizes += block.size();

    if (pendingBlockSizes > 32 * 16384) {
        chokePeer();
        unchokePeer();
        return;
    }
    emit readyToTransfer();
}

// Attempts to write 'bytes' bytes to the socket from the buffer.
// This is used by RateController, which precisely controls how much
// each client can write.
qint64 PeerWireClient::writeToSocket(qint64 bytes)
{
    qint64 totalWritten = 0;
    do {
        if (outgoingBuffer.isEmpty() && !pendingBlocks.isEmpty()) {
            BlockInfo block = pendingBlocks.takeFirst();
            pendingBlockSizes -= block.length;
            outgoingBuffer += block.block;
        }
        qint64 written = QTcpSocket::writeData(outgoingBuffer.constData(),
                                               qMin<qint64>(bytes - totalWritten, outgoingBuffer.size()));
        if (written <= 0)
            return totalWritten ? totalWritten : written;
        
        totalWritten += written;
        uploadSpeedData[0] += written;
        outgoingBuffer.remove(0, written);
    } while (totalWritten < bytes && (!outgoingBuffer.isEmpty() || !pendingBlocks.isEmpty()));

    return totalWritten;
}

// Attempts to read at most 'bytes' bytes from the socket.
qint64 PeerWireClient::readFromSocket(qint64 bytes)
{
    char buffer[1024];
    qint64 totalRead = 0;
    do {
        qint64 bytesRead = QTcpSocket::readData(buffer, qMin<qint64>(sizeof(buffer), bytes - totalRead));
        if (bytesRead <= 0)
            break;
        qint64 oldSize = incomingBuffer.size();
        incomingBuffer.resize(oldSize + bytesRead);
        memcpy(incomingBuffer.data() + oldSize, buffer, bytesRead);

        totalRead += bytesRead;
    } while (totalRead < bytes);

    if (totalRead > 0) {
        downloadSpeedData[0] += totalRead;
        emit bytesReceived(totalRead);
        processIncomingData();
    }
    return totalRead;
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

bool PeerWireClient::canTransferMore() const
{
    return !incomingBuffer.isEmpty() || QTcpSocket::bytesAvailable() > 0
        || !outgoingBuffer.isEmpty() || !pendingBlocks.isEmpty();
}

void PeerWireClient::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == transferSpeedTimer) {
        // Rotate the upload / download records.
        for (int i = 6; i >= 0; --i) {
            uploadSpeedData[i + i] = uploadSpeedData[i];
            downloadSpeedData[i + 1] = downloadSpeedData[i];
        }
        uploadSpeedData[0] = 0;
        downloadSpeedData[0] = 0;
    } else if (event->timerId() == timeoutTimer) {
        // Disconnect if we timed out; otherwise the timeout is
        // restarted.
        if (invalidateTimeout) {
            invalidateTimeout = false;
        } else {
            abort();
        }
    } else if (event->timerId() == pendingRequestTimer) {
        abort();
    } else if (event->timerId() == keepAliveTimer) {
        sendKeepAlive();
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
    write(&ProtocolIdSize, 1);
    write(ProtocolId, ProtocolIdSize);
    write(QByteArray(8, '\0'));
    write(infoHash);
    write(peerIdString);
}

void PeerWireClient::processIncomingData()
{
    invalidateTimeout = true;

    if (!receivedHandShake) {
        // Check that we received enough data
        if (incomingBuffer.size() < MinimalHeaderSize)
            return;

        // Sanity check the protocol ID
        QByteArray id = read(ProtocolIdSize + 1);
        if (id.at(0) != ProtocolIdSize || !id.mid(1).startsWith(ProtocolId)) {
            abort();
            return;
        }

        // Discard 8 reserved bytes, then read the info hash and peer ID
        (void) read(8);

        // Read infoHash
        QByteArray peerInfoHash = read(20);
        if (!infoHash.isEmpty() && peerInfoHash != infoHash) {
            abort();
            return;
        }

        emit infoHashReceived(peerInfoHash);
        if (infoHash.isEmpty()) {
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
        if (read(20) == peerIdString) {
            // We connected to ourself
            abort();
            return;
        }
    }

    // Initialize keep-alive timer
    if (!keepAliveTimer)
        keepAliveTimer = startTimer(KeepAliveInterval);

    do {
        // Find the packet length
        if (nextPacketLength == -1) {
            if (incomingBuffer.size() < 4)
                return;

            char tmp[4];
            read(tmp, sizeof(tmp));
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
        QByteArray packet = read(nextPacketLength);
        if (packet.size() != nextPacketLength) {
            abort();
            return;
        }

        switch (packet.at(0)) {
        case ChokePacket:
            // We have been choked.
            pwState |= ChokedByPeer;
            numRequestedBlocks = 0;
            if (pendingRequestTimer)
                killTimer(pendingRequestTimer);
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
            if (numRequestedBlocks)
                --numRequestedBlocks;

            // The peer sends a block.
            quint32 index = fromNetworkData(&packet.data()[1]);
            quint32 begin = fromNetworkData(&packet.data()[5]);
            emit blockReceived(int(index), int(begin), packet.mid(9));

            // Kill the pending block timer.
            if (pendingRequestTimer) {
                killTimer(pendingRequestTimer);
                pendingRequestTimer = 0;
            }
            break;
        }
        case CancelPacket: {
            // The peer cancels a block request.
            quint32 index = fromNetworkData(&packet.data()[1]);
            quint32 begin = fromNetworkData(&packet.data()[5]);
            quint32 length = fromNetworkData(&packet.data()[9]);
            for (int i = 0; i < pendingBlocks.size(); ++i) {
                const BlockInfo &blockInfo = pendingBlocks.at(i);
                if (blockInfo.pieceIndex == int(index)
                    && blockInfo.offset == int(begin)
                    && blockInfo.length == int(length)) {
                    pendingBlocks.removeAt(i);
                    break;
                }
            }
            break;
        }
        default:
            // Unsupported packet type; just ignore it.
            break;
        }
        nextPacketLength = -1;
    } while (incomingBuffer.size() > 0);
}

qint64 PeerWireClient::readData(char *data, qint64 size)
{
    int n = qMin<int>(size, incomingBuffer.size());
    memcpy(data, incomingBuffer.constData(), n);
    incomingBuffer.remove(0, n);
    return n;
}

qint64 PeerWireClient::readLineData(char *data, qint64 maxlen)
{
    return QIODevice::readLineData(data, maxlen);
}

qint64 PeerWireClient::writeData(const char *data, qint64 size)
{
    int oldSize = outgoingBuffer.size();
    outgoingBuffer.resize(oldSize + size);
    memcpy(outgoingBuffer.data() + oldSize, data, size);
    emit readyToTransfer();
    return size;
}
