/****************************************************************************
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
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

/*****************************************************************************
  QClipboard debug facilities
 *****************************************************************************/
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

#include "qwidget_p.h"
class QExtraWidget : public QWidget
{
public:
    inline QWExtra* extraData();
    inline QTLWExtra* topData();
};
inline QWExtra* QExtraWidget::extraData() { return d->extraData(); }
inline QTLWExtra* QExtraWidget::topData() { return d->topData(); }

//functions
extern uint qGlobalPostedEventsCount();
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

bool QDropEvent::provides(const char *mime) const
{
    ItemReference ref = NULL;
    if(GetDragItemReferenceNumber(current_dropobj, 1, &ref))
	return FALSE;

    UInt16 cnt = 0;
    if(CountDragItemFlavors(current_dropobj, ref, &cnt))
	return FALSE;

    FlavorType flav;
    QMacMime::QMacMimeType qmt = QMacMime::MIME_DND;
    Size sz;
    extern ScrapFlavorType qt_mac_mime_type; //qmime_mac.cpp
    if(GetFlavorDataSize(current_dropobj, ref, qt_mac_mime_type, &sz) == noErr)
	qmt = QMacMime::MIME_QT_CONVERTOR;
    for(int x = 1; x <= (int)cnt; x++) {
	if(GetFlavorType(current_dropobj, ref, x, &flav) == noErr) {
	    if(QMacMime::convertor(qmt, mime, flav))
		return TRUE;
	}
    }
    return FALSE;
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
		    char *buffer = (char *)malloc(flavorsize);
		    GetFlavorData(current_dropobj, ref, flav, buffer, &flavorsize, 0);
		    QByteArray r;
		    r.setRawData(buffer, flavorsize);
		    arrs.append(r);
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
	bool sawSBText = FALSE;
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
		    sawSBText = TRUE;
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
static DragSendDataUPP qt_mac_send_handlerUPP = NULL;
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

bool QDragManager::drag(QDragObject *o, QDragObject::DragMode mode)
{
    if(qt_mac_in_drag) {     //just make sure..
	qDebug("Qt: internal: WH0A, unexpected condition reached.");
	return FALSE;
    }
    if(object == o)
	return FALSE;
#if QT_MACOSX_VERSION >= 0x1020
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

#if 0
    /* we have to clear the button down */
    extern QGuardedPtr<QWidget> qt_button_down; //qapplication_mac.cpp
    qt_button_down = NULL;
#endif

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
#if QT_MACOSX_VERSION >= 0x1020
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
	SetDragImage(theDrag, GetGWorldPixMap((GWorldPtr)pix.handle()), pixRegion.handle(TRUE), boundsPoint, 0);
    }

    QWidget *widget = QApplication::widgetAt(fakeEvent.where.h, fakeEvent.where.v);
    if(!widget) {
	dragSource = 0;
	return FALSE;
    }
    acceptfmt = FALSE;
    acceptact = FALSE;
    drag_received = FALSE;
    qt_mac_in_drag = TRUE;
    QWExtra *extra = ((QExtraWidget*)widget)->extraData();
    if(!extra->macDndExtra) //never too late I suppose..
	qt_macdnd_register(widget, extra);
    set_drag_mode = mode;
    updateDragMode(theDrag);
    //kick off the drag by calling the callback ourselves first..
    qt_mac_tracking_handler(kDragTrackingEnterWindow, (WindowPtr)widget->hd,
			     (void *)extra->macDndExtra, theDrag);
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
    updateDragMode(theDrag);
    QMacDndExtra *macDndExtra = (QMacDndExtra*) handlerRefCon;
    current_dropobj = theDrag;
    Point mouse;
    GetDragMouse(theDrag, &mouse, 0L);

    QCursor cursor(Qt::ArrowCursor);
    if(qApp && qApp->overrideCursor()) {
	cursor = *qApp->overrideCursor();
    } else if(QWidget *widget = QApplication::widgetAt(mouse.h, mouse.v)) {
	for(QWidget *p = widget; p; p = p->parentWidget()) {
	    if(p->testAttribute(QWidget::WA_SetCursor)) {
		cursor = p->cursor();
		break;
	    }
	}
    }
    qt_mac_set_cursor(&cursor, &mouse);

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
    QWidget *widget = QApplication::widgetAt(globalMouse);
    while(widget && !widget->acceptDrops() && !widget->isTopLevel())
	widget = widget->parentWidget();
    if(!widget->acceptDrops())
	widget = NULL;
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
		current_drag_widget = widget;
	    }
	}
    }

    //set the cursor
    const QCursor *dnd_cursor = NULL;
    if(current_drag_widget && macDndExtra->acceptfmt) {
#ifndef QMAC_NO_FAKECURSOR
	if(current_drag_action == QDropEvent::Move)
	    dnd_cursor = moveCursor;
	else if(current_drag_action == QDropEvent::Copy)
	    dnd_cursor = copyCursor;
	else if(current_drag_action == QDropEvent::Link)
	    dnd_cursor = linkCursor;
#endif
    } else {
	dnd_cursor = noDropCursor;
    }
    QCursor cursor(Qt::ArrowCursor);
    if(!dnd_cursor) {
	if(qApp && qApp->overrideCursor()) {
	    cursor = *qApp->overrideCursor();
	} else if(widget) {
	    for(QWidget *p = widget; p; p = p->parentWidget()) {
		if(p->testAttribute(QWidget::WA_SetCursor)) {
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
