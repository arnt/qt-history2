/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qapplication.h"

#ifndef QT_NO_DRAGANDDROP
#include "qwidget.h"
#include "qdatetime.h"
#include "qdragobject.h"
#include "qbitmap.h"
#include "qpainter.h"
#include "qcursor.h"
#include "qevent.h"
#include <stdlib.h>
#include <string.h>
#ifndef QT_NO_ACCESSIBILITY
# include "qaccessible.h"
#endif

#include <private/qdnd_p.h>
#include <private/qt_mac_p.h>

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
static QDrag::DragOperation current_drag_operation; //current active drag action
static QDragPrivate *global_src = 0;
static DragReference current_dropobj = 0;
static QDrag::DragOperations accept_operations; 

/*****************************************************************************
  Externals
 *****************************************************************************/
extern bool qt_modal_state(); //qapplication_mac.cpp
extern uint qGlobalPostedEventsCount(); //qapplication.cpp

/*****************************************************************************
  QDnD utility functions
 *****************************************************************************/
//promise keeper
static DragSendDataUPP qt_mac_send_handlerUPP = NULL;
static OSErr qt_mac_send_handler(FlavorType flav, void *handlerRefCon, DragItemRef, DragRef theDrag)
{
    QDragPrivate *o = (QDragPrivate *)handlerRefCon;
    QMacMime::QMacMimeType qmt = QMacMime::MIME_DND;
    {
        ItemReference ref = 0;
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
            QString mime = c->mimeFor(flav);
            if(!mime.isNull()) {
                QList<QByteArray> md = c->convertFromMime(o->data->data(mime), mime, flav);
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

//used to set the dag state
void updateDragMode(DragReference drag) {
    SInt16 mod;
    GetDragModifiers(drag, &mod, NULL, NULL);
    QDrag::DragOperation op = QDrag::NoDrag;
    if((mod & (optionKey|cmdKey)) == (optionKey|cmdKey)) 
        op = QDrag::LinkDrag;
    else if((mod & optionKey) || (mod & rightOptionKey))
        op = QDrag::CopyDrag;
    else 
        op = QDrag::MoveDrag;
    if(!(accept_operations & op))
        current_drag_operation = op;
}

/*****************************************************************************
  DnD functions
 *****************************************************************************/
bool QDropData::hasFormat(const QString &mime) const
{
    ItemReference ref = 0;
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

QVariant QDropData::retrieveData(const QString &mime, QVariant::Type) const
{
    Size flavorsize=0;
    QMacMime::QMacMimeType qmt = QMacMime::MIME_DND;
    {
        ItemReference ref = 0;
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
                ItemReference ref = 0;
                if(GetDragItemReferenceNumber(current_dropobj, i, &ref)) {
                    qWarning("Qt: internal: OOps.. %s:%d", __FILE__, __LINE__);
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

QStringList QDropData::formats() const
{
    QStringList ret;

    ItemReference ref = 0;
    if(GetDragItemReferenceNumber(current_dropobj, 1, &ref)) 
        return ret;

    UInt16 cnt = 0;
    if(CountDragItemFlavors(current_dropobj, ref, &cnt))
        return ret;

    FlavorType flav;
    QMacMime::QMacMimeType qmt = QMacMime::MIME_DND;
    {
        Size sz;
        extern ScrapFlavorType qt_mac_mime_type; //qmime_mac.cpp
        if(GetFlavorDataSize(current_dropobj, ref, qt_mac_mime_type, &sz) == noErr)
            qmt = QMacMime::MIME_QT_CONVERTOR;
    }
    for(int x = 1; x <= (int)cnt; x++) {
        if(GetFlavorType(current_dropobj, ref, x, &flav) == noErr) {
            QString mime = QMacMime::flavorToMime(qmt, flav);
            if(!mime.isNull()) 
                ret.append(mime);
        }
    }
    return ret;
}

void QDragManager::timerEvent(QTimerEvent*)
{
}

bool QDragManager::eventFilter(QObject *, QEvent *)
{
    return false;
}

void QDragManager::updateMode(Qt::KeyboardModifiers)
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
#ifndef QT_NO_ACCESSIBILITY
    QAccessible::updateAccessibility(this, 0, QAccessible::DragDropEnd);
#endif
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

    if(qt_modal_state()) {
        for(QWidget *modal = q; modal; modal = modal->parentWidget()) {
            if(modal->isTopLevel()) {
                if(modal != QApplication::activeModalWidget())
                    return noErr;
                break;
            }
        }
    }

    //Dispatch events
    QDropEvent::Action event_action = QDropEvent::Copy;
    if(current_drag_operation == QDrag::CopyDrag)
        event_action = QDropEvent::Copy;
    else if(current_drag_operation == QDrag::LinkDrag)
        event_action = QDropEvent::Link;
    else if(current_drag_operation == QDrag::MoveDrag)
        event_action = QDropEvent::Move;
    bool ret = true;
    if(kind == kEventControlDragWithin) {
        QDragMoveEvent de(q->mapFromGlobal(QPoint(mouse.h, mouse.v)), 
                          QDragManager::self()->dropData);
        de.setAction(event_action);
        de.accept(true);
        de.acceptAction(true);
        QApplication::sendEvent(q, &de);
    } else if(kind == kEventControlDragEnter) {
        QDragEnterEvent de(q->mapFromGlobal(QPoint(mouse.h, mouse.v)), 
                           QDragManager::self()->dropData);
        de.setAction(event_action);
        QApplication::sendEvent(q, &de);
        de.accept(true);
        de.acceptAction(true);
        if(!de.isAccepted())
            ret = false;
    } else if(kind == kEventControlDragLeave) {
        QDragLeaveEvent de;
        QApplication::sendEvent(q, &de);
    } else if(kind == kEventControlDragReceive) {
        QDropEvent de(q->mapFromGlobal(QPoint(mouse.h, mouse.v)), 
                      QDragManager::self()->dropData);
        de.accept(true);
        de.acceptAction(true);
        de.setAction(event_action);
        if(global_src)
            global_src->target = q;
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
            qDebug("Sending <%s>(%d, %d) event to %s %s [%d] (%p)",
                   desc, pos.x(), pos.y(), q->metaObject()->className(), q->objectName().local8Bit(), ret, dragRef);
        }
    }
#endif

    //set the cursor
    bool found_cursor = false;
    if(kind == kEventControlDragWithin || kind == kEventControlDragEnter) {
        if(ret) {
            if(current_drag_operation == QDrag::MoveDrag) {
                found_cursor = true;
                SetThemeCursor(kThemeArrowCursor);
            } else if(current_drag_operation == QDrag::CopyDrag) {
                found_cursor = true;
                SetThemeCursor(kThemeCopyArrowCursor);
            } else if(current_drag_operation == QDrag::LinkDrag) {
                found_cursor = true;
                SetThemeCursor(kThemeAliasArrowCursor);
            }
        } else {
            found_cursor = true;
            SetThemeCursor(kThemeNotAllowedCursor);
        }
    }
    if(found_cursor) {
        qt_mac_set_cursor(0, 0); //just use our's
    } else {
        QCursor cursor(Qt::ArrowCursor);
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
        qt_mac_set_cursor(&cursor, &mouse);
    }

    //idle things
    if(qGlobalPostedEventsCount()) {
        QApplication::sendPostedEvents();
        QApplication::flush();
    }
    return ret;
}

QDrag::DragOperation QDragManager::drag(QDragPrivate *o, QDrag::DragOperations mode)
{
    if(qt_mac_in_drag) {     //just make sure..
        qWarning("Qt: internal: WH0A, unexpected condition reached.");
        return QDrag::NoDrag;
    }
    if(object == o)
        return QDrag::NoDrag;
    /* At the moment it seems clear that Mac OS X does not want to drag with a non-left button
       so we just bail early to prevent it */
    if(!(GetCurrentEventButtonState() & kEventMouseButtonPrimary))
        return QDrag::NoDrag;

    if(object) {
        o->source->removeEventFilter(this);
        cancel();
        beingCancelled = false;
    }

    object = o;
    global_src = o;
    o->target = 0;

#ifndef QT_NO_ACCESSIBILITY
    QAccessible::updateAccessibility(this, 0, QAccessible::DragDropStart);
#endif

    OSErr result;
    DragRef theDrag;
    if((result = NewDrag(&theDrag))) 
        return QDrag::NoDrag;
    SetDragSendProc(theDrag, make_sendUPP(), o); //fullfills the promise!

    //encode the data
    QList<QMacMime*> all = QMacMime::all(QMacMime::MIME_DND);
    QStringList fmts = o->data->formats();
    for(int i = 0; i < fmts.size(); ++i) {
        for(QList<QMacMime *>::Iterator it = all.begin(); it != all.end(); ++it) {
            QMacMime *c = (*it);
            if(c->flavorFor(fmts.at(i))) {
                for (int j = 0; j < c->countFlavors(); j++) {
                    uint flav = c->flavor(j);
                    if(c->canConvert(fmts.at(i), flav))
                        AddDragItemFlavor(theDrag, 1, flav, NULL, 0, 0); //promised for later
                }
            }
        }
    }
    { //mark it with the Qt stamp
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

    QPoint hotspot;
    QPixmap pix = o->pixmap;
    if(pix.isNull()) {
        if(!o->data->text().isNull()) {
            //get the string
            QString s = o->data->text();
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
                    p.fillRect(0, 0, tmp.width(), tmp.height(), Qt::color0);
                    p.setPen(Qt::color1);
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
        hotspot = o->hotspot;
    }

    //find the hotspot in relation to the pixmap
    Point boundsPoint;
    boundsPoint.h = fakeEvent.where.h - hotspot.x();
    boundsPoint.v = fakeEvent.where.v - hotspot.y();
    Rect boundsRect;
    SetRect(&boundsRect, boundsPoint.h, boundsPoint.v, boundsPoint.h + pix.width(), boundsPoint.v + pix.height());

    //set the drag image
    SetDragItemBounds(theDrag, (ItemReference)1 , &boundsRect);
    QRegion dragRegion(boundsPoint.h, boundsPoint.v, pix.width(), pix.height()), pixRegion;
    if(!pix.isNull()) {
        pixRegion = QRegion(0, 0, pix.width(), pix.height());
        SetDragImage(theDrag, GetGWorldPixMap((GWorldPtr)pix.handle()), pixRegion.handle(true), boundsPoint, 0);
    }

    //initialize
    accept_operations = mode;
    { //do the drag
        qt_mac_in_drag = true;
        QMacBlockingFunction block;
        updateDragMode(theDrag);
        result = TrackDrag(theDrag, &fakeEvent, dragRegion.handle(true));
        qt_mac_in_drag = false;
    }
    DisposeDrag(theDrag); //cleanup
    return current_drag_operation;
}

void QDragManager::updatePixmap()
{
}

#endif // QT_NO_DRAGANDDROP
