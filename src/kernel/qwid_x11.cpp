/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qwid_x11.cpp#167 $
**
** Implementation of QWidget and QWindow classes for X11
**
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

RCSTAG("$Id: //depot/qt/main/src/kernel/qwid_x11.cpp#167 $");


void qt_enter_modal( QWidget * );		// defined in qapp_x11.cpp
void qt_leave_modal( QWidget * );		// --- "" ---
bool qt_modal_state();				// --- "" ---
void qt_open_popup( QWidget * );		// --- "" ---
void qt_close_popup( QWidget * );		// --- "" ---
void qt_updated_rootinfo();


extern bool qt_nograb();

static QWidget *mouseGrb    = 0;
static QWidget *keyboardGrb = 0;


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


/*!
  \internal
  Creates a new widget window if \e window is null, otherwise sets the
  widget's window.
*/

void QWidget::create( WId window )
{
    if ( testWFlags(WState_Created) && window == 0 )
	return;
    setWFlags( WState_Created );		// set created flag

    if ( !parentWidget() )
	setWFlags( WType_TopLevel );		// top-level widget

    static int sw = -1, sh = -1;		// screen size

    int	   scr	    = qt_xscreen();
    bool   topLevel = testWFlags(WType_TopLevel);
    bool   popup    = testWFlags(WType_Popup);
    bool   modal    = testWFlags(WType_Modal);
    bool   desktop  = testWFlags(WType_Desktop);
    Window root_win = RootWindow(dpy,scr);
    Window parentw, destroyw = 0;
    WId	   id;

    if ( popup )				// a popup is a tool window
	setWFlags(WStyle_Tool);

    if ( sw < 0 ) {				// get the screen size
	sw = DisplayWidth(dpy,scr);
	sh = DisplayHeight(dpy,scr);
    }

    bg_col = pal.normal().background();		// default background color

    if ( modal || popup || desktop ) {		// these are top-level, too
	topLevel = TRUE;
	setWFlags( WType_TopLevel );
    }

    if ( desktop ) {				// desktop widget
	frect.setRect( 0, 0, sw, sh );
	modal = popup = FALSE;			// force these flags off
    } else if ( topLevel ) {			// calc pos/size from screen
	frect.setRect( sw/4, 3*sh/10, sw/2, 4*sh/10 );
    } else {					// child widget
	frect.setRect( 0, 0, 100, 30 );
    }
    crect = frect;				// default client rect

    if ( topLevel )
	parentw = root_win;
    else
	parentw = parentWidget()->winId();

    XSetWindowAttributes wattr;

    if ( window ) {				// override the old window
	destroyw = winid;
	id = window;
	setWinId( window );
    } else if ( desktop ) {			// desktop widget
	id = parentw;				// id = root window
	QWidget *otherDesktop = find( id );	// is there another desktop?
	if ( otherDesktop && otherDesktop->testWFlags(WPaintDesktop) ) {
	    otherDesktop->setWinId( 0 );	// remove id from widget mapper
	    setWinId( id );			// make sure otherDesktop is
	    otherDesktop->setWinId( id );	//   found first
	} else {
	    setWinId( id );
	}
    } else {
	if ( x11DefaultVisual() && x11DefaultColormap() ) {
	    id = XCreateSimpleWindow( dpy, parentw,
				      frect.left(), frect.top(),
				      frect.width(), frect.height(),
				      0,
				      black.pixel(),
				      bg_col.pixel() );
	} else {
	    wattr.background_pixel = bg_col.pixel();
	    wattr.border_pixel = black.pixel();		
	    wattr.colormap = (Colormap)x11Colormap();
	    id = XCreateWindow( dpy, parentw,
				frect.left(), frect.top(),
				frect.width(), frect.height(),
				0, x11Depth(), InputOutput,
				(Visual*)x11Visual(),
				CWBackPixel|CWBorderPixel|CWColormap,
				&wattr );
	}
	setWinId( id );				// set widget id/handle + hd
    }

    if ( topLevel && !(desktop || popup || modal) ) {
	if ( testWFlags(WStyle_Customize) ) {	// customize top-level widget
	    ulong wattr_mask = 0;
	    if ( testWFlags(WStyle_NormalBorder) ) {
		;				// ok, we already have it
	    } else {
		if ( testWFlags(WStyle_DialogBorder) ) {
		    XSetTransientForHint( dpy, id, root_win );
		} else {			// no border
		    wattr.override_redirect = TRUE;
		    wattr_mask |= CWOverrideRedirect;
		}
	    }
	    if ( testWFlags(WStyle_Tool) ) {
		wattr.save_under = TRUE;
		wattr_mask |= CWSaveUnder;
	    }
	    if ( wattr_mask )
		XChangeWindowAttributes( dpy, id, wattr_mask, &wattr );
	} else {				// normal top-level widget
	    setWFlags( WStyle_NormalBorder | WStyle_Title | WStyle_SysMenu |
		       WStyle_MinMax );
	}
    }
    if ( popup ) {				// popup widget
	XSetTransientForHint( dpy, id, parentw );
	wattr.override_redirect = TRUE;
	wattr.save_under = TRUE;
	XChangeWindowAttributes( dpy, id, CWOverrideRedirect | CWSaveUnder,
				 &wattr );
    } else if ( topLevel && !desktop ) {	// top-level widget
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
		XSetTransientForHint( dpy, id, root_win );
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
    }
    if ( testWFlags(WResizeNoErase) ) {
	wattr.bit_gravity = NorthWestGravity;	// don't erase when resizing
	XChangeWindowAttributes( dpy, id, CWBitGravity, &wattr );
    }
    setWFlags( WState_TrackMouse );
    setMouseTracking( FALSE );			// also sets event mask
    if ( desktop ) {
	setWFlags( WState_Visible );
    } else if ( topLevel ) {			// set X cursor
	QCursor *oc = QApplication::overrideCursor();
	XDefineCursor( dpy, winid, oc ? oc->handle() : curs.handle() );
	setWFlags( WCursorSet );
    }
    if ( destroyw )
	XDestroyWindow( dpy, destroyw );
}


/*!
  \internal
  Creates the widget's window. Equivalent with create(0).
  This function is usually called from the QWidget constructor.
*/

bool QWidget::create()
{
    create( 0 );
    return TRUE;
}


/*!
  \internal
  Destroys the widget window and frees up window system resources.
  Usually called from the QWidget destructor.
*/

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
	if ( mouseGrb == this )
	    releaseMouse();
	if ( keyboardGrb == this )
	    releaseKeyboard();
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


/*!
  This function is provided in case a widget should feel \e really
  bad, regret that it was even born.

  It gives the widget a fresh start, new \e parent, new widget flags
  (\e f, but as usual, use 0) at a new position in its new parent (\e p).

  If \e showIt is TRUE, show() is called once the widget has been
  recreated.

  \sa getWFlags()
*/

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
    if ( parent ) {				// insert into new parent
	parentObj = parent;			// avoid insertChild warning
	parent->insertChild( this );
    }
    bool     enable = isEnabled();		// remember status
    QSize    s	    = size();
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


/*!
  Translates the widget coordinate \e pos to global screen coordinates.
  \sa mapFromGlobal()
*/

QPoint QWidget::mapToGlobal( const QPoint &pos ) const
{
    int	   x, y;
    Window child;
    XTranslateCoordinates( dpy, winid, QApplication::desktop()->winId(),
			   pos.x(), pos.y(), &x, &y, &child );
    return QPoint( x, y );
}

/*!
  Translates the global screen coordinate \e pos to widget coordinates.
  \sa mapToGlobal()
*/

QPoint QWidget::mapFromGlobal( const QPoint &pos ) const
{
    int	   x, y;
    Window child;
    XTranslateCoordinates( dpy, QApplication::desktop()->winId(), winid,
			   pos.x(), pos.y(), &x, &y, &child );
    return QPoint( x, y );
}


/*!
  Sets the background color of this widget.

  The background color is semi-independent of the widget color group.
  Setting a new palette overwrites the background color, but setting
  the background color does not change the palette in any way.

  The window system clears the widget to the background color just before
  sending a paint event. This generally causes flicker if the widget does
  not use colorGroup().background() as background color.  A widget which
  uses colorGroup().base() as background color can avoid flicker by doing:

  \code
    setBackgroundColor( colorGroup().base() );
  \endcode

  If you want to change the color scheme of a widget, the setPalette()
  function is better suited.  Here is how to set \e thatWidget to use a
  light green (RGB value 80, 255, 80) as background color, with shades
  of green used for all the 3D effects:

  \code
    thatWidget->setPalette( QPalette( QColor(80, 255, 80) ) );
  \endcode

  You can also use QApplication::setPalette() if you want to change
  the color scheme of your entire application, or of all new widgets.
  
  \sa backgroundColor(), backgroundColorChange(), setPalette(),
  setBackgroundPixmap(), QApplication::setPalette()
*/

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

/*!
  Sets the background pixmap of the widget to \e pixmap.

  The background pixmap is tiled.  Some widgets (e.g. QLineEdit) do
  not work well with a background pixmap.

  \sa backgroundPixmap(), backgroundPixmapChange(), setBackgroundColor()
*/

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
    } else {
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


/*!
  Sets the widget cursor shape to \e cursor.

  The mouse cursor will assume this shape when it's over this widget.
  See a list of cursor shapes in the QCursor documentation.

  An editor widget would for example use an I-beam cursor:
  \code
    setCursor( ibeamCursor );
  \endcode

  \sa cursor(), QApplication::setOverrideCursor()
*/

void QWidget::setCursor( const QCursor &cursor )
{
    curs = cursor;
    QCursor *oc = QApplication::overrideCursor();
    XDefineCursor( dpy, winid, oc ? oc->handle() : curs.handle() );
    setWFlags( WCursorSet );
    XFlush( dpy );
}


/*!
  Sets the window caption (title).
  \sa caption(), setIcon(), setIconText()
*/

void QWidget::setCaption( const char *caption )
{
    if ( extra )
	delete [] extra->caption;
    else
	createExtra();
    extra->caption = qstrdup( caption );
    XStoreName( dpy, winId(), extra->caption );
}

/*!
  Sets the window icon pixmap.
  \sa icon(), setIconText(), setCaption()
*/

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

/*!
  Sets the text of the window's icon to \e iconText.
  \sa iconText(), setIcon(), setCaption()
*/

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
    if ( enable == testWFlags(WState_TrackMouse) )
	return;
    uint m;
    if ( enable ) {
	m = PointerMotionMask;
	setWFlags( WState_TrackMouse );
    } else {
	m = 0;
	clearWFlags( WState_TrackMouse );
    }
    if ( testWFlags(WType_Desktop) ) {		// desktop widget?
	if ( testWFlags(WPaintDesktop) )	// get desktop paint events
	    XSelectInput( dpy, winid, ExposureMask );
    } else {
	XSelectInput( dpy, winid,		// specify events
		      m | stdWidgetEventMask );
    }
}


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

/*!
  Releases the mouse grab.

  \sa grabMouse(), grabKeyboard(), releaseKeyboard()
*/

void QWidget::releaseMouse()
{
    if ( !qt_nograb() && mouseGrb == this ) {
	XUngrabPointer( dpy, CurrentTime );
	XFlush( dpy );
	mouseGrb = 0;
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
    if ( !qt_nograb() ) {
	if ( keyboardGrb )
	    keyboardGrb->releaseKeyboard();
	XGrabKeyboard( dpy, winid, TRUE, GrabModeAsync, GrabModeAsync,
		       CurrentTime );
	keyboardGrb = this;
    }
}

/*!
  Releases the keyboard grab.

  \sa grabKeyboard(), grabMouse(), releaseMouse()
*/

void QWidget::releaseKeyboard()
{
    if ( !qt_nograb() && keyboardGrb == this ) {
	XUngrabKeyboard( dpy, CurrentTime );
	keyboardGrb = 0;
    }
}


/*!
  Returns a pointer to the widget that is currently grabbing the
  mouse input.

  If no widget in this application is currently grabbing the mouse, 0 is
  returned.

  \sa grabMouse(), keyboardGrabber()
*/

QWidget *QWidget::mouseGrabber()
{
    return mouseGrb;
}

/*!
  Returns a pointer to the widget that is currently grabbing the
  keyboard input.

  If no widget in this application is currently grabbing the keyboard, 0
  is returned.

  \sa grabMouse(), mouseGrabber()
*/

QWidget *QWidget::keyboardGrabber()
{
    return keyboardGrb;
}


/*!
  Returns TRUE if the top-level widget containing this widget is the
  active window.

  \sa setActiveWindow(), topLevelWidget()
*/

bool QWidget::isActiveWindow() const
{
    Window win;
    int revert;
    XGetInputFocus( dpy, &win, &revert );
    QWidget *w = find( win );
    return w && w->topLevelWidget() == topLevelWidget();
}


/*!
  Sets the top-level widget containing this widget to be the active
  window.

  An active window is a top-level window that has the keyboard input
  focus.

  This function performs the same operation as clicking the mouse on
  the title bar of a top-level window.

  \sa isActiveWindow(), topLevelWidget()
*/

void QWidget::setActiveWindow()
{
    QWidget *tlw = topLevelWidget();
    if ( tlw->isVisible() )
	XSetInputFocus( dpy, tlw->winId(), RevertToNone, CurrentTime);
}


/*!
  Updates the widget unless updates are disabled or the widget is hidden.

  Updating the widget will erase the widget contents and generate a paint
  event from the window system. The paint event is processed after the
  program has returned to the main event loop.

  \sa repaint(), paintEvent(), setUpdatesEnabled(), erase()
*/

void QWidget::update()
{
    if ( (flags & (WState_Visible|WState_BlockUpdates)) == WState_Visible )
	XClearArea( dpy, winid, 0, 0, 0, 0, TRUE );
}

/*!
  Updates a rectangle (\e x, \e y, \e w, \e h) inside the widget
  unless updates are disabled or the widget is hidden.

  Updating the widget erases the widget area \e (x,y,w,h), which in turn
  generates a paint event from the window system. The paint event is
  processed after the program has returned to the main event loop.

  If \e w is negative, it is replaced with <code>width() - x</code>.
  If \e h is negative, it is replaced width <code>height() - y</code>.

  \sa repaint(), paintEvent(), setUpdatesEnabled(), erase()
*/

void QWidget::update( int x, int y, int w, int h )
{
    if ( (flags & (WState_Visible|WState_BlockUpdates)) == WState_Visible ) {
	if ( w < 0 )
	    w = crect.width()  - x;
	if ( h < 0 )
	    h = crect.height() - y;
	if ( w != 0 && h != 0 )
	    XClearArea( dpy, winid, x, y, w, h, TRUE );
    }
}


/*! \overload void QWidget::repaint( bool erase )

  This version repaints the entire widget.
*/

/*!
  Repaints the widget directly by calling paintEvent() directly,
  unless updates are disabled or the widget is hidden.

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
*/

void QWidget::repaint( int x, int y, int w, int h, bool erase )
{
    if ( (flags & (WState_Visible|WState_BlockUpdates)) == WState_Visible ) {
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

/*!
  \overload void QWidget::repaint( const QRect &r, bool erase )
*/


/*!
  Shows the widget and its child widgets.

  If its size or position has changed, Qt guarantees that a widget gets
  move and resize events just before the widget is shown.

  \sa hide(), iconify(), isVisible()
*/

void QWidget::show()
{
    if ( testWFlags(WState_Visible) )
	return;

    if ( extra ) {
	int w = crect.width();
	int h = crect.height();
	if ( w < extra->minw || h < extra->minh ||
	     w > extra->maxw || h > extra->maxh ) {
	    w = QMAX( extra->minw, QMIN( w, extra->maxw ));
	    h = QMAX( extra->minh, QMIN( h, extra->maxh ));
	    resize( w, h ); // may defer resize event :)
	}
    }

    sendDeferredEvents( FALSE, FALSE );

    if ( children() ) {
	QObjectListIt it(*children());
	register QObject *object;
	QWidget *widget;
	while ( it ) {				// show all widget children
	    object = it.current();		//   (except popups)
	    ++it;
	    if ( object->isWidgetType() ) {
		widget = (QWidget*)object;
		if ( !widget->testWFlags(WState_DoHide) )
		    widget->show();
	    }
	}
    }
    if ( testWFlags(WType_Popup) )
	raise();
    XMapWindow( dpy, winid );
    setWFlags( WState_Visible );
    clearWFlags( WState_DoHide );
    if ( testWFlags(WType_Modal) )
	qt_enter_modal( this );
    else if ( testWFlags(WType_Popup) )
	qt_open_popup( this );
}


/*!
  Hides the widget.
  \sa show(), iconify(), isVisible()
*/

void QWidget::hide()
{
    if ( testWFlags(WFocusSet) )
	clearFocus();
    setWFlags( WState_DoHide );
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
    cancelMove();
    cancelResize();
}


/*!
  Iconifies the widget.

  Calling this function has no effect for other than \link isTopLevel()
  top-level widgets\endlink.

  \sa show(), hide(), isVisible()
*/

void QWidget::iconify()
{
    if ( testWFlags(WType_TopLevel) )
	XIconifyWindow( dpy, winid, qt_xscreen() );
}


/*!
  Raises this widget to the top of the parent widget's stack.

  If there are any siblings of this widget that overlap it on the screen,
  this widget will be in front of its siblings afterwards.

  \sa lower()
*/

void QWidget::raise()
{
    QWidget *p = parentWidget();
    if ( p && p->childObjects && p->childObjects->findRef(this) >= 0 )
	p->childObjects->insert( 0, p->childObjects->take() );
    XRaiseWindow( dpy, winid );
}

/*!
  Lowers the widget to the bottom of the parent widget's stack.

  If there are siblings of this widget that overlap it on the screen, this
  widget will be obscured by its siblings afterwards.

  \sa raise()
*/

void QWidget::lower()
{
    QWidget *p = parentWidget();
    if ( p && p->childObjects && p->childObjects->findRef(this) >= 0 )
	p->childObjects->append( p->childObjects->take() );
    XLowerWindow( dpy, winid );
}


/*
  The global variable qwidget_tlw_gravity defines the window gravity of
  the next top level window to be created. We do this when setting the
  main widget's geometry and the "-geometry" command line option contains
  a negative position.
*/

int qwidget_tlw_gravity = 1;

static void do_size_hints( Display *dpy, WId winid, QWExtra *x, XSizeHints *s )
{
    if ( x ) {
	if ( x->minw > 0 || x->minh > 0 ) {	// add minimum size hints
	    s->flags |= PMinSize;
	    s->min_width  = x->minw;
	    s->min_height = x->minh;
	}
	if ( x->maxw < QCOORD_MAX || x->maxh < QCOORD_MAX ) {
	    s->flags |= PMaxSize;		// add maximum size hints
	    s->max_width  = x->maxw;
	    s->max_height = x->maxh;
	}
	if ( x->incw > 0 || x->inch > 0 ) {	// add resize increment hints
	    s->flags |= PResizeInc | PBaseSize;
	    s->width_inc = x->incw;
	    s->height_inc = x->inch;
	    s->base_width = 0;
	    s->base_height = 0;
	}
    }
    s->flags |= PWinGravity;
    s->win_gravity = qwidget_tlw_gravity;	// usually NorthWest (1)
    qwidget_tlw_gravity = 1;			// reset in case it was set
    XSetWMNormalHints( dpy, winid, s );
}


/*!
  \overload void QWidget::move( const QPoint & )
*/

/*!
  Moves the widget to the position \e (x,y) relative to the parent widget.

  A \link moveEvent() move event\endlink is generated immediately if
  the widget is visible. If the widget is invisible, the move event
  is generated when show() is called.

  This function is virtual, and all other overloaded move()
  implementations call it.

  \warning If you call move() or setGeometry() from moveEvent(), you
  may see infinite recursion.

  \sa pos(), resize(), setGeometry(), moveEvent()
*/

void QWidget::move( int x, int y )
{
    QPoint p(x,y);
    QPoint oldp = frameGeometry().topLeft();
    QRect  r = frect;
    if ( testWFlags(WType_Desktop) )
	return;
    r.moveTopLeft( p );
    setFRect( r );

    if ( !isVisible() ) {
	deferMove( oldp );
	return;
    }
    cancelMove();
    internalMove( x, y );
    QMoveEvent e( r.topLeft(), oldp );
    QApplication::sendEvent( this, &e );	// send move event immediately
}


void QWidget::internalMove( int x, int y )
{
    if ( testWFlags(WType_TopLevel) ) {
	XSizeHints size_hints;			// tell window manager
	size_hints.flags = PPosition;
	size_hints.x = x;
	size_hints.y = y;
	do_size_hints( dpy, winid, extra, &size_hints );
    }
    XMoveWindow( dpy, winid, x, y );
}


/*!
  \overload void QWidget::resize( const QSize & )
*/

/*!
  Resizes the widget to size \e w by \e h pixels.

  A \link resizeEvent() resize event\endlink is generated immediately if
  the widget is visible. If the widget is invisible, the resize event
  is generated when show() is called.

  The size is adjusted if it is outside the \link setMinimumSize()
  minimum\endlink or \link setMaximumSize() maximum\endlink widget size.

  This function is virtual, and all other overloaded resize()
  implementations call it.

  \warning If you call resize() or setGeometry() from resizeEvent(),
  you may see infinite recursion.

  \sa size(), move(), setGeometry(), resizeEvent(),
  minimumSize(),  maximumSize()
*/

void QWidget::resize( int w, int h )
{
    if ( testWFlags(WType_Desktop) )
	return;
    if ( w < 1 )				// invalid size
	w = 1;
    if ( h < 1 )
	h = 1;
    if ( extra ) {				// any size restrictions?
	w = QMIN(w,extra->maxw);
	h = QMIN(h,extra->maxh);
	w = QMAX(w,extra->minw);
	h = QMAX(h,extra->minh);
    }
    QRect r = crect;
    QSize s(w,h);
    QSize olds = size();
    r.setSize( s );
    setCRect( r );

    if ( !isVisible() ) {
	deferResize( olds );
	return;
    }
    cancelResize();
    internalResize( w, h );
    QResizeEvent e( s, olds );
    QApplication::sendEvent( this, &e );	// send resize event immediatly
}


void QWidget::internalResize( int w, int h )
{
    if ( testWFlags(WType_TopLevel) ) {
	XSizeHints size_hints;			// tell window manager
	size_hints.flags = PSize;
	size_hints.width = w;
	size_hints.height = h;
	do_size_hints( dpy, winid, extra, &size_hints );
    }
    XResizeWindow( dpy, winid, w, h );
}


/*!
  \overload void QWidget::setGeometry( const QRect & )
*/

/*!
  Sets the widget geometry to \e w by \e h, positioned at \e x,y in its
  parent widget.

  A \link resizeEvent() resize event\endlink and a \link moveEvent() move
  event\endlink are generated immediately if the widget is visible. If the
  widget is invisible, the events are generated when show() is called.

  The size is adjusted if it is outside the \link setMinimumSize()
  minimum\endlink or \link setMaximumSize() maximum\endlink widget size.

  This function is virtual, and all other overloaded setGeometry()
  implementations call it.

  \warning If you call setGeometry() from resizeEvent() or moveEvent(),
  you may see infinite recursion.

  \sa geometry(), move(), resize(), moveEvent(), resizeEvent(),
  minimumSize(), maximumSize()
*/

void QWidget::setGeometry( int x, int y, int w, int h )
{
    if ( w < 1 )				// invalid size
	w = 1;
    if ( h < 1 )
	h = 1;
    if ( extra ) {				// any size restrictions?
	w = QMIN(w,extra->maxw);
	h = QMIN(h,extra->maxh);
	w = QMAX(w,extra->minw);
	h = QMAX(h,extra->minh);
    }
    QPoint oldp = frameGeometry().topLeft();
    QSize  olds = size();
    QRect  r( x, y, w, h );
    if ( testWFlags(WType_Desktop) )
	return;

    setCRect( r );

    if ( !isVisible() ) {
	deferMove( oldp );
	deferResize( olds );
	return;
    }

    cancelMove();
    cancelResize();
    internalSetGeometry( x, y, w, h );
    QResizeEvent e1( r.size(), olds );
    QApplication::sendEvent( this, &e1 );	// send resize event
    QMoveEvent e2( r.topLeft(), oldp );
    QApplication::sendEvent( this, &e2 );	// send move event
}


void QWidget::internalSetGeometry( int x, int y, int w, int h )
{
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
}


/*!
  \overload void QWidget::setMinimumSize( const QSize &size )
*/

/*!
  Sets the minimum size of the widget to \e w by \e h pixels.

  The widget cannot be resized to a smaller size than the minimum widget
  size. The widget's size is forced to the minimum size if the current
  size is smaller.

  \sa minimumSize(), setMaximumSize(), setSizeIncrement(), resize(), size()
*/

void QWidget::setMinimumSize( int w, int h )
{
#if defined(CHECK_RANGE)
    if ( w < 0 || h < 0 )
	warning("QWidget::setMinimumSize: The smallest allowed size is (0,0)");
#endif
    createExtra();
    extra->minw = w;
    extra->minh = h;
    int minw = QMAX(w,crect.width());
    int minh = QMAX(h,crect.height());
    if ( isVisible() && (minw > w || minh > h) )
	resize( minw, minh );
    if ( testWFlags(WType_TopLevel) ) {
	XSizeHints size_hints;
	size_hints.flags = 0;
	do_size_hints( dpy, winid, extra, &size_hints );
    }
}

/*!
  \overload void QWidget::setMaximumSize( const QSize &size )
*/

/*!
  Sets the maximum size of the widget to \e w by \e h pixels.

  The widget cannot be resized to a larger size than the maximum widget
  size. The widget's size is forced to the maximum size if the current
  size is greater.

  \sa maximumSize(), setMinimumSize(), setSizeIncrement(), resize(), size()
*/

void QWidget::setMaximumSize( int w, int h )
{
#if defined(CHECK_RANGE)
    if ( w > QCOORD_MAX || h > QCOORD_MAX )
	warning("QWidget::setMaximumSize: The largest allowed size is (%d,%d)",
		 QCOORD_MAX, QCOORD_MAX );
#endif
    createExtra();
    extra->maxw = w;
    extra->maxh = h;
    int maxw = QMIN(w,crect.width());
    int maxh = QMIN(h,crect.height());
    if ( isVisible() && (maxw < w || maxh < h) )
	resize( maxw, maxh );
    if ( testWFlags(WType_TopLevel) ) {
	XSizeHints size_hints;
	size_hints.flags = 0;
	do_size_hints( dpy, winid, extra, &size_hints );
    }
}

/*!
  Sets the size increment of the widget.  When the user resizes the
  window, the size will move in steps of \e w pixels horizontally and
  \e h pixels vertically.

  Note that while you can set the size increment for all widgets, it
  has no effect except for top-level widgets.

  \warning The size increment has no effect under Windows, and may be
  disregarded by the window manager on X.

  \sa sizeIncrement(), setMinimumSize(), setMaximumSize(), resize(), size()
*/

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
/*!
  \overload void QWidget::setSizeIncrement( const QSize& )
*/

/*!
  \overload void QWidget::erase()
  This version erases the entire widget.
*/

/*!
  \overload void QWidget::erase( const QRect &r )
*/

/*!
  Erases the specified area \e (x,y,w,h) in the widget without generating
  a \link paintEvent() paint event\endlink.

  If \e w is negative, it is replaced with <code>width() - x</code>.
  If \e h is negative, it is replaced width <code>height() - y</code>.

  Child widgets are not affected.

  \sa repaint()
*/

void QWidget::erase( int x, int y, int w, int h )
{
    if ( w < 0 )
	w = crect.width()  - x;
    if ( h < 0 )
	h = crect.height() - y;
    if ( w != 0 && h != 0 )
	XClearArea( dpy, winid, x, y, w, h, FALSE );
}

/*!
  Scrolls the contents of the widget \e dx pixels rightwards and \e dy
  pixels downwards.  If \e dx/dy is negative, the scroll direction is
  leftwards/upwards.  Child widgets are moved accordingly.

  The areas of the widget that are exposed will be erased and a
  \link paintEvent() paint event\endlink will be generated.

  \warning If you call scroll() in a function which may itself be
  called from the moveEvent() or paintEvent() of a direct child of the
  widget being scrolled, you may see infinite recursion.

  \sa erase(), bitBlt()
*/

void QWidget::scroll( int dx, int dy )
{
    int x1, y1, x2, y2, w=crect.width(), h=crect.height();
    if ( dx > 0 ) {
	x1 = 0;
	x2 = dx;
	w -= dx;
    } else {
	x1 = -dx;
	x2 = 0;
	w += dx;
    }
    if ( dy > 0 ) {
	y1 = 0;
	y2 = dy;
	h -= dy;
    } else {
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


/*!
  \overload void QWidget::drawText( const QPoint &pos, const char *str )
*/

/*!
  Writes \e str at position \e x,y.

  The \e y position is the base line position of the text.  The text is
  drawn using the current font and the current foreground color.

  This function is provided for convenience.  You will generally get
  more flexible results and often higher speed by using a a \link
  QPainter painter\endlink instead.

  \sa setFont(), foregroundColor(), QPainter::drawText()
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
