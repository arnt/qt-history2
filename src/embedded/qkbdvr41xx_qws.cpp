/****************************************************************************
** $Id: $
**
** Implementation of Qt/Embedded keyboard drivers
**
** Created : 20020219
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

#include "qkbdvr41xx_qws.h"

#ifdef QT_QWS_CASSIOPEIA

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>

#include <qsocketnotifier.h>

class QWSVr41xxKbPrivate : public QObject
{
    Q_OBJECT
public:
    QWSVr41xxKbPrivate( QWSVr41xxKeyboardHandler *h );
    virtual ~QWSVr41xxKbPrivate();

    bool isOpen() { return buttonFD > 0; }

private slots:
    void readKeyboardData();

private:
    QString terminalName;
    int buttonFD;
    int kbdIdx;
    int kbdBufferLen;
    unsigned char *kbdBuffer;
    QSocketNotifier *notifier;
    QWSVr41xxKeyboardHandler *handler;
};

QWSVr41xxKeyboardHandler::QWSVr41xxKeyboardHandler()
{
    d = new QWSVr41xxKbPrivate( this );
}

QWSVr41xxKeyboardHandler::~QWSVr41xxKeyboardHandler()
{
    delete d;
}

QWSVr41xxKbPrivate::QWSVr41xxKbPrivate( QWSVr41xxKeyboardHandler *h ) : handler(h)
{
    terminalName = "/dev/buttons";
    buttonFD = -1;
    notifier = 0;

    if ((buttonFD = open(terminalName, O_RDWR | O_NDELAY, 0)) < 0)
    {
	qWarning("Cannot open %s\n", terminalName.latin1());
    }

    if ( buttonFD >= 0 ) {
	notifier = new QSocketNotifier( buttonFD, QSocketNotifier::Read, this );
	connect( notifier, SIGNAL(activated(int)),this,
		SLOT(readKeyboardData()) );
    }

    kbdBufferLen = 80;
    kbdBuffer = new unsigned char [kbdBufferLen];
    kbdIdx = 0;
}

QWSVr41xxKbPrivate::~QWSVr41xxKbPrivate()
{
    if ( buttonFD > 0 ) {
	::close( buttonFD );
	buttonFD = -1;
    }
    delete notifier;
    notifier = 0;
    delete [] kbdBuffer;
}

void QWSVr41xxKbPrivate::readKeyboardData()
{
    int n = 0;
    do {
	n  = read(buttonFD, kbdBuffer+kbdIdx, kbdBufferLen - kbdIdx );
	if ( n > 0 )
	    kbdIdx += n;
    } while ( n > 0 );

    int idx = 0;
    while ( kbdIdx - idx >= 2 ) {
	unsigned char *next = kbdBuffer + idx;
	unsigned short *code = (unsigned short *)next;
	int keycode = Qt::Key_unknown;
	switch ( (*code) & 0x0fff ) {
	    case 0x7:
		keycode = Qt::Key_Up;
		break;
	    case 0x9:
		keycode = Qt::Key_Right;
		break;
	    case 0x8:
		keycode = Qt::Key_Down;
		break;
	    case 0xa:
		keycode = Qt::Key_Left;
		break;
	    case 0x3:
		keycode = Qt::Key_Up;
		break;
	    case 0x4:
		keycode = Qt::Key_Down;
		break;
	    case 0x1:
		keycode = Qt::Key_Return;
		break;
	    case 0x2:
		keycode = Qt::Key_F4;
		break;
	    default:
		qDebug("Unrecognised key sequence %d", (int)code );
	}
	if ( (*code) & 0x8000 )
	    handler->processKeyEvent( 0, keycode, 0, FALSE, FALSE );
	else
	    handler->processKeyEvent( 0, keycode, 0, TRUE, FALSE );
	idx += 2;
    }

    int surplus = kbdIdx - idx;
    for ( int i = 0; i < surplus; i++ )
	kbdBuffer[i] = kbdBuffer[idx+i];
    kbdIdx = surplus;
}

#include "qkbdvr41xx_qws.moc"

#endif // QT_QWS_CASSIOPEIA

