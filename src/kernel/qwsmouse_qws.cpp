#include "qwindowsystem_qws.h"
#include "qwsevent.h"
#include "qwscommand.h"
#include "qwsutils.h"

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

#include <qgfx.h>

#ifdef __MIPSEL__
#include <linux/tpanel.h>
#define QWS_TOUCHPANEL
#endif

enum MouseProtocol { Unknown = -1, MouseMan = 0, IntelliMouse = 1,
                     Microsoft = 2 };
static const int mouseBufSize = 100;
static QPoint mousePos;

class MouseHandlerPrivate : public MouseHandler {
    Q_OBJECT
public:
    MouseHandlerPrivate(const QString& spec);
    ~MouseHandlerPrivate();

private:
    int mouseFD;
    int mouseIdx;
    uchar mouseBuf[mouseBufSize];
    MouseProtocol mouseProtocol;
    void handleMouseData();
private slots:
    void sendRelease();
    void readMouseData();
private:
    int obstate;
    QTimer *rtimer;
};

typedef struct {
    char *name;
    MouseProtocol id;
} MouseConfig;

static const MouseConfig mouseConfig[] = {
    { "MouseMan",	MouseMan },
    { "IntelliMouse",	IntelliMouse },
    { "Microsoft",      Microsoft },
    { 0,		Unknown }
};

typedef struct {
    int bytesPerPacket;
} MouseData;

static const MouseData mouseData[] = {
    { 3 },  // MouseMan
    { 4 },  // intelliMouse
    { 3 }   // Microsoft
};

#ifdef __MIPSEL__
static int tlx=-400;
static int tly=-400;
static int brx=400;
static int bry=400;
static int addx=400;
static int addy=400;
#endif

void MouseHandlerPrivate::readMouseData()
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

    #define EMIT_MOUSE \
	QPoint q = QPoint((prev.x()+addx)*qt_screen->width()/(brx-tlx), \
			  (prev.y()+addy)*qt_screen->height()/(bry-tly)); \
	if ( reverse ) { \
	    q.setX( qt_screen->width()-q.x() ); \
	    q.setY( qt_screen->height()-q.y() ); \
	} \
	if ( q != mousePos ) { \
	    mousePos = q; \
	    emit mouseChanged(mousePos, Qt::LeftButton); \
	    pressed = TRUE; \
	    rtimer->stop(); \
	}

    do {
	ret=read(mouseFD,data,sizeof(data));

	if(ret==sizeof(data)) {
	    // "auto calibrate" for now.
	    if ( data[0] & 0x8000 ) {
		if ( data[5] > 800 ) {
		    if ( prev_pressure - data[5] < 40 ) {
			QPoint t(data[3]-data[4],data[2]-data[1]);
			if(t.x()<tlx)
			    tlx=t.x();
			if(t.y()<tly)
			    tly=t.y();
			if(t.x()>brx)
			    brx=t.x();
			if(t.y()>bry)
			    bry=t.y();
			addx=-tlx;
			addy=-tly;
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
		    //emit mouseChanged(mousePos, 0);
		    pressed = FALSE;
		}
	    }
	}
    } while ( ret > 0 );
    if ( prev_valid > 1 ) {
	prev_valid = 0;
	EMIT_MOUSE
    }
#else
    int n;
    do {
	n = read(mouseFD, mouseBuf+mouseIdx, mouseBufSize-mouseIdx );
	if ( n > 0 )
	    mouseIdx += n;
    } while ( n > 0 );
    handleMouseData();
#endif
}

void MouseHandlerPrivate::sendRelease()
{
    emit mouseChanged(mousePos, 0);
}


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

/*
*/

void MouseHandlerPrivate::handleMouseData()
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
		qWarning( "Unknown mouse protocol in MouseHandlerPrivate" );
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


MouseHandlerPrivate::MouseHandlerPrivate(const QString& spec)
{
#ifdef QWS_TOUCHPANEL

    if ((mouseFD = open( "/dev/tpanel", O_RDONLY)) < 0) {
        qFatal( "Cannot open /dev/tpanel (%s)", strerror(errno));
    } else {
        qDebug("Opened panel %d",mouseFD);
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
#else
    static int init=0;
    if ( !init && qt_screen ) {
	init = 1;
	mousePos = QPoint(qt_screen->width()/2,
			  qt_screen->height()/2);
    }

    obstate = -1;
    int c = spec.find(':');
    QString mouseProto;
    QString mouseDev;
    if ( c >= 0 ) {
	mouseProto = spec.left(c);
	mouseDev = spec.mid(c+1);
    } else {
	mouseProto = spec;
	mouseDev = "/dev/mouse";
    }
    if ((mouseFD = open( mouseDev.local8Bit(), O_RDWR | O_NDELAY)) < 0) {
	qDebug( "Cannot open %s (%s)", mouseDev.ascii(),
		strerror(errno));
    } else {
	int idx = 0;
	mouseProtocol = Unknown;
	while (mouseProtocol == Unknown && mouseConfig[idx].name) {
	    if (mouseProto == QString(mouseConfig[idx].name)) {
		mouseProtocol = mouseConfig[idx].id;
	    }
	    idx++;
	}

	if (mouseProtocol == Unknown) {
	    qDebug("Unknown mouse protocol: %s", mouseProto.ascii());
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
			qDebug("Init intellimouse");
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
	    while (read(mouseFD, buf, 1) > 0);  // eat unwanted replies

	    mouseIdx = 0;

	    QSocketNotifier *mouseNotifier;
	    mouseNotifier = new QSocketNotifier( mouseFD, QSocketNotifier::Read, this );
	    connect(mouseNotifier, SIGNAL(activated(int)),this, SLOT(readMouseData()));
	}
    }
#endif
}
MouseHandlerPrivate::~MouseHandlerPrivate()
{
    if (mouseFD >= 0)
	close(mouseFD);
}

MouseHandler* QWSServer::newMouseHandler(const QString& spec)
{
    return new MouseHandlerPrivate(spec);
}

#include "qwsmouse_qws.moc"





