/****************************************************************************
**
** Implementation of QWidget and QWindow classes for mac.
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

#include "qt_mac.h"

#include "qimage.h"
#include "qapplication.h"
#include "qapplication_p.h"
#include "qpaintdevicemetrics.h"
#include "qpainter.h"
#include "qbitmap.h"
#include "qaccel.h"
#include "qdragobject.h"
#include "qlayout.h"
#include "qtextcodec.h"
#include "qcursor.h"
#include "qtimer.h"
#include "qstyle.h"
#include "qevent.h"
#include "qdesktopwidget.h"
#include "qstack.h"
#ifdef Q_WS_MACX
# include <ApplicationServices/ApplicationServices.h>
#endif
#include <limits.h>

#include "qwidget_p.h"

#define d d_func()
#define q q_func()

/*****************************************************************************
  QWidget debug facilities
 *****************************************************************************/
//#define DEBUG_WINDOW_RGNS
//#define DEBUG_WINDOW_CREATE

/*****************************************************************************
  QWidget globals
 *****************************************************************************/
static bool no_move_blt = FALSE;
static WId serial_id = 0;
QWidget *mac_mouse_grabber = 0;
QWidget *mac_keyboard_grabber = 0;
int mac_window_count = 0;

/*****************************************************************************
  Externals
 *****************************************************************************/
QString cfstring2qstring(CFStringRef); //qglobal.cpp
void qt_mac_unicode_reset_input(QWidget *); //qapplication_mac.cpp
void qt_mac_unicode_init(QWidget *); //qapplication_mac.cpp
void qt_mac_unicode_cleanup(QWidget *); //qapplication_mac.cpp
void qt_event_request_updates(); //qapplication_mac.cpp
void qt_event_request_activate(QWidget *); //qapplication_mac.cpp
void qt_event_request_showsheet(QWidget *); //qapplication_mac.cpp
extern void qt_mac_set_cursor(const QCursor *, const Point *); //qcursor_mac.cpp
bool qt_nograb();
RgnHandle qt_mac_get_rgn(); //qregion_mac.cpp
void qt_mac_dispose_rgn(RgnHandle r); //qregion_mac.cpp
void unclippedBitBlt(QPaintDevice *, int, int, const QPaintDevice *, int, int,
		      int, int, Qt::RasterOp, bool, bool); //qpaintdevice_mac.cpp

/*****************************************************************************
  QWidget utility functions
 *****************************************************************************/
QPoint posInWindow(QWidget *w)
{
    if(w->isTopLevel())
	return QPoint(0, 0);
    int x = 0, y = 0;
    if(QWidget *par = w->parentWidget(TRUE)) {
	QPoint p = posInWindow(par);
	x = p.x() + w->geometry().x();
	y = p.y() + w->geometry().y();
    }
    return QPoint(x, y);
}

bool qt_mac_update_sizer(QWidget *w, int up=0)
{
    if(!w || !w->isTopLevel())
	return FALSE;
    w->d->createTLExtra();
    w->d->topData()->resizer += up;
    if(w->d->topData()->resizer ||
       (w->d->extraData()->maxw && w->d->extraData()->maxh && w->d->extraData()->maxw == w->d->extraData()->minw &&
	w->d->extraData()->maxh == w->d->extraData()->minh))
	ChangeWindowAttributes((WindowRef)w->handle(), 0, kWindowResizableAttribute);
    else
	ChangeWindowAttributes((WindowRef)w->handle(), kWindowResizableAttribute, 0);
    return TRUE;
}

static inline const Rect *mac_rect(const QRect &qr)
{
    static Rect r;
    SetRect(&r, qr.left(), qr.top(), qr.right()+1, qr.bottom()+1); //qt says be inclusive!
    return &r;
}
static inline const Rect *mac_rect(const QPoint &qp, const QSize &qs) { return mac_rect(QRect(qp, qs)); }

#ifdef DEBUG_WINDOW_RGNS
static inline void debug_wndw_rgn(const char *where, QWidget *w, const QRegion &r,
				  bool clean=FALSE, bool translate=FALSE) {
    QPoint mp(posInWindow(w));
    QRect wrect(mp.x(), mp.y(), w->width(), w->height());
    qDebug("Qt: internal: %s %s %s (%s) [ %d %d %d %d ]", where, clean ? "clean" : "dirty",
	   w->className(), w->name(), wrect.x(), wrect.y(), wrect.width(), wrect.height());
    QVector<QRect> rs = r.rects();
    int offx = 0, offy = 0;
    if(translate) {
	offx = mp.x();
	offy = mp.y();
    }
    for(int i = 0; i < rs.count(); i++) {
	QRect srect(rs[i].x()+offx, rs[i].y()+offy, rs[i].width(), rs[i].height());
	// * == Completely inside the widget, - == intersects, ? == completely unrelated
	qDebug("Qt: internal: %c(%c) %d %d %d %d",
	       !wrect.intersects(srect) ? '?' : (wrect.contains(srect) ? '*' : '-'),
	       !w->clippedRegion().contains(srect) ? '?' :
	       (!QRegion(w->clippedRegion() ^ srect).isEmpty() ? '*' : '-'),
	       srect.x(), srect.y(), srect.width(), srect.height());
    }
    qDebug("Qt: internal: *****End debug..");
}
static inline void debug_wndw_rgn(const char *where, QWidget *w, const Rect *r, bool clean=FALSE) {
    debug_wndw_rgn(where + QString(" (rect)"), w,
		   QRegion(r->left, r->top, r->right - r->left, r->bottom - r->top), clean);
}
#define qt_dirty_wndw_rgn(x, who, where) do { if(qt_dirty_wndw_rgn_internal(who, where)) \
                                                 debug_wndw_rgn(x, who, where); } while(0);

#define qt_clean_wndw_rgn(x, who, where) do { if(qt_dirty_wndw_rgn_internal(who, where, TRUE)) \
                                                 debug_wndw_rgn(x, who, where, TRUE); } while(0);
#define qt_dirty_wndw_rgn(x, who, where) do { if(qt_dirty_wndw_rgn_internal(who, where)) \
                                                 debug_wndw_rgn(x, who, where); } while(0);
#else
#define clean_wndw_rgn(w, x, y)
#define debug_wndw_rgn(w, x, y)
#define qt_clean_wndw_rgn(x, who, where) qt_dirty_wndw_rgn_internal(who, where, TRUE);
#define qt_dirty_wndw_rgn(x, who, where) qt_dirty_wndw_rgn_internal(who, where);
#endif
static inline bool qt_dirty_wndw_rgn_internal(const QWidget *p, const Rect *r, bool clean=FALSE)
{
    if(qApp->closingDown())
	return FALSE;
    else if(r->right < 0 || r->bottom < 0 || r->left > p->topLevelWidget()->width() ||
	    r->top > p->topLevelWidget()->height())
	return FALSE;
    if(clean) {
	ValidWindowRect((WindowPtr)p->handle(), r);
    } else {
	InvalWindowRect((WindowPtr)p->handle(), r);
	qt_event_request_updates();
    }
    return TRUE;
}
inline bool qt_dirty_wndw_rgn_internal(const QWidget *p, const QRegion &r, bool clean=FALSE)
{
    if(qApp->closingDown())
	return FALSE;
    else if(r.isEmpty())
	return FALSE;
    else if(!r.handle())
	return qt_dirty_wndw_rgn_internal(p, mac_rect(r.boundingRect()), clean);
    if(clean) {
	ValidWindowRgn((WindowPtr)p->handle(), r.handle());
    } else {
	InvalWindowRgn((WindowPtr)p->handle(), r.handle());
	qt_event_request_updates();
    }
    return TRUE;
}

// Paint event clipping magic
extern void qt_set_paintevent_clipping(QPaintDevice* dev, const QRegion& region, QWidget *clipTo);
extern void qt_clear_paintevent_clipping(QPaintDevice *dev);

enum paint_children_ops {
    PC_None = 0x00,
    PC_Now = 0x01,
    PC_ForceErase = 0x02,
    PC_NoPaint = 0x04,
    PC_NoErase = 0x08,
    PC_Later = 0x10
};
bool qt_paint_children(QWidget *p, QRegion &r, uchar ops = PC_None)
{
    if(qApp->closingDown() || qApp->startingUp() || !p || !p->isVisible() || r.isEmpty())
	return FALSE;
    QPoint point(posInWindow(p));
    r.translate(point.x(), point.y());
    r &= p->clippedRegion(FALSE); //at least sanity check the bounds
    if(r.isEmpty())
	return FALSE;

    QObjectList chldrn = p->children();
    for(int i = chldrn.size() - 1; i >= 0; --i) {
	QObject *obj = chldrn.at(i);
	if(obj->isWidgetType()) {
	    QWidget *w = (QWidget *)obj;
	    QRegion clpr = w->clippedRegion(FALSE);
	    if(!clpr.isEmpty() && !w->isTopLevel() &&
	       w->isVisible() && clpr.contains(r.boundingRect())) {
		QRegion wr = clpr & r;
		r -= wr;
		wr.translate(-(point.x() + w->x()), -(point.y() + w->y()));
		qt_paint_children(w, wr, ops);
		if(r.isEmpty())
		    return TRUE;
	    }
	}
    }

    r.translate(-point.x(), -point.y());
    bool erase = !(ops & PC_NoErase);
    if((ops & PC_NoPaint)) {
	if(ops & PC_Later)
	   qDebug("Qt: internal: Cannot use PC_NoPaint with PC_Later!");
	if(erase) {
#ifdef DEBUG_WINDOW_RGN
	    debug_wndw_rgn("**paint_children1", p, r, TRUE, TRUE);
#endif
	    p->erase(r);
	}
    } else {
	if(ops & PC_Now) {
#ifdef DEBUG_WINDOW_RGN
	    debug_wndw_rgn("**paint_children2", p, r, TRUE, TRUE);
#endif
	    p->repaint(r);
	} else {
	    bool painted = FALSE;
	    if(ops & PC_Later); //do nothing
            else if(!p->testWState(QWidget::WState_BlockUpdates)) {
		painted = TRUE;
#ifdef DEBUG_WINDOW_RGN
                debug_wndw_rgn("**paint_children3", p, r, TRUE, TRUE);
#endif
		p->repaint(r);
	    } else if(erase) {
		erase = FALSE;
		p->erase(r);
	    }
	    if(!painted) {
#ifdef DEBUG_WINDOW_RGN
                debug_wndw_rgn("**paint_children4", p, r, TRUE, TRUE);
#endif
		p->update(r); //last try
	    } else if(p->d->extraData() && p->d->extraData()->has_dirty_area) {
#ifdef DEBUG_WINDOW_RGN
                debug_wndw_rgn("**paint_children5", p, r, TRUE, TRUE);
#endif
		qt_event_request_updates(p, r, TRUE);
	    }
	}
    }
    return FALSE;
}

static QPtrList<QWidget> qt_root_win_widgets;
static WindowPtr qt_root_win = NULL;
void qt_clean_root_win() {
    for(QPtrListIterator<QWidget> it(qt_root_win_widgets); it.current(); ++it) {
	if((*it)->hd == qt_root_win) {
#if 0
	    warning("%s:%d: %s (%s) had his handle taken away!", __FILE__, __LINE__,
		     (*it)->name(), (*it)->className());
#endif
	    (*it)->hd = NULL; //at least now we'll just crash
	}
    }
    DisposeWindow(qt_root_win);
    qt_root_win = NULL;
}
static bool qt_create_root_win() {
    if(qt_root_win)
	return FALSE;
#ifdef Q_WS_MAC9
    //FIXME NEED TO FIGURE OUT HOW TO GET DESKTOP
    //GetCWMgrPort(ret);
#else if defined(Q_WS_MACX)
    Rect r;
    int w = 0, h = 0;
    for(GDHandle g = GetMainDevice(); g; g = GetNextDevice(g)) {
	w = QMAX(w, (*g)->gdRect.right);
	h = QMAX(h, (*g)->gdRect.bottom);
    }
    SetRect(&r, 0, 0, w, h);
    CreateNewWindow(kOverlayWindowClass, kWindowNoAttributes, &r, &qt_root_win);
#endif //MACX
    if(!qt_root_win)
	return FALSE;
    qAddPostRoutine(qt_clean_root_win);
    return TRUE;
}
bool qt_recreate_root_win() {
    if(!qt_root_win)
	return FALSE;
    WindowPtr old_root_win = qt_root_win;
    qt_root_win = NULL;
    qt_create_root_win();
    for(QPtrListIterator<QWidget> it(qt_root_win_widgets); it.current(); ++it) {
	if((*it)->hd == old_root_win) {
	    (*it)->macWidgetChangedWindow();
	    (*it)->hd = qt_root_win;
	}
    }
    //cleanup old window
    DisposeWindow(old_root_win);
    return TRUE;
}

bool qt_window_rgn(WId id, short wcode, RgnHandle rgn, bool force = FALSE)
{
    QWidget *widget = QWidget::find((WId)id);
    if(qMacVersion() == Qt::MV_JAGUAR) {
	switch(wcode) {
	case kWindowOpaqueRgn:
	case kWindowStructureRgn: {
	    if(widget) {
		int x, y;
		{ //lookup the x and y, don't use qwidget because this callback can be called before its updated
		    Point px = { 0, 0 };
		    QMacSavedPortInfo si(widget);
		    LocalToGlobal(&px);
		    x = px.h;
		    y = px.v;
		}

		QWExtra *extra = widget->d->extraData();
		if(extra && !extra->mask.isEmpty()) {
		    QRegion titlebar;
		    {
			RgnHandle rgn = qt_mac_get_rgn();
			GetWindowRegion((WindowPtr)widget->handle(), kWindowTitleBarRgn, rgn);
			titlebar = qt_mac_convert_mac_region(rgn);
			qt_mac_dispose_rgn(rgn);
		    }
		    QRegion rpm = extra->mask;
		    /* This is a gross hack, something is weird with how the Mac is handling this region.
		       clearly the first paintable pixel is becoming 0,0 of this region, so to compensate
		       I just force 0,0 to be on - that way I know the region is offset like I want. Of
		       course it also means another pixel is showing that the user didn't mean to :( FIXME */
		    if(!rpm.contains(QPoint(0, 0)))
			rpm |= QRegion(0, 0, 1, 1);
		    rpm.translate(x, (y + titlebar.boundingRect().height()));
		    titlebar += rpm;
		    CopyRgn(titlebar.handle(TRUE), rgn);
		} else if(force) {
		    QRegion cr(x, y, widget->width(), widget->height());
		    CopyRgn(cr.handle(TRUE), rgn);
		}
	    }
	    return TRUE; }
	case kWindowContentRgn: {
	    if(widget) {
		int x, y;
		{ //lookup the x and y, don't use qwidget because this callback can be called before its updated
		    Point px = { 0, 0 };
		    QMacSavedPortInfo si(widget);
		    LocalToGlobal(&px);
		    x = px.h;
		    y = px.v;
		}

		QWExtra *extra = widget->d->extraData();
		if(extra && !extra->mask.isEmpty()) {
		    QRegion rpm = extra->mask;
		    rpm.translate(x, y);
		    CopyRgn(rpm.handle(TRUE), rgn);
		} else if(force) {
		    QRegion cr(x, y, widget->width(), widget->height());
		    CopyRgn(cr.handle(TRUE), rgn);
		}
	    }
	    return TRUE; }
	default: break;
	}
    } else {
	switch(wcode) {
	case kWindowOpaqueRgn:
	case kWindowStructureRgn: {
	    QRegion cr;
	    if(widget) {
		QWExtra *extra = widget->d->extraData();
		if(extra && !extra->mask.isEmpty()) {
		    QRegion rin = qt_mac_convert_mac_region(rgn);
		    QPoint g(widget->x(), widget->y());
		    int offx = 0, offy = 0;
		    QRegion rpm = extra->mask;
		    if(widget->testWFlags(Qt::WStyle_Customize) &&
		       widget->testWFlags(Qt::WStyle_NoBorder)) {
			QPoint rpm_tl = rpm.boundingRect().topLeft();
			offx = rpm_tl.x();
			offy = rpm_tl.y();
		    } else {
			Rect title_r;
			RgnHandle rgn = qt_mac_get_rgn();
			GetWindowRegion((WindowPtr)widget->handle(), kWindowTitleBarRgn, rgn);
			GetRegionBounds(rgn, &title_r);
			qt_mac_dispose_rgn(rgn);
			g.setY(g.y() + (title_r.bottom - title_r.top));
		    }
		    rin -= QRegion(g.x() + offx, g.y() + offy, widget->width(), widget->height());
		    rpm.translate(g.x(), g.y());
		    rin += rpm;
		    CopyRgn(rin.handle(TRUE), rgn);
		} else if(force) {
		    QRegion cr(widget->geometry());
		    CopyRgn(cr.handle(TRUE), rgn);
		}
	    }
	    return TRUE; }
	case kWindowContentRgn: {
	    if(widget) {
		QWExtra *extra = widget->d->extraData();
		if(extra && !extra->mask.isEmpty()) {
		    QRegion cr = extra->mask;
		    QPoint g(widget->x(), widget->y());
		    if(!widget->testWFlags(Qt::WStyle_Customize) ||
		       !widget->testWFlags(Qt::WStyle_NoBorder)) {
			Rect title_r;
			RgnHandle rgn = qt_mac_get_rgn();
			GetWindowRegion((WindowPtr)widget->handle(), kWindowTitleBarRgn, rgn);
			GetRegionBounds(rgn, &title_r);
			qt_mac_dispose_rgn(rgn);
			g.setY(g.y() + (title_r.bottom - title_r.top));
		    }
		    cr.translate(g.x(), g.y());
		    CopyRgn(cr.handle(TRUE), rgn);

		} else if(force) {
		    QRegion cr(widget->geometry());
		    CopyRgn(cr.handle(TRUE), rgn);
		}
	    }
	    return TRUE; }
	default: break;
	}
    }
    return FALSE;
}

QMAC_PASCAL OSStatus qt_window_event(EventHandlerCallRef er, EventRef event, void *)
{
    UInt32 ekind = GetEventKind(event), eclass = GetEventClass(event);
    if(eclass == kEventClassWindow) {
	WindowRef wid;
	GetEventParameter(event, kEventParamDirectObject, typeWindowRef, NULL,
			  sizeof(WindowRef), NULL, &wid);
	switch(ekind) {
	case kEventWindowGetRegion: {
	    CallNextEventHandler(er, event);
	    WindowRegionCode wcode;
	    GetEventParameter(event, kEventParamWindowRegionCode, typeWindowRegionCode, NULL,
			      sizeof(wcode), NULL, &wcode);
	    RgnHandle rgn;
	    GetEventParameter(event, kEventParamRgnHandle, typeQDRgnHandle, NULL,
			      sizeof(rgn), NULL, &rgn);
	    qt_window_rgn((WId)wid, wcode, rgn, FALSE);
	    return noErr; }
	case kEventWindowDrawContent: {
	    if(QWidget *widget = QWidget::find((WId)wid)) {
		widget->propagateUpdates(FALSE);
		return noErr;
	    }
	    break; }
	}
    } else if(eclass == kEventClassMouse && ekind == kEventMouseDown) {
	Point where;
	GetEventParameter(event, kEventParamMouseLocation, typeQDPoint, NULL,
			  sizeof(where), NULL, &where);
	bool ok;
	UInt32 count;
	GetEventParameter(event, kEventParamClickCount, typeUInt32, NULL,
			  sizeof(count), NULL, &count);
	if(count == 1 && !qApp->do_mouse_down(&where, &ok)) {
	    if(!ok)
		return noErr;
	}
    }
    return eventNotHandledErr;
}
static EventTypeSpec window_events[] = {
    { kEventClassWindow, kEventWindowGetRegion },
    { kEventClassMouse, kEventMouseDown }
};
static EventHandlerUPP mac_win_eventUPP = NULL;
static void cleanup_win_eventUPP()
{
    DisposeEventHandlerUPP(mac_win_eventUPP);
    mac_win_eventUPP = NULL;
}
static const EventHandlerUPP make_win_eventUPP()
{
    if(mac_win_eventUPP)
	return mac_win_eventUPP;
    qAddPostRoutine(cleanup_win_eventUPP);
    return mac_win_eventUPP = NewEventHandlerUPP(qt_window_event);
}

//#define QMAC_USE_WDEF
#ifdef QMAC_USE_WDEF
static QMAC_PASCAL long qt_wdef(short, WindowRef window, short message, long param)
{
    long result = 0;
    switch (message) {
    case kWindowMsgHitTest:
	result = wInContent;
	break;
    case kWindowMsgStateChanged:
    case kWindowMsgCleanUp:
    case kWindowMsgInitialize:
    case kWindowMsgDrawInCurrentPort:
    case kWindowMsgDraw:
	result = 0;
	break;
    case kWindowMsgGetFeatures: {
	SInt32 *s = (SInt32*)param;
	*s = kWindowCanGetWindowRegion;
	result = 1;
	break; }
    case kWindowMsgGetRegion: {
	GetWindowRegionRec *s = (GetWindowRegionRec *)param;
	if(qt_window_rgn((WId)window, s->regionCode, s->winRgn, TRUE))
	    result = 0;
	else
	    result = errWindowRegionCodeInvalid;
	break; }
    default:
	qDebug("Qt: internal: Shouldn't happen %s:%d %d", __FILE__, __LINE__, message);
	break;
    }
    return result;
}
#endif

QMAC_PASCAL OSStatus qt_erase(GDHandle, GrafPtr, WindowRef window, RgnHandle rgn,
			 RgnHandle outRgn, void *w)
{
    QWidget *widget = (QWidget *)w;
    if(!widget)
	widget = QWidget::find((WId)window);
    if(widget) {
	QRegion reg = qt_mac_convert_mac_region(rgn);
	{ //lookup the x and y, don't use qwidget because this callback can be called before its updated
	    Point px = { 0, 0 };
	    QMacSavedPortInfo si(widget);
	    LocalToGlobal(&px);
	    reg.translate(-px.h, -px.v);
	}
	//Clear a nobackground widget to make it transparent
	if(!widget->testAttribute(QWidget::WA_NoSystemBackground)) {
	    CGContextRef ctx;
	    CGRect r2 = CGRectMake(0, 0, widget->width(), widget->height());
	    CreateCGContextForPort(GetWindowPort((WindowPtr)widget->handle()), &ctx);
	    CGContextClearRect(ctx, r2);
	    CGContextFlush(ctx);
	    CGContextRelease(ctx);
	}
	qt_paint_children(widget, reg, PC_NoPaint | PC_ForceErase);
	QMacSavedPortInfo::flush(widget);
    } else {
	CopyRgn(rgn, outRgn);  //We don't know the widget, so let the Mac do the erasing..
    }
    return 0;
}

bool qt_mac_is_macdrawer(QWidget *w)
{
#if defined(Q_WS_MACX) && 0
    if(w && w->isTopLevel() && w->parentWidget() && w->testWFlags(Qt::WMacDrawer))
	return TRUE;
#else
    Q_UNUSED(w);
#endif
    return FALSE;
}

bool qt_mac_set_drawer_preferred_edge(QWidget *w, Qt::Dock where) //users of Qt/Mac can use this..
{
#if QT_MACOSX_VERSION >= 0x1020
    if(!qt_mac_is_macdrawer(w))
	return FALSE;
    OptionBits bits;
    if(where == Qt::DockTop)
	bits = kWindowEdgeTop;
    else if(where == Qt::DockLeft)
	bits = kWindowEdgeLeft;
    else if(where == Qt::DockRight)
	bits = kWindowEdgeRight;
    else if(where == Qt::DockBottom)
	bits = kWindowEdgeBottom;
    else
	return FALSE;
    SetDrawerPreferredEdge((WindowRef)w->handle(), bits);
    return TRUE;
#else
    return FALSE;
#endif
}

bool qt_mac_is_macsheet(QWidget *w, bool ignore_exclusion=FALSE)
{
#if defined(Q_WS_MACX) && 0
    if(w && w->isTopLevel() && w->testWFlags(Qt::WStyle_DialogBorder) &&
       (ignore_exclusion || !w->testWFlags(Qt::WMacNotSheet)) &&
       w->parentWidget() && !w->parentWidget()->topLevelWidget()->isDesktop() &&
       w->parentWidget()->topLevelWidget()->isVisible() &&
       (w->style().inherits("QAquaStyle") || w->style().inherits("QMacStyle")))
	return TRUE;
#else
    Q_UNUSED(w);
    Q_UNUSED(ignore_exclusion);
#endif
    return FALSE;
}

/*****************************************************************************
  QWidget member functions
 *****************************************************************************/
void QWidget::create(WId window, bool initializeWindow, bool destroyOldWindow)
{
    window_event = NULL;
    own_id = 0;
    HANDLE destroyw = 0;
    setWState(WState_Created);                        // set created flag

    if(!parentWidget() || parentWidget()->isDesktop())
	setWFlags(WType_TopLevel);            // top-level widget

    QRect dskr;
    if(isDesktop()) {
	int w = 0, h = 0;
	for(GDHandle g = GetMainDevice(); g; g = GetNextDevice(g)) {
	    w = QMAX(w, (*g)->gdRect.right);
	    h = QMAX(h, (*g)->gdRect.bottom);
	}
	dskr = QRect(0, 0, w, h);
    } else {
	if(QDesktopWidget *dsk = QApplication::desktop()) {
	    int deskn = dsk->primaryScreen();
	    if(parentWidget() && !parentWidget()->isDesktop())
		deskn = dsk->screenNumber(parentWidget());
	    dskr = dsk->screenGeometry(deskn);
	}
    }
    int sw = dskr.width(), sh = dskr.height();                // screen size
    bool topLevel = testWFlags(WType_TopLevel);
    bool popup = testWFlags(WType_Popup);
    bool dialog = testWFlags(WType_Dialog);
    bool desktop = testWFlags(WType_Desktop);
    WId    id;

    if(!window)                              // always initialize
	initializeWindow=TRUE;

    if(dialog || popup || desktop) {          // these are top-level, too
	topLevel = TRUE;
	setWFlags(WType_TopLevel);
	if(popup)
	    setWFlags(WStyle_Tool); // a popup is a tool window
    }
    if(topLevel && parentWidget()) {
	// if our parent has WStyle_StaysOnTop, so must we
	QWidget *ptl = parentWidget()->topLevelWidget();
	if(ptl && ptl->testWFlags(WStyle_StaysOnTop))
	    setWFlags(WStyle_StaysOnTop);
    }
    if(dialog && !testWFlags(WShowModal) && parentWidget() && parentWidget()->testWFlags(WShowModal))
	setWFlags(WShowModal);
    if(!testWFlags(WStyle_Customize) && !(desktop || popup) && !testWFlags(WShowModal))
	setWFlags(WStyle_Customize | WStyle_NormalBorder | WStyle_Title | WStyle_MinMax | WStyle_SysMenu);

    if(desktop) {                            // desktop widget
	dialog = popup = FALSE;                  // force these flags off
	crect.setRect(0, 0, sw, sh);
    } else if(topLevel) {                    // calc pos/size from screen
	crect.setRect(sw/4, 3*sh/10, sw/2, 4*sh/10);
    } else {                                    // child widget
	crect.setRect(0, 0, 100, 30);
    }

    if(window) {				// override the old window
	if(destroyOldWindow && own_id)
	    destroyw = hd;
	own_id = 0; //it has become mine!
	id = window;
	hd = (void *)id;
	macWidgetChangedWindow();
	setWinId(id);
    } else if(desktop) {			// desktop widget
	if(!qt_root_win)
	    qt_create_root_win();
	qt_root_win_widgets.append(this);
	hd = (void *)qt_root_win;
	macWidgetChangedWindow();
	id = (WId)hd;
	own_id = 0;
	setWinId(id);
    } else if(isTopLevel()) {
	own_id = 1; //I created it, I own it

	Rect r;
	SetRect(&r, crect.left(), crect.top(), crect.left(), crect.top());
	WindowClass wclass = kSheetWindowClass;
	if(popup || testWFlags(WStyle_Tool))
	    wclass = kModalWindowClass;
	else if(testWFlags(WShowModal))
	    wclass = kMovableModalWindowClass;
#if QT_MACOSX_VERSION >= 0x1020
	else if(qt_mac_is_macdrawer(this))
	    wclass = kDrawerWindowClass;
#endif
	else if(dialog && parentWidget() && !parentWidget()->topLevelWidget()->isDesktop())
	    wclass = kFloatingWindowClass;
	else if(dialog)
	    wclass = kToolbarWindowClass;
	else
	    wclass = kDocumentWindowClass;

	WindowGroupRef grp = NULL;
	WindowAttributes wattr = kWindowNoAttributes;
	if(testWFlags(WStyle_Customize)) {
	    if(qt_mac_is_macsheet(this)) {
		grp = GetWindowGroupOfClass(kMovableModalWindowClass);
		wclass = kSheetWindowClass;
	    } else {
		grp = GetWindowGroupOfClass(wclass);
		if(testWFlags(WStyle_NoBorder)) {
		    if(wclass == kDocumentWindowClass)
			wclass = kPlainWindowClass;
		    else if(wclass == kFloatingWindowClass)
			wclass = kToolbarWindowClass;
		} else {
		    if(wclass != kModalWindowClass)
			wattr |= kWindowResizableAttribute;
		}
		if(testWFlags(WStyle_NormalBorder) || testWFlags(WStyle_DialogBorder)) {
		    if(wclass == kToolbarWindowClass)
			wclass = kFloatingWindowClass;
		    if(wclass == kDocumentWindowClass)
			wattr |= kWindowStandardDocumentAttributes;
		    if(wclass == kFloatingWindowClass)
			wattr |= kWindowStandardFloatingAttributes;
		}
		if(wclass != kModalWindowClass && wclass != kMovableModalWindowClass && //no choice for these..
		   wclass != kSheetWindowClass && wclass != kPlainWindowClass) {
		    if(testWFlags(WStyle_Maximize))
			wattr |= kWindowFullZoomAttribute;
		    if(testWFlags(WStyle_Minimize))
			wattr |= kWindowCollapseBoxAttribute;
		    if(testWFlags(WStyle_Title) || testWFlags(WStyle_SysMenu))
		       wattr |= kWindowCloseBoxAttribute;
		}
	    }
	}
	wattr |= kWindowLiveResizeAttribute;

#ifdef DEBUG_WINDOW_CREATE
#define ADD_DEBUG_WINDOW_NAME(x) { x, #x }
	struct {
	    UInt32 tag;
	    const char *name;
	} known_attribs[] = {
	    ADD_DEBUG_WINDOW_NAME(kWindowStandardHandlerAttribute),
	    ADD_DEBUG_WINDOW_NAME(kWindowCollapseBoxAttribute),
	    ADD_DEBUG_WINDOW_NAME(kWindowHorizontalZoomAttribute),
	    ADD_DEBUG_WINDOW_NAME(kWindowVerticalZoomAttribute),
	    ADD_DEBUG_WINDOW_NAME(kWindowResizableAttribute),
	    ADD_DEBUG_WINDOW_NAME(kWindowNoActivatesAttribute),
	    ADD_DEBUG_WINDOW_NAME(kWindowLiveResizeAttribute),
	    ADD_DEBUG_WINDOW_NAME(kWindowCloseBoxAttribute),
	    { 0, NULL }
	}, known_classes[] = {
	    ADD_DEBUG_WINDOW_NAME(kPlainWindowClass),
	    ADD_DEBUG_WINDOW_NAME(kDrawerWindowClass),
	    ADD_DEBUG_WINDOW_NAME(kUtilityWindowClass),
	    ADD_DEBUG_WINDOW_NAME(kToolbarWindowClass),
	    ADD_DEBUG_WINDOW_NAME(kSheetWindowClass),
	    ADD_DEBUG_WINDOW_NAME(kFloatingWindowClass),
	    ADD_DEBUG_WINDOW_NAME(kDocumentWindowClass),
	    ADD_DEBUG_WINDOW_NAME(kToolbarWindowClass),
	    ADD_DEBUG_WINDOW_NAME(kMovableModalWindowClass),
	    ADD_DEBUG_WINDOW_NAME(kModalWindowClass),
	    { 0, NULL }
	};
#undef ADD_DEBUG_WINDOW_NAME
	qDebug("Qt: internal: ************* Creating new window (%s::%s)", className(), name());
	bool found_class = FALSE;
	for(int i = 0; known_classes[i].name; i++) {
	    if(wclass == known_classes[i].tag) {
		found_class = TRUE;
		qDebug("Qt: internal: ** Class: %s", known_classes[i].name);
		break;
	    }
	}
	if(!found_class)
	    qDebug("Qt: internal: !! Class: Unknown! (%d)", (int)wclass);
	if(wattr) {
	    WindowAttributes tmp_wattr = wattr;
	    qDebug("Qt: internal: ** Attributes:");
	    for(int i = 0; tmp_wattr && known_attribs[i].name; i++) {
		if((tmp_wattr & known_attribs[i].tag) == known_attribs[i].tag) {
		    tmp_wattr ^= known_attribs[i].tag;
		    qDebug("Qt: internal: * %s %s", known_attribs[i].name,
			   (GetAvailableWindowAttributes(wclass) & known_attribs[i].tag) ? "" : "(*)");
		}
	    }
	    if(tmp_wattr)
		qDebug("Qt: internal: !! Attributes: Unknown (%d)", (int)tmp_wattr);
	}
#endif

	/* Just to be extra careful we will change to the kUtilityWindowClass if the
	   requested attributes cannot be used */
	if((GetAvailableWindowAttributes(wclass) & wattr) != wattr) {
	    WindowClass tmp_class = wclass;
	    if(wclass == kToolbarWindowClass || wclass == kUtilityWindowClass)
		wclass = kFloatingWindowClass;
	    if(tmp_class != wclass) {
		if(!grp)
		    grp = GetWindowGroupOfClass(wclass);
		wclass = tmp_class;
	    }
	}

#ifdef QMAC_USE_WDEF
	if((wclass == kPlainWindowClass && wattr == kWindowNoAttributes) || testWFlags(WStyle_Tool)) {
	    WindowDefSpec wds;
	    wds.defType = kWindowDefProcPtr;
	    wds.u.defProc = NewWindowDefUPP(qt_wdef);
	    CreateCustomWindow(&wds, wclass, wattr, &r, (WindowRef *)&id);
	} else
#endif
	{
	    if(OSStatus ret = CreateNewWindow(wclass, wattr, &r, (WindowRef *)&id))
		qDebug("Qt: internal: %s:%d If you reach this error please contact Trolltech and include the\n"
		       "      WidgetFlags used in creating the widget (%ld)", __FILE__, __LINE__, ret);
	    if(!desktop) { 	//setup an event callback handler on the window
		InstallWindowEventHandler((WindowRef)id, make_win_eventUPP(),
					  GetEventTypeCount(window_events),
					  window_events, (void *)qApp, &window_event);
	    }
	}

	if(wclass == kFloatingWindowClass) //these dialogs don't hide
	    ChangeWindowAttributes((WindowRef)id, 0, kWindowHideOnSuspendAttribute |
				   kWindowNoActivatesAttribute);
#if QT_MACOSX_VERSION >= 0x1020
	if(qt_mac_is_macdrawer(this))
	    SetDrawerParent((WindowRef)id, (WindowRef)parentWidget()->handle());
#endif
	if(dialog && !parentWidget() && !testWFlags(WShowModal))
	    grp = GetWindowGroupOfClass(kDocumentWindowClass);
#ifdef Q_WS_MACX
	if(testWFlags(WStyle_StaysOnTop)) {
	    d->createTLExtra();
	    if(d->topData()->group)
		ReleaseWindowGroup(d->topData()->group);
	    CreateWindowGroup(kWindowActivationScopeNone, &d->topData()->group);
	    SetWindowGroupLevel(d->topData()->group, kCGMaximumWindowLevel);
	    SetWindowGroupParent(d->topData()->group, GetWindowGroupOfClass(kAllWindowClasses));
	    SetWindowGroup((WindowPtr)id, d->topData()->group);
	} else if(grp) {
	    SetWindowGroup((WindowPtr)id, grp);
	}
#ifdef DEBUG_WINDOW_CREATE
	if(WindowGroupRef grpf = GetWindowGroup((WindowPtr)id)) {
	    CFStringRef cfname;
	    CopyWindowGroupName(grpf, &cfname);
	    SInt32 lvl;
	    GetWindowGroupLevel(grpf, &lvl);
	    const char *from = "Default";
	    if(d->topData() && grpf == d->topData()->group)
		from = "Created";
	    else if(grpf == grp)
		from = "Copied";
	    qDebug("Qt: internal: With window group '%s' [%p] @ %d: %s",
		   cfstring2qstring(cfname).latin1(), grpf, (int)lvl, from);
	} else {
	    qDebug("Qt: internal: No window group!!!");
	}
#endif
#endif
#if 0
	//We cannot use a window content paint proc because it causes problems on 10.2 (it
	//is buggy). We have an outstanding issue with Apple right now.
	InstallWindowContentPaintProc((WindowPtr)id, NewWindowPaintUPP(qt_erase), 0, this);
#endif
	if(testWFlags(WType_Popup) || testWFlags(WStyle_Tool))
	    SetWindowModality((WindowPtr)id, kWindowModalityNone, NULL);
	fstrut_dirty = TRUE; // when we create a toplevel widget, the frame strut should be dirty
	if(!mac_window_count++)
	    QMacSavedPortInfo::setPaintDevice(this);
	hd = (void *)id;
	macWidgetChangedWindow();
	setWinId(id);
	ReshapeCustomWindow((WindowPtr)hd);
	if(qt_mac_is_macsheet(this))
	    QMacSavedPortInfo::setAlphaTransparency(this, 0.85);
#if QT_MACOSX_VERSION >= 0x1020
	else if(qt_mac_is_macdrawer(this))
	    SetDrawerOffsets((WindowPtr)hd, 0.0, 25.0);
#endif
    } else {
	while(QWidget::find(++serial_id));
	setWinId(serial_id);
	id = serial_id;
	hd = topLevelWidget()->hd;
	macWidgetChangedWindow();
	fstrut_dirty = FALSE; // non-toplevel widgets don't have a frame, so no need to update the strut
	setWinId(id);
    }

    if(desktop) { //immediately "show" a "desktop"
	setWState(WState_Visible);
    } else {
	clearWState(WState_Visible);
	dirtyClippedRegion(TRUE);
    }
    macDropEnabled = false;

    if(destroyw) {
	mac_window_count--;
	DisposeWindow((WindowPtr)destroyw);
    }
    qt_mac_unicode_init(this);
}

void QWidget::destroy(bool destroyWindow, bool destroySubWindows)
{
    deactivateWidgetCleanup();
    qt_mac_unicode_cleanup(this);
    if(isDesktop() && hd == qt_root_win && destroyWindow && own_id)
	qt_root_win_widgets.removeRef(this);
    if(testWState(WState_Created)) {
	dirtyClippedRegion(TRUE);
	if(isVisible())
	    qt_dirty_wndw_rgn("destroy",this, mac_rect(posInWindow(this), geometry().size()));
        clearWState(WState_Created);
	QObjectList chldrn = children();
	for(int i = 0; i < chldrn.size(); i++) {  // destroy all widget children
	    QObject *obj = chldrn.at(i);
	    if(obj->isWidgetType())
		((QWidget*)obj)->destroy(destroySubWindows, destroySubWindows);
        }
	if(mac_mouse_grabber == this)
	    releaseMouse();
	if(mac_keyboard_grabber == this)
	    releaseKeyboard();
	if(acceptDrops())
	    setAcceptDrops(FALSE);

        if(testWFlags(WShowModal))          // just be sure we leave modal
            qt_leave_modal(this);
        else if(testWFlags(WType_Popup))
            qApp->closePopup(this);
	if(destroyWindow && isTopLevel() && hd && own_id) {
	    mac_window_count--;
	    if(window_event) {
		RemoveEventHandler(window_event);
		window_event = NULL;
	    }
	    DisposeWindow((WindowPtr)hd);
	}
    }
    hd=0;
    setWinId(0);
}

void QWidget::reparent_helper(QWidget *parent, WFlags f, const QPoint &p, bool showIt)
{
    if(isVisible() && !isTopLevel())
	qt_dirty_wndw_rgn("reparent1", parentWidget() ? parentWidget() : this,
			  mac_rect(posInWindow(this), geometry().size()));
    dirtyClippedRegion(TRUE);

    QCursor oldcurs;
    bool setcurs=testAttribute(WA_SetCursor);
    if(setcurs) {
	oldcurs = cursor();
	unsetCursor();
    }

    EventHandlerRef old_window_event = NULL;
    WindowPtr old_hd = NULL;
    if(!isDesktop()) {
	old_hd = (WindowPtr)hd;
	old_window_event = window_event;
    }
    QWidget* oldtlw = topLevelWidget();
    reparentFocusWidgets(parent);		// fix focus chains

    setWinId(0);
    QObject::setParent_helper(parent);
    bool     dropable = acceptDrops();
    bool     enable = isEnabled();
    bool     owned = own_id;
    FocusPolicy fp = focusPolicy();
    QSize    s	    = size();
    QString capt= caption();
    widget_flags = f;
    clearWState(WState_Created | WState_Visible | WState_Hidden | WState_ExplicitShowHide);
    create();
    if(isTopLevel() || (!parent || parent->isVisible()))
	setWState(WState_Hidden);
    if(dropable)
	setAcceptDrops(FALSE);

    //get new hd, now move
    no_move_blt = TRUE;
    setGeometry(p.x(), p.y(), s.width(), s.height());
    no_move_blt = FALSE;

    //reset flags and show (if neccesary)
    setEnabled(enable);
    setFocusPolicy(fp);
    setAcceptDrops(dropable);
    if(!capt.isNull()) {
	d->topData()->caption = QString::null;
	setCaption(capt);
    }
    if(showIt)
	show();
    if(setcurs)
	setCursor(oldcurs);

    //reparent children
    QObjectList chldrn = queryList();
    for(int i = 0; i < chldrn.size(); i++) {
	QObject *obj = chldrn.at(i);
	if(obj->isWidgetType()) {
	    QWidget *w = (QWidget *)obj;
	    if(((WindowPtr)w->hd) == old_hd) {
		w->hd = hd; //all my children hd's are now mine!
		w->macWidgetChangedWindow();
	    }
	}
    }
    reparentFocusWidgets(oldtlw);

    //repaint the new area, on the window parent
    if(isVisible()) //finally paint my new area
	qt_dirty_wndw_rgn("reparent2",this, mac_rect(posInWindow(this), geometry().size()));

    //send the reparent event
    if(old_hd && owned) { //don't need old window anymore
	mac_window_count--;
	if(old_window_event) {
	    RemoveEventHandler(old_window_event);
	    old_window_event = NULL;
	}
	DisposeWindow(old_hd);
    }
}

QPoint QWidget::mapToGlobal(const QPoint &pos) const
{
    Point mac_p;
    QPoint mp(posInWindow(((QWidget *)this)));
    mac_p.h = mp.x() + pos.x();
    mac_p.v = mp.y() + pos.y();
    if(handle()) {
	QMacSavedPortInfo savedInfo(((QWidget *)this));
	LocalToGlobal(&mac_p);
    }
    return QPoint(mac_p.h, mac_p.v);
}

QPoint QWidget::mapFromGlobal(const QPoint &pos) const
{
    Point mac_p;
    mac_p.h = pos.x();
    mac_p.v = pos.y();
    if(handle()) {
	QMacSavedPortInfo savedInfo(((QWidget *)this));
	GlobalToLocal(&mac_p);
    }
    for(const QWidget *p = this; p && !p->isTopLevel(); p = p->parentWidget(TRUE)) {
	mac_p.h -= p->geometry().x();
	mac_p.v -= p->geometry().y();
    }
    return QPoint(mac_p.h, mac_p.v);
}

void QWidget::setMicroFocusHint(int x, int y, int width, int height, bool, QFont *)
{
    if ( QRect( x, y, width, height ) != microFocusHint() ) {
	createExtra();
	extraData()->micro_focus_hint.setRect( x, y, width, height );
    }
}

void QWidget::setFontSys(QFont *)
{
}

void QWidgetPrivate::updateSystemBackground()
{
    QBrush brush = q->palette().brush(q->backgroundRole());
    if(!brush.pixmap() && q->isTopLevel()) {
	QMacSavedPortInfo savedInfo(q);
	RGBColor f;
	f.red = brush.color().red() * 256;
	f.green = brush.color().green() * 256;;
	f.blue = brush.color().blue() * 256;
	RGBBackColor(&f);
    }
}


void QWidget::setCursor(const QCursor &cursor)
{
    d->createExtra();
    delete d->extraData()->curs;
    d->extraData()->curs = new QCursor(cursor);
    setAttribute(WA_SetCursor);

    if(qApp && qApp->activeWindow() &&
       QApplication::widgetAt(QCursor::pos(), TRUE) == this) {
	Point mouse_pos;
	QPoint qmp(QCursor::pos());
	mouse_pos.h = qmp.x();
	mouse_pos.v = qmp.y();

	const QCursor *n = &cursor;
	if(QApplication::overrideCursor())
	    n = QApplication::overrideCursor();
	qt_mac_set_cursor(n, &mouse_pos);
    }
}

void QWidget::unsetCursor()
{
    if(!isTopLevel()) {
	if(QWExtra *extra = d->extraData()) {
	    delete extra->curs;
	    extra->curs = 0;
	}
	setAttribute(WA_SetCursor, false);
    }

    if(qApp && qApp->activeWindow() &&
       QApplication::widgetAt(QCursor::pos(), TRUE) == this) {
	Point mouse_pos;
	QPoint qmp(QCursor::pos());
	mouse_pos.h = qmp.x();
	mouse_pos.v = qmp.y();

	const QCursor *n = NULL;
	if(QApplication::overrideCursor()) {
	    n = QApplication::overrideCursor();
	} else {
	    for(QWidget *p = this; p; p = p->parentWidget()) {
		QWExtra *extra = p->d->extraData();
		if(extra && extra->curs) {
		    n = extra->curs;
		    break;
		}
	    }
	}
	const QCursor def(Qt::ArrowCursor);
	if(!n) n = &def; //I give up..
	qt_mac_set_cursor(n, &mouse_pos);
    }
}

void QWidget::setCaption(const QString &cap)
{
    if(d->topData() && d->topData()->caption == cap)
	return; // for less flicker
    d->createTLExtra();
    d->topData()->caption = cap;
    if(isTopLevel()) {
	CFStringRef str = CFStringCreateWithCharacters(NULL, (UniChar *)cap.unicode(), cap.length());
	SetWindowTitleWithCFString((WindowPtr)hd, str);
    }
    QEvent e(QEvent::CaptionChange);
    QApplication::sendEvent(this, &e);
}

void QWidget::setIcon(const QPixmap &pixmap)
{
    if(d->topData()) {
	delete d->topData()->icon;
	d->topData()->icon = 0;
    } else {
	d->createTLExtra();
    }
    if(!pixmap.isNull())
	d->topData()->icon = new QPixmap(pixmap);
#ifdef Q_WS_MACX
    if(isTopLevel()) {
	if(qApp && qApp->mainWidget() == this) {
	    if(pixmap.isNull()) {
		RestoreApplicationDockTileImage();
	    } else {
		QImage i = pixmap.convertToImage().convertDepth(32).smoothScale(40, 40);
		for(int y = 0; y < i.height(); y++) {
		    uchar *l = i.scanLine(y);
		    for(int x = 0; x < i.width(); x+=4)
			*(l+x) = 255;
		}
		CGColorSpaceRef cs = CGColorSpaceCreateDeviceRGB();
		CGDataProviderRef dp = CGDataProviderCreateWithData(NULL, i.bits(), i.numBytes(), NULL);
		CGImageRef ir = CGImageCreate(i.width(), i.height(), 8, 32, i.bytesPerLine(),
					      cs, kCGImageAlphaNoneSkipFirst, dp,
					      0, 0, kCGRenderingIntentDefault);
		//cleanup
		SetApplicationDockTileImage(ir);
		CGImageRelease(ir);
		CGColorSpaceRelease(cs);
		CGDataProviderRelease(dp);
	    }
	}
    } else {

    }
#endif
}

void QWidget::setIconText(const QString &iconText)
{
    d->createTLExtra();
    d->topData()->iconText = iconText;
}

void QWidget::grabMouse()
{
    if(isVisible() && !qt_nograb()) {
	if(mac_mouse_grabber)
	    mac_mouse_grabber->releaseMouse();
	mac_mouse_grabber=this;
    }
}

void QWidget::grabMouse(const QCursor &)
{
    if(isVisible() && !qt_nograb()) {
	if(mac_mouse_grabber)
	    mac_mouse_grabber->releaseMouse();
	mac_mouse_grabber=this;
    }
}

void QWidget::releaseMouse()
{
    if(!qt_nograb() && mac_mouse_grabber == this)
	mac_mouse_grabber = NULL;
}

void QWidget::grabKeyboard()
{
    if(!qt_nograb()) {
	if(mac_keyboard_grabber)
	    mac_keyboard_grabber->releaseKeyboard();
	mac_keyboard_grabber = this;
    }
}

void QWidget::releaseKeyboard()
{
    if(!qt_nograb() && mac_keyboard_grabber == this)
	mac_keyboard_grabber = NULL;
}

QWidget *QWidget::mouseGrabber()
{
    return mac_mouse_grabber;
}

QWidget *QWidget::keyboardGrabber()
{
    return mac_keyboard_grabber;
}

void QWidget::setActiveWindow()
{
    QWidget *tlw = topLevelWidget();
    if(!tlw->isVisible() || !tlw->isTopLevel() || tlw->isDesktop())
	return;
    if(tlw->isPopup() || tlw->testWFlags(WStyle_Tool)) {
	ActivateWindow((WindowPtr)tlw->handle(), true);
    } else {
	if(IsWindowActive((WindowPtr)tlw->handle())) {
	    ActivateWindow((WindowPtr)tlw->handle(), true);
	    qApp->setActiveWindow(tlw);
	} else if (!isMinimized()){
	    SelectWindow((WindowPtr)tlw->handle());
	}
    }
    SetUserFocusWindow((WindowPtr)tlw->handle());
}

void QWidget::update()
{
    update(0, 0, width(), height());
}

void QWidget::update(int x, int y, int w, int h)
{
    if(!testWState(WState_BlockUpdates) && isVisible() && !clippedRegion().isEmpty()) {
	if(w < 0)
	    w = crect.width()  - x;
	if(h < 0)
	    h = crect.height() - y;
	if(w && h) {
	    QRegion r(x, y, w, h);
	    qt_event_request_updates(this, r);
#ifdef DEBUG_WINDOW_RGN
	    debug_wndw_rgn("update1", this, r, FALSE, TRUE);
#endif
	}
    }
}

void QWidget::update(const QRegion &rgn)
{
    if(!testWState(WState_BlockUpdates) && isVisible() && !clippedRegion().isEmpty()) {
	qt_event_request_updates(this, rgn);
#ifdef DEBUG_WINDOW_RGN
	debug_wndw_rgn("update2", this, rgn, FALSE, TRUE);
#endif
    }
}

void QWidget::repaint(const QRegion &r)
{
    if (testWState(WState_InPaintEvent))
	qWarning("QWidget::repaint: recursive repaint detected.");

    if ( (widget_state & (WState_Visible|WState_BlockUpdates)) != WState_Visible )
	return;

    QRegion rgn(r.intersect(d->clipRect()));
    if (rgn.isEmpty())
	return;

    setWState(WState_InPaintEvent);
    qt_set_paintevent_clipping(this, rgn, NULL);

    if (!testAttribute(WA_NoBackground)) {
	QPoint offset;
	QStack<QWidget*> parents;
	QWidget *w = q;
	while (w->d->isBackgroundInherited()) {
	    offset += w->pos();
	    w = w->parentWidget();
	    parents += w;
	}

	if(!testAttribute(WA_NoBackground)) {
	    QBrush bg = q->palette().brush(w->d->bg_role);
	    QRect rr = rgn.boundingRect();
	    bool was_unclipped = q->testWFlags(Qt::WPaintUnclipped);
	    q->clearWFlags(Qt::WPaintUnclipped);
	    QPainter p(q);
	    if(was_unclipped)
		q->setWFlags(Qt::WPaintUnclipped);
	    p.setClipRegion(rgn);
	    if(bg.pixmap())
		p.drawTiledPixmap(rr,*bg.pixmap(), QPoint(rr.x()+(offset.x()%bg.pixmap()->width()),
							  rr.y()+(offset.y()%bg.pixmap()->height())));
	    else
		p.fillRect(rr, bg.color());
	}

	if (!!parents) {
	    w = parents.pop();
	    QRegion prgn = rgn;
	    prgn.translate(offset);
	    for (;;) {
		if (w->testAttribute(QWidget::WA_ContentsPropagated)) {
		    qt_set_paintevent_clipping(w, prgn, q);
		    QPaintEvent e(prgn);
		    QApplication::sendEvent(w, &e);
		    qt_clear_paintevent_clipping(w);
		}
		if (!parents)
		    break;
		w = parents.pop();
		prgn.translate(-w->pos());
	    }
	}
    }
    QPaintEvent e(rgn);
    QApplication::sendSpontaneousEvent(this, &e);
    qt_clear_paintevent_clipping(this);
    clearWState(WState_InPaintEvent);
}

void QWidget::showWindow()
{
    if(isDesktop()) //desktop is always visible
	return;

    if(isTopLevel()) {
	d->createTLExtra();
	QDesktopWidget *dsk = QApplication::desktop();
	if (!d->topData()->is_moved && dsk) {
	    int movex = x(), movey = y();
	    QRect r = frameGeometry();
	    QRect avail = dsk->availableGeometry(dsk->screenNumber(this));
		if (r.bottom() > avail.bottom())
		    movey = avail.bottom() - r.height();
		if (r.right() > avail.right())
		    movex = avail.right() - r.width();
		// +2 to prevent going under the menu bar
		move(QMAX(avail.left(), movex), QMAX(avail.top() + 2, movey));
	}
    }
    fstrut_dirty = TRUE;
    dirtyClippedRegion(TRUE);
    if(isTopLevel()) {
	SizeWindow((WindowPtr)hd, width(), height(), 1);
	if(qt_mac_is_macsheet(this))
	    qt_event_request_showsheet(this);
#if QT_MACOSX_VERSION >= 0x1020
	else if(qt_mac_is_macdrawer(this))
	    OpenDrawer((WindowPtr)hd, kWindowEdgeDefault, true);
#endif
	else
	    ShowHide((WindowPtr)hd, 1); 	//now actually show it
    } else if(!parentWidget(TRUE) || parentWidget(TRUE)->isVisible()) {
	qt_dirty_wndw_rgn("show",this, mac_rect(posInWindow(this), geometry().size()));
    }
}

void QWidget::hideWindow()
{
    if(isDesktop()) //you can't hide the desktop!
	return;

    dirtyClippedRegion(TRUE);
    if(isTopLevel()) {
#if QT_MACOSX_VERSION >= 0x1020
	if(qt_mac_is_macdrawer(this))
	    CloseDrawer((WindowPtr)hd, true);
	else
#endif
       if(qt_mac_is_macsheet(this))
           HideSheetWindow((WindowPtr)hd);
	else
	    ShowHide((WindowPtr)hd, 0); //now we hide
	SizeWindow((WindowPtr)hd, 0, 0, 1);
	if(isActiveWindow()) {
	    QWidget *w = NULL;
	    if(parentWidget())
		w = parentWidget()->topLevelWidget();
	    if(!w || !w->isVisible()) {
		for(WindowPtr wp = GetFrontWindowOfClass(kDocumentWindowClass, true);
		    wp; wp = GetNextWindowOfClass(wp, kDocumentWindowClass, true)) {
		    if((w = QWidget::find((WId)wp)))
			break;
		}
	    }
	    if(w && w->isVisible())
		qt_event_request_activate(w);
	}
    } else if(!parentWidget(TRUE) || parentWidget(TRUE)->isVisible()) { //strange!! ###
	qt_dirty_wndw_rgn("hide",this, mac_rect(posInWindow(this), geometry().size()));
    }
    deactivateWidgetCleanup();
}



bool QWidget::isMinimized() const
{
    // true for non-toplevels that have the minimized flag, e.g. MDI children
    return IsWindowCollapsed((WindowRef)hd) || (!isTopLevel() && testWState(WState_Minimized));
}

bool QWidget::isMaximized() const
{
    return testWState(WState_Maximized);
}


void QWidget::showMinimized()
{
    show();
    if(isTopLevel() && !IsWindowCollapsed((WindowRef)hd))
	CollapseWindow((WindowPtr)hd, TRUE);

    QEvent e(QEvent::ShowMinimized);
    QApplication::sendEvent(this, &e);
    clearWState(WState_Maximized);
    setWState(WState_Minimized);
}

void QWidget::showMaximized()
{
    if(isDesktop())
	return;
    if(!isMaximized() && isTopLevel()) {
	Rect bounds;
	fstrut_dirty = TRUE;
	QDesktopWidget *dsk = QApplication::desktop();
	QRect avail = dsk->availableGeometry(dsk->screenNumber(this));
	SetRect(&bounds, avail.x(), avail.y(), avail.x() + avail.width(), avail.y() + avail.height());
	if(QWExtra   *extra = d->extraData()) {
	    if(bounds.right - bounds.left > extra->maxw)
		bounds.right = bounds.left + extra->maxw;
	    if(bounds.bottom - bounds.top > extra->maxh)
		bounds.bottom = bounds.top + extra->maxh;
	}
	if(QTLWExtra *tlextra = d->topData()) {
	    if(tlextra->normalGeometry.width() < 0)
		tlextra->normalGeometry = geometry();
	    if(fstrut_dirty)
		updateFrameStrut();
	    bounds.left += tlextra->fleft;
	    bounds.top += tlextra->ftop;
	    bounds.right -= tlextra->fright;
	    bounds.bottom -= tlextra->fbottom;
	}
	QRect orect(geometry().x(), geometry().y(), width(), height()),
	    nrect(bounds.left, bounds.top, bounds.right - bounds.left, bounds.bottom - bounds.top);
	if(orect.size() != nrect.size()) { // no real point..
	    Rect oldr;
	    SetRect(&oldr, orect.x(), orect.y(), orect.right(), orect.bottom());
	    SetWindowUserState((WindowPtr)hd, &oldr);

	    SetWindowStandardState((WindowPtr)hd, &bounds);
	    ZoomWindow((WindowPtr)hd, inZoomOut, FALSE);
	    qt_dirty_wndw_rgn("showMaxim",this, mac_rect(rect()));

	    crect = nrect;
	    if(isVisible()) {
		dirtyClippedRegion(TRUE);
		//issue a resize
		QResizeEvent qre(size(), orect.size());
		QApplication::sendEvent(this, &qre);
		//issue a move
		QMoveEvent qme(pos(), orect.topLeft());
		QApplication::sendEvent(this, &qme);
	    }
	}
    }
    show();
    QEvent e(QEvent::ShowMaximized);
    QApplication::sendEvent(this, &e);
    clearWState(WState_Minimized);
    setWState(WState_Maximized);
}

void QWidget::showNormal()
{
    if(isDesktop()) //desktop is always visible
	return;
    if(isTopLevel()) {
	if(d->topData()->fullscreen) {
	    reparent(0, d->topData()->savedFlags, QPoint(0,0));
	    setGeometry(d->topData()->normalGeometry);
	} else if(isMaximized()) {
	    Rect bounds;
	    ZoomWindow((WindowPtr)hd, inZoomIn, FALSE);
	    GetPortBounds(GetWindowPort((WindowPtr)hd), &bounds);
	    qt_dirty_wndw_rgn("showNormal",this, &bounds);
	} else {
	    CollapseWindow((WindowPtr)hd, FALSE);
	}
    }
    if(d->topData())
	d->topData()->fullscreen = 0;
    dirtyClippedRegion(TRUE);
    show();
    QEvent e(QEvent::ShowNormal);
    QApplication::sendEvent(this, &e);
    clearWState(WState_Minimized | WState_Maximized);
}

void QWidget::raise()
{
    if(isDesktop())
	return;
    if(isTopLevel()) {
	//we get to be the active process now
	ProcessSerialNumber psn;
	GetCurrentProcess(&psn);
	SetFrontProcess(&psn);
	//raise this window
	BringToFront((WindowPtr)hd);
    } else if(QWidget *p = parentWidget(TRUE)) {
	QRegion clp;
	if(isVisible())
	    clp = clippedRegion(FALSE);
	if(p->d->children.findIndex(this) >= 0) {
	    p->d->children.remove(this);
	    p->d->children.append(this);
	}
	if(isVisible()) {
	    dirtyClippedRegion(TRUE);
	    clp ^= clippedRegion(FALSE);
	    qt_dirty_wndw_rgn("raise",this, clp);
	}
    }
}

void QWidget::lower()
{
    if(isDesktop())
	return;

    if(isTopLevel()) {
	SendBehind((WindowPtr)handle(), NULL);
    } else if(QWidget *p = parentWidget(TRUE)) {
	QRegion clp;
	if(isVisible())
	    clp = clippedRegion(FALSE);
	if(p->d->children.findIndex(this) >= 0) {
	    p->d->children.remove(this);
	    p->d->children.insert(0, this);
	}
	if(isVisible()) {
	    dirtyClippedRegion(TRUE);
	    clp ^= clippedRegion(FALSE);
	    qt_dirty_wndw_rgn("lower",this, clp);
	}
    }
}


void QWidget::stackUnder(QWidget *w)
{
    if(!w || isTopLevel() || isDesktop())
	return;

    QWidget *p = parentWidget();
    if(!p || p != w->parentWidget())
	return;
    int loc = p->d->children.findIndex(w);
    QRegion clp;
    if(isVisible())
	clp = clippedRegion(FALSE);
    if(loc >= 0 && p->d->children.findIndex(this) >= 0) {
	p->d->children.remove(this);
	p->d->children.insert(loc, this);
    }
    if(isVisible()) {
	dirtyClippedRegion(TRUE);
	clp ^= clippedRegion(FALSE);
	qt_dirty_wndw_rgn("stackUnder",this, clp);
    }
}


void QWidget::setGeometry_helper(int x, int y, int w, int h, bool isMove)
{
    if (isTopLevel() && isMove) {
	d->createTLExtra();
	d->topData()->is_moved = 1;
    }
    if(isDesktop())
	return;
    if(QWExtra *extra = d->extraData()) {	// any size restrictions?
	if(isTopLevel()) {
	    qt_mac_update_sizer(this);
	    if(extra->maxw && extra->maxh && extra->maxw == extra->minw && extra->maxh == extra->minh)
		ChangeWindowAttributes((WindowRef)handle(), 0, kWindowFullZoomAttribute);
	    else
		ChangeWindowAttributes((WindowRef)handle(), kWindowFullZoomAttribute, 0);
	}
	w = QMIN(w,extra->maxw);
	h = QMIN(h,extra->maxh);
	w = QMAX(w,extra->minw);
	h = QMAX(h,extra->minh);

	// Deal with size increment
	if(QTLWExtra *top = d->topData()) {
	    if(top->incw) {
		w = w/top->incw;
		w *= top->incw;
	    }
	    if(top->inch) {
		h = h/top->inch;
		h *= top->inch;
	    }
	}
    }
    if(w < 1)                                // invalid size
	w = 1;
    if(h < 1)
	h = 1;

    QPoint oldp = pos();
    QSize  olds = size();
    if(!isTopLevel() && QSize(w, h) == olds && QPoint(x, y) == oldp)
	return;
    const bool visible = isVisible();
    QRegion oldregion, clpreg;
    if(visible) {
	oldregion = clippedRegion(FALSE);
	dirtyClippedRegion(FALSE);
	crect = QRect(x, y, w, h);
	dirtyClippedRegion(TRUE);
    } else {
	crect = QRect(x, y, w, h);
    }

    bool isResize = (olds != size());
    if(isTopLevel() && winid && own_id) {
	if(isResize && isMaximized())
	    clearWState(WState_Maximized);
	Rect r;
	SetRect(&r, x, y, x + w, y + h);
	SetWindowBounds((WindowPtr)hd, kWindowContentRgn, &r);
    }

    if(isMove || isResize) {
	if(!visible) {
	    if (isMove && pos() != oldp)
		setAttribute(WA_PendingMoveEvent, true);
	    if (isResize)
		setAttribute(WA_PendingResizeEvent, true);
	} else {
	    QRegion bltregion, clpreg = clippedRegion(FALSE);
	    const bool oldreg_empty=oldregion.isEmpty(), newreg_empty = clpreg.isEmpty();
	    if(!oldreg_empty) {
		//setup the old clipped region..
		bltregion = oldregion;
		if(isMove && !isTopLevel())
		    bltregion.translate(pos().x() - oldp.x(), pos().y() - oldp.y());
		bltregion &= clpreg;
		{   //can't blt that which is dirty
		    RgnHandle r = qt_mac_get_rgn();
		    GetWindowRegion((WindowPtr)hd, kWindowUpdateRgn, r);
		    if(!EmptyRgn(r)) {
			QRegion dirty = qt_mac_convert_mac_region(r); //the dirty region
			dirty.translate(-topLevelWidget()->geometry().x(),
					-topLevelWidget()->geometry().y());
			if(isMove && !isTopLevel()) //need to be in new coords
			    dirty.translate(pos().x() - oldp.x(), pos().y() - oldp.y());
			bltregion -= dirty;
		    }
		    qt_mac_dispose_rgn(r);
		}

		if(isMove && !no_move_blt && !isTopLevel()) {
		    QWidget *p = parentWidget(TRUE);
		    if(!p)
			p = this;
		    QMacSavedPortInfo pi(p, bltregion);
		    unclippedBitBlt(p, pos().x(), pos().y(), p, oldp.x(), oldp.y(),
				    olds.width(), olds.height(), Qt::CopyROP, TRUE, TRUE);
		}
	    }
	    if((!newreg_empty || !oldreg_empty) &&
	       (isResize || !isTopLevel() || !QDIsPortBuffered(GetWindowPort((WindowPtr)hd)))) {
		//finally issue "expose" event
		QRegion upd((oldregion + clpreg) - bltregion);
		if(isResize && !testAttribute(WA_StaticContents))
		    upd += clippedRegion();
		qt_dirty_wndw_rgn("setGeometry_helper",this, upd);
		//and force the update
		if(isResize || 1)
		    qt_event_request_updates();
	    }
	    //Do these last, as they may cause an event which paints, and messes up
	    //what we blt above
	    if(isResize) { //send the resize event..
		QResizeEvent e(size(), olds);
		QApplication::sendEvent(this, &e);
	    }
	    if(isMove && pos() != oldp) { //send the move event..
		QMoveEvent e(pos(), oldp);
		QApplication::sendEvent(this, &e);
	    }
	}
    }
}

void QWidget::setMinimumSize(int minw, int minh)
{
    if(minw < 0 || minh < 0)
	qWarning("Qt: QWidget::setMinimumSize: The smallest allowed size is (0,0)");
    d->createExtra();
    if(d->extraData()->minw == minw && d->extraData()->minh == minh)
	return;
    d->extraData()->minw = minw;
    d->extraData()->minh = minh;
    if(minw > width() || minh > height()) {
	bool resized = testWState(WState_Resized);
	resize(QMAX(minw,width()), QMAX(minh,height()));
	if(!resized)
	    clearWState(WState_Resized); //not a user resize
    }
    updateGeometry();
}

void QWidget::setMaximumSize(int maxw, int maxh)
{
    if(maxw > QWIDGETSIZE_MAX || maxh > QWIDGETSIZE_MAX) {
	qWarning("Qt: QWidget::setMaximumSize: (%s/%s) "
		"The largest allowed size is (%d,%d)",
		 name("unnamed"), className(), QWIDGETSIZE_MAX,
		QWIDGETSIZE_MAX);
	maxw = QMIN(maxw, QWIDGETSIZE_MAX);
	maxh = QMIN(maxh, QWIDGETSIZE_MAX);
    }
    if(maxw < 0 || maxh < 0) {
	qWarning("Qt: QWidget::setMaximumSize: (%s/%s) Negative sizes (%d,%d) "
		"are not possible",
		name("unnamed"), className(), maxw, maxh);
	maxw = QMAX(maxw, 0);
	maxh = QMAX(maxh, 0);
    }
    d->createExtra();
    if(d->extraData()->maxw == maxw && d->extraData()->maxh == maxh)
	return;
    d->extraData()->maxw = maxw;
    d->extraData()->maxh = maxh;
    if(maxw < width() || maxh < height()) {
	bool resized = testWState(WState_Resized);
	resize(QMIN(maxw,width()), QMIN(maxh,height()));
	if(!resized)
	    clearWState(WState_Resized); //not a user resize
    }
    updateGeometry();
}


void QWidget::setSizeIncrement(int w, int h)
{
    d->createTLExtra();
    d->topData()->incw = w;
    d->topData()->inch = h;
}

void QWidget::setBaseSize(int w, int h)
{
    d->createTLExtra();
    d->topData()->basew = w;
    d->topData()->baseh = h;
}

void QWidget::scroll(int dx, int dy)
{
    scroll(dx, dy, QRect());
}

void QWidget::scroll(int dx, int dy, const QRect& r)
{
    bool valid_rect = r.isValid();
    if(testWState(WState_BlockUpdates) &&  (valid_rect || !children()))
	return;

    QRect sr = valid_rect ? r : rect();
    if(dx == 0 && dy == 0)
	return;
    int x1, y1, x2, y2, w=sr.width(), h=sr.height();
    if(dx > 0) {
	x1 = sr.x();
	x2 = x1+dx;
	w -= dx;
    } else {
	x2 = sr.x();
	x1 = x2-dx;
	w += dx;
    }
    if(dy > 0) {
	y1 = sr.y();
	y2 = y1+dy;
	h -= dy;
    } else {
	y2 = sr.y();
	y1 = y2-dy;
	h += dy;
    }

    bool just_update = QABS(dx) > width() || QABS(dy) > height();
    if(just_update)
	update();
    QRegion bltd;
    QPoint p(posInWindow(this));
    if(w > 0 && h > 0) {
	bltd = clippedRegion(valid_rect); //offset the clip
	bltd.translate(dx, dy);
	QRegion requested(x2, y2, w, h); //only that which I blt to
	requested.translate(p.x(), p.y());
	bltd &= requested;
	bltd &= clippedRegion(valid_rect); //finally clip to clipping region
	QMacSavedPortInfo pi(this, bltd);
	unclippedBitBlt(this,x2,y2,this,x1,y1,w,h,Qt::CopyROP,TRUE,TRUE);
    }
    dirtyClippedRegion(TRUE);
    if(!valid_rect) {	// scroll children
	QPoint pd(dx, dy);
	QWidgetList moved;
	QObjectList chldrn = children();
	for(int i = 0; i < chldrn.size(); i++) {  //first move all children
	    QObject *obj = chldrn.at(i);
	    if(obj->isWidgetType()) {
		QWidget *w = (QWidget*)obj;
		w->crect = QRect(w->pos() + pd, w->size());
		moved.append(w);
	    }
	}
        //now send move events (do not do this in the above loop, breaks QAquaFocusWidget)
	for(int i = 0; i < moved.size(); i++) {
	    QWidget *w = moved.at(i);
	    QMoveEvent e(w->pos(), w->pos() - pd);
	    QApplication::sendEvent(w, &e);
	}
    }

    if(just_update)
	return;
    QRegion newarea(sr);
    newarea.translate(p.x(), p.y());
    newarea &= (clippedRegion(valid_rect) - bltd);
    qt_clean_wndw_rgn("scroll", this, newarea);
    newarea.translate(-p.x(), -p.y());
    qt_paint_children(this, newarea, PC_None);
#if 0
    if(QDIsPortBuffered(GetWindowPort((WindowPtr)hd)))
	QMacSavedPortInfo::flush(this);
#endif
}

int QWidget::metric(int m) const
{
    switch(m) {
    case QPaintDeviceMetrics::PdmHeightMM: // 75 dpi is 3dpmm
	return (metric(QPaintDeviceMetrics::PdmHeight)*100)/288;
    case QPaintDeviceMetrics::PdmWidthMM: // 75 dpi is 3dpmm
	return (metric(QPaintDeviceMetrics::PdmWidth)*100)/288;
    case QPaintDeviceMetrics::PdmWidth:
    {
	if(!isTopLevel())
	    return crect.width();
	Rect windowBounds;
	GetPortBounds(GetWindowPort(((WindowPtr)hd)), &windowBounds);
	return windowBounds.right;
    }

    case QPaintDeviceMetrics::PdmHeight:
    {
	if(!isTopLevel())
	    return crect.height();
	Rect windowBounds;
	GetPortBounds(GetWindowPort(((WindowPtr)hd)), &windowBounds);
	return windowBounds.bottom;
    }
    case QPaintDeviceMetrics::PdmDepth:// FIXME : this is a lie in most cases
	return 16;
    case QPaintDeviceMetrics::PdmDpiX: // FIXME : this is a lie in most cases
    case QPaintDeviceMetrics::PdmPhysicalDpiX:
	return 80;
    case QPaintDeviceMetrics::PdmDpiY: // FIXME : this is a lie in most cases
    case QPaintDeviceMetrics::PdmPhysicalDpiY:
	return 80;
    default: //leave this so the compiler complains when new ones are added
	qWarning("Qt: QWidget::metric unhandled parameter %d",m);
	return QPaintDevice::metric(m);// XXX
    }
    return 0;
}

void QWidgetPrivate::createSysExtra()
{
    extra->has_dirty_area = FALSE;
    extra->child_serial = extra->clip_serial = 1;
    extra->child_dirty = extra->clip_dirty = TRUE;
    extra->macDndExtra = 0;
}

void QWidgetPrivate::deleteSysExtra()
{
}

void QWidgetPrivate::createTLSysExtra()
{
    extra->topextra->group = NULL;
    extra->topextra->is_moved = 0;
    extra->topextra->resizer = 0;
}

void QWidgetPrivate::deleteTLSysExtra()
{
    if(extra->topextra->group)
	ReleaseWindowGroup(extra->topextra->group);
}

bool QWidget::acceptDrops() const
{
    return macDropEnabled;
}

void QWidget::updateFrameStrut() const
{
    QWidget *that = (QWidget *) this; //mutable
    if(isDesktop() || !fstrut_dirty) {
	that->fstrut_dirty = isVisible();
	return;
    }
    that->fstrut_dirty = FALSE;
    QTLWExtra *top = that->d->topData();
    top->fleft = top->fright = top->ftop = top->fbottom = 0;
    if(isTopLevel()) {
	Rect window_r, content_r;
	//get bounding rects
	RgnHandle rgn = qt_mac_get_rgn();
	GetWindowRegion((WindowPtr)hd, kWindowStructureRgn, rgn);
	GetRegionBounds(rgn, &window_r);
	GetWindowRegion((WindowPtr)hd, kWindowContentRgn, rgn);
	GetRegionBounds(rgn, &content_r);
	qt_mac_dispose_rgn(rgn);
	//put into qt structure
	top->fleft = content_r.left - window_r.left;
	top->ftop = content_r.top - window_r.top;
	top->fright = window_r.right - content_r.right;
	top->fbottom = window_r.bottom - window_r.bottom;
    }
}

void qt_macdnd_unregister(QWidget *widget, QWExtra *extra); //dnd_mac
void qt_macdnd_register(QWidget *widget, QWExtra *extra); //dnd_mac

void QWidget::setAcceptDrops(bool on)
{
    if((on && macDropEnabled) || (!on && !macDropEnabled))
	return;
    macDropEnabled = on;
    if(!on && !topLevelWidget()->d->extraData()) //short circuit
	return;
    topLevelWidget()->d->createExtra();
    if(on)
	qt_macdnd_register(topLevelWidget(),  topLevelWidget()->d->extraData());
    else
	qt_macdnd_unregister(topLevelWidget(), topLevelWidget()->d->extraData());
}

void QWidget::setMask(const QRegion &region)
{
    d->createExtra();
    if(region.isEmpty() && d->extraData()->mask.isEmpty())
	return;

    QRegion clp;
    if(isVisible())
	clp = clippedRegion(FALSE);
    d->extraData()->mask = region;
    if(isVisible()) {
	dirtyClippedRegion(TRUE);
	clp ^= clippedRegion(FALSE);
	qt_dirty_wndw_rgn("setMask",this, clp);
    }
    if(isTopLevel()) {
	if(qMacVersion() == Qt::MV_10_DOT_1 &&
	   testWFlags(WStyle_Customize) && testWFlags(WStyle_NoBorder)) {
	    /* We do this because the X/Y seems to move to the first paintable point
	       (ie the bounding rect of the mask). We must offset everything or else
	       we have big problems. */
	    QRect r = region.boundingRect();
	    QMacSavedPortInfo mp(this);
	    SetOrigin(r.x(), r.y());
	}
	//now let the wdef take it
	ReshapeCustomWindow((WindowPtr)hd);
    }
}

void QWidget::setMask(const QBitmap &bitmap)
{
    setMask(QRegion(bitmap));
}


void QWidget::clearMask()
{
    setMask(QRegion());
}

void QWidget::setName(const char *name)
{
    QObject::setName(name);
}

void QWidget::propagateUpdates(bool update_rgn)
{
    QRegion rgn;
    QWidget *widg = this;
    if(update_rgn) {
	widg = topLevelWidget();
	QMacSavedPortInfo savedInfo(this);
	RgnHandle mac_rgn = qt_mac_get_rgn();
	GetWindowRegion((WindowPtr)hd, kWindowUpdateRgn, mac_rgn);
	if(EmptyRgn(mac_rgn)) {
	    qt_mac_dispose_rgn(mac_rgn);
	    return;
	}
	rgn = qt_mac_convert_mac_region(mac_rgn);
	rgn.translate(-widg->geometry().x(), -widg->geometry().y());
	qt_mac_dispose_rgn(mac_rgn);
	BeginUpdate((WindowPtr)hd);
    } else {
	rgn = QRegion(rect());
    }
#ifdef DEBUG_WINDOW_RGNS
    debug_wndw_rgn("*****propagatUpdates", widg, rgn, TRUE);
#endif
    qt_paint_children(widg, rgn, PC_ForceErase);
    if(update_rgn)
	EndUpdate((WindowPtr)hd);
}

/*!
    \internal
*/
void QWidget::setRegionDirty(bool child)
{
    QWExtra *extra = d->extraData();
    if(!extra)
	return;
    if(child) {
	extra->clip_serial++;
	extra->clip_dirty = TRUE;
    } else {
	extra->child_serial++;
	extra->child_dirty = TRUE;
    }
}

/*!
    \internal
*/
void QWidget::dirtyClippedRegion(bool dirty_myself)
{
    if(qApp->closingDown())
	return;
    if(dirty_myself && !wasDeleted) {
	//dirty myself
	{
	    setRegionDirty(FALSE);
	    setRegionDirty(TRUE);
	}
	//when I get dirty so do my children
	QObjectList chldrn = queryList();
	for(int i = 0; i < chldrn.size(); i++) {
	    QObject *obj = chldrn.at(i);
	    if(obj->isWidgetType() && !obj->wasDeleted) {
		QWidget *w = (QWidget *)obj;
		if(!w->isTopLevel() && w->isVisible())
		    w->setRegionDirty(TRUE);
	    }
	}
    }

    if(!isTopLevel()) { //short circuit, there is nothing to dirty here..
	int ox = x(), oy = y(), ow = width(), oh = height();
	for(QWidget *par=this; (par = par->parentWidget(TRUE)); ) {
	    if(ox + ow < 0 || oy + oh < 0 || ox > par->width() || oy > par->height())
		return;
	    ox += par->x();
	    oy += par->y();
	}
    }
    //handle the rest of the widgets
    const QPoint myp(posInWindow(this));
    QRect myr(myp.x(), myp.y(), width(), height());
    QWidget *last = this, *w;
    int px = myp.x() - x(), py = myp.y() - y();
    for(QWidget *widg = parentWidget(); widg; last = widg, widg = widg->parentWidget()) {
	if(widg->wasDeleted) //no point in dirting
	    continue;
	myr = myr.intersect(QRect(px, py, widg->width(), widg->height()));
	widg->setRegionDirty(FALSE);

	QObjectList chldrn = widg->children();
	for(int i = 0; i < chldrn.size(); i++) {
	    QObject *obj = chldrn.at(i);
	    if(obj == last)
		break;
	    if(obj->isWidgetType() && !obj->wasDeleted) {
		w = (QWidget *)obj;
		if(!w->isTopLevel() && w->isVisible()) {
		    QPoint wp(px + w->x(), py + w->y());
		    if(myr.intersects(QRect(wp.x(), wp.y(), w->width(), w->height()))) {
			w->setRegionDirty(TRUE);
			QObjectList chldrn2 = w->queryList();
			for(int i2 = 0; i2 < chldrn2.size(); i2++) {
			    QObject *obj2 = chldrn2.at(i2);
			    if(obj2->isWidgetType() && !obj2->wasDeleted) {
				QWidget *w = (QWidget *)obj2;
				/* this relies on something that may change in the future
				   if hd for all sub widgets != toplevel widget's hd, then
				   this function will not work any longer */
				if(w->hd == hd && !w->isTopLevel() && w->isVisible())
				    w->setRegionDirty(TRUE);
			    }
			}
		    }
		}
	    }
	}
	px -= widg->x();
	py -= widg->y();
    }
}

/*!
    \internal
*/
bool QWidget::isClippedRegionDirty()
{
    if(!d->extraData() || d->extraData()->clip_dirty)
	return TRUE;
    if(/*!isTopLevel() && */(parentWidget(TRUE) && parentWidget(TRUE)->isClippedRegionDirty()))
	return TRUE;
    return FALSE;
}

/*!
    \internal
*/
uint QWidget::clippedSerial(bool do_children)
{
    d->createExtra();
    return do_children ? d->extraData()->clip_serial : d->extraData()->child_serial;
}

/*!
    \internal
*/
QRegion QWidget::clippedRegion(bool do_children)
{
    if(wasDeleted || !isVisible() || qApp->closingDown() || qApp->startingUp())
	return QRegion();

    d->createExtra();
    QWExtra *extra = d->extraData();
    if(isDesktop()) {    //the desktop doesn't participate in our clipping games
	if(!extra->clip_dirty && (!do_children || !extra->child_dirty)) {
	    if(!do_children)
		return extra->clip_sibs;
	    return extra->clip_saved;
	}
	extra->child_dirty = (extra->clip_dirty = FALSE);
	return extra->clip_sibs = extra->clip_children = QRegion(0, 0, width(), height());
    }

    if(!extra->clip_dirty && (!do_children || !extra->child_dirty)) {
	if(!do_children)
	    return extra->clip_sibs;
	return extra->clip_saved;
    }

    bool no_children = children().isEmpty();
    /* If we have no children, and we are clearly off the screen we just get an automatic
       null region. This is to allow isNull() to be a cheap test of "off-screen" plus it
       prevents all the below calculations (specifically posInWindow() is pointless). */
    QPoint mp; //My position in the window (posInWindow(this))
    if(!isTopLevel() && no_children) { //short-circuit case
	int px = x(), py = y();
	for(QWidget *par = parentWidget(TRUE); par; par = par->parentWidget(TRUE)) {
 	    if((px + width() < 0) || (py + height() < 0) ||
 	       px > par->width() || py > par->height()) {
		extra->child_dirty = (extra->clip_dirty = FALSE);
		return extra->clip_saved = extra->clip_children = extra->clip_sibs = QRegion();
	    }
	    if(par->isTopLevel())
		break;
	    px += par->x();
	    py += par->y();
	}
	mp = QPoint(px, py);
    } else {
	mp = posInWindow(this);
    }

    /* This whole vis_width / vis_height is to prevent creating very large regions,
       as RgnHandle's just use Rect's SHRT_MAX is the maximum value, which causes strange
       problems when widgets are placed further onto a window. It should be quite unlikely
       that a top level window >SHRT_MAX in either width or height. As when these change they
       should be dirtied this optimization should mean nothing otherwise */
    int vis_width = width(), vis_height = height(), vis_x = 0, vis_y = 0;
    if(QWidget *par = parentWidget(TRUE)) {
	int px = mp.x() - x(), py = mp.y() - y();
	int min_x = 0, min_y = 0,
	    max_x = mp.x() + vis_width, max_y = mp.y() + vis_height;
	for(; par; par = par->parentWidget(TRUE)) {
	    if(px > mp.x() && px - mp.x() > min_x)
		min_x = px - mp.x();
	    if(py > mp.y() && py - mp.y() > min_y)
		min_y = py - mp.y();
	    if(px + par->width() < max_x)
		max_x = px + par->width();
	    if(py + par->height() < max_y)
		max_y = py + par->height();
	    px -= par->x();
	    py -= par->y();
	}
	vis_x = min_x;
	vis_y = min_y;
	vis_width =  max_x > mp.x() ? (max_x - mp.x()) - min_x : 0;
	vis_height = max_y > mp.y() ? (max_y - mp.y()) - min_y : 0;
    }

    //clip out my children
    QRegion mask;
    if(isClippedRegionDirty() || (do_children && extra->child_dirty)) {
	extra->child_dirty = FALSE;
	extra->clip_children = QRegion(vis_x, vis_y, vis_width, vis_height);
	if(!no_children) {
	    QRect sr(vis_x, vis_y, vis_width, vis_height);
	    QObjectList chldrn = children();
	    for(int i = 0; i < chldrn.size(); i++) {
		QObject *obj = chldrn.at(i);
		if(obj->isWidgetType() && !obj->wasDeleted) {
		    QWidget *cw = (QWidget *)obj;
		    if(cw->isVisible() && !cw->isTopLevel() && sr.intersects(cw->geometry())) {
			QRegion childrgn(cw->x(), cw->y(), cw->width(), cw->height());
			if(QWExtra *cw_extra = cw->d->extraData()) {
			    if(!cw_extra->mask.isEmpty()) {
				mask = cw_extra->mask;
				mask.translate(cw->x(), cw->y());
				childrgn &= mask;
			    }
			}
			extra->clip_children -= childrgn;
		    }
		}
	    }
	}
    }

    if(isClippedRegionDirty()) {
	extra->clip_dirty = FALSE;
	extra->clip_sibs = QRegion(mp.x()+vis_x, mp.y()+vis_y, vis_width, vis_height);
	//clip my rect with my mask
	if(extra && !extra->mask.isEmpty() && (vis_width || vis_height)) {
	    mask = extra->mask;
	    mask.translate(mp.x(), mp.y());
	    extra->clip_sibs &= mask;
	}

	//clip away my siblings
	if(!isTopLevel() && parentWidget() && (vis_width || vis_height)) {
	    QPoint tmp;
	    QObjectList siblngs = parentWidget()->children();
	    for(int i = siblngs.size() - 1; i >= 0; --i) {
		QObject *obj = siblngs.at(i);
		if(obj == this) //I don't care about people behind me
		    break;
		if(obj->isWidgetType() && !obj->wasDeleted) {
		    QWidget *sw = (QWidget *)obj;
		    tmp = posInWindow(sw);
		    QRect sr(tmp.x(), tmp.y(), sw->width(), sw->height());
		    if(!sw->isTopLevel() && sw->isVisible() && extra->clip_sibs.contains(sr)) {
			QRegion sibrgn(sr);
			if(QWExtra *sw_extra = sw->d->extraData()) {
			    if(!sw_extra->mask.isEmpty()) {
				mask = sw_extra->mask;
				mask.translate(tmp.x(), tmp.y());
				sibrgn &= mask;
			    }
			}
			extra->clip_sibs -= sibrgn;
		    }
		}
	    }
	}

	/*Remove window decorations from the top level window, specifically this
	  means the GrowRgn*/
	if(isTopLevel()) {
	    QRegion contents;
	    RgnHandle r = qt_mac_get_rgn();
	    GetWindowRegion((WindowPtr)hd, kWindowContentRgn, r);
	    if(!EmptyRgn(r)) {
		contents = qt_mac_convert_mac_region(r);
		contents.translate(-geometry().x(), -geometry().y());
	    }
	    qt_mac_dispose_rgn(r);
	    extra->clip_sibs &= contents;
	} else if(parentWidget()) { //clip to parent
	    extra->clip_sibs &= parentWidget()->clippedRegion(FALSE);
	}
    }

    // If they are empty set them to null() that way we have a cheaper test
    if(extra->clip_children.isEmpty())
	extra->clip_children = QRegion();
    if(extra->clip_sibs.isEmpty())
	extra->clip_sibs = QRegion();
    if(no_children) {
	extra->clip_children = extra->clip_saved = extra->clip_sibs;
    } else if(extra->clip_children.isEmpty() || extra->clip_sibs.isEmpty()) {
	extra->clip_saved = QRegion();
    } else {
	QRegion chldrgns = extra->clip_children;
	chldrgns.translate(mp.x(), mp.y());
	extra->clip_saved = extra->clip_sibs & chldrgns;
    }

    //finally return the correct region
    if(do_children)
	return extra->clip_saved;
    return extra->clip_sibs;
}

void QWidget::resetInputContext()
{
    qt_mac_unicode_reset_input(this);
}

/*!
    \internal
*/
void QWidget::macWidgetChangedWindow()
{
}

