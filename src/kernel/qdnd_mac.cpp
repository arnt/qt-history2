/****************************************************************************
** $Id$
**
** DND implementation for mac.
**
** Created : 001019
**
** Copyright (C) 1992-2002 Trolltech AS.  All rights reserved.
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
#include <stdlib.h>
#include <string.h>

/*****************************************************************************
  QClipboard debug facilities
 *****************************************************************************/
//#define DEBUG_MAPPINGS
//#define DEBUG_DRAG_EVENTS


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
static bool acceptfmt = FALSE;
static bool acceptact = FALSE;

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
OSErr FSpLocationFromFullPath(short, const void *, FSSpec *); //qsound_mac.cpp
static QMAC_PASCAL OSErr qt_mac_tracking_handler(DragTrackingMessage, WindowPtr,
						  void *, DragReference);
void qt_macdnd_unregister(QWidget *, QWExtra *);
void qt_macdnd_register(QWidget *, QWExtra *);

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

void updateDragMode(DragReference drag) {
    if(set_drag_mode == QDragObject::DragDefault) { 
	SInt16 mod;
	GetDragModifiers(drag, &mod, NULL, NULL);
	if((mod & optionKey) || (mod & rightOptionKey)) {
//	    SetDragAllowableActions(drag, kDragActionCopy, false);
	    current_drag_action = QDropEvent::Copy;
	} else {
//	    SetDragAllowableActions(drag, kDragActionMove, false);
	    current_drag_action = QDropEvent::Move;
	}
    } else {
	if(set_drag_mode == QDragObject::DragMove)
	    current_drag_action = QDropEvent::Move;
	else if(set_drag_mode == QDragObject::DragLink)
	    current_drag_action = QDropEvent::Link;
	else if(set_drag_mode == QDragObject::DragCopy)
	    current_drag_action = QDropEvent::Copy;
	else
	    qDebug("Qt: internal: not sure how to handle..");
    }
}

bool QDropEvent::provides(const char *fmt) const
{
    const char *fmt2 = NULL;
    for(int i = 0; (fmt2 = format(i)); i++) {
	if(!qstrnicmp(fmt, fmt2, strlen(fmt)))
	    return TRUE;
    }
    return FALSE;
}

enum {
    kDragQtGeneratedMarker = 'CUTE'
};

#ifdef DEBUG_MAPPINGS
#  define MAP_FLAVOUR(x) x, #x
#else
#  define MAP_FLAVOUR(x) x
#endif
static struct SM {
    ScrapFlavorType mac_type;
#ifdef DEBUG_MAPPINGS
    const char *mac_type_name;
#endif
    const char *qt_type; 
} drag_map[] = {
    //text (unicode has highest priority)
    { MAP_FLAVOUR(kScrapFlavorTypeUnicode), "text/plain;charset=ISO-10646-UCS-2" }, 
    { MAP_FLAVOUR(kScrapFlavorTypeText), "text/plain;charset=utf-8" },
    //url's (prefer fileURL over HFS)
    { MAP_FLAVOUR(typeFileURL), "text/uri-list" }, 
    { MAP_FLAVOUR(kDragFlavorTypeHFS), "text/uri-list" },
    //end marker
    { MAP_FLAVOUR(0), NULL } 
};


#ifdef DEBUG_MAPPINGS
static inline void debug_drag_flav(DragReference theDrag, const char *where) {
    UInt16 cnt_items;
    CountDragItems(theDrag, &cnt_items);
    qDebug("---------- %s -- full payload follows %d-----------------", where, cnt_items);
    for(int i = 1; i <= cnt_items; i++) {
    ItemReference ref = NULL;
    if(GetDragItemReferenceNumber(theDrag, i, &ref)) {
	qDebug("OOps.. %s:%d", __FILE__, __LINE__);
	return;
    }
    UInt16 cnt = 0;
    if(CountDragItemFlavors(theDrag, ref, &cnt) || !cnt) 
	return;

    FlavorType info;
    Size flavorsize=0;
    if(GetFlavorDataSize(current_dropobj, ref, kDragQtGeneratedMarker, &flavorsize)) { //Mac style
	for(UInt16 i = 1; i <= cnt; i++) {
	    if(GetFlavorType(current_dropobj, ref, i, &info) == noErr) {
		const char *match = NULL;
		for(int sm = 0; drag_map[sm].qt_type; sm++) {
		    if(info == drag_map[sm].mac_type) {
			match = drag_map[sm].qt_type;
			break;
		    }
		}
		qDebug("Qt: internal: 1) %c%c%c%c -- %s", 
		       char(info >> 24), char((info >> 16) & 255), 
		       char((info >> 8) & 255), char(info & 255),
		       match ? match : "----");
	    }
	}
    } else {
	int buff_size = 0;
	char *buffer = NULL;
	Size typesize = 0, realsize = sizeof(typesize);
	realsize = sizeof(typesize);
	for(UInt16 i = 1; i <= cnt; i++) {
	    if(GetFlavorType(current_dropobj, ref, i, &info) == noErr) {
		if((info >> 16) == ('QTxx' >> 16)) {
		    if(GetFlavorDataSize(current_dropobj, ref, info, &flavorsize) || flavorsize < 4) {
			qDebug("Qt: internal: Failure to get ScrapFlavorSize for %s:%d %d %d", __FILE__, __LINE__, 
			       (int)flavorsize, (int)info);
			return;
		    }
		    GetFlavorData(current_dropobj, ref, info, &typesize, &realsize, 0);
		    if(!buffer || buff_size < typesize + 1) {
			buff_size = typesize + 1;
			if(buffer)
			    buffer = (char *)realloc(buffer, buff_size);
			else
			    buffer = (char *)malloc(buff_size);
		    }
		    GetFlavorData(current_dropobj, ref, info, buffer, &typesize, sizeof(typesize));
		    if(typesize < 0) {
			qDebug("Qt: internal: typesize negative %s:%d", __FILE__, __LINE__);
			continue;
		    }
		    *(buffer + typesize) = '\0';
		    FlavorType match=0;
		    for(int sm = 0; drag_map[sm].qt_type; sm++) {
			if(!qstrncmp(drag_map[sm].qt_type, buffer, typesize)) {
			    match = drag_map[sm].mac_type;
			    break;
			}
		    }
		    qDebug("Qt: internal: 2) %c%c%c%c -- %s", 
			   match ? char(match >> 24) : '-', match ? char((match >> 16) & 255) : '-',
			   match ? char((match >> 8) & 255) : '-', match ? char(match  & 255) : '-',
			   buffer);
		}
	    }
	}
	if(buffer)
	    free(buffer);
    }
    }
    qDebug("Qt: internal: -------------------------------------------------------");
}
#else
#define debug_drag_flav(x, y)
#endif

QByteArray QDropEvent::encodedData(const char *fmt) const
{
    QByteArray ret;
    char *buffer = NULL;
    FlavorType info = NULL;
    Size flavorsize=0;
    ItemReference ref = NULL;

    if(GetDragItemReferenceNumber(current_dropobj, 1, &ref)) {
	qDebug("Qt: internal: OOps.. %s:%d", __FILE__, __LINE__);
	return 0;
    }
    UInt16 cnt = 0;
    if(CountDragItemFlavors(current_dropobj, ref, &cnt) || !cnt) 
	return 0;

    if(GetFlavorDataSize(current_dropobj, ref, kDragQtGeneratedMarker, &flavorsize)) { //Mac style
	for(UInt16 i = 1; i <= cnt; i++) {
	    if(GetFlavorType(current_dropobj, ref, i, &info)) {
		qDebug("Qt: internal: OOps.. %s:%d", __FILE__, __LINE__);
		return 0;
	    }
	    for(int sm = 0; drag_map[sm].qt_type; sm++) {
		if(info == drag_map[sm].mac_type && !qstrcmp(fmt, drag_map[sm].qt_type)) {
		    if(GetFlavorDataSize(current_dropobj, ref, info, &flavorsize)) {
			qDebug("Qt: internal: Failure to get GetFlavorDataSize for %d", (int)info);
			return 0;
		    }
		    if(info == kScrapFlavorTypeUnicode || info == kScrapFlavorTypeText) {
			buffer = (char *)malloc(flavorsize);
			GetFlavorData(current_dropobj, ref, info, buffer, &flavorsize, 0);
			ret.assign(buffer, flavorsize);
			return ret;
		    } else if(info == kDragFlavorTypeHFS) {
			UInt16 cnt_items;
			CountDragItems(current_dropobj, &cnt_items);
			for(int i = 1, done = 0; i <= cnt_items; i++) {
			    if(i > 1) {
				if(GetDragItemReferenceNumber(current_dropobj, i, &ref)) {
				    qDebug("Qt: internal: OOps.. %s:%d", __FILE__, __LINE__);
				    return 0;
				}
				if(GetFlavorDataSize(current_dropobj, ref, info, &flavorsize)) {
				    qDebug("Qt: internal: Failure to get GetFlavorDataSize for %d", (int)info);
				    return 0;
				}
			    }
			    if(flavorsize != sizeof(HFSFlavor)) {
				qDebug("Qt: internal: %s:%d Unexpected case in HFS Flavor", __FILE__, __LINE__);
				continue;
			    }
			    FSRef fsref;
			    HFSFlavor hfs;
			    GetFlavorData(current_dropobj, ref, info, &hfs, &flavorsize, 0);
			    FSpMakeFSRef(&hfs.fileSpec, &fsref);
			    buffer = (char *)malloc(1024);
			    FSRefMakePath(&fsref, (UInt8 *)buffer, 1024);
			    QCString s = QUriDrag::localFileToUri(QString::fromUtf8(
				(const char *)buffer));
			    free(buffer);
			    buffer = NULL;
			    //now encode them to be handled by quridrag
			    int l = qstrlen(s);
			    ret.resize(ret.size()+(l+2));
			    memcpy(ret.data()+done,s,l);
			    memcpy(ret.data()+l+done,"\r\n",2);
			    done += l + 2;
			}
			return ret;
		    } else if(info == typeFileURL) {
			UInt16 cnt_items;
			CountDragItems(current_dropobj, &cnt_items);
			QCString qstr;
			for(int i = 1, done = 0, buffer_size = 0; i <= cnt_items; i++) {
			    if(i > 1) {
				if(GetDragItemReferenceNumber(current_dropobj, i, &ref)) {
				    qDebug("Qt: internal: OOps.. %s:%d", __FILE__, __LINE__);
				    return 0;
				}
				if(GetFlavorDataSize(current_dropobj, ref, info, &flavorsize)) {
				    qDebug("Qt: internal: Failure to get GetFlavorDataSize for %d", (int)info);
				    return 0;
				}
			    }
			    if(buffer_size < flavorsize) {
				buffer_size = flavorsize;
				if(buffer)
				    buffer = (char*)realloc(buffer, buffer_size);
				else
				    buffer = (char*)malloc(buffer_size);
			    }
			    GetFlavorData(current_dropobj, ref, info, buffer, &flavorsize, 0);
			    QCString tmp_str(QString::fromUtf8((const char *)buffer, flavorsize));
			    if(tmp_str.left(17) == "file://localhost/") //mac encodes a differently
				tmp_str = "file:///" + tmp_str.mid(17);
			    int l = tmp_str.length();
			    ret.resize(ret.size()+(l+2));
			    memcpy(ret.data()+done,tmp_str.data(),l);
			    memcpy(ret.data()+l+done,"\r\n",2);
			    done += l + 2;
			}
			if(buffer)
			    free(buffer);
			return qstr;
		    }
		}
	    }
	}
    } else { //Qt style drags
	for(UInt16 i = 1; i <= cnt; i++) {
	    if(GetFlavorType(current_dropobj, ref, i, &info)) {
		qDebug("Qt: internal: OOps.. %s:%d", __FILE__, __LINE__);
		return 0;
	    }
	    if((info >> 16) == ('QTxx' >> 16)) {
		if(GetFlavorDataSize(current_dropobj, ref, info, &flavorsize) || flavorsize < 4) {
		    qDebug("Qt: internal: Failure to get ScrapFlavorSize for %s:%d %d %d", __FILE__, __LINE__, 
			    (int)flavorsize, (int)info);
		    return 0;
		}
		buffer = (char *)malloc(flavorsize);
		GetFlavorData(current_dropobj, ref, info, buffer, &flavorsize, 0);
		UInt32 mimesz;
		memcpy(&mimesz, buffer, sizeof(mimesz));
		if(!qstrnicmp(buffer+sizeof(mimesz), fmt, mimesz)) {
		    int size = flavorsize - (mimesz + sizeof(mimesz));
		    memcpy(buffer, buffer + mimesz + sizeof(mimesz), size);
		    ret.assign(buffer, size);
		    return ret;
		}
		free(buffer);
		buffer = NULL;
	    }
	}
    }
    return 0;
}

const char* QDropEvent::format(int i) const
{
    FlavorType info = NULL;
    Size flavorsize = 0, typesize = 0, realsize = sizeof(typesize);
    ItemReference ref = NULL;
    unsigned short numFlavors;
    
    if(GetDragItemReferenceNumber(current_dropobj, 1, &ref)) {
	qDebug("Qt: internal: OOps.. %s:%d", __FILE__, __LINE__);
	return 0;
    }

    if(CountDragItemFlavors(current_dropobj, ref, &numFlavors)) {
	qDebug("Qt: internal: OOps.. %s:%d", __FILE__, __LINE__);
	return 0;
    }
    if(i >= numFlavors)
	return 0;


    if(GetFlavorDataSize(current_dropobj, ref, kDragQtGeneratedMarker, &flavorsize)) { //Mac style
	for(int x = 1, found = 0; x <= (int)numFlavors; x++) {
	    if(GetFlavorType(current_dropobj, ref, x, &info)) {
		qDebug("Qt: internal:OOps.. %d %s:%d", i, __FILE__, __LINE__);
		return 0;
	    }
	    for(int sm = 0; drag_map[sm].qt_type; sm++) {
		if(info == drag_map[sm].mac_type) {
		    if(found++ != i) {
#ifdef DEBUG_MAPPINGS
			qDebug("Qt: internal: QDropEvent::format(%d): %s %s", i, drag_map[sm].mac_type_name,
			       drag_map[sm].qt_type);
#endif
			return drag_map[sm].qt_type;
		    }
		}
	    }
	}
    } else {
	for(int x = 1, found = 0; x <= (int)numFlavors; x++) {
	    if(GetFlavorType(current_dropobj, ref, x, &info)) {
		qDebug("Qt: internal: OOps.. %d %s:%d", i, __FILE__, __LINE__);
		continue;
	    }
	    if((info >> 16) == ('QTxx' >> 16)) {
		if(found++ != i)
		    continue;
		if(GetFlavorDataSize(current_dropobj, ref, info, &flavorsize) || flavorsize < 4) {
		    qDebug("Qt: internal: Failure to get ScrapFlavorSize for %s:%d %d %d", __FILE__, __LINE__, 
			    (int)flavorsize, (int)info);
		    return 0;
		}
		GetFlavorData(current_dropobj, ref, info, &typesize, &realsize, 0);

		static char *buffer = NULL;
		static int buffer_len = 0;
		if(!buffer)
		    buffer = (char *)malloc((buffer_len = (typesize + 1)));
		else if(typesize + 1 > buffer_len) 
		    buffer = (char *)realloc(buffer, (buffer_len = (typesize + 1)));
		GetFlavorData(current_dropobj, ref, info, buffer, &typesize, sizeof(typesize));
		if(typesize < 0) {
		    qDebug("Qt: internal: typesize negative %s:%d", __FILE__, __LINE__);
		    continue;
		}
		*(buffer + typesize) = '\0';
#ifdef DEBUG_MAPPINGS
		qDebug("Qt: internal: QDropEvent::format(%d): %s (%c%c%c%c) [from Qt app]", i, buffer, 
		       char(info >> 24), char((info >> 16) & 255), char((info >> 8) & 255), char(info & 255));
#endif
		return buffer;
	    }
	}
    }
    return NULL;
}

void QDragManager::timerEvent(QTimerEvent*)
{
}

bool QDragManager::eventFilter(QObject *, QEvent *)
{
    return FALSE;
}

void QDragManager::updateMode(ButtonState)
{
}

void QDragManager::updateCursor()
{
}

void QDragManager::cancel(bool)
{
    if(object) {
	beingCancelled = TRUE;
	object = 0;
    }
}

void QDragManager::move(const QPoint &)
{
}

void QDragManager::drop()
{
}

bool QDragManager::drag(QDragObject *o, QDragObject::DragMode mode)
{
    if(qt_mac_in_drag) {     //just make sure..
	qDebug("Qt: internal: WH0A, unexpected condition reached.");
	return FALSE;
    }
    if(object == o)
	return FALSE;
#ifndef MACOSX_101
    /* At the moment it seems clear that Mac OS X does not want to drag with a non-left button
       so we just bail early to prevent it, however we need to find a better solution! FIXME! */
    if(!(GetCurrentEventButtonState() & 0x1))
	return FALSE;
#endif

    if(object) {
	cancel();
	if(dragSource)
	    dragSource->removeEventFilter(this);
	beingCancelled = FALSE;
    }

    object = o;
    dragSource = (QWidget *)(object->parent());
    global_src = o;
    global_src->setTarget(0);

    OSErr result;
    DragReference theDrag;
    QByteArray ar;

    if((result = NewDrag(&theDrag))) {
	dragSource = 0;
	return(!result);
    }

    if(!noDropCursor) {
	noDropCursor = new QCursor(QCursor::ForbiddenCursor);
	if(!pm_cursor[0].isNull())
	    moveCursor = new QCursor(pm_cursor[0], 0,0);
	if(!pm_cursor[1].isNull())
	    copyCursor = new QCursor(pm_cursor[1], 0,0);
	if(!pm_cursor[2].isNull())
	    linkCursor = new QCursor(pm_cursor[2], 0,0);
	qAddPostRoutine(qt_mac_dnd_cleanup);
    }

    FlavorType mactype;
    const char *fmt;
    AddDragItemFlavor(theDrag, (ItemReference)1, kDragQtGeneratedMarker, &fmt, 1, 0); //mark of McQt
    for (int i = 0; (fmt = o->format(i)); i++) {
	for(int sm = 0; drag_map[sm].qt_type; sm++) { //encode it for other Mac applications
	    if(!qstrcmp(fmt, drag_map[sm].qt_type)) {
		if((drag_map[sm].mac_type == kScrapFlavorTypeUnicode) || 
		   (drag_map[sm].mac_type == kScrapFlavorTypeText)) {
		    ar = o->encodedData(drag_map[sm].qt_type);
		    AddDragItemFlavor(theDrag, (ItemReference)1, drag_map[sm].mac_type, 
				       ar.data(), ar.size(), 0);
		} else if(drag_map[sm].mac_type == kDragFlavorTypeHFS) { //not tested!!
		    HFSFlavor hfs;
		    hfs.fileType = 'TEXT';
		    hfs.fileCreator = 'CUTE';
		    hfs.fdFlags = 0;
		    ar = o->encodedData(drag_map[sm].qt_type);
		    FSpLocationFromFullPath(ar.size(), ar.data(), &hfs.fileSpec);
		    AddDragItemFlavor(theDrag, (ItemReference)1, drag_map[sm].mac_type, 
				       &hfs, sizeof(hfs), 0);
		} else if(drag_map[sm].mac_type == typeFileURL) {
		    ar = o->encodedData(drag_map[sm].qt_type);
		    uint len = 0;
		    char *buffer = (char *)malloc(ar.size());
		    for(uint i = 0; i < ar.size(); i++) {
			if(ar[i] == '\r' && i < ar.size()-1 && ar[i+1] == '\n')
			    break;
			buffer[len++] = ar[i];
		    }
		    if(!qstrncmp(buffer, "file:///", 8)) { //Mac likes localhost to be in it!
			if(len + 9 > ar.size())
			    buffer = (char *)realloc(buffer, len + 9);
			qstrncpy(buffer + 7, buffer + 9 + 7, len - 7);
			qstrncpy(buffer + 7, "localhost", 9);
			len += 9;
		    }
		    AddDragItemFlavor(theDrag, (ItemReference)1, drag_map[sm].mac_type, 
				       buffer, len, 0);
		    free(buffer);
		}
		break;
	    }
	}
	//encode it the Qt/Mac way also
	ar = o->encodedData(fmt);
	mactype = ('Q' << 24) | ('T' << 16) | (i & 0xFFFF);
	UInt32 mimelen = strlen(fmt);
	char *buffer = (char *)malloc(ar.size() + mimelen + sizeof(mimelen));
	memcpy(buffer, &mimelen, sizeof(mimelen));
	memcpy(buffer+sizeof(mimelen), fmt, mimelen);
	memcpy(buffer+sizeof(mimelen) + mimelen, ar.data(), ar.size());
	AddDragItemFlavor(theDrag, (ItemReference)1, mactype, buffer, 
			   ar.size()+mimelen+sizeof(mimelen), 0);
	free(buffer);
    }
    debug_drag_flav(theDrag, "QDrag::tracking");

    //so we must fake an event
    EventRecord fakeEvent;
    GetGlobalMouse(&(fakeEvent.where));
    fakeEvent.message = 0;
    fakeEvent.what = mouseDown;
    fakeEvent.when = EventTimeToTicks(GetCurrentEventTime());
    fakeEvent.modifiers = GetCurrentKeyModifiers();
#ifndef MACOSX_101
    if(GetCurrentEventButtonState() & 2)
	fakeEvent.modifiers |= controlKey;
#endif

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
    SetRect(&boundsRect, boundsPoint.h, boundsPoint.v, boundsPoint.h + pix.width(), boundsPoint.v + pix.height());
    SetDragItemBounds(theDrag, (ItemReference)1 , &boundsRect);

#if defined(Q_WS_MACX) 
    QRegion dragRegion(boundsPoint.h, boundsPoint.v, pix.width(), pix.height());
    QRegion r(0, 0, pix.width(), pix.height());
    SetDragImage(theDrag, GetGWorldPixMap((GWorldPtr)pix.handle()), r.handle(TRUE), boundsPoint, 0);
#else
    QBitmap pixbits;
    pixbits = pix;
    QRegion dragRegion(pixbits);
    dragRegion.translate(boundsPoint.h, boundsPoint.v);
#endif

    QWidget *widget = QApplication::widgetAt(fakeEvent.where.h, fakeEvent.where.v, TRUE);
    if(!widget) {
	dragSource = 0;
	return FALSE;
    }
    acceptfmt = FALSE;
    acceptact = FALSE;
    drag_received = FALSE;
    qt_mac_in_drag = TRUE;
    if(!widget->extraData()->macDndExtra) //never too late I suppose..
	qt_macdnd_register(widget,  widget->extraData());
    set_drag_mode = mode;
    updateDragMode(theDrag);
    //kick off the drag by calling the callback ourselves first..
    qt_mac_tracking_handler(kDragTrackingEnterWindow, (WindowPtr)widget->hd,
			     (void *)widget->extraData()->macDndExtra, theDrag);
    //now let the mac take control..
    {
	QMacBlockingFunction block;
	result = TrackDrag(theDrag, &fakeEvent, dragRegion.handle(TRUE)); 
    }
    DisposeDrag(theDrag);
    qt_mac_in_drag = FALSE;
    dragSource = 0;

    return ((result == noErr) && (current_drag_action == QDropEvent::Move) &&
	    !acceptact);
}

void QDragManager::updatePixmap()
{
}

static QMAC_PASCAL OSErr qt_mac_receive_handler(WindowPtr, void *handlerRefCon, DragReference theDrag)
{ 
    debug_drag_flav(theDrag, "QDrag::receive");
    updateDragMode(theDrag);
    QMacDndExtra *macDndExtra = (QMacDndExtra*) handlerRefCon;
    current_dropobj = theDrag;
    Point mouse;
    GetDragMouse(theDrag, &mouse, 0L);

    const QCursor *cursor = &Qt::arrowCursor; 
    if(qApp && qApp->overrideCursor()) {
	cursor = qApp->overrideCursor();
    } else if(QWidget *widget = QApplication::widgetAt(mouse.h, mouse.v, TRUE)) {
	for(QWidget *p = widget; p; p = p->parentWidget()) {
	    if(p->ownCursor()) {
		cursor = &(p->cursor());
		break;
	    }
	}
    }
    qt_mac_set_cursor(cursor, &mouse); 

    if(!current_drag_widget)
	return dragNotAcceptedErr;
    QDropEvent de(current_drag_widget->mapFromGlobal(QPoint(mouse.h, mouse.v)));
    de.setAction(current_drag_action);
    if(global_src)
	global_src->setTarget(current_drag_widget);
    QApplication::sendEvent(current_drag_widget, &de);
#ifdef DEBUG_DRAG_EVENTS
    qDebug("Qt: internal: Sending <DragDrop>(%d::%d::%d) event to %s %s", 
	   de.isAccepted(), de.isActionAccepted(), current_drag_action,
	   current_drag_widget ? current_drag_widget->className() : "Unknown",
	   current_drag_widget ? current_drag_widget->name() : "Unknown");
#endif
    macDndExtra->acceptact = de.isActionAccepted();
    macDndExtra->acceptfmt = de.isAccepted();
    acceptact = macDndExtra->acceptact;
    acceptfmt = macDndExtra->acceptfmt;
    drag_received = TRUE;
    current_drag_widget = NULL;
    return macDndExtra->acceptfmt ? (OSErr)noErr : (OSErr)dragNotAcceptedErr;
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
    qAddPostRoutine(cleanup_dnd_receiveUPP);
    return qt_mac_receive_handlerUPP = NewDragReceiveHandlerUPP(qt_mac_receive_handler);
}

static QMAC_PASCAL OSErr qt_mac_tracking_handler(DragTrackingMessage theMessage, WindowPtr,
						  void *handlerRefCon, DragReference theDrag)
{
    if(theMessage != kDragTrackingEnterWindow && theMessage != kDragTrackingLeaveWindow &&
       theMessage != kDragTrackingInWindow) {
	return noErr;
    } else if(!theDrag) {
	qDebug("Qt: internal: DragReference null %s %d", __FILE__, __LINE__);
	return 1;
    } else if(qt_mac_in_drag && drag_received) { //ignore these
	return noErr;
    }
    updateDragMode(theDrag);
    Point mouse;
    GetDragMouse(theDrag, &mouse, 0L);
    if(!mouse.h && !mouse.v)
	GetGlobalMouse(&mouse);
    QPoint globalMouse(mouse.h, mouse.v);
    QMacDndExtra *macDndExtra = (QMacDndExtra*) handlerRefCon;
    QWidget *widget = QApplication::widgetAt(globalMouse, TRUE);
    while(widget && (!widget->acceptDrops()))
	widget = widget->parentWidget(TRUE);
    //Dispatch events
    if(widget && theMessage == kDragTrackingInWindow && widget == current_drag_widget) {
        QDragMoveEvent de(widget->mapFromGlobal(globalMouse));
	de.setAction(current_drag_action);
	if(macDndExtra->acceptact)
	    de.acceptAction();
	if(macDndExtra->acceptfmt)
	    de.accept();
	QApplication::sendEvent(widget, &de);
#ifdef DEBUG_DRAG_EVENTS
	qDebug("Sending <DragMove>(%d::%d::%d) event to %s %s", 
	       de.isAccepted(), de.isActionAccepted(), current_drag_action,
	       widget ? widget->className() : "Unknown", widget ? widget->name() : "Unknown");
#endif
	macDndExtra->acceptfmt = de.isAccepted();
	macDndExtra->acceptact = de.isActionAccepted();
	acceptfmt = macDndExtra->acceptfmt;
	acceptact = macDndExtra->acceptact;
	if(!de.isAccepted()) 
	    current_drag_widget = NULL;
    } else { 
	if(current_drag_widget && ((theMessage == kDragTrackingLeaveWindow) || 
				     (widget != current_drag_widget))) {
	    macDndExtra->acceptfmt = FALSE;
	    acceptfmt = FALSE;
	    current_dropobj = 0;
	    QDragLeaveEvent de;
	    QApplication::sendEvent(current_drag_widget, &de);
#ifdef DEBUG_DRAG_EVENTS
	    qDebug("Sending <DragLeave> event to %s %s", 
		   current_drag_widget ? current_drag_widget->className() : "Unknown",
		   current_drag_widget ? current_drag_widget->name() : "Unknown");
#endif
	}
	current_drag_widget = 0;
	if(widget) {
	    current_dropobj = theDrag;
	    if(widget != current_drag_widget) {
		QDragEnterEvent de(widget->mapFromGlobal(globalMouse));
		de.setAction(current_drag_action);
		QApplication::sendEvent(widget, &de);
#ifdef DEBUG_DRAG_EVENTS
		qDebug("Sending <DragEnter>(%d::%d::%d) event to %s %s", 
		       de.isAccepted(), de.isActionAccepted(), current_drag_action,
		       widget ? widget->className() : "Unknown", widget ? widget->name() : "Unknown");
#endif
		macDndExtra->acceptfmt = de.isAccepted();
		macDndExtra->acceptact = de.isActionAccepted();
		acceptfmt = macDndExtra->acceptfmt;
		acceptact = macDndExtra->acceptact;
		if(de.isAccepted())
		    current_drag_widget = widget;
	    }
	} 
    }

    //set the cursor
    const QCursor *cursor = NULL;
    if(current_drag_widget && macDndExtra->acceptfmt) {
#ifndef QMAC_NO_FAKECURSOR
	if(current_drag_action == QDropEvent::Move)
	    cursor = moveCursor;
	else if(current_drag_action == QDropEvent::Copy)
	    cursor = copyCursor;
	else if(current_drag_action == QDropEvent::Link)
	    cursor = linkCursor;
#endif
    } else {
	cursor = noDropCursor;
    }
    if(!cursor) {
	if(qApp && qApp->overrideCursor()) {
	    cursor = qApp->overrideCursor();
	} else if(widget) {
	    for(QWidget *p = widget; p; p = p->parentWidget()) {
		if(p->ownCursor()) {
		    cursor = &(p->cursor());
		    break;
		}	
	    }
	}
    }
    if(!cursor) //fallback..
	cursor = &Qt::arrowCursor;
    qt_mac_set_cursor(cursor, &mouse); 

    //idle things
    if(qGlobalPostedEventsCount()) {
	QApplication::sendPostedEvents();
	QApplication::flush();
    }
    return noErr;
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
    qAddPostRoutine(cleanup_dnd_trackingUPP);
    return qt_mac_tracking_handlerUPP = NewDragTrackingHandlerUPP(qt_mac_tracking_handler);
}

void qt_macdnd_unregister(QWidget *widget, QWExtra *extra)
{
    if(extra && extra->macDndExtra  && !(--extra->macDndExtra->ref)) {
	if(qt_mac_tracking_handlerUPP)
	    RemoveTrackingHandler(make_trackingUPP(), (WindowPtr)widget->handle());
	if(qt_mac_receive_handlerUPP)
	    RemoveReceiveHandler(make_receiveUPP(), (WindowPtr)widget->handle());
	delete extra->macDndExtra;
	extra->macDndExtra = 0;
    }
}

void qt_macdnd_register(QWidget *widget, QWExtra *extra)
{
    if(!extra->macDndExtra) {
	extra->macDndExtra = new QMacDndExtra;
	extra->macDndExtra->ref = 1;
	InstallTrackingHandler(make_trackingUPP(),  (WindowPtr)widget->handle(),
				extra->macDndExtra);
	InstallReceiveHandler(make_receiveUPP(), (WindowPtr)widget->handle(),
			       extra->macDndExtra);
    } else {	
	extra->macDndExtra->ref++;
    }
}

#endif // QT_NO_DRAGANDDROP
