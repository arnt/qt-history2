/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qwidget_x11.cpp#127 $
**
** Implementation of QWidget and QWindow classes for X11
**
** Author  : Haavard Nord
** Created : 931031
**
** Copyright (C) 1993-1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qwindow.h"
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

RCSTAG("$Id: //depot/qt/main/src/kernel/qwidget_x11.cpp#127 $")


void qt_enter_modal( QWidget * );		// defined in qapp_x11.cpp
void qt_leave_modal( QWidget * );		// --- "" ---
bool qt_modal_state();				// --- "" ---
void qt_open_popup( QWidget * );		// --- "" ---
void qt_close_popup( QWidget * );		// --- "" ---
void qt_updated_rootinfo();


/*****************************************************************************
  QWidget member functions
 *****************************************************************************/


extern Atom q_wm_delete_window;			// defined in qapp_x11.cpp

const uint stdWidgetEventMask =			// X event mask
	KeyPressMask | KeyReleaseMask |
	ButtonPressMask | ButtonReleaseMask |
	KeymapStateMask |
	ButtonMotionMask |
	EnterWindowMask | LeaveWindowMask |
	FocusChangeMask |
	ExposureMask |
	StructureNotifyMask | SubstructureRedirectMask;


/*----------------------------------------------------------------------------
  \internal
  Creates the widget window.
  Usually called from the QWidget constructor.
 ----------------------------------------------------------------------------*/

bool QWidget::create()
{
    if ( testWFlags(WState_Created) )		// already created
	return FALSE;
    setWFlags( WState_Created );		// set created flag

    if ( !parentWidget() )
	setWFlags( WType_TopLevel );		// top level widget

    int	   scr	    = qt_xscreen();
    int	   sw	    = DisplayWidth(dpy,scr);
    int	   sh	    = DisplayHeight(dpy,scr);
    bool   topLevel = testWFlags(WType_TopLevel);
    bool   popup    = testWFlags(WType_Popup);
    bool   modal    = testWFlags(WType_Modal);
    bool   desktop  = testWFlags(WType_Desktop);
    Window parentw;
    WId	   id;

    bg_col = pal.normal().background();		// default background color

    if ( modal || popup || desktop ) {		// these are top level, too
	topLevel = TRUE;
	setWFlags( WType_TopLevel );
    }

    if ( desktop ) {				// desktop widget
	frect.setRect( 0, 0, sw, sh );
	modal = popup = FALSE;			// force these flags off
    }
    else if ( topLevel )			// calc pos/size from screen
	frect.setRect( sw/2 - sw/4, sh/2 - sh/5, sw/2, 2*sh/5 );
    else					// child widget
	frect.setRect( 10, 10, 100, 30 );
    crect = frect;				// default client rect

    parentw = topLevel ? RootWindow(dpy,scr) : parentWidget()->winId();

    if ( desktop ) {				// desktop widget
	id = parentw;				// id = root window
	QWidget *otherDesktop = find( id );	// is there another desktop?
	if ( otherDesktop && otherDesktop->testWFlags(WPaintDesktop) ) {
	    otherDesktop->setWinId( 0 );	// remove id from widget mapper
	    setWinId( id );			// make sure otherDesktop is
	    otherDesktop->setWinId( id );	//   found first
	}
	else
	    setWinId( id );
    }
    else {
	id = XCreateSimpleWindow( dpy, parentw,
				  frect.left(), frect.top(),
				  frect.width(), frect.height(),
				  0,
				  black.pixel(),
				  bg_col.pixel() );
	setWinId( id );				// set widget id/handle + hd
    }

    if ( popup ) {				// popup widget
	XSetTransientForHint( dpy, parentw, id );
	XSetWindowAttributes v;
	v.override_redirect = TRUE;
	v.save_under = TRUE;
	XChangeWindowAttributes( dpy, id,
				 CWOverrideRedirect | CWSaveUnder, &v );
    }
    else if ( topLevel && !desktop ) {		// top level widget
	if ( modal ) {
	    QWidget *p = parentWidget();	// real parent
	    QWidget *pp = p ? p->parentWidget() : 0;
	    while ( pp && !pp->testWFlags(WType_Modal) ) {
		p = pp;				// find real parent
		pp = pp->parentWidget();
	    }
	    if ( p )				// modal to one widget
		XSetTransientForHint( dpy, id, p->winId() );
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
	XResizeWindow( dpy, id, crect.width(), crect.height() );
	XStoreName( dpy, id, title );
	Atom protocols[1];
	protocols[0] = q_wm_delete_window;	// support del window protocol
	XSetWMProtocols( dpy, id, protocols, 1 );
#if 0
	Atom drag[2];
	drag[0] = XInternAtom( dpy, "XmDRAG_DROP_ONLY", FALSE );
	drag[1] = XInternAtom( dpy, "XmDRAG_PREREGISTER", FALSE );
	XSetWMProtocols( dpy, id, drag, 2 );
#endif
    }
    if ( testWFlags(WResizeNoErase) ) {
	XSetWindowAttributes v;
	v.bit_gravity = NorthWestGravity;	// don't erase when resizing
	XChangeWindowAttributes( dpy, id, CWBitGravity, &v );
    }
    setWFlags( WMouseTracking );
    setMouseTracking( FALSE );			// also sets event mask
    if ( desktop ) {
	setWFlags( WState_Visible );
    } else if ( topLevel ) {			// set X cursor
	QCursor *oc = QApplication::overrideCursor();
	XDefineCursor( dpy, winid, oc ? oc->handle() : curs.handle() );
	setWFlags( WCursorSet );
    }
    return TRUE;
}


/*----------------------------------------------------------------------------
  \internal
  Destroys the widget window and frees up window system resources.
  Usually called from the QWidget destructor.
 ----------------------------------------------------------------------------*/

bool QWidget::destroy()
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
	    register QObject *obj;
	    while ( (obj=it.current()) ) {	// destroy all widget children
		++it;
		if ( obj->isWidgetType() )
		    ((QWidget*)obj)->destroy();
	    }
	}
	if ( testWFlags(WType_Modal) )		// just be sure we leave modal
	    qt_leave_modal( this );
	else if ( testWFlags(WType_Popup) )
	    qt_close_popup( this );
	if ( !testWFlags(WType_Desktop) )
	    XDestroyWindow( dpy, winid );
	setWinId( 0 );
    }
    return TRUE;
}


/*----------------------------------------------------------------------------
  This function is provided in case a widget should feel \e really
  bad, regret that it was even born.

  It gives the widget a fresh start, new \e parent, new widget flags
  (\e f, but as usual, use 0) at a new position in its new parent (\e p).

  If \e showIt is TRUE, show() is called once the widget has been
  recreated.

  \sa getWFlags()
 ----------------------------------------------------------------------------*/

void QWidget::recreate( QWidget *parent, WFlags f, const QPoint &p,
			bool showIt )
{
    extern void qPRCreate( const QWidget *, Window );
    WId old_winid = winid;
    if ( testWFlags(WType_Desktop) )
	old_winid = 0;
    setWinId( 0 );
    if ( parentObj )				// remove from parent
	parentObj->removeChild( this );
    if ( (parentObj = parent) )
	parentObj->insertChild( this );
    bool     enable = isEnabled();		// remember status
    QSize    s      = size();
    QPixmap *bgp    = (QPixmap *)backgroundPixmap();
    QColor   bgc    = bg_col;			// save colors
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
		XReparentWindow( dpy, w->winId(), winId(), w->geometry().x(),
				 w->geometry().y() );
	    }
	    ++it;
	}
    }
    qPRCreate( this, old_winid );
    if ( bgp )
	XSetWindowBackgroundPixmap( dpy, winid, bgp->handle() );
    else
	XSetWindowBackground( dpy, winid, bgc.pixel() );
    setGeometry( p.x(), p.y(), s.width(), s.height() );
    setEnabled( enable );
    if ( showIt )
	show();
    if ( old_winid )
	XDestroyWindow( dpy, old_winid );
}


/*----------------------------------------------------------------------------
  Translates the widget coordinate \e pos to global screen coordinates.
  \sa mapFromGlobal()
 ----------------------------------------------------------------------------*/

QPoint QWidget::mapToGlobal( const QPoint &pos ) const
{
    int	   x, y;
    Window child;
    XTranslateCoordinates( dpy, winid, QApplication::desktop()->winId(),
			   pos.x(), pos.y(), &x, &y, &child );
    return QPoint( x, y );
}

/*----------------------------------------------------------------------------
  Translates the global screen coordinate \e pos to widget coordinates.
  \sa mapToGlobal()
 ----------------------------------------------------------------------------*/

QPoint QWidget::mapFromGlobal( const QPoint &pos ) const
{
    int	   x, y;
    Window child;
    XTranslateCoordinates( dpy, QApplication::desktop()->winId(), winid,
			   pos.x(), pos.y(), &x, &y, &child );
    return QPoint( x, y );
}


/*----------------------------------------------------------------------------
  Sets the background color of this widget.

  The background color is independent of the widget color group.
  Setting a new palette overwrites the background color.

  \sa backgroundColor(), backgroundColorChange(), setPalette(),
  setBackgroundPixmap()
 ----------------------------------------------------------------------------*/

void QWidget::setBackgroundColor( const QColor &color )
{
    QColor old = bg_col;
    bg_col = color;
    XSetWindowBackground( dpy, winid, bg_col.pixel() );
    if ( extra && extra->bg_pix ) {		// kill the background pixmap
	delete extra->bg_pix;
	extra->bg_pix = 0;
    }
    backgroundColorChange( old );
}

/*----------------------------------------------------------------------------
  Sets the background pixmap of the widget to \e pixmap.

  The background pixmap is tiled.

  \sa backgroundPixmap(), backgroundPixmapChange(), setBackgroundColor()
 ----------------------------------------------------------------------------*/

void QWidget::setBackgroundPixmap( const QPixmap &pixmap )
{
    QPixmap old;
    if ( extra && extra->bg_pix )
	old = *extra->bg_pix;
    if ( pixmap.isNull() ) {
	XSetWindowBackground( dpy, winid, bg_col.pixel() );
	if ( extra && extra->bg_pix ) {
	    delete extra->bg_pix;
	    extra->bg_pix = 0;
	}
    }
    else {
	QPixmap pm = pixmap;
	if ( pm.depth() == 1 && QPixmap::defaultDepth() > 1 ) {
	    pm = QPixmap( pixmap.size() );
	    bitBlt( &pm, 0, 0, &pixmap, 0, 0, pm.width(), pm.height() );
	}
	if ( extra && extra->bg_pix )
	    delete extra->bg_pix;
	else
	    createExtra();
	extra->bg_pix = new QPixmap( pm );
	XSetWindowBackgroundPixmap( dpy, winid, pm.handle() );
	if ( testWFlags(WType_Desktop) )	// save rootinfo later
	    qt_updated_rootinfo();
    }
    backgroundPixmapChange( old );
}


/*----------------------------------------------------------------------------
  Sets the widget cursor shape to \e cursor.

  The mouse cursor will assume this shape when it's over this widget.
  See a list of cursor shapes in the QCursor documentation.

  An editor widget would for example use an I-beam cursor:
  \code
    setCursor( ibeamCursor );
  \endcode

  \sa cursor(), QApplication::setOverrideCursor()
 ----------------------------------------------------------------------------*/

void QWidget::setCursor( const QCursor &cursor )
{
    curs = cursor;
    QCursor *oc = QApplication::overrideCursor();
    XDefineCursor( dpy, winid, oc ? oc->handle() : curs.handle() );
    setWFlags( WCursorSet );
    XFlush( dpy );
}


/*----------------------------------------------------------------------------
  Sets the window caption (title).
  \sa caption(), setIcon(), setIconText()
 ----------------------------------------------------------------------------*/

void QWidget::setCaption( const char *caption )
{
    if ( extra )
	delete [] extra->caption;
    else
	createExtra();
    extra->caption = qstrdup( caption );
    XStoreName( dpy, winId(), extra->caption );
}

/*----------------------------------------------------------------------------
  Sets the window icon pixmap.
  \sa icon(), setIconText(), setCaption()
 ----------------------------------------------------------------------------*/

void QWidget::setIcon( const QPixmap &pixmap )
{
    if ( extra )
	delete extra->icon;
    else
	createExtra();
    extra->icon = new QPixmap( pixmap );
    XWMHints wm_hints;				// window manager hints
    wm_hints.input = True;
    wm_hints.icon_pixmap = extra->icon->handle();
    wm_hints.flags = IconPixmapHint;
    XSetWMHints( dpy, winId(), &wm_hints );
}

/*----------------------------------------------------------------------------
  Sets the text of the window's icon to \e iconText.
  \sa iconText(), setIcon(), setCaption()
 ----------------------------------------------------------------------------*/

void QWidget::setIconText( const char *iconText )
{
    if ( extra )
	delete [] extra->iconText;
    else
	createExtra();
    extra->iconText = qstrdup( iconText );
    XSetIconName( dpy, winId(), extra->iconText );
}


void QWidget::setMouseTracking( bool enable )
{
    if ( enable == testWFlags(WMouseTracking) )
	return;
    uint m;
    if ( enable ) {
	m = PointerMotionMask;
	setWFlags( WMouseTracking );
    } else {
	m = 0;
	clearWFlags( WMouseTracking );
    }
    if ( testWFlags(WType_Desktop) ) {		// desktop widget?
	if ( testWFlags(WPaintDesktop) )	// get desktop paint events
	    XSelectInput( dpy, winid, ExposureMask );
    } else {
	XSelectInput( dpy, winid,		// specify events
		      m | stdWidgetEventMask );
    }
}


extern bool qt_nograb();

static QWidget *mouseGrb    = 0;
static QWidget *keyboardGrb = 0;

/*----------------------------------------------------------------------------
  Grabs the mouse input.

  This widget will be the only one to receive mouse events until
  releaseMouse() is called.

  \warning Grabbing the mouse might lock the terminal.

  It is almost never necessary to grab the mouse when using Qt since Qt
  grabs and releases it sensibly.	 In particular, Qt grabs the mouse
  when a button is pressed and keeps it until the last button is
  released.

  \sa releaseMouse(), grabKeyboard(), releaseKeyboard()
 ----------------------------------------------------------------------------*/

void QWidget::grabMouse()
{
    if ( !qt_nograb() ) {
	if ( mouseGrb )
	    mouseGrb->releaseMouse();
	XGrabPointer( dpy, winid, TRUE,
		      ButtonPressMask | ButtonReleaseMask |
		      ButtonMotionMask | EnterWindowMask | LeaveWindowMask,
		      GrabModeAsync, GrabModeAsync,
		      None, None, CurrentTime );
	mouseGrb = this;
    }
}

/*----------------------------------------------------------------------------
  Grabs the mouse intput and changes the cursor shape.

  The cursor will assume shape \e cursor (for as long as the mouse focus is
  grabbed) and this widget will be the only one to receive mouse events
  until releaseMouse() is called().

  \warning Grabbing the mouse might lock the terminal.

  \sa releaseMouse(), grabKeyboard(), releaseKeyboard(), setCursor()
 ----------------------------------------------------------------------------*/

void QWidget::grabMouse( const QCursor &cursor )
{
    if ( !qt_nograb() ) {
	if ( mouseGrb )
	    mouseGrb->releaseMouse();
	XGrabPointer( dpy, winid, TRUE,
		      ButtonPressMask | ButtonReleaseMask |
		      ButtonMotionMask |
		      EnterWindowMask | LeaveWindowMask,
		      GrabModeAsync, GrabModeAsync,
		      None, cursor.handle(), CurrentTime );
	mouseGrb = this;
    }
}

/*----------------------------------------------------------------------------
  Releases the mouse grab.

  \sa grabMouse(), grabKeyboard(), releaseKeyboard()
 ----------------------------------------------------------------------------*/

void QWidget::releaseMouse()
{
    if ( !qt_nograb() && mouseGrb == this ) {
	XUngrabPointer( dpy, CurrentTime );
	XFlush( dpy );
	mouseGrb = 0;
    }
}

/*----------------------------------------------------------------------------
  Grabs all keyboard input.

  This widget will receive all keyboard events, independent of the active
  window.

  \warning Grabbing the keyboard might lock the terminal.

  \sa releaseKeyboard(), grabMouse(), releaseMouse()
 ----------------------------------------------------------------------------*/

void QWidget::grabKeyboard()
{
    if ( !qt_nograb() ) {
	if ( keyboardGrb )
	    keyboardGrb->releaseKeyboard();
	XGrabKeyboard( dpy, winid, TRUE, GrabModeAsync, GrabModeAsync,
		       CurrentTime );
	keyboardGrb = this;
    }
}

/*----------------------------------------------------------------------------
  Releases the keyboard grab.

  \sa grabKeyboard(), grabMouse(), releaseMouse()
 ----------------------------------------------------------------------------*/

void QWidget::releaseKeyboard()
{
    if ( !qt_nograb() && keyboardGrb == this ) {
	XUngrabKeyboard( dpy, CurrentTime );
	keyboardGrb = 0;
    }
}


/*----------------------------------------------------------------------------
  Returns a pointer to the widget that is currently grabbing the
  mouse input.

  If no widget in this application is currently grabbing the mouse, 0 is
  returned.

  \sa grabMouse(), keyboardGrabber()
 ----------------------------------------------------------------------------*/

QWidget *QWidget::mouseGrabber()
{
    return mouseGrb;
}

/*----------------------------------------------------------------------------
  Returns a pointer to the widget that is currently grabbing the
  keyboard input.

  If no widget in this application is currently grabbing the keyboard, 0
  is returned.

  \sa grabMouse(), mouseGrabber()
 ----------------------------------------------------------------------------*/

QWidget *QWidget::keyboardGrabber()
{
    return keyboardGrb;
}


/*----------------------------------------------------------------------------
  Sets the window this widget is part of to be the active window.
  An active window is a top level window that has the keyboard
  input focus.

  This function performs the same operation as clicking the mouse on
  the title bar of a top level window.
 ----------------------------------------------------------------------------*/

void QWidget::setActiveWindow()
{
    XSetInputFocus( dpy, topLevelWidget()->winId(), RevertToNone, CurrentTime);
}


/*----------------------------------------------------------------------------
  Gives the keyboard input focus to the widget.

  First, a \link focusOutEvent() focus out event\endlink is sent to the
  focus widget (if any) to tell it that it is about to loose the
  focus. Then a \link focusInEvent() focus in event\endlink is sent to
  this widget to tell it that it just received the focus.

  This widget must enable focus setting in order to get the keyboard input
  focus, i.e. it must call setAcceptFocus(TRUE).

  \warning If you call setFocus() in a function which may itself be
  called from focusOutEvent() or focusInEvent(), you may see infinite
  recursion.

  \sa hasFocus(), focusInEvent(), focusOutEvent(), setAcceptFocus(),
  QApplication::focusWidget()
 ----------------------------------------------------------------------------*/

void QWidget::setFocus()
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


/*----------------------------------------------------------------------------
  \internal Handles TAB.
 ----------------------------------------------------------------------------*/

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

/*----------------------------------------------------------------------------
  \internal Handles Shift+TAB.
 ----------------------------------------------------------------------------*/

bool QWidget::focusPrevChild()
{
    return TRUE;
}


/*----------------------------------------------------------------------------
  Updates the widget unless updates are disabled.

  Updating the widget will erase the widget contents and generate a paint
  event from the window system. The paint event is processed after the
  program has returned to the main event loop.

  \sa repaint(), paintEvent(), setUpdatesEnabled(), erase()
 ----------------------------------------------------------------------------*/

void QWidget::update()
{
    if ( (flags & (WState_Visible|WState_DisUpdates)) == WState_Visible )
	XClearArea( dpy, winid, 0, 0, 0, 0, TRUE );
}

/*----------------------------------------------------------------------------
  Updates a rectangle (\e x, \e y, \e w, \e h) inside the widget
  unless updates are disabled.

  Updating the widget erases the widget area \e (x,y,w,h), which in turn
  generates a paint event from the window system. The paint event is
  processed after the program has returned to the main event loop.

  If \e w is negative, it is replaced with <code>width() - x</code>.
  If \e h is negative, it is replaced width <code>height() - y</code>.

  \sa repaint(), paintEvent(), setUpdatesEnabled(), erase()
 ----------------------------------------------------------------------------*/

void QWidget::update( int x, int y, int w, int h )
{
    if ( (flags & (WState_Visible|WState_DisUpdates)) == WState_Visible ) {
	if ( w < 0 )
	    w = crect.width()  - x;
	if ( h < 0 )
	    h = crect.height() - y;
	if ( w != 0 && h != 0 )
	    XClearArea( dpy, winid, x, y, w, h, TRUE );
    }
}


/*----------------------------------------------------------------------------
  \fn void QWidget::repaint( bool erase )
  Repaints the widget directly by calling paintEvent() directly,
  unless updates are disabled.

  Erases the widget contents if \e erase is TRUE.

  Doing a repaint() usually is faster than doing an update(), but
  calling update() many times in a row will generate a single paint
  event.

  \warning If you call repaint() in a function which may itself be
  called from paintEvent(), you may see infinite recursion.

  \sa update(), paintEvent(), setUpdatesEnabled(), erase()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  Repaints the widget directly by calling paintEvent() directly,
  unless updates are disabled.

  Erases the widget area  \e (x,y,w,h) if \e erase is TRUE.

  If \e w is negative, it is replaced with <code>width() - x</code>.
  If \e h is negative, it is replaced width <code>height() - y</code>.

  Doing a repaint() usually is faster than doing an update(), but
  calling update() many times in a row will generate a single paint
  event.

  \warning If you call repaint() in a function which may itself be called
  from paintEvent(), you may see infinite recursion. The update() function
  never generates recursion.

  \sa update(), paintEvent(), setUpdatesEnabled(), erase()
 ----------------------------------------------------------------------------*/

void QWidget::repaint( int x, int y, int w, int h, bool erase )
{
    if ( (flags & (WState_Visible|WState_DisUpdates)) == WState_Visible ) {
	if ( w < 0 )
	    w = crect.width()  - x;
	if ( h < 0 )
	    h = crect.height() - y;
	QPaintEvent e( QRect(x,y,w,h) );
	if ( erase && w != 0 && h != 0 )
	    XClearArea( dpy, winid, x, y, w, h, FALSE );
	QApplication::sendEvent( this, &e );
    }
}

/*----------------------------------------------------------------------------
  \overload void QWidget::repaint( const QRect &r, bool erase )
 ----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
  Shows the widget and its child widgets.
  \sa hide(), iconify(), isVisible()
 ----------------------------------------------------------------------------*/

void QWidget::show()
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
    XMapWindow( dpy, winid );
    setWFlags( WState_Visible );
    clearWFlags( WExplicitHide );
    if ( testWFlags(WType_Modal) )
	qt_enter_modal( this );
    else if ( testWFlags(WType_Popup) )
	qt_open_popup( this );
}

/*----------------------------------------------------------------------------
  Hides the widget.
  \sa show(), iconify(), isVisible()
 ----------------------------------------------------------------------------*/

void QWidget::hide()
{
    setWFlags( WExplicitHide );
    if ( !testWFlags(WState_Visible) )
	return;
    if ( qApp->focus_widget == this )
	qApp->focus_widget = 0;			// reset focus widget
    if ( parentWidget() && parentWidget()->focusChild == this )
	parentWidget()->focusChild = 0;
    if ( testWFlags(WType_Modal) )
	qt_leave_modal( this );
    else if ( testWFlags(WType_Popup) )
	qt_close_popup( this );
    XUnmapWindow( dpy, winid );
    clearWFlags( WState_Visible );
}

/*----------------------------------------------------------------------------
  Iconifies the widget.

  Calling this function has no effect for other than \link isTopLevel()
  top level widgets\endlink.

  \sa show(), hide(), isVisible()
 ----------------------------------------------------------------------------*/

void QWidget::iconify()
{
    if ( testWFlags(WType_TopLevel) )
	XIconifyWindow( dpy, winid, qt_xscreen() );
}


/*----------------------------------------------------------------------------
  Raises this widget to the top of the parent widget's stack.

  If there are any siblings of this widget that overlap it on the screen,
  this widget will be in front of its siblings afterwards.

  \sa lower()
 ----------------------------------------------------------------------------*/

void QWidget::raise()
{
    XRaiseWindow( dpy, winid );
}

/*----------------------------------------------------------------------------
  Lowers the widget to the bottom of the parent widget's stack.

  If there are siblings of this widget that overlap it on the screen, this
  widget will be obscured by its siblings afterwards.

  \sa raise()
 ----------------------------------------------------------------------------*/

void QWidget::lower()
{
    XLowerWindow( dpy, winid );
}


static void do_size_hints( Display *dpy, WId winid, QWExtra *x, XSizeHints *s )
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
    XSetWMNormalHints( dpy, winid, s );
}


/*----------------------------------------------------------------------------
  \overload void QWidget::move( const QPoint & )
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  Moves the widget to the position \e (x,y) relative to the parent widget.

  A \link moveEvent() move event\endlink is generated immediately.

  This function is virtual, and all other overloaded move()
  implementations call it.

  \warning If you call move() or setGeometry() from moveEvent(), you
  may see infinite recursion.

  \sa resize(), setGeometry(), moveEvent()
 ----------------------------------------------------------------------------*/

void QWidget::move( int x, int y )
{
    QPoint p(x,y);
    QPoint oldp = frameGeometry().topLeft();
    QRect  r = frect;
    if ( testWFlags(WType_Desktop) )
	return;
    r.moveTopLeft( p );
    setFRect( r );
    if ( testWFlags(WType_TopLevel) ) {
	XSizeHints size_hints;			// tell window manager
	size_hints.flags = PPosition;
	size_hints.x = x;
	size_hints.y = y;
	do_size_hints( dpy, winid, extra, &size_hints );
    }
    XMoveWindow( dpy, winid, x, y );
    QMoveEvent e( r.topLeft(), oldp );
    QApplication::sendEvent( this, &e );	// send move event immediately
}


/*----------------------------------------------------------------------------
 \overload void QWidget::resize( const QSize & )
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  Resizes the widget to size \e w by \e h pixels.

  A \link resizeEvent() resize event\endlink is generated at once.

  This function is virtual, and all other overloaded resize()
  implementations call it.

  \warning If you call resize() or setGeometry() from resizeEvent(),
  you may see infinite recursion.

  \sa move(), setGeometry(), resizeEvent()
 ----------------------------------------------------------------------------*/

void QWidget::resize( int w, int h )
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
    if ( testWFlags(WType_TopLevel) ) {
	XSizeHints size_hints;			// tell window manager
	size_hints.flags = PSize;
	size_hints.width = w;
	size_hints.height = h;
	do_size_hints( dpy, winid, extra, &size_hints );
    }
    XResizeWindow( dpy, winid, w, h );
    QResizeEvent e( s, olds );
    QApplication::sendEvent( this, &e );	// send resize event immediatly
}


/*----------------------------------------------------------------------------
 \overload void QWidget::setGeometry( const QRect & )
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  Changes the widget geometry to \e w by \e h, positioned at
  \e x,y in its parent widget.

  A \link resizeEvent() resize event\endlink and a
  \link moveEvent() move event\endlink is generated immediately.

  This function is virtual, and all other overloaded setGeometry()
  implementations call it.

  \warning If you call setGeometry() from resizeEvent() or moveEvent(),
  you may see infinite recursion.

  \sa move(), resize(), moveEvent(), resizeEvent()
 ----------------------------------------------------------------------------*/

void QWidget::setGeometry( int x, int y, int w, int h )
{
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
    if ( testWFlags(WType_TopLevel) ) {
	XSizeHints size_hints;			// tell window manager
	size_hints.flags = USPosition | USSize;
	size_hints.x = x;
	size_hints.y = y;
	size_hints.width = w;
	size_hints.height = h;
	do_size_hints( dpy, winid, extra, &size_hints );
    }
    XMoveResizeWindow( dpy, winid, x, y, w, h );
    QResizeEvent e1( r.size(), olds );
    QApplication::sendEvent( this, &e1 );	// send resize event
    QMoveEvent e2( r.topLeft(), oldp );
    QApplication::sendEvent( this, &e2 );	// send move event
}


/*----------------------------------------------------------------------------
  Sets the minimum size of the widget to \e w by \e h pixels.  The
  user will not be able to resize the window to a smaller size.	 The
  programmer may, however.

  Note that while you can set the minimum size for all widgets, it has
  no effect except for top-level widgets.

  \sa minimumSize(), setMaximumSize(), setSizeIncrement(), size()
 ----------------------------------------------------------------------------*/

void QWidget::setMinimumSize( int w, int h )
{
    createExtra();
    extra->minw = w;
    extra->minh = h;
    if ( testWFlags(WType_TopLevel) ) {
	XSizeHints size_hints;
	size_hints.flags = 0;
	do_size_hints( dpy, winid, extra, &size_hints );
    }
}

/*----------------------------------------------------------------------------
  Sets the maximum size of the widget to \e w by \e h pixels.
  The user will not be able to resize the window to a larger size.  The
  programmer may, however.

  Note that while you can set the maximum size for all widgets, it has
  no effect except for top-level widgets.

  \sa maximumSize(), setMinimumSize(), setSizeIncrement(), size()
 ----------------------------------------------------------------------------*/

void QWidget::setMaximumSize( int w, int h )
{
    createExtra();
    extra->maxw = w;
    extra->maxh = h;
    if ( testWFlags(WType_TopLevel) ) {
	XSizeHints size_hints;
	size_hints.flags = 0;
	do_size_hints( dpy, winid, extra, &size_hints );
    }
}

/*----------------------------------------------------------------------------
  Sets the size increment of the widget.  When the user resizes the
  window, the size will move in steps of \e w pixels horizontally and
  \e h pixels vertically.

  Note that while you can set the size increment for all widgets, it
  has no effect except for top-level widgets.

  \warning The size increment has no effect under Windows.

  \sa sizeIncrement(), setMinimumSize(), setMaximumSize(), size()
 ----------------------------------------------------------------------------*/

void QWidget::setSizeIncrement( int w, int h )
{
    createExtra();
    extra->incw = w;
    extra->inch = h;
    if ( testWFlags(WType_TopLevel) ) {
	XSizeHints size_hints;
	size_hints.flags = 0;
	do_size_hints( dpy, winid, extra, &size_hints );
    }
}

/*----------------------------------------------------------------------------
  \overload void QWidget::erase()
  This version erases the entire widget.
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \overload void QWidget::erase( const QRect &r )
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  Erases the specified area \e (x,y,w,h) in the widget without generating
  a \link paintEvent() paint event\endlink.

  If \e w is negative, it is replaced with <code>width() - x</code>.
  If \e h is negative, it is replaced width <code>height() - y</code>.

  Child widgets are not affected.

  \sa repaint()
 ----------------------------------------------------------------------------*/

void QWidget::erase( int x, int y, int w, int h )
{
    if ( w < 0 )
	w = crect.width()  - x;
    if ( h < 0 )
	h = crect.height() - y;
    if ( w != 0 && h != 0 )
	XClearArea( dpy, winid, x, y, w, h, FALSE );
}

/*----------------------------------------------------------------------------
  Scrolls the contents of the widget \e dx pixels rightwards and \e dy
  pixels downwards.  If \e dx/dy is negative, the scroll direction is
  leftwards/upwards.  Child widgets are moved accordingly.

  The areas of the widget that are exposed will be erased and a
  \link paintEvent() paint event\endlink will be generated.

  \warning If you call scroll() in a function which may itself be
  called from the moveEvent() or paintEvent() of a direct child of the
  widget being scrolled, you may see infinite recursion.

  \sa erase(), bitBlt()
 ----------------------------------------------------------------------------*/

void QWidget::scroll( int dx, int dy )
{
    int x1, y1, x2, y2, w=crect.width(), h=crect.height();
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
    XCopyArea( dpy, winid, winid, qt_xget_readonly_gc(), x1, y1, w, h, x2, y2);
    if ( children() ) {				// scroll children
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
    if ( dx ) {
	x1 = x2 == 0 ? w : 0;
	XClearArea( dpy, winid, x1, 0, crect.width()-w, crect.height(), TRUE);
    }
    if ( dy ) {
	y1 = y2 == 0 ? h : 0;
	XClearArea( dpy, winid, 0, y1, crect.width(), crect.height()-h, TRUE);
    }
}


/*----------------------------------------------------------------------------
  \overload void QWidget::drawText( const QPoint &pos, const char *str )
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  Writes \e str at position \e x,y.

  The \e y position is the base line position of the text.  The text is
  drawn using the current font and the current foreground color.

  This function is provided for convenience.  You will generally get
  more flexible results and often higher speed by using a a \link
  QPainter painter\endlink instead.

  \sa setFont(), foregroundColor(), QPainter::drawText()
 ----------------------------------------------------------------------------*/

void QWidget::drawText( int x, int y, const char *str )
{
    if ( testWFlags(WState_Visible) ) {
	QPainter paint;
	paint.begin( this );
	paint.drawText( x, y, str );
	paint.end();
    }
}


/*----------------------------------------------------------------------------
  Internal implementation of the virtual QPaintDevice::metric() function.

  Use the QPaintDeviceMetrics class instead.
 ----------------------------------------------------------------------------*/

int QWidget::metric( int m ) const
{
    int val;
    if ( m == PDM_WIDTH || m == PDM_HEIGHT ) {
	if ( m == PDM_WIDTH )
	    val = crect.width();
	else
	    val = crect.height();
    } else {
	int scr = qt_xscreen();
	switch ( m ) {
	    case PDM_WIDTHMM:
		val = (DisplayWidthMM(dpy,scr)*crect.width())/
		      DisplayWidth(dpy,scr);
		break;
	    case PDM_HEIGHTMM:
		val = (DisplayHeightMM(dpy,scr)*crect.height())/
		      DisplayHeight(dpy,scr);
		break;
	    case PDM_NUMCOLORS:
		val = DisplayCells(dpy,scr);
		break;
	    case PDM_DEPTH:
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
