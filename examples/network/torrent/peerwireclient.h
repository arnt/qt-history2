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
#ifndef PEERWIRECLIENT_H
#define PEERWIRECLIENT_H

class QTimerEvent;

#include <QByteArray>
#include <QDebug>
#include <QHostAddress>
#include <QList>
#include <QSet>
#include <QTcpSocket>
#include <QTimer>

class PeerWireClient : public QTcpSocket
{
    Q_OBJECT
public:
    enum PeerWireStateFlag {
	ChokingPeer = 0x1,
	InterestedInPeer = 0x2,
	ChokedByPeer = 0x4,
	PeerIsInterested = 0x8
    };
    Q_DECLARE_FLAGS(PeerWireState, PeerWireStateFlag)

    PeerWireClient(QObject *parent = 0);
    ~PeerWireClient();
    void initialize(const QByteArray &infoHash, const QByteArray &peerId);

    PeerWireState peerWireState() const;
    QSet<int> availablePieces() const;

    void chokePeer();
    void unchokePeer();
    void sendInterested();
    void sendNotInterested();

    void sendKeepAlive();
    void sendPieceNotification(int piece);
    void sendPieceList(const QSet<int> &bitField, int pieceCount);
    void requestBlock(int piece, int offset, int length);
    void cancelRequest(int piece, int offset, int length);
    void sendBlock(int piece, int offset, const QByteArray &data);

    QHostAddress lastPeerAddress() const;
    quint16 lastPeerPort() const;

    qint64 downloadSpeed() const;
    qint64 downloadCount() const;
    qint64 uploadCount() const;

    int acceptBytesToRead(int bytes);
    int bufferedBytesToWrite() const;
    int acceptBytesToWrite(int bytes);

signals:
    void choked();
    void unchoked();
    void interested();
    void notInterested();

    void piecesAvailable(const QSet<int> &pieces);
    void blockRequest(int pieceIndex, int begin, int length);
    void requestCanceled(int pieceIndex, int begin, int length);
    void blockReceived(int pieceIndex, int begin, const QByteArray &data);

    void bytesReceived(qint64 size);
    
protected slots:
    void timerEvent(QTimerEvent *event);

private slots:
    void sendHandShake();
    void processIncomingData();
    void updateBytesWritten(qint64 written);
    void closeConnection();

private:
    void writeToBuffer(const char *data, int size);
    void writeToBuffer(const QByteArray &data);
    int readFromBuffer(char *data, int size);
    QByteArray readFromBuffer(int size);
    int bytesInBuffer() const;
    QByteArray incomingBuffer;
    QByteArray outgoingBuffer;

    enum PacketType {
	ChokePacket = 0,
	UnchokePacket = 1,
	InterestedPacket = 2,
	NotInterestedPacket = 3,
	HavePacket = 4,
	BitFieldPacket = 5,
	RequestPacket = 6,
	PiecePacket = 7,
	CancelPacket = 8
    };

    PeerWireState pwState;
    bool receivedHandShake;
    bool gotPeerId;
    bool sentHandShake;

    QHostAddress lastAddress;
    quint16 lastPort;
    qint64 downloaded;
    qint64 uploaded;
    qint64 downloadSpeedData[8];
    int downloadSpeedTimer;

    QByteArray infoHash;
    QByteArray peerIdString;
    QByteArray remotePeerIdString;

    int nextPacketLength;

    QSet<int> peerPieces;
    QTimer timeoutTimer;

    struct Block {
	int pieceIndex;
	int begin;
	QByteArray data;
    };
};

inline QDebug operator<<(QDebug debug, PeerWireClient *client)
{
    debug << "PeerWireClient(" << client->peerAddress().toString()
	  << ":" << client->peerPort() << ")";
    return debug;
}

#endif
