/****************************************************************************
** $Id: //depot/qt/qws/util/qws/qwsmouse_qws.cpp#1 $
**
** Implementation of Qt/Embedded mouse drivers
**
** Created : 991025
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Qt/Embedded may use this file in accordance with the
** Qt Embedded Commercial License Agreement provided with the Software.
**
** This file is not available for use under any other license without
** express written permission from the copyright holder.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qwindowsystem_qws.h"
#include "qwsevent_qws.h"
#include "qwscommand_qws.h"
#include "qwsutils_qws.h"
#include "qwsmouse_qws.h"

#include <qapplication.h>
#include <qtimer.h>

#include <stdlib.h>
#include <stdio.h>

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>

#include <qgfx_qws.h>

#ifdef __MIPSEL__
#include <linux/tpanel.h>
#define QWS_TOUCHPANEL
#endif

//#define QWS_CUSTOMTOUCHPANEL

enum MouseProtocol { Unknown = -1, MouseMan = 0, IntelliMouse = 1,
                     Microsoft = 2, QVFBMouse = 3, TPanel = 4 };

typedef struct {
    char *name;
    MouseProtocol id;
} MouseConfig;

static const MouseConfig mouseConfig[] = {
    { "MouseMan",	MouseMan },
    { "IntelliMouse",	IntelliMouse },
    { "Microsoft",      Microsoft },
    { "QVFbMouse",      QVFBMouse },
    { "TPanel",         TPanel },
    { 0,		Unknown }
};


static const int mouseBufSize = 100;
static QPoint mousePos;

/*
 * Standard mouse driver
 */

typedef struct {
    int bytesPerPacket;
} MouseData;

static const MouseData mouseData[] = {
    { 3 },  // MouseMan
    { 4 },  // intelliMouse
    { 3 }   // Microsoft
};


class QMouseHandlerPrivate : public QMouseHandler {
    Q_OBJECT
public:
    QMouseHandlerPrivate( MouseProtocol protocol, QString mouseDev );
    ~QMouseHandlerPrivate();

private:
    int mouseFD;
    int mouseIdx;
    uchar mouseBuf[mouseBufSize];
    MouseProtocol mouseProtocol;
    void handleMouseData();

private slots:
    void readMouseData();

private:
    int obstate;
};


static void limitToScreen( QPoint &pt )
{
    static int w = -1;
    static int h;
    if ( w < 0 ) {
	w = qt_screen->width();
	h = qt_screen->height();
    }

    pt.setX( QMIN( w-1, QMAX( 0, pt.x() )));
    pt.setY( QMIN( h-1, QMAX( 0, pt.y() )));
}

void QMouseHandlerPrivate::readMouseData()
{
    int n;
    do {
	n = read(mouseFD, mouseBuf+mouseIdx, mouseBufSize-mouseIdx );
	if ( n > 0 )
	    mouseIdx += n;
    } while ( n > 0 );
    handleMouseData();
}


/*
*/

void QMouseHandlerPrivate::handleMouseData()
{
    static const int accel_limit = 5;
    static const int accel = 2;

    //    printf( "handleMouseData mouseIdx=%d\n", mouseIdx );

    int idx = 0;
    int bstate = 0;
    int dx = 0, dy = 0;
    bool sendEvent = false;
    int tdx = 0, tdy = 0;

    while ( mouseIdx-idx >= mouseData[mouseProtocol].bytesPerPacket ) {
	uchar *mb = mouseBuf+idx;
	bstate = 0;
	dx = 0;
	dy = 0;
	sendEvent = false;
	switch (mouseProtocol) {
	    case MouseMan:
	    case IntelliMouse:
	    {
		if (mb[0] & 0x01)
		    bstate |= Qt::LeftButton;
		if (mb[0] & 0x02)
		    bstate |= Qt::RightButton;
		if (mb[0] & 0x04)
		    bstate |= Qt::MidButton;

		int overflow = (mb[0]>>6 )& 0x03;
		if (mouseProtocol == MouseMan && overflow) {
		    //### wheel events signalled with overflow bit, ignore for now
		}
		else {
		    bool xs = mb[0] & 0x10;
		    bool ys = mb[0] & 0x20;
		    dx = xs ? mb[1]-256 : mb[1];
		    dy = ys ? mb[2]-256 : mb[2];

		    sendEvent = true;
		}
#if 0 //debug
		if (mouseProtocol == MouseMan)
		    printf("(%2d) %02x %02x %02x ", idx, mb[0], mb[1], mb[2]);
		else
		    printf("(%2d) %02x %02x %02x %02x ",idx,mb[0],mb[1],mb[2],mb[3]);
		const char *b1 = (mb[0] & 0x01) ? "b1":"  ";//left
		const char *b2 = (mb[0] & 0x02) ? "b2":"  ";//right
		const char *b3 = (mb[0] & 0x04) ? "b3":"  ";//mid

		if ( overflow )
		    printf( "Overflow%d %s %s %s  (%4d,%4d)\n", overflow,
			    b1, b2, b3, mousePos.x(), mousePos.y() );
		else
		    printf( "%s %s %s (%+3d,%+3d)  (%4d,%4d)\n",
			    b1, b2, b3, dx, dy, mousePos.x(), mousePos.y() );
#endif
		break;
	    }
	    case Microsoft:
	        if ( ((mb[0] & 0x20) >> 3) ) {
		    bstate |= Qt::LeftButton;
		}
		if ( ((mb[0] & 0x10) >> 4) ) {
		    bstate |= Qt::RightButton;
		}

		dx=(char)(((mb[0] & 0x03) << 6) | (mb[1] & 0x3f));
		dy=-(char)(((mb[0] & 0x0c) << 4) | (mb[2] & 0x3f));
		sendEvent=true;

		break;
	    default:
		qWarning( "Unknown mouse protocol in QMouseHandlerPrivate" );
		break;
	}
	if (sendEvent) {
	    if ( QABS(dx) > accel_limit || QABS(dy) > accel_limit ) {
		dx *= accel;
		dy *= accel;
	    }
	    tdx += dx;
	    tdy += dy;
	    if ( bstate != obstate ) {
		mousePos += QPoint(tdx,-tdy);
		limitToScreen( mousePos );
		emit mouseChanged(mousePos,bstate);
		sendEvent = FALSE;
		tdx = 0;
		tdy = 0;
		obstate = bstate;
	    }
	}
	idx += mouseData[mouseProtocol].bytesPerPacket;
    }
    if ( sendEvent ) {
	mousePos += QPoint(tdx,-tdy);
	limitToScreen( mousePos );
	emit mouseChanged(mousePos,bstate);
    }

    int surplus = mouseIdx - idx;
    for ( int i = 0; i < surplus; i++ )
	mouseBuf[i] = mouseBuf[idx+i];
    mouseIdx = surplus;
}


QMouseHandlerPrivate::QMouseHandlerPrivate( MouseProtocol protocol,
					  QString mouseDev )
{
    mouseProtocol = protocol;

    if ( mouseDev.isEmpty() )
	mouseDev = "/dev/mouse";

    static int init=0;
    if ( !init && qt_screen ) {
	init = 1;
	mousePos = QPoint(qt_screen->width()/2,
			  qt_screen->height()/2);
    }

    obstate = -1;
    if ((mouseFD = open( mouseDev.local8Bit(), O_RDWR | O_NDELAY)) < 0) {
	qDebug( "Cannot open %s (%s)", mouseDev.ascii(),
		strerror(errno));
    } else {
	// Clear pending input

	tcflush(mouseFD,TCIFLUSH);

	bool ps2 = false;

	switch (mouseProtocol) {

	    case MouseMan:
		ps2 = true;
		write(mouseFD,"",1);
		usleep(50000);
		write(mouseFD,"@EeI!",5);
		break;

	    case IntelliMouse: {
		    ps2 = true;
		    const unsigned char init[] = { 243, 200, 243, 100, 243, 80 };
		    write(mouseFD,"",1);
		    usleep(50000);
		    write(mouseFD,init,6);
		}
		break;

	    case Microsoft:
		struct termios tty;

		tcgetattr(mouseFD, &tty);

		tty.c_iflag = IGNBRK | IGNPAR;
		tty.c_oflag = 0;
		tty.c_lflag = 0;
		tty.c_line = 0;
		tty.c_cc[VTIME] = 0;
		tty.c_cc[VMIN] = 1;
		tty.c_cflag = B1200 | CS7 | CREAD | CLOCAL | HUPCL;
		tcsetattr(mouseFD, TCSAFLUSH, &tty); /* set parameters */
		break;

	    default:
		qDebug("Unknown mouse protocol");
		exit(1);
	}

	if (ps2) {
	    char buf[] = { 246, 244 };
	    write(mouseFD,buf,1);
	    write(mouseFD,buf+1,1);
	}

	usleep(50000);
	tcflush(mouseFD,TCIFLUSH);	    // ### doesn't seem to work.

	char buf[2];
	while (read(mouseFD, buf, 1) > 0) { }  // eat unwanted replies

	mouseIdx = 0;

	QSocketNotifier *mouseNotifier;
	mouseNotifier = new QSocketNotifier( mouseFD, QSocketNotifier::Read, this );
	connect(mouseNotifier, SIGNAL(activated(int)),this, SLOT(readMouseData()));
    }
}

QMouseHandlerPrivate::~QMouseHandlerPrivate()
{
    if (mouseFD >= 0)
	close(mouseFD);
}

/*
 *
 */

QCalibratedMouseHandler::QCalibratedMouseHandler()
{
    clearCalibration();
}

void QCalibratedMouseHandler::clearCalibration()
{
    a = 1;
    b = 0;
    c = 0;
    d = 0;
    e = 1;
    f = 0;
    s = 1;
}

void QCalibratedMouseHandler::calibrate( QWSPointerCalibrationData *cd )
{
    QPoint dev_tl = cd->devPoints[ QWSPointerCalibrationData::TopLeft ];
    QPoint dev_br = cd->devPoints[ QWSPointerCalibrationData::BottomRight ];
    QPoint screen_tl = cd->screenPoints[ QWSPointerCalibrationData::TopLeft ];
    QPoint screen_br = cd->screenPoints[ QWSPointerCalibrationData::BottomRight ];

    s = 1 << 16;

    a = s * (screen_tl.x() - screen_br.x() ) / (dev_tl.x() - dev_br.x());
    b = 0;
    c = s * screen_tl.x() - a * dev_tl.x();

    d = 0;
    e = s * (screen_tl.y() - screen_br.y() ) / (dev_tl.y() - dev_br.y());
    f = s * screen_tl.y() - e * dev_tl.y();
}

QPoint QCalibratedMouseHandler::transform( const QPoint &p )
{
    QPoint tp;

    tp.setX( (a * p.x() + b * p.y() + c) / s );
    tp.setY( (d * p.x() + e * p.y() + f) / s );

    return tp;
}


/*
 * Handler for /dev/tpanel Linux kernel driver
 */

class QVrTPanelHandlerPrivate : public QCalibratedMouseHandler {
    Q_OBJECT
public:
    QVrTPanelHandlerPrivate(MouseProtocol, QString dev);
    ~QVrTPanelHandlerPrivate();

private:
    int mouseFD;
    MouseProtocol mouseProtocol;
private slots:
    void sendRelease();
    void readMouseData();
private:
    QTimer *rtimer;
};

QVrTPanelHandlerPrivate::QVrTPanelHandlerPrivate( MouseProtocol, QString dev ) :
    QCalibratedMouseHandler()
{
#ifdef QWS_TOUCHPANEL
    if ( dev.isEmpty() )
	dev = "/dev/tpanel";

    if ((mouseFD = open( dev, O_RDONLY)) < 0) {
        qFatal( "Cannot open %s (%s)", dev.latin1(), strerror(errno));
    } else {
        sleep(1);
    }

    struct scanparam s;
    s.interval = 20000;
    s.settletime = 480;
    if ( ioctl(mouseFD, TPSETSCANPARM, &s) < 0
      || fcntl(mouseFD, F_SETFL, O_NONBLOCK) < 0 )
	qWarning("Error initializing touch panel.");

    QSocketNotifier *mouseNotifier;
    mouseNotifier = new QSocketNotifier( mouseFD, QSocketNotifier::Read,
					 this );
    connect(mouseNotifier, SIGNAL(activated(int)),this, SLOT(readMouseData()));

    rtimer = new QTimer( this );
    connect( rtimer, SIGNAL(timeout()), this, SLOT(sendRelease()));

    printf("\033[?25l"); fflush(stdout); // VT100 cursor off
#endif
}

QVrTPanelHandlerPrivate::~QVrTPanelHandlerPrivate()
{
    if (mouseFD >= 0)
	close(mouseFD);
}

void QVrTPanelHandlerPrivate::sendRelease()
{
    emit mouseChanged(mousePos, 0);
}

void QVrTPanelHandlerPrivate::readMouseData()
{
#ifdef QWS_TOUCHPANEL
    if(!qt_screen)
	return;
    short data[6];
    int ret;
    static int prev_valid=0;
    static QPoint prev;
    static int prev_pressure = 0;
    static bool pressed = FALSE;
    static bool reverse = FALSE;  // = TRUE; Osprey axis reversed
//    static bool reverse = FALSE;  // = TRUE; Osprey axis reversed

#define EMIT_MOUSE \
	QPoint q = transform( prev ); \
	if ( reverse ) { \
	    q.setX( qt_screen->width()-q.x() ); \
	    q.setY( qt_screen->height()-q.y() ); \
	} \
	if ( q != mousePos ) { \
	    mousePos = q; \
	    emit mouseChanged(mousePos, Qt::LeftButton); \
	} \
	pressed = TRUE; \
	rtimer->stop();

    do {
	ret=read(mouseFD,data,sizeof(data));

	if(ret==sizeof(data)) {
	    // "auto calibrate" for now.
	    if ( data[0] & 0x8000 ) {
		if ( data[5] > 800 ) {
		    if ( prev_pressure - data[5] < 40 ) {
			QPoint t(data[3]-data[4],data[2]-data[1]);
			if ( prev_valid ) {
			    QPoint d = t-prev;
			    if ( d.manhattanLength() > 450 ) // scan error
				return;
			    if ( QABS(d.x()) < 3 && QABS(d.y()) < 3 )   // insignificant change
				return;
			    prev = (t+prev)/2;
			} else {
			    prev = t;
			}
			prev_valid++;
		    }
		    prev_pressure = data[5];
		}
	    } else {
		if ( prev_valid ) {
		    prev_valid = 0;
		    EMIT_MOUSE
		}
		if ( pressed ) {
		    rtimer->start( 40, TRUE );
		    pressed = FALSE;
		}
	    }
	}
    } while ( ret > 0 );

    if ( prev_valid > 1 ) {
	prev_valid = 0;
	EMIT_MOUSE
    }
#endif
}

class QCustomTPanelHandlerPrivate : public QMouseHandler {
    Q_OBJECT
public:
    QCustomTPanelHandlerPrivate(MouseProtocol, QString dev);
    ~QCustomTPanelHandlerPrivate();

private:
    int mouseFD;
private slots:
    void readMouseData();

};

QCustomTPanelHandlerPrivate::QCustomTPanelHandlerPrivate( MouseProtocol, QString )
{
#ifdef QWS_CUSTOMTOUCHPANEL
    if ((mouseFD = open( "/dev/ts", O_RDONLY)) < 0) {
        qWarning( "Cannot open /dev/ts (%s)", strerror(errno));
	return;
    } else {
        sleep(1);
    }

    QSocketNotifier *mouseNotifier;
    mouseNotifier = new QSocketNotifier( mouseFD, QSocketNotifier::Read,
					 this );
    connect(mouseNotifier, SIGNAL(activated(int)),this, SLOT(readMouseData()));
#endif
}

QCustomTPanelHandlerPrivate::~QCustomTPanelHandlerPrivate()
{
    if (mouseFD >= 0)
	close(mouseFD);
}

struct CustomTPdata {

  unsigned char status;
  unsigned short xpos;
  unsigned short ypos;

};

void QCustomTPanelHandlerPrivate::readMouseData()
{
#ifdef QWS_CUSTOMTOUCHPANEL
    if(!qt_screen)
	return;
    CustomTPdata data;

    unsigned char data2[5];

    int ret;

    ret=read(mouseFD,data2,5);

    if(ret==5) {
	data.status=data2[0];
	data.xpos=(data2[1] << 8) | data2[2];
	data.ypos=(data2[3] << 8) | data2[4];
	QPoint q;
	q.setX(data.xpos);
	q.setY(data.ypos);
	mousePos=q;
	if(data.status & 0x40) {
          emit mouseChanged(mousePos,Qt::LeftButton);
	} else {
	  emit mouseChanged(mousePos,0);
	}
    }
    if(ret<0) { 
	qDebug("Error %s",strerror(errno));
    }
#endif
}

/*
 * Virtual framebuffer mouse driver
 */

#ifndef QT_NO_QWS_VFB
#include "qvfbhdr_qws.h"
extern int qws_display_id;
#endif

class QVFbMouseHandlerPrivate : public QMouseHandler {
    Q_OBJECT
public:
    QVFbMouseHandlerPrivate(MouseProtocol, QString dev);
    ~QVFbMouseHandlerPrivate();

    bool isOpen() const { return mouseFD > 0; }

private:
    int mouseFD;
    int mouseIdx;
    uchar mouseBuf[mouseBufSize];
private slots:
    void readMouseData();
};

QVFbMouseHandlerPrivate::QVFbMouseHandlerPrivate( MouseProtocol, QString mouseDev )
{
    mouseFD = -1;
#ifndef QT_NO_QWS_VFB
    if ( mouseDev.isEmpty() )
	mouseDev = QString(QT_VFB_MOUSE_PIPE).arg(qws_display_id);

    static int init=0;
    if ( !init && qt_screen ) {
	init = 1;
	mousePos = QPoint(qt_screen->width()/2,
			  qt_screen->height()/2);
    }

    if ((mouseFD = open( mouseDev.local8Bit(), O_RDWR | O_NDELAY)) < 0) {
	qDebug( "Cannot open %s (%s)", mouseDev.ascii(),
		strerror(errno));
    } else {
	// Clear pending input
	char buf[2];
	while (read(mouseFD, buf, 1) > 0) { }

	mouseIdx = 0;

	QSocketNotifier *mouseNotifier;
	mouseNotifier = new QSocketNotifier( mouseFD, QSocketNotifier::Read, this );
	connect(mouseNotifier, SIGNAL(activated(int)),this, SLOT(readMouseData()));
    }
#endif
}

QVFbMouseHandlerPrivate::~QVFbMouseHandlerPrivate()
{
#ifndef QT_NO_QWS_VFB
    if (mouseFD >= 0)
	close(mouseFD);
#endif
}

void QVFbMouseHandlerPrivate::readMouseData()
{
#ifndef QT_NO_QWS_VFB
    int n;
    do {
	n = read(mouseFD, mouseBuf+mouseIdx, mouseBufSize-mouseIdx );
	if ( n > 0 )
	    mouseIdx += n;
    } while ( n > 0 );

    int idx = 0;
    while ( mouseIdx-idx >= int(sizeof( QPoint ) + sizeof( int )) ) {
	uchar *mb = mouseBuf+idx;
	QPoint *p = (QPoint *) mb;
	mb += sizeof( QPoint );
	int *bstate = (int *)mb;
	mousePos = *p;
	limitToScreen( mousePos );
	emit mouseChanged(mousePos, *bstate);
	idx += sizeof( QPoint ) + sizeof( int );
    }

    int surplus = mouseIdx - idx;
    for ( int i = 0; i < surplus; i++ )
	mouseBuf[i] = mouseBuf[idx+i];
    mouseIdx = surplus;
#endif
}

/*
 * return a QMouseHandler that supports /a spec.
 */

QMouseHandler* QWSServer::newMouseHandler(const QString& spec)
{
    int c = spec.find(':');
    QString mouseProto;
    QString mouseDev;
    if ( c >= 0 ) {
	mouseProto = spec.left(c);
	mouseDev = spec.mid(c+1);
    } else {
	mouseProto = spec;
    }

    MouseProtocol mouseProtocol = Unknown;

    int idx = 0;
    while (mouseProtocol == Unknown && mouseConfig[idx].name) {
	if (mouseProto == QString(mouseConfig[idx].name)) {
	    mouseProtocol = mouseConfig[idx].id;
	}
	idx++;
    }

    QMouseHandler *handler = 0;

#ifdef QWS_CUSTOMTOUCHPANEL
    handler=new QCustomTPanelHandlerPrivate(mouseProtocol,mouseDev);
#endif
    
#ifndef QWS_CUSTOMTOUCHPANEL
    switch ( mouseProtocol ) {
	case MouseMan:
	case IntelliMouse:
	case Microsoft:
	    handler = new QMouseHandlerPrivate( mouseProtocol, mouseDev );
	    break;

	case QVFBMouse:
	    handler = new QVFbMouseHandlerPrivate( mouseProtocol, mouseDev );
	    break;

	case TPanel:
	    handler = new QVrTPanelHandlerPrivate( mouseProtocol, mouseDev );
	    break;

	default:
	    qDebug( "Mouse type %s unsupported", spec.latin1() );
    }
#endif

    return handler;
}

#include "qwsmouse_qws.moc"





