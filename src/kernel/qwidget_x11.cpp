/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qwidget_x11.cpp#324 $
**
** Implementation of QWidget and QWindow classes for X11
**
** Created : 931031
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qapplication.h"
#include "qpaintdevicedefs.h"
#include "qpainter.h"
#include "qbitmap.h"
#include "qwidgetlist.h"
#include "qwidgetintdict.h"
#include "qobjectlist.h"
#include "qobjectdict.h"
#include "qaccel.h"
#include "qdragobject.h"
#include "qfocusdata.h"
#include "qabstractlayout.h"
#include "qt_x11.h"


void qt_enter_modal( QWidget * );		// defined in qapplication_x11.cpp
void qt_leave_modal( QWidget * );		// --- "" ---
bool qt_modal_state();				// --- "" ---
void qt_insert_sip( QWidget*, int, int );	// --- "" ---
int  qt_sip_count( QWidget* );			// --- "" ---
void qt_updated_rootinfo();


extern bool qt_nograb();
extern QWidget *qt_button_down;

static QWidget *mouseGrb    = 0;
static QWidget *keyboardGrb = 0;


/*****************************************************************************
  QWidget member functions
 *****************************************************************************/


extern Atom qt_wm_delete_window;		// defined in qapplication_x11.cpp
extern Atom qt_sizegrip;			// defined in qapplication_x11.cpp
extern Atom qt_wm_client_leader;		// defined in qapplication_x11.cpp

const uint stdWidgetEventMask =			// X event mask
	(uint)(
	    KeyPressMask | KeyReleaseMask |
	    ButtonPressMask | ButtonReleaseMask |
	    KeymapStateMask |
	    ButtonMotionMask |
	    EnterWindowMask | LeaveWindowMask |
	    FocusChangeMask |
	    ExposureMask |
	    StructureNotifyMask | SubstructureRedirectMask
	);

const uint stdDesktopEventMask =			// X event mask
	(uint)(
	    KeyPressMask | KeyReleaseMask |
	    KeymapStateMask |
	    EnterWindowMask | LeaveWindowMask |
	    FocusChangeMask | PropertyChangeMask
	);


/*
  The qt_ functions below are implemented in qwidgetcreate_x11.cpp.
*/

Window qt_XCreateWindow( const QWidget *creator,
			 Display *display, Window parent,
			 int x, int y, uint w, uint h,
			 int borderwidth, int depth,
			 uint windowclass, Visual *visual,
			 ulong valuemask, XSetWindowAttributes *attributes );
Window qt_XCreateSimpleWindow( const QWidget *creator,
			       Display *display, Window parent,
			       int x, int y, uint w, uint h, int borderwidth,
			       ulong border, ulong background );
void qt_XDestroyWindow( const QWidget *destroyer,
			Display *display, Window window );



/*!
  Creates a new widget window if \a window is null, otherwise sets the
  widget's window to \a window.

  Initializes the window (sets the geometry etc.) if \a initializeWindow
  is TRUE.  If \a initializeWindow is FALSE, no initialization is
  performed.  This parameter makes only sense if \a window is a valid
  window.

  Destroys the old window if \a destroyOldWindow is TRUE.  If \a
  destroyOldWindow is FALSE, you are responsible for destroying
  the window yourself (using platform native code).

  The QWidget constructor calls create(0,TRUE,TRUE) to create a window for
  this widget.
*/

void QWidget::create( WId window, bool initializeWindow, bool destroyOldWindow)
{
    if ( testWState(QWS_Created) && window == 0 )
	return;
    setWState( QWS_Created );			// set created flag
    clearWState( QWS_USPositionX );
    clearWState(QWS_DND);

    if ( !parentWidget() )
	setWFlags( WType_TopLevel );		// top-level widget

    static int sw = -1, sh = -1;		// screen size

    Display *dpy = x11Display();
    int	     scr = x11Screen();

    bool   topLevel = testWFlags(WType_TopLevel);
    bool   popup    = testWFlags(WType_Popup);
    bool   modal    = testWFlags(WType_Modal);
    bool   desktop  = testWFlags(WType_Desktop);
    Window root_win = RootWindow(dpy,scr);
    Window parentw, destroyw = 0;
    WId	   id;

    if ( !window )				// always initialize
	initializeWindow = TRUE;

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
	modal = popup = FALSE;			// force these flags off
	crect.setRect( 0, 0, sw, sh );
    } else if ( topLevel ) {			// calc pos/size from screen
	crect.setRect( sw/4, 3*sh/10, sw/2, 4*sh/10 );
    } else {					// child widget
	crect.setRect( 0, 0, 100, 30 );
    }
    fpos = crect.topLeft();			// default frame rect

    parentw = topLevel ? root_win : parentWidget()->winId();

    XSetWindowAttributes wsa;

    if ( window ) {				// override the old window
	if ( destroyOldWindow )
	    destroyw = winid;
	id = window;
	setWinId( window );
    } else if ( desktop ) {			// desktop widget
	id = (WId)parentw;			// id = root window
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
	    id = (WId)qt_XCreateSimpleWindow( this, dpy, parentw,
					 crect.left(), crect.top(),
					 crect.width(), crect.height(),
					 0,
					 black.pixel(),
					 bg_col.pixel() );
	} else {
	    wsa.background_pixel = bg_col.pixel();
	    wsa.border_pixel = black.pixel();		
	    wsa.colormap = (Colormap)x11Colormap();
	    id = (WId)qt_XCreateWindow( this, dpy, parentw,
				   crect.left(), crect.top(),
				   crect.width(), crect.height(),
				   0, x11Depth(), InputOutput,
				   (Visual*)x11Visual(),
				   CWBackPixel|CWBorderPixel|CWColormap,
				   &wsa );
	}
	setWinId( id );				// set widget id/handle + hd
    }

    if ( topLevel && !(desktop || popup || modal) ) {
	if ( testWFlags(WStyle_Customize) ) {	// customize top-level widget
	    ulong wsa_mask = 0;
	    if ( testWFlags(WStyle_NormalBorder) ) {
		;				// ok, we already have it
	    } else {
		if ( !testWFlags(WStyle_DialogBorder) ) {
		    wsa.override_redirect = TRUE;
		    wsa_mask |= CWOverrideRedirect;
		}
	    }
	    if ( testWFlags(WStyle_Tool) ) {
		wsa.save_under = TRUE;
		wsa_mask |= CWSaveUnder;
	    }
	    if ( wsa_mask && initializeWindow )
		XChangeWindowAttributes( dpy, id, wsa_mask, &wsa );
	} else {				// normal top-level widget
	    setWFlags( WStyle_NormalBorder | WStyle_Title | WStyle_SysMenu |
		       WStyle_MinMax );
	}
    }

    if ( !initializeWindow ) {
	// do no initialization
    } else if ( popup ) {			// popup widget
	XSetTransientForHint( dpy, id, parentw );
	wsa.override_redirect = TRUE;
	wsa.save_under = TRUE;
	XChangeWindowAttributes( dpy, id, CWOverrideRedirect | CWSaveUnder,
				 &wsa );
    } else if ( topLevel && !desktop ) {	// top-level widget
	QWidget *p = parentWidget();	// real parent
	if (p)
	    p = p->topLevelWidget();
	if ( testWFlags(WStyle_DialogBorder) ) {
	    if ( p )
		XSetTransientForHint( dpy, id, p->winId() );
	    else				// application-modal
		XSetTransientForHint( dpy, id, root_win );
	}

	// find the real client leader, i.e. a toplevel without parent
	while ( p && p->parentWidget()) {
	    p = p->parentWidget()->topLevelWidget();
	}
	
	XSizeHints size_hints;
	size_hints.flags = USSize | PSize | PWinGravity;
	size_hints.x = crect.left();
	size_hints.y = crect.top();
	size_hints.width = crect.width();
	size_hints.height = crect.height();
	size_hints.win_gravity = 1;		// NorthWest
	char *title = qAppName();
	XWMHints wm_hints;			// window manager hints
	wm_hints.input = True;
	wm_hints.initial_state = NormalState;
	wm_hints.flags = InputHint | StateHint;
	
	if (p) { // the real client leader (head of the group)
	    wm_hints.window_group = p->winId();
	    wm_hints.flags |= WindowGroupHint;
	}
	
	XClassHint class_hint;
	class_hint.res_name = title;		// app name and widget name
	class_hint.res_class = name() ? (char *)name() : title;
	XSetWMProperties( dpy, id, 0, 0, 0, 0, &size_hints, &wm_hints,
			  &class_hint );
	XResizeWindow( dpy, id, crect.width(), crect.height() );
	XStoreName( dpy, id, title );
	Atom protocols[1];
	protocols[0] = qt_wm_delete_window;	// support del window protocol
	XSetWMProtocols( dpy, id, protocols, 1 );
    }

    if ( initializeWindow ) {
	//    if ( testWFlags(WResizeNoErase) && initializeWindow ) {
	wsa.bit_gravity = NorthWestGravity;	// don't erase when resizing
	XChangeWindowAttributes( dpy, id, CWBitGravity, &wsa );
    }

    setWState( QWS_MouseTracking );
    setMouseTracking( FALSE );			// also sets event mask
    if ( desktop ) {
	setWState( QWS_Visible );
    } else if ( topLevel ) {			// set X cursor
	QCursor *oc = QApplication::overrideCursor();
	if ( initializeWindow )
	    XDefineCursor( dpy, winid, oc ? oc->handle() : cursor().handle() );
	setWState( QWS_OwnCursor );
    }

    if ( window ) {				// got window from outside
	XWindowAttributes a;
	XGetWindowAttributes( dpy, window, &a );
	crect.setRect( a.x, a.y, a.width, a.height );
	fpos = crect.topLeft();
	if ( a.map_state == IsUnmapped )
	    clearWState( QWS_Visible );
	else
	    setWState( QWS_Visible );
    }

    if ( destroyw )
	qt_XDestroyWindow( this, dpy, destroyw );
}


/*!
  Frees up window system resources.
  Destroys the widget window if \a destroyWindow is TRUE.

  destroy() calls itself recursively for all the child widgets,
  passing \a destroySubWindows for the \a destroyWindow parameter.
  To have more control over destruction of subwidgets,
  destroy subwidgets selectively first.

  This function is usually called from the QWidget destructor.
*/

void QWidget::destroy( bool destroyWindow, bool destroySubWindows )
{
    if ( qt_button_down == this )
	qt_button_down = 0;

    if ( testWState(QWS_Created) ) {
	clearWState( QWS_Created );
	if ( children() ) {
	    QObjectListIt it(*children());
	    register QObject *obj;
	    while ( (obj=it.current()) ) {	// destroy all widget children
		++it;
		if ( obj->isWidgetType() )
		    ((QWidget*)obj)->destroy(destroySubWindows,
					     destroySubWindows);
	    }
	}
	if ( mouseGrb == this )
	    releaseMouse();
	if ( keyboardGrb == this )
	    releaseKeyboard();
	if ( testWFlags(WType_Modal) )		// just be sure we leave modal
	    qt_leave_modal( this );
	else if ( testWFlags(WType_Popup) )
	    qApp->closePopup( this );
	if ( destroyWindow && !testWFlags(WType_Desktop) )
	    qt_XDestroyWindow( this, x11Display(), winid );
	setWinId( 0 );
    }
}


/*!
  Reparents the widget.  The widget gets a new \a parent, new widget
  flags (\a f, but as usual, use 0) at a new position in its new
  parent (\a p).

  If \a showIt is TRUE, show() is called once the widget has been
  reparent.

  If the new parent widget is in a different top-level widget, the
  reparented widget and its children are appended to the end of the
  \link setFocusPolicy() TAB chain \endlink of the new parent widget,
  in the same internal order as before.  If one of the moved widgets
  had keyboard focus, reparent() calls clearFocus() for that widget.

  If the new parent widget is in the same top-level widget as the old
  parent, reparent doesn't change the TAB order or keyboard focus.

  \sa getWFlags()
*/

void QWidget::reparent( QWidget *parent, WFlags f, const QPoint &p,
			bool showIt )
{
    extern void qPRCreate( const QWidget *, Window );
    Display *dpy = x11Display();
    WId old_winid = winid;
    if ( testWFlags(WType_Desktop) )
	old_winid = 0;
    setWinId( 0 );
    reparentFocusWidgets( parent );		// fix focus chains

    if ( parentObj ) {				// remove from parent
	parentObj->removeChild( this );
    }
    if ( parent ) {				// insert into new parent
	parentObj = parent;			// avoid insertChild warning
	parent->insertChild( this );
    } else {
	qApp->noteTopLevel(this);
    }
    bool     enable = isEnabled();		// remember status
    FocusPolicy fp = focusPolicy();
    QSize    s	    = size();
    QPixmap *bgp    = (QPixmap *)backgroundPixmap();
    QColor   bgc    = bg_col;			// save colors
    QString capt= caption();
    widget_flags = f;
    clearWState( QWS_Created | QWS_Visible );
    create();
    const QObjectList *chlist = children();
    if ( chlist ) {				// reparent children
	QObjectListIt it( *chlist );
	QObject *obj;
	while ( (obj=it.current()) ) {
	    if ( obj->isWidgetType() ) {
		QWidget *w = (QWidget *)obj;
		XReparentWindow( x11Display(), w->winId(), winId(),
				 w->geometry().x(), w->geometry().y() );
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
    setFocusPolicy( fp );
    if ( !capt.isNull() ) {
	extra->topextra->caption = QString::null;
	setCaption( capt );
    }
    if ( showIt )
	show();
    if ( old_winid )
	qt_XDestroyWindow( this, dpy, old_winid );

    QObjectList	*accelerators = queryList( "QAccel" );
    QObjectListIt it( *accelerators );
    QObject *obj;
    while ( (obj=it.current()) != 0 ) {
	++it;
	((QAccel*)obj)->repairEventFilter();
    }
    delete accelerators;
    if ( !parent ) {
	QFocusData *fd = focusData( TRUE );
	if ( fd->focusWidgets.findRef(this) < 0 )
 	    fd->focusWidgets.append( this );
    }
}


/*!
  Translates the widget coordinate \e pos to global screen coordinates.
  For example, \code mapToGlobal(QPoint(0,0))\endcode would give the
  global coordinates of the top-left pixel of the widget.
  \sa mapFromGlobal()
*/

QPoint QWidget::mapToGlobal( const QPoint &pos ) const
{
    int	   x, y;
    Window child;
    XTranslateCoordinates( x11Display(), winId(),
			   QApplication::desktop()->winId(),
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
    XTranslateCoordinates( x11Display(), QApplication::desktop()->winId(),
			   winId(), pos.x(), pos.y(), &x, &y, &child );
    return QPoint( x, y );
}


void QWidget::setSizeGrip( bool sizegrip )
{
    createExtra();
    WId id = winId();
    if ( (bool)extra->sizegrip != sizegrip ) {
	XChangeProperty(qt_xdisplay(), topLevelWidget()->winId(),
			qt_sizegrip, XA_WINDOW, 32, PropModeReplace,
			sizegrip?((unsigned char *)&id):(unsigned char*) None,
			1);
    }
}


void QWidget::setBackgroundColorDirect( const QColor &color )
{
    QColor old = bg_col;
    bg_col = color;
    if ( extra && extra->bg_pix ) {		// kill the background pixmap
	delete extra->bg_pix;
	extra->bg_pix = 0;
    }
    XSetWindowBackground( x11Display(), winId(), bg_col.pixel() );
    backgroundColorChange( old );
}

static int allow_null_pixmaps = 0;


void QWidget::setBackgroundPixmapDirect( const QPixmap &pixmap )
{
    QPixmap old;
    if ( extra && extra->bg_pix )
	old = *extra->bg_pix;
    if ( !allow_null_pixmaps && pixmap.isNull() ) {
	XSetWindowBackground( x11Display(), winId(), bg_col.pixel() );
	if ( extra && extra->bg_pix ) {
	    delete extra->bg_pix;
	    extra->bg_pix = 0;
	}
    } else {
	QPixmap pm = pixmap;
	if (!pm.isNull()) {
	    if ( pm.depth() == 1 && QPixmap::defaultDepth() > 1 ) {
		pm = QPixmap( pixmap.size() );
		bitBlt( &pm, 0, 0, &pixmap, 0, 0, pm.width(), pm.height() );
	    }
	}
	if ( extra && extra->bg_pix )
	    delete extra->bg_pix;
	else
	    createExtra();
	extra->bg_pix = new QPixmap( pm );
	XSetWindowBackgroundPixmap( x11Display(), winId(), pm.handle() );
	if ( testWFlags(WType_Desktop) )	// save rootinfo later
	    qt_updated_rootinfo();
    }
    if ( !allow_null_pixmaps ) {
	backgroundPixmapChange( old );
    }
}


/*!
  Sets the window-system background of the widget to nothing.

  Note that `nothing' is actually a pixmap that isNull(), thus you
  can check for an empty background by checking backgroundPixmap().

  \sa setBackgroundPixmap(), setBackgroundColor()

  This class should \e NOT be made virtual - it is an alternate usage
  of setBackgroundPixmap().
*/
void QWidget::setBackgroundEmpty()
{
    allow_null_pixmaps++;
    setBackgroundPixmap(QPixmap());
    allow_null_pixmaps--;
}


/*!
  Sets the widget cursor shape to \e cursor.

  The mouse cursor will assume this shape when it's over this widget.
  See a list of predefined cursor objects with a range of useful
  shapes in the QCursor documentation.

  An editor widget would for example use an I-beam cursor:
  \code
    setCursor( ibeamCursor );
  \endcode

  \sa cursor(), unsetCursor(), QApplication::setOverrideCursor()
*/

void QWidget::setCursor( const QCursor &cursor )
{
    if ( cursor.handle() != arrowCursor.handle()
	 || (extra && extra->curs) ) {
	createExtra();
	extra->curs = new QCursor(cursor);
    }
    setWState( QWS_OwnCursor );
    QCursor *oc = QApplication::overrideCursor();
    XDefineCursor( x11Display(), winId(),
		   oc ? oc->handle() : cursor.handle() );
    XFlush( x11Display() );
}


/*!
  Unset the cursor for this widget. The widget will use the cursor of
  its parent from now on.

  This functions does nothing for toplevel windows.

  \sa cursor(), setCursor(), QApplication::setOverrideCursor()
 */

void QWidget::unsetCursor()
{
    if ( !isTopLevel() ) {
	if (extra ) {
	    delete extra->curs;
	    extra->curs = 0;
	}
	clearWState( QWS_OwnCursor );
	XDefineCursor( x11Display(), winId(), None );
	XFlush( x11Display() );
    }
}

/*!
  Sets the window caption (title).
  \sa caption(), setIcon(), setIconText()
*/

void QWidget::setCaption( const QString &caption )
{
    if ( extra && extra->topextra && extra->topextra->caption == caption )
	return; // for less flicker
    createTLExtra();
    extra->topextra->caption = caption;
    XStoreName( x11Display(), winId(), caption );
}

/*!
  Sets the window icon pixmap.
  \sa icon(), setIconText(), setCaption()
*/

void QWidget::setIcon( const QPixmap &pixmap )
{
    if ( extra && extra->topextra ) {
	delete extra->topextra->icon;
	extra->topextra->icon = 0;
    } else {
	createTLExtra();
    }
    ::Pixmap icon_pixmap;
    ::Pixmap mask_pixmap;
    QBitmap mask;
    if ( pixmap.isNull() ) {
	icon_pixmap = 0;
	mask_pixmap = 0;
    } else {
	extra->topextra->icon = new QPixmap( pixmap );
	icon_pixmap = pixmap.handle();
	mask = pixmap.mask() ? *pixmap.mask() : pixmap.createHeuristicMask();
	mask_pixmap = mask.handle();
    }
    XWMHints *h = XGetWMHints( x11Display(), winId() );
    XWMHints  wm_hints;
    bool got_hints = h != 0;
    if ( !got_hints ) {
	h = &wm_hints;
	h->flags = 0;
    }
    h->icon_pixmap = icon_pixmap;
    h->icon_mask   = mask_pixmap;
    h->flags |= IconPixmapHint | IconMaskHint;
    XSetWMHints( x11Display(), winId(), h );
    if ( got_hints )
	XFree( (char *)h );
}


/*!
  Sets the text of the window's icon to \e iconText.
  \sa iconText(), setIcon(), setCaption()
*/

void QWidget::setIconText( const QString &iconText )
{
    createTLExtra();
    extra->topextra->iconText = iconText;
    XSetIconName( x11Display(), winId(), iconText );
}


void QWidget::setMouseTracking( bool enable )
{
    bool gmt = QApplication::hasGlobalMouseTracking();
    if ( enable == testWState(QWS_MouseTracking) && !gmt )
	return;
    uint m = (enable || gmt) ? (uint)PointerMotionMask : 0;
    if ( enable )
	setWState( QWS_MouseTracking );
    else
	clearWState( QWS_MouseTracking );
    if ( testWFlags(WType_Desktop) ) {		// desktop widget?
	if ( testWFlags(WPaintDesktop) )	// get desktop paint events
	    XSelectInput( x11Display(), winId(),
			  stdDesktopEventMask|ExposureMask );
	else
	    XSelectInput( x11Display(), winId(), stdDesktopEventMask );
    } else {
	XSelectInput( x11Display(), winId(),	// specify events
		      m | stdWidgetEventMask );
    }
}


/*!
  Grabs the mouse input.

  This widget will be the only one to receive mouse events until
  releaseMouse() is called.

  \warning Grabbing the mouse might lock the terminal.

  It is almost never necessary to grab the mouse when using Qt since
  Qt grabs and releases it sensibly.  In particular, Qt grabs the
  mouse when a button is pressed and keeps it until the last button is
  released.

  \sa releaseMouse(), grabKeyboard(), releaseKeyboard()
*/

void QWidget::grabMouse()
{
    if ( !qt_nograb() ) {
	if ( mouseGrb )
	    mouseGrb->releaseMouse();
	XGrabPointer( x11Display(), winId(), TRUE,
		      (uint)(ButtonPressMask | ButtonReleaseMask |
		             PointerMotionMask | EnterWindowMask | LeaveWindowMask),
		      GrabModeAsync, GrabModeAsync,
		      None, None, CurrentTime );
	mouseGrb = this;
    }
}

/*!
  Grabs the mouse input and changes the cursor shape.

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
	XGrabPointer( x11Display(), winId(), TRUE,
		      (uint)(ButtonPressMask | ButtonReleaseMask |
			     PointerMotionMask | EnterWindowMask | LeaveWindowMask),
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
	XUngrabPointer( x11Display(), CurrentTime );
	XFlush( x11Display() );
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
	XGrabKeyboard( x11Display(), winid, TRUE, GrabModeAsync, GrabModeAsync,
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
	XUngrabKeyboard( x11Display(), CurrentTime );
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
    QWidget* tlw = topLevelWidget();
    return tlw == qApp->activeWindow()
		|| (tlw->extra && tlw->extra->topextra && tlw->extra->topextra->embedded); //####

//    Window win;
//     int revert;
//     XGetInputFocus( dpy, &win, &revert );

//     if ( win == None) return FALSE;

//     QWidget *w = find( (WId)win );
//     if ( w ) {
// 	// We know that window
// 	return w->topLevelWidget() == topLevelWidget();
//     } else {
// 	// Window still may be a parent (if top-level is foreign window)
// 	Window root, parent;
// 	Window cursor = winId();
// 	Window *ch;
// 	unsigned int nch;
// 	while ( XQueryTree(dpy, cursor, &root, &parent, &ch, &nch) ) {
// 	    if (ch) XFree( (char*)ch);
// 	    if ( parent == win ) return TRUE;
// 	    if ( parent == root ) return FALSE;
// 	    cursor = parent;
// 	}
// 	return FALSE;
//     }
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
	XSetInputFocus( x11Display(), tlw->winId(), RevertToNone, CurrentTime);
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
    if ( (widget_state & (QWS_Visible|QWS_BlockUpdates)) == QWS_Visible )
	XClearArea( x11Display(), winId(), 0, 0, 0, 0, TRUE );
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
    if ( w && h &&
	 (widget_flags & (QWS_Visible|QWS_BlockUpdates)) == QWS_Visible ) {
	if ( w < 0 )
	    w = crect.width()  - x;
	if ( h < 0 )
	    h = crect.height() - y;
	if ( w != 0 && h != 0 )
	    XClearArea( x11Display(), winId(), x, y, w, h, TRUE );
    }
}

/*!
  \overload void QWidget::update( const QRect &r )
*/

/*!
  \overload void QWidget::repaint( bool erase )

  This version repaints the entire widget.
*/

/*!
  \overload void QWidget::repaint()

  This version erases and repaints the entire widget.
*/

/*!
  Repaints the widget directly by calling paintEvent() directly,
  unless updates are disabled or the widget is hidden.

  Erases the widget area  \e (x,y,w,h) if \e erase is TRUE.

  If \e w is negative, it is replaced with <code>width() - x</code>.
  If \e h is negative, it is replaced width <code>height() - y</code>.

  Doing a repaint() is usually faster than doing an update(), but
  calling update() many times in a row will generate a single paint
  event.

  \warning If you call repaint() in a function which may itself be called
  from paintEvent(), you may see infinite recursion. The update() function
  never generates recursion.

  \sa update(), paintEvent(), setUpdatesEnabled(), erase()
*/

void QWidget::repaint( int x, int y, int w, int h, bool erase )
{
    if ( (widget_state & (QWS_Visible|QWS_BlockUpdates)) == QWS_Visible ) {
	if ( w < 0 )
	    w = crect.width()  - x;
	if ( h < 0 )
	    h = crect.height() - y;
	QPaintEvent e( QRect(x,y,w,h), erase );
	if ( erase && w != 0 && h != 0 )
	    XClearArea( x11Display(), winId(), x, y, w, h, FALSE );
	QApplication::sendEvent( this, &e );
    }
}

/*!
  Repaints the widget directly by calling paintEvent() directly,
  unless updates are disabled or the widget is hidden.

  Erases the widget region  \a reg if \a erase is TRUE.

  Calling repaint() is usually faster than doing an update(), but
  calling update() many times in a row will generate a single paint
  event.

  \warning If you call repaint() in a function which may itself be called
  from paintEvent(), you may see infinite recursion. The update() function
  never generates recursion.

  \sa update(), paintEvent(), setUpdatesEnabled(), erase()
*/

void QWidget::repaint( const QRegion& reg, bool erase )
{
    if ( (widget_state & (QWS_Visible|QWS_BlockUpdates)) == QWS_Visible ) {
	if ( erase )
	    this->erase(reg);
	QPaintEvent e( reg );
	QApplication::sendEvent( this, &e );
    }
}

/*!
  \overload void QWidget::repaint( const QRect &r, bool erase )
*/


/*!
  \internal
  Platform-specific part of QWidget::show().
*/

void QWidget::showWindow()
{
    setWState( QWS_Visible );
    clearWFlags( QWS_ForceHide );

    QShowEvent e(FALSE);
    QApplication::sendEvent( this, &e );

    XMapWindow( x11Display(), winId() );
}


/*!
  \internal
  Platform-specific part of QWidget::hide().
*/

void QWidget::hideWindow()
{
    if ( qt_button_down == this )
	qt_button_down = 0;
    XUnmapWindow( x11Display(), winId() );
    if ( isPopup() )
	XFlush( x11Display() );
}


/*!
  Shows the widget minimized, as an icon.

  Calling this function has no effect for other than \link isTopLevel()
  top-level widgets\endlink.

  \sa showNormal(), showMaximized(), show(), hide(), isVisible()
*/

void QWidget::showMinimized()
{
    if ( testWFlags(WType_TopLevel) )
	XIconifyWindow( x11Display(), winId(), x11Screen() );
}

/*!
  Shows the widget maximized.

  Calling this function has no effect for other than \link isTopLevel()
  top-level widgets\endlink.

  \sa showNormal(), showMinimized(), show(), hide(), isVisible()
*/

void QWidget::showMaximized()
{
    if ( testWFlags(WType_TopLevel) ) {
	Display *dpy = x11Display();
	int scr = x11Screen();
	int sw = DisplayWidth(dpy,scr);
	int sh = DisplayHeight(dpy,scr);
	createTLExtra();
	if ( extra->topextra->normalGeometry.width() < 0 ) {
	    extra->topextra->normalGeometry = geometry();
	    setGeometry( 0, 0, sw, sh );
	}
	show();
    }
}

/*!
  Restores the widget after it has been maximized or minimized.

  Calling this function has no effect for other than \link isTopLevel()
  top-level widgets\endlink.

  \sa showMinimized(), showMaximized(), show(), hide(), isVisible()
*/

void QWidget::showNormal()
{
    if ( testWFlags(WType_TopLevel) ) {
	if ( extra && extra->topextra ) {
	    QRect r = extra->topextra->normalGeometry;
	    if ( r.width() >= 0 ) {
		// the widget has been maximized
		extra->topextra->normalGeometry = QRect(0,0,-1,-1);
		setGeometry( r );
	    }
	}
	show();
    }
}


/*!
  Raises this widget to the top of the parent widget's stack.

  If there are any siblings of this widget that overlap it on the screen,
  this widget will be visually in front of its siblings afterwards.

  \sa lower()
*/

void QWidget::raise()
{
    QWidget *p = parentWidget();
    if ( p && p->childObjects && p->childObjects->findRef(this) >= 0 )
	p->childObjects->append( p->childObjects->take() );
    XRaiseWindow( x11Display(), winId() );
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
	p->childObjects->insert( 0, p->childObjects->take() );
    XLowerWindow( x11Display(), winId() );
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
	if ( x->topextra &&
	   (x->topextra->incw > 0 || x->topextra->inch > 0) )
	{					// add resize increment hints
	    s->flags |= PResizeInc | PBaseSize;
	    s->width_inc = x->topextra->incw;
	    s->height_inc = x->topextra->inch;
	    s->base_width = 0;
	    s->base_height = 0;
	}
    }
    s->flags |= PWinGravity;
    s->win_gravity = qwidget_tlw_gravity;	// usually NorthWest (1)
    qwidget_tlw_gravity = 1;			// reset in case it was set
    XSetWMNormalHints( dpy, winid, s );
}


void QWidget::internalSetGeometry( int x, int y, int w, int h, bool isMove )
{
    Display *dpy = x11Display();

    if ( testWFlags(WType_Desktop) )
	return;
    if ( extra ) {				// any size restrictions?
	w = QMIN(w,extra->maxw);
	h = QMIN(h,extra->maxh);
	w = QMAX(w,extra->minw);
	h = QMAX(h,extra->minh);
    }
    if ( w < 1 )				// invalid size
	w = 1;
    if ( h < 1 )
	h = 1;
    QPoint oldp = pos();
    QSize  olds = size();
    QRect  r( x, y, w, h );

    // We only care about stuff that changes the geometry, or may
    // cause the window manager to change its state
    if ( r.size() == olds && oldp == r.topLeft() &&
	 (isTopLevel() == FALSE || testWState(QWS_USPositionX)) )
	return;

    setCRect( r );

    if ( isTopLevel() ) {
	setWState( QWS_ConfigPending );
	XSizeHints size_hints;
	size_hints.flags = USSize | PSize;
	if ( isMove )
	    setWState(QWS_USPositionX);
	if ( testWState(QWS_USPositionX) )
	    // also restore the usposition, otherwise it would be cleared
	    size_hints.flags |= USPosition;
	size_hints.x = x;
	size_hints.y = y;
	size_hints.width = w;
	size_hints.height = h;
	do_size_hints( dpy, winId(), extra, &size_hints );
    }

    if ( isMove )
	XMoveResizeWindow( dpy, winid, x, y, w, h );
    else
	XResizeWindow( dpy, winid, w, h );

    bool isResize = olds != r.size();

    if ( isVisible() ) {
	if ( isMove ) {
	    QMoveEvent e( r.topLeft(), oldp );
	    QApplication::sendEvent( this, &e );
	}
	if ( isResize ) {
	    QResizeEvent e( r.size(), olds );
	    QApplication::sendEvent( this, &e );
	}
	if ( !testWFlags(WResizeNoErase) )
	    repaint( TRUE );
    } else {
	if ( isMove )
	    QApplication::postEvent( this,
				     new QMoveEvent( r.topLeft(), oldp ) );
	if ( isResize )
	    QApplication::postEvent( this,
				     new QResizeEvent( r.size(), olds ) );
    }
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

void QWidget::setMinimumSize( int minw, int minh )
{
#if defined(CHECK_RANGE)
    if ( minw < 0 || minh < 0 )
	warning("QWidget::setMinimumSize: The smallest allowed size is (0,0)");
#endif
    createExtra();
    if ( extra->minw == minw && extra->minh == minh )
	return;
    extra->minw = minw;
    extra->minh = minh;
    if ( minw > width() || minh > height() )
	resize( QMAX(minw,width()), QMAX(minh,height()) );
    if ( testWFlags(WType_TopLevel) ) {
	XSizeHints size_hints;
	size_hints.flags = 0;
	do_size_hints( x11Display(), winId(), extra, &size_hints );
    }
    if ( !isTopLevel() )
	QApplication::postEvent( parentWidget(),
				 new QEvent( QEvent::LayoutHint) );
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

void QWidget::setMaximumSize( int maxw, int maxh )
{
#if defined(CHECK_RANGE)
    if ( maxw > QCOORD_MAX || maxh > QCOORD_MAX )
	warning("QWidget::setMaximumSize: (%s/%s) "
		"The largest allowed size is (%d,%d)",
		 name( "unnamed" ), className(), QCOORD_MAX, QCOORD_MAX );
    if ( maxw < 0 || maxh < 0 )
	warning("QWidget::setMaximumSize: (%s/%s) Negative sizes (%d,%d) "
		"are not possible",
		name( "unnamed" ), className(), maxw, maxh );
#endif
    createExtra();
    if ( extra->maxw == maxw && extra->maxh == maxh )
	return;
    extra->maxw = maxw;
    extra->maxh = maxh;
    if ( maxw < width() || maxh < height() )
	resize( QMIN(maxw,width()), QMIN(maxh,height()) );
    if ( testWFlags(WType_TopLevel) ) {
	XSizeHints size_hints;
	size_hints.flags = 0;
	do_size_hints( x11Display(), winId(), extra, &size_hints );
    }
    if ( !isTopLevel() )
	QApplication::postEvent( parentWidget(), new QEvent( QEvent::LayoutHint) );
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
    createTLExtra();
    QTLWExtra* x = extra->topextra;
    if ( x->incw == w && x->inch == h )
	return;
    x->incw = w;
    x->inch = h;
    if ( testWFlags(WType_TopLevel) ) {
	XSizeHints size_hints;
	size_hints.flags = 0;
	do_size_hints( x11Display(), winId(), extra, &size_hints );
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
	XClearArea( x11Display(), winId(), x, y, w, h, FALSE );
}

/*!
  Erases the area defined by \a reg, without generating a
  \link paintEvent() paint event\endlink.

  Child widgets are not affected.
*/

void QWidget::erase( const QRegion& reg )
{
    QArray<QRect> r = reg.rects();
    for (uint i=0; i<r.size(); i++) {
	const QRect& rr = r[(int)i];
	XClearArea( x11Display(), winId(),
		    rr.x(), rr.y(), rr.width(), rr.height(), FALSE );
    }
}


/*!
  Scrolls the contents of the widget \e dx pixels rightwards and \e dy
  pixels downwards.  If \e dx/dy is negative, the scroll direction is
  leftwards/upwards.  Child widgets are moved accordingly.

  The areas of the widget that are exposed will be erased and
  \link paintEvent() paint events\endlink may be generated immediately,
  or after some further event processing.

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

    if ( dx == 0 && dy == 0 )
	return;

    Display *dpy = x11Display();
    GC gc = qt_xget_readonly_gc();
    // Want expose events
    XSetGraphicsExposures( dpy, gc, TRUE );
    XCopyArea( dpy, winId(), winId(), gc, x1, y1, w, h, x2, y2);
    XSetGraphicsExposures( dpy, gc, FALSE );

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

    // Don't let the server be bogged-down with repaint events
    bool repaint_immediately = qt_sip_count( this ) < 3;

    if ( dx ) {
	x1 = x2 == 0 ? w : 0;
	if ( repaint_immediately )
	    repaint( x1, 0, crect.width()-w, crect.height(), TRUE );
	else
	    XClearArea( dpy, winid, x1, 0, crect.width()-w, crect.height(),
			TRUE);
    }
    if ( dy ) {
	y1 = y2 == 0 ? h : 0;
	if ( repaint_immediately )
	    repaint( 0, y1, crect.width(), crect.height()-h, TRUE );
	else
	    XClearArea( dpy, winid, 0, y1, crect.width(), crect.height()-h,
		       TRUE );
    }

    qt_insert_sip( this, dx, dy );
}


/*!
  \overload void QWidget::drawText( const QPoint &pos, const QString& str )
*/

/*!
  Writes \e str at position \e x,y.

  The \e y position is the base line position of the text.  The text is
  drawn using the default font and the default foreground color.

  This function is provided for convenience.  You will generally get
  more flexible results and often higher speed by using a a \link
  QPainter painter\endlink instead.

  \sa setFont(), foregroundColor(), QPainter::drawText()
*/

void QWidget::drawText( int x, int y, const QString &str )
{
    if ( testWState(QWS_Visible) ) {
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
	Display *dpy = x11Display();
	int scr = x11Screen();
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

void QWidget::createSysExtra()
{
    extra->xDndProxy = 0;
}

void QWidget::deleteSysExtra()
{
}

void QWidget::createTLSysExtra()
{
    extra->topextra->xic = 0;
}

void QWidget::deleteTLSysExtra()
{
#if !defined(X11R4)
    if (extra->topextra->xic) {
	XDestroyIC( (XIC) extra->topextra->xic );
	extra->topextra->xic = 0;
    }
#endif
}


/*!
  Returns TRUE if drop events are enabled for this widget.

  \sa setAcceptDrops()
*/

bool QWidget::acceptDrops() const
{
    return testWState(QWS_DND);
}

/*!
  Announces to the system that this widget \e may be able to
  accept drop events.

  In Qt 1.x, drop event handlers are in QDropSite.

  \sa acceptDrops()
*/

void QWidget::setAcceptDrops( bool on )
{
    if ( testWState(QWS_DND) != on ) {
	if ( on ) {
	    setWState(QWS_DND);
	    QWidget * tlw = topLevelWidget();

	    extern Atom qt_xdnd_aware;
	    Atom qt_xdnd_version = (Atom)3;
	    XChangeProperty ( tlw->x11Display(), tlw->winId(), qt_xdnd_aware,
			      XA_ATOM, 32, PropModeReplace,
			      (unsigned char *)&qt_xdnd_version, 1 );
	} else {
	    clearWState(QWS_DND);
	}
    }
}

/*!
  Causes only the parts of the widget which overlap \a region
  to be visible.  If the region includes pixels outside the
  rect() of the widget, window system controls in that area
  may or may not be visible, depending on the platform.

  Note that this effect can be slow if the region is particularly
  complex.

  \sa setMask(QBitmap), clearMask()
*/

void QWidget::setMask( const QRegion& region )
{
    XShapeCombineRegion( x11Display(), winId(), ShapeBounding, 0, 0,
			 region.handle(), ShapeSet );
}

/*!
  Causes only the pixels of the widget for which \a bitmap
  has a corresponding 1 bit
  to be visible.  If the region includes pixels outside the
  rect() of the widget, window system controls in that area
  may or may not be visible, depending on the platform.

  Note that this effect can be slow if the region is particularly
  complex.

  \sa setMask(const QRegion&), clearMask()
*/

void QWidget::setMask( const QBitmap &bitmap )
{
    XShapeCombineMask( x11Display(), winId(), ShapeBounding, 0, 0,
		       bitmap.handle(), ShapeSet );
}

/*!
  Removes any mask set by setMask().

  \sa setMask()
*/

void QWidget::clearMask()
{
    XShapeCombineMask( x11Display(), winId(), ShapeBounding, 0, 0,
		       None, ShapeSet );
}
