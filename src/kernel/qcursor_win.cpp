/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qcursor_win.cpp#21 $
**
** Implementation of QCursor class for Win32
**
** Created : 940219
**
** Copyright (C) 1994-1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qcursor.h"
#include "qbitmap.h"
#include "qapp.h"
#include "qimage.h"
#include "qdstream.h"

#if defined(_CC_BOOL_DEF_)
#undef	bool
#include <windows.h>
#define bool int
#else
#include <windows.h>
#endif

RCSTAG("$Id: //depot/qt/main/src/kernel/qcursor_win.cpp#21 $");


/*****************************************************************************
  Internal QCursorData class
 *****************************************************************************/

struct QCursorData : public QShared {
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


/*****************************************************************************
  Global cursors
 *****************************************************************************/

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
const QCursor blankCursor;


/*****************************************************************************
  QCursor member functions
 *****************************************************************************/

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
    (QCursor*)&blankCursor,
    0
};

static QCursor *find_cur( int shape )		// find predefined cursor
{
    return (uint)shape <= LastCursor ? cursorTable[shape] : 0;
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
    while ( cursorTable[shape] ) {
	delete cursorTable[shape]->data;
	cursorTable[shape]->data = 0;
	shape++;
    }
}


QCursor::QCursor()
{
    if ( QApplication::startingUp() ) {		// this is a global cursor
	data = new QCursorData;
	CHECK_PTR( data );
	data->cshape = 0;
	data->hcurs = 0;
    } else {					// default arrow cursor
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
{
    if ( bitmap.depth() != 1 || mask.depth() != 1 ||
	 bitmap.size() != mask.size() ) {
#if defined(CHECK_NULL)
	warning( "QCursor: Cannot create bitmap cursor; invalid bitmap(s)" );
#endif
	QCursor *c = (QCursor *)&arrowCursor;
	c->data->ref();
	data = c->data;
	return;
    }
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
	    sh = IDC_SIZENS;
	    break;
	case SizeHorCursor:
	    sh = IDC_SIZEWE;
	    break;
	case SizeBDiagCursor:
	    sh = IDC_SIZENESW;
	    break;
	case SizeFDiagCursor:
	    sh = IDC_SIZENWSE;
	    break;
	case SizeAllCursor:
	    sh = IDC_SIZEALL;
	    break;
	case BlankCursor:
	case BitmapCursor: {
	    QImage bbits, mbits;
	    if ( data->cshape == BlankCursor ) {
		bbits.create( 32, 32, 1, 2, QImage::BigEndian );
		bbits.fill( 1 );		// ignore color table
		mbits = bbits.copy();
		data->hx = data->hy = 16;
	    } else {
		bbits = *data->bm;
		mbits = *data->bmm;
	    }
	    int i, n = bbits.numBytes();
	    uchar *bits = bbits.scanLine( 0 );
	    uchar *mask = mbits.scanLine( 0 );
	    for ( i=0; i<n; i++ ) {
		uchar b = ~bits[i];
		uchar m = ~mask[i];
		bits[i] = ~m;
		mask[i] = b ^ m;
	    }
	    data->hcurs = CreateCursor( qWinAppInst(), data->hx, data->hy,
					bbits.width(), bbits.height(),
					bits, mask );
	    return;
	    }
	default:
#if defined(CHECK_RANGE)
	    warning( "QCursor::update: Invalid cursor shape %d", data->cshape);
#endif
	    return;
    }
    data->hcurs = LoadCursor( 0, sh );
}
