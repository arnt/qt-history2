/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qdnd_win.cpp#1 $
**
** WM_FILES implementation for Qt.
**
** Created : 980320
**
** Copyright (C) 1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qapp.h"
#include "qwidget.h"
#include "qdragobject.h"


void QDragManager::cancel()
{
    debug( "c " );
    if ( object ) {
	beingCancelled = TRUE;
	if ( object->autoDelete() )
	    delete object;
	object = 0;
    }

    // insert cancel code here

    if ( restoreCursor ) {
	QApplication::restoreOverrideCursor();
	restoreCursor = FALSE;
    }
}


void QDragManager::move( const QPoint & globalPos )
{
    // tbi
}


void QDragManager::drop()
{
    // tbi
}



void QDragManager::registerDropType( QWidget *, const char * )
{
    // tbi
}


const char * QDragMoveEvent::format( int  )
{
    return 0;
}


const QByteArray QDragMoveEvent::data( const char * format )
{
    QByteArray tmp;
    return tmp;
}


const QByteArray & QDropEvent::data( const char * format )
{
    QByteArray tmp;
    return tmp;
}
