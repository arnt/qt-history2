/****************************************************************************
** $Id$
**
** Implementation of QWidget and QWindow classes for mac
**
** Created : 001018
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

#include "qt_mac.h"

#include "qimage.h"
#include "qapplication.h"
#include "qapplication_p.h"
#include "qpaintdevicemetrics.h"
#include "qpainter.h"
#include "qbitmap.h"
#include "qwidgetlist.h"
#include "qwidgetintdict.h"
#include "qobjectlist.h"
#include "qaccel.h"
#include "qdragobject.h"
#include "qfocusdata.h"
#include "qlayout.h"
#include "qtextcodec.h"
#include <qcursor.h>
#include <qtimer.h>
#ifdef Q_WS_MACX
#include <CGContext.h>
#include <CGImage.h>
#endif
#if !defined(QMAC_QMENUBAR_NO_NATIVE)
#  include <qmenubar.h>
#endif
#if !defined( QT_NO_OPENGL ) && defined( QMAC_OPENGL_DOUBLEBUFFER )
#  include <qgl.h>
#endif


/*****************************************************************************
  QWidget debug facilities
 *****************************************************************************/
//#define DEBUG_WINDOW_RGNS

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
void qt_event_request_updates();
void qt_event_request_updates(QWidget *w, QRegion &r);
bool qt_nograb();
RgnHandle qt_mac_get_rgn(); //qregion_mac.cpp
void qt_mac_dispose_rgn(RgnHandle r); //qregion_mac.cpp

/*****************************************************************************
  QWidget utility functions
 *****************************************************************************/

QPoint posInWindow(QWidget *w)
{
    if(w->isTopLevel())
	return QPoint(0, 0);
    int x = 0, y = 0;
    if(w->parentWidget(TRUE)) {
	QPoint p = posInWindow(w->parentWidget());
	x = p.x() + w->geometry().x();
	y = p.y() + w->geometry().y();
    }
    return QPoint(x, y);
}

static inline const Rect *mac_rect(const QRect &qr)
{
    static Rect r;
    SetRect(&r, qr.left(), qr.top(), qr.right()+1, qr.bottom()+1); //qt says be inclusive!
    return &r;
}
static inline const Rect *mac_rect(const QPoint &qp, const QSize &qs) { return mac_rect(QRect(qp, qs)); }

#ifdef DEBUG_WINDOW_RGNS
static inline void debug_wndw_rgn(const char *where, QWidget *w, const QRegion &r, bool paint=FALSE) {
    QPoint mp(posInWindow(w));
    qDebug("%s %s %s (%s) [ %d %d %d %d ]", where, paint ? "paint" : "invalid",
	   w->className(), w->name(), mp.x(), mp.y(), w->width(), w->height());
    QMemArray<QRect> rs = r.rects();
    int offx = 0, offy = 0;
    if(paint) {
	offx = mp.x();
	offy = mp.y();
    }
    for(int i = 0; i < (int)rs.size(); i++)
	qDebug("%d %d %d %d", rs[i].x()+offx, rs[i].y()+offy, rs[i].width(), rs[i].height());
}
static inline void debug_wndw_rgn(const char *where, QWidget *w, const Rect *r, bool paint=FALSE) {
    debug_wndw_rgn(where, w, QRegion(r->left, r->top, r->right - r->left, r->bottom - r->top), paint);
}
#define clean_wndw_rgn(x, y, z) debug_wndw_rgn(x, y, z, TRUE);
#define qt_dirty_wndw_rgn(x, who, where) do { debug_wndw_rgn(x, who, where); \
	qt_dirty_wndw_rgn_internal(who, where); } while(0);
#else
#define clean_wndw_rgn(w, x, y)
#define debug_wndw_rgn(w, x, y)
#define qt_dirty_wndw_rgn(x, who, where) qt_dirty_wndw_rgn_internal(who, where);
#endif
static inline void qt_dirty_wndw_rgn_internal(const QWidget *p, const Rect *r)
{
    InvalWindowRect((WindowPtr)p->handle(), r);
    qt_event_request_updates();
}
inline void qt_dirty_wndw_rgn_internal(const QWidget *p, const QRegion &r)
{
    if(!r.handle()) {
	qt_dirty_wndw_rgn_internal(p, mac_rect(r.boundingRect()));
	return;
    }
    InvalWindowRgn((WindowPtr)p->handle(), r.handle());
    qt_event_request_updates();
}

// Paint event clipping magic
extern void qt_set_paintevent_clipping( QPaintDevice* dev, const QRegion& region);
extern void qt_clear_paintevent_clipping( QPaintDevice *dev );

enum paint_children_ops {
    PC_None = 0x00,
    PC_Now = 0x01,
    PC_ForceErase = 0x02,
    PC_NoPaint = 0x04,
    PC_NoErase = 0x08
};
void qt_paint_children(QWidget * p,QRegion &r, uchar ops = PC_ForceErase)
{
    if(!p || !p->isVisible() || r.isEmpty() || qApp->closingDown() || qApp->startingUp())
	return;
    QPoint point(posInWindow(p));
    r.translate(point.x(), point.y());
    r &= p->clippedRegion(FALSE); //at least sanity check the bounds
    if(r.isEmpty())
	return;

    bool r_is_empty = FALSE; //when we get here r is definetly not empty
    if(QObjectList * childObjects=(QObjectList*)p->children()) {
	QObjectListIt it(*childObjects);
	for(it.toLast(); it.current(); --it) {
	    if( (*it)->isWidgetType() ) {
		QWidget *w = (QWidget *)(*it);
		QRect br = r.boundingRect();
		if ( !w->isTopLevel() && w->isVisible() && w->clippedRegion(FALSE).contains(br) ) {
		    QRegion wr = w->clippedRegion(FALSE) & r;
		    r -= wr;
		    wr.translate( -(point.x() + w->x()), -(point.y() + w->y()) );
		    qt_paint_children(w, wr, ops);
		    if((r_is_empty = r.isEmpty()))
			break;
		}
	    }
	}
    }

    if(!r_is_empty) {
	r.translate(-point.x(), -point.y());
	if(p->extra && p->extra->has_dirty_area)
	    p->extra->dirty_area -= r;
	bool erase = !(ops & PC_NoErase) && ((ops & PC_ForceErase) || !p->testWFlags(QWidget::WRepaintNoErase));
	if((ops & PC_NoPaint)) {
	    if(erase) 
		p->erase(r);
	} else {
	    if(ops & PC_Now) {
		clean_wndw_rgn("**paint_children",p, r);
		p->repaint(r, erase);
	    } else {
		bool painted = FALSE;
		if(!p->testWState(QWidget::WState_BlockUpdates)) {
		    painted = TRUE;
		    clean_wndw_rgn("**paint_children",p, r);
		    p->repaint(r, erase);
		} else if(erase) {
		    erase = FALSE;
		    p->erase(r);
		}
		if(!painted) {
		    QRegion pa(r);
		    pa.translate(point.x(), point.y());
		    p->update(pa.boundingRect()); //last try
		}
	    }
	}
    }
}

static QPtrList<QWidget> qt_root_win_widgets;
static WindowPtr qt_root_win = NULL;
void qt_clean_root_win() {
    for(QPtrListIterator<QWidget> it(qt_root_win_widgets); it.current(); ++it) {
	if((*it)->hd == qt_root_win) {
#if 0
	    warning( "%s:%d: %s (%s) had his handle taken away!", __FILE__, __LINE__,
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
	if((*it)->hd == old_root_win)
	    (*it)->hd = qt_root_win;
    }
    //cleanup old window
    DisposeWindow(old_root_win);
    return TRUE;
}

bool qt_window_rgn(WId id, short wcode, RgnHandle rgn, bool force = FALSE)
{
    if(wcode == kWindowOpaqueRgn) {
	EmptyRgn(rgn);
	return TRUE;
    }
    QWidget *widget = QWidget::find( (WId)id );
    switch(wcode) {
    case kWindowOpaqueRgn: 
	return TRUE;
    case kWindowStructureRgn: {
	QRegion cr;
	if(widget) {
	    if(widget->extra && !widget->extra->mask.isNull()) {
		QRegion rin;
		CopyRgn(rgn, rin.handle(TRUE));
		QRegion rpm = widget->extra->mask;
		QRect rpm_br = rpm.boundingRect();
		rin -= QRegion(widget->x() + rpm_br.x(), widget->y() + rpm_br.y(), 
			       widget->width(), widget->height());
		rpm.translate(widget->x(), widget->y());
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
	    if(widget->extra && !widget->extra->mask.isNull()) {
		QRegion cr = widget->extra->mask;
		cr.translate(widget->x(), widget->y());
		CopyRgn(cr.handle(TRUE), rgn);
	    } else if(force) {
		QRegion cr(widget->geometry());
		CopyRgn(cr.handle(TRUE), rgn);
	    }
	}
	return TRUE; }
    default: break;
    }
    return FALSE;
}

static QMAC_PASCAL OSStatus qt_window_event(EventHandlerCallRef er, EventRef event, void *)
{
    UInt32 ekind = GetEventKind(event), eclass = GetEventClass(event);
    bool handled_event = TRUE;
    switch(eclass) {
    case kEventClassWindow: {
	WindowRef wid;
	GetEventParameter(event, kEventParamDirectObject, typeWindowRef, NULL,
			  sizeof(WindowRef), NULL, &wid);
	if(ekind == kEventWindowGetRegion) {
	    CallNextEventHandler(er, event);
	    WindowRegionCode wcode;
	    GetEventParameter(event, kEventParamWindowRegionCode, typeWindowRegionCode, NULL,
			      sizeof(wcode), NULL, &wcode);
	    RgnHandle rgn;
	    GetEventParameter(event, kEventParamRgnHandle, typeQDRgnHandle, NULL,
			      sizeof(rgn), NULL, &rgn);
	    qt_window_rgn((WId)wid, wcode, rgn, FALSE);
	} else {
	    handled_event = FALSE;
	}
	break; }
    default:
	handled_event = FALSE;
	break;
    }
    if(!handled_event) //let the event go through
	return CallNextEventHandler(er, event);
    return noErr; //we eat the event
}
static EventTypeSpec window_events[] = {
    { kEventClassWindow, kEventWindowGetRegion }
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
    qAddPostRoutine( cleanup_win_eventUPP );
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
	qDebug("Shouldn't happen %s:%d %d", __FILE__, __LINE__, message);
	break;
    }
    return result;
}
#endif

QMAC_PASCAL OSStatus qt_erase(GDHandle, GrafPtr, WindowRef window, RgnHandle rgn,
			 RgnHandle, void *w)
{
    QWidget *widget = (QWidget *)w;
    if(!widget)
	widget = QWidget::find( (WId)window );

    if ( widget ) {
#ifdef Q_WS_MAC9
	/* this is the right way to do this, and it works very well on mac
	   9, however macosx is not calling this with the proper region, as
	   some of the area (usually offscreen) isn't actually processed
	   here even though it is dirty, so for now this is mac9 only */
	QRegion reg;
	CopyRgn(rgn, reg.handle(TRUE)); 
	{ //lookup the x and y, don't use qwidget because this callback can be called before its updated
	    Point px = { 0, 0 };
	    QMacSavedPortInfo si(widget);
	    LocalToGlobal(&px);
	    reg.translate(-px.h, -px.v);
	}
#else
	//Clear a nobackground widget to make it transparent
	if(widget->backgroundMode() == Qt::NoBackground) {
	    CGContextRef ctx;
	    CGRect r2 = CGRectMake(0, 0, widget->width(), widget->height());
	    CreateCGContextForPort(GetWindowPort((WindowPtr)widget->handle()), &ctx);
	    CGContextClearRect(ctx, r2);
	    CGContextFlush(ctx);
	    CGContextRelease(ctx);
	}

	//this is the solution to weird things on demo example (white areas), need
	//to examine why this happens FIXME!
	Q_UNUSED(rgn);
	QRegion reg(0, 0, widget->width(), widget->height());
#endif
	qt_paint_children(widget, reg, PC_Now | PC_ForceErase);
    }
    return 0;
}

/*****************************************************************************
  QWidget member functions
 *****************************************************************************/
void QWidget::create( WId window, bool initializeWindow, bool destroyOldWindow  )
{
    window_event = NULL;
    own_id = 0;
    HANDLE destroyw = 0;
    setWState( WState_Created );                        // set created flag

    if ( !parentWidget() || parentWidget()->isDesktop() )
	setWFlags( WType_TopLevel );            // top-level widget

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
	    int d = dsk->primaryScreen();
	    if(parentWidget() && !parentWidget()->isDesktop()) 
		d = dsk->screenNumber(parentWidget());
	    dskr = dsk->screenGeometry(d);
	}
    }
    int sw = dskr.width(), sh = dskr.height();                // screen size
    bool topLevel = testWFlags( WType_TopLevel );
    bool popup = testWFlags( WType_Popup );
    bool dialog = testWFlags( WType_Dialog );
    bool desktop = testWFlags( WType_Desktop );
    WId    id;

    if ( !window )                              // always initialize
	initializeWindow=TRUE;
    bg_col = pal.normal().background();

    if ( dialog || popup || desktop ) {          // these are top-level, too
	topLevel = TRUE;
	setWFlags( WType_TopLevel );
	if ( popup ) 
	    setWFlags(WStyle_Tool); // a popup is a tool window
    }
    if ( topLevel && parentWidget() ) {
	// if our parent has WStyle_StaysOnTop, so must we
	QWidget *ptl = parentWidget()->topLevelWidget();
	if ( ptl && ptl->testWFlags( WStyle_StaysOnTop ) )
	    setWFlags(WStyle_StaysOnTop);
    }
    if ( !testWFlags(WStyle_Customize) && !(desktop || popup))
	setWFlags( WStyle_Customize | WStyle_NormalBorder | WStyle_Title | WStyle_MinMax | WStyle_SysMenu  );

    if ( desktop ) {                            // desktop widget
	dialog = popup = FALSE;                  // force these flags off
	crect.setRect( 0, 0, sw, sh );
    } else if ( topLevel ) {                    // calc pos/size from screen
	crect.setRect( sw/4, 3*sh/10, sw/2, 4*sh/10 );
    } else {                                    // child widget
	crect.setRect( 0, 0, 100, 30 );
    }

    if ( window ) {				// override the old window
	if ( destroyOldWindow && own_id )
	    destroyw = hd;
	own_id = 1; //it has become mine!
	id = window;
	hd = (void *)id;
	setWinId(id);
    } else if ( desktop ) {			// desktop widget
	if(!qt_root_win)
	    qt_create_root_win();
	qt_root_win_widgets.append(this);
	hd = (void *)qt_root_win;
	id = (WId)hd;
	own_id = 0;
	setWinId( id );
    } else if( popup || dialog || topLevel ) {
	own_id = 1; //I created it, I own it

	Rect r;
	SetRect(&r, crect.left(), crect.top(), crect.right(), crect.bottom());
	WindowClass wclass = kSheetWindowClass;
	if(popup || testWFlags(WStyle_Tool))
	    wclass = kModalWindowClass;
	else if(testWFlags(WShowModal))
	    wclass = kMovableModalWindowClass;
	else if(dialog && parentWidget() && !parentWidget()->topLevelWidget()->isDesktop())
	    wclass = kFloatingWindowClass;
	else if(dialog)
	    wclass = kToolbarWindowClass;
	else
	    wclass = kDocumentWindowClass;

	WindowGroupRef grp = NULL;
	WindowAttributes wattr = kWindowNoAttributes;
	if( testWFlags(WStyle_Customize) ) {
	    if ( testWFlags(WStyle_NormalBorder) || testWFlags( WStyle_DialogBorder) ) {
		if(wclass == kToolbarWindowClass)
		    wclass = kFloatingWindowClass;
		if(wclass == kDocumentWindowClass || wclass == kFloatingWindowClass ) 
		    wattr |= kWindowStandardDocumentAttributes;
	    } else {
		grp = GetWindowGroupOfClass(wclass);
		if( testWFlags( WStyle_NoBorder) ) {
		    if(wclass == kDocumentWindowClass)
			wclass = kSheetWindowClass;
		    else if(wclass == kFloatingWindowClass) 
			wclass = kToolbarWindowClass;
		} else {
		    wattr |= kWindowResizableAttribute;
		}
		if( testWFlags( WStyle_Maximize ) ) 
		    wattr |= kWindowFullZoomAttribute;
		if( testWFlags( WStyle_Minimize ) ) 
		    wattr |= kWindowCollapseBoxAttribute;
		if( testWFlags( WStyle_Title ) || testWFlags( WStyle_SysMenu ) ) 
		    wattr |= kWindowCloseBoxAttribute;
	    }
	}

#if 0
	long macos_version=0;
	Gestalt(gestaltSystemVersion, &macos_version);
	qDebug("%ld", macos_version);
#endif
	//wattr |= kWindowLiveResizeAttribute;
	if(popup || testWFlags(WStyle_Tool) || 
	   (!testWFlags(WShowModal) && dialog && parentWidget() && !parentWidget()->topLevelWidget()->isDesktop()))
	    wattr |= kWindowNoActivatesAttribute;
#ifdef QMAC_USE_WDEF
	if( (wclass == kPlainWindowClass && wattr == kWindowNoAttributes) || testWFlags(WStyle_Tool) ) {
	    WindowDefSpec wds;
	    wds.defType = kWindowDefProcPtr;
	    wds.u.defProc = NewWindowDefUPP(qt_wdef);
	    CreateCustomWindow(&wds, wclass, wattr, &r, (WindowRef *)&id);
	} else 
#endif
	{
	    if(CreateNewWindow(wclass, wattr, &r, (WindowRef *)&id))
		qDebug("Shouldn't happen %s:%d", __FILE__, __LINE__);
	    if(!desktop) { 	//setup an event callback handler on the window
		InstallWindowEventHandler((WindowRef)id, make_win_eventUPP(), 
					  GetEventTypeCount(window_events),
					  window_events, (void *)qApp, &window_event);
	    }
	}

	if(wclass == kFloatingWindowClass) //these dialogs don't hide
	    ChangeWindowAttributes((WindowRef)id, 0, kWindowHideOnSuspendAttribute);
#ifdef Q_WS_MACX
	if(testWFlags(WStyle_StaysOnTop)) {
	    createTLExtra();
	    if(extra->topextra->group)
		ReleaseWindowGroup(extra->topextra->group);
	    CreateWindowGroup(kWindowActivationScopeNone, &extra->topextra->group);
	    SetWindowGroupLevel(extra->topextra->group, kCGMaximumWindowLevel);
	    SetWindowGroupParent(extra->topextra->group, GetWindowGroupOfClass(kAllWindowClasses));
	    SetWindowGroup((WindowPtr)id, extra->topextra->group);
	} else if(grp) {
	    SetWindowGroup((WindowPtr)id, grp);
	}
#endif
	InstallWindowContentPaintProc((WindowPtr)id, NewWindowPaintUPP(qt_erase), 0, this);
	if(testWFlags( WType_Popup )) 
	    SetWindowModality((WindowPtr)id, kWindowModalityNone, NULL);
	fstrut_dirty = TRUE; // when we create a toplevel widget, the frame strut should be dirty
	if(!mac_window_count++)
	    QMacSavedPortInfo::setPaintDevice(this);
	hd = (void *)id;
	setWinId(id);
	ReshapeCustomWindow((WindowPtr)hd);
    } else {
	while(QWidget::find(++serial_id));
	setWinId(serial_id);
	id = serial_id;
	hd = topLevelWidget()->hd;
	fstrut_dirty = FALSE; // non-toplevel widgets don't have a frame, so no need to update the strut
	setWinId(id);
    }

    bg_col = pal.normal().background();
    setWState( WState_MouseTracking );
    setMouseTracking( FALSE );                  // also sets event mask
    if(desktop) { //immediatly "show" a "desktop"
	setWState(WState_Visible);
    } else {
	clearWState(WState_Visible);
	dirtyClippedRegion(TRUE);
    }
    macDropEnabled = false;

    if ( destroyw ) {
	mac_window_count--;
	DisposeWindow((WindowPtr)destroyw);
    }
}

void qt_mac_destroy_widget(QWidget *w); //qapplication_mac.cpp

void QWidget::destroy( bool destroyWindow, bool destroySubWindows )
{
    deactivateWidgetCleanup();
    if(isDesktop() && hd == qt_root_win)
	qt_root_win_widgets.removeRef(this);

    if(isVisible() && !isTopLevel()) {
	dirtyClippedRegion(TRUE);
	qt_dirty_wndw_rgn("destroy",this, mac_rect(posInWindow(this), geometry().size()));
    }

    if ( testWState(WState_Created) ) {
	dirtyClippedRegion(TRUE);
        clearWState( WState_Created );
        if ( children() ) {
            QObjectListIt it(*children());
            register QObject *obj;
            while ( (obj=it.current()) ) {      // destroy all widget children
                ++it;
                if ( obj->isWidgetType() )
                    ((QWidget*)obj)->destroy(destroySubWindows, destroySubWindows);
            }
        }
	if ( mac_mouse_grabber == this )
	    releaseMouse();
	if ( mac_keyboard_grabber == this )
	    releaseKeyboard();
	if ( acceptDrops() )
	    setAcceptDrops(FALSE);

        if ( testWFlags(WShowModal) )          // just be sure we leave modal
            qt_leave_modal( this );
        else if ( testWFlags(WType_Popup) )
            qApp->closePopup( this );
	if ( destroyWindow && isTopLevel() && hd && own_id) {
	    mac_window_count--;
	    if(window_event) {
		RemoveEventHandler( window_event );
		window_event = NULL;
	    }
	    DisposeWindow( (WindowPtr)hd );
	}
    }
    hd=0;
    setWinId( 0 );
    qt_mac_destroy_widget(this);
}

void QWidget::reparentSys( QWidget *parent, WFlags f, const QPoint &p,
			bool showIt )
{
    dirtyClippedRegion(TRUE);

    QCursor oldcurs;
    QPoint oldposinwindow(posInWindow(this));
    bool setcurs=testWState(WState_OwnCursor);
    if ( setcurs ) {
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
    reparentFocusWidgets( parent );		// fix focus chains

    setWinId( 0 );
    if ( parentObj ) {				// remove from parent
	QObject *oldp = parentObj;
	parentObj->removeChild( this );
	if(isVisible() && !isTopLevel() && oldp->isWidgetType())
	    qt_dirty_wndw_rgn("reparent1",this, mac_rect(oldposinwindow, geometry().size()));
    }

    if ( parent ) {				// insert into new parent
	parentObj = parent;			// avoid insertChild warning
	parent->insertChild( this );
    }
    bool     dropable = acceptDrops();
    bool     enable = isEnabled();
    bool     owned = own_id;
    FocusPolicy fp = focusPolicy();
    QSize    s	    = size();
    QString capt= caption();
    widget_flags = f;
    clearWState( WState_Created | WState_Visible | WState_ForceHide );
    if ( isTopLevel() || (!parent || parent->isVisibleTo( 0 ) ) )
	setWState( WState_ForceHide );	// new widgets do not show up in already visible parents
    if(dropable)
	setAcceptDrops(FALSE);

    //get new hd, now move
    create();
    no_move_blt = TRUE;
    if ( p.isNull() )
	resize( s );
    else
	setGeometry( p.x(), p.y(), s.width(), s.height() );
    no_move_blt = FALSE;

    //reset flags and show (if neccesary)
    setEnabled( enable );
    setFocusPolicy( fp );
    setAcceptDrops(dropable);
    if ( !capt.isNull() ) {
	extra->topextra->caption = QString::null;
	setCaption( capt );
    }
    if ( showIt )
	show();
    if ( setcurs )
	setCursor(oldcurs);

    //reparent children
    if(QObjectList	*chldn = queryList()) {
	QObjectListIt it( *chldn );
	for ( QObject *obj; (obj=it.current()); ++it ) {
	    if(obj->inherits("QAccel"))
		((QAccel*)obj)->repairEventFilter();
	    if(obj->isWidgetType()) {
		QWidget *w = (QWidget *)obj;
		if(((WindowPtr)w->hd) == old_hd)
		    w->hd = hd; //all my children hd's are now mine!
#if !defined( QT_NO_OPENGL ) && defined( QMAC_OPENGL_DOUBLEBUFFER )
		if(w->inherits("QGLWidget"))
		    ((QGLWidget *)w)->fixReparented();
#endif
	    }
	}
	delete chldn;
    }
#if !defined(QMAC_QMENUBAR_NO_NATIVE)  //make sure menubars are fixed
    if(QObjectList *menus = queryList("QMenuBar")) {
	QObjectListIt menuit( *menus );
	for ( QMenuBar *mb; (mb=(QMenuBar *)menuit.current()); ++menuit ) {
	    int was_eaten = mb->mac_eaten_menubar;
	    mb->macRemoveNativeMenubar();
	    mb->macCreateNativeMenubar();
	    if(was_eaten)
		mb->menuContentsChanged();
	}
	delete menus;
    }
#endif

    reparentFocusWidgets( oldtlw );

    //repaint the new area, on the window parent
    if(isVisible()) //finally paint my new area
	qt_dirty_wndw_rgn("reparent2",this, mac_rect(posInWindow(this), geometry().size()));

    //send the reparent event
    if ( old_hd && owned ) { //don't need old window anymore
	mac_window_count--;
	if(old_window_event) {
	    RemoveEventHandler( old_window_event );
	    old_window_event = NULL;
	}
	DisposeWindow( old_hd );
    }
}

QPoint QWidget::mapToGlobal( const QPoint &pos ) const
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

QPoint QWidget::mapFromGlobal( const QPoint &pos ) const
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

void QWidget::setMicroFocusHint(int, int, int, int, bool, QFont * )
{
}

void QWidget::setFontSys( QFont * )
{
}

void QWidget::setBackgroundColorDirect( const QColor &color )
{
    bg_col = color;

    if ( extra && extra->bg_pix ) {		// kill the background pixmap
	delete extra->bg_pix;
	extra->bg_pix = 0;
    }

    if(isTopLevel()) {
#ifdef QMAC_NO_QUARTZ
	QMacSavedPortInfo savedInfo(this);
	RGBColor f;
	f.red = bg_col.red() * 256;
	f.green = bg_col.green() * 256;;
	f.blue = bg_col.blue() * 256;
	RGBBackColor(&f);
#else //!QMAC_NO_QUARTZ
	//FIXME
#endif
    }
}

static int allow_null_pixmaps = 0;
void QWidget::setBackgroundPixmapDirect( const QPixmap &pixmap )
{
    QPixmap old;
    if ( extra && extra->bg_pix )
	old = *extra->bg_pix;
    if ( !allow_null_pixmaps && pixmap.isNull() ) {
	if ( extra && extra->bg_pix ) {
	    delete extra->bg_pix;
	    extra->bg_pix = 0;
	}
    } else {
	if (!pixmap.isNull()) {
	    QPixmap pm = pixmap;
	    if ( pm.depth() == 1 && QPixmap::defaultDepth() > 1 ) {
		pm = QPixmap( pixmap.size() );
		bitBlt( &pm, 0, 0, &pixmap, 0, 0, pm.width(), pm.height() );
	    }
	}
	if ( extra && extra->bg_pix )
	    delete extra->bg_pix;
	else
	    createExtra();
	extra->bg_pix = new QPixmap( pixmap );
    }
}

void QWidget::setBackgroundEmpty()
{
    allow_null_pixmaps++;
    setErasePixmap(QPixmap());
    allow_null_pixmaps--;
    if ( isTopLevel() )
	ReshapeCustomWindow((WindowPtr)hd);
}

void QWidget::setCursor( const QCursor &cursor )
{
    if ( cursor.handle() != arrowCursor.handle() || (extra && extra->curs) ) {
	createExtra();
	delete extra->curs;
	extra->curs = new QCursor(cursor);
    }
    setWState( WState_OwnCursor );
}

void QWidget::unsetCursor()
{
    if ( !isTopLevel() ) {
	if (extra ) {
	    delete extra->curs;
	    extra->curs = 0;
	}
	clearWState( WState_OwnCursor );
    }
}

void QWidget::setCaption( const QString &cap )
{
    if ( extra && extra->topextra && extra->topextra->caption == cap )
	return; // for less flicker
    createTLExtra();
    extra->topextra->caption = cap;
    if(isTopLevel()) {
	CFStringRef str = CFStringCreateWithCharacters(NULL, (UniChar *)cap.unicode(), cap.length());
	SetWindowTitleWithCFString((WindowPtr)hd, str);
    }
    QEvent e( QEvent::CaptionChange );
    QApplication::sendEvent( this, &e );
}

void QWidget::setIcon( const QPixmap &pixmap )
{
    if ( extra && extra->topextra ) {
	delete extra->topextra->icon;
	extra->topextra->icon = 0;
    } else {
	createTLExtra();
    }
    if ( !pixmap.isNull() )
	extra->topextra->icon = new QPixmap( pixmap );
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

void QWidget::setIconText( const QString &iconText )
{
    createTLExtra();
    extra->topextra->iconText = iconText;
}

void QWidget::grabMouse()
{
    if ( isVisible() && !qt_nograb() ) {
	if ( mac_mouse_grabber )
	    mac_mouse_grabber->releaseMouse();
	mac_mouse_grabber=this;
    }
}

void QWidget::grabMouse( const QCursor & )
{
    if ( isVisible() && !qt_nograb() ) {
	if ( mac_mouse_grabber )
	    mac_mouse_grabber->releaseMouse();
	mac_mouse_grabber=this;
    }
}

void QWidget::releaseMouse()
{
    if ( !qt_nograb() && mac_mouse_grabber == this )
	mac_mouse_grabber = NULL;	
}

void QWidget::grabKeyboard()
{
    if ( !qt_nograb() ) {
	if ( mac_keyboard_grabber )
	    mac_keyboard_grabber->releaseKeyboard();
	mac_keyboard_grabber = this;
    }
}

void QWidget::releaseKeyboard()
{
    if ( !qt_nograb() && mac_keyboard_grabber == this )
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
    if(!isVisible() || !isTopLevel() || isPopup() || isDesktop() || testWFlags( WStyle_Tool ))
	return;
    if(IsWindowActive((WindowPtr)hd))
	ActivateWindow((WindowPtr)hd, true);
    else
	SelectWindow((WindowPtr)hd);
}

void QWidget::update()
{
    update( 0, 0, width(), height() );
}

void QWidget::update( int x, int y, int w, int h )
{
    if ( !testWState(WState_BlockUpdates) && testWState( WState_Visible ) && isVisible() ) {
	if ( w < 0 )
	    w = crect.width()  - x;
	if ( h < 0 )
	    h = crect.height() - y;
	if ( w && h ) {
#if 0
	    QRegion r(x, y, w, h);
	    qt_event_request_updates(this, r);
	    debug_wndw_rgn("update", this, r);
#else
	    QPoint p(posInWindow(this));
	    qt_dirty_wndw_rgn("update", this, mac_rect(QRect(p.x() + x, p.y() + y, w, h)));
#endif
	}
    }
}

void QWidget::repaint( int x, int y, int w, int h, bool erase )
{
    if ( w < 0 )
	w = crect.width()  - x;
    if ( h < 0 )
	h = crect.height() - y;
    QRect r(x,y,w,h);
    if ( r.isEmpty() )
	return; // nothing to do
    repaint(QRegion(r), erase); //general function..
}

void QWidget::repaint( const QRegion &reg , bool erase )
{
    if ( !testWState(WState_BlockUpdates) && isVisible() ) {
	setWState( WState_InPaintEvent );
	qt_set_paintevent_clipping( this, reg );
	if ( erase )
	    this->erase(reg);

	QPaintEvent e( reg );
	QApplication::sendEvent( this, &e );
	qt_clear_paintevent_clipping( this );
	clearWState( WState_InPaintEvent );
    }
}

void QWidget::showWindow()
{
    if(isDesktop()) //desktop is always visible
	return;

    dirtyClippedRegion(TRUE);
    if ( isTopLevel() ) {
#if defined( Q_WS_MACX ) && 0 //handle transition
	if(qApp->style().inherits("QAquaStyle") && parentWidget() && testWFlags(WShowModal)) 
	    TransitionWindowAndParent((WindowPtr)hd, (WindowPtr)parentWidget()->hd,
				      kWindowSheetTransitionEffect,
				      kWindowShowTransitionAction, NULL);
#endif
	//now actually show it
	ShowHide((WindowPtr)hd, 1);
	setActiveWindow();
    } else {
	qt_dirty_wndw_rgn("show",this, mac_rect(posInWindow(this), geometry().size()));
    }
}

void QWidget::hideWindow()
{
    if(isDesktop()) //you can't hide the desktop!
	return;

    dirtyClippedRegion(TRUE);
    if ( isTopLevel() ) {
	ShowHide((WindowPtr)hd, 0);

	if(isActiveWindow()) {
	    QWidget *w = NULL;
	    if(parentWidget())
		w = parentWidget()->topLevelWidget();
	    if(!w || !w->isVisible()) {
		for(WindowPtr wp = GetFrontWindowOfClass(kDocumentWindowClass, true); 
		    wp; wp = GetNextWindowOfClass(wp, kDocumentWindowClass, true)) {
		    if((w = QWidget::find( (WId)wp )))
			break;
		}
	    }
	    if(w && w->isVisible()) 
		w->setActiveWindow();
	}
    } else if(isVisible()) {
	qt_dirty_wndw_rgn("hide",this, mac_rect(posInWindow(this), geometry().size()));
    }
    deactivateWidgetCleanup();
}



bool QWidget::isMinimized() const
{
    // true for non-toplevels that have the minimized flag, e.g. MDI children
    return IsWindowCollapsed((WindowRef)hd) || ( !isTopLevel() && testWState( WState_Minimized ) );
}

bool QWidget::isMaximized() const
{
    return testWState(WState_Maximized);
}


void QWidget::showMinimized()
{
    if(isMinimized())
	return;
    if ( isTopLevel() ) {
	if ( isVisible() ) {
	    CollapseWindow((WindowPtr)hd, TRUE);
        } else {
	    topData()->showMode = 1;
	    show();
	    clearWState( WState_Visible );
	    sendHideEventsToChildren(TRUE);
	}
    }
    QEvent e( QEvent::ShowMinimized );
    QApplication::sendEvent( this, &e );
    clearWState( WState_Maximized );
    setWState( WState_Minimized );
}

void QWidget::showMaximized()
{
    if(isMaximized() || isDesktop())
	return;
    if ( testWFlags(WType_TopLevel) ) {
	Rect bounds;
	QDesktopWidget *dsk = QApplication::desktop();
	GetAvailableWindowPositioningBounds(dsk->handle(dsk->screenNumber(this)), &bounds);
	if(QTLWExtra *tlextra = topData()) {
	    if ( tlextra->normalGeometry.width() < 0 )
		tlextra->normalGeometry = geometry();
	    if(fstrut_dirty)
		updateFrameStrut();
	    bounds.left += tlextra->fleft;
	    bounds.top += tlextra->ftop;
	    bounds.right -= tlextra->fright;
	    bounds.bottom -= tlextra->fbottom;
	}
	SetWindowStandardState((WindowPtr)hd, &bounds);
	ZoomWindow( (WindowPtr)hd, inZoomOut, FALSE);
	qt_dirty_wndw_rgn("showMaxim",this, mac_rect(rect()));

	QRect orect(geometry().x(), geometry().y(), width(), height());
	crect.setRect( bounds.left, bounds.top, bounds.right - bounds.left, 
		       bounds.bottom - bounds.top );

	if(isVisible()) {
	    dirtyClippedRegion(TRUE);

	    //issue a resize
	    QResizeEvent qre( size(), orect.size());
	    QApplication::sendEvent( this, &qre );
	    //issue a move
	    QMoveEvent qme( pos(), orect.topLeft());
	    QApplication::sendEvent( this, &qme );
	}
    }
    show();
    QEvent e( QEvent::ShowMaximized );
    QApplication::sendEvent( this, &e );
    clearWState( WState_Minimized );
    setWState( WState_Maximized );
}

void QWidget::showNormal()
{
    if(isDesktop()) //desktop is always visible
	return;

    if ( isTopLevel() ) {
	if ( topData()->fullscreen ) {
	    reparent( 0, WType_TopLevel, QPoint(0,0) );
	    topData()->fullscreen = 0;
	}
	{
	    Rect bounds;
	    ZoomWindow( (WindowPtr)hd, inZoomIn, FALSE);
	    GetPortBounds( GetWindowPort( (WindowPtr)hd ), &bounds );
	    qt_dirty_wndw_rgn("showNormal",this, &bounds);

	    QRect orect(x(), y(), width(), height());
	    if(isVisible()) {
		dirtyClippedRegion(TRUE);

		//issue a resize
		QResizeEvent qre( size(), orect.size());
		QApplication::sendEvent( this, &qre );
		//issue a move
		QMoveEvent qme( pos(), orect.topLeft());
		QApplication::sendEvent( this, &qme );
	    }
	}
    }
    dirtyClippedRegion(TRUE);
    show();
    QEvent e( QEvent::ShowNormal );
    QApplication::sendEvent( this, &e );
    clearWState( WState_Minimized | WState_Maximized );
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
	if ( p->childObjects && p->childObjects->findRef(this) >= 0 )
	    p->childObjects->append( p->childObjects->take() );
	if(isVisible()) {
	    dirtyClippedRegion(TRUE);
	    clp ^= clippedRegion(FALSE);
	    qt_dirty_wndw_rgn("raise",this, clp);
	}
    }
}

void QWidget::lower()
{
    if(!isDesktop())
	return;

    if ( isTopLevel() )
	SendBehind((WindowPtr)handle(), NULL);
    else if(QWidget *p = parentWidget(TRUE)) {
	QRegion clp;
	if(isVisible())
	    clp = clippedRegion(FALSE);
	if ( p->childObjects && p->childObjects->findRef(this) >= 0 )
	    p->childObjects->insert( 0, p->childObjects->take() );
	if(isVisible()) {
	    dirtyClippedRegion(TRUE);
	    clp ^= clippedRegion(FALSE);
	    qt_dirty_wndw_rgn("lower",this, clp);
	}
    }
}


void QWidget::stackUnder( QWidget *w )
{
    if ( !w || isTopLevel() || isDesktop() )
	return;

    QWidget *p = parentWidget();
    if(!p || p != w->parentWidget())
	return;
    int loc = p->childObjects->findRef(w);
    QRegion clp;
    if(isVisible())
	clp = clippedRegion(FALSE);
    if ( loc >= 0 && p->childObjects && p->childObjects->findRef(this) >= 0 )
	p->childObjects->insert( loc, p->childObjects->take() );
    if(isVisible()) {
	dirtyClippedRegion(TRUE);
	clp ^= clippedRegion(FALSE);
	qt_dirty_wndw_rgn("stackUnder",this, clp);
    }
}


void QWidget::internalSetGeometry( int x, int y, int w, int h, bool isMove )
{
    if ( isDesktop() )
	return;
    if ( extra ) {				// any size restrictions?
	w = QMIN(w,extra->maxw);
	h = QMIN(h,extra->maxh);
	w = QMAX(w,extra->minw);
	h = QMAX(h,extra->minh);

	// Deal with size increment
	if ( extra->topextra ) {
	    if ( extra->topextra->incw ) {
		w = w/extra->topextra->incw;
		w *= extra->topextra->incw;
	    }
	    if ( extra->topextra->inch ) {
		h = h/extra->topextra->inch;
		h *= extra->topextra->inch;
	    }
	}
    }
    if ( w < 1 )                                // invalid size
	w = 1;
    if ( h < 1 )
	h = 1;

    QPoint oldp = pos();
    QSize  olds = size();
    QRegion oldregion = clippedRegion(FALSE);
    QRect  r( x, y, w, h );
    dirtyClippedRegion(FALSE);
    crect = r;
    if (!isTopLevel() && size() == olds && oldp == pos() )
	return;
    dirtyClippedRegion(TRUE);

    bool isResize = (olds != size());
    if(isTopLevel() && winid && own_id) {
	if(isResize && isMove && isVisible()) {
	    Rect r;
	    SetRect(&r, x, y, x + w, y + h);
	    SetWindowBounds((WindowPtr)hd, kWindowContentRgn, &r);
	} else {
	    if(isResize) 
		SizeWindow((WindowPtr)hd, w, h, 1);
	    if(isMove)
		MoveWindow((WindowPtr)hd, x, y, 1);
	}
    }

    if(isMove || isResize) {
	if(!isVisible()) {
	    if ( isResize )
		QApplication::postEvent( this, new QResizeEvent( size(), olds ) );
	    if ( isMove && oldp != pos() )
		QApplication::postEvent( this, new QMoveEvent( pos(), oldp ) );
	} else {
	    QRegion bltregion, clpreg = clippedRegion(FALSE);
	    if( !oldregion.isNull() ) {
		//setup the old clipped region..
		bltregion = oldregion;
		if(isMove && !isTopLevel())
		    bltregion.translate(pos().x() - oldp.x(), pos().y() - oldp.y());
		bltregion &= clpreg;
		{   //can't blt that which is dirty
		    RgnHandle r = qt_mac_get_rgn();
		    GetWindowRegion((WindowPtr)hd, kWindowUpdateRgn, r);
		    if(!EmptyRgn(r)) {
			QRegion dirty; //the dirty region
			CopyRgn(r, dirty.handle(TRUE));
			dirty.translate(-topLevelWidget()->geometry().x(), 
					-topLevelWidget()->geometry().y());
			if(isMove && !isTopLevel()) //need to be in new coords
			    dirty.translate(pos().x() - oldp.x(), pos().y() - oldp.y());
			bltregion -= dirty;
		    }
		    qt_mac_dispose_rgn(r);
		}

		if(isMove && !no_move_blt && !isTopLevel()) {
		    QWidget *parent = parentWidget() ? parentWidget() : this;
		    QPoint tp(posInWindow(parent));
		    int px = tp.x(), py = tp.y();
#ifdef QMAC_NO_QUARTZ
		    //save the window state, and do the grunt work
		    int ow = olds.width(), oh = olds.height();
		    QMacSavedPortInfo saveportstate(this);
		    ::RGBColor f;
		    f.red = f.green = f.blue = 0;
		    RGBForeColor( &f );
		    f.red = f.green = f.blue = ~0;
		    RGBBackColor( &f );
		    PenMode(patCopy);

		    //calculate new and old rectangles
		    int nx = px + pos().x(), ny = py + pos().y();  //new
		    Rect newr; SetRect(&newr,nx, ny, nx + ow, ny + oh);
		    int ox = px + oldp.x(), oy = py + oldp.y(); //old
		    Rect oldr; SetRect(&oldr, ox, oy, ox+ow, oy+oh);
		    SetClip(bltregion.handle(TRUE));
		    //actually copy some pixels now
		    GrafPtr wport = GetWindowPort((WindowPtr)handle());
		    LockPortBits(wport);
		    BitMap *scrn = (BitMap *)*GetPortPixMap(wport);
		    CopyBits(scrn, scrn, &oldr, &newr, srcCopy, NULL);
		    UnlockPortBits(wport);
#else //!QMAC_NO_QUARTZ
		    //FIXME
#endif
		}
	    }
	    if(isResize || !isTopLevel() || !QDIsPortBuffered(GetWindowPort((WindowPtr)hd))) {
		//finally issue "expose" event
		QRegion upd((oldregion + clpreg) - bltregion);
		if(isResize && !testWFlags(WStaticContents))
		    upd += clippedRegion();
		qt_dirty_wndw_rgn("internalSetGeometry",this, upd);
		//and force the update
		if(isResize || 1)
		    qt_event_request_updates();
	    }
	    //Do these last, as they may cause an event which paints, and messes up
	    //what we blt above
	    if ( isResize ) { //send the resize event..
		QResizeEvent e( size(), olds );
		QApplication::sendEvent( this, &e );
	    }
	    if ( isMove && pos() != oldp ) { //send the move event..
		QMoveEvent e( pos(), oldp );
		QApplication::sendEvent( this, &e );
	    }
	}
    }
}

void QWidget::setMinimumSize( int minw, int minh)
{
#if defined(QT_CHECK_RANGE)
    if ( minw < 0 || minh < 0 )
	qWarning("QWidget::setMinimumSize: The smallest allowed size is (0,0)");
#endif
    createExtra();
    if ( extra->minw == minw && extra->minh == minh )
	return;
    extra->minw = minw;
    extra->minh = minh;
    if ( minw > width() || minh > height() ) {
	bool resized = testWState( WState_Resized );
	resize( QMAX(minw,width()), QMAX(minh,height()) );
	if ( !resized )
	    clearWState( WState_Resized ); //not a user resize
    }
    updateGeometry();
}

void QWidget::setMaximumSize( int maxw, int maxh)
{
#if defined(QT_CHECK_RANGE)
    if ( maxw > QWIDGETSIZE_MAX || maxh > QWIDGETSIZE_MAX ) {
	qWarning("QWidget::setMaximumSize: (%s/%s) "
		"The largest allowed size is (%d,%d)",
		 name( "unnamed" ), className(), QWIDGETSIZE_MAX,
		QWIDGETSIZE_MAX );
	maxw = QMIN( maxw, QWIDGETSIZE_MAX );
	maxh = QMIN( maxh, QWIDGETSIZE_MAX );
    }
    if ( maxw < 0 || maxh < 0 ) {
	qWarning("QWidget::setMaximumSize: (%s/%s) Negative sizes (%d,%d) "
		"are not possible",
		name( "unnamed" ), className(), maxw, maxh );
	maxw = QMAX( maxw, 0 );
	maxh = QMAX( maxh, 0 );
    }
#endif
    createExtra();
    if ( extra->maxw == maxw && extra->maxh == maxh )
	return;
    extra->maxw = maxw;
    extra->maxh = maxh;
    if ( maxw < width() || maxh < height() ) {
	bool resized = testWState( WState_Resized );
	resize( QMIN(maxw,width()), QMIN(maxh,height()) );
	if ( !resized )
	    clearWState( WState_Resized ); //not a user resize
    }
    updateGeometry();
}


void QWidget::setSizeIncrement( int w, int h )
{
    createTLExtra();
    extra->topextra->incw = w;
    extra->topextra->inch = h;
}

void QWidget::setBaseSize( int w, int h )
{
    createTLExtra();
    extra->topextra->basew = w;
    extra->topextra->baseh = h;
}

void QWidget::erase( int x, int y, int w, int h )
{
    erase( QRegion( x, y, w, h ) );
}

void QWidget::erase( const QRegion& reg )
{
    if ( backgroundMode() == NoBackground || isDesktop() )
	return;
    QRect rr(reg.boundingRect());

    int xoff = 0;
    int yoff = 0;
    if ( !isTopLevel() ) {
	if( backgroundOrigin() == QWidget::ParentOrigin ) {
	    xoff = x();
	    yoff = y();
	} else if(backgroundOrigin() == QWidget::WindowOrigin ) {
	    QWidget *topl = this;
	    while(!topl->isTopLevel() && !topl->testWFlags(WSubWindow))
		topl = topl->parentWidget(TRUE);
	    QPoint p = mapTo( topl, QPoint(0,0) );
	    xoff = p.x();
	    yoff = p.y();
	}
    }

    bool unclipped = testWFlags( WPaintUnclipped );
    clearWFlags( WPaintUnclipped );
    QPainter p( this );
    if ( unclipped )
	setWFlags( WPaintUnclipped );

    p.setClipRegion(reg);
    if ( extra && extra->bg_pix ) {
	if ( !extra->bg_pix->isNull() ) {
	    QPoint point(rr.x()+(xoff%extra->bg_pix->width()), rr.y()+(yoff%extra->bg_pix->height()));
	    p.drawTiledPixmap(rr,*extra->bg_pix, point);
	}
    } else {
	p.fillRect(rr, bg_col);
    }
    p.end();
}


void QWidget::scroll( int dx, int dy)
{
    scroll( dx, dy, QRect() );
}

void QWidget::scroll( int dx, int dy, const QRect& r )
{
    if ( testWState( WState_BlockUpdates ) )
	return;

    bool valid_rect = r.isValid();
    QRect sr = valid_rect?r:rect();
    int x1, y1, x2, y2, w=sr.width(), h=sr.height();
    if ( dx > 0 ) {
	x1 = sr.x();
	x2 = x1+dx;
	w -= dx;
    } else {
	x2 = sr.x();
	x1 = x2-dx;
	w += dx;
    }
    if ( dy > 0 ) {
	y1 = sr.y();
	y2 = y1+dy;
	h -= dy;
    } else {
	y2 = sr.y();
	y1 = y2-dy;
	h += dy;
    }

    if ( dx == 0 && dy == 0 )
	return;
    if ( w > 0 && h > 0 )
	bitBlt(this,x2,y2,this,x1,y1,w,h);

    if ( !valid_rect && children() ) {	// scroll children
	QPoint pd( dx, dy );
	QObjectListIt it(*children());
	register QObject *object;
	while ( it ) {				// move all children
	    object = it.current();
	    if ( object->isWidgetType() ) {
		QWidget *w = (QWidget *)object;
		w->move( w->pos() + pd );
	    }
	    ++it;
	}
    }

    QPoint p(posInWindow(this));
    QRegion update_rgn;
    GetWindowRegion((WindowPtr)hd, kWindowUpdateRgn, update_rgn.handle(TRUE));
    update_rgn.translate(-topLevelWidget()->geometry().x(), -topLevelWidget()->geometry().y());
    QRegion copied(clippedRegion() - update_rgn);
    copied.translate( -p.x(), -p.y() );
    copied &= QRegion(sr);
    copied.translate(dx,dy);
    repaint( QRegion(sr) - copied, !testWFlags(WRepaintNoErase) );
    if(QDIsPortBuffered(GetWindowPort((WindowPtr)hd)))
	QMacSavedPortInfo::flush(this);
}

void QWidget::drawText( int x, int y, const QString &str )
{
    if ( testWState(WState_Visible) ) {
	QPainter paint;
	paint.begin( this );
	paint.drawText( x, y, str );
	paint.end();
    }
}

int QWidget::metric( int m ) const
{
    switch(m) {
    case QPaintDeviceMetrics::PdmHeightMM: // 75 dpi is 3dpmm
	return (metric(QPaintDeviceMetrics::PdmHeight)*100)/288;
    case QPaintDeviceMetrics::PdmWidthMM: // 75 dpi is 3dpmm
	return (metric(QPaintDeviceMetrics::PdmWidth)*100)/288;
    case QPaintDeviceMetrics::PdmWidth:
    {
	if ( !isTopLevel() )
	    return crect.width();
	Rect windowBounds;
	GetPortBounds( GetWindowPort( ((WindowPtr)hd) ), &windowBounds );
	return windowBounds.right;
    }

    case QPaintDeviceMetrics::PdmHeight:
    {
	if ( !isTopLevel() )
	    return crect.height();
	Rect windowBounds;
	GetPortBounds( GetWindowPort( ((WindowPtr)hd) ), &windowBounds );
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
	qWarning("QWidget::metric unhandled parameter %d",m);
	return QPaintDevice::metric(m);// XXX
    }
    return 0;
}

void QWidget::createSysExtra()
{
    extra->has_dirty_area = FALSE;
    extra->child_dirty = extra->clip_dirty = TRUE;
    extra->macDndExtra = 0;
}

void QWidget::deleteSysExtra()
{
}

void QWidget::createTLSysExtra()
{
    extra->topextra->group = NULL;
}

void QWidget::deleteTLSysExtra()
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

    that->fstrut_dirty = FALSE;
    QTLWExtra *top = that->topData();
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

void qt_macdnd_unregister( QWidget *widget, QWExtra *extra ); //dnd_mac
void qt_macdnd_register( QWidget *widget, QWExtra *extra ); //dnd_mac

void QWidget::setAcceptDrops( bool on )
{
    if ( (on && macDropEnabled) || (!on && !macDropEnabled) )
	return;
    topLevelWidget()->createExtra();
    if ( (macDropEnabled = on) )
	qt_macdnd_register( topLevelWidget(),  topLevelWidget()->extraData());
    else
	qt_macdnd_unregister( topLevelWidget(), topLevelWidget()->extraData() );
}

void QWidget::setMask( const QRegion &region )
{
    createExtra();
    if ( region.isNull() && extra->mask.isNull() )
	return;

    QRegion clp;
    if(isVisible())
	clp = clippedRegion(FALSE);
    extra->mask = region;
    if(isVisible()) {
	dirtyClippedRegion(TRUE);
	clp ^= clippedRegion(FALSE);
	qt_dirty_wndw_rgn("setMask",this, clp);
    }
    if ( isTopLevel() ) {
	/* We do this because the X/Y seems to move to the first paintable point
	   (ie the bounding rect of the mask). We must offset everything or else
	   we have big problems. */
	QRect r = region.boundingRect();
	QMacSavedPortInfo mp(this);
	SetOrigin(r.x(), r.y());
	//now let the wdef take it
	ReshapeCustomWindow((WindowPtr)hd);
    }
}

void QWidget::setMask( const QBitmap &bitmap )
{
    setMask( QRegion( bitmap ) );
}


void QWidget::clearMask()
{
    setMask( QRegion() );
}

void QWidget::setName( const char *name )
{
    QObject::setName( name );
}

void QWidget::propagateUpdates()
{
#ifdef QMAC_NO_QUARTZ
    QMacSavedPortInfo savedInfo(this);
#endif
    QRegion rgn;
    GetWindowRegion((WindowPtr)hd, kWindowUpdateRgn, rgn.handle(TRUE));
    if(!rgn.isEmpty()) {
	rgn.translate(-topLevelWidget()->geometry().x(),
			    -topLevelWidget()->geometry().y());
	debug_wndw_rgn("*****propagatUpdates", topLevelWidget(), rgn );
	BeginUpdate((WindowPtr)hd);
	qt_paint_children( this, rgn );
	EndUpdate((WindowPtr)hd);
    }
}

void QWidget::dirtyClippedRegion(bool dirty_myself)
{
    if(dirty_myself) {
	//dirty myself
	if(extra)
	    extra->child_dirty = extra->clip_dirty = TRUE;
	//when I get dirty so do my children
	if(QObjectList *chldn = queryList()) {
	    QObjectListIt it(*chldn);
	    for(QObject *obj; (obj = it.current()); ++it ) {
		if(obj->isWidgetType()) {
		    QWidget *w = (QWidget *)(*it);
		    if(w->topLevelWidget() == topLevelWidget() &&
		       !w->isTopLevel() && w->isVisible() && w->extra)
			w->extra->clip_dirty = TRUE;
		}
	    }
	}
    }

    //handle the rest of the widgets
    QPoint myp(posInWindow(this));
    QRect myr(myp.x(), myp.y(), width(), height());
    QWidget *last = this, *w;
    for(QWidget *widg = parentWidget(); widg; last = widg, widg = widg->parentWidget()) {
	QPoint widgp(posInWindow(widg));
	myr = myr.intersect(QRect(widgp.x(), widgp.y(), widg->width(), widg->height()));
	if(widg->extra)
	    widg->extra->child_dirty = TRUE;

	if(const QObjectList *chldn = widg->children()) {
	    for(QObjectListIt it(*chldn); it.current() && it.current() != last; ++it) {
		if((*it)->isWidgetType()) {
		    w = (QWidget *)(*it);
		    if(w->topLevelWidget() == topLevelWidget() && !w->isTopLevel() && w->isVisible()) {
			QPoint wp(posInWindow(w));
			if(myr.intersects(QRect(wp.x(), wp.y(), w->width(), w->height()))) {
			    if(w->extra)
				w->extra->clip_dirty = TRUE;
			    if(QObjectList *chldn2 = w->queryList()) {
				QObjectListIt it2(*chldn2);
				for(QObject *obj; (obj = it2.current()); ++it2 ) {
				    if(obj->isWidgetType()) {
					QWidget *w = (QWidget *)(*it2);
					if(w->topLevelWidget() == topLevelWidget() &&
					   !w->isTopLevel() && w->isVisible() && w->extra)
					    w->extra->clip_dirty = TRUE;
				    }
				}
				delete chldn2;
			    }
			}
		    }
		}
	    }
	}
    }
}

bool QWidget::isClippedRegionDirty()
{
    if(!extra || extra->clip_dirty)
	return TRUE;
    if(/*!isTopLevel() && */(parentWidget(TRUE) && parentWidget(TRUE)->isClippedRegionDirty()))
	return TRUE;
    return FALSE;
}

#ifndef QMAC_NO_QUARTZ
CGContextRef QWidget::macCGClippedContext(bool do_children) const
{
    CGContextRef ctx = QPaintDevice::macCGContext();
    if(!extra)
	createExtra();
    if(extra->clip_dirty || (do_children && extra->child_dirty) ||
       (do_children && !extra->ctx_children_clipped)) {
	extra->ctx_children_clipped = do_children;
	QRegion reg = clippedRegion(do_children);
	Rect r;
	ValidWindowRect((WindowPtr)hd, &r);
	ClipCGContextToRegion(ctx, &r, reg.handle(TRUE));
    }
    return ctx;
}
#endif

QRegion QWidget::clippedRegion(bool do_children)
{
    //the desktop doesn't participate in our clipping games
    if(isDesktop()) {
	createExtra();
	if(!extra->clip_dirty && (!do_children || !extra->child_dirty)) {
	    if(!do_children)
		return extra->clip_sibs;
	    return extra->clip_saved;
	}
	extra->child_dirty = (extra->clip_dirty = FALSE);
	return extra->clip_sibs = extra->clip_children = QRegion(0, 0, width(), height());
    }

    if(!isVisible() ||  (qApp->closingDown() || qApp->startingUp()))
	return QRegion();
    createExtra();

    if(!extra->clip_dirty && (!do_children || !extra->child_dirty)) {
	if(!do_children)
	    return extra->clip_sibs;
	return extra->clip_saved;
    }

    QRegion mask;
    //clip out my children
    if(do_children && extra->child_dirty) {
	extra->child_dirty = FALSE;
	extra->clip_children = QRegion(0, 0, width(), height());
	if(const QObjectList *chldnlst=children()) {
	    for(QObjectListIt it(*chldnlst); it.current(); ++it) {
		if((*it)->isWidgetType()) {
		    QWidget *cw = (QWidget *)(*it);
		    if( cw->isVisible() && !cw->isTopLevel() ) {
			QRegion childrgn(cw->x(), cw->y(), cw->width(), cw->height());
			if(cw->extra && !cw->extra->mask.isNull()) {
			    mask = cw->extra->mask;
			    mask.translate(cw->x(), cw->y());
			    childrgn &= mask;
			}
			extra->clip_children -= childrgn;
		    }
		}
	    }
	}
    }

    if(isClippedRegionDirty()) {
	extra->clip_dirty = FALSE;
	QPoint tmp = posInWindow(this);
	extra->clip_sibs = QRegion(tmp.x(), tmp.y(), width(), height());
	//clip my rect with my mask
	if(extra && !extra->mask.isNull()) {
	    mask = extra->mask;
	    mask.translate(tmp.x(), tmp.y());
	    extra->clip_sibs &= mask;
	}

	//clip away my siblings
	if(!isTopLevel() && parentWidget()) {
	    if(const QObjectList *siblst = parentWidget()->children()) {
		//loop to this because its in zorder, and i don't care about people behind me
		QObjectListIt it(*siblst);
		for(it.toLast(); it.current() && it.current() != this; --it) {
		    if((*it)->isWidgetType()) {
			QWidget *sw = (QWidget *)(*it);
			tmp = posInWindow(sw);
			QRect sr(tmp.x(), tmp.y(), sw->width(), sw->height());
			if(!sw->isTopLevel() && sw->isVisible() && extra->clip_sibs.contains(sr)) {
			    QRegion sibrgn(sr);
			    if(sw->extra && !sw->extra->mask.isNull()) {
				mask = sw->extra->mask;
				mask.translate(tmp.x(), tmp.y());
				sibrgn &= mask;
			    }
			    extra->clip_sibs -= sibrgn;
			}
		    }
		}
	    }
	}

	if(isTopLevel()) {
	    QRegion contents;
	    RgnHandle r = qt_mac_get_rgn();
	    GetWindowRegion((WindowPtr)hd, kWindowContentRgn, r);
	    if(!EmptyRgn(r)) {
		CopyRgn(r, contents.handle(TRUE));
		contents.translate(-geometry().x(), -geometry().y());
	    }
	    qt_mac_dispose_rgn(r);
	    extra->clip_sibs &= contents;
	}
	else if(parentWidget()) {
	    extra->clip_sibs &= parentWidget()->clippedRegion(FALSE);
	}
    }

    //translate my stuff and my children now
    QRegion chldrgns = extra->clip_children;
    QPoint mp = posInWindow(this);
    chldrgns.translate(mp.x(), mp.y());
    extra->clip_saved = extra->clip_sibs & chldrgns;

    if(do_children)
	return extra->clip_saved;
    return extra->clip_sibs;
}

void QWidget::resetInputContext()
{

}
