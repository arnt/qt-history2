/****************************************************************************
** $Id: $
**
** DND implementation for mac.
**
** Created : 001019
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Macintosh may use this file in accordance with the Qt Commercial
** License Agreement provided with the Software.
**
** This file is not available for use under any other license without
** express written permission from the copyright holder.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
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
#include "qpainter.h"
#include "qcursor.h"

struct QMacDndExtra {
    QWidget *widget;
    bool acceptfmt;
    bool acceptact;
    int ref;
};

//internal globals
bool qt_mac_in_drag = FALSE;
static bool drag_received = FALSE;
static QDragObject::DragMode set_drag_mode; //passed in drag mode
static QDropEvent::Action current_drag_action; //current active drag action
static QDragObject *global_src = 0;
static QWidget *current_drag_widget = 0;
static DragReference current_dropobj = 0;
//cursors
static QCursor *noDropCursor = 0;
static QCursor *moveCursor = 0;
static QCursor *copyCursor = 0;
static QCursor *linkCursor = 0;
//default pixmap
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
//functions
extern uint qGlobalPostedEventsCount();
int qt_activate_null_timers(); //qapplication_mac.cpp
static QMAC_PASCAL OSErr qt_mac_tracking_handler( DragTrackingMessage theMessage, WindowPtr,
						  void *handlerRefCon, DragReference theDrag );
void qt_macdnd_unregister( QWidget *widget, QWExtra *extra );
void qt_macdnd_register( QWidget *widget, QWExtra *extra );

static void qt_mac_dnd_cleanup()
{
    delete noDropCursor;
    noDropCursor = NULL;
    delete moveCursor;
    moveCursor = NULL;
    delete copyCursor;
    copyCursor = NULL;
    delete linkCursor;
    linkCursor = NULL;
}

void updateDragMode() {
    if(set_drag_mode == QDragObject::DragDefault) { 
	UInt32 mod = GetCurrentKeyModifiers();
	if((mod & optionKey) || (mod & rightOptionKey))
	    current_drag_action = QDropEvent::Copy;
	else
	    current_drag_action = QDropEvent::Move;
    } else {
	if(set_drag_mode == QDragObject::DragMove)
	    current_drag_action = QDropEvent::Move;
	else if(set_drag_mode == QDragObject::DragLink)
	    current_drag_action = QDropEvent::Link;
	else if(set_drag_mode == QDragObject::DragCopy)
	    current_drag_action = QDropEvent::Copy;
	else
	    qDebug("not sure how to handle..");
    }
}

bool QDropEvent::provides( const char *fmt ) const
{
    const char *fmt2 = NULL;
    for ( int i = 0; (fmt2 = format( i ) ); i++ ) {
	if (!strncasecmp( fmt, fmt2, strlen( fmt )) )
	    return TRUE;
    }
    return FALSE;
}

static struct {
    ScrapFlavorType mac_type;
    const char *qt_type; 
} scrap_map[] = {
    { kScrapFlavorTypeUnicode, "text/plain;charset=ISO-10646-UCS-2" }, //highest priority
    { kScrapFlavorTypeText, "text/plain" },
    { 0, NULL } 
};

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

    for ( UInt16 i = 1; i <= cnt; i++ ) {

	if ( GetFlavorType(current_dropobj, ref, i, &info )) {
	    qDebug( "OOps.. %s:%d", __FILE__, __LINE__ );
	    return 0;
	}

	for(int sm = 0; scrap_map[sm].qt_type; sm++) {
	    if ( info == scrap_map[sm].mac_type && !strcmp(fmt, scrap_map[sm].qt_type) ) {
		if ( GetFlavorDataSize( current_dropobj, ref, info, &flavorsize ) ) {
		    qDebug( "Failure to get GetFlavorDataSize for %d", (int)info );
		    return 0;
		}
		buffer = (char *)malloc( flavorsize );
		GetFlavorData( current_dropobj, ref, info, buffer, &flavorsize, 0 );
		ret.assign( buffer, flavorsize );
		return ret;
	    }
	}

	if ( (info >> 16) != ('QTxx' >> 16) ) {
	    qDebug( "%s:%d Unknown type %c%c%c%c", __FILE__, __LINE__,
		    char(info >> 24), char((info >> 16) & 255), char((info >> 8) & 255), char(info & 255 ) );
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
	    int size = flavorsize - (mimesz + sizeof(mimesz));
	    memcpy( buffer, buffer + mimesz + sizeof(mimesz), size );
	    ret.assign( buffer, size );
	    return ret;
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

    for( ; i < (int)numFlavors; i++) {
	if ( GetFlavorType(current_dropobj, ref, i+1, &info ) ) {
	    qDebug( "OOps.. %d %s:%d", i, __FILE__, __LINE__ );
	    return 0;
	}
	if ((info >> 16) == ('QTxx' >> 16)) 
	    break;
	for(int sm = 0; scrap_map[sm].qt_type; sm++) 
	    if(info == scrap_map[sm].mac_type)
		return scrap_map[sm].qt_type;
	qDebug( "%s:%d Unknown type %c%c%c%c", __FILE__, __LINE__,
		char(info >> 24), char((info >> 16) & 255), char((info >> 8) & 255), 
		char(info & 255 ) );
    }
    if(i >= (int)numFlavors)
	return 0;

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
    if(qt_activate_null_timers()) 
	QApplication::flush();
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

bool QDragManager::drag( QDragObject *o, QDragObject::DragMode mode )
{
    //just make sure..
    if(qt_mac_in_drag) {
	qWarning("Whoa! This should never happen!");
	return FALSE;
    }
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

    FlavorType mactype;
    const char *fmt;
    for ( int i = 0; (fmt = o->format(i)); i++ ) {
	for(int sm = 0; scrap_map[sm].qt_type; sm++) {     //handle text/plain specially so other apps can get it
	    if(!strcmp(fmt, scrap_map[sm].qt_type)) {
		ar = o->encodedData(scrap_map[sm].qt_type);
		AddDragItemFlavor( theDrag, (ItemReference)1, scrap_map[sm].mac_type, ar.data(), ar.size(), 0 );
		continue;
	    }
	}

	//encode it..
	ar = o->encodedData( fmt );
	mactype = ('Q' << 24) | ('T' << 16) | (i & 0xFFFF);
	UInt32 mimelen = strlen( fmt );
	char *buffer = (char *)malloc( ar.size() + mimelen + sizeof(mimelen) );
	memcpy( buffer, &mimelen, sizeof(mimelen) );
	memcpy( buffer+sizeof(mimelen), fmt, mimelen );
	memcpy( buffer+sizeof(mimelen) + mimelen, ar.data(), ar.size() );
	AddDragItemFlavor( theDrag, (ItemReference)1, mactype, buffer, ar.size()+mimelen+sizeof(mimelen), 0 );
    }

    //so we must fake an event
    EventRecord fakeEvent;
    GetGlobalMouse(&(fakeEvent.where));
    fakeEvent.what = mouseDown;
    fakeEvent.when = GetDblTime();
    fakeEvent.modifiers = 0;

    Rect boundsRect;
    Point boundsPoint;
    QPoint hotspot;
    QPixmap pix = o->pixmap();
    if(pix.isNull()) {
	if(QTextDrag::canDecode(o)) {
	    //get the string
	    QString s;
	    QTextDrag::decode(o, s);
	    if(s.length() > 13) 
		s = s.left(13) + "...";
	    //draw it
	    QFont f(qApp->font());
	    f.setPointSize(12);
	    QFontMetrics fm(f);
	    QPixmap tmp(fm.width(s), fm.height());
	    QPainter p(&tmp);
	    p.fillRect(0, 0, tmp.width(), tmp.height(), color0);
	    p.setPen(color1);
	    p.setFont(f);
	    p.drawText(0, fm.ascent(), s);
	    //save it
	    pix = tmp;
	    hotspot = QPoint(tmp.width() / 2, tmp.height() / 2);
	} else {
	    pix = QImage(default_pm);
	    hotspot = QPoint(default_pm_hotx, default_pm_hoty);
	}
    } else {
	hotspot = QPoint(o->pixmapHotSpot().x(), o->pixmapHotSpot().y());
    }

    boundsPoint.h = fakeEvent.where.h - hotspot.x();
    boundsPoint.v = fakeEvent.where.v - hotspot.y();
    SetRect( &boundsRect, boundsPoint.h, boundsPoint.v, boundsPoint.h + pix.width(), boundsPoint.v + pix.height() );
    SetDragItemBounds( theDrag, (ItemReference)1 , &boundsRect );

#if defined( Q_WS_MACX ) 
    QRegion dragRegion(boundsPoint.h, boundsPoint.v, pix.width(), pix.height());
    QRegion r(0, 0, pix.width(), pix.height());
    SetDragImage(theDrag, GetGWorldPixMap((GWorldPtr)pix.handle()), r.handle(TRUE), boundsPoint, 0);
#else
    QBitmap pixbits;
    pixbits = pix;
    QRegion dragRegion(pixbits);
    dragRegion.translate(boundsPoint.h, boundsPoint.v);
#endif

    drag_received = FALSE;
    qt_mac_in_drag = TRUE;
    //kick off the drag by calling the callback ourselves first..
    QWidget *widget = QApplication::widgetAt(fakeEvent.where.h, fakeEvent.where.v, TRUE);
    if(!widget->extraData()->macDndExtra) //never too late I suppose..
	qt_macdnd_register( widget,  widget->extraData());
    set_drag_mode = mode;
    updateDragMode();
    qt_mac_tracking_handler( kDragTrackingEnterWindow, (WindowPtr)widget->hd,
			     (void *)widget->extraData()->macDndExtra, theDrag );
    int tid = startTimer( 1 );
    //now let the mac take control..
    result = TrackDrag( theDrag, &fakeEvent, dragRegion.handle(TRUE) ); 
    killTimer( tid );
    DisposeDrag( theDrag );
    qt_mac_in_drag = FALSE;

    return ((result == noErr)  && drag_received &&
            (current_drag_action == QDropEvent::Move) && 
	    !widget->extraData()->macDndExtra->acceptact);
}

void QDragManager::updatePixmap()
{
}

static QMAC_PASCAL OSErr qt_mac_receive_handler(WindowPtr, void *handlerRefCon, DragReference theDrag)
{ 
    updateDragMode();
    QMacDndExtra *macDndExtra = (QMacDndExtra*) handlerRefCon;
    current_dropobj = theDrag;
    Point mouse;
    GetDragMouse( theDrag, &mouse, 0L );
    QWidget *widget = QApplication::widgetAt( mouse.h, mouse.v, true );
    while( widget && !widget->acceptDrops() )
	widget = widget->parentWidget(TRUE);
    if(!widget)
	return 1;
    QDropEvent de( widget->mapFromGlobal( QPoint( mouse.h, mouse.v )) );
    de.setAction(current_drag_action);
    QApplication::sendEvent( widget, &de );
    macDndExtra->acceptact = de.isActionAccepted();
    macDndExtra->acceptfmt = de.isAccepted();
    return (drag_received = macDndExtra->acceptfmt) ? noErr : dragNotAcceptedErr;
}
static DragReceiveHandlerUPP qt_mac_receive_handlerUPP = NULL;
static void cleanup_dnd_receiveUPP() 
{
    if(qt_mac_receive_handlerUPP) {
	DisposeDragReceiveHandlerUPP(qt_mac_receive_handlerUPP);
	qt_mac_receive_handlerUPP = NULL;
    }
}    
static const DragReceiveHandlerUPP make_receiveUPP() 
{
    if(qt_mac_receive_handlerUPP)
	return qt_mac_receive_handlerUPP;
    qAddPostRoutine( cleanup_dnd_receiveUPP );
    return qt_mac_receive_handlerUPP = NewDragReceiveHandlerUPP(qt_mac_receive_handler);
}

static QMAC_PASCAL OSErr qt_mac_tracking_handler( DragTrackingMessage theMessage, WindowPtr,
						  void *handlerRefCon, DragReference theDrag )
{
    if(theMessage != kDragTrackingEnterWindow && theMessage != kDragTrackingLeaveWindow &&
       theMessage != kDragTrackingInWindow) {
	return 1;
    } else if(!theDrag) {
	qDebug( "DragReference null %s %d", __FILE__, __LINE__ );
	return 1;
    } else if(qt_mac_in_drag && drag_received) { //ignore these
	return 0;
    }
    updateDragMode();
    Point mouse;
    GetDragMouse( theDrag, &mouse, 0L );
    if(!mouse.h && !mouse.v)
	GetGlobalMouse(&mouse);
    QPoint globalMouse( mouse.h, mouse.v );
    QMacDndExtra *macDndExtra = (QMacDndExtra*) handlerRefCon;
    QWidget *widget = QApplication::widgetAt( globalMouse, TRUE );
    while ( widget && (!widget->acceptDrops()) )
	widget = widget->parentWidget(TRUE);
    //Dispatch events
    if (widget && theMessage == kDragTrackingInWindow && widget == current_drag_widget ) {
        QDragMoveEvent de( widget->mapFromGlobal( globalMouse ) );
	de.setAction(current_drag_action);
	QApplication::sendEvent( widget, &de );
	macDndExtra->acceptfmt = de.isAccepted();
	macDndExtra->acceptact = de.isActionAccepted();
    } else { 
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
	    if(widget != current_drag_widget) {
		QDragEnterEvent de( widget->mapFromGlobal( globalMouse ) );
		de.setAction(current_drag_action);
		QApplication::sendEvent(widget, &de );
		macDndExtra->acceptfmt = de.isAccepted();
		macDndExtra->acceptact = de.isActionAccepted();
		current_drag_widget = widget;
	    }
	} 
    }

    //set the cursor
    if ( !noDropCursor ) {
	noDropCursor = new QCursor( QCursor::ForbiddenCursor );
#if 0 //need to make our own cursors, FIXME!
	if ( !pm_cursor[0].isNull() )
	    moveCursor = new QCursor(pm_cursor[0], 0,0);
	if ( !pm_cursor[1].isNull() )
	    copyCursor = new QCursor(pm_cursor[1], 0,0);
	if ( !pm_cursor[2].isNull() )
	    linkCursor = new QCursor(pm_cursor[2], 0,0);
#endif
	qAddPostRoutine( qt_mac_dnd_cleanup );
    }
    const QCursor *cursor = NULL;
    if(widget && macDndExtra->acceptfmt) {
	if(current_drag_action == QDropEvent::Move)
	    cursor = moveCursor;
	else if(current_drag_action == QDropEvent::Copy)
	    cursor = copyCursor;
	else if(current_drag_action == QDropEvent::Link)
	    cursor = linkCursor;
    } else {
	cursor = noDropCursor;
    }
    if(!cursor) {
	if(qApp && qApp->overrideCursor())
	    cursor = qApp->overrideCursor();
	else
	    cursor = &Qt::arrowCursor;
    }
    qt_mac_set_cursor(cursor, &mouse); 

    //idle things
    if(qGlobalPostedEventsCount()) {
	QApplication::sendPostedEvents();
	QApplication::flush();
    }
    return 0;
}
static DragTrackingHandlerUPP qt_mac_tracking_handlerUPP = NULL;
static void cleanup_dnd_trackingUPP() 
{
    if(qt_mac_tracking_handlerUPP) {
	DisposeDragTrackingHandlerUPP(qt_mac_tracking_handlerUPP);
	qt_mac_tracking_handlerUPP = NULL;
    }
}    
static const DragTrackingHandlerUPP make_trackingUPP() 
{
    if(qt_mac_tracking_handlerUPP)
	return qt_mac_tracking_handlerUPP;
    qAddPostRoutine( cleanup_dnd_trackingUPP );
    return qt_mac_tracking_handlerUPP = NewDragTrackingHandlerUPP(qt_mac_tracking_handler);
}

void qt_macdnd_unregister( QWidget *widget, QWExtra *extra )
{
    if ( extra && extra->macDndExtra  && !(--extra->macDndExtra->ref)) {
	if(qt_mac_tracking_handlerUPP)
	    RemoveTrackingHandler( make_trackingUPP(), (WindowPtr)widget->handle() );
	if(qt_mac_receive_handlerUPP)
	    RemoveReceiveHandler( make_receiveUPP(), (WindowPtr)widget->handle() );
	delete extra->macDndExtra;
	extra->macDndExtra = 0;
    }
}

void qt_macdnd_register( QWidget *widget, QWExtra *extra )
{
    if ( !extra->macDndExtra ) {
	extra->macDndExtra = new QMacDndExtra;
	extra->macDndExtra->ref = 1;
	InstallTrackingHandler( make_trackingUPP(),  (WindowPtr)widget->handle(),
				extra->macDndExtra );
	InstallReceiveHandler( make_receiveUPP(), (WindowPtr)widget->handle(),
			       extra->macDndExtra );
    } else {	
	extra->macDndExtra->ref++;
    }
}

#endif // QT_NO_DRAGANDDROP
