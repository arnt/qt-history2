/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qcur_win.cpp#1 $
**
** Implementation of QCursor class for Windows + NT
**
** Author  : Haavard Nord
** Created : 940219
**
** Copyright (C) 1994 by Troll Tech as.  All rights reserved.
**
*****************************************************************************/

#include "qcursor.h"
#include "qapp.h"
#include <windows.h>

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qcur_win.cpp#1 $";
#endif


// --------------------------------------------------------------------------
// Global cursors
//

const QCursor arrowCursor;
const QCursor upArrowCursor;
const QCursor crossCursor;
const QCursor hourGlassCursor;
const QCursor ibeamCursor;
const QCursor sizeVerCursor;
const QCursor sizeHorCursor;
const QCursor sizeBDiagCursor;
const QCursor sizeFDiagCursor;
const QCursor sizeAllCursor;


// --------------------------------------------------------------------------
// QCursor member functions
//

static QCursor *cursorTable[] = {		// order is important!!
    (QCursor*)&arrowCursor,
    (QCursor*)&upArrowCursor,
    (QCursor*)&crossCursor,
    (QCursor*)&hourGlassCursor,
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
    POINT p;
    GetCursorPos( &p );
    return QPoint( (QCOOT)p.x, (QCOOT)p.y );
}

void QCursor::setPos( int x, int y )		// set cursor position
{
    SetCursorPos( x, y );
}


void QCursor::update()				// update/load cursor
{
    if ( hcurs )				// already loaded
	return;

    char const *sh;
    switch ( cshape ) {				// map to windows cursor
	case ArrowCursor:
	    sh = IDC_ARROW;
	    break;
	case UpArrowCursor:
	    sh = IDC_UPARROW;
	    break;
	case CrossCursor:
	    sh = IDC_CROSS;
	    break;
	case HourGlassCursor:
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
	default:
#if defined(CHECK_RANGE)
	    warning( "QCursor::update: Invalid cursor shape %d", cshape );
#endif
	    return;
    }
    hcurs = LoadCursor( 0, sh );
}
