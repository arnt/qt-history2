/****************************************************************************
**
** Implementation of QWidget and QWindow classes for X11.
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

#include "qwidget.h"
#include "qevent.h"
#include "qdesktopwidget.h"
#include "qapplication.h"
#include "qapplication_p.h"
#include "qnamespace.h"
#include "qpaintdevicemetrics.h"
#include "qpainter.h"
#include "qbitmap.h"
#include "qlayout.h"
#include "qtextcodec.h"
#include "qdatetime.h"
#include "qcursor.h"
#include "qstack.h"

// Paint event clipping magic
extern void qt_set_paintevent_clipping( QPaintDevice* dev, const QRegion& region);
extern void qt_clear_paintevent_clipping();

#ifdef Q_Q4PAINTER
#include "qx11gc.h"
#define QPaintDevice QX11GC
#endif

#include "qt_x11_p.h"
#include <stdlib.h>

// NOT REVISED

// defined in qapplication_x11.cpp
bool qt_wstate_iconified( WId );
void qt_updated_rootinfo();

#ifndef QT_NO_XIM
#include "qinputcontext_p.h"
#endif

#include "qwidget_p.h"
#define d d_func()
#define q q_func()

extern bool qt_dnd_enable( QWidget* w, bool on );
extern bool qt_nograb();

static QWidget *mouseGrb    = 0;
static QWidget *keyboardGrb = 0;

// defined in qapplication_x11.cpp
extern Time qt_x_time;
extern Time qt_x_user_time;

int qt_x11_create_desktop_on_screen = -1;


/*****************************************************************************
  QWidget member functions
 *****************************************************************************/

extern bool qt_broken_wm;

// defined in qapplication_x11.cpp
extern bool qt_net_supports(Atom);

#if defined (QT_TABLET_SUPPORT)
extern XDevice *devStylus;
extern XDevice *devEraser;
extern XEventClass event_list_stylus[7];
extern XEventClass event_list_eraser[7];
extern int qt_curr_events_stylus;
extern int qt_curr_events_eraser;
#endif

const uint stdWidgetEventMask =			// X event mask
	(uint)(
	    KeyPressMask | KeyReleaseMask |
	    ButtonPressMask | ButtonReleaseMask |
	    KeymapStateMask |
	    ButtonMotionMask | PointerMotionMask |
	    EnterWindowMask | LeaveWindowMask |
	    FocusChangeMask |
	    ExposureMask |
	    PropertyChangeMask |
	    StructureNotifyMask
	);

const uint stdDesktopEventMask =			// X event mask
       (uint)(
           KeymapStateMask |
	   EnterWindowMask | LeaveWindowMask |
	   PropertyChangeMask
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


static void qt_insert_sip( QWidget* scrolled_widget, int dx, int dy )
{
    QX11Data::ScrollInProgress sip = { X11->sip_serial++, scrolled_widget, dx, dy };
    X11->sip_list.append( sip );

    XClientMessageEvent client_message;
    client_message.type = ClientMessage;
    client_message.window = scrolled_widget->winId();
    client_message.format = 32;
    client_message.message_type = ATOM(Qt_Scroll_Done);
    client_message.data.l[0] = sip.id;

    XSendEvent(X11->display, scrolled_widget->winId(), False, NoEventMask,
	(XEvent*)&client_message );
}

static int qt_sip_count( QWidget* scrolled_widget )
{
    int sips=0;

    for (int i = 0; i < X11->sip_list.size(); ++i) {
	const QX11Data::ScrollInProgress &sip = X11->sip_list.at(i);
	if ( sip.scrolled_widget == scrolled_widget )
	    sips++;
    }

    return sips;
}


static void create_wm_client_leader()
{
    if ( X11->wm_client_leader ) return;

    X11->wm_client_leader =
	XCreateSimpleWindow( QPaintDevice::x11AppDisplay(),
			     QPaintDevice::x11AppRootWindow(),
			     0, 0, 1, 1, 0, 0, 0 );

    // set client leader property to itself
    XChangeProperty( QPaintDevice::x11AppDisplay(),
		     X11->wm_client_leader, ATOM(Wm_Client_Leader),
		     XA_WINDOW, 32, PropModeReplace,
		     (unsigned char *)&X11->wm_client_leader, 1 );

    // If we are session managed, inform the window manager about it
    QByteArray session = qApp->sessionId().toLatin1();
    if ( !session.isEmpty() ) {
	XChangeProperty( QPaintDevice::x11AppDisplay(),
			 X11->wm_client_leader, ATOM(Sm_Client_Id),
			 XA_STRING, 8, PropModeReplace,
			 (unsigned char *)session.data(), session.size() );
    }
}


Q_GUI_EXPORT void qt_x11_enforce_cursor( QWidget * w )
{
    if ( w->testAttribute( QWidget::WA_SetCursor ) ) {
	QCursor *oc = QApplication::overrideCursor();
	if ( oc ) {
	    XDefineCursor( w->x11Display(), w->winId(), oc->handle() );
	} else if ( w->isEnabled() ) {
	    XDefineCursor( w->x11Display(), w->winId(), w->cursor().handle() );
	} else {
	    // enforce the windows behavior of clearing the cursor on
	    // disabled widgets
	    XDefineCursor( w->x11Display(), w->winId(), None );
	}
    } else {
	XDefineCursor( w->x11Display(), w->winId(), None );
    }
}

Q_GUI_EXPORT void qt_wait_for_window_manager( QWidget* w )
{
    QApplication::flushX();
    XEvent ev;
    QTime t;
    t.start();
    while ( !XCheckTypedWindowEvent( w->x11Display(), w->winId(), ReparentNotify, &ev ) ) {
	if ( XCheckTypedWindowEvent( w->x11Display(), w->winId(), MapNotify, &ev ) )
	    break;
	if ( t.elapsed() > 500 )
	    return; // give up, no event available
	qApp->syncX(); // non-busy wait
    }
    qApp->x11ProcessEvent( &ev );
    if ( XCheckTypedWindowEvent( w->x11Display(), w->winId(), ConfigureNotify, &ev ) )
	qApp->x11ProcessEvent( &ev );
}

static void qt_net_change_wm_state(const QWidget* w, bool set, Atom one, Atom two = 0)
{
    if (w->isShown()) {
	// managed by WM
        XEvent e;
        e.xclient.type = ClientMessage;
        e.xclient.message_type = ATOM(Net_Wm_State);
        e.xclient.display = w->x11Display();
        e.xclient.window = w->winId();
        e.xclient.format = 32;
        e.xclient.data.l[ 0 ] = set ? 1 : 0;
        e.xclient.data.l[ 1 ] = one;
        e.xclient.data.l[ 2 ] = two;
        e.xclient.data.l[ 3 ] = 0;
        e.xclient.data.l[ 4 ] = 0;
        XSendEvent(w->x11Display(), RootWindow(w->x11Display(), w->x11Screen()),
		   False, (SubstructureNotifyMask|SubstructureRedirectMask), &e);
    } else {
        Atom ret;
	int format = 0, status;
	unsigned char *data = 0;
	unsigned long nitems = 0, after = 0;
	Atom *old_states = 0;
	status = XGetWindowProperty(w->x11Display(), w->winId(),
				    ATOM(Net_Wm_State), 0, 1024, False,
				    XA_ATOM, &ret, &format, &nitems,
				    &after, &data);
	if (status == Success && ret == XA_ATOM && format == 32 && nitems > 0)
	    old_states = (Atom *) data;
	else
	    nitems = 0;

	Atom *new_states = new Atom[nitems + 2];
	int i, j = 0;
	for (i = 0; i < (int)nitems; ++i) {
	    if (old_states[i] && old_states[i] != one && old_states[i] != two)
		new_states[j++] = old_states[i];
	}

	if (set) {
	    if (one) new_states[j++] = one;
	    if (two) new_states[j++] = two;
	}

	if (j)
	    XChangeProperty(w->x11Display(), w->winId(), ATOM(Net_Wm_State), XA_ATOM, 32,
			    PropModeReplace, (uchar *) new_states, j);
	else
	    XDeleteProperty(w->x11Display(), w->winId(), ATOM(Net_Wm_State));

	delete [] new_states;
	if (data) XFree(data);
    }
}

/*!
    Creates a new widget window if \a window is 0, otherwise sets the
    widget's window to \a window.

    Initializes the window (sets the geometry etc.) if \a
    initializeWindow is TRUE. If \a initializeWindow is FALSE, no
    initialization is performed. This parameter only makes sense if \a
    window is a valid window.

    Destroys the old window if \a destroyOldWindow is TRUE. If \a
    destroyOldWindow is FALSE, you are responsible for destroying the
    window yourself (using platform native code).

    The QWidget constructor calls create(0,TRUE,TRUE) to create a
    window for this widget.
*/

void QWidget::create( WId window, bool initializeWindow, bool destroyOldWindow)
{
    if ( testWState(WState_Created) && window == 0 )
	return;

    // set created flag
    setWState( WState_Created );

    bool popup = testWFlags(WType_Popup);
    bool dialog = testWFlags(WType_Dialog);
    bool desktop = testWFlags(WType_Desktop);

    // top-level widget
    if ( !parentWidget() || parentWidget()->isDesktop() )
	setWFlags( WType_TopLevel );

    // these are top-level, too
    if ( dialog || popup || desktop || testWFlags(WStyle_Splash))
	setWFlags( WType_TopLevel );

    // a popup stays on top
    if ( popup )
	setWFlags(WStyle_StaysOnTop);

    bool topLevel = testWFlags(WType_TopLevel);
    Window parentw, destroyw = 0;
    WId	   id;

    // always initialize
    if ( !window )
	initializeWindow = TRUE;

#ifdef Q_Q4PAINTER
    if (destroyOldWindow) {
	delete deviceGC;
	deviceGC = 0;
    }
    if (!deviceGC)
	deviceGC = new QX11GC(this);
#endif

    if ( desktop &&
	 qt_x11_create_desktop_on_screen >= 0 &&
	 qt_x11_create_desktop_on_screen != x11Screen() ) {
	// desktop on a certain screen other than the default requested
#ifdef Q_Q4PAINTER
	QX11GCData* xd = static_cast<QX11GC *>(deviceGC)->getX11Data(true);
#else
	QPaintDeviceX11Data* xd = getX11Data( TRUE );
#endif
	xd->x_screen = qt_x11_create_desktop_on_screen;
	xd->x_depth = QPaintDevice::x11AppDepth( xd->x_screen );
	xd->x_cells = QPaintDevice::x11AppCells( xd->x_screen );
	xd->x_colormap = QPaintDevice::x11AppColormap( xd->x_screen );
	xd->x_defcolormap = QPaintDevice::x11AppDefaultColormap( xd->x_screen );
	xd->x_visual = QPaintDevice::x11AppVisual( xd->x_screen );
	xd->x_defvisual = QPaintDevice::x11AppDefaultVisual( xd->x_screen );
#ifdef Q_Q4PAINTER
	static_cast<QX11GC *>(deviceGC)->setX11Data(xd);
#else
	setX11Data( xd );
#endif
    } else if ( parentWidget() &&  parentWidget()->x11Screen() != x11Screen() ) {
	// if we have a parent widget, move to its screen if necessary
#ifdef Q_Q4PAINTER
	QX11GCData* xd = static_cast<QX11GC *>(deviceGC)->getX11Data(true);
#else
	QPaintDeviceX11Data* xd = getX11Data( TRUE );
#endif
	xd->x_screen = parentWidget()->x11Screen();
	xd->x_depth = QPaintDevice::x11AppDepth( xd->x_screen );
	xd->x_cells = QPaintDevice::x11AppCells( xd->x_screen );
	xd->x_colormap = QPaintDevice::x11AppColormap( xd->x_screen );
	xd->x_defcolormap = QPaintDevice::x11AppDefaultColormap( xd->x_screen );
	xd->x_visual = QPaintDevice::x11AppVisual( xd->x_screen );
	xd->x_defvisual = QPaintDevice::x11AppDefaultVisual( xd->x_screen );
#ifdef Q_Q4PAINTER
	static_cast<QX11GC *>(deviceGC)->setX11Data(xd);
#else
	setX11Data( xd );
#endif
    }

    //get display, screen number, root window and desktop geometry for
    //the current screen
    Display *dpy = x11Display();
    int scr = x11Screen();
    Window root_win = RootWindow( dpy, scr );
    int sw = DisplayWidth(dpy,scr);
    int sh = DisplayHeight(dpy,scr);

    if ( desktop ) {				// desktop widget
	dialog = popup = FALSE;			// force these flags off
	crect.setRect( 0, 0, sw, sh );
    } else if ( topLevel ) {			// calc pos/size from screen
	crect.setRect( sw/4, 3*sh/10, sw/2, 4*sh/10 );
    } else {					// child widget
	crect.setRect( 0, 0, 100, 30 );
    }

    parentw = topLevel ? root_win : parentWidget()->winId();

    XSetWindowAttributes wsa;

    if ( window ) {				// override the old window
	if ( destroyOldWindow )
	    destroyw = winid;
	id = window;
	setWinId( window );
	XWindowAttributes a;
	XGetWindowAttributes( dpy, window, &a );
	crect.setRect( a.x, a.y, a.width, a.height );

	if ( a.map_state == IsUnmapped )
	    clearWState( WState_Visible );
	else
	    setWState( WState_Visible );

#ifdef Q_Q4PAINTER
	QX11GCData* xd = static_cast<QX11GC *>(deviceGC)->getX11Data(true);
#else
	QPaintDeviceX11Data* xd = getX11Data( TRUE );
#endif

	// find which screen the window is on...
	xd->x_screen = QPaintDevice::x11AppScreen(); // by default, use the default :)
	int i;
	for ( i = 0; i < ScreenCount( dpy ); i++ ) {
	    if ( RootWindow( dpy, i ) == a.root ) {
		xd->x_screen = i;
		break;
	    }
	}

	xd->x_depth = a.depth;
	xd->x_cells = DisplayCells( dpy, xd->x_screen );
	xd->x_visual = a.visual;
	xd->x_defvisual = ( XVisualIDFromVisual( a.visual ) ==
			    XVisualIDFromVisual( (Visual*)x11AppVisual(x11Screen()) ) );
	xd->x_colormap = a.colormap;
	xd->x_defcolormap = ( a.colormap == x11AppColormap( x11Screen() ) );
#ifdef Q_Q4PAINTER
	static_cast<QX11GC *>(deviceGC)->setX11Data(xd);
#else
	setX11Data( xd );
#endif
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
					      QColor(black).pixel(x11Screen()),
					      QColor(white).pixel(x11Screen()));
	} else {
	    wsa.background_pixel = QColor(white).pixel(x11Screen());
	    wsa.border_pixel = QColor(black).pixel(x11Screen());
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

#ifndef QT_NO_XFTFREETYPE
    if (rendhd) {
	XftDrawDestroy( (XftDraw *) rendhd );
	rendhd = 0;
    }

    if ( X11->has_xft )
	rendhd = (HANDLE) XftDrawCreate( dpy, id, (Visual *) x11Visual(),
					 x11Colormap() );
#endif // QT_NO_XFTFREETYPE

    // NET window types
    long net_wintypes[7] = { 0, 0, 0, 0, 0, 0, 0 };
    int curr_wintype = 0;

    // NET window states
    long net_winstates[6] = { 0, 0, 0, 0, 0, 0 };
    int curr_winstate = 0;

    struct {
        ulong flags, functions, decorations;
        long input_mode;
        ulong status;
    } mwmhints;

    mwmhints.flags = mwmhints.functions = 0L;
    mwmhints.decorations = (1L << 0); // MWM_DECOR_ALL
    mwmhints.input_mode = 0L;
    mwmhints.status = 0L;

    if (topLevel && ! (desktop || popup)) {
	ulong wsa_mask = 0;

	if ( testWFlags(WStyle_Splash) ) {
            if (qt_net_supports(ATOM(Net_Wm_Window_Type_Splash))) {
                clearWFlags( WX11BypassWM );
                net_wintypes[curr_wintype++] = ATOM(Net_Wm_Window_Type_Splash);
	    } else {
		setWFlags( WX11BypassWM | WStyle_Tool | WStyle_NoBorder );
	    }
        }
	if (testWFlags(WStyle_Customize)) {
	    mwmhints.decorations = 0L;
	    mwmhints.flags |= (1L << 1); // MWM_HINTS_DECORATIONS

	    if ( testWFlags( WStyle_NoBorder ) ) {
		// override netwm type - quick and easy for KDE noborder
		net_wintypes[curr_wintype++] = ATOM(Kde_Net_Wm_Window_Type_Override);
	    } else {
		if ( testWFlags( WStyle_NormalBorder | WStyle_DialogBorder ) ) {
		    mwmhints.decorations |= (1L << 1); // MWM_DECOR_BORDER
		    mwmhints.decorations |= (1L << 2); //  MWM_DECOR_RESIZEH
		}

		if ( testWFlags( WStyle_Title ) )
		    mwmhints.decorations |= (1L << 3); // MWM_DECOR_TITLE

		if ( testWFlags( WStyle_SysMenu ) )
		    mwmhints.decorations |= (1L << 4); // MWM_DECOR_MENU

		if ( testWFlags( WStyle_Minimize ) )
		    mwmhints.decorations |= (1L << 5); // MWM_DECOR_MINIMIZE

		if ( testWFlags( WStyle_Maximize ) )
		    mwmhints.decorations |= (1L << 6); // MWM_DECOR_MAXIMIZE
	    }

	    if (testWFlags(WStyle_Tool)) {
		wsa.save_under = True;
		wsa_mask |= CWSaveUnder;

		// utility netwm type
		net_wintypes[curr_wintype++] = ATOM(Net_Wm_Window_Type_Utility);
	    }
	} else if (testWFlags(WType_Dialog)) {
	    setWFlags(WStyle_NormalBorder | WStyle_Title |
		      WStyle_SysMenu | WStyle_ContextHelp);

	    // dialog netwm type
            net_wintypes[curr_wintype++] = ATOM(Net_Wm_Window_Type_Dialog);
	} else {
	    setWFlags(WStyle_NormalBorder | WStyle_Title |
		      WStyle_MinMax | WStyle_SysMenu);

	    // maximized netwm state
	    if (testWState(WState_Maximized)) {
		net_winstates[curr_winstate++] = ATOM(Net_Wm_State_Maximized_Vert);
		net_winstates[curr_winstate++] = ATOM(Net_Wm_State_Maximized_Horz);
	    }
	}

	// ### need a better way to do this
	if (inherits("QPopupMenu")) {
	    // menu netwm type
	    net_wintypes[curr_wintype++] = ATOM(Net_Wm_Window_Type_Menu);
	} else if (inherits("QToolBar")) {
	    // toolbar netwm type
	    net_wintypes[curr_wintype++] = ATOM(Net_Wm_Window_Type_Toolbar);
	}

	// normal netwm type - default
	net_wintypes[curr_wintype++] = ATOM(Net_Wm_Window_Type_Normal);

	// stays on top
	if (testWFlags(WStyle_StaysOnTop)) {
	    net_winstates[curr_winstate++] = ATOM(Net_Wm_State_Above);
	    net_winstates[curr_winstate++] = ATOM(Net_Wm_State_Stays_On_Top);
}

        if (testWFlags(WShowModal)) {
            mwmhints.input_mode = 3L; // MWM_INPUT_FULL_APPLICATION_MODAL
            mwmhints.flags |= (1L << 2); // MWM_HINTS_INPUT_MODE

            net_winstates[curr_winstate++] = ATOM(Net_Wm_State_Modal);
        }

        if ( testWFlags( WX11BypassWM ) ) {
	    wsa.override_redirect = True;
	    wsa_mask |= CWOverrideRedirect;
	}

	if ( wsa_mask && initializeWindow )
	    XChangeWindowAttributes( dpy, id, wsa_mask, &wsa );
    } else {
	if (! testWFlags(WStyle_Customize))
	    setWFlags(WStyle_NormalBorder | WStyle_Title |
		      WStyle_MinMax | WStyle_SysMenu);
    }


    if ( !initializeWindow ) {
	// do no initialization
    } else if ( popup ) {			// popup widget
	wsa.override_redirect = True;
	wsa.save_under = True;
	XChangeWindowAttributes( dpy, id, CWOverrideRedirect | CWSaveUnder,
				 &wsa );
    } else if ( topLevel && !desktop ) {	// top-level widget
	QWidget *p = parentWidget();	// real parent
	if (p)
	    p = p->topLevelWidget();

 	if (dialog || testWFlags(WStyle_DialogBorder) || testWFlags(WStyle_Tool)) {
	    if ( p )
		XSetTransientForHint( dpy, id, p->winId() );
	    else				// application-modal
		XSetTransientForHint( dpy, id, root_win );
	}

	// find the real client leader, i.e. a toplevel without parent
	while ( p && p->parentWidget())
	    p = p->parentWidget()->topLevelWidget();

	XSizeHints size_hints;
	size_hints.flags = USSize | PSize | PWinGravity;
	size_hints.x = crect.left();
	size_hints.y = crect.top();
	size_hints.width = crect.width();
	size_hints.height = crect.height();
	size_hints.win_gravity =
	    QApplication::reverseLayout() ? NorthEastGravity : NorthWestGravity;

	XWMHints wm_hints;			// window manager hints
	wm_hints.input = True;
	wm_hints.initial_state = NormalState;
	wm_hints.flags = InputHint | StateHint;

	if ( !X11->wm_client_leader )
	    create_wm_client_leader();

	wm_hints.window_group = X11->wm_client_leader;
	wm_hints.flags |= WindowGroupHint;

	XClassHint class_hint;
	class_hint.res_name = (char *) qAppName(); // application name
	class_hint.res_class = (char *) qAppClass();	// application class

       	XSetWMProperties( dpy, id, 0, 0, 0, 0, &size_hints, &wm_hints, &class_hint );

	XResizeWindow( dpy, id, crect.width(), crect.height() );
	XStoreName( dpy, id, qAppName() );
	Atom protocols[4];
	int n = 0;
	protocols[n++] = ATOM(Wm_Delete_Window);	// support del window protocol
	protocols[n++] = ATOM(Wm_Take_Focus);	// support take focus window protocol
	protocols[n++] = ATOM(Net_Wm_Ping);	// support _NET_WM_PING protocol
	if ( testWFlags( WStyle_ContextHelp ) )
	    protocols[n++] = ATOM(Net_Wm_Context_Help);
	XSetWMProtocols( dpy, id, protocols, n );

        // set mwm hints
        if ( mwmhints.flags != 0l )
            XChangeProperty(dpy, id, ATOM(Motif_Wm_Hints), ATOM(Motif_Wm_Hints), 32,
                            PropModeReplace, (unsigned char *) &mwmhints, 5);

	// set _NET_WM_WINDOW_TYPE
	if (curr_wintype > 0)
	    XChangeProperty(dpy, id, ATOM(Net_Wm_Window_Type), XA_ATOM, 32, PropModeReplace,
			    (unsigned char *) net_wintypes, curr_wintype);

	// set _NET_WM_WINDOW_STATE
	if (curr_winstate > 0)
	    XChangeProperty(dpy, id, ATOM(Net_Wm_State), XA_ATOM, 32, PropModeReplace,
			    (unsigned char *) net_winstates, curr_winstate);

	// set _NET_WM_PID
	long curr_pid = getpid();
	XChangeProperty(dpy, id, ATOM(Net_Wm_Pid), XA_CARDINAL, 32, PropModeReplace,
			(unsigned char *) &curr_pid, 1);

	// when we create a toplevel widget, the frame strut should be dirty
	fstrut_dirty = 1;

	// declare the widget's object name as window role
	XChangeProperty( dpy, id,
			 ATOM(Wm_Window_Role), XA_STRING, 8, PropModeReplace,
			 (unsigned char *)name(), qstrlen( name() ) );

	// set client leader property
	XChangeProperty( dpy, id, ATOM(Wm_Client_Leader),
			 XA_WINDOW, 32, PropModeReplace,
			 (unsigned char *)&X11->wm_client_leader, 1 );
    } else {
	// non-toplevel widgets don't have a frame, so no need to
	// update the strut
	fstrut_dirty = 0;
    }

    if ( initializeWindow ) {
	// don't erase when resizing
	wsa.bit_gravity = QApplication::reverseLayout() ? NorthEastGravity : NorthWestGravity;
	XChangeWindowAttributes( dpy, id, CWBitGravity, &wsa );
    }

    // set X11 event mask
    if (desktop) {
	QWidget* main_desktop = find( id );
	if ( main_desktop->testWFlags(WPaintDesktop) )
	    XSelectInput( dpy, id, stdDesktopEventMask | ExposureMask );
	else
	    XSelectInput( dpy, id, stdDesktopEventMask );
    } else {
	XSelectInput( dpy, id, stdWidgetEventMask );
#if defined (QT_TABLET_SUPPORT)
 	if ( devStylus != NULL )
 	    XSelectExtensionEvent( dpy, id, event_list_stylus, qt_curr_events_stylus );
	if ( devEraser != NULL )
	    XSelectExtensionEvent( dpy, id, event_list_eraser, qt_curr_events_eraser );
#endif
    }

    if ( desktop ) {
	setWState( WState_Visible );
    } else if ( topLevel ) {			// set X cursor
	setAttribute( WA_SetCursor );
	if ( initializeWindow )
	    qt_x11_enforce_cursor( this );
    }

    if ( destroyw )
	qt_XDestroyWindow( this, dpy, destroyw );
}


/*!
    Frees up window system resources. Destroys the widget window if \a
    destroyWindow is TRUE.

    destroy() calls itself recursively for all the child widgets,
    passing \a destroySubWindows for the \a destroyWindow parameter.
    To have more control over destruction of subwidgets, destroy
    subwidgets selectively first.

    This function is usually called from the QWidget destructor.
*/

void QWidget::destroy( bool destroyWindow, bool destroySubWindows )
{
    deactivateWidgetCleanup();
    if ( testWState(WState_Created) ) {
	clearWState( WState_Created );
	QObjectList childs = children();
	for (int i = 0; i < childs.size(); ++i) { // destroy all widget children
	    register QObject *obj = childs.at(i);
	    if ( obj->isWidgetType() )
		static_cast<QWidget*>(obj)->destroy(destroySubWindows,
						    destroySubWindows);
	}
	if ( mouseGrb == this )
	    releaseMouse();
	if ( keyboardGrb == this )
	    releaseKeyboard();
	if ( isTopLevel() )
	    X11->deferred_map.take(this);
	if ( testWFlags(WShowModal) )		// just be sure we leave modal
	    qt_leave_modal( this );
	else if ( testWFlags(WType_Popup) )
	    qApp->closePopup( this );

#ifndef QT_NO_XFTFREETYPE
	if ( rendhd) {
	    if ( destroyWindow )
		XftDrawDestroy( (XftDraw *) rendhd );
	    else
		free( (void*) rendhd );
	    rendhd = 0;
	}
#endif // QT_NO_XFTFREETYPE

	if ( testWFlags(WType_Desktop) ) {
	    if ( acceptDrops() )
		qt_dnd_enable( this, FALSE );
	} else {
	    if ( destroyWindow )
		qt_XDestroyWindow( this, x11Display(), winid );
	}
	setWinId( 0 );

	extern void qPRCleanup( QWidget *widget ); // from qapplication_x11.cpp
	if ( testWState(WState_Reparented) )
	    qPRCleanup(this);
#ifdef Q_Q4PAINTER
	delete deviceGC;
	deviceGC = 0;
#endif
    }
}

void QWidget::reparent_helper( QWidget *parent, WFlags f, const QPoint &p, bool showIt )
{
    extern void qPRCreate( const QWidget *, Window );

    Display *dpy = x11Display();
    QCursor oldcurs;
    bool setcurs = testAttribute(WA_SetCursor);
    if ( setcurs ) {
	oldcurs = cursor();
	unsetCursor();
    }

    // dnd unregister (we will register again below)
    bool accept_drops = acceptDrops();
    setAcceptDrops( FALSE );

    QWidget* oldtlw = topLevelWidget();
    QWidget *oldparent = parentWidget();
    WId old_winid = winid;
    if ( testWFlags(WType_Desktop) )
	old_winid = 0;
    setWinId( 0 );

    // hide and reparent our own window away. Otherwise we might get
    // destroyed when emitting the child remove event below. See QWorkspace.
    XUnmapWindow( x11Display(), old_winid );
    XReparentWindow( x11Display(), old_winid,
		     RootWindow( x11Display(), x11Screen() ), 0, 0 );

    if ( isTopLevel() ) {
        // input contexts are associated with toplevel widgets, so we need
        // destroy the context here.  if we are reparenting back to toplevel,
        // then we will have another context created, otherwise we will
        // use our new toplevel's context
        d->destroyInputContext();
    }

    if ( isTopLevel() || !parent ) // we are toplevel, or reparenting to toplevel
        d->topData()->parentWinId = 0;

    QObject::setParent_helper(parent);
    bool     enable = isEnabled();		// remember status
    FocusPolicy fp = focusPolicy();
    QSize    s	    = size();
    QString capt = windowTitle();
    widget_flags = f;
    clearWState(WState_Created | WState_Visible | WState_Hidden | WState_ExplicitShowHide);
    create();
    if ( isTopLevel() || (!parent || parent->isVisible() ) )
	setWState(WState_Hidden);

    QObjectList chlist = children();
    for (int i = 0; i < chlist.size(); ++i) { // reparent children
	QObject *obj = chlist.at(i);
	if ( obj->isWidgetType() ) {
	    QWidget *w = (QWidget *)obj;
	    if ( !w->isTopLevel() ) {
		XReparentWindow( x11Display(), w->winId(), winId(),
				 w->geometry().x(), w->geometry().y() );
	    } else if ( w->isPopup()
			|| w->testWFlags(WStyle_DialogBorder)
			|| w->testWFlags(WType_Dialog)
			|| w->testWFlags(WStyle_Tool) ) {
		/*
		  when reparenting toplevel windows with toplevel-transient children,
		  we need to make sure that the window manager gets the updated
		  WM_TRANSIENT_FOR information... unfortunately, some window managers
		  don't handle changing WM_TRANSIENT_FOR before the toplevel window is
		  visible, so we unmap and remap all toplevel-transient children *after*
		  the toplevel parent has been mapped.  thankfully, this is easy in Qt :)
		*/
		XUnmapWindow(w->x11Display(), w->winId());
		XSetTransientForHint(w->x11Display(), w->winId(), winId());
		QApplication::postEvent(w, new QEvent(QEvent::ShowWindowRequest));
	    }
	}
    }
    qPRCreate( this, old_winid );
    d->updateSystemBackground();

    setGeometry( p.x(), p.y(), s.width(), s.height() );
    setEnabled( enable );
    setFocusPolicy( fp );
    if ( !capt.isNull() ) {
	d->extra->topextra->caption = QString::null;
	setWindowTitle( capt );
    }
    if ( showIt )
	show();
    if ( old_winid )
	qt_XDestroyWindow( this, dpy, old_winid );
    if ( setcurs )
	setCursor(oldcurs);

    reparentFocusWidgets( oldtlw );

    // re-register dnd
    if (oldparent)
	oldparent->d->checkChildrenDnd();

    if ( accept_drops )
	setAcceptDrops( TRUE );
    else {
	d->checkChildrenDnd();
	d->topData()->dnd = 0;
	qt_dnd_enable(this, (d->extra && d->extra->children_use_dnd));
    }
}


/*!
    Translates the widget coordinate \a pos to global screen
    coordinates. For example, \c{mapToGlobal(QPoint(0,0))} would give
    the global coordinates of the top-left pixel of the widget.

    \sa mapFromGlobal() mapTo() mapToParent()
*/

QPoint QWidget::mapToGlobal( const QPoint &pos ) const
{
    int	   x, y;
    Window child;
    XTranslateCoordinates( x11Display(), winId(),
			   QApplication::desktop()->screen(x11Screen())->winId(),
			   pos.x(), pos.y(), &x, &y, &child );
    return QPoint( x, y );
}

/*!
    Translates the global screen coordinate \a pos to widget
    coordinates.

    \sa mapToGlobal() mapFrom() mapFromParent()
*/

QPoint QWidget::mapFromGlobal( const QPoint &pos ) const
{
    int	   x, y;
    Window child;
    XTranslateCoordinates( x11Display(),
			   QApplication::desktop()->screen(x11Screen())->winId(),
			   winId(), pos.x(), pos.y(), &x, &y, &child );
    return QPoint( x, y );
}

/*!
    When a widget gets focus, it should call setMicroFocusHint() with
    some appropriate position and size, \a x, \a y, \a width and \a
    height. This has no \e visual effect, it just provides hints to
    any system-specific input handling tools.

    The \a text argument should be TRUE if this is a position for text
    input.

    In the Windows version of Qt, this method sets the system caret,
    which is used for user Accessibility focus handling.  If \a text
    is TRUE, it also sets the IME composition window in Far East Asian
    language input systems.

    In the X11 version of Qt, if \a text is TRUE, this method sets the
    XIM "spot" point for complex language input handling.

    The font \a f is a rendering hint to the currently active input method.
    If \a f is 0 the widget's font is used.

    \sa microFocusHint()
*/
void QWidget::setMicroFocusHint(int x, int y, int width, int height,
				bool text, QFont *f )
{
#ifndef QT_NO_XIM
    if ( text ) {
	QWidget* tlw = topLevelWidget();
	QTLWExtra *topdata = tlw->d->topData();

	// trigger input context creation if it hasn't happened already
	d->createInputContext();
	QInputContext *qic = (QInputContext *) topdata->xic;

	if ( X11->xim && qic ) {
	    QPoint p( x, y );
	    QPoint p2 = mapTo( topLevelWidget(), QPoint( 0, 0 ) );
	    p = mapTo( topLevelWidget(), p);
	    qic->setXFontSet( f ? *f : fnt );
	    qic->setComposePosition(p.x(), p.y() + height);
	    qic->setComposeArea(p2.x(), p2.y(), this->width(), this->height());
	}
    }
#endif

    if ( QRect( x, y, width, height ) != microFocusHint() ) {
	d->createExtra();
	d->extraData()->micro_focus_hint.setRect( x, y, width, height );
    }
}


void QWidgetPrivate::setFont_syshelper( QFont * )
{
    // Nothing
}

void QWidgetPrivate::updateSystemBackground()
{
    QBrush brush = q->palette().brush(q->backgroundRole());
    if (brush.style() == Qt::NoBrush || q->testAttribute(QWidget::WA_NoSystemBackground))
	XSetWindowBackgroundPixmap(q->x11Display(), q->winId(), None);
    else if (brush.pixmap())
	XSetWindowBackgroundPixmap(q->x11Display(), q->winId(),
				   isBackgroundInherited()
				   ? ParentRelative
				   : brush.pixmap()->handle());
    else
	XSetWindowBackground( q->x11Display(), q->winId(), brush.color().pixel(q->x11Screen()) );
}

void QWidget::setCursor( const QCursor &cursor )
{
    if (cursor.shape() != Qt::ArrowCursor
	|| (d->extra && d->extra->curs)) {
	d->createExtra();
	delete d->extra->curs;
	d->extra->curs = new QCursor(cursor);
    }
    setAttribute(WA_SetCursor);
    qt_x11_enforce_cursor(this);
    XFlush(x11Display());
}

void QWidget::unsetCursor()
{
    if ( d->extra ) {
	delete d->extra->curs;
	d->extra->curs = 0;
    }
    if ( !isTopLevel() )
	setAttribute(WA_SetCursor, false);
    qt_x11_enforce_cursor( this );
    XFlush( x11Display() );
}

static XTextProperty*
qstring_to_xtp( const QString& s )
{
    static XTextProperty tp = { 0, 0, 0, 0 };
    static bool free_prop = TRUE; // we can't free tp.value in case it references
    // the data of the static QCString below.
    if ( tp.value ) {
	if ( free_prop )
	    XFree( tp.value );
	tp.value = 0;
	free_prop = TRUE;
    }

    static const QTextCodec* mapper = QTextCodec::codecForLocale();
    int errCode = 0;
    if ( mapper ) {
	QByteArray mapped = mapper->fromUnicode(s);
	char* tl[2];
	tl[0] = mapped.data();
	tl[1] = 0;
	errCode = XmbTextListToTextProperty( QPaintDevice::x11AppDisplay(),
					     tl, 1, XStdICCTextStyle, &tp );
#if defined(QT_DEBUG)
	if ( errCode < 0 )
	    qDebug( "qstring_to_xtp result code %d", errCode );
#endif
    }
    if ( !mapper || errCode < 0 ) {
	static QByteArray qcs;
	qcs = s.ascii();
	tp.value = (uchar*)qcs.data();
	tp.encoding = XA_STRING;
	tp.format = 8;
	tp.nitems = qcs.length();
	free_prop = FALSE;
    }

    // ### If we knew WM could understand unicode, we could use
    // ### a much simpler, cheaper encoding...
    /*
	tp.value = (XChar2b*)s.unicode();
	tp.encoding = XA_UNICODE; // wish
	tp.format = 16;
	tp.nitems = s.length();
    */

    return &tp;
}

void QWidget::setWindowModified(bool mod)
{
    setAttribute(WA_WindowModified, mod);
    QEvent e(QEvent::ModifiedChange);
    QApplication::sendEvent(this, &e);
}

bool QWidget::isWindowModified() const
{
    return testAttribute(WA_WindowModified);
}

void QWidget::setWindowTitle( const QString &caption )
{
    if ( QWidget::windowTitle() == caption )
	return;

    d->topData()->caption = caption;
    XSetWMName( x11Display(), winId(), qstring_to_xtp(caption) );

    QByteArray net_wm_name = caption.toUtf8();
    XChangeProperty(x11Display(), winId(), ATOM(Net_Wm_Name), ATOM(Utf8_String), 8,
		    PropModeReplace, (unsigned char *)net_wm_name.data(), net_wm_name.size());

    QEvent e( QEvent::WindowTitleChange );
    QApplication::sendEvent( this, &e );
}

void QWidget::setWindowIcon( const QPixmap &pixmap )
{
    if ( d->extra && d->extra->topextra ) {
	delete d->extra->topextra->icon;
	d->extra->topextra->icon = 0;
    } else {
	d->createTLExtra();
    }
    Pixmap icon_pixmap = 0;
    Pixmap mask_pixmap = 0;
    if ( !pixmap.isNull() ) {
	QPixmap* pm = new QPixmap( pixmap );
	d->extra->topextra->icon = pm;
	if ( !pm->mask() )
	    pm->setMask( pm->createHeuristicMask() ); // may do detach()
	icon_pixmap = pm->handle();
	if ( pm->mask() )
	    mask_pixmap = pm->mask()->handle();
    }
    XWMHints *h = XGetWMHints( x11Display(), winId() );
    XWMHints  wm_hints;
    bool got_hints = h != 0;
    if ( !got_hints ) {
	h = &wm_hints;
	h->flags = 0;
    }
    h->icon_pixmap = icon_pixmap;
    h->icon_mask = mask_pixmap;
    h->flags |= IconPixmapHint | IconMaskHint;
    XSetWMHints( x11Display(), winId(), h );
    if ( got_hints )
	XFree( (char *)h );
    QEvent e( QEvent::WindowIconChange );
    QApplication::sendEvent( this, &e );
}

void QWidget::setWindowIconText( const QString &iconText )
{
    d->createTLExtra();
    d->extra->topextra->iconText = iconText;

    XSetWMIconName( x11Display(), winId(), qstring_to_xtp(iconText) );

    QByteArray icon_name = iconText.toUtf8();
    XChangeProperty(x11Display(), winId(), ATOM(Net_Wm_Icon_Name), ATOM(Utf8_String), 8,
		    PropModeReplace, (unsigned char *) icon_name.data(), icon_name.size());

    QEvent e( QEvent::IconTextChange );
    QApplication::sendEvent( this, &e );
}


/*!
    Grabs the mouse input.

    This widget receives all mouse events until releaseMouse() is
    called; other widgets get no mouse events at all. Keyboard
    events are not affected. Use grabKeyboard() if you want to grab
    that.

    \warning Bugs in mouse-grabbing applications very often lock the
    terminal. Use this function with extreme caution, and consider
    using the \c -nograb command line option while debugging.

    It is almost never necessary to grab the mouse when using Qt, as
    Qt grabs and releases it sensibly. In particular, Qt grabs the
    mouse when a mouse button is pressed and keeps it until the last
    button is released.

    Note that only visible widgets can grab mouse input. If
    isVisible() returns FALSE for a widget, that widget cannot call
    grabMouse().

    \sa releaseMouse() grabKeyboard() releaseKeyboard() grabKeyboard()
    focusWidget()
*/

void QWidget::grabMouse()
{
    if ( isVisible() && !qt_nograb() ) {
	if ( mouseGrb )
	    mouseGrb->releaseMouse();
#ifndef QT_NO_DEBUG
	int status =
#endif
	    XGrabPointer( x11Display(), winId(), False,
			  (uint)( ButtonPressMask | ButtonReleaseMask |
				  PointerMotionMask | EnterWindowMask |
				  LeaveWindowMask ),
			  GrabModeAsync, GrabModeAsync,
			  None, None, qt_x_time );
#ifndef QT_NO_DEBUG
	if ( status ) {
	    const char *s =
		status == GrabNotViewable ? "\"GrabNotViewable\"" :
		status == AlreadyGrabbed  ? "\"AlreadyGrabbed\"" :
		status == GrabFrozen      ? "\"GrabFrozen\"" :
		status == GrabInvalidTime ? "\"GrabInvalidTime\"" :
		"<?>";
	    qWarning( "Grabbing the mouse failed with %s", s );
	}
#endif
	mouseGrb = this;
    }
}

/*!
    \overload

    Grabs the mouse input and changes the cursor shape.

    The cursor will assume shape \a cursor (for as long as the mouse
    focus is grabbed) and this widget will be the only one to receive
    mouse events until releaseMouse() is called().

    \warning Grabbing the mouse might lock the terminal.

    \sa releaseMouse(), grabKeyboard(), releaseKeyboard(), setCursor()
*/

void QWidget::grabMouse( const QCursor &cursor )
{
    if ( !qt_nograb() ) {
	if ( mouseGrb )
	    mouseGrb->releaseMouse();
#ifndef QT_NO_DEBUG
	int status =
#endif
	XGrabPointer( x11Display(), winId(), False,
		      (uint)(ButtonPressMask | ButtonReleaseMask |
			     PointerMotionMask | EnterWindowMask | LeaveWindowMask),
		      GrabModeAsync, GrabModeAsync,
		      None, cursor.handle(), qt_x_time );
#ifndef QT_NO_DEBUG
	if ( status ) {
	    const char *s =
		status == GrabNotViewable ? "\"GrabNotViewable\"" :
		status == AlreadyGrabbed  ? "\"AlreadyGrabbed\"" :
		status == GrabFrozen      ? "\"GrabFrozen\"" :
		status == GrabInvalidTime ? "\"GrabInvalidTime\"" :
					    "<?>";
	    qWarning( "Grabbing the mouse failed with %s", s );
	}
#endif
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
	XUngrabPointer( x11Display(),  qt_x_time );
	XFlush( x11Display() );
	mouseGrb = 0;
    }
}

/*!
    Grabs the keyboard input.

    This widget reveives all keyboard events until releaseKeyboard()
    is called; other widgets get no keyboard events at all. Mouse
    events are not affected. Use grabMouse() if you want to grab that.

    The focus widget is not affected, except that it doesn't receive
    any keyboard events. setFocus() moves the focus as usual, but the
    new focus widget receives keyboard events only after
    releaseKeyboard() is called.

    If a different widget is currently grabbing keyboard input, that
    widget's grab is released first.

    \sa releaseKeyboard() grabMouse() releaseMouse() focusWidget()
*/

void QWidget::grabKeyboard()
{
    if ( !qt_nograb() ) {
	if ( keyboardGrb )
	    keyboardGrb->releaseKeyboard();
	XGrabKeyboard( x11Display(), winid, False, GrabModeAsync, GrabModeAsync,
		       qt_x_time );
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
	XUngrabKeyboard( x11Display(), qt_x_time );
	keyboardGrb = 0;
    }
}


/*!
    Returns the widget that is currently grabbing the mouse input.

    If no widget in this application is currently grabbing the mouse,
    0 is returned.

    \sa grabMouse(), keyboardGrabber()
*/

QWidget *QWidget::mouseGrabber()
{
    return mouseGrb;
}

/*!
    Returns the widget that is currently grabbing the keyboard input.

    If no widget in this application is currently grabbing the
    keyboard, 0 is returned.

    \sa grabMouse(), mouseGrabber()
*/

QWidget *QWidget::keyboardGrabber()
{
    return keyboardGrb;
}

/*!
    Sets the top-level widget containing this widget to be the active
    window.

    An active window is a visible top-level window that has the
    keyboard input focus.

    This function performs the same operation as clicking the mouse on
    the title bar of a top-level window. On X11, the result depends on
    the Window Manager. If you want to ensure that the window is
    stacked on top as well you should also call raise(). Note that the
    window must be visible, otherwise setActiveWindow() has no effect.

    On Windows, if you are calling this when the application is not
    currently the active one then it will not make it the active
    window.  It will flash the task bar entry blue to indicate that
    the window has done something. This is because Microsoft do not
    allow an application to interrupt what the user is currently doing
    in another application.

    \sa isActiveWindow(), topLevelWidget(), show()
*/

void QWidget::setActiveWindow()
{
    QWidget *tlw = topLevelWidget();
    if ( tlw->isVisible() && !tlw->d->topData()->embedded ) {
	XSetInputFocus( x11Display(), tlw->winId(), RevertToNone, qt_x_time);

#ifndef QT_NO_XIM
	// trigger input context creation if it hasn't happened already
	d->createInputContext();

	if (tlw->d->topData()->xic) {
	    QInputContext *qic = (QInputContext *) tlw->d->topData()->xic;
	    qic->setFocus();
	}
#endif
    }
}


void QWidget::update()
{
    if ((widget_state & (WState_Visible|WState_BlockUpdates)) == WState_Visible ) {
// 	d->removePendingPaintEvents(); // ### this is far too slow to go in
	d->invalidated_region = d->clipRect();
	QApplication::postEvent(this, new QEvent(QEvent::UpdateRequest));
    }
}

void QWidget::update(const QRegion &rgn)
{
    if ((widget_state & (WState_Visible|WState_BlockUpdates)) == WState_Visible) {
	d->invalidated_region |= (rgn & d->clipRect());
	QApplication::postEvent(this, new QEvent(QEvent::UpdateRequest));
    }
}

void QWidget::update(int x, int y, int w, int h)
{
    if (w && h && (widget_state & (WState_Visible|WState_BlockUpdates)) == WState_Visible) {
	if ( w < 0 )
	    w = crect.width()  - x;
	if ( h < 0 )
	    h = crect.height() - y;
	if ( w != 0 && h != 0 ) {
	    d->invalidated_region |= (d->clipRect() & QRect(x, y, w, h));
	    QApplication::postEvent(this, new QEvent(QEvent::UpdateRequest));
	}
    }
}

struct QX11DoubleBuffer
{
    enum {
	MaxWidth  = SHRT_MAX,
	MaxHeight = SHRT_MAX
    };

    Qt::HANDLE hd, rendhd;
    int screen, depth;
    int width, height;
};
static QX11DoubleBuffer *global_double_buffer = 0;

void qt_discard_double_buffer()
{
    if (!global_double_buffer) return;

    XFreePixmap(QPaintDevice::x11AppDisplay(), global_double_buffer->hd);
#ifndef QT_NO_XFTFREETYPE
    if (X11->use_xrender && X11->has_xft)
	XftDrawDestroy((XftDraw *) global_double_buffer->rendhd);
#endif

    delete global_double_buffer;
    global_double_buffer = 0;
}

static
void qt_x11_get_double_buffer(Qt::HANDLE &hd, Qt::HANDLE &rendhd,
			      int screen, int depth, int width, int height)
{
    // the db should consist of 128x128 chunks
    width  = qMin((( width / 128) + 1) * 128, (int)QX11DoubleBuffer::MaxWidth);
    height = qMin(((height / 128) + 1) * 128, (int)QX11DoubleBuffer::MaxHeight);

    if (global_double_buffer) {
	if (global_double_buffer->screen == screen
	    && global_double_buffer->depth == depth
	    && global_double_buffer->width >= width
	    && global_double_buffer->height >= height) {
	    hd = global_double_buffer->hd;
	    rendhd = global_double_buffer->rendhd;
	    return;
	}

	width  = qMax(global_double_buffer->width,  width);
	height = qMax(global_double_buffer->height, height);

	qt_discard_double_buffer();
    }

    global_double_buffer = new QX11DoubleBuffer;
    global_double_buffer->hd =
	XCreatePixmap(QPaintDevice::x11AppDisplay(), hd, width, height, depth);

#ifndef QT_NO_XFTFREETYPE
    if (X11->use_xrender && X11->has_xft)
	global_double_buffer->rendhd =
	    (Qt::HANDLE) XftDrawCreate(QPaintDevice::x11AppDisplay(),
				       global_double_buffer->hd,
				       (Visual *) QPaintDevice::x11AppVisual(),
				       QPaintDevice::x11AppColormap());
#endif

    global_double_buffer->screen = screen;
    global_double_buffer->depth = depth;
    global_double_buffer->width = width;
    global_double_buffer->height = height;

    hd = global_double_buffer->hd;
    rendhd = global_double_buffer->rendhd;
}

void QWidget::repaint(const QRegion& rgn)
{
    if (testWState(WState_InPaintEvent))
	qWarning("QWidget::repaint: recursive repaint detected.");

    if ( (widget_state & (WState_Visible|WState_BlockUpdates)) != WState_Visible
	 || !testAttribute(WA_Mapped) )
	return;

    if (rgn.isEmpty())
	return;

    if (!d->invalidated_region.isEmpty())
	d->invalidated_region -= rgn;

    setWState(WState_InPaintEvent);

    QRect br = rgn.boundingRect();
    bool do_clipping = (br != QRect(0,0,crect.width(),crect.height()));

    QPoint dboff;
    bool double_buffer = (!testAttribute(WA_PaintOnScreen)
			  && br.width()  <= QX11DoubleBuffer::MaxWidth
			  && br.height() <= QX11DoubleBuffer::MaxHeight);

    HANDLE old_hd = hd;
    HANDLE old_rendhd = rendhd;

    if (double_buffer) {
	qt_x11_get_double_buffer(hd, rendhd, x11Screen(), x11Depth(), br.width(), br.height());

	dboff = br.topLeft();
	QPainter::setRedirected(this, this, dboff);

	QRegion region_in_pm(rgn);
	region_in_pm.translate(-dboff);
	if (do_clipping)
	    qt_set_paintevent_clipping(this, region_in_pm);
    } else {
	if (do_clipping)
	    qt_set_paintevent_clipping(this, rgn);
    }

    if (testAttribute(WA_NoSystemBackground)) {
	if (double_buffer) {
	    GC gc = qt_xget_temp_gc(x11Screen(), false);
	    XCopyArea(x11Display(), winId(), hd, gc,
		      br.x(), br.y(), br.width(), br.height(), 0, 0);
	}
    } else if (!testAttribute(WA_NoBackground)) {
	QPoint offset;
	QStack<QWidget*> parents;
	QWidget *w = q;
	while (w->d->isBackgroundInherited()) {
	    offset += w->pos();
	    w = w->parentWidget();
	    parents += w;
	}

	if (double_buffer) {
	    extern void qt_erase_background(Qt::HANDLE, int screen,
					    int x, int y, int width, int height,
					    const QBrush &brush, int offx, int offy);
	    qt_erase_background(q->hd, q->x11Screen(),
				br.x() - dboff.x(), br.y() - dboff.y(),
				br.width(), br.height(), q->pal.brush(w->d->bg_role),
				br.x() + offset.x(), br.y() + offset.y());
	} else {
	    QVector<QRect> rects = rgn.rects();
	    for (int i = 0; i < rects.size(); ++i) {
		const QRect &rr = rects[i];
		XClearArea( q->x11Display(), q->winId(),
			    rr.x(), rr.y(), rr.width(), rr.height(), False );
	    }
	}

	if (!!parents) {
	    w = parents.pop();
	    for (;;) {
		if (w->testAttribute(QWidget::WA_ContentsPropagated)) {
		    QPainter::setRedirected(w, q, offset + dboff);
		    QRect rr = d->clipRect();
		    rr.moveBy(offset);
		    QPaintEvent e(rr);
		    bool was_in_paint_event = w->testWState(WState_InPaintEvent);
		    w->setWState(WState_InPaintEvent);
		    QApplication::sendEvent(w, &e);
		    if(!was_in_paint_event)
			w->clearWState(WState_InPaintEvent);
		    QPainter::restoreRedirected(w);
		}
		if (!parents)
		    break;
		w = parents.pop();
		offset -= w->pos();
	    }
	}
    }

    QPaintEvent e(rgn);
    QApplication::sendSpontaneousEvent( this, &e );

    if (do_clipping)
	qt_clear_paintevent_clipping();

    if (double_buffer) {
	QPainter::restoreRedirected(this);

	GC gc = qt_xget_temp_gc(x11Screen(), false);
	QVector<QRect> rects = rgn.rects();
	for (int i = 0; i < rects.size(); ++i) {
	    const QRect &rr = rects[i];
	    XCopyArea(x11Display(), hd, winId(), gc,
		      rr.x() - br.x(), rr.y() - br.y(),
		      rr.width(), rr.height(),
		      rr.x(), rr.y());
	}

	hd = old_hd;
	rendhd = old_rendhd;

	if (!qApp->active_window) {
	    extern int qt_double_buffer_timer;
	    if (qt_double_buffer_timer)
		qApp->killTimer(qt_double_buffer_timer);
	    qt_double_buffer_timer = qApp->startTimer(500);
	}
    }

    clearWState(WState_InPaintEvent);

    if (testAttribute(WA_ContentsPropagated))
	d->updatePropagatedBackground(&rgn);
}

void QWidget::setWindowState(uint newstate)
{
    bool needShow = FALSE;
    uint oldstate = windowState();
    if (isTopLevel()) {
	if ((oldstate & WindowMaximized) != (newstate & WindowMaximized)) {
	    if (qt_net_supports(ATOM(Net_Wm_State_Maximized_Horz))
		&& qt_net_supports(ATOM(Net_Wm_State_Maximized_Vert))) {
		qt_net_change_wm_state(this, (newstate & WindowMaximized),
				       ATOM(Net_Wm_State_Maximized_Horz),
				       ATOM(Net_Wm_State_Maximized_Vert));
	    } else if (isVisible()) {
		QRect maxRect = QApplication::desktop()->availableGeometry(this);

		// save original geometry
		QTLWExtra *top = d->topData();
		if ( top->normalGeometry.width() < 0 )
		    top->normalGeometry = geometry();

		// the wm was not smart enough to adjust our size, do that manually
		updateFrameStrut();
		setGeometry(maxRect.x() + top->fleft, maxRect.y() + top->ftop,
			    maxRect.width() - top->fleft - top->fright,
			    maxRect.height() - top->ftop - top->fbottom);
	    }
	}

	if ((oldstate & WindowFullScreen) != (newstate & WindowFullScreen)) {
	    if (qt_net_supports(ATOM(Net_Wm_State_FullScreen))) {
		qt_net_change_wm_state(this, (newstate & WindowFullScreen),
				       ATOM(Net_Wm_State_FullScreen));
	    } else {
		needShow = isVisible();

		QTLWExtra *top = d->topData();
		if (newstate & WindowFullScreen) {
		    if ( top->normalGeometry.width() < 0 )
			top->normalGeometry = QRect( pos(), size() );
		    top->savedFlags = getWFlags();
		    reparent(0, WType_TopLevel | WStyle_Customize | WStyle_NoBorder |
			     // preserve some widget flags
			     (getWFlags() & 0xffff0000),
			     mapToGlobal(QPoint(0, 0)));
		    QRect r = qApp->desktop()->screenGeometry(this);
		    move( r.topLeft() );
		    resize( r.size() );
		} else {
		    reparent( 0, top->savedFlags, QPoint(0,0) );
		    QRect r = top->normalGeometry;
		    if ( r.width() >= 0 ) {
			// the widget has been maximized
			top->normalGeometry = QRect(0,0,-1,-1);
			move( r.topLeft() );
			resize( r.size() );
		    }
		}
	    }
	}

	if ((oldstate & WindowMinimized) != (newstate & WindowMinimized)) {
	    if (isVisible()) {
		if (newstate & WindowMinimized) {
		    XEvent e;
		    e.xclient.type = ClientMessage;
		    e.xclient.message_type = ATOM(Wm_Change_State);
		    e.xclient.display = x11Display();
		    e.xclient.window = winid;
		    e.xclient.format = 32;
		    e.xclient.data.l[0] = IconicState;
		    e.xclient.data.l[1] = 0;
		    e.xclient.data.l[2] = 0;
		    e.xclient.data.l[3] = 0;
		    e.xclient.data.l[4] = 0;
		    XSendEvent(x11Display(), RootWindow(x11Display(), x11Screen()),
			       False, (SubstructureNotifyMask|SubstructureRedirectMask), &e);
		} else {
		    XMapWindow(x11Display(), winId());
		}
	    }

	    needShow = false;
	}
    }

    widget_state &= ~(WState_Minimized | WState_Maximized | WState_FullScreen);
    if (newstate & WindowMinimized)
	widget_state |= WState_Minimized;
    if (newstate & WindowMaximized)
	widget_state |= WState_Maximized;
    if (newstate & WindowFullScreen)
	widget_state |= WState_FullScreen;

    if (needShow)
	show();

    if (newstate & WindowActive)
	setActiveWindow();
}

/*!
  \internal
  Platform-specific part of QWidget::show().
*/

void QWidget::showWindow()
{
    if ( isTopLevel()  ) {
	XWMHints *h = XGetWMHints( x11Display(), winId() );
	XWMHints  wm_hints;
	bool got_hints = h != 0;
	if ( !got_hints ) {
	    h = &wm_hints;
	    h->flags = 0;
	}
	h->initial_state = testWState(WState_Minimized) ? IconicState : NormalState;
	h->flags |= StateHint;
	XSetWMHints( x11Display(), winId(), h );
	if ( got_hints )
	    XFree( (char *)h );

	if (qt_x_user_time != CurrentTime) {
	    XChangeProperty(x11Display(), winId(), ATOM(Net_Wm_User_Time), XA_CARDINAL,
			    32, PropModeReplace, (unsigned char *) &qt_x_user_time, 1);
	}

	if ( d->d->topData()->parentWinId &&
	     d->topData()->parentWinId != QPaintDevice::x11AppRootWindow(x11Screen()) &&
	     !isMinimized() ) {
	    X11->deferred_map.append(this);
	    return;
	}

	if (isMaximized() && !(qt_net_supports(ATOM(Net_Wm_State_Maximized_Horz))
			       && qt_net_supports(ATOM(Net_Wm_State_Maximized_Vert)))) {
	    QRect maxRect = QApplication::desktop()->availableGeometry(this);

	    // save original geometry
	    QTLWExtra *top = d->topData();
	    if (top->normalGeometry.width() < 0)
		top->normalGeometry = geometry();
	    setGeometry(maxRect);

	    XMapWindow( x11Display(), winId() );
	    qt_wait_for_window_manager(this);

	    // the wm was not smart enough to adjust our size, do that manually
	    updateFrameStrut();
	    setGeometry(maxRect.x() + top->fleft, maxRect.y() + top->ftop,
			maxRect.width() - top->fleft - top->fright,
			maxRect.height() - top->ftop - top->fbottom);
	    return;
	}
    }
    XMapWindow( x11Display(), winId() );
}


/*!
  \internal
  Platform-specific part of QWidget::hide().
*/

void QWidget::hideWindow()
{
    clearWState( WState_Exposed );
    deactivateWidgetCleanup();
    if ( isTopLevel() ) {
	X11->deferred_map.take(this);
	if ( winId() ) // in nsplugin, may be 0
	    XWithdrawWindow( x11Display(), winId(), x11Screen() );

	QTLWExtra *top = d->topData();
	crect.moveTopLeft( QPoint(crect.x() - top->fleft, crect.y() - top->ftop ) );

	// zero the frame strut and mark it dirty
	top->fleft = top->fright = top->ftop = top->fbottom = 0;
	fstrut_dirty = TRUE;

	XFlush( x11Display() );
    } else {
	if ( winId() ) // in nsplugin, may be 0
	    XUnmapWindow( x11Display(), winId() );
    }
}

/*!
    Raises this widget to the top of the parent widget's stack.

    After this call the widget will be visually in front of any
    overlapping sibling widgets.

    \sa lower(), stackUnder()
*/

void QWidget::raise()
{
    QWidget *p = parentWidget();
    if ( p && p->d->children.findIndex(this) >= 0 ) {
	p->d->children.remove(this);
	p->d->children.append(this);
    }
    XRaiseWindow( x11Display(), winId() );
}

/*!
    Lowers the widget to the bottom of the parent widget's stack.

    After this call the widget will be visually behind (and therefore
    obscured by) any overlapping sibling widgets.

    \sa raise(), stackUnder()
*/

void QWidget::lower()
{
    QWidget *p = parentWidget();
    if ( p && p->d->children.findIndex(this) >= 0 ) {
	p->d->children.remove(this);
	p->d->children.prepend(this);
    }
    XLowerWindow( x11Display(), winId() );
}


/*!
    Places the widget under \a w in the parent widget's stack.

    To make this work, the widget itself and \a w must be siblings.

    \sa raise(), lower()
*/
void QWidget::stackUnder( QWidget* w)
{
    QWidget *p = parentWidget();
    if ( !w || isTopLevel() || p != w->parentWidget() || this == w )
	return;
    if ( p && p->d->children.findIndex(w) >= 0 && p->d->children.findIndex(this) >= 0 ) {
	p->d->children.remove(this);
	p->d->children.insert(p->d->children.findIndex(w), this);
    }
    Window stack[2];
    stack[0] = w->winId();;
    stack[1] = winId();
    XRestackWindows( x11Display(), stack, 2 );
}



/*
  The global variable qt_widget_tlw_gravity defines the window gravity of
  the next top level window to be created. We do this when setting the
  main widget's geometry and the "-geometry" command line option contains
  a negative position.
*/

// ### this should be set to NorthEast for reversed layout!
int qt_widget_tlw_gravity = NorthWestGravity;

static void do_size_hints( QWidget* widget, QWExtra *x )
{
    XSizeHints s;
    s.flags = 0;
    if ( x ) {
	s.x = widget->x();
	s.y = widget->y();
	s.width = widget->width();
	s.height = widget->height();
	if ( x->minw > 0 || x->minh > 0 ) {	// add minimum size hints
	    s.flags |= PMinSize;
	    s.min_width  = x->minw;
	    s.min_height = x->minh;
	}
	s.flags |= PMaxSize;		// add maximum size hints
	s.max_width  = x->maxw;
	s.max_height = x->maxh;
	if ( x->topextra &&
	   (x->topextra->incw > 0 || x->topextra->inch > 0) )
	{					// add resize increment hints
	    s.flags |= PResizeInc | PBaseSize;
	    s.width_inc = x->topextra->incw;
	    s.height_inc = x->topextra->inch;
	    s.base_width = x->topextra->basew;
	    s.base_height = x->topextra->baseh;
	}

	if ( x->topextra && x->topextra->uspos) {
	    s.flags |= USPosition;
	    s.flags |= PPosition;
	}
	if ( x->topextra && x->topextra->ussize) {
	    s.flags |= USSize;
	    s.flags |= PSize;
	}
    }
    s.flags |= PWinGravity;
    s.win_gravity = qt_widget_tlw_gravity;	// usually NorthWest
    // ### should this be set to NE for right to left languages.
    qt_widget_tlw_gravity = NorthWestGravity;	// reset in case it was set
    XSetWMNormalHints( widget->x11Display(), widget->winId(), &s );
}


void QWidget::setGeometry_helper( int x, int y, int w, int h, bool isMove )
{
    Display *dpy = x11Display();

    if ( testWFlags(WType_Desktop) )
	return;
    clearWState(WState_Maximized);
    if ( d->extra ) {				// any size restrictions?
	w = qMin(w,d->extra->maxw);
	h = qMin(h,d->extra->maxh);
	w = qMax(w,d->extra->minw);
	h = qMax(h,d->extra->minh);
    }
    if ( w < 1 )				// invalid size
	w = 1;
    if ( h < 1 )
	h = 1;
    QPoint oldPos( pos() );
    QSize oldSize( size() );
    QRect oldGeom( crect );
    QRect  r( x, y, w, h );

    // We only care about stuff that changes the geometry, or may
    // cause the window manager to change its state
    if ( !isTopLevel() && oldGeom == r )
	return;

    crect = r;
    bool isResize = size() != oldSize;

    if ( isTopLevel() ) {
	if ( isMove )
	    d->topData()->uspos = 1;
	if ( isResize )
	    d->topData()->ussize = 1;
	do_size_hints( this, d->extra );
    }

    if ( isMove ) {
	if (! qt_broken_wm)
	    // pos() is right according to ICCCM 4.1.5
	    XMoveResizeWindow( dpy, winid, pos().x(), pos().y(), w, h );
	else
	    // work around 4Dwm's incompliance with ICCCM 4.1.5
	    XMoveResizeWindow( dpy, winid, x, y, w, h );
    } else if ( isResize )
	XResizeWindow( dpy, winid, w, h );

    if ( isVisible() ) {
	if ( isMove && pos() != oldPos ) {
	    if ( ! qt_broken_wm ) {
		// pos() is right according to ICCCM 4.1.5
		QMoveEvent e( pos(), oldPos );
		QApplication::sendEvent( this, &e );
	    } else {
		// work around 4Dwm's incompliance with ICCCM 4.1.5
		QMoveEvent e( crect.topLeft(), oldGeom.topLeft() );
		QApplication::sendEvent( this, &e );
	    }
	}
	if ( isResize ) {
	    // set config pending only on resize, see qapplication_x11.cpp, translateConfigEvent()
	    setWState( WState_ConfigPending );

	    QResizeEvent e( size(), oldSize );
	    QApplication::sendEvent( this, &e );

	    // Process events immediately rather than in
	    // translateConfigEvent to avoid message process delay.
	    if (!testAttribute(WA_StaticContents))
		testWState(WState_InPaintEvent)?update():repaint();
	}
    } else {
	if (isMove && pos() != oldPos)
	    setAttribute(WA_PendingMoveEvent, true);
	if (isResize)
	    setAttribute(WA_PendingResizeEvent, true);
    }
}


/*!
    \overload

    This function corresponds to setMinimumSize( QSize(minw, minh) ).
    Sets the minimum width to \a minw and the minimum height to \a
    minh.
*/

void QWidget::setMinimumSize( int minw, int minh )
{
    if ( minw < 0 || minh < 0 )
	qWarning("QWidget::setMinimumSize: The smallest allowed size is (0,0)");
    d->createExtra();
    if ( d->extra->minw == minw && d->extra->minh == minh )
	return;
    d->extra->minw = minw;
    d->extra->minh = minh;
    if ( minw > width() || minh > height() ) {
	bool resized = testAttribute(WA_Resized);
	resize( qMax(minw,width()), qMax(minh,height()) );
	setAttribute(WA_Resized, resized); //not a user resize
    }
    if ( testWFlags(WType_TopLevel) )
	do_size_hints( this, d->extra );
    updateGeometry();
}

/*!
    \overload

    This function corresponds to setMaximumSize( QSize(\a maxw, \a
    maxh) ). Sets the maximum width to \a maxw and the maximum height
    to \a maxh.
*/
void QWidget::setMaximumSize( int maxw, int maxh )
{
    if ( maxw > QWIDGETSIZE_MAX || maxh > QWIDGETSIZE_MAX ) {
	qWarning("QWidget::setMaximumSize: (%s/%s) "
		"The largest allowed size is (%d,%d)",
		 name( "unnamed" ), className(), QWIDGETSIZE_MAX,
		QWIDGETSIZE_MAX );
	maxw = qMin( maxw, QWIDGETSIZE_MAX );
	maxh = qMin( maxh, QWIDGETSIZE_MAX );
    }
    if ( maxw < 0 || maxh < 0 ) {
	qWarning("QWidget::setMaximumSize: (%s/%s) Negative sizes (%d,%d) "
		"are not possible",
		name( "unnamed" ), className(), maxw, maxh );
	maxw = qMax( maxw, 0 );
	maxh = qMax( maxh, 0 );
    }
    d->createExtra();
    if ( d->extra->maxw == maxw && d->extra->maxh == maxh )
	return;
    d->extra->maxw = maxw;
    d->extra->maxh = maxh;
    if ( maxw < width() || maxh < height() ) {
	bool resized = testAttribute(WA_Resized);
	resize( qMin(maxw,width()), qMin(maxh,height()) );
	setAttribute(WA_Resized, resized); //not a user resize
    }
    if ( testWFlags(WType_TopLevel) )
	do_size_hints( this, d->extra );
    updateGeometry();
}

/*!
    \overload

    Sets the x (width) size increment to \a w and the y (height) size
    increment to \a h.
*/
void QWidget::setSizeIncrement( int w, int h )
{
    QTLWExtra* x = d->topData();
    if ( x->incw == w && x->inch == h )
	return;
    x->incw = w;
    x->inch = h;
    if ( testWFlags(WType_TopLevel) )
	do_size_hints( this, d->extra );
}

/*!
    \overload

    This corresponds to setBaseSize( QSize(\a basew, \a baseh) ). Sets
    the widgets base size to width \a basew and height \a baseh.
*/
void QWidget::setBaseSize( int basew, int baseh )
{
    d->createTLExtra();
    QTLWExtra* x = d->topData();
    if ( x->basew == basew && x->baseh == baseh )
	return;
    x->basew = basew;
    x->baseh = baseh;
    if ( testWFlags(WType_TopLevel) )
	do_size_hints( this, d->extra );
}
/*!
    Scrolls the widget including its children \a dx pixels to the
    right and \a dy downwards. Both \a dx and \a dy may be negative.

    After scrolling, scroll() sends a paint event for the the part
    that is read but not written. For example, when scrolling 10
    pixels rightwards, the leftmost ten pixels of the widget need
    repainting. The paint event may be delivered immediately or later,
    depending on some heuristics.

    \sa QScrollView erase() bitBlt()
*/

void QWidget::scroll( int dx, int dy )
{
    scroll( dx, dy, QRect() );
}

/*!
    \overload

    This version only scrolls \a r and does not move the children of
    the widget.

    If \a r is empty or invalid, the result is undefined.

    \sa QScrollView erase() bitBlt()
*/
void QWidget::scroll( int dx, int dy, const QRect& r )
{
    if ( testWState( WState_BlockUpdates ) && d->children.isEmpty() )
	return;
    bool valid_rect = r.isValid();
    bool just_update = QABS( dx ) > width() || QABS( dy ) > height();
    if ( just_update )
	update();
    QRect sr = valid_rect?r:clipRegion().boundingRect();
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

    Display *dpy = x11Display();
    GC gc = qt_xget_readonly_gc( x11Screen(), FALSE );
    // Want expose events
    if ( w > 0 && h > 0 && !just_update ) {
	XSetGraphicsExposures( dpy, gc, True );
	XCopyArea( dpy, winId(), winId(), gc, x1, y1, w, h, x2, y2);
	XSetGraphicsExposures( dpy, gc, False );
    }

    if ( !valid_rect && !d->children.isEmpty() ) {	// scroll children
	QPoint pd( dx, dy );
	for (int i = 0; i < d->children.size(); ++i) { // move all children
	    register QObject *object = d->children.at(i);
	    if ( object->isWidgetType() ) {
		QWidget *w = static_cast<QWidget *>(object);
		w->move( w->pos() + pd );
	    }
	}
    }

    if ( just_update )
	return;

    // Don't let the server be bogged-down with repaint events
    bool repaint_immediately = (qt_sip_count( this ) < 3 && !testWState(WState_InPaintEvent));

    if ( dx ) {
	int x = x2 == sr.x() ? sr.x()+w : sr.x();
	if ( repaint_immediately )
	    repaint(x, sr.y(), QABS(dx), sr.height());
	else
	    XClearArea( dpy, winid, x, sr.y(), QABS(dx), sr.height(), True );
    }
    if ( dy ) {
	int y = y2 == sr.y() ? sr.y()+h : sr.y();
	if ( repaint_immediately )
	    repaint( sr.x(), y, sr.width(), QABS(dy) );
	else
	    XClearArea( dpy, winid, sr.x(), y, sr.width(), QABS(dy), True );
    }

    qt_insert_sip( this, dx, dy ); // #### ignores r
}

/*!
    Internal implementation of the virtual QPaintDevice::metric()
    function.

    Use the QPaintDeviceMetrics class instead.

    \a m is the metric to get.
*/

int QWidget::metric( int m ) const
{
    int val;
    if ( m == QPaintDeviceMetrics::PdmWidth ) {
	val = crect.width();
    } else if ( m == QPaintDeviceMetrics::PdmHeight ) {
	val = crect.height();
    } else {
	Display *dpy = x11Display();
	int scr = x11Screen();
	switch ( m ) {
	    case QPaintDeviceMetrics::PdmDpiX:
	    case QPaintDeviceMetrics::PdmPhysicalDpiX:
		val = QPaintDevice::x11AppDpiX( scr );
		break;
	    case QPaintDeviceMetrics::PdmDpiY:
	    case QPaintDeviceMetrics::PdmPhysicalDpiY:
		val = QPaintDevice::x11AppDpiY( scr );
		break;
	    case QPaintDeviceMetrics::PdmWidthMM:
		val = (DisplayWidthMM(dpy,scr)*crect.width())/
		      DisplayWidth(dpy,scr);
		break;
	    case QPaintDeviceMetrics::PdmHeightMM:
		val = (DisplayHeightMM(dpy,scr)*crect.height())/
		      DisplayHeight(dpy,scr);
		break;
	    case QPaintDeviceMetrics::PdmNumColors:
		val = x11Cells();
		break;
	    case QPaintDeviceMetrics::PdmDepth:
		val = x11Depth();
		break;
	    default:
		val = 0;
		qWarning( "QWidget::metric: Invalid metric command" );
	}
    }
    return val;
}

void QWidgetPrivate::createSysExtra()
{
    extra->xDndProxy = 0;
    extra->children_use_dnd = FALSE;
    extra->compress_events = TRUE;
}

void QWidgetPrivate::deleteSysExtra()
{
}

void QWidgetPrivate::createTLSysExtra()
{
    // created lazily
    extra->topextra->xic = 0;
}

void QWidgetPrivate::deleteTLSysExtra()
{
    destroyInputContext();
}

/*
   examine the children of our parent up the tree and set the
   children_use_dnd extra data appropriately... this is used to keep DND enabled
   for widgets that are reparented and don't have DND enabled, BUT *DO* have
   children (or children of children ...) with DND enabled...
*/
void QWidgetPrivate::checkChildrenDnd()
{
    QWidget *widget = q;
    while (widget && !widget->isDesktop()) {
	// note: this isn't done for the desktop widget
	bool children_use_dnd = FALSE;
	for (int i = 0; i < widget->d->children.size(); ++i) {
	    const QObject *object = widget->d->children.at(i);
	    if ( object->isWidgetType() ) {
		const QWidget *child = static_cast<const QWidget *>(object);
		children_use_dnd = (children_use_dnd ||
				    child->acceptDrops() ||
				    (child->d->extra &&
				     child->d->extra->children_use_dnd));
	    }
	}

	widget->d->createExtra();
	widget->d->extra->children_use_dnd = children_use_dnd;

	widget = widget->parentWidget();
    }
}

/*!
    \property QWidget::acceptDrops
    \brief whether drop events are enabled for this widget

    Setting this property to TRUE announces to the system that this
    widget \e may be able to accept drop events.

    If the widget is the desktop (QWidget::isDesktop()), this may
    fail if another application is using the desktop; you can call
    acceptDrops() to test if this occurs.

    \warning
    Do not modify this property in a Drag&Drop event handler.
*/
bool QWidget::acceptDrops() const
{
    return testWState( WState_DND );
}

void QWidget::setAcceptDrops( bool on )
{
    if ( (bool)testWState(WState_DND) != on ) {
	if ( qt_dnd_enable( this, on ) ) {
	    if ( on )
		setWState( WState_DND );
	    else
		clearWState( WState_DND );
	}

	d->checkChildrenDnd();
    }
}

/*!
    \overload

    Causes only the parts of the widget which overlap \a region to be
    visible. If the region includes pixels outside the rect() of the
    widget, window system controls in that area may or may not be
    visible, depending on the platform.

    Note that this effect can be slow if the region is particularly
    complex.

    \sa setMask(), clearMask()
*/

void QWidget::setMask( const QRegion& region )
{
    XShapeCombineRegion( x11Display(), winId(), ShapeBounding, 0, 0,
			 region.handle(), ShapeSet );
}

/*!
    Causes only the pixels of the widget for which \a bitmap has a
    corresponding 1 bit to be visible. If the region includes pixels
    outside the rect() of the widget, window system controls in that
    area may or may not be visible, depending on the platform.

    Note that this effect can be slow if the region is particularly
    complex.

    \sa setMask(), clearMask()
*/

void QWidget::setMask( const QBitmap &bitmap )
{
    QBitmap bm = bitmap;
    if ( bm.x11Screen() != x11Screen() )
	bm.x11SetScreen( x11Screen() );
    XShapeCombineMask( x11Display(), winId(), ShapeBounding, 0, 0,
		       bm.handle(), ShapeSet );
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

/*!\reimp
 */
void QWidget::setName( const char *name )
{
    QObject::setName( name );
    if ( isTopLevel() ) {
	XChangeProperty( x11Display(), winId(),
			 ATOM(Wm_Window_Role), XA_STRING, 8, PropModeReplace,
			 (unsigned char *)name, qstrlen( name ) );
    }
}


/*!
  \internal

  Computes the frame rectangle when needed.  This is an internal function, you
  should never call this.
*/

void QWidget::updateFrameStrut() const
{
    QWidget *that = (QWidget *) this;

    if (! isVisible() || isDesktop()) {
	that->fstrut_dirty = (! isVisible());
	return;
    }

    Atom type_ret;
    Window l = winId(), w = winId(), p, r; // target window, it's parent, root
    Window *c;
    int i_unused;
    unsigned int nc;
    unsigned char *data_ret;
    unsigned long l_unused;

    while (XQueryTree(QPaintDevice::x11AppDisplay(), w, &r, &p, &c, &nc)) {
	if (c && nc > 0)
	    XFree(c);

	if (! p) {
	    qWarning("QWidget::updateFrameStrut(): ERROR - no parent");
	    return;
	}

	// if the parent window is the root window, an Enlightenment virtual root or
	// a NET WM virtual root window, stop here
	data_ret = 0;
	if (p == r ||
	    (XGetWindowProperty(QPaintDevice::x11AppDisplay(), p,
				ATOM(Enlightenment_Desktop), 0, 1, False, XA_CARDINAL,
				&type_ret, &i_unused, &l_unused, &l_unused,
				&data_ret) == Success &&
	     type_ret == XA_CARDINAL)) {
	    if (data_ret)
		XFree(data_ret);

	    break;
	} else if (qt_net_supports(ATOM(Net_Virtual_Roots)) && X11->net_virtual_root_list) {
	    int i = 0;
	    while (X11->net_virtual_root_list[i] != 0) {
		if (X11->net_virtual_root_list[i++] == p)
		    break;
	    }
	}

	l = w;
	w = p;
    }

    // we have our window
    int transx, transy;
    XWindowAttributes wattr;
    if (XTranslateCoordinates(QPaintDevice::x11AppDisplay(), l, w,
			      0, 0, &transx, &transy, &p) &&
	XGetWindowAttributes(QPaintDevice::x11AppDisplay(), w, &wattr)) {
	QTLWExtra *top = that->d->topData();
	top->fleft = transx;
	top->ftop = transy;
	top->fright = wattr.width - crect.width() - top->fleft;
	top->fbottom = wattr.height - crect.height() - top->ftop;

	// add the border_width for the window managers frame... some window managers
	// do not use a border_width of zero for their frames, and if we the left and
	// top strut, we ensure that pos() is absolutely correct.  frameGeometry()
	// will still be incorrect though... perhaps i should have foffset as well, to
	// indicate the frame offset (equal to the border_width on X).
	// - Brad
	top->fleft += wattr.border_width;
	top->fright += wattr.border_width;
	top->ftop += wattr.border_width;
	top->fbottom += wattr.border_width;
    }

    that->fstrut_dirty = 0;
}


void QWidgetPrivate::createInputContext()
{
    QWidget *tlw = q->topLevelWidget();
    QTLWExtra *topdata = tlw->d->topData();

#ifndef QT_NO_XIM
    if (X11->xim) {
	if (! topdata->xic) {
	    QInputContext *qic = new QInputContext(tlw);
	    topdata->xic = (void *) qic;
	}
    } else
#endif // QT_NO_XIM
	{
	    // qDebug("QWidget::createInputContext: no xim");
	    topdata->xic = 0;
	}
}


void QWidgetPrivate::destroyInputContext()
{
#ifndef QT_NO_XIM
    QInputContext *qic = (QInputContext *) d->extra->topextra->xic;
    delete qic;
#endif // QT_NO_XIM
    d->extra->topextra->xic = 0;
}


/*!
    This function is called when the user finishes input composition,
    e.g. changes focus to another widget, moves the cursor, etc.
*/
void QWidget::resetInputContext()
{
#ifndef QT_NO_XIM
    if (X11->xim_style & XIMPreeditCallbacks) {
	QWidget *tlw = topLevelWidget();
	QTLWExtra *topdata = tlw->d->topData();

	// trigger input context creation if it hasn't happened already
	d->createInputContext();

	if (topdata->xic) {
	    QInputContext *qic = (QInputContext *) topdata->xic;
	    qic->reset();
	}
    }
#endif // QT_NO_XIM
}


void QWidgetPrivate::focusInputContext()
{
#ifndef QT_NO_XIM
    QWidget *tlw = q->topLevelWidget();
    QTLWExtra *topdata = tlw->d->topData();

    // trigger input context creation if it hasn't happened already
    createInputContext();

    if (topdata->xic) {
	QInputContext *qic = (QInputContext *) topdata->xic;
	qic->setFocus();
    }
#endif // QT_NO_XIM
}

void QWidget::setWindowTransparency(double)
{
}

double QWidget::windowTransparency() const
{
    return 0.0;
}
