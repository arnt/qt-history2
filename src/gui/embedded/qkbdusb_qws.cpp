/****************************************************************************
**
** Implementation of Qt/Embedded USB keyboard drivers.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qkbdusb_qws.h"

#ifndef QT_NO_QWS_KEYBOARD

#include "qgfx_qws.h"

#include "qwindowsystem_qws.h"
#include "qapplication.h"
#include "qsocketnotifier.h"
#include "qnamespace.h"
#include "qtimer.h"

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
    QWSUsbKbPrivate( QWSPC101KeyboardHandler *, const QString & );
    ~QWSUsbKbPrivate();

private slots:
    void readKeyboardData();

private:
    QWSPC101KeyboardHandler *handler;
    int fd;
};

QWSUsbKeyboardHandler::QWSUsbKeyboardHandler(const QString &device)
    : QWSPC101KeyboardHandler( device )
{
    d = new QWSUsbKbPrivate( this, device );
}

QWSUsbKeyboardHandler::~QWSUsbKeyboardHandler()
{
    delete d;
}

QWSUsbKbPrivate::QWSUsbKbPrivate( QWSPC101KeyboardHandler *h, const QString &device ) : handler(h)
{
    fd = ::open(device.isEmpty()?"/dev/input/event0":device.latin1(),O_RDONLY, 0);
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
#ifndef QT_QWS_USE_KEYCODES
    // Handle SOME keys, otherwise it's useless.
    if(key==103) {
	handler->processKeyEvent( 0, Qt::Key_Up, 0, event.value!=0, false );
    } else if(key==106) {
	handler->processKeyEvent( 0, Qt::Key_Right, 0, event.value!=0, false  );
    } else if(key==108) {
	handler->processKeyEvent( 0, Qt::Key_Down, 0, event.value!=0, false );
    } else if(key==105) {
	handler->processKeyEvent( 0, Qt::Key_Left, 0, event.value!=0, false );
    } else
#endif
    {
	if( event.value == 0 ) {
	    key=key | 0x80;
	}
	handler->doKey(key);
    }
}

#include "qkbdusb_qws.moc"

#endif // QT_NO_QWS_KEYBOARD

