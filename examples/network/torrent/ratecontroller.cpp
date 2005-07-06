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

#include <QtCore>

Q_GLOBAL_STATIC(RateController, rateController);

static const int MaxTimerDelay = 500;
static const int DefaultTimerDelay = 250;
static const int MinimumTimerDelay = 100;

RateController::RateController()
{
    uploadLimitBytes = 1024 * 15;
    downloadLimitBytes = 1024 * 1024;
    transmitTimer = startTimer(DefaultTimerDelay);
    timerDelay = DefaultTimerDelay;
}

RateController *RateController::instance()
{
    return rateController();
}

void RateController::setUploadLimit(int bytesPerSecond)
{
    uploadLimitBytes = bytesPerSecond;
}

int RateController::uploadLimit() const
{
    return uploadLimitBytes;
}

void RateController::setDownloadLimit(int bytesPerSecond)
{
    downloadLimitBytes = bytesPerSecond;
}

int RateController::downloadLimit() const
{
    return downloadLimitBytes;
}

void RateController::addClient(PeerWireClient *client)
{
    client->setReadBufferSize(downloadLimitBytes * 10);
    clients << client;
}

void RateController::removeClient(PeerWireClient *client)
{
    clients.removeAll(client);
}

void RateController::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == transmitTimer && clients.size() > 0) {
	QList<PeerWireClient *> transmittingClients;
	foreach (PeerWireClient *client, clients) {
	    if (client->state() == QAbstractSocket::ConnectedState
		&& (client->bufferedBytesToWrite() > 0 || client->bytesAvailable() > 0)) {
		transmittingClients << client;
	    }
	}
	if (transmittingClients.isEmpty()) {
	    int newTimerDelay = qMin(timerDelay + 10, MaxTimerDelay);
	    if (newTimerDelay != timerDelay) {
		timerDelay = newTimerDelay;
		killTimer(transmitTimer);
		transmitTimer = startTimer(timerDelay);
	    }
	    return;
	}
	int newTimerDelay = qMax(timerDelay - 10, MinimumTimerDelay);
	if (newTimerDelay != timerDelay) {
	    timerDelay = newTimerDelay;
	    killTimer(transmitTimer);
	    transmitTimer = startTimer(timerDelay);
	}

	int bytesLeftToWrite = uploadLimitBytes / (1000 / timerDelay);
	int writeChunk = bytesLeftToWrite / transmittingClients.size();
	int bytesLeftToRead = downloadLimitBytes / (1000 / timerDelay);
	int readChunk = bytesLeftToRead / transmittingClients.size();

	do {
	    foreach (PeerWireClient *client, transmittingClients) {
		if (client->state() != QAbstractSocket::ConnectedState) {
		    transmittingClients.removeAll(client);
		    continue;
		}

		int bytesToWrite = qMin(qMin(bytesLeftToWrite, writeChunk), client->bufferedBytesToWrite());
		if (bytesToWrite > 0) {
		    int written = client->acceptBytesToWrite(bytesToWrite);
		    if (written > 0) {
			bytesLeftToWrite -= written;
		    } else {
			transmittingClients.removeAll(client);
			continue;
		    }
		}

		int bytesToRead = qMin<int>(qMin(bytesLeftToRead, readChunk), client->bytesAvailable());
		if (bytesToRead > 0) {
		    bytesLeftToRead -= client->acceptBytesToRead(bytesToRead);
		}

		if (bytesToWrite == 0 && bytesToRead == 0)
		    transmittingClients.removeAll(client);
	    }
	} while (!transmittingClients.isEmpty() && (bytesLeftToWrite > 0 || bytesLeftToRead > 0));
    }
}

