/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qwid_x11.cpp#7 $
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
#include "qobjcoll.h"
#include "qapp.h"
#include "qcolor.h"
#include "qpixmap.h"
#include "qwininfo.h"
#define	 GC GC_QQQ
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qwid_x11.cpp#7 $";
#endif


// --------------------------------------------------------------------------
// QWidget member functions
//

extern Atom q_wm_delete_window;			// defined in qapp_x11.cpp

bool QWidget::create()				// create widget
{
    if ( testFlag( WState_Created ) )		// already created
	return FALSE;

    if ( !parentWidget() )
	setFlag( WType_Overlap );		// overlapping widget

    int	   screen = qXScreen();			// X11 screen
    QSize  dsz = QWinInfo::displaySize();	// size of display
    bool   overlap = testFlag( WType_Overlap );
    Window parentwin;
    int	   border;
    WId	   id;

    ncrect.setRect( dsz.width()/2-dsz.width()/4,// default non-client rect
		    dsz.height()/2 - dsz.height()/5,
		    dsz.width()/2, 2*dsz.height()/5 );
    rect = ncrect;				// default client rect

    if ( overlap ) {				// overlapping widget
	parentwin = RootWindow(dpy,screen);
	border = 0;
    }
    else {					// child widget
	parentwin = parentWidget()->id();
	border = testFlag(WStyle_Border) ? 1 : 0;
    }

    id = XCreateSimpleWindow( dpy, parentwin,
			      ncrect.left(), ncrect.top(),
			      ncrect.width(), ncrect.height(),
			      border,
			      BlackPixel(dpy,screen),
			      WhitePixel(dpy,screen) );
    set_id( id );				// set widget id/handle + hd
    setDevType( PDT_WIDGET );

    XSizeHints size_hints;
    if ( overlap ) {				// only top level widgets
	size_hints.flags = PPosition | PSize | PWinGravity;
	size_hints.x = rect.left();
	size_hints.y = rect.top();
	size_hints.width = rect.width();
	size_hints.height = rect.height();
	size_hints.win_gravity = 1;		// NortWest
    }
    char *title = qAppName();
    XSetStandardProperties( dpy, id, title, title, 0, 0, 0,
			    overlap ? &size_hints : 0 );
    if ( overlap ) {				// only top level widgets
	XWMHints wm_hints;			// window manager hints
	wm_hints.input = True;
	wm_hints.initial_state = NormalState;
	wm_hints.flags = InputHint | StateHint;
	XSetWMHints( dpy, id, &wm_hints );
	Atom protocols[1];
	protocols[0] = q_wm_delete_window;	// support del window protocol
	XSetWMProtocols( dpy, id, protocols, 1 );
    }
    setMouseMoveEvents( FALSE );
    gc = XCreateGC( dpy, id, 0, 0 );		// create graphics context
    XSetFont( dpy, gc, fnt.fontId() );
    XSetBackground( dpy, gc, WhitePixel(dpy,screen) );
    XSetForeground( dpy, gc, BlackPixel(dpy,screen) );
    setCursor( arrowCursor );			// default cursor
    setFlag( WState_Created );
    return TRUE;
}

bool QWidget::destroy()				// destroy widget
{
    if ( testFlag( WState_Created ) ) {
	clearFlag( WState_Created );
	XFreeGC( dpy, gc );			// free graphics context
	XDestroyWindow( dpy, ident );
	set_id( 0 );
    }
    return TRUE;
}


bool QWidget::setMouseMoveEvents( bool onOff )
{
    bool v = testFlag( WEtc_MouseMove );
    ulong mm;
    if ( onOff ) {
	mm = PointerMotionMask;
	setFlag( WEtc_MouseMove );
    }
    else {
	mm = 0;
	clearFlag( WEtc_MouseMove );
    }
    XSelectInput( dpy, ident,			// specify events
		  mm|
		  KeyPressMask|KeyReleaseMask|
		  ButtonPressMask|ButtonReleaseMask|
		  KeymapStateMask|
		  ButtonMotionMask|
		  EnterWindowMask|LeaveWindowMask|
		  FocusChangeMask|
		  ExposureMask|
		  StructureNotifyMask|
		  SubstructureRedirectMask
		);
    return v;
}

QColor QWidget::backgroundColor() const		// get background color
{
    if ( bg_col.isValid() )
	return bg_col;
    return white;				// white is default background
}

QColor QWidget::foregroundColor() const		// get foreground color
{
    if ( fg_col.isValid() )
	return fg_col;
    return black;				// black is default foreground
}

void QWidget::setBackgroundColor( const QColor &c )
{						// set background color
    bg_col = c;
    XSetBackground( dpy, gc, c.pixel() );
    XSetWindowBackground( dpy, ident, c.pixel() );
    update();
}

void QWidget::setForegroundColor( const QColor &c )
{						// set foreground color
    fg_col = c;
    XSetForeground( dpy, gc, c.pixel() );
    update();
}


QFont QWidget::font() const			// get font
{
    return fnt;
}

void QWidget::setFont( const QFont &f )		// set font
{

    Font fid = f.fontId();
    if ( fid )
	XSetFont( dpy, gc, fid );
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


bool QWidget::update()				// update widget
{
    XClearArea( dpy, ident, 0, 0, 0, 0, TRUE );
    return TRUE;
}

bool QWidget::show()				// show widget
{
    if ( testFlag( WState_Visible ) )
	return FALSE;
    if ( children() ) {
	QObjectListIt it(*children());
	while ( it ) {				// show all widget children
	    QObject *object = it.current();
	    if ( object->isWidgetType() )
		((QWidget*)object)->show();
	    ++it;
	}	
    }
    XMapWindow( dpy, ident );
    setFlag( WState_Visible );
    return TRUE;
}

bool QWidget::hide()				// hide widget
{
    if ( !testFlag( WState_Visible ) )
	return FALSE;
    XUnmapWindow( dpy, ident );
    clearFlag( WState_Visible );
    return TRUE;
}

bool QWidget::raise()				// raise widget
{
    XRaiseWindow( dpy, ident );
    return TRUE;
}

bool QWidget::lower()				// lower widget
{
    XLowerWindow( dpy, ident );
    return TRUE;
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

bool QWidget::move( int x, int y )		// move widget
{
    QPoint p(x,y);
    QRect r = ncrect;
    if ( r.topLeft() == p )			// same position
	return FALSE;
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
    return TRUE;
}

bool QWidget::resize( int w, int h )		// resize widget
{
    if ( w < 1 )				// invalid size
	w = 1;
    if ( h < 1 )
	h = 1;
    QRect r = rect;
    QSize s(w,h);
    if ( r.size() == s )			// same size
	return FALSE;
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
    return TRUE;
}

bool QWidget::changeGeometry( int x, int y, int w, int h )
{						// move and resize widget
    if ( w < 1 )				// invalid size
	w = 1;
    if ( h < 1 )
	h = 1;
    QRect  r( x, y, w, h );
    if ( r == ncrect )
	return FALSE;
    qXRequestConfig( this );
    setNCRect( r );				// ZE BIGK WESTJON!!!!
    if ( testFlag(WType_Overlap) ) {
	XSizeHints size_hints;			// tell window manager
	size_hints.flags = USPosition | USSize;
	size_hints.x = x;
	size_hints.y = y;
	size_hints.width = rect.width();
	size_hints.height = rect.height();
	do_size_hints( dpy, ident, extra, &size_hints );
    }
    XMoveResizeWindow( dpy, ident, x, y, rect.width(), rect.height() );
    QResizeEvent evt1( r.size() );
    SEND_EVENT( this, &evt1 );			// send resize event
    QMoveEvent evt2( r.topLeft() );
    SEND_EVENT( this, &evt2 );			// send move event
    return TRUE;
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


bool QWidget::erase()				// erase widget contents
{
    XClearArea( dpy, ident, 0, 0, 0, 0, FALSE );
    return TRUE;
}

bool QWidget::scroll( int dx, int dy )		// scroll widget contents
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
    if ( dx ) {
	x1 = x2 == 0 ? w : 0;
	XClearArea( dpy, ident, x1, 0, sz.width()-w, sz.height(), FALSE );
    }
    if ( dy ) {
	y1 = y2 == 0 ? h : 0;
	XClearArea( dpy, ident, 0, y1, sz.width(), sz.height()-h, FALSE );
    }
    return TRUE;
}


bool QWidget::drawText( int x, int y, const char *str )
{						// draw text in widget
    if ( testFlag( WState_Visible ) )
	XDrawString( dpy, ident, gc, x, y, str, strlen(str));
    return TRUE;
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
