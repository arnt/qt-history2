#include "qws.h"
#include "qwsevent.h"
#include "qwscommand.h"
#include "qwsutils.h"

#include <qapplication.h>

#include <stdlib.h>
#include <stdio.h>

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>

enum MouseProtocol { Unknown = -1, MouseMan = 0, IntelliMouse = 1 };

typedef struct {
    const char *name;
    MouseProtocol id;
} MouseConfig;

static const MouseConfig mouseConfig[] = {
    { "MouseMan",	MouseMan },
    { "IntelliMouse",	IntelliMouse },
    { 0,		Unknown }
};

typedef struct {
    int bytesPerPacket;
} MouseData;

static const MouseData mouseData[] = {
    { 3 },  // MouseMan
    { 4 }   // intelliMouse
};

static const char mouseDev[] = "/dev/mouse";
static const int mouseBufSize = 100;

static int mouseFD = -1;
static int mouseIdx;
static uchar *mouseBuf;
static MouseProtocol mouseProtocol = Unknown;
static QSocketNotifier *mouseNotifier = 0;

void QWSServer::openMouse()
{
    if (mouseFD < 0)
	closeMouse();

    if ((mouseFD = open( mouseDev, O_RDWR | O_NDELAY)) < 0) {
	printf( "Cannot open %s (%s)\n", mouseDev, strerror(errno));
	exit(1);
    }

    if (mouseProtocol == Unknown)
    {
	// what mouse is to be used?
	QString proto(getenv("QWS_MOUSE_PROTO"));

	if (proto.isEmpty()) {
	    printf("Mouse protocol not specified - assuming MouseMan\n");
	    proto = "MouseMan";
	}
	else {
	    printf("Request protocol %s\n", proto.ascii());
	}

	int idx = 0;
	while (mouseProtocol == Unknown && mouseConfig[idx].name) {
	    if (proto == QString(mouseConfig[idx].name)) {
		mouseProtocol = mouseConfig[idx].id;
	    }
	    idx++;
	}

	if (mouseProtocol == Unknown) {
	    printf("Unknown mouse protocol: %s\n", proto.ascii());
	    exit(1);
	}
    }

    // Clear pending input
    tcflush(mouseFD,TCIFLUSH);

    bool ps2 = FALSE;

    switch (mouseProtocol) {

	case MouseMan:
	    ps2 = TRUE;
	    write(mouseFD,"",1);
	    usleep(50000);
	    write(mouseFD,"@EeI!",5);
	    break;

	case IntelliMouse: {
		ps2 = TRUE;
		const unsigned char init[] = { 243, 200, 243, 100, 243, 80 };
		write(mouseFD,"",1);
		usleep(50000);
		write(mouseFD,init,6);
	    }
	    break;

	default:
	    printf("Unknown mouse protocol\n");
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
    while (read(mouseFD, buf, 1) > 0);  // eat unwanted replies

    mouseBuf = new uchar [mouseBufSize];
    mouseIdx = 0;

    mouseNotifier = new QSocketNotifier( mouseFD, QSocketNotifier::Read, this );
    connect(mouseNotifier, SIGNAL(activated(int)),this, SLOT(readMouseData()));
}

void QWSServer::closeMouse()
{
    if (mouseFD >= 0)
    {
	delete mouseNotifier;
	delete [] mouseBuf;
	close(mouseFD);
	mouseFD = -1;
    }
}

void QWSServer::readMouseData()
{
    int n;
    do {
	n = read(mouseFD, mouseBuf+mouseIdx, mouseBufSize-mouseIdx );
	if ( n > 0 ) {
	    mouseIdx += n;
	    handleMouseData();
	}
    } while ( n > 0 );
}

/*
*/

void QWSServer::handleMouseData()
{
    static const int accel_limit = 5;
    static const int accel = 2;

    //    printf( "handleMouseData mouseIdx=%d\n", mouseIdx );

    int idx = 0;
    int bstate = 0;
    int dx = 0, dy = 0;
    bool sendEvent = FALSE;

    while ( mouseIdx-idx >= mouseData[mouseProtocol].bytesPerPacket ) {
	uchar *mb = mouseBuf+idx;
	bstate = 0;
	dx = 0;
	dy = 0;
	sendEvent = FALSE;
	switch (mouseProtocol) {
	    case MouseMan:
	    case IntelliMouse:

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

		    sendEvent = TRUE;
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
	if (sendEvent) {
	    if ( QABS(dx) > accel_limit || QABS(dy) > accel_limit ) {
		dx *= accel;
		dy *= accel;
	    }
	    QPoint m = mousePos + QPoint(dx,-dy);
	
	    setMouse(m,bstate);
	}
	idx += mouseData[mouseProtocol].bytesPerPacket;
    }

    int surplus = mouseIdx - idx;
    for ( int i = 0; i < surplus; i++ )
	mouseBuf[i] = mouseBuf[idx+i];
    mouseIdx = surplus;


    //printf( "exit handleMouseData mouseIdx=%d\n", mouseIdx );

// ###### What was this doing here?
//  if(getenv("QWS_NOACCEL")==0 && probed_card==0) {
//      probe_bus(fb_vinfo.xres,fb_vinfo.yres,fb_vinfo.bits_per_pixel);
//  }
}

