/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qcursor_win.cpp#9 $
**
** Implementation of QCursor class for Windows
**
** Author  : Haavard Nord
** Created : 940219
**
** Copyright (C) 1994,1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qcursor.h"
#include "qbitmap.h"
#include "qapp.h"
#include "qimage.h"
#include "qdstream.h"
#include <windows.h>

RCSTAG("$Id: //depot/qt/main/src/kernel/qcursor_win.cpp#9 $")


// --------------------------------------------------------------------------
// Internal QCursorData class
//

struct QCursorData : QShared {			// internal cursor data
    QCursorData();
   ~QCursorData();
    int	      cshape;
    QBitmap  *bm, *bmm;
    short     hx, hy;
    HCURSOR   hcurs;
};

QCursorData::QCursorData()
{
    bm = bmm = 0;
    hx = hy  = 0;
}

QCursorData::~QCursorData()
{
    if ( bm || bmm ) {
	delete bm;
	delete bmm;
	if ( hcurs )
	    DestroyCursor( hcurs );
    }
}


// --------------------------------------------------------------------------
// Global cursors
//

const QCursor arrowCursor;
const QCursor upArrowCursor;
const QCursor crossCursor;
const QCursor waitCursor;
const QCursor ibeamCursor;
const QCursor sizeVerCursor;
const QCursor sizeHorCursor;
const QCursor sizeBDiagCursor;
const QCursor sizeFDiagCursor;
const QCursor sizeAllCursor;


// --------------------------------------------------------------------------
// QCursor member functions
//

static QCursor *cursorTable[] = {		// the order is important!!
    (QCursor*)&arrowCursor,
    (QCursor*)&upArrowCursor,
    (QCursor*)&crossCursor,
    (QCursor*)&waitCursor,
    (QCursor*)&ibeamCursor,
    (QCursor*)&sizeVerCursor,
    (QCursor*)&sizeHorCursor,
    (QCursor*)&sizeBDiagCursor,
    (QCursor*)&sizeFDiagCursor,
    (QCursor*)&sizeAllCursor,
    0
};

static QCursor *find_cur( int shape )		// find predefined cursor
{
    return (uint)shape <= SizeAllCursor ? cursorTable[shape] : 0;
}


void QCursor::initialize()
{
    int shape = ArrowCursor;
    while ( cursorTable[shape] ) {
	cursorTable[shape]->data->cshape = shape;
	shape++;
    }
}

void QCursor::cleanup()
{
    int shape = ArrowCursor;
#if defined(CHECK_MEMORY)
    bool mc = memchkSetReporting( FALSE );	// get rid of stupid messages
#endif
    while ( cursorTable[shape] ) {
	delete cursorTable[shape]->data;
	cursorTable[shape]->data = 0;
	shape++;
    }
#if defined(CHECK_MEMORY)
    memchkSetReporting( mc );
#endif
}


QCursor::QCursor()
{
    if ( QApplication::startingUp() ) {		// this is a global cursor
	data = new QCursorData;
	CHECK_PTR( data );
	data->cshape = 0;
	data->hcurs = 0;
    }
    else {					// default arrow cursor
	data = arrowCursor.data;
	data->ref();
    }
}

QCursor::QCursor( int shape )			// cursor with shape
{
    QCursor *c = find_cur( shape );
    if ( !c )					// not found
	c = (QCursor *)&arrowCursor;		//   then use arrowCursor
    c->data->ref();
    data = c->data;
}

QCursor::QCursor( const QBitmap &bitmap, const QBitmap &mask,
		  int hotX, int hotY )
{						// define own cursor
    data = new QCursorData;
    CHECK_PTR( data );
    data->bm  = new QBitmap( bitmap );
    data->bmm = new QBitmap( mask );
    data->hcurs = 0;
    data->cshape = BitmapCursor;
    data->hx = hotX >= 0 ? hotX : bitmap.width()/2;
    data->hy = hotY >= 0 ? hotY : bitmap.height()/2;
}

QCursor::QCursor( const QCursor &c )
{
    data = c.data;
    data->ref();
}

QCursor::~QCursor()
{
    if ( data && data->deref() )
	delete data;
}


QCursor &QCursor::operator=( const QCursor &c )
{
    c.data->ref();				// avoid c = c
    if ( data->deref() )
	delete data;
    data = c.data;
    return *this;
}


int QCursor::shape() const			// get cursor shape
{
    return data->cshape;
}

void QCursor::setShape( int shape )		// set cursor shape
{
    QCursor *c = find_cur( shape );		// find one of the global ones
    if ( !c )					// not found
	c = (QCursor *)&arrowCursor;		//   then use arrowCursor
    c->data->ref();
    if ( data->deref() )			// make shallow copy
	delete data;
    data = c->data;
}


const QBitmap *QCursor::bitmap() const
{
    return data->bm;
}

const QBitmap *QCursor::mask() const
{
    return data->bmm;
}

QPoint QCursor::hotSpot() const
{
    return QPoint( data->hx, data->hy );
}


HANDLE QCursor::handle() const
{
    if ( !data->hcurs )
	update();
    return data->hcurs;
}


QPoint QCursor::pos()				// get cursor position
{
    POINT p;
    GetCursorPos( &p );
    return QPoint( p.x, p.y );
}

void QCursor::setPos( int x, int y )		// set cursor position
{
    SetCursorPos( x, y );
}


void QCursor::update() const			// update/load cursor
{
    if ( data->hcurs )				// already loaded
	return;

    char const *sh;
    switch ( data->cshape ) {			// map to windows cursor
	case ArrowCursor:
	    sh = IDC_ARROW;
	    break;
	case UpArrowCursor:
	    sh = IDC_UPARROW;
	    break;
	case CrossCursor:
	    sh = IDC_CROSS;
	    break;
	case WaitCursor:
	    sh = IDC_WAIT;
	    break;
	case IbeamCursor:
	    sh = IDC_IBEAM;
	    break;
	case SizeVerCursor:
	    sh = IDC_SIZEWE;
	    break;
	case SizeHorCursor:
	    sh = IDC_SIZENS;
	    break;
	case SizeBDiagCursor:
	    sh = IDC_SIZENESW;
	    break;
	case SizeFDiagCursor:
	    sh = IDC_SIZENWSE;
	    break;
	case SizeAllCursor:
	    sh = IDC_SIZE;
	    break;
	case BitmapCursor: {
	    int w = data->bm->width();
	    int h = data->bm->height();
	    int len = (w+7)/8*h;
	    uchar *bits = new uchar[len];
	    uchar *mask = new uchar[len];
	    GetBitmapBits( data->bm->hbm(),  len, bits );
	    GetBitmapBits( data->bmm->hbm(), len, mask );
	    int i;
	    for ( i=0; i<len; i++ ) {
		uchar b = ~bits[i];
		uchar m = ~mask[i];
		bits[i] = ~m;
		mask[i] = b ^ m;
	    }
	    data->hcurs = CreateCursor( qWinAppInst(), data->hx, data->hy,
					w, h, bits, mask );
	    delete [] bits;
	    delete [] mask;
/*
	    QImage c, m;
	    c = *data->bm;
	    m = *data->bmm;
	    c = c.convertBitOrder( QImage::BigEndian );
	    m = m.convertBitOrder( QImage::BigEndian );
	    data->hcurs = CreateCursor( qWinAppInst(), data->hx, data->hy,
					c.width(), c.height(),
					c.bits(),  m.bits() );
*/
	    return;
	    }
	default:
#if defined(CHECK_RANGE)
	    warning( "QCursor::update: Invalid cursor shape %d", data->cshape );
#endif
	    return;
    }
    data->hcurs = LoadCursor( 0, sh );
}
