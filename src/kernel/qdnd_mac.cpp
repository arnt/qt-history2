/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qdnd_mac.cpp
**
** DND implementation for mac.
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
** licenses for Unix/X11 may use this file in accordance with the Qt Commercial
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

#include "qapplication.h"

#ifndef QT_NO_DRAGANDDROP
#include "qwidget.h"
#include "qintdict.h"
#include "qdatetime.h"
#include "qdict.h"
#include "qdragobject.h"
#include "qobjectlist.h"
#include "qbitmap.h"
#include "qt_mac.h"


bool QDropEvent::provides( const char * ) const
{
    return TRUE;
}

QByteArray QDropEvent::encodedData( const char * ) const
{
    return QByteArray();
}

const char* QDropEvent::format( int i ) const
{
    if ( i == 0 )
	return "text/plain";
    return 0;
}

void QDragManager::timerEvent( QTimerEvent* )
{
    return;
}

bool QDragManager::eventFilter( QObject *, QEvent * )
{
    return FALSE;
}

void QDragManager::updateMode( ButtonState )
{
}

void QDragManager::updateCursor()
{
}

void QDragManager::cancel( bool )
{
    if ( object ) {
    beingCancelled = TRUE;
    object = 0;
    }
}

void QDragManager::move( const QPoint & )
{
}

void QDragManager::drop()
{
}

static QDragObject *global_src = 0;

EventRecord fakeEvent;


bool QDragManager::drag( QDragObject *o, QDragObject::DragMode mode )
{
    if ( object == o )
	return FALSE;

    if ( object ) {
	cancel();
	if ( dragSource )
	    dragSource->removeEventFilter( this );
	beingCancelled = FALSE;
    }

    object = o;
    dragSource = (QWidget *)(object->parent());
    global_src = o;
    global_src->setTarget(0);

    OSErr result;
    DragReference theDrag;
    RgnHandle dragRegion;

    if ( result = NewDrag(&theDrag) )
	return( !result );

    char *test = "abcdefghijk";
    AddDragItemFlavor( theDrag, (ItemReference)test, 'DATA', test, 11, flavorSenderOnly );
    AddDragItemFlavor( theDrag, (ItemReference)test, 'TEXT', 0L, 0L, 0 );

    GetMouse( &(fakeEvent.where) );
    Rect boundsRect;
    SetRect( &boundsRect, fakeEvent.where.h, fakeEvent.where.v, 
	     fakeEvent.where.h + 100, fakeEvent.where.v + 20 );
    dragRegion = NewRgn();
    OpenRgn();
    FrameRect(&boundsRect);
    CloseRgn(dragRegion);
 
    SetDragItemBounds( theDrag, (ItemReference)test , &boundsRect );

    fakeEvent.what = 0;
    fakeEvent.when = 0;
    fakeEvent.modifiers = 0;

    result = TrackDrag( theDrag, &fakeEvent, dragRegion );

    DisposeRgn(dragRegion);
    DisposeDrag(theDrag);

    return !result;
}

void QDragManager::updatePixmap()
{
}


struct QMacDndExtra {
    QWidget *widget;
    bool acceptfmt;
    bool acceptact;
    int ref;
};

DragReference current_dropobj = 0;

OSErr MyReceiveHandler(WindowPtr, void *handlerRefCon,
		       DragReference theDrag)
{ 
    qDebug( "MyReceiveHandler" );
    QMacDndExtra *macDndExtra = (QMacDndExtra*) handlerRefCon;
    Point mouse;
    QPoint globalMouse;
    current_dropobj = theDrag;
    GetDragMouse( theDrag, &mouse, 0L );
    globalMouse = QPoint( mouse.h, mouse.v );
    QWidget *widget = QApplication::widgetAt( mouse.h, mouse.v, true );
    if ( !widget || (!widget->acceptDrops()) )
	return 0;

    QDropEvent de( widget->mapFromGlobal( globalMouse ) );
    QApplication::sendEvent( macDndExtra->widget, &de );
    macDndExtra->acceptact = de.isActionAccepted();

    return 0;
}

QWidget *current_drag_widget = 0;

//FIXME: This is duplicated code
static QWidget *recursive_match(QWidget *widg, int x, int y)
{
    // Keep looking until we find ourselves in a widget with no kiddies
    // where the x,y is
    if(!widg) 
	return 0;

    const QObjectList *objl=widg->children();
    if(!objl) // No children 
	return widg;

    QObjectListIt it(*objl);
    for(it.toLast(); it.current(); --it) {
	if((*it)->isWidgetType()) {
	    QWidget *curwidg=(QWidget *)(*it);
	    if(curwidg->isVisible()) {
		int wx=curwidg->x(), wy=curwidg->y();
		int wx2=wx+curwidg->width(), wy2=wy+curwidg->height();
		if(x>=wx && y>=wy && x<=wx2 && y<=wy2) {
		    return recursive_match(curwidg,x-wx,y-wy);
		} 
	    }
	}
    }
    // If we get here, it's within a widget that has children, but isn't in any
    // of the children
    return widg;
}

OSErr MyTrackingHandler( DragTrackingMessage theMessage, WindowPtr theWindow,
			 void *handlerRefCon, DragReference theDrag )
{
    QMacDndExtra *macDndExtra = (QMacDndExtra*) handlerRefCon;
    Point mouse;
    QPoint globalMouse;
    DragAttributes attributes;
    RgnHandle hiliteRgn;

    GetDragMouse( theDrag, &mouse, 0L );
    globalMouse = QPoint( mouse.h, mouse.v );
    qDebug( "coords %d %d", mouse.h, mouse.v );

    Point local;
    local.h = mouse.h;
    local.v = mouse.v;
    SetPortWindowPort( (WindowPtr)macDndExtra->widget->winId() );
    GlobalToLocal( &local );
    QWidget *widget = recursive_match( macDndExtra->widget, local.h, local.v );

//  FIXME: Mention to sam that this line below did not work.
//  QWidget *widget = QApplication::widgetAt( mouse.h, mouse.v, true );
    qDebug( "widget %d", widget );
    if ( widget && (!widget->acceptDrops()) )
	widget = 0;

    if (widget && (theMessage == kDragTrackingInWindow) &&
	(widget == current_drag_widget) ) {
	qDebug( "dragTrackingInWindow" );
        QDragMoveEvent de( widget->mapFromGlobal( globalMouse ) );
	QApplication::sendEvent( widget, &de );
	macDndExtra->acceptfmt = de.isAccepted();
	macDndExtra->acceptact = de.isActionAccepted();
	return 0;
    }

    if ( current_drag_widget && ((theMessage == kDragTrackingLeaveWindow) || 
				 (widget != current_drag_widget))) {
	qDebug( "dragTrackingLeaveWindow" );
	macDndExtra->acceptfmt = FALSE;
	current_dropobj = 0;
	QDragLeaveEvent de;
	QApplication::sendEvent( current_drag_widget, &de );
	current_drag_widget = 0;
    }

    if ( widget ) {
	qDebug( "dragTrackingEnterWindow" );
	current_dropobj = theDrag;
	QDragEnterEvent de( widget->mapFromGlobal( globalMouse ) );
	QApplication::sendEvent(widget, &de );
	macDndExtra->acceptfmt = de.isAccepted();
	macDndExtra->acceptact = de.isActionAccepted();
	current_drag_widget = widget;
    }

    return 0;
}

void qt_macdnd_unregister( QWidget *widget, QWExtra *extra )
{
    if ( extra && extra->macDndExtra ) {
	extra->macDndExtra->ref--;
	if ( extra->macDndExtra->ref == 0 ) {
	    RemoveTrackingHandler( MyTrackingHandler, (WindowPtr)widget->winId() );
	    RemoveReceiveHandler( MyReceiveHandler, (WindowPtr)widget->winId() );
	    delete extra->macDndExtra;
	    extra->macDndExtra = 0;
	}
    }
}

void qt_macdnd_register( QWidget *widget, QWExtra *extra )
{
    if ( !extra->macDndExtra ) {
	OSErr result;
	extra->macDndExtra = new QMacDndExtra;
	extra->macDndExtra->ref = 1;
	qDebug( "qt_macdnd_register %d %s", widget, widget->name() );
	extra->macDndExtra->widget = widget->topLevelWidget();
	if ( result = InstallTrackingHandler( MyTrackingHandler, 
					      (WindowPtr)widget->winId(),
					      extra->macDndExtra ))
	    return;
	if ( result = InstallReceiveHandler( MyReceiveHandler,
					     (WindowPtr)widget->winId(),
					     extra->macDndExtra ))
	    return;
    } 	else {	
	extra->macDndExtra->ref++;
    }
}

#endif // QT_NO_DRAGANDDROP
