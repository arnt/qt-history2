/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qapplication.h"

#ifndef QT_NO_DRAGANDDROP
#include "qwidget.h"
#include "qintdict.h"
#include "qdatetime.h"
#include "qdict.h"
#include "qdragobject.h"
#include "qbitmap.h"
#include "qt_mac.h"
#include "qpainter.h"
#include "qcursor.h"
#include "qevent.h"
#include <stdlib.h>
#include <string.h>

#define d d_func()
#define q q_func()

/*****************************************************************************
  QDnD debug facilities
 *****************************************************************************/
//#define DEBUG_DRAG_EVENTS

/*****************************************************************************
  QDnD globals
 *****************************************************************************/
bool qt_mac_in_drag = false;
static bool drag_received = false;
static QDragObject::DragMode set_drag_mode; //passed in drag mode
static QDropEvent::Action current_drag_action; //current active drag action
static QDragObject *global_src = 0;
static DragReference current_dropobj = 0;
static bool acceptfmt = false;
static bool acceptact = false;


/*****************************************************************************
  Externals
 *****************************************************************************/
extern uint qGlobalPostedEventsCount(); //qapplication.cpp

/*****************************************************************************
  QDnD utility functions
 *****************************************************************************/
//promise keeper
static DragSendDataUPP qt_mac_send_handlerUPP = NULL;
static QMAC_PASCAL OSErr qt_mac_send_handler(FlavorType flav, void *handlerRefCon, DragItemRef, DragRef theDrag)
{
    QDragObject *o = (QDragObject*)handlerRefCon;
    QMacMime::QMacMimeType qmt = QMacMime::MIME_DND;
    {
        ItemReference ref = NULL;
        if(GetDragItemReferenceNumber(theDrag, 1, &ref) == noErr) {
            Size sz;
            extern ScrapFlavorType qt_mac_mime_type; //qmime_mac.cpp
            if(GetFlavorDataSize(theDrag, ref, qt_mac_mime_type, &sz) == noErr)
                qmt = QMacMime::MIME_QT_CONVERTOR;
        }
    }
    while(1) {
        QList<QMacMime*> all = QMacMime::all(qmt);
        for(QList<QMacMime *>::Iterator it = all.begin(); it != all.end(); ++it) {
            QMacMime *c = (*it);
            if(const char *mime = c->mimeFor(flav)) {
                QList<QByteArray> md = c->convertFromMime(o->encodedData(mime), mime, flav);
                int item_ref = 1;
                for(QList<QByteArray>::Iterator it = md.begin(); it != md.end(); ++it)
                    SetDragItemFlavorData(theDrag, (ItemReference)item_ref++, flav, (*it).data(), (*it).size(), 0);
                return noErr;
            }
        }
        if(qmt == QMacMime::MIME_DND)
            break;
        qmt = QMacMime::MIME_DND; //now just try anything..
    }
    return cantGetFlavorErr;
}
static void cleanup_dnd_sendUPP()
 {
    if(qt_mac_send_handlerUPP) {
        DisposeDragSendDataUPP(qt_mac_send_handlerUPP);
        qt_mac_send_handlerUPP = NULL;
    }
}
static const DragSendDataUPP make_sendUPP()
{
    if(qt_mac_send_handlerUPP)
        return qt_mac_send_handlerUPP;
    qAddPostRoutine(cleanup_dnd_sendUPP);
    return qt_mac_send_handlerUPP = NewDragSendDataUPP(qt_mac_send_handler);
}

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

//used to set the dag state
void updateDragMode(DragReference drag) {
    if(set_drag_mode == QDragObject::DragDefault) {
        SInt16 mod;
        GetDragModifiers(drag, &mod, NULL, NULL);
        if((mod & optionKey) || (mod & rightOptionKey)) {
//            SetDragAllowableActions(drag, kDragActionCopy, false);
            current_drag_action = QDropEvent::Copy;
        } else {
//            SetDragAllowableActions(drag, kDragActionMove, false);
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

struct QMacDndExtra {
    QWidget *widget;
    bool acceptfmt;
    bool acceptact;
    int ref;
};

/*****************************************************************************
  DnD functions
 *****************************************************************************/
bool QDropEvent::provides(const char *mime) const
{
    ItemReference ref = NULL;
    if(GetDragItemReferenceNumber(current_dropobj, 1, &ref))
        return false;

    UInt16 cnt = 0;
    if(CountDragItemFlavors(current_dropobj, ref, &cnt))
        return false;

    FlavorType flav;
    QMacMime::QMacMimeType qmt = QMacMime::MIME_DND;
    Size sz;
    extern ScrapFlavorType qt_mac_mime_type; //qmime_mac.cpp
    if(GetFlavorDataSize(current_dropobj, ref, qt_mac_mime_type, &sz) == noErr)
        qmt = QMacMime::MIME_QT_CONVERTOR;
    for(int x = 1; x <= (int)cnt; x++) {
        if(GetFlavorType(current_dropobj, ref, x, &flav) == noErr) {
            if(QMacMime::convertor(qmt, mime, flav))
                return true;
        }
    }
    return false;
}

QByteArray QDropEvent::encodedData(const char *mime) const
{
    Size flavorsize=0;
    QMacMime::QMacMimeType qmt = QMacMime::MIME_DND;
    {
        ItemReference ref = NULL;
        if(GetDragItemReferenceNumber(current_dropobj, 1, &ref) == noErr) {
            Size sz;
            extern ScrapFlavorType qt_mac_mime_type; //qmime_mac.cpp
            if(GetFlavorDataSize(current_dropobj, ref, qt_mac_mime_type, &sz) == noErr)
                qmt = QMacMime::MIME_QT_CONVERTOR;
        }
    }
    QList<QMacMime*> all = QMacMime::all(qmt);
    for(QList<QMacMime *>::Iterator it = all.begin(); it != all.end(); ++it) {
        QMacMime *c = (*it);
        int flav = c->flavorFor(mime);
        if(flav) {
            UInt16 cnt_items;
            CountDragItems(current_dropobj, &cnt_items);
            QList<QByteArray> arrs;
            for(int i = 1; i <= cnt_items; i++) {
                ItemReference ref = NULL;
                if(GetDragItemReferenceNumber(current_dropobj, i, &ref)) {
                    qDebug("Qt: internal: OOps.. %s:%d", __FILE__, __LINE__);
                    return QByteArray();
                }
                if(GetFlavorDataSize(current_dropobj, ref, flav, &flavorsize) == noErr) {
                    char *buffer = static_cast<char *>(malloc(flavorsize));
                    GetFlavorData(current_dropobj, ref, flav, buffer, &flavorsize, 0);
                    arrs.append(QByteArray::fromRawData(buffer, flavorsize));
                }
            }
            if(!arrs.isEmpty())
                return c->convertToMime(arrs, mime, flav);
        }
    }
    return QByteArray();
}

const char* QDropEvent::format(int n) const
{
    const char* mime = NULL;
    if(n >= 0) {
        ItemReference ref = NULL;
        if(GetDragItemReferenceNumber(current_dropobj, 1, &ref))
            return NULL;

        UInt16 cnt = 0;
        if(CountDragItemFlavors(current_dropobj, ref, &cnt))
            return NULL;
        if(n >= cnt)
            return 0;

        FlavorType flav;
        bool sawSBText = false;
        QMacMime::QMacMimeType qmt = QMacMime::MIME_DND;
        {
            Size sz;
            extern ScrapFlavorType qt_mac_mime_type; //qmime_mac.cpp
            if(GetFlavorDataSize(current_dropobj, ref, qt_mac_mime_type, &sz) == noErr)
                qmt = QMacMime::MIME_QT_CONVERTOR;
        }
        for(int x = 1; x <= (int)cnt; x++) {
            if(GetFlavorType(current_dropobj, ref, x, &flav) == noErr) {
                if(flav == kScrapFlavorTypeText) {
                    sawSBText = true;
                } else if(const char *m = QMacMime::flavorToMime(qmt, flav)) {
                    if(!n) {
                        mime = m;
                        break;
                    }
                    n--;
                }
            }
            if(!mime && sawSBText && !n)
                mime = QMacMime::flavorToMime(qmt, kScrapFlavorTypeText);
        }
    }
    return n ? NULL : mime;
}

void QDragManager::timerEvent(QTimerEvent*)
{
}

bool QDragManager::eventFilter(QObject *, QEvent *)
{
    return false;
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
        beingCancelled = true;
        object = 0;
    }
}

void QDragManager::move(const QPoint &)
{
}

void QDragManager::drop()
{
}


bool QWidgetPrivate::qt_mac_dnd_event(uint kind, DragRef dragRef)
{
    current_dropobj = dragRef;
    updateDragMode(dragRef);

    Point mouse;
    GetDragMouse(dragRef, &mouse, 0L);
    if(!mouse.h && !mouse.v)
        GetGlobalMouse(&mouse);

    //Dispatch events
    bool ret = true;
    if(kind == kEventControlDragWithin) {
        QDragMoveEvent de(q->mapFromGlobal(QPoint(mouse.h, mouse.v)));
        de.setAction(current_drag_action);
        QApplication::sendEvent(q, &de);
    } else if(kind == kEventControlDragEnter) {
        QDragEnterEvent de(q->mapFromGlobal(QPoint(mouse.h, mouse.v)));
        de.setAction(current_drag_action);
        QApplication::sendEvent(q, &de);
        if(!de.isAccepted())
            ret = false;
    } else if(kind == kEventControlDragLeave) {
        QDragLeaveEvent de;
        QApplication::sendEvent(q, &de);
    } else if(kind == kEventControlDragReceive) {
        QDropEvent de(q->mapFromGlobal(QPoint(mouse.h, mouse.v)));
        de.setAction(current_drag_action);
        if(global_src)
            global_src->setTarget(q);
        QApplication::sendEvent(q, &de);
        if(!de.isAccepted())
            ret = false;
    } else {
        return false;
    }

#ifdef DEBUG_DRAG_EVENTS
    {
        const char *desc = 0;
        switch(kind) {
        case kEventControlDragWithin: desc = "DragMove"; break;
        case kEventControlDragEnter: desc = "DragEnter"; break;
        case kEventControlDragLeave: desc = "DragLeave"; break;
        case kEventControlDragReceive: desc = "DragDrop"; break;
        }
        if(desc) {
            QPoint pos(q->mapFromGlobal(QPoint(mouse.h, mouse.v)));
            qDebug("Sending <%s>(%d, %d) event to %s %s [%d]",
                   desc, pos.x(), pos.y(), q->className(), q->objectName().local8Bit(), ret);
        }
    }
#endif

    //set the cursor
    const QCursor *dnd_cursor = NULL;
    if(kind == kEventControlDragWithin || kind == kEventControlDragEnter) {
        if(ret) {
            if(current_drag_action == QDropEvent::Move)
                dnd_cursor = moveCursor;
            else if(current_drag_action == QDropEvent::Copy)
                dnd_cursor = copyCursor;
            else if(current_drag_action == QDropEvent::Link)
                dnd_cursor = linkCursor;
        } else {
            dnd_cursor = noDropCursor;
        }
    }
    QCursor cursor(Qt::ArrowCursor);
    if(!dnd_cursor) {
        if(qApp && qApp->overrideCursor()) {
            cursor = *qApp->overrideCursor();
        } else if(q) {
            for(QWidget *p = q; p; p = p->parentWidget()) {
                if(p->testAttribute(Qt::WA_SetCursor)) {
                    cursor = p->cursor();
                    break;
                }
            }
        }
    } else {
        cursor = *dnd_cursor;
    }
    qt_mac_set_cursor(&cursor, &mouse);

    //idle things
    if(qGlobalPostedEventsCount()) {
        QApplication::sendPostedEvents();
        QApplication::flush();
    }
    return ret;
}

bool QDragManager::drag(QDragObject *o, QDragObject::DragMode mode)
{
    if(qt_mac_in_drag) {     //just make sure..
        qDebug("Qt: internal: WH0A, unexpected condition reached.");
        return false;
    }
    if(object == o)
        return false;
    /* At the moment it seems clear that Mac OS X does not want to drag with a non-left button
       so we just bail early to prevent it, however we need to find a better solution! FIXME! */
    if(!(GetCurrentEventButtonState() & kEventMouseButtonPrimary))
        return false;

    if(object) {
        cancel();
        if(dragSource)
            dragSource->removeEventFilter(this);
        beingCancelled = false;
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
    SetDragSendProc(theDrag, make_sendUPP(), o); //fullfills the promise!

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

    const char* mime;
    QList<QMacMime*> all = QMacMime::all(QMacMime::MIME_DND);
    for (int i = 0; (mime = o->format(i)); i++) {
        for(QList<QMacMime *>::Iterator it = all.begin(); it != all.end(); ++it) {
            QMacMime *c = (*it);
            if(c->flavorFor(mime)) {
                for (int j = 0; j < c->countFlavors(); j++) {
                    uint flav = c->flavor(j);
                    if(c->canConvert(mime, flav))
                        AddDragItemFlavor(theDrag, 1, flav, NULL, 0, 0); //promised for later
                }
            }
        }
    }
    {
        char t = 123;
        extern ScrapFlavorType qt_mac_mime_type; //qmime_mac.cpp
        AddDragItemFlavor(theDrag, 1, qt_mac_mime_type, &t, 1, 0);
    }

    //so we must fake an event
    EventRecord fakeEvent;
    GetGlobalMouse(&(fakeEvent.where));
    fakeEvent.message = 0;
    fakeEvent.what = mouseDown;
    fakeEvent.when = EventTimeToTicks(GetCurrentEventTime());
    fakeEvent.modifiers = GetCurrentKeyModifiers();
    if(GetCurrentEventButtonState() & 2)
        fakeEvent.modifiers |= controlKey;

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
            if(!s.isEmpty()) {
                //draw it
                QFont f(qApp->font());
                f.setPointSize(12);
                QFontMetrics fm(f);
                QPixmap tmp(fm.width(s), fm.height());
                if(!tmp.isNull()) {
                    QPainter p(&tmp);
                    p.fillRect(0, 0, tmp.width(), tmp.height(), color0);
                    p.setPen(color1);
                    p.setFont(f);
                    p.drawText(0, fm.ascent(), s);
                    //save it
                    pix = tmp;
                    hotspot = QPoint(tmp.width() / 2, tmp.height() / 2);
                }
            }
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

    QRegion dragRegion(boundsPoint.h, boundsPoint.v, pix.width(), pix.height()), pixRegion;
    if(!pix.isNull()) {
        pixRegion = QRegion(0, 0, pix.width(), pix.height());
        SetDragImage(theDrag, GetGWorldPixMap((GWorldPtr)pix.handle()), pixRegion.handle(true), boundsPoint, 0);
    }

    QWidget *widget = QApplication::widgetAt(fakeEvent.where.h, fakeEvent.where.v);
    if(!widget) {
        dragSource = 0;
        return false;
    }
    acceptfmt = false;
    acceptact = false;
    drag_received = false;
    qt_mac_in_drag = true;
    set_drag_mode = mode;
    updateDragMode(theDrag);
    {
        QMacBlockingFunction block;
        result = TrackDrag(theDrag, &fakeEvent, dragRegion.handle(true));
    }
    DisposeDrag(theDrag);
    qt_mac_in_drag = false;
    dragSource = 0;

    return ((result == noErr) && (current_drag_action == QDropEvent::Move) &&
            !acceptact);
}

void QDragManager::updatePixmap()
{
}



#endif // QT_NO_DRAGANDDROP
