/****************************************************************************
** $Id: $
**
** Implementation of Qt/Embedded keyboard drivers
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

#include "qkbdtty_qws.h"

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
#include <termios.h>


#ifdef Q_OS_QNX6
#include "qwskeyboard_qnx.cpp"
#endif

#ifdef Q_OS_LINUX
#include <sys/kd.h>
#include <sys/vt.h>
#endif


#define VTSWITCHSIG SIGUSR2

static bool vtActive = true;
static int  vtQws = 0;
static int  kbdFD = -1;

#if defined(Q_OS_LINUX)
static void vtSwitchHandler(int /*sig*/)
{
    if (vtActive) {
	qwsServer->enablePainting(false);
	qt_screen->save();
	if (ioctl(kbdFD, VT_RELDISP, 1) == 0) {
	    vtActive = false;
	    qwsServer->closeMouse();
	}
	else {
	    qwsServer->enablePainting(true);
	}
	usleep(200000);
    }
    else {
	if (ioctl(kbdFD, VT_RELDISP, VT_ACKACQ) == 0) {
	    qwsServer->enablePainting(true);
	    vtActive = true;
	    qt_screen->restore();
	    qwsServer->openMouse();
	    qwsServer->refresh();
	}
    }
    signal(VTSWITCHSIG, vtSwitchHandler);
}
#endif


//===========================================================================

//
// Tty keyboard
//

#ifndef QT_NO_QWS_KBDTTY

class QWSTtyKbPrivate : public QObject
{
    Q_OBJECT
public:
    QWSTtyKbPrivate( QWSPC101KeyboardHandler * );
    ~QWSTtyKbPrivate();

private slots:
    void readKeyboardData();

private:
    QWSPC101KeyboardHandler *handler;
    struct termios origTermData;
};

QWSTtyKeyboardHandler::QWSTtyKeyboardHandler()
{
    d = new QWSTtyKbPrivate( this );
}

QWSTtyKeyboardHandler::~QWSTtyKeyboardHandler()
{
    delete d;
}

void QWSTtyKeyboardHandler::processKeyEvent(int unicode, int keycode,
		    int modifiers, bool isPress, bool autoRepeat)
{
#if defined(Q_OS_LINUX)
    // Virtual console switching
    int term = 0;
    bool ctrl = modifiers & Qt::ControlButton;
    bool alt = modifiers & Qt::AltButton;
    if (ctrl && alt && keycode >= Qt::Key_F1 && keycode <= Qt::Key_F10)
	term = keycode - Qt::Key_F1 + 1;
    else if (ctrl && alt && keycode == Qt::Key_Left)
	term = QMAX(vtQws - 1, 1);
    else if (ctrl && alt && keycode == Qt::Key_Right)
	term = QMIN(vtQws + 1, 10);
    if (term && !isPress) {
	ioctl(kbdFD, VT_ACTIVATE, term);
	return;
    }
#endif

    QWSPC101KeyboardHandler::processKeyEvent( unicode, keycode, modifiers,
	isPress, autoRepeat );
}


QWSTtyKbPrivate::QWSTtyKbPrivate( QWSPC101KeyboardHandler *h ) : handler(h)
{
    kbdFD = ::open("/dev/tty0", O_RDWR | O_NDELAY, 0);

    if ( kbdFD >= 0 ) {
	QSocketNotifier *notifier;
	notifier = new QSocketNotifier( kbdFD, QSocketNotifier::Read, this );
	connect( notifier, SIGNAL(activated(int)),this,
		 SLOT(readKeyboardData()) );
    } else {
	qDebug( "Cannot open keyboard" );
    }

    // save for restore.
    tcgetattr( kbdFD, &origTermData );

    struct termios termdata;
    tcgetattr( kbdFD, &termdata );

#if defined(Q_OS_LINUX)
    ioctl(kbdFD, KDSKBMODE, K_RAW);
#endif

    termdata.c_iflag = (IGNPAR | IGNBRK) & (~PARMRK) & (~ISTRIP);
    termdata.c_oflag = 0;
    termdata.c_cflag = CREAD | CS8;
    termdata.c_lflag = 0;
    termdata.c_cc[VTIME]=0;
    termdata.c_cc[VMIN]=1;
    cfsetispeed(&termdata, 9600);
    cfsetospeed(&termdata, 9600);
    tcsetattr(kbdFD, TCSANOW, &termdata);

#if defined(Q_OS_LINUX)
    signal(VTSWITCHSIG, vtSwitchHandler);

    struct vt_mode vtMode;
    ioctl(kbdFD, VT_GETMODE, &vtMode);

    // let us control VT switching
    vtMode.mode = VT_PROCESS;
    vtMode.relsig = VTSWITCHSIG;
    vtMode.acqsig = VTSWITCHSIG;
    ioctl(kbdFD, VT_SETMODE, &vtMode);

    struct vt_stat vtStat;
    ioctl(kbdFD, VT_GETSTATE, &vtStat);
    vtQws = vtStat.v_active;
#endif
}

QWSTtyKbPrivate::~QWSTtyKbPrivate()
{
    if (kbdFD >= 0) {
#if defined(Q_OS_LINUX)
	ioctl(kbdFD, KDSKBMODE, K_XLATE);
#endif
	tcsetattr(kbdFD, TCSANOW, &origTermData);
	::close(kbdFD);
	kbdFD = -1;
    }
}

void QWSTtyKbPrivate::readKeyboardData()
{
    unsigned char buf[81];
    int n = read(kbdFD, buf, 80 );
    for ( int loop = 0; loop < n; loop++ )
	handler->doKey(buf[loop]);
}

#endif // QT_NO_QWS_KBDTTY

#include "qkbdtty_qws.moc"

#endif // QT_NO_QWS_KEYBOARD

