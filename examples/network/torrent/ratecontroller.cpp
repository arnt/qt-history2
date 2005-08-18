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
#include "ratecontroller.h"

#include <QtCore/QtCore>

Q_GLOBAL_STATIC(RateController, rateController)

RateController *RateController::instance()
{
    return rateController();
}

void RateController::addSocket(PeerWireClient *socket)
{
    connect(socket, SIGNAL(readyToTransfer()), this, SLOT(scheduleTransfer()));
    socket->setReadBufferSize(downLimit * 4);
    sockets << socket;
    scheduleTransfer();
}

void RateController::removeSocket(PeerWireClient *socket)
{
    disconnect(socket, SIGNAL(readyToTransfer()), this, SLOT(scheduleTransfer()));
    socket->setReadBufferSize(0);
    sockets.remove(socket);
}

void RateController::setDownloadLimit(int bytesPerSecond)
{
    downLimit = bytesPerSecond;
    foreach (PeerWireClient *socket, sockets)
        socket->setReadBufferSize(downLimit * 4);
}

void RateController::scheduleTransfer()
{
    if (transferScheduled)
        return;
    transferScheduled = true;
    QTimer::singleShot(50, this, SLOT(transfer()));
}

void RateController::transfer()
{
    transferScheduled = false;
    if (sockets.isEmpty())
        return;

    int msecs = 1000;
    if (!stopWatch.isNull())
        msecs = qMin(msecs, stopWatch.elapsed());

    qint64 bytesToWrite = (upLimit * msecs) / 1000;
    qint64 bytesToRead = (downLimit * msecs) / 1000;
    if (bytesToWrite == 0 && bytesToRead == 0) {
        scheduleTransfer();
        return;
    }

    QSet<PeerWireClient *> pendingSockets;
    foreach (PeerWireClient *client, sockets) {
        if (client->canTransferMore())
            pendingSockets << client;
    }
    if (pendingSockets.isEmpty())
        return;

    stopWatch.start();

    qint64 writeChunk = qMax<qint64>(1, bytesToWrite / pendingSockets.size());
    qint64 readChunk = qMax<qint64>(1, bytesToRead / pendingSockets.size());

    bool canTransferMore;
    do {
        canTransferMore = false;
        foreach (PeerWireClient *socket, pendingSockets) {
            if (socket->state() != QAbstractSocket::ConnectedState) {
                pendingSockets.remove(socket);
                continue;
            }

            bool dataTransferred = false;
            qint64 readBytes = socket->readFromSocket(qMin<qint64>(readChunk, bytesToRead));
            if (readBytes > 0) {
                bytesToRead -= readBytes;
                dataTransferred = true;
            }

            qint64 chunkSize = qMin<qint64>(writeChunk, bytesToWrite);
            qint64 writtenBytes = socket->writeToSocket(qMin(upLimit - socket->bytesToWrite(), chunkSize));
            if (writtenBytes > 0) {
                bytesToWrite -= writtenBytes;
                dataTransferred = true;
            }

            if (dataTransferred && socket->canTransferMore())
                canTransferMore = true;
            else
                pendingSockets.remove(socket);
        }
    } while (canTransferMore && bytesToWrite > 0 && bytesToRead > 0 && !pendingSockets.isEmpty());

    if (canTransferMore)
        scheduleTransfer();
}
