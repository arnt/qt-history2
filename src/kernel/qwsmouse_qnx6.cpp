/****************************************************************************
** $Id$
**
** Implementation of Qt/Embedded Qnx mouse drivers
**
** Copyright (C) 1999-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Qt/Embedded may use this file in accordance with the
** Qt Embedded Commercial License Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qwindowsystem_qws.h"
#include "qsocketnotifier.h"
#include "qwsevent_qws.h"
#include "qwscommand_qws.h"
#include "qwsutils_qws.h"
#include "qwsmouse_qws.h"

#include <qapplication.h>
#include <qpointarray.h>

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include <qgfx_qws.h>

#ifdef Q_OS_QNX6

#ifndef QQNXMOUSEHANDLERPRIVATEIMPL
#define QQNXMOUSEHANDLERPRIVATEIMPL
#include <sys/dcmd_input.h>

class QQnxMouseHandlerPrivate : public QWSMouseHandler {
    Q_OBJECT

    public:
	QQnxMouseHandlerPrivate(MouseProtocol &, QString);
	~QQnxMouseHandlerPrivate();

	void clearCalibration();
	void calibrate();
	void getCalibration( QWSPointerCalibrationData * );

    private:
	int mouseFD;
	int index;
	QSocketNotifier *mouseNotifier;
	void *buffer;
	void getData();

	private slots:
	    void readMouseData(int);
};

QQnxMouseHandlerPrivate::QQnxMouseHandlerPrivate(MouseProtocol &protocol,QString dev) : QWSMouseHandler() {
    mouseFD = open(dev.latin1(), O_RDONLY|O_NONBLOCK);

    if (mouseFD == -1) {
	qFatal("Qnx :: Cannot access pointer device\n");
    }

    mouseNotifier = new QSocketNotifier(mouseFD, QSocketNotifier::Read, this );
    connect(mouseNotifier, SIGNAL(activated(int)),this, SLOT(readMouseData(int)));
}

QQnxMouseHandlerPrivate::~QQnxMouseHandlerPrivate(){};
void QQnxMouseHandlerPrivate::clearCalibration(){};
void QQnxMouseHandlerPrivate::calibrate(){};
void QQnxMouseHandlerPrivate::getCalibration(QWSPointerCalibrationData *){};

void QQnxMouseHandlerPrivate::getData() {
    int number_read = read(mouseFD, ((_mouse_packet *)buffer) + index,
	    sizeof(_mouse_packet));
    if ( number_read > 0 && index < 10 ) {
	index++;
	getData();
    }
}

void QQnxMouseHandlerPrivate::readMouseData(int fd) {
    buffer = malloc(sizeof(_mouse_packet) * 11);
    QPoint t ( mousePos.x(), mousePos.y() );
    index = 0;
    bool queuedEvents = FALSE;

    getData();

    for ( int i = 0 ; i < index ; i++ ) {
	//qDebug("%d mouse events read",i);
	_mouse_packet *packet = ((_mouse_packet *)buffer) + i;

	t.setX(t.x() + packet->dx);
	t.setY(t.y() - packet->dy);

	limitToScreen( t );

	if (packet->hdr.buttons & _POINTER_BUTTON_LEFT) {
	    queuedEvents = FALSE;
	    mouseChanged(t, LeftButton);
	} else if (packet->hdr.buttons & _POINTER_BUTTON_RIGHT) {
	    queuedEvents = FALSE;
	    mouseChanged(t, RightButton);
	} else if (packet->hdr.buttons & _POINTER_BUTTON_MIDDLE) {
	    queuedEvents = FALSE;
	    mouseChanged(t, MidButton);
	} else
	    queuedEvents = TRUE;
    }

    if (queuedEvents)
	mouseChanged(t, NoButton);

    free(buffer);
}

#include "qwsmouse_qnx6.moc"
#endif

#endif // Q_OS_QNX6
