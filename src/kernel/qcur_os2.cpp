/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qcur_os2.cpp#10 $
**
** Implementation of QCursor class for OS/2 PM
**
** Created : 940712
**
** Copyright (C) 1994-1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qcursor.h"
#include "qapp.h"
#define	 INCL_WIN
#include <os2.h>

RCSTAG("$Id: //depot/qt/main/src/kernel/qcur_os2.cpp#10 $");


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


/*****************************************************************************
  QCursor member functions
 *****************************************************************************/

static QCursor *cursorTable[] = {		// order is important!!
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

void QCursor::initialize()			// initialize all cursors
{
    int shape = ArrowCursor;
    while ( cursorTable[shape] ) {
	cursorTable[shape]->cshape = shape;
	shape++;
    }
}

void QCursor::cleanup()
{
}


QCursor *QCursor::locate( int shape )		// get global cursor
{
    return shape >= ArrowCursor && shape <= SizeAllCursor ?
	   cursorTable[shape] : 0;
}


QCursor::QCursor()				// default arrow cursor
{
    if ( qApp )					// when not initializing
	setShape( ArrowCursor );
}

QCursor::QCursor( int shape )			// cursor with shape
{
    setShape( shape );
}

QCursor::~QCursor()
{
}


bool QCursor::setShape( int shape )		// set cursor shape
{
    cshape = shape;
    QCursor *c = locate( shape );		// find one of the global ones
    if ( c ) {
	c->update();
	hcurs = c->hcurs;			// copy attributes
    }
    else {					// no such global cursor!?
	hcurs = 0;
	return FALSE;
    }
    return TRUE;
}


QPoint QCursor::pos()				// get cursor position
{
    POINTL p;
    WinQueryPointerPos( HWND_DESKTOP, &p );
    return QPoint( p.x, p.y );
}

void QCursor::setPos( int x, int y )		// set cursor position
{
    WinSetPointerPos( HWND_DESKTOP, x, y );
}


void QCursor::update()				// update/load cursor
{
    if ( hcurs )				// already loaded
	return;

    int sh;
    switch ( cshape ) {				// map to windows cursor
	case ArrowCursor:
	    sh = SPTR_ARROW;
	    break;
	case UpArrowCursor:
	    sh = SPTR_ARROW;
	    break;
	case CrossCursor:
	    sh = SPTR_ARROW;
	    break;
	case WaitCursor:
	    sh = SPTR_WAIT;
	    break;
	case IbeamCursor:
	    sh = SPTR_TEXT;
	    break;
	case SizeVerCursor:
	    sh = SPTR_SIZEWE;
	    break;
	case SizeHorCursor:
	    sh = SPTR_SIZENS;
	    break;
	case SizeBDiagCursor:
	    sh = SPTR_SIZENESW;
	    break;
	case SizeFDiagCursor:
	    sh = SPTR_SIZENWSE;
	    break;
	case SizeAllCursor:
	    sh = SPTR_SIZE;
	    break;
	default:
#if defined(CHECK_RANGE)
	    warning( "QCursor::update: Invalid cursor shape %d", cshape );
#endif
	    return;
    }
    hcurs = WinQuerySysPointer( HWND_DESKTOP, sh, TRUE );
}
