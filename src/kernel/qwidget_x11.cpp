/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qwidget_x11.cpp#2 $
**
** Implementation of QWidget and QView classes for X11
**
** Author  : Haavard Nord
** Created : 931031
**
** Copyright (C) 1993,1994 by Troll Tech as.  All rights reserved.
**
*****************************************************************************/

#include "qview.h"
#include "qapp.h"
#include "qcolor.h"
#include "qwininfo.h"
#define	 GC GC_QQQ
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qwidget_x11.cpp#2 $";
#endif


// --------------------------------------------------------------------------
// QWidget member functions
//

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
    set_id( id );				// set widget id/handle
    hd = id;					// set paint device drawable
    setDevType( PDT_WIDGET );

    XSizeHints size_hints;
    if ( overlap ) {				// only top level widgets
	size_hints.flags = PPosition | PSize /*| PMinSize*/ ;
	size_hints.x = rect.left();
	size_hints.y = rect.top();
	size_hints.width = rect.width();
	size_hints.height = rect.height();
	size_hints.min_width = 0;
	size_hints.min_height = 0;
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
	protocols[0] = qXDelWinProtocol();	// support del window protocol
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
    bool v = testFlag( WEtc_MouMove );
    ulong mm;
    if ( onOff ) {
	mm = PointerMotionMask;
	setFlag( WEtc_MouMove );
    }
    else {
	mm = 0;
	clearFlag( WEtc_MouMove );
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

bool QWidget::move( int x, int y )		// move widget
{
    QRect r = ncrect;
    r.setTopLeft( QPoint(x,y) );
    setNCRect( r );
    if ( testFlag(WX11_OddWM) ) {		// strange window manager
	x = rect.left();			// use client rect pos
	y = rect.top();
    }
    XMoveWindow( dpy, ident, x, y );
    if ( !testFlag(WState_Visible) && testFlag(WType_Overlap) ) {
	XSizeHints size_hints;			// tell window manager
	size_hints.flags = PPosition;
	size_hints.x = x;
	size_hints.y = y;
	XSetNormalHints( dpy, ident, &size_hints );
    }
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
    QRect r = ncrect;
    QSize s(w,h);
    r.setSize( s );
    setNCRect( r );
    XResizeWindow( dpy, ident, rect.width(), rect.height() );
    if ( !testFlag(WState_Visible) && testFlag(WType_Overlap) ) {
	XSizeHints size_hints;			// tell window manager
	size_hints.flags = PSize;
	size_hints.width = rect.width();
	size_hints.height = rect.height();
	XSetNormalHints( dpy, ident, &size_hints );
    }
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
    QRect r( x, y, w, h );
    setNCRect( r );
    if ( testFlag(WX11_OddWM) ) {		// strange window manager
	x = rect.left();			// use client rect pos
	y = rect.top();
    }
    XMoveResizeWindow( dpy, ident, x, y, rect.width(), rect.height() );
    if ( !testFlag(WState_Visible) && testFlag(WType_Overlap) ) {
	XSizeHints size_hints;			// tell window manager
	size_hints.flags = USPosition | USSize;
	size_hints.x = x;
	size_hints.y = y;
	size_hints.width = rect.width();
	size_hints.height = rect.height();
	XSetNormalHints( dpy, ident, &size_hints );
    }
    QResizeEvent evt1( r.size() );
    SEND_EVENT( this, &evt1 );			// send resize event
    QMoveEvent evt2( r.topLeft() );
    SEND_EVENT( this, &evt2 );			// send move event
    return TRUE;
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

void QView::setCaption( const char *s )			// set caption text
{
    ctext = s;
    XStoreName( dpy, id(), (const char *)ctext );
}

void QView::setIconText( const char *s )		// set icon text
{
    itext = s;
    XSetIconName( dpy, id(), (const char *)itext );
}
