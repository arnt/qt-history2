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

#include "qaccel.h"
#include "qapplication.h"
#include "qapplication_p.h"
#include "qbitmap.h"
#include "qcursor.h"
#include "qdesktopwidget.h"
#include "qdragobject.h"
#include "qevent.h"
#include "qimage.h"
#include "qlayout.h"
#include "qpaintdevicemetrics.h"
#include "qpaintengine_mac.h"
#include "qpainter.h"
#include "qstack.h"
#include "qstyle.h"
#include "qtextcodec.h"
#include "qtimer.h"

#include <ApplicationServices/ApplicationServices.h>
#include <limits.h>

#include "qwidget_p.h"

#define d d_func()
#define q q_func()

QList<WId> QWidgetPrivate::request_updates_pending_list;

/*****************************************************************************
  QWidget debug facilities
 *****************************************************************************/
//#define DEBUG_WINDOW_RGNS
//#define DEBUG_WINDOW_CREATE

/*****************************************************************************
  QWidget globals
 *****************************************************************************/
static bool no_move_blt = false;
static WId serial_id = 0;
static WindowGroupRef qt_mac_stays_on_top_group = 0;
QWidget *mac_mouse_grabber = 0;
QWidget *mac_keyboard_grabber = 0;
int mac_window_count = 0;

/*****************************************************************************
  Externals
 *****************************************************************************/
QSize qt_naturalWidgetSize(QWidget *); //qwidget.cpp
QString cfstring2qstring(CFStringRef); //qglobal.cpp
QSize qt_initial_size(QWidget *w); //qwidget.cpp
void qt_mac_clip_cg_handle(CGContextRef, const QRegion &, const QPoint &, bool); //qpaintdevice_mac.cpp
void qt_mac_unicode_reset_input(QWidget *); //qapplication_mac.cpp
void qt_mac_unicode_init(QWidget *); //qapplication_mac.cpp
void qt_mac_unicode_cleanup(QWidget *); //qapplication_mac.cpp
void qt_event_request_updates(); //qapplication_mac.cpp
void qt_event_request_updates(QWidget *w, const QRegion &r, bool subtract = false); // qapplication_mac.cpp
void qt_event_request_activate(QWidget *); //qapplication_mac.cpp
bool qt_event_remove_activate(); //qapplication_mac.cpp
void qt_mac_event_release(QWidget *w); //qapplication_mac.cpp
void qt_event_request_showsheet(QWidget *); //qapplication_mac.cpp
IconRef qt_mac_create_iconref(const QPixmap &); //qpixmap_mac.cpp
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
    QPoint ret(0, 0);
    if(w->isTopLevel())
	return ret;
    if(QWidget *par = w->parentWidget())
	ret = posInWindow(par) + w->pos();
    return ret;
}

static void qt_mac_release_stays_on_top_group()
{
    ReleaseWindowGroup(qt_mac_stays_on_top_group);
    if(GetWindowGroupRetainCount(qt_mac_stays_on_top_group) == 1) { //only the global pointer exists
	ReleaseWindowGroup(qt_mac_stays_on_top_group);
	qt_mac_stays_on_top_group = 0;
    }
}

/* Use this function instead of ReleaseWindowGroup, this will be sure to release the
   stays on top window group (created with qt_mac_get_stays_on_top_group below) */
static void qt_mac_release_window_group(WindowGroupRef group)
{
    if(group == qt_mac_stays_on_top_group)
	qt_mac_release_stays_on_top_group();
    else
	ReleaseWindowGroup(group);
}
#define ReleaseWindowGroup(x) Are you sure you wanted to do that? (you wanted qt_mac_release_window_group)

/* We create one static stays on top window group so that all stays on top (aka popups) will
   fall into the same group and be able to be raise()'d with releation to one another (from
   within the same window group). */
static WindowGroupRef qt_mac_get_stays_on_top_group()
{
    if(!qt_mac_stays_on_top_group) {
	CreateWindowGroup(kWindowActivationScopeNone, &qt_mac_stays_on_top_group);
	SetWindowGroupLevel(qt_mac_stays_on_top_group, kCGMaximumWindowLevel);
	SetWindowGroupParent(qt_mac_stays_on_top_group, GetWindowGroupOfClass(kAllWindowClasses));
    }
    RetainWindowGroup(qt_mac_stays_on_top_group);
    return qt_mac_stays_on_top_group;
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
				  bool clean=false, bool translate=false) {
    QPoint mp(posInWindow(w));
    QRect wrect(mp.x(), mp.y(), w->width(), w->height());
    qDebug("Qt: internal: %s %s %s (%s) [ %d %d %d %d ]", where, clean ? "clean" : "dirty",
	   w->className(), w->objectName(), wrect.x(), wrect.y(), wrect.width(), wrect.height());
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
	       !w->d->clippedRegion().contains(srect) ? '?' :
	       (!QRegion(w->d->clippedRegion() ^ srect).isEmpty() ? '*' : '-'),
	       srect.x(), srect.y(), srect.width(), srect.height());
    }
    qDebug("Qt: internal: *****End debug..");
}
static inline void debug_wndw_rgn(const char *where, QWidget *w, const Rect *r, bool clean=false) {
    debug_wndw_rgn(where + QString(" (rect)"), w,
		   QRegion(r->left, r->top, r->right - r->left, r->bottom - r->top), clean);
}
#define qt_dirty_wndw_rgn(x, who, where) do { if(qt_dirty_wndw_rgn_internal(who, where)) \
                                                 debug_wndw_rgn(x, who, where); } while(0);

#define qt_clean_wndw_rgn(x, who, where) do { if(qt_dirty_wndw_rgn_internal(who, where, true)) \
                                                 debug_wndw_rgn(x, who, where, true); } while(0);
#define qt_dirty_wndw_rgn(x, who, where) do { if(qt_dirty_wndw_rgn_internal(who, where)) \
                                                 debug_wndw_rgn(x, who, where); } while(0);
#else
#define clean_wndw_rgn(w, x, y)
#define debug_wndw_rgn(w, x, y)
#define qt_clean_wndw_rgn(x, who, where) qt_dirty_wndw_rgn_internal(who, where, true);
#define qt_dirty_wndw_rgn(x, who, where) qt_dirty_wndw_rgn_internal(who, where);
#endif
static inline bool qt_dirty_wndw_rgn_internal(const QWidget *p, const Rect *r, bool clean=false)
{
    if(qApp->closingDown())
	return false;
    else if(r->right < 0 || r->bottom < 0 || r->left > p->topLevelWidget()->width() ||
	    r->top > p->topLevelWidget()->height())
	return false;
    if(clean) {
	ValidWindowRect((WindowPtr)p->handle(), r);
    } else {
	InvalWindowRect((WindowPtr)p->handle(), r);
        qt_event_request_updates();
    }
    return true;
}
inline bool qt_dirty_wndw_rgn_internal(const QWidget *p, const QRect &r, bool clean=false)
{
    if(qApp->closingDown() || r.isNull())
	return true;
    return qt_dirty_wndw_rgn_internal(p, mac_rect(r), clean);
}
inline bool qt_dirty_wndw_rgn_internal(const QWidget *p, const QRegion &r, bool clean=false)
{
    if(qApp->closingDown())
	return false;
    else if(r.isEmpty())
	return false;
    else if(!r.handle())
	return qt_dirty_wndw_rgn_internal(p, r.boundingRect(), clean);
    if(clean) {
	ValidWindowRgn((WindowPtr)p->handle(), r.handle());
    } else {
	InvalWindowRgn((WindowPtr)p->handle(), r.handle());
	qt_event_request_updates();
    }
    return true;
}

void qt_mac_update_metal_style(QWidget *w)
{
    if(w->isTopLevel()) {
	if(w->testAttribute(QWidget::WA_MacMetalStyle))
	    ChangeWindowAttributes((WindowRef)w->handle(), kWindowMetalAttribute, 0);
	else
	    ChangeWindowAttributes((WindowRef)w->handle(), 0, kWindowMetalAttribute);
    }
}

static OSStatus qt_mac_create_window(WindowClass wclass, WindowAttributes wattr,
				     Rect *geo, WindowPtr *w)
{
    OSStatus ret;
    if(geo->right == geo->left)
	geo->right++;
    if(geo->bottom == geo->top)
	geo->bottom++;
    if(QSysInfo::MacintoshVersion >= QSysInfo::MV_PANTHER) {
	Rect null_rect; SetRect(&null_rect, 0, 0, 0, 0);
	ret = CreateNewWindow(wclass, wattr, &null_rect, w);
	if(ret == noErr) {
	    ret = SetWindowBounds(*w, kWindowContentRgn, geo);
	    if(ret != noErr)
		qWarning("%s:%d This error shouldn't really ever happen!!!", __FILE__, __LINE__);
	}
    } else {
	ret = CreateNewWindow(wclass, wattr, geo, w);
    }
    return ret;
}

// Paint event clipping magic
extern void qt_set_paintevent_clipping(QPaintDevice* dev, const QRegion& region, QWidget *clipTo);
extern void qt_clear_paintevent_clipping(QPaintDevice *dev);

QMAC_PASCAL OSStatus QWidgetPrivate::qt_window_event(EventHandlerCallRef er, EventRef event, void *)
{
    UInt32 ekind = GetEventKind(event), eclass = GetEventClass(event);
    if(eclass == kEventClassWindow) {
	WindowRef wid;
	GetEventParameter(event, kEventParamDirectObject, typeWindowRef, 0,
			  sizeof(WindowRef), 0, &wid);
	switch(ekind) {
	case kEventWindowGetRegion: {
	    CallNextEventHandler(er, event);
	    WindowRegionCode wcode;
	    GetEventParameter(event, kEventParamWindowRegionCode, typeWindowRegionCode, 0,
			      sizeof(wcode), 0, &wcode);
	    RgnHandle rgn;
	    GetEventParameter(event, kEventParamRgnHandle, typeQDRgnHandle, NULL,
			      sizeof(rgn), NULL, &rgn);
	    if(QWidgetPrivate::qt_window_rgn((WId)wid, wcode, rgn, false))
		SetEventParameter(event, kEventParamRgnHandle, typeQDRgnHandle, sizeof(rgn), &rgn);
	    return noErr; }
	case kEventWindowDrawContent: {
	    if(QWidget *widget = QWidget::find((WId)wid)) {
		widget->d->propagateUpdates(false);
		return noErr;
	    }
	    break; }
	}
    } else if(eclass == kEventClassMouse && ekind == kEventMouseDown) {
	Point where;
	GetEventParameter(event, kEventParamMouseLocation, typeQDPoint, 0,
			  sizeof(where), 0, &where);
	bool ok;
	UInt32 count;
	GetEventParameter(event, kEventParamClickCount, typeUInt32, 0,
			  sizeof(count), 0, &count);
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
static EventHandlerUPP mac_win_eventUPP = 0;
static void cleanup_win_eventUPP()
{
    DisposeEventHandlerUPP(mac_win_eventUPP);
    mac_win_eventUPP = 0;
}
static const EventHandlerUPP make_win_eventUPP()
{
    if(mac_win_eventUPP)
	return mac_win_eventUPP;
    qAddPostRoutine(cleanup_win_eventUPP);
    return mac_win_eventUPP = NewEventHandlerUPP(QWidgetPrivate::qt_window_event);
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
	if(qt_window_rgn((WId)window, s->regionCode, s->winRgn, true))
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
	QMacSavedPortInfo::flush(widget);
    } else {
	CopyRgn(rgn, outRgn);  //We don't know the widget, so let the Mac do the erasing..
    }
    return 0;
}

bool qt_mac_is_macdrawer(QWidget *w)
{
#if 0
    if(w && w->isTopLevel() && w->parentWidget() && w->testWFlags(Qt::WMacDrawer))
	return true;
#else
    Q_UNUSED(w);
#endif
    return false;
}

bool qt_mac_set_drawer_preferred_edge(QWidget *w, Qt::Dock where) //users of Qt/Mac can use this..
{
#if QT_MACOSX_VERSION >= 0x1020
    if(!qt_mac_is_macdrawer(w))
	return false;
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
	return false;
    SetDrawerPreferredEdge((WindowRef)w->handle(), bits);
    return true;
#else
    return false;
#endif
}

bool qt_mac_is_macsheet(QWidget *w, bool ignore_exclusion=false)
{
#if 0
    if(w && w->isTopLevel() && w->testWFlags(Qt::WStyle_DialogBorder) &&
       (ignore_exclusion || !w->testWFlags(Qt::WMacNoSheet)) &&
       w->parentWidget() && !w->parentWidget()->topLevelWidget()->isDesktop() &&
       w->parentWidget()->topLevelWidget()->isVisible()
       && ::qt_cast<QMacStyle *>(&w->style()))
	return true;
#else
    Q_UNUSED(w);
    Q_UNUSED(ignore_exclusion);
#endif
    return false;
}



/*****************************************************************************
  QWidgetPrivate member functions
 *****************************************************************************/
void QWidgetPrivate::qt_event_request_flush_updates()
{
    QList<WId> update_list = request_updates_pending_list;
    request_updates_pending_list.clear(); //clear now and let it get filled elsewhere
    for(QList<WId>::Iterator it = update_list.begin(); it != update_list.end(); ++it) {
	QWidget *widget = QWidget::find((*it));
	if(widget && widget->d->extra && widget->d->extra->has_dirty_area &&
	   widget->topLevelWidget()->isVisible()) {
	    widget->d->extra->has_dirty_area = FALSE;
	    QRegion r = widget->d->extra->dirty_area;
	    widget->d->extra->dirty_area = QRegion();
	    QRegion cr = widget->d->clippedRegion();
	    if(!widget->isTopLevel()) {
		QPoint point(posInWindow(widget));
		cr.translate(-point.x(), -point.y());
	    }
	    if(!r.isEmpty())
		widget->repaint(r & cr);
	}
    }
}

void QWidgetPrivate::propagateUpdates(bool update_rgn)
{
    QRegion rgn;
    QWidget *widg = q;
    if (update_rgn) {
	widg = q->topLevelWidget();
	QMacSavedPortInfo savedInfo(q);
	RgnHandle mac_rgn = qt_mac_get_rgn();
	GetWindowRegion((WindowPtr)q->hd, kWindowUpdateRgn, mac_rgn);
	if(EmptyRgn(mac_rgn)) {
	    qt_mac_dispose_rgn(mac_rgn);
	    return;
	}
	rgn = qt_mac_convert_mac_region(mac_rgn);
	rgn.translate(-widg->geometry().x(), -widg->geometry().y());
	qt_mac_dispose_rgn(mac_rgn);
	BeginUpdate((WindowPtr)q->hd);
    } else {
	rgn = QRegion(q->rect());
    }
#ifdef DEBUG_WINDOW_RGNS
    debug_wndw_rgn("*****propagatUpdates", widg, rgn, true);
#endif
    qt_paint_children(widg, rgn);
    if(update_rgn)
	EndUpdate((WindowPtr)q->hd);
}

void QWidgetPrivate::qt_mac_destroy_cg_hd(QWidget *w, bool children)
{
    if(w->cg_hd) { //just release it and another will be created later..
	if(CFGetRetainCount(w->cg_hd) == 1)
	    CGContextFlush((CGContextRef)w->cg_hd);
	CGContextRelease((CGContextRef)w->cg_hd);
	w->cg_hd = 0;
    }
    if(children) {
	QObjectList chldrn = w->children();
	for(int i = chldrn.size() - 1; i >= 0; --i) {
	    QObject *obj = chldrn.at(i);
	    if(obj->isWidgetType())
		qt_mac_destroy_cg_hd(((QWidget*)obj), true);
	}
    }
}

bool QWidgetPrivate::qt_mac_update_sizer(QWidget *w, int up=0)
{
    if(!w || !w->isTopLevel())
	return false;
    
    w->d->createTLExtra();
    w->d->extraData()->topextra->resizer += up;
    {
	WindowClass wclass;
	GetWindowClass((WindowPtr)w->handle(), &wclass);
	if(!(GetAvailableWindowAttributes(wclass) & kWindowResizableAttribute))
	    return TRUE;
    }
    bool remove_grip = (w->d->extraData()->topextra->resizer ||
			(w->d->extraData()->maxw && w->d->extraData()->maxh &&
			 w->d->extraData()->maxw == w->d->extraData()->minw && w->d->extraData()->maxh == w->d->extraData()->minh));
    
    WindowAttributes attr;
    GetWindowAttributes((WindowRef)w->handle(), &attr);
    if(remove_grip) {
	if(attr & kWindowResizableAttribute) {
	    ChangeWindowAttributes((WindowRef)w->handle(), kWindowNoAttributes,
                                   kWindowResizableAttribute);
	    w->d->dirtyClippedRegion(true);
	    ReshapeCustomWindow((WindowPtr)w->handle());
	    qt_dirty_wndw_rgn("Remove size grip", w, w->rect());
	}
    } else if(!(attr & kWindowResizableAttribute)) {
	ChangeWindowAttributes((WindowRef)w->handle(), kWindowResizableAttribute,
			       kWindowNoAttributes);
	w->d->dirtyClippedRegion(true);
	ReshapeCustomWindow((WindowPtr)w->handle());
	qt_dirty_wndw_rgn("Add size grip", w, w->rect());
    }
    return true;
}

bool QWidgetPrivate::qt_paint_children(QWidget *p, QRegion &r, uchar ops)
{
    if(qApp->closingDown() || qApp->startingUp() || !p || !p->isVisible() || r.isEmpty())
	return false;
    QPoint point(posInWindow(p));
    r.translate(point.x(), point.y());
    r &= p->d->clippedRegion(false); //at least sanity check the bounds
    if(r.isEmpty())
	return false;
    
    QObjectList chldrn = p->children();
    for(int i = chldrn.size() - 1; i >= 0; --i) {
	QObject *obj = chldrn.at(i);
	if(obj->isWidgetType()) {
	    QWidget *w = (QWidget *)obj;
	    QRegion clpr = w->d->clippedRegion(false);
	    if(!clpr.isEmpty() && !w->isTopLevel() &&
	       w->isVisible() && clpr.contains(r.boundingRect())) {
		QRegion wr = clpr & r;
		r -= wr;
		wr.translate(-(point.x() + w->x()), -(point.y() + w->y()));
		qt_paint_children(w, wr, ops);
		if(r.isEmpty())
		    return true;
	    }
	}
    }
    
    r.translate(-point.x(), -point.y());
    if((ops & PC_NoPaint)) {
	if(ops & PC_Later)
            qDebug("Qt: internal: Cannot use PC_NoPaint with PC_Later!");
    } else {
	if(ops & PC_Now) {
#ifdef DEBUG_WINDOW_RGN
	    debug_wndw_rgn("**paint_children2", p, r, true, true);
#endif
	    p->repaint(r);
	} else {
	    bool painted = false;
	    if(ops & PC_Later); //do nothing
            else if(!p->testWState(QWidget::WState_BlockUpdates)) {
		painted = true;
#ifdef DEBUG_WINDOW_RGN
                debug_wndw_rgn("**paint_children3", p, r, true, true);
#endif
		p->repaint(r);
	    }
	    if(!painted) {
#ifdef DEBUG_WINDOW_RGN
                debug_wndw_rgn("**paint_children4", p, r, true, true);
#endif
		p->update(r); //last try
	    } else if(p->d->extraData() && p->d->extraData()->has_dirty_area) {
#ifdef DEBUG_WINDOW_RGN
                debug_wndw_rgn("**paint_children5", p, r, true, true);
#endif
		qt_event_request_updates(p, r, true);
	    }
	}
    }
    return false;
}

static QList<QWidget *> qt_root_win_widgets;
static WindowPtr qt_root_win = 0;
void QWidgetPrivate::qt_clean_root_win() {
    for(int i = 0; i < qt_root_win_widgets.size(); ++i) {
	QWidget *w = qt_root_win_widgets.at(i);
	if(w->hd == qt_root_win) {
#if 0
	    warning("%s:%d: %s (%s) had his handle taken away!", __FILE__, __LINE__,
                    (*it)->name(), (*it)->className());
#endif
	    qt_mac_destroy_cg_hd(w, false);
	    w->hd = 0; //at least now we'll just crash
	}
    }
    DisposeWindow(qt_root_win);
    qt_root_win = 0;
}

bool QWidgetPrivate::qt_create_root_win() {
    if (qt_root_win)
	return false;
    Rect r;
    int w = 0, h = 0;
    for(GDHandle g = GetMainDevice(); g; g = GetNextDevice(g)) {
	w = qMax(w, (*g)->gdRect.right);
	h = qMax(h, (*g)->gdRect.bottom);
    }
    SetRect(&r, 0, 0, w, h);
    qt_mac_create_window(kOverlayWindowClass, kWindowNoAttributes, &r, &qt_root_win);
    if(!qt_root_win)
	return false;
    qAddPostRoutine(qt_clean_root_win);
    return true;
}

bool QWidgetPrivate::qt_recreate_root_win() {
    if(!qt_root_win)
	return false;
    WindowPtr old_root_win = qt_root_win;
    qt_root_win = 0;
    qt_create_root_win();
    for (int i = 0; i < qt_root_win_widgets.size(); ++i) {
	QWidget *w = qt_root_win_widgets.at(i);
	if (w->hd == old_root_win) {
	    w->hd = qt_root_win;
	}
    }
    //cleanup old window
    DisposeWindow(old_root_win);
    return true;
}

bool QWidgetPrivate::qt_window_rgn(WId id, short wcode, RgnHandle rgn, bool force = false)
{
    QWidget *widget = QWidget::find(id);
    if(QSysInfo::MacintoshVersion == QSysInfo::MV_10_DOT_1) {
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
                        if(!rpm.contains(QPoint(0, 0)) && rpm.boundingRect().topLeft() != QPoint(0, 0))
                            rpm |= QRegion(0, 0, 1, 1);
                        rpm.translate(x, (y + titlebar.boundingRect().height()));
                        titlebar += rpm;
                        CopyRgn(titlebar.handle(true), rgn);
		} else if(force) {
		    QRegion cr(x, y, widget->width(), widget->height());
		    CopyRgn(cr.handle(true), rgn);
		}
	    }
                return true; }
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
                        CopyRgn(rpm.handle(true), rgn);
                    } else if(force) {
                        QRegion cr(x, y, widget->width(), widget->height());
                        CopyRgn(cr.handle(true), rgn);
                    }
                }
                return true; }
            default: break;
	}
    } else {
	switch(wcode) {
            case kWindowStructureRgn: {
                bool ret = false;
                if(widget) {
                    if(widget->d->extra && !widget->d->extra->mask.isEmpty()) {
                        QRegion rin = qt_mac_convert_mac_region(rgn);
                        if(!rin.isEmpty()) {
                            QPoint rin_tl = rin.boundingRect().topLeft(); //in offset
                            rin.translate(-rin_tl.x(), -rin_tl.y()); //bring into same space as below
                            QRegion mask = widget->d->extra->mask;
                            if(!widget->testWFlags(Qt::WStyle_Customize) || !widget->testWFlags(Qt::WStyle_NoBorder)) {
                                QRegion title;
                                {
                                    RgnHandle rgn = qt_mac_get_rgn();
                                    GetWindowRegion((WindowPtr)widget->handle(), kWindowTitleBarRgn, rgn);
                                    title = qt_mac_convert_mac_region(rgn);
                                    qt_mac_dispose_rgn(rgn);
                                }
                                QRect br = title.boundingRect();
                                mask.translate(0, br.height()); //put the mask 'under' the titlebar..
                                title.translate(-br.x(), -br.y());
                                mask += title;
                            }
                            
                            QRegion cr = rin & mask;
                            cr.translate(rin_tl.x(), rin_tl.y()); //translate back to incoming space
                            CopyRgn(cr.handle(true), rgn);
                        }
                        ret = true;
                    } else if(force) {
                        QRegion cr(widget->geometry());
                        CopyRgn(cr.handle(true), rgn);
                        ret = true;
                    }
                }
                return ret; }
            default: break;
	}
    }
    return false;
}

/*****************************************************************************
  QWidget member functions
 *****************************************************************************/
void QWidget::create(WId window, bool initializeWindow, bool destroyOldWindow)
{
    d->window_event = 0;
    d->own_id = 0;
    HANDLE destroyw = 0;
    setWState(WState_Created);                        // set created flag

    if(!parentWidget() || parentWidget()->isDesktop())
	setWFlags(WType_TopLevel);            // top-level widget

    QRect dskr;
    if(isDesktop()) {
	int w = 0, h = 0;
	for(GDHandle g = GetMainDevice(); g; g = GetNextDevice(g)) {
	    w = qMax(w, (*g)->gdRect.right);
	    h = qMax(h, (*g)->gdRect.bottom);
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
	initializeWindow=true;

    if(dialog || popup || desktop) {          // these are top-level, too
	topLevel = true;
	setWFlags(WType_TopLevel);
	if(popup)
	    setWFlags(WStyle_Tool|WStyle_StaysOnTop); // a popup is a tool window
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
	dialog = popup = false;                  // force these flags off
	data->crect.setRect(0, 0, sw, sh);
    } else if(topLevel) {                    // calc pos/size from screen
	data->crect.setRect(sw/4, 3*sh/10, sw/2, 4*sh/10);
    } else {                                    // child widget
	data->crect.setRect(0, 0, 100, 30);
    }

    if(window) {				// override the old window
	if(destroyOldWindow && d->own_id)
	    destroyw = hd;
	d->own_id = 0; //it has become mine!
	id = window;
	hd = (Qt::HANDLE)id;
	setWinId(id);
    } else if(desktop) {			// desktop widget
	if(!qt_root_win)
            QWidgetPrivate::qt_create_root_win();
	qt_root_win_widgets.append(this);
	hd = (void *)qt_root_win;
	id = (WId)hd;
	d->own_id = 0;
	setWinId(id);
    } else if(isTopLevel()) {
	d->own_id = 1; //I created it, I own it

	Rect r;
	SetRect(&r, data->crect.left(), data->crect.top(), data->crect.left(), data->crect.top());
	WindowClass wclass = kSheetWindowClass;
	if(popup || testWFlags(WStyle_Splash) == WStyle_Splash)
	    wclass = kModalWindowClass;
	else if(testWFlags(WShowModal))
	    wclass = kMovableModalWindowClass;
#if QT_MACOSX_VERSION >= 0x1020
	else if(qt_mac_is_macdrawer(this))
	    wclass = kDrawerWindowClass;
#endif
	else if (testWFlags(WStyle_Tool) && qstrcmp(objectName(), "toolTipTip") == 0) // Tool tips
	    wclass = kHelpWindowClass;
	else if(testWFlags(WStyle_Tool)
                || (dialog && parentWidget() && !parentWidget()->topLevelWidget()->isDesktop()))
	    wclass = kFloatingWindowClass;
	else if(dialog)
	    wclass = kToolbarWindowClass;
	else
	    wclass = kDocumentWindowClass;

	WindowGroupRef grp = 0;
	WindowAttributes wattr = kWindowNoAttributes;
	if(testWFlags(WStyle_Customize)) {
	    if(qt_mac_is_macsheet(this)) {
		grp = GetWindowGroupOfClass(kMovableModalWindowClass);
		wclass = kSheetWindowClass;
	    } else {
		grp = GetWindowGroupOfClass(wclass);
                // Shift things around a bit to get the correct window class based on the presence
                // (or lack) of the border.
		if(testWFlags(WStyle_NoBorder)) {
		    if(wclass == kDocumentWindowClass)
			wclass = kPlainWindowClass;
		    else if(wclass == kFloatingWindowClass)
			wclass = kToolbarWindowClass;
		} else {
		    if(wclass != kModalWindowClass)
			wattr |= kWindowResizableAttribute;
                    if(wclass == kToolbarWindowClass) {
                        if(!parentWidget() || parentWidget()->isDesktop())
                            wclass = kDocumentWindowClass;
                        else
                            wclass = kFloatingWindowClass;
                    }
		}
		// Only add extra decorations (well, buttons) for widgets that can have them
		// and have an actual border we can put them on.
                if(wclass != kModalWindowClass && wclass != kMovableModalWindowClass
                    && wclass != kSheetWindowClass && wclass != kPlainWindowClass
                    && !testWFlags(WStyle_NoBorder)) {
		    if(testWFlags(WStyle_Maximize))
			wattr |= kWindowFullZoomAttribute;
		    if(testWFlags(WStyle_Minimize))
			wattr |= kWindowCollapseBoxAttribute;
		    if(testWFlags(WStyle_Title) || testWFlags(WStyle_SysMenu))
		       wattr |= kWindowCloseBoxAttribute;
		}
	    }
	}
	if(testWFlags(Qt::WMacMetal))
	    wattr |= kWindowMetalAttribute;
	if(testWFlags(WStyle_Tool))
	    wattr |= kWindowHideOnSuspendAttribute;
	wattr |= kWindowLiveResizeAttribute;
	if(testWFlags(WStyle_Tool) && testWFlags(WStyle_Splash) != WStyle_Splash)
	    wattr |= kWindowHideOnSuspendAttribute;

#ifdef DEBUG_WINDOW_CREATE
#define ADD_DEBUG_WINDOW_NAME(x) { x, #x }
	struct {
	    UInt32 tag;
	    const char *name;
	} known_attribs[] = {
	    ADD_DEBUG_WINDOW_NAME(kWindowMetalAttribute),
	    ADD_DEBUG_WINDOW_NAME(kWindowHideOnSuspendAttribute),
	    ADD_DEBUG_WINDOW_NAME(kWindowStandardHandlerAttribute),
	    ADD_DEBUG_WINDOW_NAME(kWindowCollapseBoxAttribute),
	    ADD_DEBUG_WINDOW_NAME(kWindowHorizontalZoomAttribute),
	    ADD_DEBUG_WINDOW_NAME(kWindowVerticalZoomAttribute),
	    ADD_DEBUG_WINDOW_NAME(kWindowResizableAttribute),
	    ADD_DEBUG_WINDOW_NAME(kWindowNoActivatesAttribute),
	    ADD_DEBUG_WINDOW_NAME(kWindowLiveResizeAttribute),
	    ADD_DEBUG_WINDOW_NAME(kWindowCloseBoxAttribute),
	    ADD_DEBUG_WINDOW_NAME(kWindowHideOnSuspendAttribute),
	    { 0, 0 }
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
	    { 0, 0 }
	};
#undef ADD_DEBUG_WINDOW_NAME
	qDebug("Qt: internal: ************* Creating new window (%s::%s)", className(), objectName());
	bool found_class = false;
	for(int i = 0; known_classes[i].name; i++) {
	    if(wclass == known_classes[i].tag) {
		found_class = true;
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
	    if(OSStatus ret = qt_mac_create_window(wclass, wattr, &r, (WindowRef *)&id))
		qDebug("Qt: internal: %s:%d If you reach this error please contact Trolltech and include the\n"
		       "      WidgetFlags used in creating the widget (%ld)", __FILE__, __LINE__, ret);
	    if(!desktop) { 	//setup an event callback handler on the window
		InstallWindowEventHandler((WindowRef)id, make_win_eventUPP(),
					  GetEventTypeCount(window_events),
					  window_events, static_cast<void *>(qApp), &d->window_event);
	    }
	}

	if(wclass == kFloatingWindowClass) //these dialogs don't hide
	    ChangeWindowAttributes((WindowRef)id, kWindowNoAttributes, kWindowNoActivatesAttribute);
#if QT_MACOSX_VERSION >= 0x1020
	if(qt_mac_is_macdrawer(this))
	    SetDrawerParent((WindowRef)id, (WindowRef)parentWidget()->handle());
#endif
	if(dialog && !parentWidget() && !testWFlags(WShowModal))
	    grp = GetWindowGroupOfClass(kDocumentWindowClass);
	if(testWFlags(WStyle_StaysOnTop)) {
	    d->createTLExtra();
	    if(d->topData()->group)
		qt_mac_release_window_group(d->topData()->group);
	    d->topData()->group = qt_mac_get_stays_on_top_group();
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
#if 0
	//We cannot use a window content paint proc because it causes problems on 10.2 (it
	//is buggy). We have an outstanding issue with Apple right now.
	InstallWindowContentPaintProc((WindowPtr)id, NewWindowPaintUPP(qt_erase), 0, this);
#endif
	if(testWFlags(WType_Popup) || testWFlags(WStyle_Tool))
	    SetWindowModality((WindowPtr)id, kWindowModalityNone, NULL);
	data->fstrut_dirty = TRUE; // when we create a toplevel widget, the frame strut should be dirty
	hd = (void *)id;
	if(!mac_window_count++)
	    QMacSavedPortInfo::setPaintDevice(this);
	setWinId(id);
	if(d->extra && !d->extra->mask.isEmpty())
	   ReshapeCustomWindow((WindowPtr)hd);
	if(qt_mac_is_macsheet(this))
	    setWindowOpacity(0.70);
#if QT_MACOSX_VERSION >= 0x1020
	else if(qt_mac_is_macdrawer(this))
	    SetDrawerOffsets((WindowPtr)hd, 0.0, 25.0);
#endif
    } else {
	while(QWidget::find(++serial_id));
	setWinId(serial_id);
	id = serial_id;
	hd = topLevelWidget()->hd;
	data->fstrut_dirty = false; // non-toplevel widgets don't have a frame, so no need to update the strut
	setWinId(id);
    }

    if(desktop) { //immediately "show" a "desktop"
	setWState(WState_Visible);
    } else {
	clearWState(WState_Visible);
	d->dirtyClippedRegion(true);
    }
    d->macDropEnabled = false;

    if(destroyw) {
	mac_window_count--;
	DisposeWindow((WindowPtr)destroyw);
    }
    qt_mac_unicode_init(this);
}

void QWidget::destroy(bool destroyWindow, bool destroySubWindows)
{
    deactivateWidgetCleanup();
    qt_mac_event_release(this);
    qt_mac_unicode_cleanup(this);
    if(isDesktop() && hd == qt_root_win && destroyWindow && d->own_id)
	qt_root_win_widgets.remove(this);
    if(testWState(WState_Created)) {
	d->dirtyClippedRegion(true);
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
	    setAcceptDrops(false);

        if(testWFlags(WShowModal))          // just be sure we leave modal
            qt_leave_modal(this);
        else if(testWFlags(WType_Popup))
            qApp->closePopup(this);
	if(destroyWindow) {
	    d->qt_mac_destroy_cg_hd(this, false);
	    if(isTopLevel() && hd && d->own_id) {
		mac_window_count--;
		if(d->window_event) {
		    RemoveEventHandler(d->window_event);
		    d->window_event = 0;
		}
		DisposeWindow((WindowPtr)hd);
	    }
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
    d->dirtyClippedRegion(true);

    QCursor oldcurs;
    bool setcurs=testAttribute(WA_SetCursor);
    if(setcurs) {
	oldcurs = cursor();
	unsetCursor();
    }

    EventHandlerRef old_window_event = 0;
    WindowPtr old_hd = 0;
    if(!isDesktop()) {
	old_hd = (WindowPtr)hd;
	old_window_event = d->window_event;
    }
    QWidget* oldtlw = topLevelWidget();
    reparentFocusWidgets(parent);		// fix focus chains

    setWinId(0);
    QObject::setParent_helper(parent);
    bool     dropable = acceptDrops();
    bool     enable = isEnabled();
    bool     owned = d->own_id;
    FocusPolicy fp = focusPolicy();
    QSize    s	    = size();
    QString capt = windowTitle();
    data->widget_flags = f;
    clearWState(WState_Created | WState_Visible | WState_Hidden | WState_ExplicitShowHide);
    create();
    if(isTopLevel() || (!parent || parent->isVisible()))
	setWState(WState_Hidden);
    if(dropable)
	setAcceptDrops(false);

    //get new hd, now move
    no_move_blt = true;
    setGeometry(p.x(), p.y(), s.width(), s.height());
    no_move_blt = false;

    //reset flags and show (if neccesary)
    setEnabled(enable);
    setFocusPolicy(fp);
    setAcceptDrops(dropable);
    if(!capt.isNull()) {
	d->topData()->caption = QString::null;
	setWindowTitle(capt);
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
		d->qt_mac_destroy_cg_hd(w, false);
	    }
	}
    }
    reparentFocusWidgets(oldtlw);

    //repaint the new area, on the window parent
    if(isVisible()) //finally paint my new area
	qt_dirty_wndw_rgn("reparent2",this, mac_rect(posInWindow(this), geometry().size()));

    //cleanup
    if(old_hd && owned) { //don't need old window anymore
	mac_window_count--;
	if(old_window_event) {
	    RemoveEventHandler(old_window_event);
	    old_window_event = 0;
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
    for(const QWidget *p = this; p && !p->isTopLevel(); p = p->parentWidget()) {
	mac_p.h -= p->geometry().x();
	mac_p.v -= p->geometry().y();
    }
    return QPoint(mac_p.h, mac_p.v);
}

void QWidget::setMicroFocusHint(int x, int y, int width, int height, bool text, QFont *)
{
    if(!width)
	width = 1;
    if(!height)
	height = 1;
    if(text && QRect(x, y, width, height) != microFocusHint()) {
	d->createExtra();
	d->extraData()->micro_focus_hint.setRect(x, y, width, height);
    }
}

void QWidgetPrivate::setFont_syshelper(QFont *)
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

    if(qApp && qApp->activeWindow() && QApplication::widgetAt(QCursor::pos()) == this) {
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

    if(qApp && qApp->activeWindow() && QApplication::widgetAt(QCursor::pos()) == this) {
	Point mouse_pos;
	QPoint qmp(QCursor::pos());
	mouse_pos.h = qmp.x();
	mouse_pos.v = qmp.y();

	const QCursor *n = 0;
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

void QWidget::setWindowModified(bool mod)
{
    setAttribute(WA_WindowModified, mod);
    if(isTopLevel())
	SetWindowModified((WindowRef)handle(), mod);
    QEvent e(QEvent::ModifiedChange);
    QApplication::sendEvent(this, &e);
}

bool QWidget::isWindowModified() const
{
    return testAttribute(WA_WindowModified);
}

void QWidget::setWindowTitle(const QString &cap)
{
    if(d->topData() && d->topData()->caption == cap)
	return; // for less flicker
    d->createTLExtra();
    d->topData()->caption = cap;
    if(isTopLevel()) {
	CFStringRef str = CFStringCreateWithCharacters(0, (UniChar *)cap.unicode(), cap.length());
	SetWindowTitleWithCFString((WindowPtr)hd, str);
    }
    QEvent e(QEvent::WindowTitleChange);
    QApplication::sendEvent(this, &e);
}

void QWidget::setWindowIcon(const QPixmap &pixmap)
{
    if(d->topData()) {
	delete d->topData()->icon;
	d->topData()->icon = 0;
    } else {
	d->createTLExtra();
    }
    if(!pixmap.isNull())
	d->topData()->icon = new QPixmap(pixmap);
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
		CGDataProviderRef dp = CGDataProviderCreateWithData(0, i.bits(), i.numBytes(), 0);
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
	SetWindowProxyIcon((WindowRef)handle(), qt_mac_create_iconref(pixmap));
    }
    QEvent e( QEvent::WindowIconChange );
    QApplication::sendEvent( this, &e );
}

void QWidget::setWindowIconText(const QString &iconText)
{
    d->createTLExtra();
    d->topData()->iconText = iconText;
    QEvent e( QEvent::IconTextChange );
    QApplication::sendEvent( this, &e );
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
	mac_mouse_grabber = 0;
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
	mac_keyboard_grabber = 0;
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
    qt_event_remove_activate();
    if(tlw->isPopup() || tlw->testWFlags(WStyle_Tool)) {
	ActivateWindow((WindowPtr)tlw->handle(), true);
    } else {
	if(IsWindowActive((WindowPtr)tlw->handle())) {
	    ActivateWindow((WindowPtr)tlw->handle(), true);
	    qApp->setActiveWindow(tlw);
	} else if(!isMinimized()){
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
    if(!testWState(WState_BlockUpdates) && isVisible() && !d->clippedRegion().isEmpty()) {
	if(w < 0)
	    w = data->crect.width()  - x;
	if(h < 0)
	    h = data->crect.height() - y;
	if(w && h) {
	    QRegion r(x, y, w, h);
	    qt_event_request_updates(this, r);
#ifdef DEBUG_WINDOW_RGN
	    debug_wndw_rgn("update1", this, r, false, true);
#endif
	}
    }
}

void QWidget::update(const QRegion &rgn)
{
    if(!testWState(WState_BlockUpdates) && isVisible() && !d->clippedRegion().isEmpty()) {
	qt_event_request_updates(this, rgn);
#ifdef DEBUG_WINDOW_RGN
	debug_wndw_rgn("update2", this, rgn, false, true);
#endif
    }
}

void QWidget::repaint(const QRegion &rgn)
{
    if (testWState(WState_InPaintEvent))
	qWarning("QWidget::repaint: recursive repaint detected.");

    if ( (data->widget_state & (WState_Visible|WState_BlockUpdates)) != WState_Visible )
	return;

    if (rgn.isEmpty())
	return;

    setWState(WState_InPaintEvent);
    qt_set_paintevent_clipping(this, rgn, 0);

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
		    bool was_in_paint_event = w->testWState(WState_InPaintEvent);
		    w->setWState(WState_InPaintEvent);
		    QApplication::sendEvent(w, &e);
		    if(!was_in_paint_event)
			w->clearWState(WState_InPaintEvent);
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

    if (testAttribute(WA_ContentsPropagated))
	d->updatePropagatedBackground(&rgn);
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
		if(r.bottom() > avail.bottom())
		    movey = avail.bottom() - r.height();
		if(r.right() > avail.right())
		    movex = avail.right() - r.width();
		// +2 to prevent going under the menu bar
		move(qMax(avail.left(), movex), qMax(avail.top() + 2, movey));
	}
    }
    data->fstrut_dirty = true;
    d->dirtyClippedRegion(true);
    if(isTopLevel()) {
	SizeWindow((WindowPtr)hd, width(), height(), true);
	if(qt_mac_is_macsheet(this))
	    qt_event_request_showsheet(this);
#if QT_MACOSX_VERSION >= 0x1020
	else if(qt_mac_is_macdrawer(this))
	    OpenDrawer((WindowPtr)hd, kWindowEdgeDefault, true);
#endif
	else {
	    ShowHide((WindowPtr)hd, true);	//now actually show it
            // it seems that collapse window doesn't work unless the window is actually shown,
            // so catch it again.
            if(windowState() & WindowMinimized)
                CollapseWindow((WindowPtr)hd, true);
        }
#ifndef QMAC_NO_FAKECURSOR
	if(qstrcmp(name(), "fake_cursor") != 0)
#endif
	    qt_event_request_activate(this);
    } else if(!parentWidget() || parentWidget()->isVisible()) {
	qt_dirty_wndw_rgn("show",this, mac_rect(posInWindow(this), geometry().size()));
    }
}

void QWidget::hideWindow()
{
    if(isDesktop()) //you can't hide the desktop!
	return;

    d->dirtyClippedRegion(true);
    if(isTopLevel()) {
#if QT_MACOSX_VERSION >= 0x1020
	if(qt_mac_is_macdrawer(this))
	    CloseDrawer((WindowPtr)hd, true);
	else
#endif
       if(qt_mac_is_macsheet(this))
           HideSheetWindow((WindowPtr)hd);
	else
	    ShowHide((WindowPtr)hd, false); //now we hide
	SizeWindow((WindowPtr)hd, 0, 0, true);
	if(isActiveWindow()) {
	    QWidget *w = 0;
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
    } else if(!parentWidget() || parentWidget()->isVisible()) { //strange!! ###
	qt_dirty_wndw_rgn("hide",this, mac_rect(posInWindow(this), geometry().size()));
    }
    deactivateWidgetCleanup();
    qt_mac_event_release(this);
}

void QWidget::setWindowState(uint newstate)
{
    uint oldstate = windowState();

    bool needShow = FALSE;
    if (isTopLevel()) {
	if ((oldstate & WindowMinimized) != (newstate & WindowMinimized))
	    CollapseWindow((WindowPtr)hd, (newstate & WindowMinimized) ? true : false);

	if((oldstate & WindowFullScreen) != (newstate & WindowFullScreen)) {
	    if(newstate & WindowFullScreen) {
		if(QTLWExtra *tlextra = d->topData()) {
		    if(tlextra->normalGeometry.width() < 0) {
			if(testAttribute(WA_Resized))
			    tlextra->normalGeometry = geometry();
			else
			    tlextra->normalGeometry = QRect(pos(), qt_initial_size(this));
		    }
		    tlextra->savedFlags = getWFlags();
		}
		setParent(0, WType_TopLevel | WStyle_Customize | WStyle_NoBorder |
			  (getWFlags() & 0xffff0000)); 			  // preserve some widget flags
                // This is a bit evil, but it keeps things like toolbars from being
                // obscured by the menubar as listed in task 41205.
		int finalScreen = qApp->desktop()->screenNumber(this);
		QRect screen = qApp->desktop()->screenGeometry(finalScreen);
		GDHandle g = GetMainDevice();
		int i = 0;
		while (i < finalScreen) {
		    g = GetNextDevice(g);
		    ++i;
		}
		RgnHandle rgn = NewRgn();
		// Give us the region that basically is the screen minus menubar and rectangle
		// the dock has -- a tad bit more than GetAvailableWindowPositioningBounds.
		if (GetAvailableWindowPositioningRegion(g, rgn) == noErr) {
		    Rect r;
		    GetRegionBounds(rgn, &r);
		    screen = QRect(r.left, r.top, r.right - r.left, r.bottom - r.top);
		}
		DisposeRgn(rgn);
		move(screen.topLeft());
		resize(screen.size());
	    } else {
		setParent(0, d->topData()->savedFlags);
		setGeometry(d->topData()->normalGeometry);
	    }
	}

	if((oldstate & WindowMaximized) != (newstate & WindowMaximized)) {
	    if(newstate & WindowMaximized) {
		Rect bounds;
		data->fstrut_dirty = TRUE;
		QDesktopWidget *dsk = QApplication::desktop();
		QRect avail = dsk->availableGeometry(dsk->screenNumber(this));
		SetRect(&bounds, avail.x(), avail.y(), avail.x() + avail.width(), avail.y() + avail.height());
		if(QWExtra *extra = d->extraData()) {
		    if(bounds.right - bounds.left > extra->maxw)
			bounds.right = bounds.left + extra->maxw;
		    if(bounds.bottom - bounds.top > extra->maxh)
			bounds.bottom = bounds.top + extra->maxh;
		}
		if(QTLWExtra *tlextra = d->topData()) {
		    if(tlextra->normalGeometry.width() < 0) {
			if(testAttribute(WA_Resized))
			    tlextra->normalGeometry = geometry();
			else
			    tlextra->normalGeometry = QRect(pos(), qt_initial_size(this));
		    }
		    if(data->fstrut_dirty)
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
		    if(QTLWExtra *tlextra = d->topData()) 
			SetRect(&oldr, tlextra->normalGeometry.left(), tlextra->normalGeometry.top(), 
				tlextra->normalGeometry.right()+1, tlextra->normalGeometry.bottom()+1);
		    else
			SetRect(&oldr, orect.x(), orect.y(), orect.right(), orect.bottom());
		    SetWindowUserState((WindowPtr)hd, &oldr);
		    qt_dirty_wndw_rgn("showMaxim", this, mac_rect(rect()));
		    SetWindowStandardState((WindowPtr)hd, &bounds);
		    ZoomWindow((WindowPtr)hd, inZoomOut, false);

		    data->crect = nrect;
		    if(isVisible()) {
			d->dirtyClippedRegion(TRUE);
			//issue a resize
			QResizeEvent qre(size(), orect.size());
			QApplication::sendEvent(this, &qre);
			//issue a move
			QMoveEvent qme(pos(), orect.topLeft());
			QApplication::sendEvent(this, &qme);
		    }
		}
	    } else {
		ZoomWindow((WindowPtr)hd, inZoomIn, false);
		if(QTLWExtra *tlextra = topData()) {
		    if(tlextra->normalGeometry.width() < 0)
			clearWState(WState_Resized);
		}
		Rect bounds;
		ZoomWindow((WindowPtr)hd, inZoomIn, false);
		GetPortBounds(GetWindowPort((WindowPtr)hd), &bounds);
		qt_dirty_wndw_rgn("un-maximize",this, &bounds);
	    }
	}
    }

    data->widget_state &= ~(WState_Minimized | WState_Maximized | WState_FullScreen);
    if (newstate & WindowMinimized)
	data->widget_state |= WState_Minimized;
    if (newstate & WindowMaximized)
	data->widget_state |= WState_Maximized;
    if (newstate & WindowFullScreen)
	data->widget_state |= WState_FullScreen;

    if(needShow)
	show();

    if(newstate & WindowActive)
	setActiveWindow();

    QEvent e(QEvent::WindowStateChange);
    QApplication::sendEvent(this, &e);
}

void QWidget::raise()
{
    if(isDesktop())
	return;
    if(isTopLevel()) {
	//raise this window
	BringToFront((WindowPtr)hd);
	//we get to be the active process now
	ProcessSerialNumber psn;
	GetCurrentProcess(&psn);
	SetFrontProcessWithOptions(&psn, kSetFrontProcessFrontWindowOnly);
    } else if(QWidget *p = parentWidget()) {
	QRegion clp;
	if(isVisible())
	    clp = d->clippedRegion(false);
	if(p->d->children.indexOf(this) >= 0) {
	    p->d->children.remove(this);
	    p->d->children.append(this);
	}
	if(isVisible()) {
	    d->dirtyClippedRegion(true);
	    clp ^= d->clippedRegion(false);
            qt_dirty_wndw_rgn("raise", this, clp);
	}
    }
}

void QWidget::lower()
{
    if(isDesktop())
	return;

    if(isTopLevel()) {
	SendBehind((WindowPtr)handle(), 0);
    } else if(QWidget *p = parentWidget()) {
	QRegion clp;
	if(isVisible())
	    clp = d->clippedRegion(false);
	if(p->d->children.indexOf(this) >= 0) {
	    p->d->children.remove(this);
	    p->d->children.insert(0, this);
	}
	if(isVisible()) {
	    d->dirtyClippedRegion(true);
	    clp ^= d->clippedRegion(false);
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
    int loc = p->d->children.indexOf(w);
    QRegion clp;
    if(isVisible())
	clp = d->clippedRegion(false);
    if(loc >= 0 && p->d->children.indexOf(this) >= 0) {
	p->d->children.remove(this);
	p->d->children.insert(loc, this);
    }
    if(isVisible()) {
	d->dirtyClippedRegion(true);
	clp ^= d->clippedRegion(false);
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
            QWidgetPrivate::qt_mac_update_sizer(this);
	    if (testWFlags(WStyle_Maximize)) {
		if (extra->maxw && extra->maxh && extra->maxw == extra->minw
			&& extra->maxh == extra->minh)
		    ChangeWindowAttributes((WindowRef)handle(), kWindowNoAttributes,
					   kWindowFullZoomAttribute);
		else
		    ChangeWindowAttributes((WindowRef)handle(), kWindowFullZoomAttribute,
					   kWindowNoAttributes);
	    }
	}
	w = qMin(w,extra->maxw);
	h = qMin(h,extra->maxh);
	w = qMax(w,extra->minw);
	h = qMax(h,extra->minh);

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
	oldregion = d->clippedRegion(false);
	d->dirtyClippedRegion(false);
	data->crect = QRect(x, y, w, h);
	d->dirtyClippedRegion(true);
    } else {
	data->crect = QRect(x, y, w, h);
    }

    bool isResize = (olds != size());
    if(isTopLevel() && data->winid && d->own_id) {
	if(isResize && isMaximized())
	    clearWState(WState_Maximized);
	if (isMove)
	    MoveWindow((WindowPtr)hd, x, y, false);
	if (isResize)
	    SizeWindow((WindowPtr)hd, w, h, true);
	d->dirtyClippedRegion(TRUE);
    }

    if(isMove || isResize) {
	if(isResize && isTopLevel())
            QWidgetPrivate::qt_mac_destroy_cg_hd(this, true);
	if(!visible) {
	    if (isMove && pos() != oldp)
		setAttribute(WA_PendingMoveEvent, true);
	    if (isResize)
		setAttribute(WA_PendingResizeEvent, true);
	} else {
	    QRegion bltregion, clpreg = d->clippedRegion(false);
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
		    QWidget *p = parentWidget();
		    if(!p)
			p = this;
		    QMacSavedPortInfo pi(p, bltregion);
		    unclippedBitBlt(p, pos().x(), pos().y(), p, oldp.x(), oldp.y(),
				    olds.width(), olds.height(), Qt::CopyROP, true, true);
		}
	    }
	    if((!newreg_empty || !oldreg_empty) &&
	       (isResize || !isTopLevel() || !QDIsPortBuffered(GetWindowPort((WindowPtr)hd)))) {
		//finally issue "expose" event
		QRegion upd((oldregion + clpreg) - bltregion);
		if(isResize && !testAttribute(WA_StaticContents))
		    upd += d->clippedRegion();
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
	bool resized = testAttribute(WA_Resized);
	resize(qMax(minw,width()), qMax(minh,height()));
	setAttribute(WA_Resized, resized); //not a user resize
    }
    updateGeometry();
}

void QWidget::setMaximumSize(int maxw, int maxh)
{
    if(maxw > QWIDGETSIZE_MAX || maxh > QWIDGETSIZE_MAX) {
	qWarning("Qt: QWidget::setMaximumSize: (%s/%s) "
		"The largest allowed size is (%d,%d)",
		 objectName("unnamed"), className(), QWIDGETSIZE_MAX,
		QWIDGETSIZE_MAX);
	maxw = qMin(maxw, QWIDGETSIZE_MAX);
	maxh = qMin(maxh, QWIDGETSIZE_MAX);
    }
    if(maxw < 0 || maxh < 0) {
	qWarning("Qt: QWidget::setMaximumSize: (%s/%s) Negative sizes (%d,%d) "
		"are not possible",
		 objectName("unnamed"), className(), maxw, maxh);
	maxw = qMax(maxw, 0);
	maxh = qMax(maxh, 0);
    }
    d->createExtra();
    if(d->extraData()->maxw == maxw && d->extraData()->maxh == maxh)
	return;
    d->extraData()->maxw = maxw;
    d->extraData()->maxh = maxh;
    if(maxw < width() || maxh < height()) {
	bool resized = testAttribute(WA_Resized);
	resize(qMin(maxw,width()), qMin(maxh,height()));
	setAttribute(WA_Resized, resized); //not a user resize
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
	bltd = d->clippedRegion(valid_rect); //offset the clip
	bltd.translate(dx, dy);
	QRegion requested(x2, y2, w, h); //only that which I blt to
	requested.translate(p.x(), p.y());
	bltd &= requested;
	bltd &= d->clippedRegion(valid_rect); //finally clip to clipping region
	{   //can't blt that which is dirty
	    RgnHandle r = qt_mac_get_rgn();
	    GetWindowRegion((WindowPtr)hd, kWindowUpdateRgn, r);
	    if(!EmptyRgn(r)) {
		QRegion dirty = qt_mac_convert_mac_region(r); //the dirty region
		dirty.translate(-topLevelWidget()->geometry().x(),
				-topLevelWidget()->geometry().y());
		bltd -= dirty;
	    }
	    qt_mac_dispose_rgn(r);
	}
    }
    d->dirtyClippedRegion(true);
    if(!valid_rect) {	// scroll children
	QPoint pd(dx, dy);
	QWidgetList moved;
	QObjectList chldrn = children();
	for(int i = 0; i < chldrn.size(); i++) {  //first move all children
	    QObject *obj = chldrn.at(i);
	    if(obj->isWidgetType()) {
		QWidget *w = (QWidget*)obj;
		w->data->crect = QRect(w->pos() + pd, w->size());
		moved.append(w);
	    }
	}
        //now send move events (do not do this in the above loop, breaks QAquaFocusWidget)
	for(int i = 0; i < moved.size(); i++) {
	    QWidget *w = moved.at(i);
	    w->d->dirtyClippedRegion(true);
	    QMoveEvent e(w->pos(), w->pos() - pd);
	    QApplication::sendEvent(w, &e);
	}
    }
    {
	QMacSavedPortInfo pi(this, bltd);
	unclippedBitBlt(this,x2,y2,this,x1,y1,w,h,Qt::CopyROP,true,true);
    }

    if(just_update)
	return;
    QRegion newarea(sr);
    newarea.translate(p.x(), p.y());
    newarea &= (d->clippedRegion(valid_rect) - bltd);
    qt_clean_wndw_rgn("scroll", this, newarea);
    newarea.translate(-p.x(), -p.y());
    QWidgetPrivate::qt_paint_children(this, newarea, QWidgetPrivate::PC_None);
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
	    return data->crect.width();
	Rect windowBounds;
	GetPortBounds(GetWindowPort(((WindowPtr)hd)), &windowBounds);
	return windowBounds.right;
    }

    case QPaintDeviceMetrics::PdmHeight:
    {
	if(!isTopLevel())
	    return data->crect.height();
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
    extra->has_dirty_area = false;
    extra->child_serial = extra->clip_serial = 1;
    extra->child_dirty = extra->clip_dirty = true;
    extra->macDndExtra = 0;
}

void QWidgetPrivate::deleteSysExtra()
{
}

void QWidgetPrivate::createTLSysExtra()
{
    extra->topextra->group = 0;
    extra->topextra->is_moved = 0;
    extra->topextra->resizer = 0;
}

void QWidgetPrivate::deleteTLSysExtra()
{
    if(extra->topextra->group)
	qt_mac_release_window_group(extra->topextra->group);
}

bool QWidget::acceptDrops() const
{
    return d->macDropEnabled;
}

void QWidget::updateFrameStrut() const
{
    QWidget *that = (QWidget *) this; //mutable
    if(!data->fstrut_dirty) {
	that->data->fstrut_dirty = isVisible();
	return;
    }
    that->data->fstrut_dirty = false;
    QTLWExtra *top = that->d->topData();
    top->fleft = top->fright = top->ftop = top->fbottom = 0;
    if(!isDesktop() && isTopLevel()) {
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
    if((on && d->macDropEnabled) || (!on && !d->macDropEnabled))
	return;
    d->macDropEnabled = on;
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
	clp = d->clippedRegion(false);
    d->extra->mask = region;
    if(isVisible()) {
	d->dirtyClippedRegion(true);
	clp ^= d->clippedRegion(false);
	qt_dirty_wndw_rgn("setMask", this, clp);
    }
    if(isTopLevel())
	ReshapeCustomWindow((WindowPtr)hd); //now let the wdef take it
}

void QWidget::setMask(const QBitmap &bitmap)
{
    setMask(QRegion(bitmap));
}


void QWidget::clearMask()
{
    setMask(QRegion());
}

/*!
    \internal
*/
void QWidgetPrivate::setRegionDirty(bool child)
{
    QWExtra *extra = extraData();
    if(!extra)
	return;
    if(child) {
	extra->clip_serial++;
	extra->clip_dirty = true;
    } else {
	extra->child_serial++;
	extra->child_dirty = true;
    }
}

/*!
    \internal
*/
void QWidgetPrivate::dirtyClippedRegion(bool dirty_myself)
{
    if (qApp->closingDown())
	return;
    if (dirty_myself && !wasDeleted) {
	//dirty myself
	{
	    setRegionDirty(false);
	    setRegionDirty(true);
	}
	//when I get dirty so do my children
	QObjectList chldrn = q->queryList();
	for (int i = 0; i < chldrn.size(); i++) {
	    QObject *obj = chldrn.at(i);
	    if (obj->isWidgetType()) {
		QWidget *w = static_cast<QWidget *>(obj);
		if(!w->d->wasDeleted && !w->isTopLevel() && w->isVisible())
		    w->d->setRegionDirty(true);
	    }
	}
    }

    if (!q->isTopLevel()) { //short circuit, there is nothing to dirty here..
	int ox = q->x(), oy = q->y(), ow = q->width(), oh = q->height();
	for (QWidget *par = q; (par = par->parentWidget()); ) {
	    if (ox + ow < 0 || oy + oh < 0 || ox > par->width() || oy > par->height())
		return;
	    ox += par->x();
	    oy += par->y();
	    if (par->isTopLevel())
		break;
	}
    }
    //handle the rest of the widgets
    const QPoint myp(posInWindow(q));
    QRect myr(myp.x(), myp.y(), q->width(), q->height());
    QWidget *last = q, *w, *parent = q->parentWidget();
    int px = myp.x() - q->x(), py = myp.y() - q->y();
    for (QWidget *widg = parent; widg; last = widg, widg = widg->parentWidget()) {
	if (widg->d->wasDeleted) //no point in dirting
	    continue;
	myr = myr.intersect(QRect(px, py, widg->width(), widg->height()));
	widg->d->setRegionDirty(false);
	if(widg == parent)
	    widg->d->setRegionDirty(true);

	QObjectList chldrn = widg->children();
	for (int i = 0; i < chldrn.size(); i++) {
	    QObject *obj = chldrn.at(i);
	    if (obj == last)
		break;
	    if (obj->isWidgetType()) {
		w = static_cast<QWidget *>(obj);
		if(!w->d->wasDeleted && !w->isTopLevel() && w->isVisible()) {
		    QPoint wp(px + w->x(), py + w->y());
		    if(myr.intersects(QRect(wp.x(), wp.y(), w->width(), w->height()))) {
			w->d->setRegionDirty(true);
			QObjectList chldrn2 = w->queryList();
			for(int i2 = 0; i2 < chldrn2.size(); i2++) {
			    QObject *obj2 = chldrn2.at(i2);
			    if(obj2->isWidgetType()) {
				QWidget *w = static_cast<QWidget *>(obj2);
				/* this relies on something that may change in the future
				   if hd for all sub widgets != toplevel widget's hd, then
				   this function will not work any longer */
				if (!w->d->wasDeleted && w->hd == q->hd && !w->isTopLevel() && w->isVisible())
				    w->d->setRegionDirty(true);
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
bool QWidgetPrivate::isClippedRegionDirty()
{
    if(!extraData() || extraData()->clip_dirty)
	return true;
    if(!q->isTopLevel() && q->parentWidget() && q->parentWidget()->d->isClippedRegionDirty())
	return true;
    return false;
}

/*!
    \internal
*/
uint QWidgetPrivate::clippedSerial(bool do_children)
{
    createExtra();
    return do_children ? extraData()->clip_serial : extraData()->child_serial;
}

/*!
    \internal
*/
Qt::HANDLE QWidget::macCGHandle(bool do_children)
{
    //setup handle
    if(!q->cg_hd) {
	Rect port_rect;
        CGrafPtr cgraf = GetWindowPort(static_cast<WindowPtr>(q->handle()));
	GetPortBounds(cgraf, &port_rect);
	CreateCGContextForPort(cgraf, reinterpret_cast<CGContextRef *>(&q->cg_hd));
        CGContextRef cgr = static_cast<CGContextRef>(q->cg_hd);
	SyncCGContextOriginWithPort(cgr, cgraf);
	CGContextTranslateCTM(cgr, 0, (port_rect.bottom - port_rect.top));
	CGContextScaleCTM(cgr, 1, -1);
    }
    qt_mac_clip_cg_handle(static_cast<CGContextRef>(q->cg_hd),
                          d->clippedRegion(do_children),
                          QPoint(0, 0), false);
    return q->cg_hd;
}

/*!
    \internal
*/
QRegion QWidgetPrivate::clippedRegion(bool do_children)
{
    if(wasDeleted || !q->isVisible() || qApp->closingDown() || qApp->startingUp())
	return QRegion();

    createExtra();
    QWExtra *extra = extraData();
    if(q->isDesktop()) {    //the desktop doesn't participate in our clipping games
	if(!extra->clip_dirty && (!do_children || !extra->child_dirty)) {
	    if(!do_children)
		return extra->clip_sibs;
	    return extra->clip_saved;
	}
	extra->child_dirty = (extra->clip_dirty = false);
	return extra->clip_sibs = extra->clip_children = QRegion(0, 0, q->width(), q->height());
    }

    if(!extra->clip_dirty && (!do_children || !extra->child_dirty)) {
	if(!do_children)
	    return extra->clip_sibs;
	return extra->clip_saved;
    }

    bool no_children = q->children().isEmpty();
    /* If we have no children, and we are clearly off the screen we just get an automatic
       null region. This is to allow isNull() to be a cheap test of "off-screen" plus it
       prevents all the below calculations (specifically posInWindow() is pointless). */
    QPoint mp; //My position in the window (posInWindow(this))
    if(!q->isTopLevel() && no_children) { //short-circuit case
	int px = q->x(), py = q->y();
	for(QWidget *par = q->parentWidget(); par; par = par->parentWidget()) {
	    if((px + q->width() < 0) || (py + q->height() < 0) ||
	       px > par->width() || py > par->height()) {
		extra->child_dirty = (extra->clip_dirty = false);
		return extra->clip_saved = extra->clip_children = extra->clip_sibs = QRegion();
	    }
	    if(par->isTopLevel())
		break;
	    px += par->x();
	    py += par->y();
	}
	mp = QPoint(px, py);
    } else {
	mp = posInWindow(q);
    }

    /* This whole vis_width / vis_height is to prevent creating very large regions,
       as RgnHandle's just use Rect's SHRT_MAX is the maximum value, which causes strange
       problems when widgets are placed further onto a window. It should be quite unlikely
       that a top level window >SHRT_MAX in either width or height. As when these change they
       should be dirtied this optimization should mean nothing otherwise */
    int vis_width = q->width(), vis_height = q->height(), vis_x = 0, vis_y = 0;
    if(QWidget *par = (q->isTopLevel() ? 0 : q->parentWidget())) {
	int px = mp.x() - q->x(), py = mp.y() - q->y();
	int min_x = 0, min_y = 0,
	    max_x = mp.x() + vis_width, max_y = mp.y() + vis_height;
	for(; par; par = par->parentWidget()) {
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
	    if(par->isTopLevel())
		break;
	}
	vis_x = min_x;
	vis_y = min_y;
	vis_width =  max_x > mp.x() ? (max_x - mp.x()) - min_x : 0;
	vis_height = max_y > mp.y() ? (max_y - mp.y()) - min_y : 0;
    }

    //clip out my children
    QRegion mask;
    if(isClippedRegionDirty() || (do_children && extra->child_dirty)) {
	extra->child_dirty = false;
	extra->clip_children = QRegion(vis_x, vis_y, vis_width, vis_height);
	if(!no_children) {
	    QRect sr(vis_x, vis_y, vis_width, vis_height);
	    QObjectList chldrn = q->children();
	    for(int i = 0; i < chldrn.size(); i++) {
		QObject *obj = chldrn.at(i);
		if(obj->isWidgetType()) {
		    QWidget *cw = static_cast<QWidget *>(obj);
		    if (!cw->d->wasDeleted && cw->isVisible() && !cw->isTopLevel()
                        && sr.intersects(cw->geometry())) {
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

    if (isClippedRegionDirty()) {
	extra->clip_dirty = false;
	extra->clip_sibs = QRegion(mp.x()+vis_x, mp.y()+vis_y, vis_width, vis_height);
	//clip my rect with my mask
	if(extra && !extra->mask.isEmpty() && (vis_width || vis_height)) {
	    mask = extra->mask;
	    mask.translate(mp.x(), mp.y());
	    extra->clip_sibs &= mask;
	}

	//clip away my siblings
	if(!q->isTopLevel() && q->parentWidget() && (vis_width || vis_height)) {
	    QPoint tmp;
	    QObjectList siblngs = q->parentWidget()->children();
	    for(int i = siblngs.size() - 1; i >= 0; --i) {
		QObject *obj = siblngs.at(i);
		if(obj == q) //I don't care about people behind me
		    break;
		if (obj->isWidgetType()) {
		    QWidget *sw = static_cast<QWidget *>(obj);
		    if (!sw->d->wasDeleted) {
                        tmp = posInWindow(sw);
                        QRect sr(tmp.x(), tmp.y(), sw->width(), sw->height());
                        if (!sw->isTopLevel() && sw->isVisible() && extra->clip_sibs.contains(sr)) {
                            QRegion sibrgn(sr);
                            if (QWExtra *sw_extra = sw->d->extraData()) {
                                if (!sw_extra->mask.isEmpty()) {
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
	}

	/*Remove window decorations from the top level window, specifically this
	  means the GrowRgn*/
	if (q->isTopLevel()) {
	    QRegion contents;
	    RgnHandle r = qt_mac_get_rgn();
	    GetWindowRegion((WindowPtr)q->hd, kWindowContentRgn, r);
	    if(!EmptyRgn(r)) {
		contents = qt_mac_convert_mac_region(r);
		contents.translate(-q->geometry().x(), -q->geometry().y());
	    }
	    qt_mac_dispose_rgn(r);
	    extra->clip_sibs &= contents;
	} else if(q->parentWidget()) { //clip to parent
	    extra->clip_sibs &= q->parentWidget()->d->clippedRegion(false);
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


void QWidget::setWindowOpacity(double level)
{
    if(!isTopLevel())
	return;

    level = qMin(qMax(level, 0), 1.0);
    QMacSavedPortInfo::setWindowAlpha(this, level);
    d->topData()->opacity = (uchar)(level * 255);
}

double QWidget::windowOpacity() const
{
    return isTopLevel() ? ((QWidget*)this)->d->topData()->opacity / 255.0 : 0.0;
}

QPaintEngine *QWidget::engine() const
{
    if (!d->paintEngine) {
#if defined( USE_CORE_GRAPHICS )
	const_cast<QWidget *>(this)->d->paintEngine = new QCoreGraphicsPaintEngine(const_cast<QWidget *>(this));
#else
	const_cast<QWidget *>(this)->d->paintEngine = new QQuickDrawPaintEngine(const_cast<QWidget *>(this));
#endif
    }
    return d->paintEngine;
}
