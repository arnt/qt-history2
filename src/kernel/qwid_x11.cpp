/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qwid_x11.cpp#16 $
**
** Implementation of QWidget and QView classes for X11
**
** Author  : Haavard Nord
** Created : 931031
**
** Copyright (C) 1993,1994 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qview.h"
#include "qapp.h"
#include "qpaintdc.h"
#include "qpixmap.h"
#include "qobjcoll.h"
#define	 GC GC_QQQ
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qwid_x11.cpp#16 $";
#endif


// --------------------------------------------------------------------------
// QWidget member functions
//

extern Atom q_wm_delete_window;			// defined in qapp_x11.cpp

const ulong stdWidgetEventMask =		// X event mask
	KeyPressMask | KeyReleaseMask |
	ButtonPressMask | ButtonReleaseMask |
	KeymapStateMask |
	ButtonMotionMask |
	EnterWindowMask | LeaveWindowMask |
	FocusChangeMask |
	ExposureMask |
	StructureNotifyMask |
	SubstructureRedirectMask;

bool QWidget::create()				// create widget
{
    if ( testFlag( WState_Created ) )		// already created
	return FALSE;

    setFlag( WState_Created );			// set created flag
    setDevType( PDT_WIDGET );			// set widget paint device flag

    if ( !parentWidget() )
	setFlag( WType_Overlap );		// overlapping widget

    int	   screen = qXScreen();			// X11 screen
    int	   sw = DisplayWidth( dpy, screen );	// screen width
    int	   sh = DisplayHeight( dpy, screen );	// screen height
    bool   overlap = testFlag( WType_Overlap );
    bool   popup   = testFlag( WType_Popup );
    bool   desktop = testFlag( WType_Desktop );
    Window parentwin;
    int	   border = 0;
    WId	   id;

    fg_col = black;				// set default foreground color
    bg_col = white;				// set default background color

    if ( desktop ) {				// desktop widget
	ncrect.setRect( 0, 0, sw, sh );
	overlap = popup = FALSE;		// force these flags off
    }
    else					// default non-client rect
	ncrect.setRect( sw/2 - sw/4, sh/2 - sh/5, sw/2, 2*sh/5 );
    rect = ncrect;				// default client rect

    if ( overlap || popup || desktop )		// overlapping widget
	parentwin = RootWindow( dpy, screen );
    else {					// child widget
	parentwin = parentWidget()->id();
	if ( testFlag(WStyle_Border) )		// has a border
	    border = 1;
    }

    if ( desktop ) {				// desktop widget
	id = parentwin;				// id = root window
	QWidget *otherDesktop = find( id );	// is there another desktop?
	if ( otherDesktop && otherDesktop->testFlag(WPaintDesktop) ) {
	    otherDesktop->set_id( 0 );		// remove id from widget mapper
	    set_id( id );			// make sure otherDesktop is
	    otherDesktop->set_id( id );		//   found first
	}
	else
	    set_id( id );
    }
    else {
	id = XCreateSimpleWindow( dpy, parentwin,
				  ncrect.left(), ncrect.top(),
				  ncrect.width(), ncrect.height(),
				  border,
				  fg_col.pixel(),
				  bg_col.pixel() );
	set_id( id );				// set widget id/handle + hd
    }

    if ( popup ) {				// popup widget
	XSetTransientForHint( dpy, parentwin, id );
	XSetWindowAttributes v;
	v.override_redirect = TRUE;
	v.save_under = TRUE;
	XChangeWindowAttributes( dpy, id,
				 CWOverrideRedirect | CWSaveUnder,
				 &v );
    }
    else if ( overlap ) {			// top level widget
#if 0
	XSetWindowAttributes v;
	v.bit_gravity = NorthWestGravity;	// don't erase when resizing
	XChangeWindowAttributes( dpy, id,
				 CWBitGravity,
				 &v );
#endif
	XSizeHints size_hints;
	size_hints.flags = PPosition | PSize | PWinGravity;
	size_hints.x = rect.left();
	size_hints.y = rect.top();
	size_hints.width = rect.width();
	size_hints.height = rect.height();
	size_hints.win_gravity = 1;		// NortWest
	char *title = qAppName();
	XWMHints wm_hints;			// window manager hints
	wm_hints.input = True;
	wm_hints.initial_state = NormalState;
	wm_hints.flags = InputHint | StateHint;
	XClassHint class_hint;
	class_hint.res_name = title;		// app name and widget name
	class_hint.res_class = name() ? (char *)name() : title;
	XSetWMProperties( dpy, id, 0, 0, 0, 0, &size_hints, &wm_hints,
			  &class_hint );
	XStoreName( dpy, id, title );
	Atom protocols[1];
	protocols[0] = q_wm_delete_window;	// support del window protocol
	XSetWMProtocols( dpy, id, protocols, 1 );
    }
    setMouseMoveEvents( FALSE );		// events only when button down
    gc = qXAllocGC( fnt.fontId(), bg_col.pixel(),
		    fg_col.pixel(), !testFlag(WPaintUnclipped) );
    if ( testFlag(WPaintUnclipped) )		// paint direct on device
	XSetSubwindowMode( dpy, gc, IncludeInferiors );

    if ( !desktop )
	setCursor( arrowCursor );		// default cursor
    return TRUE;
}

bool QWidget::destroy()				// destroy widget
{
    if ( this == activeWidget )
	activeWidget = 0;
    if ( testFlag( WState_Created ) ) {
	clearFlag( WState_Created );
	if ( children() ) {
	    QObjectListIt it(*children());
	    register QObject *object;
	    while ( it ) {			// destroy all widget children
		object = it.current();
		if ( object->isWidgetType() )
		    ((QWidget*)object)->destroy();
		++it;
	    }
	}
	qXFreeGC( gc );				// free graphics context
	if ( !testFlag(WType_Desktop) )
	    XDestroyWindow( dpy, ident );
	set_id( 0 );
    }
    return TRUE;
}


bool QWidget::setMouseMoveEvents( bool onOff )
{
    bool v = testFlag( WGetMouseMove );
    ulong mm;
    if ( onOff ) {
	mm = PointerMotionMask;
	setFlag( WGetMouseMove );
    }
    else {
	mm = 0;
	clearFlag( WGetMouseMove );
    }
    if ( testFlag(WType_Desktop) ) {		// desktop widget?
	if ( testFlag(WPaintDesktop) )		// get desktop paint events
	    XSelectInput( dpy, ident, ExposureMask );
    }
    else
	XSelectInput( dpy, ident,		// specify events
		      mm | stdWidgetEventMask );
    return v;
}

QColor QWidget::backgroundColor() const		// get background color
{
    return bg_col;
}

QColor QWidget::foregroundColor() const		// get foreground color
{
    return fg_col;
}

void QWidget::setBackgroundColor( const QColor &c )
{						// set background color
    bg_col = c;
    gc = qXChangeGC( gc, fnt.fontId(), bg_col.pixel(), fg_col.pixel(),
		     !testFlag(WPaintUnclipped) );
    XSetWindowBackground( dpy, ident, bg_col.pixel() );
    update();
}

void QWidget::setForegroundColor( const QColor &c )
{						// set foreground color
    fg_col = c;
    gc = qXChangeGC( gc, fnt.fontId(), bg_col.pixel(), fg_col.pixel(),
		     !testFlag(WPaintUnclipped) );
    update();
}


QFont QWidget::font() const			// get font
{
    return fnt;
}

void QWidget::setFont( const QFont &f )		// set font
{

    Font fid = f.fontId();
    gc = qXChangeGC( gc, fid, bg_col.pixel(), fg_col.pixel(),
		     !testFlag(WPaintUnclipped) );
    fnt = f;
}

QCursor QWidget::cursor() const			// get cursor
{
    return curs;
}

void QWidget::setCursor( const QCursor &c )	// set cursor
{
    ((QCursor*)&c)->update();
    XDefineCursor( dpy, ident, c.cursor );
    curs = c;
}


void QWidget::grabKeyboard()
{
    if ( !testFlag(WState_KGrab) ) {
	setFlag( WState_KGrab );
	XGrabKeyboard( dpy, ident, TRUE, GrabModeSync, GrabModeSync,
		       CurrentTime );
    }
}

void QWidget::releaseKeyboard()
{
    if ( testFlag(WState_KGrab) ) {
	clearFlag( WState_KGrab );
	XUngrabKeyboard( dpy, CurrentTime );
    }
}


void QWidget::grabMouse( bool exclusive )
{
    if ( !testFlag(WState_MGrab) ) {
	setFlag( WState_MGrab );
	XGrabPointer( dpy, ident, TRUE,
		      ButtonPressMask | ButtonReleaseMask | ButtonMotionMask |
		      EnterWindowMask | LeaveWindowMask,
		      GrabModeAsync, GrabModeAsync,
		      None, None, CurrentTime );
    }
}

void QWidget::releaseMouse()
{
    if ( testFlag(WState_MGrab) ) {
	clearFlag( WState_MGrab );
	XUngrabPointer( dpy, CurrentTime );
    }
}


void QWidget::setFocus()			// set keyboard focus
{
    if ( activeWidget == this ) {		// is active widget
	if ( !testFlag(WState_FocusA) ) {
	    QEvent evt( Event_FocusOut );
	    if ( SEND_EVENT( this, &evt ) )
		setFlag( (WState_FocusA | WState_FocusP) );
	}
	return;
    }
    else if ( activeWidget ) {			// send focus-out
	activeWidget->clearFlag( WState_FocusA );
	if ( activeWidget->parent && activeWidget->parent == parent )
	    activeWidget->clearFlag( WState_FocusP );
	QEvent evt( Event_FocusOut );
	SEND_EVENT( activeWidget, &evt );
    }
    setFlag( WState_FocusA );
    activeWidget = this;
}

QWidget *QWidget::widgetInFocus()		// get focus widget
{
    return activeWidget;
}


void QWidget::update()				// update widget
{
    XClearArea( dpy, ident, 0, 0, 0, 0, TRUE );
}

void QWidget::update( int x, int y, int w, int h )
{						// update part of widget
    XClearArea( dpy, ident, x, y, w, h, TRUE );
}


void qXOpenPopup( QWidget * );			// def in qapp_x11.cpp
void qXClosePopup( QWidget * );			// def in qapp_x11.cpp

void QWidget::show()				// show widget
{
    if ( testFlag(WState_Visible) )
	return;
    if ( children() ) {
	QObjectListIt it(*children());
	register QObject *object;
	QWidget *widget;
	while ( it ) {				// show all widget children
	    object = it.current();		//   (except popups)
	    if ( object->isWidgetType() ) {
		widget = (QWidget*)object;
		if ( !widget->testFlag(WExplicitHide) )
		    widget->show();
	    }
	    ++it;
	}
    }
    XMapWindow( dpy, ident );
    setFlag( WState_Visible );
    clearFlag( WExplicitHide );
    if ( testFlag(WType_Popup) )
	qXOpenPopup( this );
}

void QWidget::hide()				// hide widget
{
    if ( !testFlag(WState_Visible) ) {		// not visible
	setFlag( WExplicitHide );
	return;
    }
    if ( testFlag(WType_Popup) )
	qXClosePopup( this );
    XUnmapWindow( dpy, ident );
    clearFlag( WState_Visible );
}

void QWidget::raise()				// raise widget
{
    XRaiseWindow( dpy, ident );
}

void QWidget::lower()				// lower widget
{
    XLowerWindow( dpy, ident );
}


void qXRequestConfig( const QWidget * );	// defined in qapp_x11.cpp

static void do_size_hints( Display *dpy, WId ident, QWExtra *x, XSizeHints *s )
{
    if ( x ) {
	if ( x->minw >= 0 && x->minh >= 0 ) {	// add minimum size hints
	    s->flags |= PMinSize;
	    s->min_width = x->minw;
	    s->min_height = x->minh;
	}
	if ( x->maxw >= 0 && x->maxw >= 0 ) {	// add maximum size hints
	    s->flags |= PMaxSize;
	    s->max_width = x->maxw;
	    s->max_height = x->maxh;
	}
	if ( x->incw >= 0 && x->inch >= 0 ) {	// add resize increment hints
	    s->flags |= PResizeInc | PBaseSize;
	    s->width_inc = x->incw;
	    s->height_inc = x->inch;
	    s->base_width = 0;
	    s->base_height = 0;
	}
    }
    s->flags |= PWinGravity;
    s->win_gravity = 1;				// NorthWest
    XSetNormalHints( dpy, ident, s );
}

void QWidget::move( int x, int y )		// move widget
{
    QPoint p(x,y);
    QRect r = ncrect;
    if ( r.topLeft() == p || testFlag(WType_Desktop) )
	return;
    qXRequestConfig( this );
    r.setTopLeft( p );
    setNCRect( r );
    if ( testFlag(WType_Overlap) ) {
	XSizeHints size_hints;			// tell window manager
	size_hints.flags = PPosition;
	size_hints.x = x;
	size_hints.y = y;
	do_size_hints( dpy, ident, extra, &size_hints );
    }
    XMoveWindow( dpy, ident, x, y );
    QMoveEvent evt( r.topLeft() );
    SEND_EVENT( this, &evt );			// send move event
}

void QWidget::resize( int w, int h )		// resize widget
{
    if ( w < 1 )				// invalid size
	w = 1;
    if ( h < 1 )
	h = 1;
    QRect r = rect;
    QSize s(w,h);
    if ( r.size() == s || testFlag(WType_Desktop) )
	return;
    qXRequestConfig( this );
    r.setSize( s );
    setRect( r );
    if ( testFlag(WType_Overlap) ) {
	XSizeHints size_hints;			// tell window manager
	size_hints.flags = PSize;
	size_hints.width = w;
	size_hints.height = h;
	do_size_hints( dpy, ident, extra, &size_hints );
    }
    XResizeWindow( dpy, ident, w, h );
    QResizeEvent evt( s );
    SEND_EVENT( this, &evt );			// send resize event
}

void QWidget::changeGeometry( int x, int y, int w, int h )
{						// move and resize widget
    if ( w < 1 )				// invalid size
	w = 1;
    if ( h < 1 )
	h = 1;
    QRect  r( x, y, w, h );
    if ( r == rect || testFlag(WType_Desktop) )
	return;
    qXRequestConfig( this );
    setRect( r );
    if ( testFlag(WType_Overlap) ) {
	XSizeHints size_hints;			// tell window manager
	size_hints.flags = USPosition | USSize;
	size_hints.x = x;
	size_hints.y = y;
	size_hints.width = w;
	size_hints.height = h;
	do_size_hints( dpy, ident, extra, &size_hints );
    }
    XMoveResizeWindow( dpy, ident, x, y, w, h );
    QResizeEvent evt1( r.size() );
    SEND_EVENT( this, &evt1 );			// send resize event
    QMoveEvent evt2( r.topLeft() );
    SEND_EVENT( this, &evt2 );			// send move event
}


void QWidget::setMinimumSize( int w, int h )	// set minimum size
{
    if ( testFlag(WType_Overlap) ) {
	createExtra();
	extra->minw = w;
	extra->minh = h;
	XSizeHints size_hints;
	size_hints.flags = 0;
	do_size_hints( dpy, ident, extra, &size_hints );
    }
}

void QWidget::setMaximumSize( int w, int h )	// set maximum size
{
    if ( testFlag(WType_Overlap) ) {
	createExtra();
	extra->maxw = w;
	extra->maxh = h;
	XSizeHints size_hints;
	size_hints.flags = 0;
	do_size_hints( dpy, ident, extra, &size_hints );
    }
}

void QWidget::setSizeIncrement( int w, int h )
{						// set size increment
    if ( testFlag(WType_Overlap) ) {
	createExtra();
	extra->incw = w;
	extra->inch = h;
	XSizeHints size_hints;
	size_hints.flags = 0;
	do_size_hints( dpy, ident, extra, &size_hints );
    }
}


void QWidget::repaint( const QRect &r, bool eraseArea )
{
    if ( !isVisible() )				// ignore if not visible
	return;
    QPaintEvent e( r );				// send fake paint event
    if ( eraseArea )
	XClearArea( dpy, ident, r.x(), r.y(), r.width(), r.height(), FALSE );
    paintEvent( &e );
}


void QWidget::erase()				// erase widget contents
{
    XClearArea( dpy, ident, 0, 0, 0, 0, FALSE );
}

void QWidget::scroll( int dx, int dy )		// scroll widget contents
{
    QSize sz = clientSize();
    int x1, y1, x2, y2, w=sz.width(), h=sz.height();
    if ( dx > 0 ) {
	x1 = 0;
	x2 = dx;
	w -= dx;
    }
    else {
	x1 = -dx;
	x2 = 0;
	w += dx;
    }
    if ( dy > 0 ) {
	y1 = 0;
	y2 = dy;
	h -= dy;
    }
    else {
	y1 = -dy;
	y2 = 0;
	h += dy;
    }
    XCopyArea( dpy, ident, ident, gc, x1, y1, w, h, x2, y2 );
    if ( children() ) {				// scroll children
	QPoint pd( dx, dy );
	QObjectListIt it(*children());
	register QObject *object;
	while ( it ) {				// move all children
	    object = it.current();
	    if ( object->isWidgetType() ) {
		QWidget *w = (QWidget *)object;
		w->move( w->clientGeometry().topLeft()+pd );
	    }
	    ++it;
	}
    }
    if ( dx ) {
	x1 = x2 == 0 ? w : 0;
	XClearArea( dpy, ident, x1, 0, sz.width()-w, sz.height(), TRUE );
    }
    if ( dy ) {
	y1 = y2 == 0 ? h : 0;
	XClearArea( dpy, ident, 0, y1, sz.width(), sz.height()-h, TRUE );
    }
}


void QWidget::drawText( int x, int y, const char *str )
{						// draw text in widget
    if ( testFlag( WState_Visible ) )
	XDrawString( dpy, ident, gc, x, y, str, strlen(str));
}


long QWidget::metric( int m ) const		// get metric information
{
    long val;
    if ( m == PDM_WIDTH || m == PDM_HEIGHT ) {
	if ( m == PDM_WIDTH )
	    val = rect.width();
	else
	    val = rect.height();
    }
    else {
	int scr = qXScreen();
	switch ( m ) {
	    case PDM_WIDTHMM:
	        val = ((long)DisplayWidthMM(dpy,scr)*rect.width())/
		      DisplayWidth(dpy,scr);
		break;
	    case PDM_HEIGHTMM:
	        val = ((long)DisplayHeightMM(dpy,scr)*rect.height())/
		      DisplayHeight(dpy,scr);
		break;
	    case PDM_NUMCOLORS:
		val = DisplayCells(dpy,scr);
		break;
	    case PDM_NUMPLANES:
		val = DisplayPlanes(dpy,scr);
		break;
	    default:
		val = 0;
#if defined(CHECK_RANGE)
		warning( "QWidget::metric: Invalid metric command" );
#endif
	}
    }
    return val;
}


// --------------------------------------------------------------------------
// QView member functions
//

void QView::setCaption( const char *s )		// set caption text
{
    ctext = s;
    XStoreName( dpy, id(), (const char *)ctext );
}

void QView::setIconText( const char *s )	// set icon text
{
    itext = s;
    XSetIconName( dpy, id(), (const char *)itext );
}

void QView::setIcon( QPixMap *pm )		// set icon pixmap
{
    if ( ipm != pm ) {
	delete ipm;
	ipm = pm;
    }
    XWMHints wm_hints;				// window manager hints
    wm_hints.input = True;
    wm_hints.icon_pixmap = ipm->handle();
    wm_hints.flags = IconPixmapHint;
    XSetWMHints( display(), id(), &wm_hints );    
}
