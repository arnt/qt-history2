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

#include <stdlib.h>
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

DragReference current_dropobj = 0;

static const int default_pm_hotx = -2;
static const int default_pm_hoty = -16;
static const char* default_pm[] = {
"13 9 3 1",
".      c None",
"       c #000000",
"X      c #FFFFFF",
"X X X X X X X",
" X X X X X X ",
"X ......... X",
" X.........X ",
"X ......... X",
" X.........X ",
"X ......... X",
" X X X X X X ",
"X X X X X X X",
};


bool QDropEvent::provides( const char *fmt ) const
{
    const char *fmt2 = NULL;
    for ( int i = 0; (fmt2 = format( i ) ); i++ ) {
	if (!strncasecmp( fmt, fmt2, strlen( fmt )) )
	    return TRUE;
    }
    return FALSE;
}

QByteArray QDropEvent::encodedData( const char *fmt ) const
{
    QByteArray ret;
    char *buffer = NULL;
    FlavorType info = NULL;
    Size flavorsize=0;
    ItemReference ref = NULL;

    if ( GetDragItemReferenceNumber( current_dropobj, 1, &ref ) ) {
	qDebug( "OOps.. %s:%d", __FILE__, __LINE__ );
	return 0;
    }

    UInt16 cnt = 0;
    if ( CountDragItemFlavors( current_dropobj, ref, &cnt ) || !cnt ) 
	return 0;

    bool find_text = (!strcmp( fmt, "text/plain" ));
    for ( UInt16 i = 1; i <= cnt; i++ ) {

	if ( GetFlavorType(current_dropobj, ref, i, &info )) {
	    qDebug( "OOps.. %s:%d", __FILE__, __LINE__ );
	    return 0;
	}

	if ( find_text && info == 'TEXT' ) {
	    if ( GetFlavorDataSize( current_dropobj, ref, info, &flavorsize ) ) {
		qDebug( "Failure to get GetFlavorDataSize for %d", (int)info );
		return 0;
	    }
	    buffer = (char *)malloc( flavorsize );
	    GetFlavorData( current_dropobj, ref, info, buffer, &flavorsize, 0 );
	    ret.duplicate( buffer, flavorsize );
	    delete buffer;
	    return ret;
	} else {
	    if ( (info >> 16) != ('QTxx' >> 16) ) {
		qDebug( "Unknown type %c%c%c%c", char(info >> 24),  
			char((info >> 16) & 255), char((info >> 8) & 255),
			char(info & 255 ));
		continue;
	    }
	    
	    if ( GetFlavorDataSize( current_dropobj, ref, info, &flavorsize ) ) {
		qDebug( "Failure to get GetFlavorDataSize for %d", (int)info );
		return 0;
	    }

	    buffer = (char *)malloc( flavorsize );
	    GetFlavorData( current_dropobj, ref, info, buffer, &flavorsize, 0 );
	    UInt32 mimesz;
	    memcpy( &mimesz, buffer, sizeof(mimesz) );
	    if( !strncasecmp( buffer+sizeof(mimesz), fmt, mimesz ) ) {
	    // FIXME: Tell sam secret drag bus errored when using assign
	    // at the point when the QByteArray destroyed.
		ret.duplicate( buffer + mimesz + sizeof(mimesz), 
			       flavorsize - (mimesz + sizeof(mimesz)) );
		delete buffer;
		return ret;
	    }
	}
    }
    return 0;
}

const char* QDropEvent::format( int i ) const
{
    char *buffer = NULL;
    FlavorType info = NULL;
    Size flavorsize = 0, typesize = 0, realsize = sizeof(typesize);
    ItemReference ref = NULL;
    unsigned short numFlavors;
    
    if ( GetDragItemReferenceNumber( current_dropobj, 1, &ref ) ) {
	qDebug( "OOps.. %s:%d", __FILE__, __LINE__ );
	return 0;
    }

    if ( CountDragItemFlavors( current_dropobj, ref, &numFlavors ) ) {
	qDebug( "OOps.. %s:%d", __FILE__, __LINE__ );
	return 0;
    }
    if ( i >= numFlavors )
	return 0;

    if ( GetFlavorType(current_dropobj, ref, i+1, &info ) ) {
	qDebug( "OOps.. %d %s:%d", i, __FILE__, __LINE__ );
	return 0;
    }

    if ( info == 'TEXT' ) {
	return "text/plain";
    }

    if ((info >> 16) != ('QTxx' >> 16)) {
	qDebug( "Unknown type %c%c%c%c", char(info >> 24), char((info >> 16) & 255),
		char((info >> 8) & 255), char(info & 255 ) );
        return 0;
    }
    if ( GetFlavorDataSize( current_dropobj, ref, info, &flavorsize) || flavorsize < 4 ) {
	qDebug( "Failure to get ScrapFlavorSize for %s:%d %d %d", __FILE__, __LINE__, 
	       (int)flavorsize, (int)info );
	return 0;
    }
    GetFlavorData( current_dropobj, ref, info, &typesize, &realsize, 0 );

    buffer = (char *)malloc(typesize + 1);
    GetFlavorData( current_dropobj, ref, info, buffer, &typesize, sizeof(typesize) );

    if (typesize < 0) {
	qDebug( "typesize negative %s:%d", __FILE__, __LINE__ );
	return 0;
    }

    *(buffer + typesize) = '\0';
    return buffer;
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


bool QDragManager::drag( QDragObject *o, QDragObject::DragMode )
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
    QByteArray ar;

    if ( (result = NewDrag(&theDrag)) )
	return( !result );

    if ( o->provides( "text/plain" ) ) {
	ar = o->encodedData( "text/plain" );
	AddDragItemFlavor( theDrag, (ItemReference)1, 'TEXT',
			   ar.data(), ar.size(), 0 );
    }
	
    //now the other formats
    FlavorType mactype;
    const char *fmt;
    for ( int i = 0; (fmt = o->format(i)); i++ ) {
	if (!strcmp( fmt, "text/plain" ))
	    continue; //already did that

	//encode it..
	ar = o->encodedData( fmt );
	mactype = ('Q' << 24) | ('T' << 16) | (i & 0xFFFF);
	UInt32 mimelen = strlen( fmt );
	char *buffer = (char *)malloc( ar.size() + mimelen + sizeof(mimelen) );
	memcpy( buffer, &mimelen, sizeof(mimelen) );
	memcpy( buffer+sizeof(mimelen), fmt, mimelen );
	memcpy( buffer+sizeof(mimelen) + mimelen, ar.data(), ar.size() );
	AddDragItemFlavor( theDrag, (ItemReference)1, mactype,
			   buffer, ar.size()+mimelen+sizeof(mimelen), 0 );
    }

    GetMouse( &(fakeEvent.where) );
    LocalToGlobal( &(fakeEvent.where) );

    Rect boundsRect;
    Point boundsPoint;
    QPoint hotspot;
    QPixmap pix = o->pixmap();
    if(pix.isNull()) {
	pix = QImage(default_pm);
	hotspot = QPoint(default_pm_hotx, default_pm_hoty);
    } else {
	hotspot = QPoint(o->pixmapHotSpot().x(), o->pixmapHotSpot().y());
    }

    boundsPoint.h = fakeEvent.where.h - hotspot.x();
    boundsPoint.v = fakeEvent.where.v - hotspot.y();
    SetRect( &boundsRect, boundsPoint.h, boundsPoint.v, boundsPoint.h + pix.width(), boundsPoint.v + pix.height() );
    SetDragItemBounds( theDrag, (ItemReference)1 , &boundsRect );

#ifdef Q_WS_MACX 
    QRegion dragRegion(boundsPoint.h, boundsPoint.v, pix.width(), pix.height());
	QRegion r(0, 0, pix.width(), pix.height());
	SetDragImage(theDrag, GetGWorldPixMap((GWorldPtr)pix.handle()), (RgnHandle)r.handle(), boundsPoint, 0);
#else
    QBitmap pixbits;
    pixbits = pix;
    QRegion dragRegion(pixbits);
    dragRegion.translate(boundsPoint.h, boundsPoint.v);
#endif

    fakeEvent.what = 0;
    fakeEvent.when = 0;
    fakeEvent.modifiers = 0;
    result = TrackDrag( theDrag, &fakeEvent, (RgnHandle)dragRegion.handle() );

    DisposeDrag( theDrag );
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


QMAC_PASCAL OSErr MyReceiveHandler(WindowPtr, void *handlerRefCon,
		       DragReference theDrag)
{ 
    QMacDndExtra *macDndExtra = (QMacDndExtra*) handlerRefCon;
    Point mouse;
    QPoint globalMouse;
    current_dropobj = theDrag;
    GetDragMouse( theDrag, &mouse, 0L );
    globalMouse = QPoint( mouse.h, mouse.v );
    QWidget *widget = QApplication::widgetAt( mouse.h, mouse.v, true );
    if ( !widget || (!widget->acceptDrops()) )
	return 1;

    QDropEvent de( widget->mapFromGlobal( globalMouse ) );
    QApplication::sendEvent( widget, &de );
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

QMAC_PASCAL OSErr MyTrackingHandler( DragTrackingMessage theMessage, WindowPtr,
			 void *handlerRefCon, DragReference theDrag )
{
    QMacDndExtra *macDndExtra = (QMacDndExtra*) handlerRefCon;
    Point mouse;
    QPoint globalMouse;

    if(!theDrag) {
	qDebug( "DragReference null %s %d", __FILE__, __LINE__ );
	return 1;
    }

    GetDragMouse( theDrag, &mouse, 0L );
    globalMouse = QPoint( mouse.h, mouse.v );

    Point local;
    local.h = mouse.h;
    local.v = mouse.v;
    QMacSavedPortInfo savedInfo(macDndExtra->widget);;
    GlobalToLocal( &local );
    QWidget *widget = recursive_match( macDndExtra->widget, local.h, local.v );

//  FIXME: Mention to sam that this line below did not work.
//  QWidget *widget = QApplication::widgetAt( mouse.h, mouse.v, true );

    if ( widget && (!widget->acceptDrops()) )
	widget = 0;

    if (widget && (theMessage == kDragTrackingInWindow) &&
	(widget == current_drag_widget) ) {
        QDragMoveEvent de( widget->mapFromGlobal( globalMouse ) );
	QApplication::sendEvent( widget, &de );
	macDndExtra->acceptfmt = de.isAccepted();
	macDndExtra->acceptact = de.isActionAccepted();
	return 0;
    }

    if ( current_drag_widget && ((theMessage == kDragTrackingLeaveWindow) || 
				 (widget != current_drag_widget))) {
	macDndExtra->acceptfmt = FALSE;
	current_dropobj = 0;
	QDragLeaveEvent de;
	QApplication::sendEvent( current_drag_widget, &de );
	current_drag_widget = 0;
    }

    if ( widget ) {
	current_dropobj = theDrag;
	QDragEnterEvent de( widget->mapFromGlobal( globalMouse ) );
	QApplication::sendEvent(widget, &de );
	macDndExtra->acceptfmt = de.isAccepted();
	macDndExtra->acceptact = de.isActionAccepted();
	current_drag_widget = widget;
    }
    QApplication::sendPostedEvents();

    return 0;
}

void qt_macdnd_unregister( QWidget *widget, QWExtra *extra )
{
    if ( extra && extra->macDndExtra ) {
	extra->macDndExtra->ref--;
	if ( extra->macDndExtra->ref == 0 ) {
	    RemoveTrackingHandler( NewDragTrackingHandlerUPP(MyTrackingHandler), (WindowPtr)widget->winId() );
	    RemoveReceiveHandler( NewDragReceiveHandlerUPP(MyReceiveHandler), (WindowPtr)widget->winId() );
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
	extra->macDndExtra->widget = widget->topLevelWidget();
	if ( (result = InstallTrackingHandler( NewDragTrackingHandlerUPP(MyTrackingHandler), 
					      (WindowPtr)widget->winId(),
					      extra->macDndExtra )))
	    return;
	if ( (result = InstallReceiveHandler( NewDragReceiveHandlerUPP(MyReceiveHandler),
					     (WindowPtr)widget->winId(),
					     extra->macDndExtra )))
	    return;
    } 	else {	
	extra->macDndExtra->ref++;
    }
}

#endif // QT_NO_DRAGANDDROP
