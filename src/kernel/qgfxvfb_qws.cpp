/*****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpaintdevice.h#73 $
**
** Implementation of QGfxvfb (virtual frame buffer driver)
** 
** Created : 940721
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
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

#if QT_FEATURE_QWS_VFB

#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/shm.h>

#include "qvfbhdr_qws.h"

// Unaccelerated screen/driver setup. Can be overridden by accelerated
// drivers

class QVFbScreen : public QScreen
{
public:
    QVFbScreen();
    virtual ~QVFbScreen();
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
	{ hdr->dirty = true; hdr->update = hdr->update.unite( r ); }

    bool success;
    unsigned char *shmrgn;
    QVFbHeader *hdr;
};

static QVFbScreen *qvfb_screen;

class QVFbScreenCursor : public QScreenCursor
{
public:
    QVFbScreenCursor();

    virtual void set( const QImage &image, int hotx, int hoty );
    virtual void move( int x, int y );
};

QVFbScreenCursor::QVFbScreenCursor() : QScreenCursor()
{
}

void QVFbScreenCursor::set( const QImage &image, int hotx, int hoty )
{
    QWSDisplay::grab();
    qvfb_screen->setDirty( QRect( data->x - QMAX( data->hotx, hotx ),
				  data->y - QMAX( data->hoty, hoty ),
				  QMAX( image.width(), data->width ),
				  QMAX( image.height(), data->height ) ) );
    QScreenCursor::set( image, hotx, hoty );
    QWSDisplay::ungrab();
}

void QVFbScreenCursor::move( int x, int y )
{
    QWSDisplay::grab();
    QRect r( data->x - data->hotx, data->y - data->hoty,
	     data->width, data->height );
    r |= QRect( x - data->hotx, y - data->hoty, data->width, data->height );
    qvfb_screen->setDirty( r );
    QScreenCursor::move( x, y );
    QWSDisplay::ungrab();
}


template <const int depth, const int type>
class QGfxVFb : public QGfxRaster<depth,type>
{
public:
    QGfxVFb(unsigned char *b,int w,int h);

    virtual void drawPoint( int,int );
    virtual void drawPoints( const QPointArray &,int,int );
    virtual void drawLine( int,int,int,int );
    virtual void drawRect( int,int,int,int );
    virtual void drawPolyline( const QPointArray &,int,int );
    virtual void drawPolygon( const QPointArray &,bool,int,int );
    virtual void blt( int,int,int,int );
    virtual void scroll( int,int,int,int,int,int );
    virtual void stretchBlt( int,int,int,int,int,int );
    virtual void tiledBlt( int,int,int,int );
};

template <const int depth, const int type>
QGfxVFb<depth,type>::QGfxVFb(unsigned char *b,int w,int h)
    : QGfxRaster<depth, type>( b, w, h )
{
}

template <const int depth, const int type>
void QGfxVFb<depth,type>::drawPoint( int x, int y )
{
    QWSDisplay::grab();
    qvfb_screen->setDirty( QRect( x+xoffs, y+yoffs, 1, 1 ) );
    QGfxRaster<depth,type>::drawPoint( x, y );
    QWSDisplay::ungrab();
}

template <const int depth, const int type>
void QGfxVFb<depth,type>::drawPoints( const QPointArray &pa,int x,int y )
{
    QWSDisplay::grab();
    qvfb_screen->setDirty( clipbounds );
    QGfxRaster<depth,type>::drawPoints( pa, x, y );
    QWSDisplay::ungrab();
}

template <const int depth, const int type>
void QGfxVFb<depth,type>::drawLine( int x1,int y1,int x2,int y2 )
{
    QWSDisplay::grab();
    QRect r;
    r.setCoords( x1+xoffs, y1+yoffs, x2+xoffs, y2+yoffs );
    qvfb_screen->setDirty( r );
    QGfxRaster<depth,type>::drawLine( x1, y1, x2, y2 );
    QWSDisplay::ungrab();
}

template <const int depth, const int type>
void QGfxVFb<depth,type>::drawRect( int x,int y,int w,int h )
{
    QWSDisplay::grab();
    qvfb_screen->setDirty( QRect( x+xoffs, y+yoffs, w, h ) );
    QGfxRaster<depth,type>::drawRect( x, y, w, h );
    QWSDisplay::ungrab();
}

template <const int depth, const int type>
void QGfxVFb<depth,type>::drawPolyline( const QPointArray &pa,int x,int y )
{
    QWSDisplay::grab();
    qvfb_screen->setDirty( clipbounds );
    QGfxRaster<depth,type>::drawPolyline( pa, x, y );
    QWSDisplay::ungrab();
}

template <const int depth, const int type>
void QGfxVFb<depth,type>::drawPolygon( const QPointArray &pa,bool w,int x,int y )
{
    QWSDisplay::grab();
    qvfb_screen->setDirty( clipbounds );
    QGfxRaster<depth,type>::drawPolygon( pa, w, x, y );
    QWSDisplay::ungrab();
}

template <const int depth, const int type>
void QGfxVFb<depth,type>::blt( int x,int y,int w,int h )
{
    QWSDisplay::grab();
    qvfb_screen->setDirty( QRect( x+xoffs, y+yoffs, w, h ) );
    QGfxRaster<depth,type>::blt( x, y, w, h );
    QWSDisplay::ungrab();
}

template <const int depth, const int type>
void QGfxVFb<depth,type>::scroll( int x,int y,int w,int h,int sx,int sy )
{
    QWSDisplay::grab();
    int dy = sy - y;
    int dx = sx - x;
    qvfb_screen->setDirty( QRect(QMIN(x,sx) + xoffs, QMIN(y,sy) + yoffs,
			   w+abs(dx), h+abs(dy)) );
    QGfxRaster<depth,type>::scroll( x, y, w, h, sx, sy );
    QWSDisplay::ungrab();
}

template <const int depth, const int type>
void QGfxVFb<depth,type>::stretchBlt( int x,int y,int w,int h,int sx,int sy )
{
    QWSDisplay::grab();
    qvfb_screen->setDirty( QRect( x + xoffs, y + yoffs, w, h) );
    QGfxRaster<depth,type>::stretchBlt( x, y, w, h, sx, sy );
    QWSDisplay::ungrab();
}

template <const int depth, const int type>
void QGfxVFb<depth,type>::tiledBlt( int x,int y,int w,int h )
{
    QWSDisplay::grab();
    qvfb_screen->setDirty( QRect(x + xoffs, y + yoffs, w, h) );
    QGfxRaster<depth,type>::tiledBlt( x, y, w, h );
    QWSDisplay::ungrab();
}


/*
*/

QVFbScreen::QVFbScreen() : QScreen()
{
}

QVFbScreen::~QVFbScreen()
{
}

bool QVFbScreen::connect()
{
    key_t key = ftok( QT_VFB_MOUSE_PIPE, 'b' );
     
    int shmId = shmget( key, 0, 0 );
    if ( shmId != -1 )
	shmrgn = (unsigned char *)shmat( shmId, 0, 0 );

    if ( (int)shmrgn == -1 || shmrgn == 0 )
	return FALSE;

    hdr = (QVFbHeader *) shmrgn;
    data = shmrgn + hdr->dataoffset;

    w = hdr->width;
    h = hdr->height;
    d = hdr->depth;
    lstep = hdr->linestep;

    qDebug( "Connected to VFB server: %d x %d x %d", w, h, d );

    size = lstep * h;
    mapsize = size;
    screencols = 0;

    return TRUE;
}

void QVFbScreen::disconnect()
{
    shmdt( shmrgn );
}

bool QVFbScreen::initCard()
{
    if(d==8) {
	screencols=256;
#if QT_FEATURE_QWS_DEPTH_8GRAYSCALE
	// Build greyscale palette
	for(int loopc=0;loopc<256;loopc++) {
	    screenclut[loopc]=qRgb(loopc,loopc,loopc);
	}
#elif QT_FEATURE_QWS_DEPTH_8DIRECT
	for(int loopc=0;loopc<256;loopc++) {
	    int a,b,c;
	    a=((loopc & 0xe0) >> 5) << 5;
	    b=((loopc & 0x18) >> 3) << 6;
	    c=(loopc & 0x07) << 5;
	    a=a | 0x3f;
	    b=b | 0x3f;
	    c=c | 0x3f;
	    screenclut[loopc]=qRgb(cmap.red[loopc] >> 8,
				   cmap.green[loopc] >> 8,
				   cmap.blue[loopc] >> 8);
	}
#else
	// 6x6x6 216 color cube
	int idx = 0;
	for( int ir = 0x0; ir <= 0xff; ir+=0x33 ) {
	    for( int ig = 0x0; ig <= 0xff; ig+=0x33 ) {
		for( int ib = 0x0; ib <= 0xff; ib+=0x33 ) {
		    screenclut[idx]=qRgb( ir, ig, ib );
		    idx++;
		}
	    }
	}
	screencols=idx;
#endif
	memcpy( hdr->clut, screenclut, sizeof( QRgb ) * screencols );
    }

    return true;
}

void QVFbScreen::shutdownCard()
{
    // Set back the original mode
    if ( qt_sw_cursor )
	qt_screencursor->hide();
}

int QVFbScreen::initCursor(void* e, bool init)
{
    qt_sw_cursor=true;
    // ### until QLumpManager works Ok with multiple connected clients,
    // we steal a chunk of shared memory
    SWCursorData *data = (SWCursorData *)e - 1;
    qt_screencursor=new QVFbScreenCursor();
    qt_screencursor->init( data, init );
    return sizeof(SWCursorData);
}

void QVFbScreen::setMode(int ,int ,int)
{
}

// save the state of the graphics card
// This is needed so that e.g. we can restore the palette when switching
// between linux virtual consoles.
void QVFbScreen::save()
{
    // nothing to do.
}

// restore the state of the graphics card.
void QVFbScreen::restore()
{
}

QGfx * QVFbScreen::createGfx(unsigned char * bytes,int w,int h,int d, int linestep)
{
    QGfx* ret;
    if(d==1) {
	ret = new QGfxRaster<1,0>(bytes,w,h);
#if QT_FEATURE_QWS_DEPTH_16
    } else if(d==16) {
	ret = new QGfxRaster<16,0>(bytes,w,h);
#endif
#if QT_FEATURE_QWS_DEPTH_15
    } else if(d==15) {
	ret = new QGfxRaster<15,0>(bytes,w,h);
#endif
#if QT_FEATURE_QWS_DEPTH_8
    } else if(d==8) {
	ret = new QGfxVFb<8,0>(bytes,w,h);
#endif
#if QT_FEATURE_QWS_DEPTH_8GRAYSCALE
    } else if(d==8) {
	ret = new QGfxVFb<8,0>(bytes,w,h);
#endif
#if QT_FEATURE_QWS_DEPTH_8DIRECT
    } else if(d==8) {
	ret = new QGfxVFb<8,0>(bytes,w,h);
#endif
#if QT_FEATURE_QWS_DEPTH_32
    } else if(d==32) {
	ret = new QGfxVFb<32,0>(bytes,w,h);
#endif
    } else {
	qFatal("Can't drive depth %d",d);
	ret = 0; // silence gcc
    }
    ret->setLineStep(linestep);
    return ret;
}

extern "C" QScreen * qt_get_screen(char *,unsigned char *)
{
    if ( !qt_screen ) {
	qvfb_screen = new QVFbScreen();
	if ( qvfb_screen->connect() ) {
	    qt_screen = qvfb_screen;
	} else {
	    delete qvfb_screen;
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

#endif

