/****************************************************************************
**
** Implementation of QCursor class for FB.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qcursor.h"

#ifndef QT_NO_CURSOR
#include "qbitmap.h"
#include "qimage.h"
#include "qapplication.h"
#include "qdatastream.h"
#include "qwsdisplay_qws.h"

static int nextCursorId = Qt::BitmapCursor + 1;

/*****************************************************************************
  Internal QCursorData class
 *****************************************************************************/

struct QCursorData : public QShared
{
    QCursorData( int s = 0, int i = 0 );
   ~QCursorData();
    int	      cshape;
    int       id;
    QBitmap  *bm, *bmm;
    short     hx, hy;
};

QCursorData::QCursorData( int s, int i )
{
    cshape = s;
    id = i;
    bm = bmm = 0;
    hx = hy  = 0;
}

QCursorData::~QCursorData()
{
    if ( bm )
	delete bm;
    if ( bmm )
	delete bmm;
}


/*****************************************************************************
  Global cursors
 *****************************************************************************/

static QCursor cursorTable[Qt::LastCursor+1];

QCursor *QCursor::find_cur( int shape )		// find predefined cursor
{
    return (uint)shape <= Qt::LastCursor ? &cursorTable[shape] : 0;
}


static bool initialized = FALSE;

void QCursor::cleanup()
{
    int shape;
    for( shape = 0; shape <= LastCursor; shape++ ) {
	delete cursorTable[shape].data;
	cursorTable[shape].data = 0;
    }
    initialized = FALSE;
}


void QCursor::initialize()
{
    int shape;
    for( shape = 0; shape <= LastCursor ; shape++ )
	cursorTable[shape].data = new QCursorData( shape, shape );
    initialized = TRUE;
    qAddPostRoutine( cleanup );
}

Qt::HANDLE QCursor::handle() const
{
    return (Qt::HANDLE)data->id;
}


QCursor::QCursor()
{
    if ( !initialized ) {
	if ( qApp->startingUp() ) {
	    data = 0;
	    return;
	}
	initialize();
    }
    QCursor* c = &cursorTable[0];
    c->data->ref();
    data = c->data;
}



QCursor::QCursor( int shape )
{
    if ( !initialized )
	initialize();
    QCursor *c = find_cur( shape );
    if ( !c )					// not found
	c = &cursorTable[0];	//   then use ArrowCursor
    c->data->ref();
    data = c->data;
}


void QCursor::setBitmap( const QBitmap &bitmap, const QBitmap &mask,
			 int hotX, int hotY )
{
    if ( !initialized )
	initialize();
    if ( bitmap.depth() != 1 || mask.depth() != 1 ||
	 bitmap.size() != mask.size() ) {
	qWarning( "QCursor: Cannot create bitmap cursor; invalid bitmap(s)" );
	QCursor *c = &cursorTable[0];
	c->data->ref();
	data = c->data;
	return;
    }
    data = new QCursorData;
    data->bm  = new QBitmap( bitmap );
    data->bmm = new QBitmap( mask );
    data->cshape = BitmapCursor;
    data->id = nextCursorId++;
    data->hx = hotX >= 0 ? hotX : bitmap.width()/2;
    data->hy = hotY >= 0 ? hotY : bitmap.height()/2;

    QPaintDevice::qwsDisplay()->defineCursor(data->id, *data->bm,
					    *data->bmm, data->hx, data->hy);
}

QCursor::QCursor( const QCursor &c )
{
    if ( !initialized )
	initialize();
    data = c.data;				// shallow copy
    data->ref();
}

QCursor::~QCursor()
{
    if ( data && data->deref() )
	delete data;
}


QCursor &QCursor::operator=( const QCursor &c )
{
    if ( !initialized )
	initialize();
    c.data->ref();				// avoid c = c
    if ( data->deref() )
	delete data;
    data = c.data;
    return *this;
}


int QCursor::shape() const
{
    if ( !initialized )
	initialize();
    return data->cshape;
}

void QCursor::setShape( int shape )
{
    if ( !initialized )
	initialize();
    QCursor *c = find_cur( shape );		// find one of the global ones
    if ( !c )					// not found
	c = &cursorTable[0];	//   then use ArrowCursor
    c->data->ref();
    if ( data->deref() )			// make shallow copy
	delete data;
    data = c->data;
}


const QBitmap *QCursor::bitmap() const
{
    if ( !initialized )
	initialize();
    return data->bm;
}

const QBitmap *QCursor::mask() const
{
    if ( !initialized )
	initialize();
    return data->bmm;
}

QPoint QCursor::hotSpot() const
{
    if ( !initialized )
	initialize();
    return QPoint( data->hx, data->hy );
}

void QCursor::update() const
{
    if ( !initialized )
	initialize();
    register QCursorData *d = data;		// cheat const!

    if ( d->cshape == BitmapCursor ) {
	// XXX
	return;
    }
    if ( d->cshape >= SizeVerCursor && d->cshape < SizeAllCursor ||
	 d->cshape == BlankCursor ) {
	//	int i = (d->cshape - SizeVerCursor)*2;
	// XXX data: cursor_bits16[i], 16,16
	// XXX mask: cursor_bits16[i+1], 16,16
	return;
    }
    if ( d->cshape >= SplitVCursor && d->cshape <= PointingHandCursor ) {
	//int i = (d->cshape - SplitVCursor)*2;
	// XXX data: cursor_bits32[i], 32, 32
	// XXX mask: cursor_bits32[i+1], 32, 32
	//int hs = d->cshape != PointingHandCursor? 16 : 0;
	// XXX ...
	return;
    }

    // XXX standard shapes?
}

#endif //QT_NO_CURSOR



extern int *qt_last_x,*qt_last_y;

QPoint QCursor::pos()
{
    // This doesn't know about hotspots yet so we disable it
    //qt_accel_update_cursor();
    if ( qt_last_x )
	return QPoint( *qt_last_x,*qt_last_y );
    else
	return QPoint();
}

void QCursor::setPos( int x, int y )
{
    // Need to check, since some X servers generate null mouse move
    // events, causing looping in applications which call setPos() on
    // every mouse move event.
    //
    if (pos() == QPoint(x,y))
	return;

    // XXX XWarpPointer( qt_xdisplay(), None, qt_xrootwin(), 0, 0, 0, 0, x, y );
}
