/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qwidget_x11.cpp#67 $
**
** Implementation of QWidget and QView classes for X11
**
** Author  : Haavard Nord
** Created : 931031
**
** Copyright (C) 1993-1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qview.h"
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
static char ident[] = "$Id: //depot/qt/main/src/kernel/qwidget_x11.cpp#67 $";
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
    if ( testFlag(WState_Created) )		// already created
	return FALSE;
    setFlag( WState_Created );			// set created flag

    if ( !parentWidget() )
	setFlag( WType_Overlap );		// overlapping widget

    int	   screen = qt_xscreen();		// X11 screen
    int	   sw = DisplayWidth( dpy, screen );	// screen width
    int	   sh = DisplayHeight( dpy, screen );	// screen height
    bool   overlap = testFlag( WType_Overlap );
    bool   popup   = testFlag( WType_Popup );
    bool   modal   = testFlag( WType_Modal );
    bool   desktop = testFlag( WType_Desktop );
    Window parentwin;
    int	   border = 0;
    WId	   id;

    bg_col = pal.normal().background();		// set default background color

    if ( modal ) {				// modal windows overlap
	overlap = TRUE;
	setFlag( WType_Overlap );
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
				  frect.left(), frect.top(),
				  frect.width(), frect.height(),
				  border,
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
    if ( testFlag(WResizeNoErase) ) {
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
	setFlag( WCursorSet );
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
    if ( testFlag(WState_Created) ) {
	clearFlag( WState_Created );
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
	if ( testFlag(WType_Modal) )		// just be sure we leave modal
	    qt_leave_modal( this );
	else if ( testFlag(WType_Popup) )
	    qt_close_popup( this );
	if ( !testFlag(WType_Desktop) )
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
    if ( testFlag(WType_Desktop) )
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
    clearFlag( WState_Created );
    clearFlag( WState_Visible );
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
    bool v = testFlag( WMouseTracking );
    ulong m;
    if ( enable ) {
	m = PointerMotionMask;
	setFlag( WMouseTracking );
    }
    else {
	m = 0;
	clearFlag( WMouseTracking );
    }
    if ( testFlag(WType_Desktop) ) {		// desktop widget?
	if ( testFlag(WPaintDesktop) )		// get desktop paint events
	    XSelectInput( dpy, ident, ExposureMask );
    }
    else
	XSelectInput( dpy, ident,		// specify events
		      m | stdWidgetEventMask );
    return v;
}

/*!
Returns the background color of this widget.

The background color is independent of the color group.
The background color will be overwritten when setting a new palette.
\sa setBackgroundColor(). */

QColor QWidget::backgroundColor() const		// get background color
{
    return bg_col;
}

/*!
Returns the foreground color of this widget.

The foreground color equals <code>colorGroup().foreground()</code>.

\sa backgroundColor() and colorGroup().
*/

QColor QWidget::foregroundColor() const		// get foreground color
{
    return colorGroup().foreground();
}

/*!
Sets the background color of this widget.

The background color is independent of the widget color group.<br>
Notice that the background color will be overwritten when setting
a new palette.
\sa backgroundColor() and setPalette(). */

void QWidget::setBackgroundColor( const QColor &c )
{						// set background color
    bg_col = c;
    XSetWindowBackground( dpy, ident, bg_col.pixel() );
    update();
}

/*!
Sets the background pixmap of the widget to \e pm.

The background pixmap is tiled.
*/

void QWidget::setBackgroundPixmap( const QPixmap &pm )
{
    if ( !pm.isNull() ) {
	XSetWindowBackgroundPixmap( dpy, ident, pm.handle() );
	if ( testFlag(WType_Desktop) )		// save rootinfo later
	    qt_updated_rootinfo();
	update();
    }
}


/*!
Returns the font currently set for the widget.

QFontInfo will tell you what font is actually being used.

\sa setFont(), fontMetrics() and fontInfo().
*/

QFont &QWidget::font()
{
    return fnt;
}

/*!
Sets the font of the widget.

The fontInfo() function reports the actual font that is being used by the
widget.

This code fragment switches to a bold version of whatever font is being used:
\code
  QFont f = font();
  f.setWeight( QFont::Bold );
  setFont( f );
\endcode

\sa font() and fontInfo().
*/

void QWidget::setFont( const QFont &font )	// set font
{
    fnt = font;
    update();
}


/*!
Returns the widget cursor shape.
\sa setCursor().
*/

QCursor QWidget::cursor() const
{
    return curs;
}

/*!
Sets the widget cursor shape.

The mouse cursor will assume this shape when it's over this widget.
The available shapes are listed in the QCursor documentation.

An editor widget would for example use an I-beam cursor:
\code
  setCursor( ibeamCursor );
\endcode

\sa cursor().
*/

void QWidget::setCursor( const QCursor &cursor )
{
    curs = cursor;
    QCursor *appc = QApplication::cursor();
    XDefineCursor( dpy, ident, appc ? appc->handle() : curs.handle() );
    setFlag( WCursorSet );
    XFlush( dpy );
}


/*!
Grabs the mouse input.

The widget will continue to get mouse events until releaseMouse() is called.

\warning This might lock your terminal.

It is almost never necessary to grab the mouse when using Qt since Qt
grabs and releases it sensibly.  In particular, Qt grabs the mouse
when a button is pressed and keeps it until the last button is
released.

\sa releaseMouse(). */

void QWidget::grabMouse()
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

/*!
Grabs the mouse intput and change the cursor shape.

The cursor will assume shape \e cursor (for as long as the mouse focus is
grabbed) and the widget will continue getting mouse events until
releaseMouse() is called().

\sa releaseMouse(), setCursor(). */

void QWidget::grabMouse( const QCursor &cursor )
{
    if ( !testFlag(WState_MGrab) ) {
	setFlag( WState_MGrab );
	XGrabPointer( dpy, ident, TRUE,
		      ButtonPressMask | ButtonReleaseMask | ButtonMotionMask |
		      EnterWindowMask | LeaveWindowMask,
		      GrabModeAsync, GrabModeAsync,
		      None, cursor.handle(), CurrentTime );
    }
}

/*!
Releases the mouse from a grab.

\sa grabMouse().
*/

void QWidget::releaseMouse()
{
    if ( testFlag(WState_MGrab) ) {
	clearFlag( WState_MGrab );
	XUngrabPointer( dpy, CurrentTime );
    }
}

/*!
Grabs the keyboard input focus.

This widget will receive all keyboard
events, no matter where the mouse cursor is.  \sa releaseKeyboard(),
grabMouse(), releaseMouse().
*/

void QWidget::grabKeyboard()
{
    if ( !testFlag(WState_KGrab) ) {
	setFlag( WState_KGrab );
	XGrabKeyboard( dpy, ident, TRUE, GrabModeSync, GrabModeSync,
		       CurrentTime );
    }
}

/*!  Releases the keyboard focus.  The keyboard events will follow
  their natural inclination. \sa grabKeyboard(), grabMouse(),
  releaseMouse(). */

void QWidget::releaseKeyboard()
{
    if ( testFlag(WState_KGrab) ) {
	clearFlag( WState_KGrab );
	XUngrabKeyboard( dpy, CurrentTime );
    }
}


/*!
Gives this widget the keyboard input focus.
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
Enables or disables updates of this widget.  If updates are
disabled, the widget will not receive repaint events.
*/

bool QWidget::enableUpdates( bool enable )	// enable widget update/repaint
{
    bool last = !testFlag( WNoUpdates );
    if ( enable )
	clearFlag( WNoUpdates );
    else
	setFlag( WNoUpdates );
    return last;
}

/*! Updates the entire widget.  If updates are enabled, of course.  The
  default action of this is to clear the widget, so widgets that wish
  to actually interact with the user should reimplement it.  */

void QWidget::update()				// update widget
{
    if ( !testFlag(WNoUpdates) )
	XClearArea( dpy, ident, 0, 0, 0, 0, TRUE );
}

/*!
Updates a rectangle (\e x, \e y, \e w, \e h) inside the widget.

Calling update() will generate a paint event from the X server.
*/

void QWidget::update( int x, int y, int w, int h )
{						// update part of widget
    if ( !testFlag(WNoUpdates) )
	XClearArea( dpy, ident, x, y, w, h, TRUE );
}

/*!
Repaints the widget directly.

Doing a repaint() is faster than doing an update(), but since
repaint() does not make a server trip and there is some time skew
between the server and client, your client may get confused. */

void QWidget::repaint( const QRect &r, bool eraseArea )
{
    if ( !isVisible() || testFlag(WNoUpdates) )	// ignore repaint
	return;
    QPaintEvent e( r );				// send fake paint event
    if ( eraseArea )
	XClearArea( dpy, ident, r.x(), r.y(), r.width(), r.height(), FALSE );
    QApplication::sendEvent( this, &e );
}


/*! Makes the widget and its children visible on the screen. */

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
    if ( testFlag(WType_Modal) )
	qt_enter_modal( this );
    else if ( testFlag(WType_Popup) )
	qt_open_popup( this );
}

/*! Makes the widget invisible. */

void QWidget::hide()				// hide widget
{
    setFlag( WExplicitHide );
    if ( !testFlag(WState_Visible) )		// not visible
	return;
    if ( qApp->focus_widget == this )
	qApp->focus_widget = 0;			// reset focus widget
    if ( parentWidget() && parentWidget()->focusChild == this )
	parentWidget()->focusChild = 0;
    if ( testFlag(WType_Modal) )
	qt_leave_modal( this );
    else if ( testFlag(WType_Popup) )
	qt_close_popup( this );
    XUnmapWindow( dpy, ident );
    clearFlag( WState_Visible );
}


/*! Raises the widget to the top of the window stack. */

void QWidget::raise()				// raise widget
{
    XRaiseWindow( dpy, ident );
}

/*! Lowers the widget to the bottom of the windows stack.  Only the
  parent window will be behind this one afterwards. */

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

/*! Moves the widget.  \e x and \e y are relative to the widget's
  parent.  If necessary, the window manager is told about the
  change. A \link QWidget::moveEvent move event \endlink is sent at
  once. \sa resize(), setGeometry(), QWidget::moveEvent().*/

void QWidget::move( int x, int y )		// move widget
{
    QPoint p(x,y);
    QRect r = frect;
    if ( testFlag(WType_Desktop) )
	return;
    r.setTopLeft( p );
    setFRect( r );
    if ( testFlag(WType_Overlap) ) {
	XSizeHints size_hints;			// tell window manager
	size_hints.flags = PPosition;
	size_hints.x = x;
	size_hints.y = y;
	do_size_hints( dpy, ident, extra, &size_hints );
    }
    XMoveWindow( dpy, ident, x, y );
    QMoveEvent e( r.topLeft() );
    QApplication::sendEvent( this, &e );	// send move event immediately
}

/*! \fn void QWidget::move( const QPoint &p )

  Moves the widget to position \e p, which is relative to the widget's
  arent.  If necessary, the window manager is told about the
  change.  A \link moveEvent move event \endlink is sent at
  once. \sa resize(), setGeometry(), moveEvent(). */

/*! Resizes the widget to size \e w pixels by \e h.  If necessary, the
  window manager is told about the change.  A resize event is sent at
  once. \sa move(), setGeometry(), resizeEvent(). */

void QWidget::resize( int w, int h )		// resize widget
{
    if ( w < 1 )				// invalid size
	w = 1;
    if ( h < 1 )
	h = 1;
    QRect r = crect;
    QSize s(w,h);
    if ( testFlag(WType_Desktop) )
	return;
    r.setSize( s );
    setCRect( r );
    if ( testFlag(WType_Overlap) ) {
	XSizeHints size_hints;			// tell window manager
	size_hints.flags = PSize;
	size_hints.width = w;
	size_hints.height = h;
	do_size_hints( dpy, ident, extra, &size_hints );
    }
    XResizeWindow( dpy, ident, w, h );
    QResizeEvent e( s );
    QApplication::sendEvent( this, &e );	// send resize event immediatly
}

/*! \fn void QWidget::resize(const QSize & p)

  Resizes the widget to size \e p.  If necessary, the window manager
  is told about the change.  A resize event is sent at once. \sa
  move(), setGeometry(), resizeEvent(). */

/*! \fn void QWidget::setGeometry( const QRect &r )

  Changes the widget geometry to \e r, relative to its parent
  widget.  If necessary, the window manager is informed.  First a
  resize and then a move event is sent to the widget itself.  \sa
  move(), resize(). */

/*! Changes the widget geometry to \e w pixels by \e h, positioned at
  \e x,y in its parent widget.  If necessary, the window manager is
  informed.  First a resize and then a move event is sent to the widget
  itself.

  This function is virtual, and all other overloaded setGeometry()
  implementations call it.

  \sa move(), resize(). */

void QWidget::setGeometry( int x, int y, int w, int h )
{						// move and resize widget
    if ( w < 1 )				// invalid size
	w = 1;
    if ( h < 1 )
	h = 1;
    QRect  r( x, y, w, h );
    if ( testFlag(WType_Desktop) )
	return;
    setCRect( r );
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
    QResizeEvent e1( r.size() );
    QApplication::sendEvent( this, &e1 );	// send resize event
    QMoveEvent e2( r.topLeft() );
    QApplication::sendEvent( this, &e2 );	// send move event
}

/*! Sets the minimum size of the widget to \e w pixels wide and \e h
  pixels high.  The user will not be able to resize the widget to a
  smaller size.  \sa setMaximumSize(), setSizeIncrement(), size() and
  geometry(). */

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

/*! Sets the maximum size of the widget to \e w pixels wide and \e h
  pixels high.  The user will not be able to resize the widget to a
  larger size.  \sa setMinimumSize(), setSizeIncrement(), size() and
  geometry(). */

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

/*! Sets the size increment of the widget.  When the user resizes the
  widget, the size will move in steps of \e w horizontally and \e h
  vertically.  Both default to 1. \sa setMaximumSize() and
  setSizeIncrement(). */

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

/*! Clears the widget on-screen.  Child widgets are not disturbed, and
  repaint events are \e not generated. */

void QWidget::erase()				// erase widget contents
{
    XClearArea( dpy, ident, 0, 0, 0, 0, FALSE );
}

/*! Move the contents of the widget \e dx pixels rightwards and \e dy
  pixels downwards.  If \e dx/dy is negative, the move is
  leftwards/upwards.  Child widgets are moved accordingly.

  The leftmost/top/rightmost/bottom (you know where) part of the
  widget is cleared, and repaint events will eventually be generated
  for it.  It's better, though, to redraw at once, if you wait for
  repaint() the screen may flicker.

  If there are windows on top of your widget, you will get repaint
  events for areas of your widget that are scrolled.

  \sa setBackgroundColor(), setBackgroundPixmap(). */

void QWidget::scroll( int dx, int dy )		// scroll widget contents
{
    QSize sz = size();
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
	XClearArea( dpy, ident, x1, 0, sz.width()-w, sz.height(), TRUE );
    }
    if ( dy ) {
	y1 = y2 == 0 ? h : 0;
	XClearArea( dpy, ident, 0, y1, sz.width(), sz.height()-h, TRUE );
    }
}

/*! \fn void QWidget::drawText( const QPoint &p, const char *s )

  Writes \e s to position \e p.  The y position is the base line, not
  the top or bottom of the text.  The text is drawn in the current
  font and so on.  \sa setFont(), FontMetrics(),
  QPainter::drawText. */

/*! Writes \e str to position \e x,y.  The y position is the base
  line, not the top or bottom of the text.  The text is drawn in the
  current font and so on.  \sa setFont(), FontMetrics(),
  QPainter::drawText. */

void QWidget::drawText( int x, int y, const char *str )
{						// draw text in widget
    if ( testFlag( WState_Visible ) ) {
	QPainter paint;
	paint.begin( this );
	paint.drawText( x, y, str );
	paint.end();
    }
}


/*! Returns any of several widget metrics.  This may be of use
  e.g. for selecting monochrome or color appearance in a widget, but
  application programmers should use QPaintDeviceMetrics instead, or
  even better, write code that works without depending on these metrics.

  The metric commands are defined in qpaintdc.h.  At the time of
  writing, <code>PDM_WIDTH, PDM_HEIGHT, PDM_WIDTHMM, PDM_HEIGHTMM,
  PDM_NUMCOLORS</code> and \c PDM_NUMPLANES are supported.  \c
  PDM_NUMCOLORS returns the number of actual colors, not the number of
  possible colors, use \e PDM_NUMPLANES if that's what you want. */

long QWidget::metric( int m ) const		// get metric information
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

void QView::setIcon( QPixmap *pm )		// set icon pixmap
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
