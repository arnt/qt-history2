/****************************************************************************
** $Id: qt/src/kernel/qqnxmouse_qws.cpp   2.3.1emb   edited 2001-09-24 $
**
** Implementation of Qt/Embedded Qnx mouse drivers
**
** Created : 991025
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
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

#if defined(_OS_QNX6_)

#include <sys/dcmd_input.h>

static QPoint &mousePos = QWSServer::mousePosition;

static void limitToScreen( QPoint &pt )
{
    static int w = -1;
    static int h;
    if ( w < 0 ) {
    w = qt_screen->deviceWidth();
    h = qt_screen->deviceHeight();
    }
 
    pt.setX( QMIN( w-1, QMAX( 0, pt.x() )));
    pt.setY( QMIN( h-1, QMAX( 0, pt.y() )));
}
 
QWSMouseHandler::QWSMouseHandler()
{
    QWSServer::setMouseHandler(this);
}
 
QWSMouseHandler::~QWSMouseHandler()
{
}

class QQnxMouseHandler : public QWSMouseHandler {
	Q_OBJECT

public:
	QQnxMouseHandler(QString dev);
	~QQnxMouseHandler(){};

	void clearCalibration(){};
	void calibrate(){};
	void getCalibration( QWSPointerCalibrationData * ) {}

private:
	int mouseFD;
	int index;
    QSocketNotifier *mouseNotifier;
	void *buffer;
	void getData();

private slots:
	void readMouseData(int);
};

QQnxMouseHandler::QQnxMouseHandler(QString dev) : QWSMouseHandler() {
	mouseFD = open(dev.latin1(), O_RDONLY|O_NONBLOCK);

	if (mouseFD == -1) {
		qFatal("Qnx :: Cannot access pointer device\n");
	}

	mouseNotifier = new QSocketNotifier(mouseFD, QSocketNotifier::Read, this );
    connect(mouseNotifier, SIGNAL(activated(int)),this, SLOT(readMouseData(int)));
}

void QQnxMouseHandler::getData() {
	int number_read = read(mouseFD, ((_mouse_packet *)buffer) + index,
		sizeof(_mouse_packet));
	if ( number_read > 0 && index < 10 ) {
		index++;
		getData();
	}
}

void QQnxMouseHandler::readMouseData(int fd) {
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
			emit(mouseChanged(t, LeftButton));
		} else if (packet->hdr.buttons & _POINTER_BUTTON_RIGHT) {
			queuedEvents = FALSE;
			emit(mouseChanged(t, RightButton));
		} else if (packet->hdr.buttons & _POINTER_BUTTON_MIDDLE) {
			queuedEvents = FALSE;
			emit(mouseChanged(t, MidButton));
		} else
			queuedEvents = TRUE;
	}

	if (queuedEvents)
		emit(mouseChanged(t, NoButton));

	free(buffer);
}

// we are only accepting a device for the spec at the moment
QWSMouseHandler* QWSServer::newMouseHandler(const QString& spec)
{
    static int init=0;
    if ( !init && qt_screen ) {
	    init = 1;
    }
	return new QQnxMouseHandler(spec);
}

void QCalibratedMouseHandler::clearCalibration() {}
void QCalibratedMouseHandler::calibrate( QWSPointerCalibrationData *){}
void QCalibratedMouseHandler::getCalibration( QWSPointerCalibrationData * ) {}

#include "qwsmouse_qnx.moc"

#endif // _OS_QNX_


