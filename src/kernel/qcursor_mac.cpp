/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qcursor_mac.cpp
**
** Implementation of QCursor class for mac
**
** Created : 001019
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Unix/X11/FIXME may use this file in accordance with the Qt Commercial
** License Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qcursor.h"
#include "qbitmap.h"
#include "qimage.h"
#include "qapplication.h"
#include "qdatastream.h"
#include "qnamespace.h"
#include "qt_mac.h"
#include <stdlib.h>

// NOT REVISED


/*****************************************************************************
  Global cursors
 *****************************************************************************/

static const int cursors = 15;
static QCursor cursorTable[cursors];
static const int arrowCursorIdx = 0;
QT_STATIC_CONST_IMPL QCursor & Qt::arrowCursor = cursorTable[0];
QT_STATIC_CONST_IMPL QCursor & Qt::upArrowCursor = cursorTable[1];
QT_STATIC_CONST_IMPL QCursor & Qt::crossCursor = cursorTable[2];
QT_STATIC_CONST_IMPL QCursor & Qt::waitCursor = cursorTable[3];
QT_STATIC_CONST_IMPL QCursor & Qt::ibeamCursor = cursorTable[4];
QT_STATIC_CONST_IMPL QCursor & Qt::sizeVerCursor = cursorTable[5];
QT_STATIC_CONST_IMPL QCursor & Qt::sizeHorCursor = cursorTable[6];
QT_STATIC_CONST_IMPL QCursor & Qt::sizeBDiagCursor = cursorTable[7];
QT_STATIC_CONST_IMPL QCursor & Qt::sizeFDiagCursor = cursorTable[8];
QT_STATIC_CONST_IMPL QCursor & Qt::sizeAllCursor = cursorTable[9];
QT_STATIC_CONST_IMPL QCursor & Qt::blankCursor = cursorTable[10];
QT_STATIC_CONST_IMPL QCursor & Qt::splitHCursor = cursorTable[11];
QT_STATIC_CONST_IMPL QCursor & Qt::splitVCursor = cursorTable[12];
QT_STATIC_CONST_IMPL QCursor & Qt::pointingHandCursor = cursorTable[13];
QT_STATIC_CONST_IMPL QCursor & Qt::forbiddenCursor = cursorTable[14];

/*****************************************************************************
  Internal QCursorData class
 *****************************************************************************/

struct QCursorData : public QShared
{
    QCursorData( int s = 0 );
   ~QCursorData();
    int	      cshape;
    int hx, hy;
    QBitmap  *bm, *bmm;
    bool my_cursor;
    CursPtr   hcurs;
#if 0
    XColor    fg,bg;
    Pixmap    pm, pmm;
#endif
};

QCursorData::QCursorData( int s )
{
    cshape = s;
    bm = bmm = 0;
    hx = hy  = -1;
    hcurs = NULL;
    my_cursor = FALSE;

#if 0

    pm = pmm = 0;
#endif
}

QCursorData::~QCursorData()
{
#if 0
    Display *dpy = qt_xdisplay();
    if ( pm )
	XFreePixmap( dpy, pm );
    if ( pmm )
	XFreePixmap( dpy, pmm );
#endif
    if ( hcurs && my_cursor )
	free(hcurs);
    if ( bm )
	delete bm;
    if ( bmm )
	delete bmm;
}

//**}

QCursor *QCursor::find_cur( int shape )		// find predefined cursor
{
    return (uint)shape <= LastCursor ? &cursorTable[shape] : 0;
}


static bool initialized = FALSE;
void QCursor::cleanup()
{
    if ( !initialized )
	return;
    
    int shape;
    for( shape = 0; shape < cursors; shape++ ) {
	delete cursorTable[shape].data;
	cursorTable[shape].data = 0;
    }
    initialized = FALSE;
}


void QCursor::initialize()
{
    InitCursor();
    int shape;
    for( shape = 0; shape < cursors; shape++ )
	cursorTable[shape].data = new QCursorData( shape );
    initialized = TRUE;
    qAddPostRoutine( cleanup );
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
    QCursor* c = &cursorTable[arrowCursorIdx];
    c->data->ref();
    data = c->data;
}



QCursor::QCursor(int shape)
{
    if ( !initialized )
	initialize();
    QCursor *c = find_cur( shape );
    if ( !c )					// not found
	c = &cursorTable[arrowCursorIdx];	//   then use arrowCursor
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
#if defined(QT_CHECK_NULL)
	qWarning( "QCursor: Cannot create bitmap cursor; invalid bitmap(s)" );
#endif
	QCursor *c = &cursorTable[arrowCursorIdx];
	c->data->ref();
	data = c->data;
	return;
    }
    data = new QCursorData;
    Q_CHECK_PTR( data );
    data->bm  = new QBitmap( bitmap );
    data->bmm = new QBitmap( mask );
    data->cshape = BitmapCursor;
    data->hx = hotX >= 0 ? hotX : bitmap.width()/2;
    data->hy = hotY >= 0 ? hotY : bitmap.height()/2;
    data->hcurs = NULL;

#if 0
    data->fg.red   = 0 << 8;
    data->fg.green = 0 << 8;
    data->fg.blue  = 0 << 8;
    data->bg.red   = 255 << 8;
    data->bg.green = 255 << 8;
    data->bg.blue  = 255 << 8;
#endif
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
    QCursor *c = find_cur( shape );
    if ( !c )					// not found
	c = &cursorTable[arrowCursorIdx];	//   then use arrowCursor
    c->data->ref();
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



Qt::HANDLE QCursor::handle() const
{
    if ( !initialized )
	initialize();
    if ( !data->hcurs )
	update();
    return data->hcurs;
}



QPoint QCursor::pos()
{
    Point p;
    GetMouse(&p);
    LocalToGlobal(&p);
    return QPoint(p.h, p.v);
}


// some kruft I found on the web.. it doesn't work, but I want to test more FIXME
#define MTemp 0x828
#define RawMouse 0x82c
#define CrsrNewCouple 0x8ce

void QCursor::setPos( int x, int y)
{
    HideCursor();
    Point where;
    where.h = x;
    where.v = y;
    *((Point *) RawMouse) = where ;
    *((Point *) MTemp) = where ;
    *((short *) CrsrNewCouple) = -1 ;
    ShowCursor ( ) ;
}

void QCursor::update() const
{
    if ( !initialized )
	initialize();
    register QCursorData *d = data;		// cheat const!
    if ( d->hcurs )				// already loaded
	return;

    if ( d->cshape == BitmapCursor ) {
#if 0
	d->hcurs = XCreatePixmapCursor( dpy, d->bm->handle(), d->bmm->handle(),
					&d->fg, &d->bg, d->hx, d->hy );

#else
	qDebug("Oops.. fix this.. %s:%d", __FILE__, __LINE__);
#endif
	return;
    }

    switch ( d->cshape ) {			// map Q cursor to X cursor
    case ArrowCursor:
    {
	static Cursor arrow;
	static bool got_arrow = FALSE;
	if(!got_arrow) {
	    got_arrow = TRUE;
	    GetQDGlobalsArrow(&arrow);
	}
	d->hcurs = &arrow;
	break;
    }
    case CrossCursor:
    {
	if(CursHandle c = GetCursor(::crossCursor))
	    d->hcurs = *c;
	break;
    }
    case WaitCursor:
    {
	if(CursHandle c = GetCursor(::watchCursor))
	    d->hcurs = *c;
	break;
    }
    case IbeamCursor:
    {
	if(CursHandle c = GetCursor(::iBeamCursor))
	    d->hcurs = *c;
	break;
    }
    case SizeAllCursor:
    {
	if(CursHandle c = GetCursor(::plusCursor))
	    d->hcurs = *c;
	break;
    }

#define QT_USE_APPROXIMATE_CURSORS
#ifdef QT_USE_APPROXIMATE_CURSORS
    case SizeVerCursor: 
    {
	static const uchar cur_ver_bits[] = {
	    0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0x03, 0xc0, 0x07, 0xe0, 0x0f, 0xf0,
	    0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x01, 0x80, 0x0f, 0xf0,
	    0x07, 0xe0, 0x03, 0xc0, 0x01, 0x80, 0x00, 0x00 };
	static const uchar mcur_ver_bits[] = {
	    0x00, 0x00, 0x03, 0x80, 0x07, 0xc0, 0x0f, 0xe0, 0x1f, 0xf0, 0x3f, 0xf8,
	    0x7f, 0xfc, 0x07, 0xc0, 0x07, 0xc0, 0x07, 0xc0, 0x7f, 0xfc, 0x3f, 0xf8,
	    0x1f, 0xf0, 0x0f, 0xe0, 0x07, 0xc0, 0x03, 0x80 };

	d->my_cursor = TRUE;
	d->hcurs = (CursPtr)malloc(sizeof(Cursor));
	memcpy(d->hcurs->data, cur_ver_bits, sizeof(cur_ver_bits));
	memcpy(d->hcurs->mask, mcur_ver_bits, sizeof(mcur_ver_bits));
	break;
    }

    case SizeHorCursor:
    {
	static const uchar cur_hor_bits[] = {
	    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x20, 0x18, 0x30,
	    0x38, 0x38, 0x7f, 0xfc, 0x7f, 0xfc, 0x38, 0x38, 0x18, 0x30, 0x08, 0x20,
	    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	static const uchar mcur_hor_bits[] = {
	    0x00, 0x00, 0x00, 0x00, 0x04, 0x40, 0x0c, 0x60, 0x1c, 0x70, 0x3c, 0x78,
	    0x7f, 0xfc, 0xff, 0xfe, 0xff, 0xfe, 0xff, 0xfe, 0x7f, 0xfc, 0x3c, 0x78,
	    0x1c, 0x70, 0x0c, 0x60, 0x04, 0x40, 0x00, 0x00 };

	d->my_cursor = TRUE;
	d->hcurs = (CursPtr)malloc(sizeof(Cursor));
	memcpy(d->hcurs->data, cur_hor_bits, sizeof(cur_hor_bits));
	memcpy(d->hcurs->mask, mcur_hor_bits, sizeof(mcur_hor_bits));
	break;
    }

    case SizeBDiagCursor:
    {
	static const uchar cur_fdiag_bits[] = {
	    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xf8, 0x00, 0xf8, 0x00, 0x78,
	    0x00, 0xf8, 0x01, 0xd8, 0x23, 0x88, 0x37, 0x00, 0x3e, 0x00, 0x3c, 0x00,
	    0x3e, 0x00, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00 };
	static const uchar mcur_fdiag_bits[] = {
	    0x00, 0x00, 0x00, 0x00, 0x07, 0xfc, 0x03, 0xfc, 0x01, 0xfc, 0x00, 0xfc,
	    0x41, 0xfc, 0x63, 0xfc, 0x77, 0xdc, 0x7f, 0x8c, 0x7f, 0x04, 0x7e, 0x00,
	    0x7f, 0x00, 0x7f, 0x80, 0x7f, 0xc0, 0x00, 0x00 };

	d->my_cursor = TRUE;
	d->hcurs = (CursPtr)malloc(sizeof(Cursor));
	memcpy(d->hcurs->data, cur_fdiag_bits, sizeof(cur_fdiag_bits));
	memcpy(d->hcurs->mask, mcur_fdiag_bits, sizeof(mcur_fdiag_bits));
	break;
    }
    case SizeFDiagCursor:
    {
	static const uchar cur_bdiag_bits[] = {
	    0x00, 0x00, 0x00, 0x00, 0x3f, 0x00, 0x3e, 0x00, 0x3c, 0x00, 0x3e, 0x00,
	    0x37, 0x00, 0x23, 0x88, 0x01, 0xd8, 0x00, 0xf8, 0x00, 0x78, 0x00, 0xf8,
	    0x01, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	static const uchar mcur_bdiag_bits[] = {
	    0x00, 0x00, 0x7f, 0xc0, 0x7f, 0x80, 0x7f, 0x00, 0x7e, 0x00, 0x7f, 0x04,
	    0x7f, 0x8c, 0x77, 0xdc, 0x63, 0xfc, 0x41, 0xfc, 0x00, 0xfc, 0x01, 0xfc,
	    0x03, 0xfc, 0x07, 0xfc, 0x00, 0x00, 0x00, 0x00 };

	d->my_cursor = TRUE;
	d->hcurs = (CursPtr)malloc(sizeof(Cursor));
	memcpy(d->hcurs->data, cur_bdiag_bits, sizeof(cur_bdiag_bits));
	memcpy(d->hcurs->mask, mcur_bdiag_bits, sizeof(mcur_bdiag_bits));
	break;
    }
    case BlankCursor:
    {
	d->my_cursor = TRUE;
	d->hcurs = (CursPtr)malloc(sizeof(Cursor));
	memset(d->hcurs->data, 0x00, sizeof(d->hcurs->data));
	memset(d->hcurs->mask, 0x00, sizeof(d->hcurs->data));
	break;
    }


    case UpArrowCursor:
    {
	static unsigned char cur_up_arrow_bits[] = {
	    0x00, 0x80, 0x01, 0x40, 0x01, 0x40, 0x02, 0x20, 0x02, 0x20, 0x04, 0x10,
	    0x04, 0x10, 0x08, 0x08, 0x0f, 0x78, 0x01, 0x40, 0x01, 0x40, 0x01, 0x40,
	    0x01, 0x40, 0x01, 0x40, 0x01, 0x40, 0x01, 0xc0 };
	static unsigned char mcur_up_arrow_bits[] = {
	    0x00, 0x80, 0x01, 0xc0, 0x01, 0xc0, 0x03, 0xe0, 0x03, 0xe0, 0x07, 0xf0,
	    0x07, 0xf0, 0x0f, 0xf8, 0x0f, 0xf8, 0x01, 0xc0, 0x01, 0xc0, 0x01, 0xc0,
	    0x01, 0xc0, 0x01, 0xc0, 0x01, 0xc0, 0x01, 0xc0 };

	d->my_cursor = TRUE;
	d->hcurs = (CursPtr)malloc(sizeof(Cursor));
	memcpy(d->hcurs->data, cur_up_arrow_bits, sizeof(cur_up_arrow_bits));
	memcpy(d->hcurs->mask, mcur_up_arrow_bits, sizeof(mcur_up_arrow_bits));
	break;
    }


    case SplitVCursor:
    {
	static unsigned char cur_vsplit_bits[] = {
	    0x04, 0x80, 0x04, 0x80, 0x04, 0x80, 0x04, 0x80, 0x04, 0x80, 0x04, 0x80, 
	    0x24, 0x90, 0x7c, 0xf8, 0x24, 0x90, 0x04, 0x80, 0x04, 0x80, 0x04, 0x80, 
	    0x04, 0x80, 0x04, 0x80, 0x04, 0x80, 0x00, 0x00 };
	static unsigned char mcur_vsplit_bits[] = {
	    0x04, 0x80, 0x04, 0x80, 0x04, 0x80, 0x04, 0x80, 0x04, 0x80, 0x04, 0x80, 
	    0x24, 0x90, 0x7c, 0xf8, 0x24, 0x90, 0x04, 0x80, 0x04, 0x80, 0x04, 0x80, 
	    0x04, 0x80, 0x04, 0x80, 0x04, 0x80, 0x00, 0x00 };

	d->my_cursor = TRUE;
	d->hcurs = (CursPtr)malloc(sizeof(Cursor));
	memcpy(d->hcurs->data, cur_vsplit_bits, sizeof(cur_vsplit_bits));
	memcpy(d->hcurs->mask, mcur_vsplit_bits, sizeof(mcur_vsplit_bits));
	break;
    }
    case SplitHCursor:
    {
	static unsigned char cur_hsplit_bits[] = {
	    0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x03, 0x80, 0x01, 0x00, 0x01, 0x00, 
	    0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 0xff, 0xfe, 0x01, 0x00, 0x01, 0x00, 
	    0x03, 0x80, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00 };
	static unsigned char mcur_hsplit_bits[] = {
	    0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x03, 0x80, 0x01, 0x00, 0x01, 0x00, 
	    0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 0xff, 0xfe, 0x01, 0x00, 0x01, 0x00, 
	    0x03, 0x80, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00 };

	d->my_cursor = TRUE;
	d->hcurs = (CursPtr)malloc(sizeof(Cursor));
	memcpy(d->hcurs->data, cur_hsplit_bits, sizeof(cur_hsplit_bits));
	memcpy(d->hcurs->mask, mcur_hsplit_bits, sizeof(mcur_hsplit_bits));
	break;
    }
#if 0
    case PointingHandCursor:
    {
	d->my_cursor = TRUE;
	d->hcurs = (CursPtr)malloc(sizeof(Cursor));
	memcpy(d->hcurs->data, cur_ver_bits, sizeof(cur_ver_bits));
	memcpy(d->hcurs->mask, mcur_ver_bits, sizeof(mcur_ver_bits));
	break;
    }
    case ForbiddenCursor:
    {
	d->my_cursor = TRUE;
	d->hcurs = (CursPtr)malloc(sizeof(Cursor));
	memcpy(d->hcurs->data, cur_ver_bits, sizeof(cur_ver_bits));
	memcpy(d->hcurs->mask, mcur_ver_bits, sizeof(mcur_ver_bits));
	break;
    }
#endif

#endif
    default:
#if defined(QT_CHECK_RANGE)
	qWarning( "QCursor::update: Invalid cursor shape %d", d->cshape );
#endif
	return;
    }

    if(d->hcurs && d->my_cursor) {
	d->hcurs->hotSpot.h = data->hx >= 0 ? data->hx : 8;
	d->hcurs->hotSpot.v = data->hy >= 0 ? data->hy : 8;
    }
}
