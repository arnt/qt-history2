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
static DragReference qt_mac_current_dragRef = 0;

/*****************************************************************************
  Externals
 *****************************************************************************/
extern bool qt_modal_state(); //qapplication_mac.cpp
extern uint qGlobalPostedEventsCount(); //qapplication.cpp

/*****************************************************************************
  QDnD utility functions
 *****************************************************************************/
//promise keeper
static DragSendDataUPP qt_mac_send_handlerUPP = 0;
static OSErr qt_mac_send_handler(FlavorType flav, void *data, DragItemRef, DragRef dragRef)
{
    QDragPrivate *o = (QDragPrivate *)data;
    QMacMime::QMacMimeType qmt = QMacMime::MIME_DND;
    {
        ItemReference ref = 0;
        if(GetDragItemReferenceNumber(dragRef, 1, &ref) == noErr) {
            Size sz;
            extern ScrapFlavorType qt_mac_mime_type; //qmime_mac.cpp
            if(GetFlavorDataSize(dragRef, ref, qt_mac_mime_type, &sz) == noErr)
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
                    SetDragItemFlavorData(dragRef, (ItemReference)item_ref++, flav, (*it).data(), (*it).size(), 0);
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
        qt_mac_send_handlerUPP = 0;
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

//action management
#define MAP_MAC_ENUM(x) x
struct mac_enum_mapper
{
    int mac_code;
    int qt_code;
};
static mac_enum_mapper dnd_action_symbols[] = {
    { kDragActionAlias, MAP_MAC_ENUM(QDrag::LinkAction) },
    { kDragActionMove, MAP_MAC_ENUM(QDrag::MoveAction) },
    { kDragActionCopy, MAP_MAC_ENUM(QDrag::CopyAction) },
    { 0, MAP_MAC_ENUM(0) }
};
static DragActions qt_mac_dnd_map_qt_actions(QDrag::DropActions qActions)
{
    DragActions ret = kDragActionNothing;
    for(int i = 0; dnd_action_symbols[i].qt_code; ++i) {
        if(qActions & dnd_action_symbols[i].qt_code)
            ret |= dnd_action_symbols[i].mac_code;
    }
    return ret;
}
static QDrag::DropActions qt_mac_dnd_map_mac_actions(DragActions macActions)
{
    QDrag::DropActions ret = QDrag::IgnoreAction;
    for(int i = 0; dnd_action_symbols[i].qt_code; ++i) {
        if(macActions & dnd_action_symbols[i].mac_code)
            ret |= QDrag::DropAction(dnd_action_symbols[i].qt_code);
    }
    return ret;
}
static QDrag::DropAction qt_mac_dnd_map_mac_preferred_action(DragActions macActions)
{
    static QDrag::DropAction preferred_actions[] = { QDrag::CopyAction, QDrag::LinkAction, //in order
                                                      QDrag::MoveAction, QDrag::IgnoreAction };
    const QDrag::DropActions qtActions = qt_mac_dnd_map_mac_actions(macActions);
    for(int i = 0; preferred_actions[i] != QDrag::IgnoreAction; ++i) {
        if(qtActions & preferred_actions[i])
            return preferred_actions[i];
    }
    return QDrag::IgnoreAction;
}
static void qt_mac_dnd_update_action(DragReference dragRef) {
    SInt16 mod;
    GetDragModifiers(dragRef, &mod, 0, 0);
    DragActions action = kDragActionNothing;
    if((mod & (optionKey|cmdKey)) == (optionKey|cmdKey))
        action = kDragActionAlias;
    else if((mod & optionKey) || (mod & rightOptionKey))
        action = kDragActionCopy;
    else
        action = kDragActionMove;
    DragActions allowed = kDragActionNothing;
    GetDragAllowableActions(dragRef, &allowed);
    if(allowed & action) 
        SetDragDropAction(dragRef, action);
}

/*****************************************************************************
  DnD functions
 *****************************************************************************/
bool QDropData::hasFormat(const QString &mime) const
{
    ItemReference ref = 0;
    if(GetDragItemReferenceNumber(qt_mac_current_dragRef, 1, &ref))
        return false;

    UInt16 cnt = 0;
    if(CountDragItemFlavors(qt_mac_current_dragRef, ref, &cnt))
        return false;

    FlavorType flav;
    QMacMime::QMacMimeType qmt = QMacMime::MIME_DND;
    Size sz;
    extern ScrapFlavorType qt_mac_mime_type; //qmime_mac.cpp
    if(GetFlavorDataSize(qt_mac_current_dragRef, ref, qt_mac_mime_type, &sz) == noErr)
        qmt = QMacMime::MIME_QT_CONVERTOR;
    for(int x = 1; x <= (int)cnt; x++) {
        if(GetFlavorType(qt_mac_current_dragRef, ref, x, &flav) == noErr) {
            if(QMacMime::convertor(qmt, mime, flav))
                return true;
        }
    }
    return false;
}

QVariant QDropData::retrieveData(const QString &mime, QVariant::Type type) const
{
    Size flavorsize=0;
    QVariant ret;
    QMacMime::QMacMimeType qmt = QMacMime::MIME_DND;
    {
        ItemReference ref = 0;
        if(GetDragItemReferenceNumber(qt_mac_current_dragRef, 1, &ref) == noErr) {
            Size sz;
            extern ScrapFlavorType qt_mac_mime_type; //qmime_mac.cpp
            if(GetFlavorDataSize(qt_mac_current_dragRef, ref, qt_mac_mime_type, &sz) == noErr)
                qmt = QMacMime::MIME_QT_CONVERTOR;
        }
    }
    QList<QMacMime*> all = QMacMime::all(qmt);
    for(QList<QMacMime *>::Iterator it = all.begin(); it != all.end(); ++it) {
        QMacMime *c = (*it);
        int flav = c->flavorFor(mime);
        if(flav) {
            UInt16 cnt_items;
            CountDragItems(qt_mac_current_dragRef, &cnt_items);
            QList<QByteArray> arrs;
            for(int i = 1; i <= cnt_items; i++) {
                ItemReference ref = 0;
                if(GetDragItemReferenceNumber(qt_mac_current_dragRef, i, &ref)) {
                    qWarning("Qt: internal: OOps.. %s:%d", __FILE__, __LINE__);
                    return QByteArray();
                }
                if(GetFlavorDataSize(qt_mac_current_dragRef, ref, flav, &flavorsize) == noErr) {
                    char *buffer = static_cast<char *>(malloc(flavorsize));
                    GetFlavorData(qt_mac_current_dragRef, ref, flav, buffer, &flavorsize, 0);
                    arrs.append(QByteArray::fromRawData(buffer, flavorsize));
                }
            }
            if(!arrs.isEmpty()) {
                QByteArray mime_data = c->convertToMime(arrs, mime, flav);
#if 0
                if(type == QVariant::String) 
                    ret = QString::fromUtf8(mime_data);
                else 
#endif
                    ret = mime_data;
                break;
            }
        }
    }
    return ret;
}

QStringList QDropData::formats() const
{
    QStringList ret;

    ItemReference ref = 0;
    if(GetDragItemReferenceNumber(qt_mac_current_dragRef, 1, &ref))
        return ret;

    UInt16 cnt = 0;
    if(CountDragItemFlavors(qt_mac_current_dragRef, ref, &cnt))
        return ret;

    FlavorType flav;
    QMacMime::QMacMimeType qmt = QMacMime::MIME_DND;
    {
        Size sz;
        extern ScrapFlavorType qt_mac_mime_type; //qmime_mac.cpp
        if(GetFlavorDataSize(qt_mac_current_dragRef, ref, qt_mac_mime_type, &sz) == noErr)
            qmt = QMacMime::MIME_QT_CONVERTOR;
    }
    for(int x = 1; x <= (int)cnt; x++) {
        if(GetFlavorType(qt_mac_current_dragRef, ref, x, &flav) == noErr) {
            QString mime = QMacMime::flavorToMime(qmt, flav);
            if(!mime.isNull() && !ret.contains(mime)) 
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
    qt_mac_current_dragRef = dragRef;
    qt_mac_dnd_update_action(dragRef);

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

    DragActions macActions = kDragActionNothing;
    GetDragDropAction(dragRef, &macActions);
    QDrag::DropAction qtAction = qt_mac_dnd_map_mac_preferred_action(macActions); 

    //Dispatch events
    bool ret = true;
    if(kind == kEventControlDragWithin) {
        QDragMoveEvent de(q->mapFromGlobal(QPoint(mouse.h, mouse.v)), qtAction,
                          QDragManager::self()->dropData);
        de.accept();
        QApplication::sendEvent(q, &de);
    } else if(kind == kEventControlDragEnter) {
        QDragEnterEvent de(q->mapFromGlobal(QPoint(mouse.h, mouse.v)), qtAction,
                           QDragManager::self()->dropData);
        QApplication::sendEvent(q, &de);
        de.accept();
        if(!de.isAccepted())
            ret = false;
    } else if(kind == kEventControlDragLeave) {
        QDragLeaveEvent de;
        QApplication::sendEvent(q, &de);
    } else if(kind == kEventControlDragReceive) {
        QDropEvent de(q->mapFromGlobal(QPoint(mouse.h, mouse.v)), qtAction,
                      QDragManager::self()->dropData);
        de.accept();
        if(QDragManager::self()->object) {
            QDragManager::self()->dragPrivate()->target = q;
            QDragManager::self()->dragPrivate()->executed_action = qtAction;
        }
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
                   desc, pos.x(), pos.y(), q->metaObject()->className(), 
                   q->objectName().local8Bit(), ret, dragRef);
        }
    }
#endif

    //set the cursor
    bool found_cursor = false;
    if(kind == kEventControlDragWithin || kind == kEventControlDragEnter) {
        if(ret) {
            if(qtAction == QDrag::MoveAction) {
                found_cursor = true;
                SetThemeCursor(kThemeArrowCursor);
            } else if(qtAction == QDrag::CopyAction) {
                found_cursor = true;
                SetThemeCursor(kThemeCopyArrowCursor);
            } else if(qtAction == QDrag::LinkAction) {
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

QDrag::DropAction QDragManager::drag(QDrag *o)
{
    if(qt_mac_in_drag) {     //just make sure..
        qWarning("Qt: internal: WH0A, unexpected condition reached.");
        return QDrag::IgnoreAction;
    }
    if(object == o)
        return QDrag::IgnoreAction;
    /* At the moment it seems clear that Mac OS X does not want to drag with a non-left button
       so we just bail early to prevent it */
    if(!(GetCurrentEventButtonState() & kEventMouseButtonPrimary))
        return QDrag::IgnoreAction;

    if(object) {
        o->d->source->removeEventFilter(this);
        cancel();
        beingCancelled = false;
    }

    object = o;
    o->d->target = 0;

#ifndef QT_NO_ACCESSIBILITY
    QAccessible::updateAccessibility(this, 0, QAccessible::DragDropStart);
#endif

    OSErr result;
    DragRef dragRef;
    if((result = NewDrag(&dragRef)))
        return QDrag::IgnoreAction;
    SetDragSendProc(dragRef, make_sendUPP(), o->d); //fullfills the promise!
    SetDragAllowableActions(dragRef, //setup the actions
                            qt_mac_dnd_map_qt_actions(o->d->possible_actions), 
                            false);

    //encode the data
    QList<QMacMime*> all = QMacMime::all(QMacMime::MIME_DND);
    QStringList fmts = o->d->data->formats();
    for(int i = 0; i < fmts.size(); ++i) {
        for(QList<QMacMime *>::Iterator it = all.begin(); it != all.end(); ++it) {
            QMacMime *c = (*it);
            if(c->flavorFor(fmts.at(i))) {
                for (int j = 0; j < c->countFlavors(); j++) {
                    uint flav = c->flavor(j);
                    if(c->canConvert(fmts.at(i), flav))
                        AddDragItemFlavor(dragRef, 1, flav, 0, 0, 0); //promised for later
                }
            }
        }
    }
    { //mark it with the Qt stamp
        char t = 123;
        extern ScrapFlavorType qt_mac_mime_type; //qmime_mac.cpp
        AddDragItemFlavor(dragRef, 1, qt_mac_mime_type, &t, 1, 0);
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
    QPixmap pix = o->d->pixmap;
    if(pix.isNull()) {
        if(!o->d->data->text().isNull()) {
            //get the string
            QString s = o->d->data->text();
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
        hotspot = o->d->hotspot;
    }

    //find the hotspot in relation to the pixmap
    Point boundsPoint;
    boundsPoint.h = fakeEvent.where.h - hotspot.x();
    boundsPoint.v = fakeEvent.where.v - hotspot.y();
    Rect boundsRect;
    SetRect(&boundsRect, boundsPoint.h, boundsPoint.v, boundsPoint.h + pix.width(), boundsPoint.v + pix.height());

    //set the drag image
    SetDragItemBounds(dragRef, (ItemReference)1 , &boundsRect);
    QRegion dragRegion(boundsPoint.h, boundsPoint.v, pix.width(), pix.height()), pixRegion;
    if(!pix.isNull()) {
        pixRegion = QRegion(0, 0, pix.width(), pix.height());
        SetDragImage(dragRef, GetGWorldPixMap((GWorldPtr)pix.handle()), pixRegion.handle(true), boundsPoint, 0);
    }

    { //do the drag
        qt_mac_in_drag = true;
        QMacBlockingFunction block;
        qt_mac_dnd_update_action(dragRef);
        result = TrackDrag(dragRef, &fakeEvent, dragRegion.handle(true));
        qt_mac_in_drag = false;
    }
    DragActions ret = kDragActionNothing;
    GetDragDropAction(dragRef, &ret);
    DisposeDrag(dragRef); //cleanup
    return qt_mac_dnd_map_mac_preferred_action(ret);
}

void QDragManager::updatePixmap()
{
}

#endif // QT_NO_DRAGANDDROP
