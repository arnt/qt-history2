/*****************************************************************************
** $Id$
**
** Implementation of QGfxRaster (unaccelerated graphics context) class for
** Embedded Qt
** Created : 940721
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

#include "qgfxraster_qws.h"

#ifndef QT_NO_QWS_TRANSFORMED

#include "qmemorymanager_qws.h"
#include "qwsdisplay_qws.h"

#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>


#ifndef Q_OS_QNX6_
#define QT_TRANS_SCREEN_BASE	QLinuxFbScreen
//#define QT_TRANS_SCREEN_BASE	QVFbScreen
#include "qgfxlinuxfb_qws.h"
#else
#define QT_TRANS_SCREEN_BASE	QQnxScreen
#include "qwsgfx_qnx.h"
#endif

#define QT_TRANS_CURSOR_BASE	QScreenCursor
#define QT_TRANS_GFX_BASE	QGfxRaster
//#define QT_TRANS_CURSOR_BASE	QVFbScreenCursor
//#define QT_TRANS_GFX_BASE	QGfxVFb


class QTransformedScreen : public QT_TRANS_SCREEN_BASE
{
public:
    QTransformedScreen( int display_id );
    virtual ~QTransformedScreen();

    virtual bool connect( const QString &displaySpec );
    virtual int initCursor(void* e, bool init);
    virtual QGfx * createGfx(unsigned char *,int,int,int,int);

    enum Transformation { None, Rot90, Rot180, Rot270 };
    Transformation transformation() const { return trans; }

    virtual bool isTransformed() const;
    virtual QSize mapToDevice( const QSize & ) const;
    virtual QSize mapFromDevice( const QSize & ) const;
    virtual QPoint mapToDevice( const QPoint &, const QSize & ) const;
    virtual QPoint mapFromDevice( const QPoint &, const QSize & ) const;
    virtual QRect mapToDevice( const QRect &, const QSize & ) const;
    virtual QRect mapFromDevice( const QRect &, const QSize & ) const;
    virtual QImage mapToDevice( const QImage & ) const;
    virtual QImage mapFromDevice( const QImage & ) const;
    virtual QRegion mapToDevice( const QRegion &, const QSize & ) const;
    virtual QRegion mapFromDevice( const QRegion &, const QSize & ) const;
    virtual int transformOrientation() const;

private:
    Transformation trans;
    QScreen *driver;
};

static QTransformedScreen *qt_trans_screen;

// Unaccelerated screen/driver setup. Can be overridden by accelerated
// drivers

QTransformedScreen::QTransformedScreen( int display_id )
    : QT_TRANS_SCREEN_BASE( display_id )
{
    qt_trans_screen = this;
    trans = None;
}

QTransformedScreen::~QTransformedScreen()
{
}

bool QTransformedScreen::connect( const QString &displaySpec )
{
    if ( displaySpec.find( ":Rot270" ) >= 0 )
	trans = Rot270;
    else if ( displaySpec.find( ":Rot180" ) >= 0 )
	trans = Rot180;
    else if ( displaySpec.find( ":Rot90" ) >= 0 )
	trans = Rot90;

    bool result = QT_TRANS_SCREEN_BASE::connect( displaySpec );
    if ( result ) {
	QSize s = mapFromDevice( QSize(w, h) );
	w = s.width();
	h = s.height();
    }

    return result;
}

bool QTransformedScreen::isTransformed() const
{
    return trans != None;
}

QSize QTransformedScreen::mapToDevice( const QSize &s ) const
{
    if ( trans == Rot90 || trans == Rot270 ) {
	return QSize( s.height(), s.width() );
    }

    return s;
}

QSize QTransformedScreen::mapFromDevice( const QSize &s ) const
{
    if ( trans == Rot90 || trans == Rot270 ) {
	return QSize( s.height(), s.width() );
    }

    return s;
}

QPoint QTransformedScreen::mapToDevice( const QPoint &p, const QSize &s ) const
{
    QPoint rp( p );

    switch ( trans ) {
	case Rot90:
	    rp.setX( p.y() );
	    rp.setY( s.width() - p.x() - 1 );
	    break;
	case Rot180:
	    rp.setX( s.width() - p.x() - 1 );
	    rp.setY( s.height() - p.y() - 1 );
	    break;
	case Rot270:
	    rp.setX( s.height() - p.y() - 1 );
	    rp.setY( p.x() );
	    break;
	default:
	    break;
    }

    return rp;
}

QPoint QTransformedScreen::mapFromDevice( const QPoint &p, const QSize &s ) const
{
    QPoint rp( p );

    switch ( trans ) {
	case Rot90:
	    rp.setX( s.height() - p.y() - 1 );
	    rp.setY( p.x() );
	    break;
	case Rot180:
	    rp.setX( s.width() - p.x() - 1 );
	    rp.setY( s.height() - p.y() - 1 );
	    break;
	case Rot270:
	    rp.setX( p.y() );
	    rp.setY( s.width() - p.x() - 1 );
	    break;
	default:
	    break;
    }

    return rp;
}

QRect QTransformedScreen::mapToDevice( const QRect &r, const QSize &s ) const
{
    QRect tr;
    switch ( trans ) {
	case Rot90:
	    tr.setCoords( r.y(), s.width() - r.x() - 1,
			  r.bottom(), s.width() - r.right() - 1 );
	    break;
	case Rot180:
	    tr.setCoords( s.width() - r.x() - 1, s.height() - r.y() - 1,
			  s.width() - r.right() - 1, s.height() - r.bottom() - 1 );
	    break;
	case Rot270:
	    tr.setCoords( s.height() - r.y() - 1, r.x(),
			  s.height() - r.bottom() - 1, r.right() );
	    break;
	default:
	    tr = r;
	    break;
    }

    return tr.normalize();
}

QRect QTransformedScreen::mapFromDevice( const QRect &r, const QSize &s ) const
{
    QRect tr;
    switch ( trans ) {
	case Rot90:
	    tr.setCoords( s.height() - r.y() - 1, r.x(),
			  s.height() - r.bottom() - 1, r.right() );
	    break;
	case Rot180:
	    tr.setCoords( s.width() - r.x() - 1, s.height() - r.y() - 1,
			  s.width() - r.right() - 1, s.height() - r.bottom() - 1 );
	    break;
	case Rot270:
	    tr.setCoords( r.y(), s.width() - r.x() - 1,
			  r.bottom(), s.width() - r.right() - 1 );
	    break;
	default:
	    tr = r;
	    break;
    }

    return tr.normalize();
}


QImage QTransformedScreen::mapToDevice( const QImage &img ) const
{
    if ( img.isNull() || trans == None )
	return img;

    int iw = img.width();
    int ih = img.height();
    int w = iw;
    int h = ih;
    if ( trans == Rot90 || trans == Rot270 ) {
	w = ih;
	h = iw;
    }

    QImage rimg( w, h, img.depth(), img.numColors(), img.bitOrder() );

    for ( int i = 0; i < img.numColors(); i++ ) {
	rimg.colorTable()[i] = img.colorTable()[i];
    }

    #define START_LOOP for ( int y = 0; y < ih; y++ ) { \
			   for ( int x = 0; x < iw; x++ ) {
    #define END_LOOP } }

    int pixel;
    if ( img.depth() > 8 ) {
	switch ( trans ) {
	    case Rot90:
		START_LOOP
		    pixel = img.pixel(x, y);
		    rimg.setPixel( y, iw - x - 1, pixel );
		END_LOOP
		break;

	    case Rot270:
		START_LOOP
		    pixel = img.pixel(x, y);
		    rimg.setPixel( ih - y - 1, x, pixel );
		END_LOOP
		break;

	    default:
		START_LOOP
		    pixel = img.pixel(x, y);
		    rimg.setPixel( iw - x - 1, ih - y - 1, pixel );
		END_LOOP
	}
    } else {
	switch ( trans ) {
	    case Rot90:
		START_LOOP
		    pixel = img.pixelIndex(x, y);
		    rimg.setPixel( y, iw - x - 1, pixel );
		END_LOOP
		break;

	    case Rot270:
		START_LOOP
		    pixel = img.pixelIndex(x, y);
		    rimg.setPixel( ih - y - 1, x, pixel );
		END_LOOP
		break;

	    default:
		START_LOOP
		    pixel = img.pixelIndex(x, y);
		    rimg.setPixel( iw - x - 1, ih - y - 1, pixel );
		END_LOOP
	}
    }

    #undef START_LOOP
    #undef END_LOOP

/*
    for ( int y = 0; y < img.height(); y++ ) {
	for ( int x = 0; x < img.width(); x++ ) {
	    int pixel;
	    if ( img.depth() > 8 )
		pixel = img.pixel(x, y);
	    else
		pixel = img.pixelIndex(x, y);
	    switch ( trans ) {
		case Rot90:
		    rimg.setPixel( y, img.width() - x - 1, pixel );
		    break;

		case Rot270:
		    rimg.setPixel( img.height() - y - 1, x, pixel );
		    break;

		default:
		    rimg.setPixel( img.width() - x - 1, img.height() - y - 1, pixel );
	    }
	}
    }
*/
    rimg.setAlphaBuffer( img.hasAlphaBuffer() );
    rimg.setOffset( img.offset() );

    return rimg;
}

QImage QTransformedScreen::mapFromDevice( const QImage &img ) const
{
    if ( img.isNull() || trans == None )
	return img;

    int iw = img.width();
    int ih = img.height();
    int w = iw;
    int h = ih;
    if ( trans == Rot90 || trans == Rot270 ) {
	w = img.height();
	h = img.width();
    }

    QImage rimg( w, h, img.depth(), img.numColors(), img.bitOrder() );

    for ( int i = 0; i < img.numColors(); i++ ) {
	rimg.colorTable()[i] = img.colorTable()[i];
    }

    #define START_LOOP for ( int y = 0; y < ih; y++ ) { \
			   for ( int x = 0; x < iw; x++ ) {
    #define END_LOOP } }

    int pixel;
    if ( img.depth() > 8 ) {
	switch ( trans ) {
	    case Rot90:
		START_LOOP
		    pixel = img.pixel(x, y);
		    rimg.setPixel( ih - y - 1, x, pixel );
		END_LOOP
		break;
	    case Rot270:
		START_LOOP
		    pixel = img.pixel(x, y);
		    rimg.setPixel( y, iw - x - 1, pixel );
		END_LOOP
		break;
	    default:
		START_LOOP
		    pixel = img.pixel(x, y);
		    rimg.setPixel( iw - x - 1, ih - y - 1, pixel );
		END_LOOP
	}
    } else {
	switch ( trans ) {
	    case Rot90:
		START_LOOP
		    pixel = img.pixelIndex(x, y);
		    rimg.setPixel( ih - y - 1, x, pixel );
		END_LOOP
		break;
	    case Rot270:
		START_LOOP
		    pixel = img.pixelIndex(x, y);
		    rimg.setPixel( y, iw - x - 1, pixel );
		END_LOOP
		break;
	    default:
		START_LOOP
		    pixel = img.pixelIndex(x, y);
		    rimg.setPixel( iw - x - 1, ih - y - 1, pixel );
		END_LOOP
	}
    }

    #undef START_LOOP
    #undef END_LOOP

/*
    for ( int y = 0; y < img.height(); y++ ) {
	for ( int x = 0; x < img.width(); x++ ) {
	    int pixel;
	    if ( img.depth() > 8 )
		pixel = img.pixel(x, y);
	    else
		pixel = img.pixelIndex(x, y);
	    switch ( trans ) {
		case Rot90:
		    rimg.setPixel( img.height() - y - 1, x, pixel );
		    break;

		case Rot270:
		    rimg.setPixel( y, img.width() - x - 1, pixel );
		    break;

		default:
		    rimg.setPixel( img.width() - x - 1, img.height() - y - 1, pixel );
	    }
	}
    }
*/
    rimg.setAlphaBuffer( img.hasAlphaBuffer() );
    rimg.setOffset( img.offset() );

    return rimg;
}

QRegion QTransformedScreen::mapToDevice( const QRegion &rgn, const QSize &s ) const
{
    if ( trans == None )
	return rgn;

    QRegion trgn;
    QMemArray<QRect> a = rgn.rects();
    QRect tr;
    const QRect *r = a.data();

    int w = s.width();
    int h = s.height();
    int size = a.size();

    switch ( trans ) {
	case Rot270:
	    for ( int i = 0; i < size; i++, r++ ) {
		tr.setCoords( h - r->y() - 1, r->x(),
			      h - r->bottom() - 1, r->right() );
		trgn |= tr.normalize();
	    }
	    break;
	case Rot90:
	    for ( int i = 0; i < size; i++, r++ ) {
		tr.setCoords( r->y(), w - r->x() - 1,
			      r->bottom(), w - r->right() - 1 );
		trgn |= tr.normalize();
	    }
	    break;
	case Rot180:
	    for ( int i = 0; i < size; i++, r++ ) {
		tr.setCoords( w - r->x() - 1, h - r->y() - 1,
			      w - r->right() - 1, h - r->bottom() - 1 );
		trgn |= tr.normalize();
	    }
	    break;
	default:
	    break;
    }
/*
    for ( int i = 0; i < (int)a.size(); i++ ) {
	const QRect &r = a[i];
	QRect tr;
	switch ( trans ) {
	    case Rot90:
		tr.setCoords( r.y(), s.width() - r.x() - 1,
			      r.bottom(), s.width() - r.right() - 1 );
		break;
	    case Rot180:
		tr.setCoords( s.width() - r.x() - 1, s.height() - r.y() - 1,
			      s.width() - r.right() - 1, s.height() - r.bottom() - 1 );
		break;
	    case Rot270:
		tr.setCoords( s.height() - r.y() - 1, r.x(),
			      s.height() - r.bottom() - 1, r.right() );
		break;
	    default:
		break;
	}
	trgn |= tr.normalize();
    }
*/
    return trgn;
}

QRegion QTransformedScreen::mapFromDevice( const QRegion &rgn, const QSize &s ) const
{
    if ( trans == None )
	return rgn;

    QRegion trgn;
    QMemArray<QRect> a = rgn.rects();
    const QRect *r = a.data();
    QRect tr;

    int w = s.width();
    int h = s.height();
    int size = a.size();

    switch ( trans ) {
	case Rot270:
	    for ( int i = 0; i < size; i++, r++ ) {
		tr.setCoords( r->y(), w - r->x() - 1,
			      r->bottom(), w - r->right() - 1 );
		trgn |= tr.normalize();
	    }
	    break;
	case Rot90:
	    for ( int i = 0; i < size; i++, r++ ) {
		tr.setCoords( h - r->y() - 1, r->x(),
			      h - r->bottom() - 1, r->right() );
		trgn |= tr.normalize();
	    }
	    break;
	case Rot180:
	    for ( int i = 0; i < size; i++, r++ ) {
		tr.setCoords( w - r->x() - 1, h - r->y() - 1,
			      w - r->right() - 1, h - r->bottom() - 1 );
		trgn |= tr.normalize();
	    }
	    break;
	default:
	    break;
    }

/*
    QRegion trgn;
    QArray<QRect> a = rgn.rects();
    const QRect *r = a.data();
    QRect tr;

    int w = s.width();
    int h = s.height();
    int size = a.size();

    switch ( trans ) {
	case Rot270:
	    for ( int i = 0; i < size; i++, r++ ) {
		tr.setCoords( r->y(), w - r->x() - 1,
			      r->bottom(), w - r->right() - 1 );
		trgn |= tr.normalize();
	    }
	    break;
	case Rot90:
	    for ( int i = 0; i < size; i++, r++ ) {
		tr.setCoords( h - r->y() - 1, r->x(),
			      h - r->bottom() - 1, r->right() );
		trgn |= tr.normalize();
	    }
	    break;
	case Rot180:
	    for ( int i = 0; i < size; i++, r++ ) {
		tr.setCoords( w - r->x() - 1, h - r->y() - 1,
			      w - r->right() - 1, h - r->bottom() - 1 );
		trgn |= tr.normalize();
	    }
	    break;
	default:
	    break;
    }

/*
    QRegion trgn;
    QArray<QRect> a = rgn.rects();
    for ( int i = 0; i < (int)a.size(); i++ ) {
	const QRect &r = a[i];
	QRect tr;
	switch ( trans ) {
	    case Rot90:
		tr.setCoords( s.height() - r.y() - 1, r.x(),
			      s.height() - r.bottom() - 1, r.right() );
		break;
	    case Rot180:
		tr.setCoords( s.width() - r.x() - 1, s.height() - r.y() - 1,
			      s.width() - r.right() - 1, s.height() - r.bottom() - 1 );
		break;
	    case Rot270:
		tr.setCoords( r.y(), s.width() - r.x() - 1,
			      r.bottom(), s.width() - r.right() - 1 );
		break;
	    default:
		break;
	}
	trgn |= tr.normalize();
    }
*/
    return trgn;
}

/*!
  0 = none
  1..3 = rotates 90..270
  4..7 = mirrored 0..3
*/
int QTransformedScreen::transformOrientation() const
{
    return (int)trans;
}

#ifndef QT_NO_QWS_CURSOR

class QTransformedScreenCursor : public QT_TRANS_CURSOR_BASE
{
public:
    QTransformedScreenCursor() : QT_TRANS_CURSOR_BASE() {}
    virtual void init(SWCursorData *da, bool init = FALSE);
    virtual void set( const QImage &image, int hotx, int hoty );
};

void QTransformedScreenCursor::init( SWCursorData *da, bool init )
{
    QT_TRANS_CURSOR_BASE::init( da, init );
    QSize s = qt_trans_screen->mapFromDevice( QSize(clipWidth, clipHeight) );
    clipWidth = s.width();
    clipHeight = s.height();
}

void QTransformedScreenCursor::set( const QImage &image, int hotx, int hoty )
{
    QImage rimg = qt_trans_screen->mapToDevice( image );
    QPoint tp = qt_trans_screen->mapToDevice( QPoint( hotx, hoty ), image.size() );
    QT_TRANS_CURSOR_BASE::set( rimg, tp.x(), tp.y() );
}

#endif

template <const int depth, const int type>
class QGfxTransformedRaster : public QT_TRANS_GFX_BASE<depth,type>
{
public:
    QGfxTransformedRaster(unsigned char *,int w,int h);
    virtual ~QGfxTransformedRaster();

    virtual void setSource(const QImage * i);
    virtual void drawPoint( int,int );
    virtual void drawPoints( const QPointArray &,int,int );
    virtual void drawLine( int,int,int,int );
    virtual void fillRect( int,int,int,int );
    virtual void drawPolygon( const QPointArray &,bool,int,int );
    virtual void drawPolyline( const QPointArray &,int,int );
    virtual void blt( int,int,int,int,int,int );
#if !defined(QT_NO_MOVIE) || !defined(QT_NO_TRANSFORMATIONS)
    virtual void stretchBlt( int rx,int ry,int w,int h, int sw,int sh );
#endif
    
    inline int tx( int x, int y ) {
	switch ( qt_trans_screen->transformation() ) {
	    case QTransformedScreen::Rot90:
		return y - xoffs + yoffs;
	    case QTransformedScreen::Rot180:
		return (width - x - 1) - xoffs - xoffs;
	    case QTransformedScreen::Rot270:
		return (height - y - 1) - xoffs - yoffs;
	    default:
		return x;
	}
    }
    inline int ty( int x, int y ) {
	switch ( qt_trans_screen->transformation() ) {
	    case QTransformedScreen::Rot90:
		return (width - x - 1) - yoffs - xoffs;
	    case QTransformedScreen::Rot180:
		return (height - y - 1) - yoffs - yoffs;
	    case QTransformedScreen::Rot270:
		return x - yoffs + xoffs;
	    default:
		return y;
	}
    }

protected:
    virtual void setSourceWidgetOffset( int x, int y );
    bool inDraw;
};

template <const int depth, const int type>
QGfxTransformedRaster<depth,type>::QGfxTransformedRaster(unsigned char *b,int w,int h)
: QT_TRANS_GFX_BASE<depth,type>( b, w, h ), inDraw( FALSE )
{
}

template <const int depth, const int type>
QGfxTransformedRaster<depth,type>::~QGfxTransformedRaster()
{
}

template <const int depth, const int type>
void QGfxTransformedRaster<depth,type>::setSourceWidgetOffset(int x, int y)
{
    if ( srcbits == buffer ) {
	switch ( qt_trans_screen->transformation() ) {
	    case QTransformedScreen::Rot90:
		srcwidgetoffs = QPoint( y, width - x - srcwidth );
		break;
	    case QTransformedScreen::Rot180:
		srcwidgetoffs = QPoint( width - x - srcwidth, height - y - srcheight );
		break;
	    case QTransformedScreen::Rot270:
		srcwidgetoffs = QPoint( height - y - srcheight, x );
		break;
	    default:
		srcwidgetoffs = QPoint( x, y );
		break;
	}
    } else
	srcwidgetoffs = QPoint( x, y );
}

template <const int depth, const int type>
void QGfxTransformedRaster<depth,type>::setSource(const QImage * i)
{
    QT_TRANS_GFX_BASE<depth,type>::setSource(i);
    QSize s = qt_screen->mapToDevice( QSize(i->width(), i->height()) );
    srcwidth = s.width();
    srcheight = s.height();
}

template <const int depth, const int type>
void QGfxTransformedRaster<depth,type>::drawPoint( int x, int y )
{
    QT_TRANS_GFX_BASE<depth,type>::drawPoint( tx(x,y), ty(x,y) );
}

template <const int depth, const int type>
void QGfxTransformedRaster<depth,type>::drawPoints( const QPointArray &a, int idx, int num )
{
    QPointArray na( num );

    for ( int i = 0; i < num; i++ ) {
	int x, y;
	a.point( i+idx, &x, &y );
	na.setPoint( i, tx(x,y), ty(x,y) );
    }

    QT_TRANS_GFX_BASE<depth,type>::drawPoints( na, 0, num );
}

template <const int depth, const int type>
void QGfxTransformedRaster<depth,type>::drawLine( int x1, int y1, int x2, int y2 )
{
    if ( inDraw ) {
	QT_TRANS_GFX_BASE<depth,type>::drawLine( x1, y1, x2, y2 );
    } else {
	inDraw = TRUE;
	QT_TRANS_GFX_BASE<depth,type>::drawLine( tx(x1,y1), ty(x1,y1),
					  tx(x2,y2), ty(x2,y2) );
	inDraw = FALSE;
    }
}

template <const int depth, const int type>
void QGfxTransformedRaster<depth,type>::fillRect( int x, int y, int w, int h )
{
    if ( w == 0 || h == 0 )
	return;
    QRect r( x, y, w, h );
    if ( cbrush.style() == SolidPattern ) {
	r.setCoords( tx(x,y), ty(x,y), tx(x+w-1,y+h-1), ty(x+w-1,y+h-1) );
	r = r.normalize();
    }
    QT_TRANS_GFX_BASE<depth,type>::fillRect( r.x(), r.y(), r.width(), r.height() );
}

template <const int depth, const int type>
void QGfxTransformedRaster<depth,type>::drawPolygon( const QPointArray &a, bool w, int idx, int num )
{
    switch ( qt_trans_screen->transformation() ) {
	case QTransformedScreen::Rot90:
	    stitchedges=QPolygonScanner::Edge(QPolygonScanner::Bottom+QPolygonScanner::Left);
	    break;
	case QTransformedScreen::Rot180:
	    stitchedges=QPolygonScanner::Edge(QPolygonScanner::Bottom+QPolygonScanner::Right);
	    break;
	case QTransformedScreen::Rot270:
	    stitchedges=QPolygonScanner::Edge(QPolygonScanner::Top+QPolygonScanner::Right);
	    break;
	default:
	    stitchedges=QPolygonScanner::Edge(QPolygonScanner::Left+QPolygonScanner::Top);
	    break;
    }
    if ( inDraw ) {
	QT_TRANS_GFX_BASE<depth,type>::drawPolygon( a, w, idx, num );
    } else {
	inDraw = TRUE;
	QPointArray na( num );

	for ( int i = 0; i < num; i++ ) {
	    int x, y;
	    a.point( i+idx, &x, &y );
	    na.setPoint( i, tx(x,y), ty(x,y) );
	}

	QT_TRANS_GFX_BASE<depth,type>::drawPolygon( na, w, 0, num );
	inDraw = FALSE;
    }
    stitchedges=QPolygonScanner::Edge(QPolygonScanner::Left+QPolygonScanner::Top);
}

template <const int depth, const int type>
void QGfxTransformedRaster<depth,type>::drawPolyline( const QPointArray &a, int idx, int num )
{
    if ( inDraw ) {
	QT_TRANS_GFX_BASE<depth,type>::drawPolyline( a, idx, num );
    } else {
	inDraw = TRUE;
	QPointArray na( num );

	for ( int i = 0; i < num; i++ ) {
	    int x, y;
	    a.point( i+idx, &x, &y );
	    na.setPoint( i, tx(x,y), ty(x,y) );
	}

	QT_TRANS_GFX_BASE<depth,type>::drawPolyline( na, 0, num );
	inDraw = FALSE;
    }
}

template <const int depth, const int type>
void QGfxTransformedRaster<depth,type>::blt( int x, int y, int w, int h, int sx, int sy )
{
    if ( w == 0 || h == 0 )
	return;
    QRect r;
    r.setCoords( tx(x,y), ty(x,y), tx(x+w-1,y+h-1), ty(x+w-1,y+h-1) );
    r = r.normalize();
    int rsx;
    int rsy;
    switch ( qt_trans_screen->transformation() ) {
	case QTransformedScreen::Rot90:
	    rsx = sy;
	    rsy = srcwidth - sx - w;
	    break;
	case QTransformedScreen::Rot180:
	    rsx = srcwidth - sx - w;
	    rsy = srcheight - sy - h;
	    break;
	case QTransformedScreen::Rot270:
	    rsx = srcheight - sy - h;
	    rsy = sx;
	    break;
	default:
	    rsx = sx;
	    rsy = sy;
	    break;
    }
    QT_TRANS_GFX_BASE<depth,type>::blt( r.x(), r.y(), r.width(), r.height(), rsx, rsy );
}

#if !defined(QT_NO_MOVIE) || !defined(QT_NO_TRANSFORMATIONS)
template <const int depth, const int type>
void QGfxTransformedRaster<depth,type>::stretchBlt( int x, int y, int w, int h,
						    int sw, int sh )
{
    if ( w == 0 || h == 0 )
	return;
    QRect r;
    r.setCoords( tx(x,y), ty(x,y), tx(x+w-1,y+h-1), ty(x+w-1,y+h-1) );
    r = r.normalize();
    QSize rs = qt_trans_screen->mapToDevice( QSize( sw, sh ) );
    QT_TRANS_GFX_BASE<depth,type>::stretchBlt( r.x(), r.y(), r.width(), r.height(),
					rs.width(), rs.height() );
}
#endif

int QTransformedScreen::initCursor(void* e, bool init)
{
#ifndef QT_NO_QWS_CURSOR
    qt_sw_cursor=true;
    // ### until QLumpManager works Ok with multiple connected clients,
    // we steal a chunk of shared memory
    SWCursorData *data = (SWCursorData *)e - 1;
    qt_screencursor=new QTransformedScreenCursor();
    qt_screencursor->init( data, init );
    return sizeof(SWCursorData);
#else
    return 0;
#endif
}

QGfx *QTransformedScreen::createGfx(unsigned char * bytes,int w,int h,int d, int linestep )
{
    QGfx* ret = 0;
    if(d==1) {
	ret = new QGfxTransformedRaster<1,0>(bytes,w,h);
#ifndef QT_NO_QWS_DEPTH_16
    } else if(d==16) {
	ret = new QGfxTransformedRaster<16,0>(bytes,w,h);
#endif
#ifndef QT_NO_QWS_DEPTH_8
    } else if (d==8) {
	ret = new QGfxTransformedRaster<8,0>(bytes,w,h);
#endif
#ifndef QT_NO_QWS_DEPTH_4
    } else if (d==4) {
	ret = new QGfxTransformedRaster<4,0>(bytes,w,h);
#endif
#ifndef QT_NO_QWS_DEPTH_8GRAYSCALE
    } else if (d==8) {
	ret = new QGfxTransformedRaster<8,0>(bytes,w,h);
#endif
#ifndef QT_NO_QWS_DEPTH_24
    } else if (d==24) {
	ret = new QGfxTransformedRaster<24,0>(bytes,w,h);
#endif
#ifndef QT_NO_QWS_DEPTH_32
    } else if (d==32) {
	ret = new QGfxTransformedRaster<32,0>(bytes,w,h);
#endif
    } else {
	qFatal("Can't drive depth %d",d);
    }
    ret->setLineStep(linestep);
    return ret;
}


extern "C" QScreen * qt_get_screen_transformed(int display_id)
{
    return new QTransformedScreen( display_id );
}

#endif // QT_NO_QWS_TRANSFORMED
