/****************************************************************************
** $Id: $
**
** Implementation of Qt/Embedded USB keyboard drivers
**
** Created : 20020218
**
** Copyright (C) 1992-2002 Trolltech AS.  All rights reserved.
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

#include "qkbdusb_qws.h"

#ifndef QT_NO_QWS_KEYBOARD

#include "qgfx_qws.h"

#include "qwindowsystem_qws.h"
#include "qapplication.h"
#include "qsocketnotifier.h"
#include "qnamespace.h"
#include "qtimer.h"

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>


#ifdef Q_OS_LINUX
#include <sys/kd.h>
#include <sys/vt.h>
#endif


/* USB driver */


class QWSUsbKbPrivate : public QObject
{
    Q_OBJECT
public:
    QWSUsbKbPrivate( QWSPC101KeyboardHandler * );
    ~QWSUsbKbPrivate();

private slots:
    void readKeyboardData();

private:
    QWSPC101KeyboardHandler *handler;
    int fd;
};

QWSUsbKeyboardHandler::QWSUsbKeyboardHandler()
{
    d = new QWSUsbKbPrivate( this );
}

QWSUsbKeyboardHandler::~QWSUsbKeyboardHandler()
{
    delete d;
}

QWSUsbKbPrivate::QWSUsbKbPrivate( QWSPC101KeyboardHandler *h ) : handler(h)
{
    fd = ::open(getenv("QWS_USB_KEYBOARD"),O_RDONLY, 0);
    if ( fd >= 0 ) {
	QSocketNotifier *notifier;
	notifier = new QSocketNotifier( fd, QSocketNotifier::Read, this );
	connect( notifier, SIGNAL(activated(int)),this,
		 SLOT(readKeyboardData()) );
    }
}

QWSUsbKbPrivate::~QWSUsbKbPrivate()
{
    ::close(fd);
}

struct Myinputevent {

    unsigned int dummy1;
    unsigned int dummy2;
    unsigned short type;
    unsigned short code;
    unsigned int value;

};

void QWSUsbKbPrivate::readKeyboardData()
{
    Myinputevent event;
    int n = read(fd, &event, sizeof(Myinputevent) );
    if ( n != 16 )
	return;
    int key=event.code;
    if(key==103) {
	handler->processKeyEvent( 0, Qt::Key_Up, 0, event.value!=0, false );
    } else if(key==106) {
	handler->processKeyEvent( 0, Qt::Key_Right, 0, event.value!=0, false  );
    } else if(key==108) {
	handler->processKeyEvent( 0, Qt::Key_Down, 0, event.value!=0, false );
    } else if(key==105) {
	handler->processKeyEvent( 0, Qt::Key_Left, 0, event.value!=0, false );
    } else {
	if( event.value == 0 ) {
	    key=key | 0x80;
	}
	handler->doKey(key);
    }
}

#include "qkbdusb_qws.moc"

#endif // QT_NO_QWS_KEYBOARD

