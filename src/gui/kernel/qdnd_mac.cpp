/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qapplication.h"

#ifndef QT_NO_DRAGANDDROP
#include "qbitmap.h"
#include "qcursor.h"
#include "qevent.h"
#include "qpainter.h"
#include "qurl.h"
#include "qwidget.h"
#include <stdlib.h>
#include <string.h>
#ifndef QT_NO_ACCESSIBILITY
# include "qaccessible.h"
#endif

#include <private/qapplication_p.h>
#include <private/qdnd_p.h>
#include <private/qt_mac_p.h>

struct QMacDndAnswerRecord {
    QRect rect;
    Qt::KeyboardModifiers modifiers;
    Qt::MouseButtons buttons;
    Qt::DropAction lastAction;
    void clear() {
        rect = QRect();
        modifiers = Qt::NoModifier;
        buttons = Qt::NoButton;
        lastAction = Qt::IgnoreAction;
    }
} qt_mac_dnd_answer_rec;

/*****************************************************************************
  QDnD debug facilities
 *****************************************************************************/
//#define DEBUG_DRAG_EVENTS
//#define DEBUG_DRAG_PROMISES

/*****************************************************************************
  QDnD globals
 *****************************************************************************/
bool qt_mac_in_drag = false;
static DragReference qt_mac_current_dragRef = 0;

/*****************************************************************************
  Externals
 *****************************************************************************/
extern void qt_mac_send_modifiers_changed(quint32, QObject *); //qapplication_mac.cpp
extern uint qGlobalPostedEventsCount(); //qapplication.cpp

/*****************************************************************************
  QDnD utility functions
 *****************************************************************************/

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
#ifdef DEBUG_DRAG_EVENTS
# define MAP_MAC_ENUM(x) x, #x
#else
# define MAP_MAC_ENUM(x) x
#endif
struct mac_enum_mapper
{
    int mac_code;
    int qt_code;
#ifdef DEBUG_DRAG_EVENTS
    char *qt_desc;
#endif
};
static mac_enum_mapper dnd_action_symbols[] = {
    { kDragActionAlias, MAP_MAC_ENUM(Qt::LinkAction) },
    { kDragActionMove, MAP_MAC_ENUM(Qt::MoveAction) },
    { kDragActionGeneric, MAP_MAC_ENUM(Qt::CopyAction) },
    { kDragActionCopy, MAP_MAC_ENUM(Qt::CopyAction) },
    { 0, MAP_MAC_ENUM(0) }
};
static DragActions qt_mac_dnd_map_qt_actions(Qt::DropActions qActions)
{
    DragActions ret = kDragActionNothing;
    for(int i = 0; dnd_action_symbols[i].qt_code; ++i) {
        if(qActions & dnd_action_symbols[i].qt_code)
            ret |= dnd_action_symbols[i].mac_code;
    }
    return ret;
}
static Qt::DropActions qt_mac_dnd_map_mac_actions(DragActions macActions)
{
#ifdef DEBUG_DRAG_EVENTS
    qDebug("Converting DND ActionList: 0x%lx", macActions);
#endif
    Qt::DropActions ret = Qt::IgnoreAction;
    for(int i = 0; dnd_action_symbols[i].qt_code; ++i) {
#ifdef DEBUG_DRAG_EVENTS
        qDebug(" %d) [%s] : %s", i, dnd_action_symbols[i].qt_desc,
                (macActions & dnd_action_symbols[i].mac_code) ? "true" : "false");
#endif
        if(macActions & dnd_action_symbols[i].mac_code)
            ret |= Qt::DropAction(dnd_action_symbols[i].qt_code);
    }
    return ret;
}
static Qt::DropAction qt_mac_dnd_map_mac_default_action(DragActions macActions)
{
    static Qt::DropAction preferred_actions[] = { Qt::CopyAction, Qt::LinkAction, //in order
        Qt::MoveAction, Qt::IgnoreAction };
    Qt::DropAction ret = Qt::IgnoreAction;
    const Qt::DropActions qtActions = qt_mac_dnd_map_mac_actions(macActions);
    for(int i = 0; preferred_actions[i] != Qt::IgnoreAction; ++i) {
        if(qtActions & preferred_actions[i]) {
            ret = preferred_actions[i];
            break;
        }
    }
#ifdef DEBUG_DRAG_EVENTS
    for(int i = 0; dnd_action_symbols[i].qt_code; ++i) {
        if(dnd_action_symbols[i].qt_code == ret) {
            qDebug("Got default action: %s [0x%lx]", dnd_action_symbols[i].qt_desc, macActions);
            break;
        }
    }
#endif
    return ret;
}
static void qt_mac_dnd_update_action(DragReference dragRef) {
    SInt16 modifiers;
    GetDragModifiers(dragRef, &modifiers, 0, 0);
    qt_mac_send_modifiers_changed(modifiers, qApp);

    Qt::DropAction qtAction = Qt::IgnoreAction;
    {
        DragActions macAllowed = kDragActionNothing;
        GetDragAllowableActions(dragRef, &macAllowed);
        Qt::DropActions qtAllowed = qt_mac_dnd_map_mac_actions(macAllowed);
        qtAction = QDragManager::self()->defaultAction(qtAllowed, QApplication::keyboardModifiers());
#if 1
        if(!(qtAllowed & qtAction))
            qtAction = qt_mac_dnd_map_mac_default_action(macAllowed);
#endif
    }
    DragActions macAction = qt_mac_dnd_map_qt_actions(qtAction);

    DragActions currentActions;
    GetDragDropAction(dragRef, &currentActions);
    if(currentActions != macAction) {
        SetDragDropAction(dragRef, macAction);
        QDragManager::self()->emitActionChanged(qtAction);
    }
}

/*****************************************************************************
  DnD functions
 *****************************************************************************/
bool QDropData::hasFormat_sys(const QString &mime) const
{
    PasteboardRef board;
    if(GetDragPasteboard(qt_mac_current_dragRef, &board) != noErr) {
        qDebug("DnD: Cannot get PasteBoard!");
        return false;
    }
    return QMacPasteboard(board, QMacPasteboardMime::MIME_DND).hasFormat(mime);
}

QVariant QDropData::retrieveData_sys(const QString &mime, QVariant::Type type) const
{
    PasteboardRef board;
    if(GetDragPasteboard(qt_mac_current_dragRef, &board) != noErr) {
        qDebug("DnD: Cannot get PasteBoard!");
        return QVariant();
    }
    return QMacPasteboard(board, QMacPasteboardMime::MIME_DND).retrieveData(mime, type);
}

QStringList QDropData::formats_sys() const
{
    PasteboardRef board;
    if(GetDragPasteboard(qt_mac_current_dragRef, &board) != noErr) {
        qDebug("DnD: Cannot get PasteBoard!");
        return QStringList();
    }
    return QMacPasteboard(board, QMacPasteboardMime::MIME_DND).formats();
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
    Q_Q(QWidget);
    qt_mac_current_dragRef = dragRef;
    if (kind != kEventControlDragLeave)
        qt_mac_dnd_update_action(dragRef);

    Point mouse;
    GetDragMouse(dragRef, &mouse, 0L);
    if(!mouse.h && !mouse.v)
        GetGlobalMouse(&mouse);

    if(QApplicationPrivate::modalState()) {
        for(QWidget *modal = q; modal; modal = modal->parentWidget()) {
            if(modal->isWindow()) {
                if(modal != QApplication::activeModalWidget())
                    return noErr;
                break;
            }
        }
    }

    //lookup the possible actions
    DragActions allowed = kDragActionNothing;
    GetDragAllowableActions(dragRef, &allowed);
    Qt::DropActions qtAllowed = qt_mac_dnd_map_mac_actions(allowed);

    //lookup the source
    QMimeData *dropdata = QDragManager::self()->dropData;
    if(QDragManager::self()->source())
        dropdata = QDragManager::self()->dragPrivate()->data;

    //Dispatch events
    bool ret = true;
    if(kind == kEventControlDragWithin) {
        if (!qt_mac_dnd_answer_rec.rect.isEmpty()) {
            if (qt_mac_dnd_answer_rec.rect.contains(q->mapFromGlobal(QPoint(mouse.h, mouse.v)))
                    && QApplication::mouseButtons() == qt_mac_dnd_answer_rec.buttons
                    && QApplication::keyboardModifiers() == qt_mac_dnd_answer_rec.modifiers) {
                return qt_mac_dnd_answer_rec.lastAction == Qt::IgnoreAction;
            } else {
                qt_mac_dnd_answer_rec.clear();
            }
        }
        QDragMoveEvent de(q->mapFromGlobal(QPoint(mouse.h, mouse.v)), qtAllowed, dropdata,
                QApplication::mouseButtons(), QApplication::keyboardModifiers());
        DragActions currentAction = kDragActionNothing;
        OSStatus err = GetDragDropAction(dragRef, &currentAction);
        if (err == noErr && currentAction != kDragActionNothing) {
            // This looks a bit evil, but we only ever set one action, so it's OK.
            de.setDropAction(Qt::DropAction(int(qt_mac_dnd_map_mac_actions(currentAction))));
            de.accept();
        }
        QApplication::sendEvent(q, &de);
        if(!de.isAccepted() || de.dropAction() == Qt::IgnoreAction)
            ret = false;

        if (!de.answerRect().isEmpty()) {
            qt_mac_dnd_answer_rec.rect = de.answerRect();
            qt_mac_dnd_answer_rec.buttons = de.mouseButtons();
            qt_mac_dnd_answer_rec.modifiers = de.keyboardModifiers();
            qt_mac_dnd_answer_rec.lastAction = de.dropAction();
        }
        SetDragDropAction(dragRef, qt_mac_dnd_map_qt_actions(de.dropAction()));
    } else if(kind == kEventControlDragEnter) {
        qt_mac_dnd_answer_rec.clear();
        QDragEnterEvent de(q->mapFromGlobal(QPoint(mouse.h, mouse.v)), qtAllowed, dropdata,
                           QApplication::mouseButtons(), QApplication::keyboardModifiers());
        QApplication::sendEvent(q, &de);
        if(!de.isAccepted())
            ret = false;
        SetDragDropAction(dragRef, qt_mac_dnd_map_qt_actions(de.dropAction()));
    } else if(kind == kEventControlDragLeave) {
        QDragLeaveEvent de;
        QApplication::sendEvent(q, &de);
    } else if(kind == kEventControlDragReceive) {
        QDropEvent de(q->mapFromGlobal(QPoint(mouse.h, mouse.v)), qtAllowed, dropdata,
                      QApplication::mouseButtons(), QApplication::keyboardModifiers());
        if(QDragManager::self()->object)
            QDragManager::self()->dragPrivate()->target = q;
        QApplication::sendEvent(q, &de);
        if(!de.isAccepted()) {
            ret = false;
            SetDragDropAction(dragRef, kDragActionNothing);
        } else {
            if(QDragManager::self()->object)
                QDragManager::self()->dragPrivate()->executed_action = de.dropAction();
            SetDragDropAction(dragRef, qt_mac_dnd_map_qt_actions(de.dropAction()));
        }
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
            qDebug("Sending <%s>(%d, %d) event to %p(%s::%s) [%d] (%p)",
                   desc, pos.x(), pos.y(), q, q->metaObject()->className(),
                   q->objectName().toLocal8Bit().constData(), ret, dragRef);
        }
    }
#endif

    //set the cursor
    bool found_cursor = false;
    if(kind == kEventControlDragWithin || kind == kEventControlDragEnter) {
        ThemeCursor cursor = kThemeNotAllowedCursor;
        found_cursor = true;
        if(ret) {
            DragActions action = kDragActionNothing;
            GetDragDropAction(dragRef, &action);
            switch(qt_mac_dnd_map_mac_default_action(action)) {
            case Qt::IgnoreAction:
                cursor = kThemeNotAllowedCursor;
                break;
            case Qt::MoveAction:
                cursor = kThemeArrowCursor;
                break;
            case Qt::CopyAction:
                cursor = kThemeCopyArrowCursor;
                break;
            case Qt::LinkAction:
                cursor = kThemeAliasArrowCursor;
                break;
            default:
                cursor = kThemeNotAllowedCursor;
                break;
            }
        }
        SetThemeCursor(cursor);
    }
    if(found_cursor) {
        qt_mac_set_cursor(0, QPoint()); //just use our's
    } else {
        QCursor cursor(Qt::ArrowCursor);
        if(qApp && qApp->overrideCursor()) {
            cursor = *qApp->overrideCursor();
        } else if(q) {
            for(QWidget *p = q; p; p = p->parentWidget()) {
                if(p->isEnabled() && p->testAttribute(Qt::WA_SetCursor)) {
                    cursor = p->cursor();
                    break;
                }
            }
        }
        qt_mac_set_cursor(&cursor, QPoint(mouse.h, mouse.v));
    }

    //idle things
    if(qGlobalPostedEventsCount()) {
        QApplication::sendPostedEvents();
        QApplication::flush();
    }
    return ret;
}

Qt::DropAction QDragManager::drag(QDrag *o)
{
    if(qt_mac_in_drag) {     //just make sure..
        qWarning("Qt: Internal error: WH0A, unexpected condition reached");
        return Qt::IgnoreAction;
    }
    if(object == o)
        return Qt::IgnoreAction;
    /* At the moment it seems clear that Mac OS X does not want to drag with a non-left button
       so we just bail early to prevent it */
    if(!(GetCurrentEventButtonState() & kEventMouseButtonPrimary))
        return Qt::IgnoreAction;

    if(object) {
        dragPrivate()->source->removeEventFilter(this);
        cancel();
        beingCancelled = false;
    }

    object = o;
    dragPrivate()->target = 0;

#ifndef QT_NO_ACCESSIBILITY
    QAccessible::updateAccessibility(this, 0, QAccessible::DragDropStart);
#endif

    //setup the data
    QMacPasteboard dragBoard(QMacPasteboardMime::MIME_DND);
    dragBoard.setMimeData(dragPrivate()->data);

    //create the drag
    OSErr result;
    DragRef dragRef;
    if((result = NewDragWithPasteboard(dragBoard.pasteBoard(), &dragRef)))
        return Qt::IgnoreAction;
    //setup the actions
    DragActions possibleActions = qt_mac_dnd_map_qt_actions(dragPrivate()->possible_actions);
    SetDragAllowableActions(dragRef, //local
                            possibleActions,
                            true);
    SetDragAllowableActions(dragRef, //remote (same as local)
                            possibleActions,
                            false);


    QPoint hotspot;
    QPixmap pix = dragPrivate()->pixmap;
    if(pix.isNull()) {
        if(dragPrivate()->data->hasText() || dragPrivate()->data->hasUrls()) {
            //get the string
            QString s = dragPrivate()->data->hasText() ? dragPrivate()->data->text()
                                                : dragPrivate()->data->urls().first().toString();
            if(s.length() > 26)
                s = s.left(23) + QChar(0x2026);
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
            pix = QPixmap(default_pm);
            hotspot = QPoint(default_pm_hotx, default_pm_hoty);
        }
    } else {
        hotspot = dragPrivate()->hotspot;
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

    //find the hotspot in relation to the pixmap
    Point boundsPoint;
    boundsPoint.h = fakeEvent.where.h - hotspot.x();
    boundsPoint.v = fakeEvent.where.v - hotspot.y();
    Rect boundsRect;
    SetRect(&boundsRect, boundsPoint.h, boundsPoint.v, boundsPoint.h + pix.width(), boundsPoint.v + pix.height());

    //set the drag image
    QRegion dragRegion(boundsPoint.h, boundsPoint.v, pix.width(), pix.height()), pixRegion;
    if(!pix.isNull()) {
        HIPoint hipoint = { -hotspot.x(), -hotspot.y() };
        CGImageRef img = (CGImageRef)pix.macCGHandle();
        SetDragImageWithCGImage(dragRef, img, &hipoint, 0);
        CGImageRelease(img);
    }

    SetDragItemBounds(dragRef, (ItemReference)1 , &boundsRect);
    { //do the drag
        qt_mac_in_drag = true;
        QMacBlockingFunction block;
        qt_mac_dnd_update_action(dragRef);
        result = TrackDrag(dragRef, &fakeEvent, dragRegion.handle(true));
        qt_mac_in_drag = false;
    }
    object = 0;
    if(result == noErr) {
        DragActions ret = kDragActionNothing;
        GetDragDropAction(dragRef, &ret);
        DisposeDrag(dragRef); //cleanup
        return qt_mac_dnd_map_mac_default_action(ret);
    }
    DisposeDrag(dragRef); //cleanup
    return Qt::IgnoreAction;
}

void QDragManager::updatePixmap()
{
}

#endif // QT_NO_DRAGANDDROP
