/*****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpaintdevice.h#73 $
**
** Implementation of QGfxvnc (remote frame buffer driver)
** Proof of concept driver only.
** 
** Created : 20000703
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing.
**
*****************************************************************************/

#include "qgfxraster_qws.h"

#ifndef QT_NO_QWS_VNC

#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <netinet/in.h>
#include "qtimer.h"
#include "qwindowsystem_qws.h"
#include "qgfxvnc_qws.h"

#define MAP_TILE_SIZE	16
#define MAP_WIDTH	1280/MAP_TILE_SIZE
#define MAP_HEIGHT	1024/MAP_TILE_SIZE

bool qvncEnabled = FALSE;

struct QVNCHeader
{
    bool dirty;
    uchar map[MAP_HEIGHT][MAP_WIDTH];
};

class QVNCScreen : public QScreen
{
public:
    QVNCScreen();
    virtual ~QVNCScreen();
    virtual bool initCard();
    virtual bool connect();
    virtual void disconnect();
    virtual int initCursor(void*, bool);
    virtual void shutdownCard();
    virtual QGfx * createGfx(unsigned char *,int,int,int,int);
    virtual void save();
    virtual void restore();
    virtual void setMode(int nw,int nh,int nd);

    void setDirty( QRect r )
	{ hdr->dirty = TRUE;
	  int x1 = r.x()/MAP_TILE_SIZE;
	  int y1 = r.y()/MAP_TILE_SIZE;
	  for ( int y = y1; y <= r.bottom()/MAP_TILE_SIZE && y < MAP_HEIGHT; y++ )
	      for ( int x = x1; x <= r.right()/MAP_TILE_SIZE && x < MAP_WIDTH; x++ )
		  hdr->map[y][x] = 1;
	}

    bool success;
    QVNCServer *vncServer;
    unsigned char *shmrgn;
    QVNCHeader *hdr;
};

static QVNCScreen *qvnc_screen = 0;

static struct {
    int keysym;
    int keycode;
} keyMap[] = {
    { 0xff08, Qt::Key_Backspace },
    { 0xff09, Qt::Key_Tab       },
    { 0xff0d, Qt::Key_Return    },
    { 0xff1b, Qt::Key_Escape	},
    { 0xff63, Qt::Key_Insert	},
    { 0xffff, Qt::Key_Delete	},
    { 0xff50, Qt::Key_Home	},
    { 0xff57, Qt::Key_End	},
    { 0xff55, Qt::Key_Prior	},
    { 0xff56, Qt::Key_Next	},
    { 0xff51, Qt::Key_Left	},
    { 0xff52, Qt::Key_Up  	},
    { 0xff53, Qt::Key_Right	},
    { 0xff54, Qt::Key_Down	},
    { 0xffbe, Qt::Key_F1  	},
    { 0xffbf, Qt::Key_F2  	},
    { 0xffc0, Qt::Key_F3  	},
    { 0xffc1, Qt::Key_F4  	},
    { 0xffc2, Qt::Key_F5  	},
    { 0xffc3, Qt::Key_F6  	},
    { 0xffc4, Qt::Key_F7  	},
    { 0xffc5, Qt::Key_F8  	},
    { 0xffc6, Qt::Key_F9  	},
    { 0xffc7, Qt::Key_F10 	},
    { 0xffc8, Qt::Key_F11 	},
    { 0xffc9, Qt::Key_F12 	},
    { 0xffe1, Qt::Key_Shift	},
    { 0xffe2, Qt::Key_Shift	},
    { 0xffe3, Qt::Key_Control	},
    { 0xffe4, Qt::Key_Control	},
    { 0xffe7, Qt::Key_Meta  	},
    { 0xffe8, Qt::Key_Meta  	},
    { 0xffe9, Qt::Key_Alt   	},
    { 0xffea, Qt::Key_Alt   	},
    { 0, 0 }
};

/*
 */
void QRfbRect::read( QSocket *s )
{
    Q_UINT16 buf[4];
    s->readBlock( (char*)buf, 8 );
    x = ntohs( buf[0] );
    y = ntohs( buf[1] );
    w = ntohs( buf[2] );
    h = ntohs( buf[3] );
}

void QRfbRect::write( QSocket *s )
{
    Q_UINT16 buf[4];
    buf[0] = htons( x );
    buf[1] = htons( y );
    buf[2] = htons( w );
    buf[3] = htons( h );
    s->writeBlock( (char*)buf, 8 );
}

/*
 */
void QRfbPixelFormat::read( QSocket *s )
{
    char buf[16];
    s->readBlock( buf, 16 );
    bitsPerPixel = buf[0];
    depth = buf[1];
    bigEndian = buf[2];
    trueColor = buf[3];

    Q_UINT16 a = ntohs( *(Q_UINT16 *)(buf + 4) );
    redBits = 0;
    while ( a ) { a >>= 1; redBits++; }

    a = ntohs( *(Q_UINT16 *)(buf + 6) );
    greenBits = 0;
    while ( a ) { a >>= 1; greenBits++; }

    a = ntohs( *(Q_UINT16 *)(buf + 8) );
    blueBits = 0;
    while ( a ) { a >>= 1; blueBits++; }

    redShift = buf[10];
    greenShift = buf[11];
    blueShift = buf[12];
}

void QRfbPixelFormat::write( QSocket *s )
{
    char buf[16];
    buf[0] = bitsPerPixel;
    buf[1] = depth;
    buf[2] = bigEndian;
    buf[3] = trueColor;

    Q_UINT16 a = 0;
    for ( int i = 0; i < redBits; i++ ) a = (a << 1) | 1;
    *(Q_UINT16 *)(buf + 4) = htons( a );

    a = 0;
    for ( int i = 0; i < greenBits; i++ ) a = (a << 1) | 1;
    *(Q_UINT16 *)(buf + 6) = htons( a );

    a = 0;
    for ( int i = 0; i < blueBits; i++ ) a = (a << 1) | 1;
    *(Q_UINT16 *)(buf + 8) = htons( a );
    
    buf[10] = redShift;
    buf[11] = greenShift;
    buf[12] = blueShift;
    s->writeBlock( buf, 16 );
}


/*
 */
void QRfbServerInit::setName( const char *n )
{
    delete name;
    name = new char [strlen(n) + 1];
    strcpy( name, n );
}

void QRfbServerInit::read( QSocket *s )
{
    s->readBlock( (char *)&width, 2 );
    width = ntohs( width );
    s->readBlock( (char *)&height, 2 );
    height = ntohs( height );
    format.read( s );
    
    Q_UINT32 len;
    s->readBlock( (char *)&len, 4 );
    len = ntohl( len );

    name = new char [len + 1];
    s->readBlock( name, len );
    name[len] = '\0';
}

void QRfbServerInit::write( QSocket *s )
{
    Q_UINT16 t = htons(width);
    s->writeBlock( (char *)&t, 2 );
    t = htons(height);
    s->writeBlock( (char *)&t, 2 );
    format.write( s );
    Q_UINT32 len = strlen( name );
    len = htonl(len);
    s->writeBlock( (char *)&len, 4 );
    s->writeBlock( name, strlen( name ) );
}

/*
 */
bool QRfbSetEncodings::read( QSocket *s )
{
    if ( s->bytesAvailable() < 3 )
	return FALSE;

    char tmp;
    s->readBlock( &tmp, 1 );	// padding
    s->readBlock( (char *)&count, 2 );
    count = ntohs( count );

    return TRUE;
}

/*
 */
bool QRfbFrameBufferUpdateRequest::read( QSocket *s )
{
    if ( s->bytesAvailable() < 9 )
	return FALSE;

    s->readBlock( &incremental, 1 );
    rect.read( s );

    return TRUE;
}

/*
 */
bool QRfbKeyEvent::read( QSocket *s )
{
    if ( s->bytesAvailable() < 7 )
	return FALSE;

    s->readBlock( &down, 1 );
    Q_UINT16 tmp;
    s->readBlock( (char *)&tmp, 2 );  // padding

    Q_UINT32 key;
    s->readBlock( (char *)&key, 4 );
    key = ntohl( key );

    unicode = 0;
    keycode = 0;
    int i = 0;
    while ( keyMap[i].keysym && !keycode ) {
	if ( keyMap[i].keysym == (int)key )
	    keycode = keyMap[i].keycode;
	i++;
    }
    if ( !keycode ) {
	if ( key <= 0xff ) {
	    unicode = key;
	    if ( key >= ' ' && key <= 'z' )
		keycode = Qt::Key_Space + key - ' ';
	    else if ( key >= 'A' && key <= 'Z' )
		keycode = Qt::Key_A + key - 'A';
	}
    }

    return TRUE;
}

/*
 */
bool QRfbPointerEvent::read( QSocket *s )
{
    if ( s->bytesAvailable() < 5 )
	return FALSE;

    char buttonMask;
    s->readBlock( &buttonMask, 1 );
    buttons = 0;
    if ( buttonMask & 1 )
	buttons |= Qt::LeftButton;
    if ( buttonMask & 2 )
	buttons |= Qt::MidButton;
    if ( buttonMask & 4 )
	buttons |= Qt::RightButton;

    Q_UINT16 tmp;
    s->readBlock( (char *)&tmp, 2 );
    x = ntohs( tmp );
    s->readBlock( (char *)&tmp, 2 );
    y = ntohs( tmp );

    return TRUE;
}

/*
 */
bool QRfbClientCutText::read( QSocket *s )
{
    if ( s->bytesAvailable() < 7 )
	return FALSE;

    char tmp[3];
    s->readBlock( tmp, 3 );	// padding
    s->readBlock( (char *)&length, 4 );
    length = ntohl( length );

    return TRUE;
}

/*
 */
QVNCServer::QVNCServer()
    : QServerSocket( 5900, 0, 0 )
{
    qDebug( "QVNCServer created" );
    handleMsg = FALSE;
    client = 0;
    encodingsPending = 0;
    cutTextPending = 0;
    keymod = 0;
}

QVNCServer::~QVNCServer()
{
    delete client;
}

void QVNCServer::newConnection( int socket )
{
    qDebug("new connection");
    if ( client ) {
	qDebug( "Killing old client" );
	delete client;
    }
    client = new QSocket(this);
    connect(client,SIGNAL(readyRead()),this,SLOT(readClient()));
    connect(client,SIGNAL(delayedCloseFinished()),this,SLOT(discardClient()));
    client->setSocket(socket);
    client->setMode(QSocket::Binary);
    handleMsg = FALSE;
    encodingsPending = 0;
    cutTextPending = 0;
    supportHextile = FALSE;

    // send protocol version
    char *proto = "RFB 003.003\n";
    client->writeBlock( proto, 12 );
    state = Protocol;
}

void QVNCServer::readClient()
{
    switch ( state ) {
	case Protocol:
	    if ( client->bytesAvailable() >= 12 ) {
		char proto[13];
		client->readBlock( proto, 12 );
		proto[12] = '\0';
		qDebug( "Client protocol version %s", proto );
		// No authentication
		Q_UINT32 auth = htonl(1);
		client->writeBlock( (char *) &auth, sizeof(auth) );
		state = Init;
	    }
	    break;

	case Init:
	    if ( client->bytesAvailable() >= 1 ) {
		Q_UINT8 shared;
		client->readBlock( (char *) &shared, 1 );
		qDebug( "Read client init message" );

		// Server Init msg
		QRfbServerInit sim;
		QRfbPixelFormat &format = sim.format;
		switch ( qvnc_screen->depth() ) {
		    case 32:
			format.bitsPerPixel = 32;
			format.depth = 32;
			format.bigEndian = 0;
			format.trueColor = TRUE;
			format.redBits = 8;
			format.greenBits = 8;
			format.blueBits = 8;
			format.redShift = 16;
			format.greenShift = 8;
			format.blueShift = 0;
			break;

		    case 16:
			format.bitsPerPixel = 16;
			format.depth = 16;
			format.bigEndian = 0;
			format.trueColor = TRUE;
			format.redBits = 5;
			format.greenBits = 6;
			format.blueBits = 5;
			format.redShift = 11;
			format.greenShift = 5;
			format.blueShift = 0;
			break;

		    default:
			qFatal( "QVNC cannot drive depth %d", qvnc_screen->depth() );
		}
		sim.width = qvnc_screen->width();
		sim.height = qvnc_screen->height();
		sim.setName( "Qt/Embedded VNC Server" );
		sim.write( client );
		state = Connected;
	    }
	    break;

	case Connected:
	    do {
		if ( !handleMsg ) {
		    client->readBlock( (char *)&msgType, 1 );
		    handleMsg = TRUE;
		}
		if ( handleMsg ) {
		    switch ( msgType  ) {
			case SetPixelFormat:
			    setPixelFormat();
			    break;
			case FixColourMapEntries:
			    qDebug( "Arrrgh: got FixColourMapEntries" );
			    handleMsg = FALSE;
			    break;
			case SetEncodings:
			    setEncodings();
			    break;
			case FramebufferUpdateRequest:
			    frameBufferUpdateRequest();
			    break;
			case KeyEvent:
			    keyEvent();
			    break;
			case PointerEvent:
			    pointerEvent();
			    break;
			case ClientCutText:
			    clientCutText();
			    break;
			default:
			    qDebug( "Unknown message type: %d", (int)msgType );
			    handleMsg = FALSE;
		    }
		}
	    } while ( !handleMsg && client->bytesAvailable() );
	    break;
    }
}

void QVNCServer::setPixelFormat()
{
    if ( client->bytesAvailable() >= 19 ) {
	char buf[3];
	client->readBlock( buf, 3 ); // just padding
	pixelFormat.read( client );
	qDebug( "Want format: %d %d %d %d %d %d %d %d %d %d",
	    (int)pixelFormat.bitsPerPixel,
	    (int)pixelFormat.depth,
	    (int)pixelFormat.bigEndian,
	    (int)pixelFormat.trueColor,
	    (int)pixelFormat.redBits,
	    (int)pixelFormat.greenBits,
	    (int)pixelFormat.blueBits,
	    (int)pixelFormat.redShift,
	    (int)pixelFormat.greenShift,
	    (int)pixelFormat.blueShift );

	if ( pixelFormat.bitsPerPixel != 16 && pixelFormat.bitsPerPixel != 32 ) {
	    qDebug( "Cannot handle %d bpp client", pixelFormat.bitsPerPixel );
	    discardClient();
	}
	handleMsg = FALSE;
    }
}

void QVNCServer::setEncodings()
{
    QRfbSetEncodings enc;

    if ( !encodingsPending && enc.read( client ) ) {
	encodingsPending = enc.count;
	if ( !encodingsPending )
	    handleMsg = FALSE;
    }

    if ( encodingsPending && (unsigned)client->bytesAvailable() >=
				encodingsPending * sizeof(Q_UINT32) ) {
	for ( int i = 0; i < encodingsPending; i++ ) {
	    Q_UINT32 enc;
	    client->readBlock((char *)&enc, sizeof(Q_UINT32));
	    enc = ntohl( enc );
	    if ( enc == 5 )
		supportHextile = TRUE;
	    qDebug( "Can do %d", enc );
	}
	handleMsg = FALSE;
	encodingsPending = 0;
	qDebug( "Read SetEncodingsMsg" );
    }
}

void QVNCServer::frameBufferUpdateRequest()
{
    QRfbFrameBufferUpdateRequest ev;

    if ( ev.read( client ) ) {
	if ( !ev.incremental ) {
	    QWSDisplay::grab( TRUE );
	    QRect r( ev.rect.x, ev.rect.y, ev.rect.w, ev.rect.h );
	    qvnc_screen->setDirty( r );
	    QWSDisplay::ungrab();
	}
	if ( supportHextile )
	    sendHextile();
	else
	    sendRaw();
	handleMsg = FALSE;
    }
}

void QVNCServer::pointerEvent()
{
    QRfbPointerEvent ev;
    if ( ev.read( client ) ) {
	QWSServer::sendMouseEvent( QPoint(ev.x, ev.y), ev.buttons );
	handleMsg = FALSE;
    }
}

void QVNCServer::keyEvent()
{
    QRfbKeyEvent ev;

    if ( ev.read( client ) ) {
	if ( ev.keycode == Qt::Key_Shift )
	    keymod = ev.down ? keymod | Qt::ShiftButton :
			       keymod & ~Qt::ShiftButton;
	else if ( ev.keycode == Qt::Key_Control )
	    keymod = ev.down ? keymod | Qt::ControlButton :
			       keymod & ~Qt::ControlButton;
	else if ( ev.keycode == Qt::Key_Alt )
	    keymod = ev.down ? keymod | Qt::AltButton :
			       keymod & ~Qt::AltButton;
	if ( ev.unicode || ev.keycode )
	    QWSServer::sendKeyEvent( ev.unicode, ev.keycode, keymod,
				     ev.down, FALSE );
	handleMsg = FALSE;
    }
}

void QVNCServer::clientCutText()
{
    QRfbClientCutText ev;

    if ( ev.read( client ) ) {
	cutTextPending = ev.length;
	if ( !cutTextPending )
	    handleMsg = FALSE;
    }

    if ( cutTextPending && client->bytesAvailable() >= cutTextPending ) {
	char *text = new char [cutTextPending+1];
	client->readBlock( text, cutTextPending );
	delete text;
	cutTextPending = 0;
	handleMsg = FALSE;
    }
}

bool QVNCServer::checkFill( const QRfbRect &rect )
{
    if ( qvnc_screen->depth() == 16 ) {
	ushort *data = (ushort *)(qvnc_screen->base() +
			rect.y * qvnc_screen->linestep() + rect.x * 2);
	ushort pixel = *data;
	for ( int i = rect.y; i < rect.y+rect.h; i++ ) {
	    data = (ushort *)(qvnc_screen->base() +
		    i * qvnc_screen->linestep() + rect.x * 2);
	    for ( int j = 0; j < rect.w; j++ ) {
		if ( pixel != *data )
		    return FALSE;
		data++;
	    }
	}
    } else if ( qvnc_screen->depth() == 32 ) {
	Q_UINT32 *data = (Q_UINT32 *)(qvnc_screen->base() +
			rect.y * qvnc_screen->linestep() + rect.x * 4);
	Q_UINT32 pixel = *data;
	for ( int i = rect.y; i < rect.y+rect.h; i++ ) {
	    data = (Q_UINT32 *)(qvnc_screen->base() +
		    i * qvnc_screen->linestep() + rect.x * 4);
	    for ( int j = 0; j < rect.w; j++ ) {
		if ( pixel != *data )
		    return FALSE;
		data++;
	    }
	}
    }

    return TRUE;
}

int QVNCServer::getPixel( uchar **data )
{
    int r, g, b;

    if ( qvnc_screen->depth() == 16 ) {
	ushort p = *((ushort *)*data);
	r = (p >> 11) & 0x1f;
	g = (p >> 5) & 0x3f;
	b = p & 0x1f;
	r <<= 3;
	g <<= 2;
	b <<= 3;
	*data += 2;
    } else if ( qvnc_screen->depth() == 32 ) {
	uint p = *((uint *)*data);
	r = (p >> 16) & 0xff;
	g = (p >> 8) & 0xff;
	b = p & 0xff;
	*data += 4;
    }

    r >>= (8 - pixelFormat.redBits);
    g >>= (8 - pixelFormat.greenBits);
    b >>= (8 - pixelFormat.blueBits);

    return (r << pixelFormat.redShift) |
	   (g << pixelFormat.greenShift) |
	   (b << pixelFormat.blueShift);
}

void QVNCServer::sendHextile()
{
    QWSDisplay::grab( TRUE );

    Q_UINT16 count = 0;
    int vtiles = (qvnc_screen->height()+MAP_TILE_SIZE-1)/MAP_TILE_SIZE;
    int htiles = (qvnc_screen->width()+MAP_TILE_SIZE-1)/MAP_TILE_SIZE;
    if ( qvnc_screen->hdr->dirty ) {
	for ( int y = 0; y < vtiles; y++ )
	    for ( int x = 0; x < htiles; x++ )
		if ( qvnc_screen->hdr->map[y][x] )
		    count++;
    }

    char tmp = 0;
    client->writeBlock( &tmp, 1 ); // type
    client->writeBlock( &tmp, 1 ); // padding
    count = htons( count );
    client->writeBlock( (char *)&count, 2 );

    if ( qvnc_screen->hdr->dirty ) {
	QRfbRect rect;
	rect.y = 0;
	rect.h = MAP_TILE_SIZE;
	for ( int y = 0; y < vtiles; y++ ) {
	    if ( rect.y + MAP_TILE_SIZE > qvnc_screen->height() )
		rect.h = qvnc_screen->height() - rect.y;
	    rect.x = 0;
	    rect.w = MAP_TILE_SIZE;
	    for ( int x = 0; x < htiles; x++ ) {
		if ( qvnc_screen->hdr->map[y][x] ) {
		    if ( rect.x + MAP_TILE_SIZE > qvnc_screen->width() )
			rect.w = qvnc_screen->width() - rect.x;
		    rect.write( client );

		    Q_UINT32 encoding = htonl(5);	// hextile encoding
		    client->writeBlock( (char *)&encoding, 4 );

		    if ( checkFill( rect ) ) {
			Q_UINT8 subenc = 2;			// subencoding
			client->writeBlock( (char *)&subenc, 1 );
			int pixel;
			uchar *data = qvnc_screen->base() +
					rect.y * qvnc_screen->linestep() +
					rect.x * qvnc_screen->depth() / 8;
			pixel = getPixel( &data );
			client->writeBlock( (char *)&pixel, pixelFormat.bitsPerPixel/8);
		    } else {
			Q_UINT8 subenc = 1;			// subencoding
			client->writeBlock( (char *)&subenc, 1 );
			int pixel;
			for ( int i = rect.y; i < rect.y+rect.h; i++ ) {
			    uchar *data = qvnc_screen->base() +
					    i * qvnc_screen->linestep() +
					    rect.x * qvnc_screen->depth() / 8;
			    for ( int j = 0; j < rect.w; j++ ) {
				pixel = getPixel( &data );
				client->writeBlock( (char *)&pixel, pixelFormat.bitsPerPixel/8);
			    }
			}
		    }
		}
		rect.x += MAP_TILE_SIZE;
	    }
	    rect.y += MAP_TILE_SIZE;
	}

	qvnc_screen->hdr->dirty = FALSE;
	memset( qvnc_screen->hdr->map, 0, MAP_WIDTH*MAP_HEIGHT );
    }

    QWSDisplay::ungrab();
}

void QVNCServer::sendRaw()
{
    QWSDisplay::grab( TRUE );

    QRegion rgn;

    if ( qvnc_screen->hdr->dirty ) {
	for ( int y = 0; y < MAP_HEIGHT; y++ )
	    for ( int x = 0; x < MAP_WIDTH; x++ )
		if ( qvnc_screen->hdr->map[y][x] )
		    rgn += QRect( x*MAP_TILE_SIZE, y*MAP_TILE_SIZE, MAP_TILE_SIZE, MAP_TILE_SIZE );

	rgn &= QRect( 0, 0, qvnc_screen->width()-1, qvnc_screen->height()-1 );
    }

    char tmp = 0;
    client->writeBlock( &tmp, 1 ); // type
    client->writeBlock( &tmp, 1 ); // padding
    Q_UINT16 count = htons( rgn.rects().count() );
    client->writeBlock( (char *)&count, 2 );

    for ( unsigned int idx = 0; idx < rgn.rects().count(); idx++ ) {
	QRfbRect rect;
	rect.x = rgn.rects()[idx].x();
	rect.y = rgn.rects()[idx].y();
	rect.w = rgn.rects()[idx].width();
	rect.h = rgn.rects()[idx].height();
	rect.write( client );

	Q_UINT32 encoding = htonl(0);	// raw encoding
	client->writeBlock( (char *)&encoding, 4 );

	int pixel;
	for ( int i = rect.y; i < rect.y+rect.h; i++ ) {
	    uchar *data = qvnc_screen->base() + i * qvnc_screen->linestep() +
			    rect.x * qvnc_screen->depth() / 8;
	    for ( int j = 0; j < rect.w; j++ ) {
		pixel = getPixel( &data );
		client->writeBlock( (char *)&pixel, pixelFormat.bitsPerPixel/8);
	    }
	}
    }
    qvnc_screen->hdr->dirty = FALSE;
    memset( qvnc_screen->hdr->map, 0, MAP_WIDTH*MAP_HEIGHT );

    QWSDisplay::ungrab();
}

void QVNCServer::discardClient()
{
    delete client;
    client = 0;
    qDebug( "QVNCServer::discardClient()" );
}

/*
 */
#ifndef QT_NO_QWS_CURSOR
class QVNCScreenCursor : public QScreenCursor
{
public:
    QVNCScreenCursor();

    virtual void set( const QImage &image, int hotx, int hoty );
    virtual void move( int x, int y );
};

QVNCScreenCursor::QVNCScreenCursor() : QScreenCursor()
{
}

void QVNCScreenCursor::set( const QImage &image, int hotx, int hoty )
{
    QWSDisplay::grab( TRUE );
    QRect r( data->x - hotx, data->y - hoty, image.width(), image.height() );
    qvnc_screen->setDirty( data->bound | r );
    QScreenCursor::set( image, hotx, hoty );
    QWSDisplay::ungrab();
}

void QVNCScreenCursor::move( int x, int y )
{
    QWSDisplay::grab( TRUE );
    QRect r( x - data->hotx, y - data->hoty, data->width, data->height );
    qvnc_screen->setDirty( r | data->bound );
    QScreenCursor::move( x, y );
    QWSDisplay::ungrab();
}
#endif


template <const int depth, const int type>
class QGfxVNC : public QGfxRaster<depth,type>
{
public:
    QGfxVNC(unsigned char *b,int w,int h);
    virtual ~QGfxVNC();

    virtual void drawPoint( int,int );
    virtual void drawPoints( const QPointArray &,int,int );
    virtual void drawLine( int,int,int,int );
    virtual void drawRect( int,int,int,int );
    virtual void drawPolyline( const QPointArray &,int,int );
    virtual void drawPolygon( const QPointArray &,bool,int,int );
    virtual void blt( int,int,int,int );
    virtual void scroll( int,int,int,int,int,int );
#if !defined(QT_NO_MOVIE) || !defined(QT_NO_TRANSFORMATIONS)
    virtual void stretchBlt( int,int,int,int,int,int );
#endif
    virtual void tiledBlt( int,int,int,int );
};

template <const int depth, const int type>
QGfxVNC<depth,type>::QGfxVNC(unsigned char *b,int w,int h)
    : QGfxRaster( b, w, h )
{
}

template <const int depth, const int type>
QGfxVNC<depth,type>::~QGfxVNC()
{
}

template <const int depth, const int type>
void QGfxVNC<depth,type>::drawPoint( int x, int y )
{
    QWSDisplay::grab( TRUE );
    qvnc_screen->setDirty( QRect( x+xoffs, y+yoffs, 1, 1 ) & clipbounds );
    QGfxRaster<depth,type>::drawPoint( x, y );
    QWSDisplay::ungrab();
}

template <const int depth, const int type>
void QGfxVNC<depth,type>::drawPoints( const QPointArray &pa,int x,int y )
{
    QWSDisplay::grab( TRUE );
    QRect r = pa.boundingRect();
    r.moveBy( xoffs, yoffs );
    qvnc_screen->setDirty( r & clipbounds );
    QGfxRaster<depth,type>::drawPoints( pa, x, y );
    QWSDisplay::ungrab();
}

template <const int depth, const int type>
void QGfxVNC<depth,type>::drawLine( int x1,int y1,int x2,int y2 )
{
    QWSDisplay::grab( TRUE );
    QRect r;
    r.setCoords( x1+xoffs, y1+yoffs, x2+xoffs, y2+yoffs );
    qvnc_screen->setDirty( r & clipbounds );
    QGfxRaster<depth,type>::drawLine( x1, y1, x2, y2 );
    QWSDisplay::ungrab();
}

template <const int depth, const int type>
void QGfxVNC<depth,type>::drawRect( int x,int y,int w,int h )
{
    QWSDisplay::grab( TRUE );
    qvnc_screen->setDirty( QRect( x+xoffs, y+yoffs, w, h ) & clipbounds );
    QGfxRaster<depth,type>::drawRect( x, y, w, h );
    QWSDisplay::ungrab();
}

template <const int depth, const int type>
void QGfxVNC<depth,type>::drawPolyline( const QPointArray &pa,int x,int y )
{
    QWSDisplay::grab( TRUE );
    QRect r = pa.boundingRect();
    r.moveBy( xoffs, yoffs );
    qvnc_screen->setDirty( r & clipbounds );
    QGfxRaster<depth,type>::drawPolyline( pa, x, y );
    QWSDisplay::ungrab();
}

template <const int depth, const int type>
void QGfxVNC<depth,type>::drawPolygon( const QPointArray &pa,bool w,int x,int y )
{
    QWSDisplay::grab( TRUE );
    QRect r = pa.boundingRect();
    r.moveBy( xoffs, yoffs );
    qvnc_screen->setDirty( r & clipbounds );
    QGfxRaster<depth,type>::drawPolygon( pa, w, x, y );
    QWSDisplay::ungrab();
}

template <const int depth, const int type>
void QGfxVNC<depth,type>::blt( int x,int y,int w,int h )
{
    QWSDisplay::grab( TRUE );
    qvnc_screen->setDirty( QRect( x+xoffs, y+yoffs, w, h ) & clipbounds );
    QGfxRaster<depth,type>::blt( x, y, w, h );
    QWSDisplay::ungrab();
}

template <const int depth, const int type>
void QGfxVNC<depth,type>::scroll( int x,int y,int w,int h,int sx,int sy )
{
    QWSDisplay::grab( TRUE );
    int dy = sy - y;
    int dx = sx - x;
    qvnc_screen->setDirty( QRect(QMIN(x,sx) + xoffs, QMIN(y,sy) + yoffs,
			   w+abs(dx), h+abs(dy)) & clipbounds );
    QGfxRaster<depth,type>::scroll( x, y, w, h, sx, sy );
    QWSDisplay::ungrab();
}

#if !defined(QT_NO_MOVIE) || !defined(QT_NO_TRANSFORMATIONS)
template <const int depth, const int type>
void QGfxVNC<depth,type>::stretchBlt( int x,int y,int w,int h,int sx,int sy )
{
    QWSDisplay::grab( TRUE );
    qvnc_screen->setDirty( QRect( x + xoffs, y + yoffs, w, h) & clipbounds );
    QGfxRaster<depth,type>::stretchBlt( x, y, w, h, sx, sy );
    QWSDisplay::ungrab();
}
#endif

template <const int depth, const int type>
void QGfxVNC<depth,type>::tiledBlt( int x,int y,int w,int h )
{
    QWSDisplay::grab( TRUE );
    qvnc_screen->setDirty( QRect(x + xoffs, y + yoffs, w, h) & clipbounds );
    QGfxRaster<depth,type>::tiledBlt( x, y, w, h );
    QWSDisplay::ungrab();
}


/*
*/

QVNCScreen::QVNCScreen() : QScreen()
{
}

QVNCScreen::~QVNCScreen()
{
}

bool QVNCScreen::connect()
{
    QScreen::connect();

    key_t key = ftok( "/dev/fb0", 'v' );
     
    int shmId = shmget( key, 0, 0 );
    if ( shmId != -1 )
	shmrgn = (unsigned char *)shmat( shmId, 0, 0 );
    else {
	struct shmid_ds shm;
	shmctl( shmId, IPC_RMID, &shm );
	shmId = shmget( key, sizeof(QVNCHeader), IPC_CREAT|0666);
	shmrgn = (unsigned char *)shmat( shmId, 0, 0 );
    }

    if ( (int)shmrgn == -1 || shmrgn == 0 )
	return FALSE;

    hdr = (QVNCHeader *) shmrgn;

    return TRUE;
}

void QVNCScreen::disconnect()
{
    QScreen::disconnect();
    shmdt( shmrgn );
}

bool QVNCScreen::initCard()
{
    QScreen::initCard();

    vncServer = new QVNCServer();

    hdr->dirty = FALSE;
    memset( qvnc_screen->hdr->map, 0, MAP_WIDTH*MAP_HEIGHT );

    return true;
}

void QVNCScreen::shutdownCard()
{
    delete vncServer;
    QScreen::shutdownCard();
}

int QVNCScreen::initCursor(void* e, bool init)
{
#ifndef QT_NO_QWS_CURSOR
    qt_sw_cursor=true;
    // ### until QLumpManager works Ok with multiple connected clients,
    // we steal a chunk of shared memory
    SWCursorData *data = (SWCursorData *)e - 1;
    qt_screencursor=new QVNCScreenCursor();
    qt_screencursor->init( data, init );
    return sizeof(SWCursorData);
#else
    return 0;
#endif
}

void QVNCScreen::setMode(int ,int ,int)
{
}

// save the state of the graphics card
// This is needed so that e.g. we can restore the palette when switching
// between linux virtual consoles.
void QVNCScreen::save()
{
    QScreen::save();
}

// restore the state of the graphics card.
void QVNCScreen::restore()
{
    QScreen::restore();
}

QGfx * QVNCScreen::createGfx(unsigned char * bytes,int w,int h,int d, int linestep)
{
    QGfx* ret = 0;
    if(d==1) {
	ret = new QGfxRaster<1,0>(bytes,w,h);
#ifndef QT_NO_QWS_DEPTH_16
    } else if(d==16) {
	if ( bytes == qt_screen->base() )
	    ret = new QGfxVNC<16,0>(bytes,w,h);
	else
	    ret = new QGfxRaster<16,0>(bytes,w,h);
#endif
#ifndef QT_NO_QWS_DEPTH_15
    } else if(d==15) {
	ret = new QGfxRaster<15,0>(bytes,w,h);
#endif
#ifndef QT_NO_QWS_DEPTH_8
    } else if (d==8) {
	ret = new QGfxRaster<8,0>(bytes,w,h);
#endif
#ifndef QT_NO_QWS_DEPTH_8GRAYSCALE
    } else if (d==8) {
	ret = new QGfxRaster<8,0>(bytes,w,h);
#endif
#ifndef QT_NO_QWS_DEPTH_32
    } else if (d==32) {
	if ( bytes == qt_screen->base() )
	    ret = new QGfxVNC<32,0>(bytes,w,h);
	else
	    ret = new QGfxRaster<32,0>(bytes,w,h);
#endif
    } else {
	qFatal("Can't drive depth %d",d);
    }
    ret->setLineStep(linestep);
    return ret;
}

extern "C" QScreen * qt_get_screen(char *,unsigned char *)
{
    if ( !qt_screen ) {
	qvnc_screen = new QVNCScreen();
	if ( qvnc_screen->connect() ) {
	    qt_screen = qvnc_screen;
	    qvncEnabled = TRUE;
	} else {
	    delete qvnc_screen;
	}
    }

    if ( !qt_screen ) {
	const char *term = getenv( "TERM" );
	if ( QString( term ) == "xterm" ) {
	    qFatal( "$TERM=xterm - To continue would corrupt X11 - aborting" );
	}
	qt_screen = new QScreen();
	qt_screen->connect();
    }

    return qt_screen;
}

#endif // QT_NO_QWS_VNC


