/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qwidget_x11.cpp#77 $
**
** Implementation of QWidget and QWindow classes for X11
**
** Author  : Haavard Nord
** Created : 931031
**
** Copyright (C) 1993-1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qwindow.h"
#include "qpalette.h"
#include "qapp.h"
#include "qpaintdc.h"
#include "qpainter.h"
#include "qpixmap.h"
#include "qwidcoll.h"
#include "qobjcoll.h"
#define	 GC GC_QQQ
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qwidget_x11.cpp#77 $";
#endif


void qt_enter_modal( QWidget * );		// defined in qapp_x11.cpp
void qt_leave_modal( QWidget * );		// --- "" ---
bool qt_modal_state();				// --- "" ---
void qt_open_popup( QWidget * );		// --- "" ---
void qt_close_popup( QWidget * );		// --- "" ---
void qt_updated_rootinfo();


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
	StructureNotifyMask | SubstructureRedirectMask;


/*!
\internal
Creates the widget window.
Usually called from the QWidget constructor.
*/

bool QWidget::create()				// create widget
{
    if ( testWFlags(WState_Created) )		// already created
	return FALSE;
    setWFlags( WState_Created );		// set created flag

    if ( !parentWidget() )
	setWFlags( WType_Overlap );		// overlapping widget

    int	   screen = qt_xscreen();		// X11 screen
    int	   sw = DisplayWidth( dpy, screen );	// screen width
    int	   sh = DisplayHeight( dpy, screen );	// screen height
    bool   overlap = testWFlags( WType_Overlap );
    bool   popup   = testWFlags( WType_Popup );
    bool   modal   = testWFlags( WType_Modal );
    bool   desktop = testWFlags( WType_Desktop );
    Window parentwin;
    WId	   id;

    bg_col = pal.normal().background();		// default background color

    if ( modal ) {				// modal windows overlap
	overlap = TRUE;
	setWFlags( WType_Overlap );
    }

    if ( desktop ) {				// desktop widget
	frect.setRect( 0, 0, sw, sh );
	overlap = popup = FALSE;		// force these flags off
    }
    else if ( overlap || popup )		// parentless widget
	frect.setRect( sw/2 - sw/4, sh/2 - sh/5, sw/2, 2*sh/5 );
    else					// child widget
	frect.setRect( 10, 10, 100, 30 );
    crect = frect;				// default client rect

    if ( overlap || popup || desktop )		// overlapping widget
	parentwin = RootWindow( dpy, screen );
    else					// child widget
	parentwin = parentWidget()->id();

    if ( desktop ) {				// desktop widget
	id = parentwin;				// id = root window
	QWidget *otherDesktop = find( id );	// is there another desktop?
	if ( otherDesktop && otherDesktop->testWFlags(WPaintDesktop) ) {
	    otherDesktop->set_id( 0 );		// remove id from widget mapper
	    set_id( id );			// make sure otherDesktop is
	    otherDesktop->set_id( id );		//   found first
	}
	else
	    set_id( id );
    }
    else {
	id = XCreateSimpleWindow( dpy, parentwin,
				  frect.left(), frect.top(),
				  frect.width(), frect.height(),
				  0,
				  black.pixel(),
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
	if ( modal ) {
	    if ( parentWidget() )		// modal to one widget
		XSetTransientForHint( dpy, id, parentWidget()->id() );
	    else				// application-modal
		XSetTransientForHint( dpy, id, qt_xrootwin() );
	}
	XSizeHints size_hints;
	size_hints.flags = PPosition | PSize | PWinGravity;
	size_hints.x = crect.left();
	size_hints.y = crect.top();
	size_hints.width = crect.width();
	size_hints.height = crect.height();
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
    if ( testWFlags(WResizeNoErase) ) {
	XSetWindowAttributes v;
	v.bit_gravity = NorthWestGravity;	// don't erase when resizing
	XChangeWindowAttributes( dpy, id,
				 CWBitGravity,
				 &v );
    }
    setMouseTracking( FALSE );			// also sets event mask
    if ( overlap ) {				// set X cursor
	QCursor *appc = QApplication::cursor();
	XDefineCursor( dpy, ident, appc ? appc->handle() : curs.handle() );
	setWFlags( WCursorSet );
    }
    return TRUE;
}


/*!
\internal
Destroys the widget window and frees up window system resources.
Usually called from the QWidget destructor.
*/

bool QWidget::destroy()				// destroy widget
{
    if ( qApp->focus_widget == this )
	qApp->focus_widget = 0;			// reset focus widget
    if ( parentWidget() && parentWidget()->focusChild == this )
	parentWidget()->focusChild = 0;
    if ( testWFlags(WState_Created) ) {
	clearWFlags( WState_Created );
	focusChild = 0;
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
	if ( testWFlags(WType_Modal) )		// just be sure we leave modal
	    qt_leave_modal( this );
	else if ( testWFlags(WType_Popup) )
	    qt_close_popup( this );
	if ( !testWFlags(WType_Desktop) )
	    XDestroyWindow( dpy, ident );
	set_id( 0 );
	emit destroyed();			// send out destroyed signal
    }
    return TRUE;
}


/*!
This function is provided in case a widget should feel \e really
bad, regret that it was even born.

It gives the widget a fresh start, new \e parent, new widget flags
(\e f but as usual, use 0) at a new position in its new parent (\e p).

If \e showIt is TRUE, show() is called once the widget has been
recreated.
*/

void QWidget::recreate( QWidget *parent, WFlags f, const QPoint &p,
			bool showIt )
{
    extern void qPRCreate( const QWidget *, Window );
    WId old_ident = ident;
    if ( testWFlags(WType_Desktop) )
	old_ident = 0;
    set_id( 0 );
    if ( parentObj )				// remove from parent
	parentObj->removeChild( this );
    if ( (parentObj = parent) )
	parentObj->insertChild( this );
    bool was_disabled = isDisabled();
    QSize s = size();				// save size
    QColor bgc = bg_col;			// save colors
    flags = f;
    clearWFlags( WState_Created | WState_Visible );
    create();
    const QObjectList *chlist = children();
    if ( chlist ) {				// reparent children
	QObjectListIt it( *chlist );
	QObject *obj;
	while ( (obj=it.current()) ) {
	    if ( obj->isWidgetType() ) {
		QWidget *w = (QWidget *)obj;
		XReparentWindow( dpy, w->id(), id(), w->geometry().x(),
				 w->geometry().y() );
	    }
	    ++it;
	}
    }
    qPRCreate( this, old_ident );
    setBackgroundColor( bgc );			// restore colors
    setGeometry( p.x(), p.y(), s.width(), s.height() );
    if ( was_disabled )
	disable();
    if ( showIt )
	show();
    if ( old_ident )
	XDestroyWindow( dpy, old_ident );
}


bool QWidget::setMouseTracking( bool enable )
{
    bool v = testWFlags( WMouseTracking );
    ulong m;
    if ( enable ) {
	m = PointerMotionMask;
	setWFlags( WMouseTracking );
    }
    else {
	m = 0;
	clearWFlags( WMouseTracking );
    }
    if ( testWFlags(WType_Desktop) ) {		// desktop widget?
	if ( testWFlags(WPaintDesktop) )	// get desktop paint events
	    XSelectInput( dpy, ident, ExposureMask );
    }
    else
	XSelectInput( dpy, ident,		// specify events
		      m | stdWidgetEventMask );
    return v;
}

/*!
  Sets the background color of this widget.

  The background color is independent of the widget color group.<br>
  Notice that the background color will be overwritten when setting
  a new palette.
  \sa backgroundColor(), setPalette(), setBackgroundPixmap()
*/

void QWidget::setBackgroundColor( const QColor &c )
{						// set background color
    bg_col = c;
    XSetWindowBackground( dpy, ident, bg_col.pixel() );
    update();
}

/*!
  Sets the background pixmap of the widget to \e pm.

  The background pixmap is tiled.
  \sa setBackgroundColor()
*/

void QWidget::setBackgroundPixmap( const QPixmap &pm )
{
    if ( !pm.isNull() ) {
	XSetWindowBackgroundPixmap( dpy, ident, pm.handle() );
	if ( testWFlags(WType_Desktop) )	// save rootinfo later
	    qt_updated_rootinfo();
	update();
    }
}


/*!
  Sets the widget cursor shape to \e cursor.

  The mouse cursor will assume this shape when it's over this widget.
  See a list of cursor shapes in the QCursor documentation.

  An editor widget would for example use an I-beam cursor:
  \code
    setCursor( ibeamCursor );
  \endcode

  \sa cursor()
*/

void QWidget::setCursor( const QCursor &cursor )
{
    curs = cursor;
    QCursor *appc = QApplication::cursor();
    XDefineCursor( dpy, ident, appc ? appc->handle() : curs.handle() );
    setWFlags( WCursorSet );
    XFlush( dpy );
}


extern bool qt_nograb();

/*!
  Grabs the mouse input.

  This widget will be the only one to receive mouse events until
  releaseMouse() is called.

  \warning Grabbing the mouse might lock the terminal.

  It is almost never necessary to grab the mouse when using Qt since Qt
  grabs and releases it sensibly.	 In particular, Qt grabs the mouse
  when a button is pressed and keeps it until the last button is
  released.

  \sa releaseMouse(), grabKeyboard(), releaseKeyboard()
*/

void QWidget::grabMouse()
{
    if ( !testWFlags(WState_MGrab) ) {
	setWFlags( WState_MGrab );
	if ( !qt_nograb() )
	    XGrabPointer( dpy, ident, TRUE,
			  ButtonPressMask | ButtonReleaseMask |
			  ButtonMotionMask | EnterWindowMask | LeaveWindowMask,
			  GrabModeAsync, GrabModeAsync,
			  None, None, CurrentTime );
    }
}

/*!
  Grabs the mouse intput and changes the cursor shape.

  The cursor will assume shape \e cursor (for as long as the mouse focus is
  grabbed) and this widget will be the only one to receive mouse events
  until releaseMouse() is called().

  \warning Grabbing the mouse might lock the terminal.

  \sa releaseMouse(), grabKeyboard(), releaseKeyboard(), setCursor()
*/

void QWidget::grabMouse( const QCursor &cursor )
{
    if ( !testWFlags(WState_MGrab) ) {
	setWFlags( WState_MGrab );
	if ( !qt_nograb() )
	    XGrabPointer( dpy, ident, TRUE,
			  ButtonPressMask | ButtonReleaseMask |
			  ButtonMotionMask |
			  EnterWindowMask | LeaveWindowMask,
			  GrabModeAsync, GrabModeAsync,
			  None, cursor.handle(), CurrentTime );
    }
}

/*!
  Releases the mouse grab.

  \sa grabMouse(), grabKeyboard(), releaseKeyboard()
*/

void QWidget::releaseMouse()
{
    if ( testWFlags(WState_MGrab) ) {
	clearWFlags( WState_MGrab );
	if ( !qt_nograb() ) {
	    XUngrabPointer( dpy, CurrentTime );
	    XFlush( dpy );
	}
    }
}

/*!
  Grabs all keyboard input.

  This widget will receive all keyboard events, independent of the active
  window.

  \warning Grabbing the keyboard might lock the terminal.

  \sa releaseKeyboard(), grabMouse(), releaseMouse()
*/

void QWidget::grabKeyboard()
{
    if ( !testWFlags(WState_KGrab) ) {
	setWFlags( WState_KGrab );
	if ( !qt_nograb() )
	    XGrabKeyboard( dpy, ident, TRUE, GrabModeSync, GrabModeSync,
			   CurrentTime );
    }
}

/*!
  Releases the keyboard grab.

  \sa grabKeyboard(), grabMouse(), releaseMouse()
*/

void QWidget::releaseKeyboard()
{
    if ( testWFlags(WState_KGrab) ) {
	clearWFlags( WState_KGrab );
	if ( !qt_nograb() )
	    XUngrabKeyboard( dpy, CurrentTime );
    }
}


/*!
  Gives the keyboard input focus to the widget.
*/

void QWidget::setFocus()			// set keyboard input focus
{
    QWidget *oldFocus = qApp->focusWidget();
    if ( this == oldFocus )			// has already focus
	return;
    if ( !acceptFocus() )			// cannot take focus
	return;
    if ( oldFocus ) {				// goodbye to old focus widget
	qApp->focus_widget = 0;
	QFocusEvent out( Event_FocusOut );
	QApplication::sendEvent( oldFocus, &out );
    }
    QWidget *top, *w, *p;
    top = this;
    while ( top->parentWidget() )		// find top level widget
	top = top->parentWidget();
    w = top;
    while ( w->focusChild )			// reset focus chain
	w = w->focusChild;
    w = w->parentWidget();
    while ( w ) {
	w->focusChild = 0;
	w = w->parentWidget();
    }
    w = this;
    while ( (p=w->parentWidget()) ) {		// build new focus chain
	p->focusChild = w;
	w = p;
    }
    qApp->focus_widget = this;
    QFocusEvent in( Event_FocusIn );
    QApplication::sendEvent( this, &in );
}

/*!
  \internal Handles TAB.
*/

bool QWidget::focusNextChild()
{
    QWidget *p = parentWidget();
    if ( !p )
	return FALSE;
    QObjectList *c = (QObjectList *)p->children();
    if ( c->findRef(this) < 0 )			// why, not found?
	return FALSE;
    while ( TRUE ) {
	c->next();
	if ( !c->current() )
	    c->first();
	if ( c->current()->isWidgetType() ) {
	    QWidget *w = (QWidget*)c->current();
	    if ( w->acceptFocus() ) {
		w->setFocus();
		return TRUE;
	    }
	}
    }
    return TRUE;
}

/*!
  \internal Handles Shift+TAB.
*/

bool QWidget::focusPrevChild()
{
    return TRUE;
}


/*!
  Enables widget updates if \e enable is TRUE, or  disables widget updates
  if \e enable is FALSE.

  If updates are disabled, the widget will not receive repaint events.
  \sa update(), repaint(), paintEvent()
*/

bool QWidget::enableUpdates( bool enable )	// enable widget update/repaint
{
    bool last = !testWFlags( WNoUpdates );
    if ( enable )
	clearWFlags( WNoUpdates );
    else
	setWFlags( WNoUpdates );
    return last;
}

/*!
  Updates the widget unless updates are disabled.

  Updating the widget will erase the widget contents and generate a paint
  event from the window system.
  \sa repaint(), paintEvent(), enableUpdates(), erase()
*/

void QWidget::update()				// update widget
{
    if ( !testWFlags(WNoUpdates) )
	XClearArea( dpy, ident, 0, 0, 0, 0, TRUE );
}

/*!
  Updates a rectangle (\e x, \e y, \e w, \e h) inside the widget
  unless updates are disabled.

  Updating the widget will erase the widget area \e (x,y,w,h) and
  generate a paint event from the window system.
  \sa repaint(), paintEvent(), enableUpdates(), erase()
*/

void QWidget::update( int x, int y, int w, int h )
{						// update part of widget
    if ( !testWFlags(WNoUpdates) )
	XClearArea( dpy, ident, x, y, w, h, TRUE );
}

/*!
  \fn void QWidget::repaint( bool erase )
  Repaints the widget directly by calling paintEvent() directly,
  unless updates are disabled.

  Erases the widget contents if \e erase is TRUE.

  Doing a repaint() usually is faster than doing an update(), but
  calling update() many times in a row will generate a single paint
  event.
  \sa update(), paintEvent(), enableUpdates(), erase()
*/

/*!
  \fn void QWidget::repaint( int x, int y, int w, int h, bool erase )
  Repaints the widget directly by calling paintEvent() directly,
  unless updates are disabled.

  Erases the widget area  \e (x,y,w,h) if \e erase is TRUE.

  Doing a repaint() usually is faster than doing an update(), but
  calling update() many times in a row will generate a single paint
  event.
  \sa update(), paintEvent(), enableUpdates(), erase()
*/

/*!
  Repaints the widget directly by calling paintEvent() directly,
  unless updates are disabled.

  Erases the widget area  \e r if \e erase is TRUE.

  Doing a repaint() usually is faster than doing an update(), but
  calling update() many times in a row will generate a single paint
  event.
  \sa update(), paintEvent(), enableUpdates(), erase()
*/

void QWidget::repaint( const QRect &r, bool erase )
{
    if ( !isVisible() || testWFlags(WNoUpdates) ) // ignore repaint
	return;
    QPaintEvent e( r );				// send fake paint event
    if ( erase )
	XClearArea( dpy, ident, r.x(), r.y(), r.width(), r.height(), FALSE );
    QApplication::sendEvent( this, &e );
}


/*!
  Makes the widget and its children visible on the screen.
  \sa hide(), isVisible()
*/

void QWidget::show()				// show widget
{
    if ( testWFlags(WState_Visible) )
	return;
    if ( children() ) {
	QObjectListIt it(*children());
	register QObject *object;
	QWidget *widget;
	while ( it ) {				// show all widget children
	    object = it.current();		//   (except popups)
	    if ( object->isWidgetType() ) {
		widget = (QWidget*)object;
		if ( !widget->testWFlags(WExplicitHide) )
		    widget->show();
	    }
	    ++it;
	}
    }
    XMapWindow( dpy, ident );
    setWFlags( WState_Visible );
    clearWFlags( WExplicitHide );
    if ( testWFlags(WType_Modal) )
	qt_enter_modal( this );
    else if ( testWFlags(WType_Popup) )
	qt_open_popup( this );
}

/*!
  Makes the widget invisible.
  \sa show(), isVisible()
*/

void QWidget::hide()				// hide widget
{
    setWFlags( WExplicitHide );
    if ( !testWFlags(WState_Visible) )		// not visible
	return;
    if ( qApp->focus_widget == this )
	qApp->focus_widget = 0;			// reset focus widget
    if ( parentWidget() && parentWidget()->focusChild == this )
	parentWidget()->focusChild = 0;
    if ( testWFlags(WType_Modal) )
	qt_leave_modal( this );
    else if ( testWFlags(WType_Popup) )
	qt_close_popup( this );
    XUnmapWindow( dpy, ident );
    clearWFlags( WState_Visible );
}


/*!
  Raises this widget to the top of the window stack.
  \sa lower()
*/

void QWidget::raise()				// raise widget
{
    XRaiseWindow( dpy, ident );
}

/*!
  Lowers the widget to the bottom of the windows stack.  Only the
  parent window will be behind this one afterwards.
  \sa raise()
*/

void QWidget::lower()				// lower widget
{
    XLowerWindow( dpy, ident );
}


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

/*!
  \fn void QWidget::move( const QPoint &p )
  Moves the widget to position \e p relative to the parent widget.

  A \link moveEvent() move event\endlink is generated immediately.
  \sa resize(), setGeometry(), moveEvent()
*/

/*!
  Moves the widget to the position \e (x,y) relative to the parent widget.

  A \link moveEvent() move event\endlink is generated immediately.

  This function is virtual, and all other overloaded move()
  implementations call it.
  \sa resize(), setGeometry(), moveEvent()
*/

void QWidget::move( int x, int y )		// move widget
{
    QPoint p(x,y);
    QPoint oldp = frameGeometry().topLeft();
    QRect  r = frect;
    if ( testWFlags(WType_Desktop) )
	return;
    r.setTopLeft( p );
    setFRect( r );
    if ( testWFlags(WType_Overlap) ) {
	XSizeHints size_hints;			// tell window manager
	size_hints.flags = PPosition;
	size_hints.x = x;
	size_hints.y = y;
	do_size_hints( dpy, ident, extra, &size_hints );
    }
    XMoveWindow( dpy, ident, x, y );
    QMoveEvent e( r.topLeft(), oldp );
    QApplication::sendEvent( this, &e );	// send move event immediately
}


/*!
 \fn void QWidget::resize(const QSize & p)
  Resizes the widget to size \e p.

  A \link resizeEvent() resize event\endlink is generated at once.
  \sa move(), setGeometry(), resizeEvent()
*/

/*!
  Resizes the widget to size \e w by \e h pixels.

  A \link resizeEvent() resize event\endlink is generated at once.

  This function is virtual, and all other overloaded resize()
  implementations call it.
  \sa move(), setGeometry(), resizeEvent()
*/

void QWidget::resize( int w, int h )		// resize widget
{
    if ( w < 1 )				// invalid size
	w = 1;
    if ( h < 1 )
	h = 1;
    QRect r = crect;
    QSize s(w,h);
    QSize olds = size();
    if ( testWFlags(WType_Desktop) )
	return;
    r.setSize( s );
    setCRect( r );
    if ( testWFlags(WType_Overlap) ) {
	XSizeHints size_hints;			// tell window manager
	size_hints.flags = PSize;
	size_hints.width = w;
	size_hints.height = h;
	do_size_hints( dpy, ident, extra, &size_hints );
    }
    XResizeWindow( dpy, ident, w, h );
    QResizeEvent e( s, olds );
    QApplication::sendEvent( this, &e );	// send resize event immediatly
}

/*!
  \fn void QWidget::setGeometry( const QRect &r )
  Changes the widget geometry to \e r, relative to its parent
  widget.

  A \link resizeEvent() resize event\endlink and a
  \link moveEvent() move event\endlink is generated immediately.
  \sa move(), resize()
*/

/*!
  Changes the widget geometry to \e w by \e h, positioned at
  \e x,y in its parent widget.

  A \link resizeEvent() resize event\endlink and a
  \link moveEvent() move event\endlink is generated immediately.

  This function is virtual, and all other overloaded setGeometry()
  implementations call it.

  \sa move(), resize()
*/

void QWidget::setGeometry( int x, int y, int w, int h )
{						// move and resize widget
    if ( w < 1 )				// invalid size
	w = 1;
    if ( h < 1 )
	h = 1;
    QPoint oldp = frameGeometry().topLeft();
    QSize  olds = size();
    QRect  r( x, y, w, h );
    if ( testWFlags(WType_Desktop) )
	return;
    setCRect( r );
    if ( testWFlags(WType_Overlap) ) {
	XSizeHints size_hints;			// tell window manager
	size_hints.flags = USPosition | USSize;
	size_hints.x = x;
	size_hints.y = y;
	size_hints.width = w;
	size_hints.height = h;
	do_size_hints( dpy, ident, extra, &size_hints );
    }
    XMoveResizeWindow( dpy, ident, x, y, w, h );
    QResizeEvent e1( r.size(), olds );
    QApplication::sendEvent( this, &e1 );	// send resize event
    QMoveEvent e2( r.topLeft(), oldp );
    QApplication::sendEvent( this, &e2 );	// send move event
}

/*!
  Sets the minimum size of the widget to \e w by \e h pixels.
  The user will not be able to resize the window to a smaller size.
  \sa setMaximumSize(), setSizeIncrement(), frameGeometry(), size()
*/

void QWidget::setMinimumSize( int w, int h )	// set minimum size
{
    if ( testWFlags(WType_Overlap) ) {
	createExtra();
	extra->minw = w;
	extra->minh = h;
	XSizeHints size_hints;
	size_hints.flags = 0;
	do_size_hints( dpy, ident, extra, &size_hints );
    }
}

/*!
  Sets the maximum size of the widget to \e w by \e h pixels.
  The user will not be able to resize the window to a larger size.
  \sa setMinimumSize(), setSizeIncrement(), frameGeometry(), size()
*/

void QWidget::setMaximumSize( int w, int h )	// set maximum size
{
    if ( testWFlags(WType_Overlap) ) {
	createExtra();
	extra->maxw = w;
	extra->maxh = h;
	XSizeHints size_hints;
	size_hints.flags = 0;
	do_size_hints( dpy, ident, extra, &size_hints );
    }
}

/*!
  Sets the size increment of the widget.  When the user resizes the
  window, the size will move in steps of \e w pixels horizontally and
  \e h pixels vertically.

  The default setting is 1 for both \e w and \e h.
  \sa setMinimumSize(), setMaximumSize(), frameGeometry(), size()
*/

void QWidget::setSizeIncrement( int w, int h )
{						// set size increment
    if ( testWFlags(WType_Overlap) ) {
	createExtra();
	extra->incw = w;
	extra->inch = h;
	XSizeHints size_hints;
	size_hints.flags = 0;
	do_size_hints( dpy, ident, extra, &size_hints );
    }
}

/*!
  Erases the widget contents without generating a \link paintEvent()
  paint event\endlink.

  Child widgets are not affected.

  \sa repaint()
*/

void QWidget::erase()				// erase widget contents
{
    XClearArea( dpy, ident, 0, 0, 0, 0, FALSE );
}

/*!
  Scrolls the contents of the widget \e dx pixels rightwards and \e dy
  pixels downwards.  If \e dx/dy is negative, the scroll direction is
  leftwards/upwards.  Child widgets are moved accordingly.

  The areas of the widget that are exposed will be erased and a
  \link paintEvent() paint event\endlink will be generated.

  \sa erase(), bitBlt()
*/

void QWidget::scroll( int dx, int dy )		// scroll widget contents
{
    QSize s = size();
    int x1, y1, x2, y2, w=s.width(), h=s.height();
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
    XCopyArea( dpy, ident, ident, qt_xget_readonly_gc(), x1, y1, w, h, x2, y2);
    if ( children() ) {				// scroll children
	QPoint pd( dx, dy );
	QObjectListIt it(*children());
	register QObject *object;
	while ( it ) {				// move all children
	    object = it.current();
	    if ( object->isWidgetType() ) {
		QWidget *w = (QWidget *)object;
		w->move( w->geometry().topLeft()+pd );
	    }
	    ++it;
	}
    }
    if ( dx ) {
	x1 = x2 == 0 ? w : 0;
	XClearArea( dpy, ident, x1, 0, s.width()-w, s.height(), TRUE );
    }
    if ( dy ) {
	y1 = y2 == 0 ? h : 0;
	XClearArea( dpy, ident, 0, y1, s.width(), s.height()-h, TRUE );
    }
}


/*! \fn void QWidget::drawText( const QPoint &pos, const char *str )
  Writes \e str at position \e pos. The \e pos.y() position is the base line
  position of the text.
  The text is drawn using the current font and the current foreground color.

  We recommend using a \link QPainter painter\endlink instead.
  \sa setFont(), setPalette(), QPainter::drawText()
*/

/*!
  Writes \e str at position \e x,y.  The \e y position is the base
  line position of the text.
  The text is drawn using the current font and the current foreground color.

  We recommend using a \link QPainter painter\endlink instead.
  \sa setFont(), setPalette(), QPainter::drawText()
*/

void QWidget::drawText( int x, int y, const char *str )
{
    if ( testWFlags(WState_Visible) ) {
	QPainter paint;
	paint.begin( this );
	paint.drawText( x, y, str );
	paint.end();
    }
}


/*!
  Internal implementation of the virtual QPaintDevice::metric() function.

  Use the QPaintDeviceMetrics class instead.
*/

long QWidget::metric( int m ) const		// return widget metrics
{
    long val;
    if ( m == PDM_WIDTH || m == PDM_HEIGHT ) {
	if ( m == PDM_WIDTH )
	    val = crect.width();
	else
	    val = crect.height();
    }
    else {
	int scr = qt_xscreen();
	switch ( m ) {
	    case PDM_WIDTHMM:
		val = ((long)DisplayWidthMM(dpy,scr)*crect.width())/
		      DisplayWidth(dpy,scr);
		break;
	    case PDM_HEIGHTMM:
		val = ((long)DisplayHeightMM(dpy,scr)*crect.height())/
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
// QWindow member functions
//

/*!
  Sets the window caption (title).
  \sa caption()
*/

void QWindow::setCaption( const char *text )	// set caption text
{
    ctext = text;
    XStoreName( dpy, id(), (const char *)ctext );
}

/*!
  Sets the text of the window's icon.
  \sa iconText()
*/

void QWindow::setIconText( const char *text )	// set icon text
{
    itext = text;
    XSetIconName( dpy, id(), (const char *)itext );
}

/*!
  Sets the window icon pixmap.
*/

void QWindow::setIcon( QPixmap *pixmap )	// set icon pixmap
{
    if ( ipm != pixmap ) {
	delete ipm;
	ipm = pixmap;
    }
    XWMHints wm_hints;				// window manager hints
    wm_hints.input = True;
    wm_hints.icon_pixmap = ipm->handle();
    wm_hints.flags = IconPixmapHint;
    XSetWMHints( display(), id(), &wm_hints );
}
