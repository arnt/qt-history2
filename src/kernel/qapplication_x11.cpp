/****************************************************************************
** $Id$
**
** Implementation of X11 startup routines and event handling
**
** Created : 931029
**
** Copyright (C) 1992-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Unix/X11 may use this file in accordance with the Qt Commercial
** License Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

// ### needed for solaris-g++ in beta5
#define QT_CLEAN_NAMESPACE

// Get the system specific includes and defines
#include "qplatformdefs.h"

// Solaris redefines connect to __xnet_connect when _XOPEN_SOURCE_EXTENDED is
// defined. This breaks our sources.
#if defined(connect)
# undef connect
#endif

// POSIX Large File Support redefines truncate -> truncate64
#if defined(truncate)
# undef truncate
#endif

#include "qapplication.h"
#include "qapplication_p.h"
#include "qcolor_p.h"
#include "qcursor.h"
#include "qwidget.h"
#include "qwidget_p.h"
#include "qobjectlist.h"
#include "qwidgetlist.h"
#include "qwidgetintdict.h"
#include "qbitarray.h"
#include "qpainter.h"
#include "qpixmapcache.h"
#include "qdatetime.h"
#include "qtextcodec.h"
#include "qdatastream.h"
#include "qbuffer.h"
#include "qsocketnotifier.h"
#include "qsessionmanager.h"
#include "qvaluelist.h"
#include "qdict.h"
#include "qguardedptr.h"
#include "qclipboard.h"
#include "qwhatsthis.h" // ######## dependency
#include "qsettings.h"
#include "qstylefactory.h"
#include "qfileinfo.h"

// Input method stuff - UNFINISHED
#include "qinputcontext_p.h"
#include "qinternal_p.h" // shared double buffer cleanup

#if defined(QT_THREAD_SUPPORT)
# include "qthread.h"
#endif

#if defined(QT_DEBUG) && defined(Q_OS_LINUX)
# include "qfile.h"
#endif

#include "qt_x11.h"

#if defined(QT_MODULE_OPENGL)
#include <GL/glx.h>
#endif

#include <stdlib.h>
#include <ctype.h>
#include <locale.h>
#include <errno.h>

//#define X_NOT_BROKEN
#ifdef X_NOT_BROKEN
// Some X libraries are built with setlocale #defined to _Xsetlocale,
// even though library users are then built WITHOUT such a definition.
// This creates a problem - Qt might setlocale() one value, but then
// X looks and doesn't see the value Qt set. The solution here is to
// implement _Xsetlocale just in case X calls it - redirecting it to
// the real libC version.
//
# ifndef setlocale
extern "C" char *_Xsetlocale(int category, const char *locale);
char *_Xsetlocale(int category, const char *locale)
{
    //qDebug("_Xsetlocale(%d,%s),category,locale");
    return setlocale(category,locale);
}
# endif // setlocale
#endif // X_NOT_BROKEN


// resolve the conflict between X11's FocusIn and QEvent::FocusIn
const int XFocusOut = FocusOut;
const int XFocusIn = FocusIn;
#undef FocusOut
#undef FocusIn

const int XKeyPress = KeyPress;
const int XKeyRelease = KeyRelease;
#undef KeyPress
#undef KeyRelease


// Fix old X libraries
#ifndef XK_KP_Home
#define XK_KP_Home              0xFF95
#endif
#ifndef XK_KP_Left
#define XK_KP_Left              0xFF96
#endif
#ifndef XK_KP_Up
#define XK_KP_Up                0xFF97
#endif
#ifndef XK_KP_Right
#define XK_KP_Right             0xFF98
#endif
#ifndef XK_KP_Down
#define XK_KP_Down              0xFF99
#endif
#ifndef XK_KP_Prior
#define XK_KP_Prior             0xFF9A
#endif
#ifndef XK_KP_Next
#define XK_KP_Next              0xFF9B
#endif
#ifndef XK_KP_End
#define XK_KP_End               0xFF9C
#endif
#ifndef XK_KP_Insert
#define XK_KP_Insert            0xFF9E
#endif
#ifndef XK_KP_Delete
#define XK_KP_Delete            0xFF9F
#endif


/*****************************************************************************
  Internal variables and functions
 *****************************************************************************/
static const char *appName;			// application name
static const char *appFont	= 0;		// application font
static const char *appBGCol	= 0;		// application bg color
static const char *appFGCol	= 0;		// application fg color
static const char *appBTNCol	= 0;		// application btn color
static const char *mwGeometry	= 0;		// main widget geometry
static const char *mwTitle	= 0;		// main widget title
//Ming-Che 10/10
static char    *ximServer	= 0;		// XIM Server will connect to
static bool	mwIconic	= FALSE;	// main widget iconified
//Ming-Che 10/10
static bool	noxim		= FALSE;	// connect to xim or not
static Display *appDpy		= 0;		// X11 application display
static char    *appDpyName	= 0;		// X11 display name
static bool	appForeignDpy	= FALSE;        // we didn't create display
static bool	appSync		= FALSE;	// X11 synchronization
#if defined(QT_DEBUG)
static bool	appNoGrab	= FALSE;	// X11 grabbing enabled
static bool	appDoGrab	= FALSE;	// X11 grabbing override (gdb)
#endif
static int	appScreen;			// X11 screen number
static int	appScreenCount;			// X11 screen count
static bool	app_save_rootinfo = FALSE;	// save root info
static bool	app_do_modal	= FALSE;	// modal mode
static Window	curWin = 0;			// current window

static GC*	app_gc_ro	= 0;		// read-only GC
static GC*	app_gc_tmp	= 0;		// temporary GC
static GC*	app_gc_ro_m	= 0;		// read-only GC (monochrome)
static GC*	app_gc_tmp_m	= 0;		// temporary GC (monochrome)
Atom		qt_wm_protocols		= 0;	// window manager protocols
Atom		qt_wm_delete_window	= 0;	// delete window protocol
Atom		qt_wm_take_focus	= 0;	// take focus window protocol
static Atom	qt_qt_scrolldone	= 0;	// scroll synchronization
Atom		qt_net_wm_context_help	= 0;	// context help

static Atom	qt_xsetroot_id		= 0;
Atom            qt_xa_clipboard         = 0;
Atom		qt_selection_property	= 0;
Atom            qt_clipboard_sentinel   = 0;
Atom		qt_selection_sentinel	= 0;
Atom		qt_wm_state		= 0;
static Atom     qt_settings_timestamp	= 0;    // Qt >=3 settings timestamp
static Atom	qt_input_encoding	= 0;	// Qt desktop properties
static Atom	qt_resource_manager	= 0;	// X11 Resource manager
Atom		qt_sizegrip		= 0;	// sizegrip
Atom		qt_wm_client_leader	= 0;
Atom		qt_window_role		= 0;
Atom		qt_sm_client_id		= 0;
Atom		qt_xa_motif_wm_hints	= 0;
Atom		qt_kwin_running	= 0;
Atom		qt_kwm_running	= 0;
Atom		qt_gbackground_properties	= 0;
Atom		qt_x_incr		= 0;
bool		qt_broken_wm		= FALSE;

// NET WM support
Atom		qt_net_supported	= 0;
Atom		qt_net_virtual_roots	= 0;
Atom		qt_net_wm_state		= 0;
Atom		qt_net_wm_state_modal	= 0;
Atom		qt_net_wm_state_max_v	= 0;
Atom		qt_net_wm_state_max_h	= 0;
Atom            qt_net_wm_window_type   = 0;
Atom            qt_net_wm_window_type_normal	= 0;
Atom            qt_net_wm_window_type_dialog	= 0;
Atom            qt_net_wm_window_type_toolbar	= 0;
Atom            qt_net_wm_window_type_override	= 0;	// KDE extension
Atom		qt_net_wm_frame_strut		= 0;	// KDE extension
Atom		qt_net_wm_state_stays_on_top	= 0;	// KDE extension
// Enlightenment support
Atom		qt_enlightenment_desktop	= 0;

// window managers list of supported "stuff"
Atom		*qt_net_supported_list	= 0;
// list of virtual root windows
Window		*qt_net_virtual_root_list	= 0;

// current focus model
static const int FocusModel_Unknown = -1;
static const int FocusModel_Other = 0;
static const int FocusModel_PointerRoot = 1;
static int qt_focus_model = -1;

// TRUE if Qt is compiled w/ XRender support and XRender exists on the connected
// Display
bool	qt_use_xrender	= FALSE;

// modifier masks for alt/meta - detected when the application starts
static long qt_alt_mask = 0;
static long qt_meta_mask = 0;
// modifier mask to remove mode switch from modifiers that have alt/meta set
// this problem manifests itself on HP/UX 10.20 at least, and without it
// modifiers do not work at all...
static long qt_mode_switch_remove_mask = 0;

// flags for extensions for special Languages, currently only for RTL languages
static bool 	qt_use_rtl_extensions = FALSE;

static Window	mouseActWindow	     = 0;	// window where mouse is
static int	mouseButtonPressed   = 0;	// last mouse button pressed
static int	mouseButtonState     = 0;	// mouse button state
static Time	mouseButtonPressTime = 0;	// when was a button pressed
static short	mouseXPos, mouseYPos;		// mouse pres position in act window
static short	mouseGlobalXPos, mouseGlobalYPos; // global mouse press position

extern QWidgetList *qt_modal_stack;		// stack of modal widgets
static bool	    ignoreNextMouseReleaseEvent = FALSE; // ignore the next mouse release
							 // event if return from a modal
							 // widget

static QWidget     *popupButtonFocus = 0;
static QWidget     *popupOfPopupButtonFocus = 0;
static bool	    popupCloseDownMode = FALSE;
static bool	    popupGrabOk;

static bool sm_blockUserInput = FALSE;		// session management

// one day in the future we will be able to have static objects in libraries....
static QGuardedPtr<QWidget>* activeBeforePopup = 0; // focus handling with popups

#if defined (QT_TABLET_SUPPORT)
// since XInput event classes aren't created until we actually open an XInput
// device, here is a static list that we will use later on...
const int INVALID_EVENT = -1;
const int TOTAL_XINPUT_EVENTS = 7;

XDevice *devStylus = NULL;
XDevice *devEraser = NULL;
XEventClass event_list_stylus[TOTAL_XINPUT_EVENTS];
XEventClass event_list_eraser[TOTAL_XINPUT_EVENTS];

int curr_events_stylus = 0;
int curr_events_eraser = 0;

// well, luckily we only need to do this once.
static int xinput_motion = INVALID_EVENT;
static int xinput_key_press = INVALID_EVENT;
static int xinput_key_release = INVALID_EVENT;
static int xinput_button_press = INVALID_EVENT;
static int xinput_button_release = INVALID_EVENT;

// making this assumption on XFree86, since we can only use 1 device,
// the pressure for the eraser and the stylus should be the same, if they aren't
// well, they certainly have a strange pen then...
static int max_pressure;
extern bool chokeMouse;
#endif

typedef int (*QX11EventFilter) (XEvent*);
QX11EventFilter qt_set_x11_event_filter (QX11EventFilter filter);

static QX11EventFilter qt_x11_event_filter = 0;
QX11EventFilter qt_set_x11_event_filter (QX11EventFilter filter)
{
    QX11EventFilter old_filter = qt_x11_event_filter;
    qt_x11_event_filter = filter;
    return old_filter;
}
static bool qt_x11EventFilter( XEvent* ev )
{
    if ( qt_x11_event_filter  && qt_x11_event_filter( ev )  )
	return TRUE;
    return qApp->x11EventFilter( ev );
}

typedef void (*VFPTR)();
typedef QValueList<VFPTR> QVFuncList;
void qt_install_preselect_handler( VFPTR );
void qt_remove_preselect_handler( VFPTR );
static QVFuncList *qt_preselect_handler = 0;
void qt_install_postselect_handler( VFPTR );
void qt_remove_postselect_handler( VFPTR );
static QVFuncList *qt_postselect_handler = 0;
void qt_install_preselect_handler( VFPTR handler )
{
    if ( !qt_preselect_handler )
	qt_preselect_handler = new QVFuncList;
    qt_preselect_handler->append( handler );
}
void qt_remove_preselect_handler( VFPTR handler )
{
    if ( qt_preselect_handler ) {
	QVFuncList::Iterator it = qt_preselect_handler->find( handler );
	if ( it != qt_preselect_handler->end() )
		qt_preselect_handler->remove( it );
    }
}
void qt_install_postselect_handler( VFPTR handler )
{
    if ( !qt_postselect_handler )
	qt_postselect_handler = new QVFuncList;
    qt_postselect_handler->prepend( handler );
}
void qt_remove_postselect_handler( VFPTR handler )
{
    if ( qt_postselect_handler ) {
	QVFuncList::Iterator it = qt_postselect_handler->find( handler );
	if ( it != qt_postselect_handler->end() )
		qt_postselect_handler->remove( it );
    }
}




#if !defined(QT_NO_XIM)
XIM		qt_xim			= 0;
XIMStyle	qt_xim_style		= 0;
// static XIMStyle	xim_preferred_style	= XIMPreeditPosition | XIMStatusNothing;
static XIMStyle	xim_preferred_style	= XIMPreeditCallbacks | XIMStatusNothing;
#endif

static int composingKeycode=0;
static QTextCodec * input_mapper = 0;

Time		qt_x_time = CurrentTime;
extern bool     qt_check_clipboard_sentinel( XEvent* ); //def in qclipboard_x11.cpp
extern bool	qt_check_selection_sentinel( XEvent* ); //def in qclipboard_x11.cpp

static void	qt_save_rootinfo();
static bool	qt_try_modal( QWidget *, XEvent * );

int		qt_ncols_option  = 216;		// used in qcolor_x11.cpp
int		qt_visual_option = -1;
bool		qt_cmap_option	 = FALSE;
QWidget	       *qt_button_down	 = 0;		// widget got last button-down

struct QScrollInProgress {
    static long serial;
    QScrollInProgress( QWidget* w, int x, int y ) :
    id( serial++ ), scrolled_widget( w ), dx( x ), dy( y ) {}
    long id;
    QWidget* scrolled_widget;
    int dx, dy;
};
long QScrollInProgress::serial=0;
static QPtrList<QScrollInProgress> *sip_list = 0;


// stuff in qt_xdnd.cpp
// setup
extern void qt_xdnd_setup();
// x event handling
extern void qt_handle_xdnd_enter( QWidget *, const XEvent *, bool );
extern void qt_handle_xdnd_position( QWidget *, const XEvent *, bool );
extern void qt_handle_xdnd_status( QWidget *, const XEvent *, bool );
extern void qt_handle_xdnd_leave( QWidget *, const XEvent *, bool );
extern void qt_handle_xdnd_drop( QWidget *, const XEvent *, bool );
extern void qt_handle_xdnd_finished( QWidget *, const XEvent *, bool );
extern void qt_xdnd_handle_selection_request( const XSelectionRequestEvent * );
extern bool qt_xdnd_handle_badwindow();

extern void qt_motifdnd_handle_msg( QWidget *, const XEvent *, bool );
extern void qt_x11_motifdnd_init();

// client message atoms
extern Atom qt_xdnd_enter;
extern Atom qt_xdnd_position;
extern Atom qt_xdnd_status;
extern Atom qt_xdnd_leave;
extern Atom qt_xdnd_drop;
extern Atom qt_xdnd_finished;
// xdnd selection atom
extern Atom qt_xdnd_selection;
extern bool qt_xdnd_dragging;

// gui or non-gui from qapplication.cpp
extern bool qt_is_gui_used;

extern bool qt_resolve_symlinks; // from qapplication.cpp

// Paint event clipping magic
extern void qt_set_paintevent_clipping( QPaintDevice* dev, const QRegion& region);
extern void qt_clear_paintevent_clipping();


// Palette handling
extern QPalette *qt_std_pal;
extern void qt_create_std_palette();

void qt_x11_intern_atom( const char *, Atom * );

static QPtrList<QWidget>* deferred_map_list = 0;
static void qt_deferred_map_cleanup()
{
    delete deferred_map_list;
    deferred_map_list = 0;
}
void qt_deferred_map_add( QWidget* w)
{
    if ( !deferred_map_list ) {
	deferred_map_list = new QPtrList<QWidget>;
	qAddPostRoutine( qt_deferred_map_cleanup );
    }
    deferred_map_list->append( w );
}
void qt_deferred_map_take( QWidget* w )
{
    if (deferred_map_list ) {
	deferred_map_list->remove( w );
    }
}
static bool qt_deferred_map_contains( QWidget* w )
{
    if (!deferred_map_list)
	return FALSE;
    else
	return deferred_map_list->contains( w );
}


class QETWidget : public QWidget		// event translator widget
{
public:
    void setWState( WFlags f )		{ QWidget::setWState(f); }
    void clearWState( WFlags f )	{ QWidget::clearWState(f); }
    void setWFlags( WFlags f )		{ QWidget::setWFlags(f); }
    void clearWFlags( WFlags f )	{ QWidget::clearWFlags(f); }
    bool translateMouseEvent( const XEvent * );
    bool translateKeyEventInternal( const XEvent *, int& count, QString& text, int& state, char& ascii, int &code,
				    QEvent::Type &type );
    bool translateKeyEvent( const XEvent *, bool grab );
    bool translatePaintEvent( const XEvent * );
    bool translateConfigEvent( const XEvent * );
    bool translateCloseEvent( const XEvent * );
    bool translateScrollDoneEvent( const XEvent * );
    bool translateWheelEvent( int global_x, int global_y, int delta, int state, Orientation orient );
#if defined (QT_TABLET_SUPPORT)
    bool translateXinputEvent( const XEvent* );
#endif

};




// ************************************************************************
// X Input Method support
// ************************************************************************

#if !defined(QT_NO_XIM)

#if defined(Q_C_CALLBACKS)
extern "C" {
#endif // Q_C_CALLBACKS

#ifdef USE_X11R6_XIM
    static void xim_create_callback(XIM /*im*/,
				    XPointer /*client_data*/,
				    XPointer /*call_data*/)
    {
	// qDebug("xim_create_callback");
	QApplication::create_xim();
    }

    static void xim_destroy_callback(XIM /*im*/,
				     XPointer /*client_data*/,
				     XPointer /*call_data*/)
    {
	// qDebug("xim_destroy_callback");
	QApplication::close_xim();
	XRegisterIMInstantiateCallback(appDpy, 0, 0, 0,
				       (XIMProc) xim_create_callback, 0);
    }

#endif // USE_X11R6_XIM

#if defined(Q_C_CALLBACKS)
}
#endif // Q_C_CALLBACKS

#endif // QT_NO_XIM


/*! \internal
  Creates the application input method.
 */
void QApplication::create_xim()
{
#ifndef QT_NO_XIM
    qt_xim = XOpenIM( appDpy, 0, 0, 0 );
    if ( qt_xim ) {

#ifdef USE_X11R6_XIM
	XIMCallback destroy;
	destroy.callback = (XIMProc) xim_destroy_callback;
	destroy.client_data = 0;
	if ( XSetIMValues( qt_xim, XNDestroyCallback, &destroy, (char *) 0 ) != 0 )
	    qWarning( "Xlib dosn't support destroy callback");
#endif // USE_X11R6_XIM

	XIMStyles *styles = 0;
	XGetIMValues(qt_xim, XNQueryInputStyle, &styles, (char *) 0, (char *) 0);
	if ( styles ) {
	    int i;
	    for ( i = 0; !qt_xim_style && i < styles->count_styles; i++ ) {
		if ( styles->supported_styles[i] == xim_preferred_style ) {
		    qt_xim_style = xim_preferred_style;
		    break;
		}
	    }
	    // if the preferred input style couldn't be found, look for
	    // Nothing
	    for ( i = 0; !qt_xim_style && i < styles->count_styles; i++ ) {
		if ( styles->supported_styles[i] == (XIMPreeditNothing |
						     XIMStatusNothing) ) {
		    qt_xim_style = XIMPreeditNothing | XIMStatusNothing;
		    break;
		}
	    }
	    // ... and failing that, None.
	    for ( i = 0; !qt_xim_style && i < styles->count_styles; i++ ) {
		if ( styles->supported_styles[i] == (XIMPreeditNone |
						     XIMStatusNone) ) {
		    qt_xim_style = XIMPreeditNone | XIMStatusNone;
		    break;
		}
	    }

	    // qDebug("QApplication: using im style %lx", qt_xim_style);
	    XFree( (char *)styles );
	}

	if ( qt_xim_style ) {

#ifdef USE_X11R6_XIM
	    XUnregisterIMInstantiateCallback(appDpy, 0, 0, 0,
					     (XIMProc) xim_create_callback, 0);
#endif // USE_X11R6_XIM

	    QWidgetList *list= qApp->topLevelWidgets();
	    QWidgetListIt it(*list);
	    QWidget * w;
	    while( (w=it.current()) != 0 ) {
		++it;
		w->createTLSysExtra();
	    }
	    delete list;
	} else {
	    // Give up
	    qWarning( "No supported input style found."
		      "  See InputMethod documentation.");
	    close_xim();
	}
    }
#endif // QT_NO_XIM
}


/*! \internal
  Closes the application input method.
*/
void QApplication::close_xim()
{
#ifndef QT_NO_XIM
    // Calling XCloseIM gives a Purify FMR error
    // XCloseIM( qt_xim );
    // We prefer a less serious memory leak

    qt_xim = 0;
    QWidgetList *list = qApp->topLevelWidgets();
    QWidgetListIt it(*list);
    while(it.current()) {
	it.current()->destroyInputContext();
	++it;
    }
    delete list;
#endif // QT_NO_XIM
}


/*****************************************************************************
  Default X error handlers
 *****************************************************************************/

#if defined(Q_C_CALLBACKS)
extern "C" {
#endif

static bool x11_ignore_badwindow;
static bool x11_badwindow;

    // starts to ignore bad window errors from X
void qt_ignore_badwindow()
{
    x11_ignore_badwindow = TRUE;
    x11_badwindow = FALSE;
}

    // ends ignoring bad window errors and returns whether an error
    // had happen.
bool qt_badwindow()
{
    x11_ignore_badwindow = FALSE;
    return x11_badwindow;
}

static int (*original_x_errhandler)(Display*dpy,XErrorEvent*);
static int (*original_xio_errhandler)(Display*dpy);

static int qt_x_errhandler( Display *dpy, XErrorEvent *err )
{
    if ( err->error_code == BadWindow ) {
	x11_badwindow = TRUE;
	if ( err->request_code == 25 && qt_xdnd_handle_badwindow() )
	    return 0;
	if ( x11_ignore_badwindow )
	    return 0;
    }
    else if ( err->error_code == BadMatch
	      && err->request_code == 42 /* X_SetInputFocus */ ) {
	return 0;
    }

    char errstr[256];
    XGetErrorText( dpy, err->error_code, errstr, 256 );
    qWarning( "X Error: %s %d\n  Major opcode:  %d\n  Resource id:  0x%lx",
	      errstr, err->error_code, err->request_code, err->resourceid );
    //### we really should distinguish between severe, non-severe and
    //### application specific errors
    return 0;
}


static int qt_xio_errhandler( Display * )
{
    qWarning( "%s: Fatal IO error: client killed", appName );
    exit( 1 );
    //### give the application a chance for a proper shutdown instead,
    //### exit(1) doesn't help.
    return 0;
}

#if defined(Q_C_CALLBACKS)
}
#endif


// Memory leak: if the app exits before qt_init_internal(), this dict
// isn't released correctly.
static QAsciiDict<Atom> *atoms_to_be_created = 0;
static bool create_atoms_now = 0;

/*****************************************************************************
  qt_x11_intern_atom() - efficiently interns an atom, now or later.

  If the application is being initialized, this function stores the
  adddress of the atom and qt_init_internal will do the actual work
  quickly. If the application is running, the atom is created here.

  Neither argument may point to temporary variables.
 *****************************************************************************/

void qt_x11_intern_atom( const char *name, Atom *result)
{
    if ( !name || !result || *result )
	return;

    if ( create_atoms_now ) {
	*result = XInternAtom( appDpy, name, False );
    } else {
	if ( !atoms_to_be_created ) {
	    atoms_to_be_created = new QAsciiDict<Atom>;
	    atoms_to_be_created->setAutoDelete( FALSE );
	}
	atoms_to_be_created->insert( name, result );
	*result = 0;
    }
}


static void qt_x11_process_intern_atoms()
{
    if ( atoms_to_be_created ) {
#if defined(XlibSpecificationRelease) && (XlibSpecificationRelease >= 6)
	int i = atoms_to_be_created->count();
	Atom * res = (Atom *)malloc( i * sizeof( Atom ) );
	Atom ** resp = (Atom **)malloc( i * sizeof( Atom* ) );
	char ** names = (char **)malloc( i * sizeof(const char*));

	i = 0;
	QAsciiDictIterator<Atom> it( *atoms_to_be_created );
	while( it.current() ) {
	    res[i] = 0;
	    resp[i] = it.current();
	    names[i] = qstrdup(it.currentKey());
	    i++;
	    ++it;
	}
	XInternAtoms( appDpy, names, i, False, res );
	while( i ) {
	    i--;
	    delete [] names[i];
	    if ( res[i] && resp[i] )
		*(resp[i]) = res[i];
	}
	free( res );
	free( resp );
	free( names );
#else
	QAsciiDictIterator<Atom> it( *atoms_to_be_created );
	Atom * result;
	const char * name;
	while( (result = it.current()) != 0 ) {
	    name = it.currentKey();
	    ++it;
	    *result = XInternAtom( appDpy, name, False );
	}
#endif
	delete atoms_to_be_created;
	atoms_to_be_created = 0;
	create_atoms_now = TRUE;
    }
}


/*! \internal
    apply the settings to the application
*/
bool QApplication::x11_apply_settings()
{
    if (! qt_std_pal)
	qt_create_std_palette();

    Atom type;
    int format;
    long offset = 0;
    unsigned long nitems, after = 1;
    unsigned char *data = 0;
    QDateTime timestamp, settingsstamp;
    static QDateTime appliedstamp;
    bool update_timestamp = FALSE;

    if (XGetWindowProperty(appDpy, QPaintDevice::x11AppRootWindow(),
			   qt_settings_timestamp, 0, 0,
			   False, AnyPropertyType, &type, &format, &nitems,
			   &after, &data) == Success && format == 8) {
	if (data)
	    XFree(data);

	QBuffer ts;
	ts.open(IO_WriteOnly);

	while (after > 0) {
	    XGetWindowProperty(appDpy, QPaintDevice::x11AppRootWindow(),
			       qt_settings_timestamp,
			       offset, 1024, False, AnyPropertyType,
			       &type, &format, &nitems, &after, &data);
	    if (format == 8) {
		ts.writeBlock((const char *) data, nitems);
		offset += nitems / 4;
	    }

	    XFree(data);
	}

	QDataStream d(ts.buffer(), IO_ReadOnly);
	d >> timestamp;
    }

    QSettings settings;
    settingsstamp = settings.lastModficationTime("/qt/font");
    if (! settingsstamp.isValid())
	return FALSE;

    if ( appliedstamp == settingsstamp )
	return TRUE;
    appliedstamp = settingsstamp;

    if (! timestamp.isValid() || settingsstamp > timestamp)
	update_timestamp = TRUE;

    /*
      Qt settings. This is now they are written into the datastream.

      /qt/Palette/ *             - QPalette
      /qt/font                   - QFont
      /qt/libraryPath            - QStringList
      /qt/style                  - QString
      /qt/doubleClickInterval    - int
      /qt/cursorFlashTime        - int
      /qt/wheelScrollLines       - int
      /qt/colorSpec              - QString
      /qt/defaultCodec           - QString
      /qt/globalStrut            - QSize
      /qt/GUIEffects             - QStringList
      /qt/Font Substitutions/ *  - QStringList
      /qt/Font Substitutions/... - QStringList
    */

    QString str;
    QStringList strlist;
    int i, num;
    QPalette pal(QApplication::palette());
    strlist = settings.readListEntry("/qt/Palette/active");
    if (strlist.count() == QColorGroup::NColorRoles) {
	for (i = 0; i < QColorGroup::NColorRoles; i++)
	    pal.setColor(QPalette::Active, (QColorGroup::ColorRole) i,
			 QColor(strlist[i]));
    }
    strlist = settings.readListEntry("/qt/Palette/inactive");
    if (strlist.count() == QColorGroup::NColorRoles) {
	for (i = 0; i < QColorGroup::NColorRoles; i++)
	    pal.setColor(QPalette::Inactive, (QColorGroup::ColorRole) i,
			 QColor(strlist[i]));
    }
    strlist = settings.readListEntry("/qt/Palette/disabled");
    if (strlist.count() == QColorGroup::NColorRoles) {
	for (i = 0; i < QColorGroup::NColorRoles; i++)
	    pal.setColor(QPalette::Disabled, (QColorGroup::ColorRole) i,
			 QColor(strlist[i]));
    }


    // workaround for KDE 3.0, which messes up the buttonText value of
    // the disabled palette in QSettings
    if ( pal.disabled().buttonText() == pal.active().buttonText() ) {
	pal.setColor( QPalette::Disabled, QColorGroup::ButtonText,
		      pal.disabled().foreground() );
    }

    if (pal != *qt_std_pal && pal != QApplication::palette()) {
	QApplication::setPalette(pal, TRUE);
	*qt_std_pal = pal;
    }

    QFont font(QApplication::font());
    // read new font
    str = settings.readEntry("/qt/font");
    if (! str.isNull() && ! str.isEmpty()) {
	font.fromString(str);

	if (font != QApplication::font())
	    QApplication::setFont(font, TRUE);
    }

    // read library (ie. plugin) path list
    QString libpathkey =
	QString("/qt/%1.%2/libraryPath").arg( QT_VERSION >> 16 ).arg( (QT_VERSION & 0xff00 ) >> 8 );
    QStringList pathlist = settings.readListEntry(libpathkey, ':');
    if (! pathlist.isEmpty()) {
	QStringList::ConstIterator it = pathlist.begin();
	while (it != pathlist.end())
	    QApplication::addLibraryPath(*it++);
    }

    // read new QStyle
    QString stylename = settings.readEntry("/qt/style");
    if (! stylename.isNull() && ! stylename.isEmpty())
	QApplication::setStyle(stylename);
    else
	stylename = "default";

    num =
	settings.readNumEntry("/qt/doubleClickInterval",
			      QApplication::doubleClickInterval());
    QApplication::setDoubleClickInterval(num);

    num =
	settings.readNumEntry("/qt/cursorFlashTime",
			      QApplication::cursorFlashTime());
    QApplication::setCursorFlashTime(num);

    num =
	settings.readNumEntry("/qt/wheelScrollLines",
			      QApplication::wheelScrollLines());
    QApplication::setWheelScrollLines(num);

    QString colorspec = settings.readEntry("/qt/colorSpec", "default");
    if (colorspec == "normal")
	QApplication::setColorSpec(QApplication::NormalColor);
    else if (colorspec == "custom")
	QApplication::setColorSpec(QApplication::CustomColor);
    else if (colorspec == "many")
	QApplication::setColorSpec(QApplication::ManyColor);
    else if (colorspec != "default")
	colorspec = "default";

    QString defaultcodec = settings.readEntry("/qt/defaultCodec", "none");
    if (defaultcodec != "none") {
	QTextCodec *codec = QTextCodec::codecForName(defaultcodec);
	if (codec)
	    qApp->setDefaultCodec(codec);
    }

    QStringList strut = settings.readListEntry("/qt/globalStrut");
    if (! strut.isEmpty()) {
	if (strut.count() == 2) {
	    QSize sz(strut[0].toUInt(), strut[1].toUInt());

	    if (sz.isValid())
		QApplication::setGlobalStrut(sz);
	}
    }

    QStringList effects = settings.readListEntry("/qt/GUIEffects");

    QApplication::setEffectEnabled( Qt::UI_General,  effects.contains("general") );
    QApplication::setEffectEnabled( Qt::UI_AnimateMenu, effects.contains("animatemenu") );
    QApplication::setEffectEnabled( Qt::UI_FadeMenu, effects.contains("fademenu") );
    QApplication::setEffectEnabled( Qt::UI_AnimateCombo, effects.contains("animatecombo") );
    QApplication::setEffectEnabled( Qt::UI_AnimateTooltip, effects.contains("animatetooltip") );
    QApplication::setEffectEnabled( Qt::UI_FadeTooltip, effects.contains("fadetooltip") );

    QStringList fontsubs =
	settings.entryList("/qt/Font Substitutions");
    if (!fontsubs.isEmpty()) {
	QStringList subs;
	QString fam, skey;
	QStringList::Iterator it = fontsubs.begin();
	while (it != fontsubs.end()) {
	    fam = (*it++).latin1();
	    skey = "/qt/Font Substitutions/" + fam;
	    subs = settings.readListEntry(skey);
	    QFont::insertSubstitutions(fam, subs);
	}
    }

    qt_broken_wm =
	settings.readBoolEntry("/qt/brokenWindowManager", FALSE);

    qt_resolve_symlinks =
	settings.readBoolEntry("/qt/resolveSymlinks", TRUE);

    qt_use_rtl_extensions =
    	settings.readBoolEntry("/qt/useRtlExtensions", FALSE);

    if (update_timestamp) {
	QBuffer stamp;
	QDataStream s(stamp.buffer(), IO_WriteOnly);
	s << settingsstamp;

	XChangeProperty(appDpy, QPaintDevice::x11AppRootWindow(), qt_settings_timestamp,
			qt_settings_timestamp, 8, PropModeReplace,
			(unsigned char *) stamp.buffer().data(),
			stamp.buffer().size());
    }

    return TRUE;
}


// read the _QT_INPUT_ENCODING property and apply the settings to
// the application
static void qt_set_input_encoding()
{
    Atom type;
    int format;
    ulong  nitems, after = 1;
    const char *data;

    int e = XGetWindowProperty( appDpy, QPaintDevice::x11AppRootWindow(),
				qt_input_encoding, 0, 1024,
				False, XA_STRING, &type, &format, &nitems,
				&after,  (unsigned char**)&data );
    if ( e != Success || !nitems || type == None ) {
	// Always use the locale codec, since we have no examples of non-local
	// XIMs, and since we cannot get a sensible answer about the encoding
	// from the XIM.
	input_mapper = QTextCodec::codecForLocale();

    } else {
	if ( !qstricmp( data, "locale" ) )
	    input_mapper = QTextCodec::codecForLocale();
	else
	    input_mapper = QTextCodec::codecForName( data );
	// make sure we have an input codec
	if( !input_mapper )
	    input_mapper = QTextCodec::codecForName( "ISO 8859-1" );
    }
    if ( input_mapper->mibEnum() == 11 ) // 8859-8
	input_mapper = QTextCodec::codecForName( "ISO 8859-8-I");
    if( data )
	XFree( (char *)data );
}

// set font, foreground and background from x11 resources. The
// arguments may override the resource settings.
static void qt_set_x11_resources( const char* font = 0, const char* fg = 0,
				  const char* bg = 0, const char* button = 0 )
{
    if ( !qt_std_pal )
	qt_create_std_palette();

    QCString resFont, resFG, resBG, resEF, sysFont;

    QApplication::setEffectEnabled( Qt::UI_General, FALSE);
    QApplication::setEffectEnabled( Qt::UI_AnimateMenu, FALSE);
    QApplication::setEffectEnabled( Qt::UI_FadeMenu, FALSE);
    QApplication::setEffectEnabled( Qt::UI_AnimateCombo, FALSE );
    QApplication::setEffectEnabled( Qt::UI_AnimateTooltip, FALSE );
    QApplication::setEffectEnabled( Qt::UI_FadeTooltip, FALSE );

    if ( QApplication::desktopSettingsAware() && !QApplication::x11_apply_settings()  ) {
	int format;
	ulong  nitems, after = 1;
	QCString res;
	long offset = 0;
	Atom type = None;

	while (after > 0) {
	    uchar *data;
	    XGetWindowProperty( appDpy, QPaintDevice::x11AppRootWindow(),
				qt_resource_manager,
				offset, 8192, False, AnyPropertyType,
				&type, &format, &nitems, &after,
				&data );
	    res += (char*)data;
	    offset += 2048; // offset is in 32bit quantities... 8192/4 == 2048
	    if ( data )
		XFree( (char *)data );
	}

	QCString key, value;
	int l = 0, r;
	QCString apn = appName;
	int apnl = apn.length();
	int resl = res.length();

	while (l < resl) {
	    r = res.find( '\n', l );
	    if ( r < 0 )
		r = resl;
	    while ( isspace((uchar) res[l]) )
		l++;
	    bool mine = FALSE;
	    if ( res[l] == '*' &&
		 (res[l+1] == 'f' || res[l+1] == 'b' || res[l+1] == 'g' ||
		  res[l+1] == 'F' || res[l+1] == 'B' || res[l+1] == 'G' ||
		  res[l+1] == 's' || res[l+1] == 'S' ) )
		{
		    // OPTIMIZED, since we only want "*[fbgs].."

		    QCString item = res.mid( l, r - l ).simplifyWhiteSpace();
		    int i = item.find( ":" );
		    key = item.left( i ).stripWhiteSpace().mid(1).lower();
		    value = item.right( item.length() - i - 1 ).stripWhiteSpace();
		    mine = TRUE;
		} else if ( res[l] == appName[0] ) {
		    if ( res.mid(l,apnl) == apn &&
			 (res[l+apnl] == '.' || res[l+apnl] == '*' ) )
			{
			    QCString item = res.mid( l, r - l ).simplifyWhiteSpace();
			    int i = item.find( ":" );
			    key = item.left( i ).stripWhiteSpace().mid(apnl+1).lower();
			    value = item.right( item.length() - i - 1 ).stripWhiteSpace();
			    mine = TRUE;
			}
		}

	    if ( mine ) {
		if ( !font && key == "systemfont")
		    sysFont = value.copy();
		if ( !font && key == "font")
		    resFont = value.copy();
		else if  ( !fg &&  key == "foreground" )
		    resFG = value.copy();
		else if ( !bg && key == "background")
		    resBG = value.copy();
		else if ( key == "guieffects")
		    resEF = value.copy();
		// NOTE: if you add more, change the [fbg] stuff above
	    }

	    l = r + 1;
	}
    }
    if ( !sysFont.isEmpty() )
	resFont = sysFont;
    if ( resFont.isEmpty() )
	resFont = font;
    if ( resFG.isEmpty() )
	resFG = fg;
    if ( resBG.isEmpty() )
	resBG = bg;
    if ( !resFont.isEmpty() ) {				// set application font
	QFont fnt;
	fnt.setRawName( resFont );

	if ( fnt != QApplication::font() )
	    QApplication::setFont( fnt, TRUE );
    }


    if ( button || !resBG.isEmpty() || !resFG.isEmpty() ) {// set app colors
	QColor btn;
	QColor bg;
	QColor fg;
	if ( !resBG.isEmpty() )
	    bg = QColor(QString(resBG));
	else
	    bg = qt_std_pal->active().background();
	if ( !resFG.isEmpty() )
	    fg = QColor(QString(resFG));
	else
	    fg = qt_std_pal->active().foreground();
	if ( button )
	    btn = QColor( button );
	else if ( !resBG.isEmpty() )
	    btn = bg;
	else
	    btn = qt_std_pal->active().button();

	int h,s,v;
	fg.hsv(&h,&s,&v);
	QColor base = Qt::white;
	bool bright_mode = FALSE;
	if (v >= 255-50) {
	    base = btn.dark(150);
	    bright_mode = TRUE;
	}

	QColorGroup cg( fg, btn, btn.light(),
			btn.dark(), btn.dark(150), fg, Qt::white, base, bg );
	if (bright_mode) {
	    cg.setColor( QColorGroup::HighlightedText, base );
	    cg.setColor( QColorGroup::Highlight, Qt::white );
	} else {
	    cg.setColor( QColorGroup::HighlightedText, Qt::white );
	    cg.setColor( QColorGroup::Highlight, Qt::darkBlue );
	}
	QColor disabled( (fg.red()+btn.red())/2,
			 (fg.green()+btn.green())/2,
			 (fg.blue()+btn.blue())/2);
	QColorGroup dcg( disabled, btn, btn.light( 125 ), btn.dark(), btn.dark(150),
			 disabled, Qt::white, Qt::white, bg );
	if (bright_mode) {
	    dcg.setColor( QColorGroup::HighlightedText, base );
	    dcg.setColor( QColorGroup::Highlight, Qt::white );
	} else {
	    dcg.setColor( QColorGroup::HighlightedText, Qt::white );
	    dcg.setColor( QColorGroup::Highlight, Qt::darkBlue );
	}
	QPalette pal( cg, dcg, cg );
	if ( pal != *qt_std_pal && pal != QApplication::palette() )
	    QApplication::setPalette( pal, TRUE );
	*qt_std_pal = pal;
    }

    if ( !resEF.isEmpty() ) {
	QStringList effects = QStringList::split(" ",resEF);
	QApplication::setEffectEnabled( Qt::UI_General,  effects.contains("general") );
	QApplication::setEffectEnabled( Qt::UI_AnimateMenu, effects.contains("animatemenu") );
	QApplication::setEffectEnabled( Qt::UI_FadeMenu, effects.contains("fademenu") );
	QApplication::setEffectEnabled( Qt::UI_AnimateCombo, effects.contains("animatecombo") );
	QApplication::setEffectEnabled( Qt::UI_AnimateTooltip, effects.contains("animatetooltip") );
	QApplication::setEffectEnabled( Qt::UI_FadeTooltip, effects.contains("fadetooltip") );
    }
}


// update the supported array
void qt_get_net_supported()
{
    Atom type;
    int format;
    long offset = 0;
    unsigned long nitems, after;
    unsigned char *data;

    int e = XGetWindowProperty(appDpy, QPaintDevice::x11AppRootWindow(),
			       qt_net_supported, 0, 0,
			       False, XA_ATOM, &type, &format, &nitems, &after, &data);
    if (data)
	XFree(data);

    if (qt_net_supported_list)
	delete [] qt_net_supported_list;
    qt_net_supported_list = 0;

    if (e == Success && type == XA_ATOM && format == 32) {
	QBuffer ts;
	ts.open(IO_WriteOnly);

	while (after > 0) {
	    XGetWindowProperty(appDpy, QPaintDevice::x11AppRootWindow(),
			       qt_net_supported, offset, 1024,
			       False, XA_ATOM, &type, &format, &nitems, &after, &data);

	    if (type == XA_ATOM && format == 32) {
		ts.writeBlock((const char *) data, nitems * 4);
		offset += nitems;
	    } else
		after = 0;
	    if (data)
		XFree(data);
	}

	// compute nitems
	QByteArray buffer(ts.buffer());
	nitems = buffer.size() / sizeof(Atom);
	qt_net_supported_list = new Atom[nitems + 1];
	Atom *a = (Atom *) buffer.data();
	uint i;
	for (i = 0; i < nitems; i++)
	    qt_net_supported_list[i] = a[i];
	qt_net_supported_list[nitems] = 0;
    }
}


bool qt_net_supports(Atom atom)
{
    if (! qt_net_supported_list)
	return FALSE;

    bool supported = FALSE;
    int i = 0;
    while (qt_net_supported_list[i] != 0) {
	if (qt_net_supported_list[i++] == atom) {
	    supported = TRUE;
	    break;
	}
    }

    return supported;
}


// update the virtual roots array
void qt_get_net_virtual_roots()
{
    if (qt_net_virtual_root_list)
	delete [] qt_net_virtual_root_list;
    qt_net_virtual_root_list = 0;

    if (! qt_net_supports(qt_net_virtual_roots))
	return;

    Atom type;
    int format;
    long offset = 0;
    unsigned long nitems, after;
    unsigned char *data;

    int e = XGetWindowProperty(appDpy, QPaintDevice::x11AppRootWindow(),
			       qt_net_virtual_roots, 0, 0,
			       False, XA_ATOM, &type, &format, &nitems, &after, &data);
    if (data)
	XFree(data);

    if (e == Success && type == XA_ATOM && format == 32) {
	QBuffer ts;
	ts.open(IO_WriteOnly);

	while (after > 0) {
	    XGetWindowProperty(appDpy, QPaintDevice::x11AppRootWindow(),
			       qt_net_virtual_roots, offset, 1024,
			       False, XA_ATOM, &type, &format, &nitems, &after, &data);

	    if (type == XA_ATOM && format == 32) {
		ts.writeBlock((const char *) data, nitems * 4);
		offset += nitems;
	    } else
		after = 0;
	    if (data)
		XFree(data);
	}

	// compute nitems
	QByteArray buffer(ts.buffer());
	nitems = buffer.size() / sizeof(Window);
	qt_net_supported_list = new Window[nitems + 1];
	Window *a = (Window *) buffer.data();
	uint i;
	for (i = 0; i < nitems; i++)
	    qt_net_virtual_root_list[i] = a[i];
	qt_net_virtual_root_list[nitems] = 0;
    }
}

static void qt_check_focus_model()
{
    Window fw = None;
    int unused;
    XGetInputFocus( appDpy, &fw, &unused );
    if ( fw == PointerRoot )
	qt_focus_model = FocusModel_PointerRoot;
    else
	qt_focus_model = FocusModel_Other;
}


/*
  Returns a truecolor visual (if there is one). 8-bit TrueColor visuals
  are ignored, unless the user has explicitly requested -visual TrueColor.
  The SGI X server usually has an 8 bit default visual, but the application
  can also ask for a truecolor visual. This is what we do if
  QApplication::colorSpec() is QApplication::ManyColor.
*/

static Visual *find_truecolor_visual( Display *dpy, int *depth, int *ncols )
{
    XVisualInfo *vi, rvi;
    int best=0, n, i;
    int scr = DefaultScreen(dpy);
    rvi.c_class = TrueColor;
    rvi.screen  = scr;
    vi = XGetVisualInfo( dpy, VisualClassMask | VisualScreenMask,
			 &rvi, &n );
    if ( vi ) {
	for ( i=0; i<n; i++ ) {
	    if ( vi[i].depth > vi[best].depth )
		best = i;
	}
    }
    Visual *v = DefaultVisual(dpy,scr);
    if ( !vi || (vi[best].visualid == XVisualIDFromVisual(v)) ||
	 (vi[best].depth <= 8 && qt_visual_option != TrueColor) )
	{
	*depth = DefaultDepth(dpy,scr);
	*ncols = DisplayCells(dpy,scr);
    } else {
	v = vi[best].visual;
	*depth = vi[best].depth;
	*ncols = vi[best].colormap_size;
    }
    if ( vi )
	XFree( (char *)vi );
    return v;
}


/*****************************************************************************
  qt_init() - initializes Qt for X11
 *****************************************************************************/

#define XK_MISCELLANY
#define XK_LATIN1
#include <X11/keysymdef.h>

// ### This should be static but it isn't because of the friend declaration
// ### in qpaintdevice.h which then should have a static too but can't have
// ### it because "storage class specifiers invalid in friend function
// ### declarations" :-) Ideas anyone?
void qt_init_internal( int *argcptr, char **argv,
		       Display *display, Qt::HANDLE visual, Qt::HANDLE colormap )
{
    if ( display ) {
	// Qt part of other application

	appForeignDpy = TRUE;
	appName = qstrdup( "Qt-subapplication" );
	appDpy  = display;

	// Install default error handlers

	original_x_errhandler = XSetErrorHandler( qt_x_errhandler );
	original_xio_errhandler = XSetIOErrorHandler( qt_xio_errhandler );
    } else {
	// Qt controls everything (default)

	const char *p;
	int argc = *argcptr;
	int j;

	// Install default error handlers

	original_x_errhandler = XSetErrorHandler( qt_x_errhandler );
	original_xio_errhandler = XSetIOErrorHandler( qt_xio_errhandler );

	// Set application name

	p = strrchr( argv[0], '/' );
	appName = p ? p + 1 : argv[0];

	// Read global settings file
#if !defined(QT_NO_XIM)
	QString ximInputStyle = QSettings().readEntry( "/qt/XIMInputStyle", QObject::trUtf8( "On The Spot" ) );
	if ( ximInputStyle == "On The Spot" )
	    xim_preferred_style = XIMPreeditCallbacks | XIMStatusNothing;
	else if ( ximInputStyle == "Over The Spot" )
	    xim_preferred_style = XIMPreeditPosition | XIMStatusNothing;
	else if ( ximInputStyle == "Off The Spot" )
	    xim_preferred_style = XIMPreeditArea | XIMStatusArea;
	else if ( ximInputStyle == "Root" )
	    xim_preferred_style = XIMPreeditNothing | XIMStatusNothing;
#endif // !QT_NO_XIM

	// Get command line params

	j = 1;
	for ( int i=1; i<argc; i++ ) {
	    if ( argv[i] && *argv[i] != '-' ) {
		argv[j++] = argv[i];
		continue;
	    }
	    QCString arg = argv[i];
	    if ( arg == "-display" ) {
		if ( ++i < argc )
		    appDpyName = argv[i];
	    } else if ( arg == "-fn" || arg == "-font" ) {
		if ( ++i < argc )
		    appFont = argv[i];
	    } else if ( arg == "-bg" || arg == "-background" ) {
		if ( ++i < argc )
		    appBGCol = argv[i];
	    } else if ( arg == "-btn" || arg == "-button" ) {
		if ( ++i < argc )
		    appBTNCol = argv[i];
	    } else if ( arg == "-fg" || arg == "-foreground" ) {
		if ( ++i < argc )
		    appFGCol = argv[i];
	    } else if ( arg == "-name" ) {
		if ( ++i < argc )
		    appName = argv[i];
	    } else if ( arg == "-title" ) {
		if ( ++i < argc )
		    mwTitle = argv[i];
	    } else if ( arg == "-geometry" ) {
		if ( ++i < argc )
		    mwGeometry = argv[i];
		//Ming-Che 10/10
	    } else if ( arg == "-im" ) {
		if ( ++i < argc )
		    ximServer = argv[i];
	    } else if ( arg == "-noxim" ) {
		noxim=TRUE;
		//
	    } else if ( arg == "-iconic" ) {
		mwIconic = !mwIconic;
	    } else if ( arg == "-ncols" ) {   // xv and netscape use this name
		if ( ++i < argc )
		    qt_ncols_option = QMAX(0,atoi(argv[i]));
	    } else if ( arg == "-visual" ) {  // xv and netscape use this name
		if ( ++i < argc ) {
		    QCString s = QCString(argv[i]).lower();
		    if ( s == "truecolor" ) {
			qt_visual_option = TrueColor;
		    } else {
			// ### Should we honor any others?
		    }
		}
#ifndef QT_NO_XIM
	    } else if ( arg == "-inputstyle" ) {
		if ( ++i < argc ) {
		    QCString s = QCString(argv[i]).lower();
		    if ( s == "onthespot" )
			xim_preferred_style = XIMPreeditCallbacks |
					      XIMStatusNothing;
		    else if ( s == "overthespot" )
			xim_preferred_style = XIMPreeditPosition |
					      XIMStatusNothing;
		    else if ( s == "offthespot" )
			xim_preferred_style = XIMPreeditArea |
					      XIMStatusArea;
		    else if ( s == "root" )
			xim_preferred_style = XIMPreeditNothing |
					      XIMStatusNothing;
		}
#endif
	    } else if ( arg == "-cmap" ) {    // xv uses this name
		qt_cmap_option = TRUE;
	    }
#if defined(QT_DEBUG)
	    else if ( arg == "-sync" )
		appSync = !appSync;
	    else if ( arg == "-nograb" )
		appNoGrab = !appNoGrab;
	    else if ( arg == "-dograb" )
		appDoGrab = !appDoGrab;
#endif
	    else
		argv[j++] = argv[i];
	}

	*argcptr = j;

#if defined(QT_DEBUG) && defined(Q_OS_LINUX)
	if ( !appNoGrab && !appDoGrab ) {
	    QCString s;
	    s.sprintf( "/proc/%d/cmdline", getppid() );
	    QFile f( s );
	    if ( f.open( IO_ReadOnly ) ) {
		s.truncate( 0 );
		int c;
		while ( (c = f.getch()) > 0 ) {
		    if ( c == '/' )
			s.truncate( 0 );
		    else
			s += (char)c;
		}
		if ( s == "gdb" ) {
		    appNoGrab = TRUE;
		    qDebug( "Qt: gdb: -nograb added to command-line options.\n"
			    "\t Use the -dograb option to enforce grabbing." );
		}
		f.close();
	    }
	}
#endif
	// Connect to X server

	if( qt_is_gui_used ) {
	    if ( ( appDpy = XOpenDisplay(appDpyName) ) == 0 ) {
		qWarning( "%s: cannot connect to X server %s", appName,
			  XDisplayName(appDpyName) );
		exit( 1 );
	    }

	    if ( appSync )				// if "-sync" argument
		XSynchronize( appDpy, TRUE );
	}
    }
    // Common code, regardless of whether display is foreign.

    // Get X parameters

    if( qt_is_gui_used ) {
	appScreen = DefaultScreen(appDpy);
	appScreenCount = ScreenCount(appDpy);

	// Set X paintdevice parameters

	QPaintDevice::x_appdisplay = appDpy;
	QPaintDevice::x_appscreen = appScreen;
	QPaintDevice::x_appdepth = DefaultDepth(appDpy,appScreen);
	QPaintDevice::x_appcells = DisplayCells(appDpy,appScreen);
	QPaintDevice::x_approotwindow = RootWindow(appDpy, appScreen);

	// allocate the arrays for the QPaintDevice data
	QPaintDevice::x_appdepth_arr = new int[ appScreenCount ];
	QPaintDevice::x_appcells_arr = new int[ appScreenCount ];
	QPaintDevice::x_approotwindow_arr = new Qt::HANDLE[ appScreenCount ];
	QPaintDevice::x_appcolormap_arr = new Qt::HANDLE[ appScreenCount ];
	QPaintDevice::x_appdefcolormap_arr = new bool[ appScreenCount ];
	QPaintDevice::x_appvisual_arr = new void*[ appScreenCount ];
	QPaintDevice::x_appdefvisual_arr = new bool[ appScreenCount ];
	Q_CHECK_PTR( QPaintDevice::x_appdepth_arr );
	Q_CHECK_PTR( QPaintDevice::x_appcells_arr );
	Q_CHECK_PTR( QPaintDevice::x_approotwindow_arr );
	Q_CHECK_PTR( QPaintDevice::x_appcolormap_arr );
	Q_CHECK_PTR( QPaintDevice::x_appdefcolormap_arr );
	Q_CHECK_PTR( QPaintDevice::x_appvisual_arr );
	Q_CHECK_PTR( QPaintDevice::x_appdefvisual_arr );

	int screen;
	for ( screen = 0; screen < appScreenCount; screen++ ) {
	    QPaintDevice::x_appdepth_arr[ screen ] = DefaultDepth(appDpy, screen);
	    QPaintDevice::x_appcells_arr[ screen ] = DisplayCells(appDpy, screen);
	    QPaintDevice::x_approotwindow_arr[ screen ] = RootWindow(appDpy, screen);

	    // ### TODO - make Qt obey -visual -cmap and -ncols options for all screens
	    if ( screen != appScreen ) {
		// for now, just take the defaults for all screens other than the
		// default
		QPaintDevice::x_appcolormap_arr[screen] =
		    DefaultColormap( appDpy, screen );
		QPaintDevice::x_appdefcolormap_arr[screen] = TRUE;
		QPaintDevice::x_appvisual_arr[screen] = DefaultVisual( appDpy, screen );
		QPaintDevice::x_appdefvisual_arr[screen] = TRUE;
	    }
	}

	// setup the visual and colormap for the default screen - this will need to
	// be moved inside the above loop eventually...
	Visual *vis;
	if (! visual) {
	    // use the default visual
	    QPaintDevice::x_appdepth = DefaultDepth(appDpy,appScreen);
	    QPaintDevice::x_appcells = DisplayCells(appDpy,appScreen);

	    vis = DefaultVisual(appDpy,appScreen);
	    QPaintDevice::x_appdefvisual = TRUE;
	    QPaintDevice::x_appdefvisual_arr[appScreen] = TRUE;

	    if ( qt_visual_option == TrueColor ||
		 QApplication::colorSpec() == QApplication::ManyColor ) {
		// find custom visual

		vis = find_truecolor_visual( appDpy, &QPaintDevice::x_appdepth,
					     &QPaintDevice::x_appcells );

		QPaintDevice::x_appvisual = vis;
		QPaintDevice::x_appvisual_arr[appScreen] = vis;
		QPaintDevice::x_appdefvisual =
		    (XVisualIDFromVisual(vis) ==
		     XVisualIDFromVisual(DefaultVisual(appDpy,appScreen)));
		QPaintDevice::x_appdefvisual_arr[appScreen] =
		    QPaintDevice::x_appdefvisual;
	    }

#if defined( QT_MODULE_OPENGL )
	    // If we are using OpenGL widgets we HAVE to make sure
	    // that the default visual is GL enabled, otherwise it
	    // will wreck havock when e.g trying to render to
	    // GLXPixmaps via QPixmap. This is because QPixmap is
	    // always created with a QPaintDevice that uses
	    // x_appvisual per default. Preferably, use a visual that
            // has depth and stencil buffers.

	    int nvis;
	    XVisualInfo * vi;
	    XVisualInfo visInfo;
	    memset( &visInfo, 0, sizeof(XVisualInfo) );
	    visInfo.visualid = XVisualIDFromVisual( vis );
	    visInfo.screen = appScreen;
	    vi = XGetVisualInfo( appDpy, VisualIDMask | VisualScreenMask,
				 &visInfo, &nvis );
	    if ( vi ) {
		int useGL;
		glXGetConfig( appDpy, vi, GLX_USE_GL, &useGL );
		if ( !useGL ) {
		    // We have to find another visual that is GL capable
		    int i;
		    XVisualInfo * visuals;
		    memset( &visInfo, 0, sizeof(XVisualInfo) );
		    visInfo.screen = appScreen;
		    visInfo.c_class = vi->c_class;
		    visInfo.depth = vi->depth;
		    visuals = XGetVisualInfo( appDpy, VisualClassMask |
					      VisualDepthMask |
					      VisualScreenMask, &visInfo,
					      &nvis );
		    if ( visuals ) {
			for ( i = 0; i < nvis; i++ ) {
			    glXGetConfig( appDpy, &visuals[i], GLX_USE_GL,
					  &useGL );
			    if ( useGL ) {
				vis = visuals[i].visual;
				QPaintDevice::x_appdefvisual = FALSE;
				QPaintDevice::x_appdefvisual_arr[appScreen] = FALSE;
				break;
			    }
			}
			XFree( visuals );
		    }
		}
		XFree( vi );
	    }
#endif
     	    QPaintDevice::x_appvisual = vis;
	    QPaintDevice::x_appvisual_arr[appScreen] = vis;
	} else {
	    // use the provided visual
	    vis = (Visual *) visual;

	    // figure out the depth of the visual we are using
	    ulong depth_bits = vis->red_mask | vis->green_mask | vis->blue_mask;
	    QPaintDevice::x_appdepth = 0;
	    while ( depth_bits & 1 ) {
		++QPaintDevice::x_appdepth;
		depth_bits >>= 1;
	    }

	    QPaintDevice::x_appcells = vis->map_entries;
	    QPaintDevice::x_appvisual = vis;
	    QPaintDevice::x_appdefvisual = FALSE;
	    QPaintDevice::x_appvisual_arr[appScreen] = vis;
	    QPaintDevice::x_appdefvisual_arr[appScreen] = FALSE;
	}

	// work around a bug in vnc where DisplayCells returns 8 when Xvnc is run
	// with depth 8
	if (QPaintDevice::x_appdepth == 8)
	    QPaintDevice::x_appcells = 256;

	for ( screen = 0; screen < appScreenCount; screen++ ) {
	    // work around a bug in vnc where DisplayCells returns 8 when Xvnc is run
	    // with depth 8
	    if (QPaintDevice::x_appdepth_arr[ screen ] == 8)
		QPaintDevice::x_appcells_arr[ screen ] = 256;
	}

	if (! colormap) {
	    if ( vis->c_class == TrueColor ) {
		QPaintDevice::x_appdefcolormap = QPaintDevice::x_appdefvisual;
		QPaintDevice::x_appdefcolormap_arr[appScreen] =
		    QPaintDevice::x_appdefvisual_arr[appScreen];
	    } else {
		QPaintDevice::x_appdefcolormap =
		    !qt_cmap_option && QPaintDevice::x_appdefvisual;
		QPaintDevice::x_appdefcolormap_arr[appScreen] =
		    !qt_cmap_option && QPaintDevice::x_appdefvisual;
	    }

	    if ( QPaintDevice::x_appdefcolormap ) {
		XStandardColormap *stdcmap;
		VisualID vid = XVisualIDFromVisual((Visual *) QPaintDevice::x_appvisual);
		int i, count;

		QPaintDevice::x_appcolormap = 0;
		QPaintDevice::x_appcolormap_arr[appScreen] = 0;

		QString serverVendor( ServerVendor( appDpy) );
		if ( ! serverVendor.contains( "Hewlett-Packard" ) ) {
		    // on HPUX 10.20 local displays, the RGB_DEFAULT_MAP colormap
		    // doesn't give us correct colors. Why this happens, I have
		    // no clue, so we disable this for HPUX
		    if (XGetRGBColormaps(appDpy, QPaintDevice::x11AppRootWindow(),
				     &stdcmap, &count, XA_RGB_DEFAULT_MAP)) {
		        i = 0;
			while (i < count && QPaintDevice::x_appcolormap == 0) {
			    if (stdcmap[i].visualid == vid) {
				QPaintDevice::x_appcolormap = stdcmap[i].colormap;
				QPaintDevice::x_appcolormap_arr[appScreen] = stdcmap[i].colormap;
			    }
			    i++;
			}

		        XFree( (char *)stdcmap );
		    }
		}

		if (QPaintDevice::x_appcolormap == 0) {
		    QPaintDevice::x_appcolormap = DefaultColormap(appDpy,appScreen);
		    QPaintDevice::x_appcolormap_arr[appScreen] =
			QPaintDevice::x_appcolormap;
		}
	    } else {
		QPaintDevice::x_appcolormap =
		    XCreateColormap(appDpy, QPaintDevice::x11AppRootWindow(),
				    vis, AllocNone);
		QPaintDevice::x_appcolormap_arr[appScreen] =
		    QPaintDevice::x_appcolormap;
	    }
	} else {
	    QPaintDevice::x_appcolormap = colormap;
	    QPaintDevice::x_appdefcolormap = FALSE;
	    QPaintDevice::x_appcolormap_arr[appScreen] = colormap;
	    QPaintDevice::x_appdefcolormap_arr[appScreen] = FALSE;
	}

	// Support protocols

	qt_x11_intern_atom( "WM_PROTOCOLS", &qt_wm_protocols );
	qt_x11_intern_atom( "WM_DELETE_WINDOW", &qt_wm_delete_window );
	qt_x11_intern_atom( "WM_STATE", &qt_wm_state );
	qt_x11_intern_atom( "WM_TAKE_FOCUS", &qt_wm_take_focus );
	qt_x11_intern_atom( "WM_CLIENT_LEADER", &qt_wm_client_leader);
	qt_x11_intern_atom( "WINDOW_ROLE", &qt_window_role);
	qt_x11_intern_atom( "SM_CLIENT_ID", &qt_sm_client_id);
	qt_x11_intern_atom( "CLIPBOARD", &qt_xa_clipboard );
	qt_x11_intern_atom( "RESOURCE_MANAGER", &qt_resource_manager );
	qt_x11_intern_atom( "INCR", &qt_x_incr );
	qt_x11_intern_atom( "_XSETROOT_ID", &qt_xsetroot_id );
	qt_x11_intern_atom( "_QT_SELECTION", &qt_selection_property );
	qt_x11_intern_atom( "_QT_CLIPBOARD_SENTINEL", &qt_clipboard_sentinel );
	qt_x11_intern_atom( "_QT_SELECTION_SENTINEL", &qt_selection_sentinel );
	qt_x11_intern_atom( "_QT_SCROLL_DONE", &qt_qt_scrolldone );
	qt_x11_intern_atom( "_QT_INPUT_ENCODING", &qt_input_encoding );
	qt_x11_intern_atom( "_QT_SIZEGRIP", &qt_sizegrip );
	qt_x11_intern_atom( "_NET_WM_CONTEXT_HELP", &qt_net_wm_context_help );
	qt_x11_intern_atom( "_MOTIF_WM_HINTS", &qt_xa_motif_wm_hints );
	qt_x11_intern_atom( "KWIN_RUNNING", &qt_kwin_running );
	qt_x11_intern_atom( "KWM_RUNNING", &qt_kwm_running );
	qt_x11_intern_atom( "GNOME_BACKGROUND_PROPERTIES", &qt_gbackground_properties );

	QString atomname("_QT_SETTINGS_TIMESTAMP_");
	atomname += XDisplayName(appDpyName);
	qt_x11_intern_atom( atomname.latin1(), &qt_settings_timestamp );

	qt_x11_intern_atom( "_NET_SUPPORTED", &qt_net_supported );
	qt_x11_intern_atom( "_NET_VIRTUAL_ROOTS", &qt_net_virtual_roots );
	qt_x11_intern_atom( "_NET_WM_STATE", &qt_net_wm_state );
	qt_x11_intern_atom( "_NET_WM_STATE_MODAL", &qt_net_wm_state_modal );
	qt_x11_intern_atom( "_NET_WM_STATE_MAXIMIZED_VERT", &qt_net_wm_state_max_v );
	qt_x11_intern_atom( "_NET_WM_STATE_MAXIMIZED_HORZ", &qt_net_wm_state_max_h );
	qt_x11_intern_atom( "_NET_WM_WINDOW_TYPE", &qt_net_wm_window_type );
	qt_x11_intern_atom( "_NET_WM_WINDOW_TYPE_NORMAL",
			    &qt_net_wm_window_type_normal );
	qt_x11_intern_atom( "_NET_WM_WINDOW_TYPE_DIALOG",
			    &qt_net_wm_window_type_dialog );
	qt_x11_intern_atom( "_NET_WM_WINDOW_TYPE_TOOLBAR",
			    &qt_net_wm_window_type_toolbar );
	qt_x11_intern_atom( "_KDE_NET_WM_WINDOW_TYPE_OVERRIDE",
			    &qt_net_wm_window_type_override );
	qt_x11_intern_atom( "_KDE_NET_WM_FRAME_STRUT", &qt_net_wm_frame_strut );
	qt_x11_intern_atom( "_NET_WM_STATE_STAYS_ON_TOP",
			    &qt_net_wm_state_stays_on_top );
	qt_x11_intern_atom( "ENLIGHTENMENT_DESKTOP", &qt_enlightenment_desktop );

	qt_xdnd_setup();
	qt_x11_motifdnd_init();

	// Finally create all atoms
	qt_x11_process_intern_atoms();

	// initialize NET lists
	qt_get_net_supported();
	qt_get_net_virtual_roots();

#ifndef QT_NO_XRENDER
	// See if XRender is supported on the connected display
	int xrender_eventbase, xrender_errorbase;
	if (XRenderQueryExtension(appDpy, &xrender_eventbase, &xrender_errorbase)) {
	    // XRender is supported, let's see if we have a PictFormat for the
	    // default visual
	    XRenderPictFormat *format =
		XRenderFindVisualFormat(appDpy,
					(Visual *) QPaintDevice::x_appvisual);
	    qt_use_xrender = (format != 0) && (QPaintDevice::x_appdepth != 8);
	}
#endif // QT_NO_XRENDER

#ifndef QT_NO_XKB
	// If XKB is detected, set the GrabsUseXKBState option so input method
	// compositions continue to work (ie. deadkeys)
	unsigned int state = XkbPCF_GrabsUseXKBStateMask;
	(void) XkbSetPerClientControls(appDpy, state, &state);
#endif

	// look at the modifier mapping, and get the correct masks for alt/meta
	// find the alt/meta masks
	XModifierKeymap *map = XGetModifierMapping(appDpy);
	if (map) {
	    int i, maskIndex = 0, mapIndex = 0;
	    for (maskIndex = 0; maskIndex < 8; maskIndex++) {
		for (i = 0; i < map->max_keypermod; i++) {
		    if (map->modifiermap[mapIndex]) {
			KeySym sym =
			    XKeycodeToKeysym(appDpy, map->modifiermap[mapIndex], 0);
			if ( qt_alt_mask == 0 &&
			     ( sym == XK_Alt_L || sym == XK_Alt_R ) ) {
			    qt_alt_mask = 1 << maskIndex;
			}
			if ( qt_meta_mask == 0 &&
			     (sym == XK_Meta_L || sym == XK_Meta_R ) ) {
			    qt_meta_mask = 1 << maskIndex;
			}
		    }
		    mapIndex++;
		}
	    }

	    // not look for mode_switch in qt_alt_mask and qt_meta_mask - if it is
	    // present in one or both, then we set qt_mode_switch_remove_mask.
	    // see QETWidget::translateKeyEventInternal for an explanation
	    // of why this is needed
	    mapIndex = 0;
	    for ( maskIndex = 0; maskIndex < 8; maskIndex++ ) {
		if ( qt_alt_mask  != ( 1 << maskIndex ) &&
		     qt_meta_mask != ( 1 << maskIndex ) ) {
		    for ( i = 0; i < map->max_keypermod; i++ )
			mapIndex++;
		    continue;
		}

		for ( i = 0; i < map->max_keypermod; i++ ) {
		    if ( map->modifiermap[ mapIndex ] ) {
			KeySym sym =
			    XKeycodeToKeysym( appDpy, map->modifiermap[ mapIndex ], 0 );
			if ( sym == XK_Mode_switch ) {
			    qt_mode_switch_remove_mask |= 1 << maskIndex;
			}
		    }
		    mapIndex++;
		}
	    }

	    XFreeModifiermap(map);
	} else {
	    // assume defaults
	    qt_alt_mask = Mod1Mask;
	    qt_meta_mask = Mod4Mask;
	    qt_mode_switch_remove_mask = 0;
	}

	// Misc. initialization

	QColor::initialize();
	QFont::initialize();
	QCursor::initialize();
	QPainter::initialize();
    }

#if defined(QT_THREAD_SUPPORT)
    QThread::initialize();
#endif

    if( qt_is_gui_used ) {
	qApp->setName( appName );

	XSelectInput( appDpy, QPaintDevice::x11AppRootWindow(),
		      KeymapStateMask |
		      EnterWindowMask | LeaveWindowMask |
		      PropertyChangeMask
		      );
    }

    // XIM segfaults on Solaris with "C" locale!
    // The idea was that the "en_US" locale is maybe installed on all systems
    // with a "C" locale and could be used as a fallback instead of "C". This
    // is not the case.
    // We'll have to take a XIM / no XIM decision at run-time.
    // Taking a decision at compile-time using QT_NO_XIM is not enough.
#if defined (Q_OS_SOLARIS) && !defined(QT_NO_XIM)
    const char* locale = ::setlocale( LC_ALL, "" );
    if ( !locale || qstrcmp( locale, "C" ) == 0 ) {
	locale = ::setlocale( LC_ALL, "en_US" );
	Q_ASSERT( qstrcmp( locale, "en_US" ) == 0 );
    }
#else
    setlocale( LC_ALL, "" );		// use correct char set mapping
#endif

    setlocale( LC_NUMERIC, "C" );	// make sprintf()/scanf() work

    if ( qt_is_gui_used ) {

#ifndef QT_NO_XIM
	qt_xim = 0;
	QString ximServerName(ximServer);
	if (ximServer)
	    ximServerName.prepend("@im=");
	else
	    ximServerName = "";

	if ( !XSupportsLocale() )
	    qWarning("Qt: Locales not supported on X server");

#ifdef USE_X11R6_XIM
	else if ( XSetLocaleModifiers (ximServerName.ascii()) == 0 )
	    qWarning( "Qt: Cannot set locale modifiers: %s",
		      ximServerName.ascii());
	else if (! noxim)
	    XRegisterIMInstantiateCallback(appDpy, 0, 0, 0,
					   (XIMProc) xim_create_callback, 0);
#else // !USE_X11R6_XIM
	else if ( XSetLocaleModifiers ("") == 0 )
	    qWarning("Qt: Cannot set locale modifiers");
	else if (! noxim)
	    QApplication::create_xim();
#endif // USE_X11R6_XIM
#endif // QT_NO_XIM

	qt_set_input_encoding();

	// be smart about the size of the default font. most X servers have helvetica
	// 12 point available at 2 resolutions:
	//     75dpi (12 pixels) and 100dpi (17 pixels).
	// At 95 DPI, a 12 point font should be 16 pixels tall - in which case a 17
	// pixel font is a closer match than a 12 pixel font
	int ptsz =
	    (int) ( ( ( QPaintDevice::x11AppDpiY() >= 95 ? 17. : 12. ) *
		      72. / (float) QPaintDevice::x11AppDpiY() ) + 0.5 );

	QFont f(
#if defined(Q_OS_SOLARIS) || defined(Q_OS_HPUX)
		"Interface System",
#else
		"Helvetica",
#endif // Q_OS_SOLARIS
		ptsz );
	QApplication::setFont( f );

	qt_set_x11_resources( appFont, appFGCol, appBGCol, appBTNCol);

#if defined (QT_TABLET_SUPPORT)
	int ndev,
	    i,
	    j;
	bool gotStylus,
	     gotEraser;
	XDeviceInfo *devices,
	    *devs;
	XInputClassInfo *ip;
	XAnyClassPtr any;
	XValuatorInfoPtr v;
	XAxisInfoPtr a;
	XDevice *dev;
	XEventClass *ev_class;
	int curr_event_count;

#if !defined(Q_OS_IRIX)
	// XFree86 divides a stylus and eraser into 2 devices, so we must do for both...
	const char XFREENAMESTYLUS[] = "stylus";
	const char XFREENAMEERASER[] = "eraser";
#endif

	devices = XListInputDevices( appDpy, &ndev);
	if ( devices == NULL ) {
	    qWarning( "Failed to get list of devices" );
	    ndev = -1;
	}
	dev = NULL;
	for ( devs = devices, i = 0; i < ndev; i++, devs++ ) {
	    gotStylus = gotEraser = FALSE;
#if defined(Q_OS_IRIX)
	    if ( !strncmp( devs->name, WACOM_NAME, sizeof(WACOM_NAME) - 1 ) ) {
		gotStylus = TRUE;
#else
	    if ( !strncmp( devs->name, XFREENAMESTYLUS, sizeof(XFREENAMESTYLUS) - 1 ) )
		gotStylus = TRUE;
	    else if ( !strncmp( devs->name, XFREENAMEERASER, sizeof(XFREENAMEERASER) - 1 ) )
		gotEraser = TRUE;
	    if ( gotStylus || gotEraser ) {
#endif
		// I only wanted to do this once, so wrap pointers around these
		curr_event_count = 0;

		if ( gotStylus ) {
		    devStylus = XOpenDevice( appDpy, devs->id );
		    dev = devStylus;
		    ev_class = event_list_stylus;
		} else if ( gotEraser ) {
		    devEraser = XOpenDevice( appDpy, devs->id );
		    dev = devEraser;
		    ev_class = event_list_eraser;
		}
		if ( dev == NULL ) {
		    qWarning( "Failed to open device" );
		} else {
		    if ( dev->num_classes > 0 ) {
			for ( ip = dev->classes, j = 0; j < devs->num_classes;
			      ip++, j++ ) {
			    switch ( ip->input_class ) {
			    case KeyClass:
				DeviceKeyPress( dev, xinput_key_press,
						ev_class[curr_event_count] );
				curr_event_count++;
				DeviceKeyRelease( dev, xinput_key_release,
						  ev_class[curr_event_count] );
				curr_event_count++;
				break;
			    case ButtonClass:
				DeviceButtonPress( dev, xinput_button_press,
						   ev_class[curr_event_count] );
				curr_event_count++;
				DeviceButtonRelease( dev, xinput_button_release,
						     ev_class[curr_event_count] );
				curr_event_count++;
				break;
			    case ValuatorClass:
				// I'm only going to be interested in motion when the
				// stylus is already down anyway!
				DeviceMotionNotify( dev, xinput_motion,
						    ev_class[curr_event_count] );
				curr_event_count++;
				break;
			    default:
				break;
			    }
			}
		    }
		}
		// get the min/max value for pressure!
		any = (XAnyClassPtr) ( devs->inputclassinfo );
		if ( dev == devStylus ) {
		    curr_events_stylus = curr_event_count;
		    for (j = 0; j < devs->num_classes; j++) {
			if ( any->c_class == ValuatorClass ) {
			    v = (XValuatorInfoPtr) any;
			    a = (XAxisInfoPtr) ((char *) v +
						sizeof (XValuatorInfo));
#if defined (Q_OS_IRIX)
			    max_pressure = a[WAC_PRESSURE_I].max_value;
#else
			    max_pressure = a[2].max_value;
#endif
			    // got the max pressure no need to go further...
			    break;
			}
			any = (XAnyClassPtr) ((char *) any + any->length);
		    }
		} else {
		    curr_events_eraser = curr_event_count;
		}
		// at this point we are assuming there is only one
		// wacom device...
#if defined (Q_OS_IRIX)
		if ( devStylus != NULL ) {
#else
		if ( devStylus != NULL && devEraser != NULL ) {
#endif
		    break;
		}
	    }
	} // end for loop
	XFreeDeviceList( devices );
#endif // QT_TABLET_SUPPORT

    } else {
	// read some non-GUI settings when not using the X server...

	QSettings settings;

	// read library (ie. plugin) path list
	QStringList pathlist =
	    settings.readListEntry("/qt/libraryPath", ':');
	if (! pathlist.isEmpty()) {
	    QStringList::ConstIterator it = pathlist.begin();
	    while (it != pathlist.end())
		QApplication::addLibraryPath(*it++);
	}

	QString defaultcodec = settings.readEntry("/qt/defaultCodec", "none");
	if (defaultcodec != "none") {
	    QTextCodec *codec = QTextCodec::codecForName(defaultcodec);
	    if (codec)
		qApp->setDefaultCodec(codec);
	}

	qt_resolve_symlinks =
	    settings.readBoolEntry("/qt/resolveSymlinks", TRUE);
    }


}


#ifndef QT_NO_STYLE
 // run-time search for default style
void QApplication::x11_initialize_style()
{
    Atom type;
    int format;
    unsigned long length, after;
    uchar *data;
#ifndef QT_NO_STYLE_WINDOWS
    if ( !app_style
	 && XGetWindowProperty( appDpy, QPaintDevice::x11AppRootWindow(), qt_kwin_running, 0, 1,
			     False, AnyPropertyType, &type, &format,
			     &length, &after, &data ) == Success
	 && length ) {
	if ( data ) XFree( (char *)data );
	// kwin is there. check if KDE's styles are available, otherwise use windows style
	if ( (app_style = QStyleFactory::create("highcolor") ) == 0 )
	    app_style = QStyleFactory::create("windows");
    }
    if ( !app_style
	 && XGetWindowProperty( appDpy, QPaintDevice::x11AppRootWindow(), qt_kwm_running, 0, 1,
			     False, AnyPropertyType, &type, &format,
			     &length, &after, &data ) == Success
	 && length ) {
	if ( data ) XFree( (char *)data );
	// kwm is there, looks like KDE1
	app_style = QStyleFactory::create("windows");
    }
#endif
#ifndef QT_NO_STYLE_MOTIFPLUS
    // maybe another desktop?
    if ( !app_style
	 && XGetWindowProperty( appDpy, QPaintDevice::x11AppRootWindow(), qt_gbackground_properties, 0, 1,
			     False, AnyPropertyType, &type, &format,
			     &length, &after, &data ) == Success
	 && length ) {
	if ( data ) XFree( (char *)data );
	// default to MotifPlus with hovering
	app_style = QStyleFactory::create("motifplus" );
    }
#endif
}
#endif

void qt_init( int *argcptr, char **argv, QApplication::Type )
{
    qt_init_internal( argcptr, argv, 0, 0, 0 );
}

void qt_init( Display *display, Qt::HANDLE visual, Qt::HANDLE colormap )
{
    qt_init_internal( 0, 0, display, visual, colormap );
}


/*****************************************************************************
  qt_cleanup() - cleans up when the application is finished
 *****************************************************************************/

void qt_cleanup()
{
    if ( app_save_rootinfo )			// root window must keep state
	qt_save_rootinfo();

    if ( qt_is_gui_used ) {
	QPixmapCache::clear();
	QPainter::cleanup();
	QCursor::cleanup();
	QFont::cleanup();
	QColor::cleanup();
	QSharedDoubleBuffer::cleanup();
    }
#if defined(QT_THREAD_SUPPORT)
    QThread::cleanup();
#endif

#if defined (QT_TABLET_SUPPORT)
    if ( devStylus != NULL )
	XCloseDevice( appDpy, devStylus );
    if ( devEraser != NULL )
	XCloseDevice( appDpy, devEraser );
#endif

#if !defined(QT_NO_XIM)
    if ( qt_xim )
	QApplication::close_xim();
#endif

    if ( qt_is_gui_used ) {
	int screen;
	for ( screen = 0; screen < appScreenCount; screen++ ) {
	    if ( ! QPaintDevice::x11AppDefaultColormap( screen ) )
		XFreeColormap( QPaintDevice::x11AppDisplay(),
			       QPaintDevice::x11AppColormap( screen ) );
	}
    }

#define QT_CLEANUP_GC(g) if (g) { for (int i=0;i<appScreenCount;i++){if(g[i])XFreeGC(appDpy,g[i]);} delete [] g; g = 0; }
    QT_CLEANUP_GC(app_gc_ro);
    QT_CLEANUP_GC(app_gc_ro_m);
    QT_CLEANUP_GC(app_gc_tmp);
    QT_CLEANUP_GC(app_gc_tmp_m);
#undef QT_CLEANUP_GC

    delete sip_list;
    sip_list = 0;

    // Reset the error handlers
    XSetErrorHandler( original_x_errhandler );
    XSetIOErrorHandler( original_xio_errhandler );

    if ( qt_is_gui_used && !appForeignDpy )
	XCloseDisplay( appDpy );		// close X display
    appDpy = 0;

    if ( QPaintDevice::x_appdepth_arr )
	delete [] QPaintDevice::x_appdepth_arr;
    if ( QPaintDevice::x_appcells_arr )
	delete [] QPaintDevice::x_appcells_arr;
    if ( QPaintDevice::x_appcolormap_arr )
	delete []QPaintDevice::x_appcolormap_arr;
    if ( QPaintDevice::x_appdefcolormap_arr )
	delete [] QPaintDevice::x_appdefcolormap_arr;
    if ( QPaintDevice::x_appvisual_arr )
	delete [] QPaintDevice::x_appvisual_arr;
    if ( QPaintDevice::x_appdefvisual_arr )
	delete [] QPaintDevice::x_appdefvisual_arr;

    if ( appForeignDpy ) {
	delete [] (char *)appName;
	appName = 0;
    }

    delete activeBeforePopup;
    activeBeforePopup = 0;

    if (qt_net_supported_list)
	delete [] qt_net_supported_list;
    qt_net_supported_list = 0;

    if (qt_net_virtual_root_list)
	delete [] qt_net_virtual_root_list;
    qt_net_virtual_root_list = 0;
}


/*****************************************************************************
  Platform specific global and internal functions
 *****************************************************************************/

void qt_save_rootinfo()				// save new root info
{
    Atom type;
    int format;
    unsigned long length, after;
    uchar *data;

    if ( qt_xsetroot_id ) {			// kill old pixmap
	if ( XGetWindowProperty( appDpy, QPaintDevice::x11AppRootWindow(),
				 qt_xsetroot_id, 0, 1,
				 True, AnyPropertyType, &type, &format,
				 &length, &after, &data ) == Success ) {
	    if ( type == XA_PIXMAP && format == 32 && length == 1 &&
		 after == 0 && data ) {
		XKillClient( appDpy, *((Pixmap*)data) );
	    }
	    Pixmap dummy = XCreatePixmap( appDpy, QPaintDevice::x11AppRootWindow(),
					  1, 1, 1 );
	    XChangeProperty( appDpy, QPaintDevice::x11AppRootWindow(),
			     qt_xsetroot_id, XA_PIXMAP, 32,
			     PropModeReplace, (uchar *)&dummy, 1 );
	    XSetCloseDownMode( appDpy, RetainPermanent );
	}
    }
    if ( data )
	XFree( (char *)data );
}

void qt_updated_rootinfo()
{
    app_save_rootinfo = TRUE;
}

bool qt_wstate_iconified( WId winid )
{
    Atom type;
    int format;
    unsigned long length, after;
    uchar *data;
    int r = XGetWindowProperty( appDpy, winid, qt_wm_state, 0, 2,
				 False, AnyPropertyType, &type, &format,
				 &length, &after, &data );
    bool iconic = FALSE;
    if ( r == Success && data && format == 32 ) {
	// Q_UINT32 *wstate = (Q_UINT32*)data;
	unsigned long *wstate = (unsigned long *) data;
	iconic = (*wstate == IconicState );
	XFree( (char *)data );
    }
    return iconic;
}

const char *qAppName()				// get application name
{
    return appName;
}

Display *qt_xdisplay()				// get current X display
{
    return appDpy;
}

int qt_xscreen()				// get current X screen
{
    return appScreen;
}

// ### REMOVE 4.0
WId qt_xrootwin()				// get X root window
{
    return QPaintDevice::x11AppRootWindow();
}

WId qt_xrootwin( int scrn )			// get X root window for screen
{
    return QPaintDevice::x11AppRootWindow( scrn );
}

bool qt_nograb()				// application no-grab option
{
#if defined(QT_DEBUG)
    return appNoGrab;
#else
    return FALSE;
#endif
}

static GC create_gc( int scrn, bool monochrome )
{
    GC gc;
    if ( monochrome ) {
	Pixmap pm = XCreatePixmap( appDpy, RootWindow( appDpy, scrn ), 8, 8, 1 );
	gc = XCreateGC( appDpy, pm, 0, 0 );
	XFreePixmap( appDpy, pm );
    } else {
	if ( QPaintDevice::x11AppDefaultVisual( scrn ) ) {
	    gc = XCreateGC( appDpy, RootWindow( appDpy, scrn ), 0, 0 );
	} else {
	    Window w;
	    XSetWindowAttributes a;
	    a.background_pixel = Qt::black.pixel( scrn );
	    a.border_pixel = Qt::black.pixel( scrn );
	    a.colormap = QPaintDevice::x11AppColormap( scrn );
	    w = XCreateWindow( appDpy, RootWindow( appDpy, scrn ), 0, 0, 100, 100,
			       0, QPaintDevice::x11AppDepth( scrn ), InputOutput,
			       (Visual*)QPaintDevice::x11AppVisual( scrn ),
			       CWBackPixel|CWBorderPixel|CWColormap, &a );
	    gc = XCreateGC( appDpy, w, 0, 0 );
	    XDestroyWindow( appDpy, w );
	}
    }
    XSetGraphicsExposures( appDpy, gc, False );
    return gc;
}

GC qt_xget_readonly_gc( int scrn, bool monochrome )	// get read-only GC
{
    if ( scrn < 0 || scrn >= appScreenCount ) {
	qDebug("invalid screen %d %d", scrn, appScreenCount );
	QWidget* bla = 0;
	bla->setName("hello");
    }
    GC gc;
    if ( monochrome ) {
	if ( !app_gc_ro_m )			// create GC for bitmap
	    memset( (app_gc_ro_m = new GC[appScreenCount]), 0, appScreenCount * sizeof( GC ) );
	if ( !app_gc_ro_m[scrn] )
	    app_gc_ro_m[scrn] = create_gc( scrn, TRUE );
	gc = app_gc_ro_m[scrn];
    } else {					// create standard GC
	if ( !app_gc_ro )
	    memset( (app_gc_ro = new GC[appScreenCount]), 0, appScreenCount * sizeof( GC ) );
	if ( !app_gc_ro[scrn] )
	    app_gc_ro[scrn] = create_gc( scrn, FALSE );
	gc = app_gc_ro[scrn];
    }
    return gc;
}

GC qt_xget_temp_gc( int scrn, bool monochrome )		// get temporary GC
{
    if ( scrn < 0 || scrn >= appScreenCount ) {
	qDebug("invalid screen (tmp) %d %d", scrn, appScreenCount );
	QWidget* bla = 0;
	bla->setName("hello");
    }
    GC gc;
    if ( monochrome ) {
	if ( !app_gc_tmp_m )			// create GC for bitmap
	    memset( (app_gc_tmp_m = new GC[appScreenCount]), 0, appScreenCount * sizeof( GC ) );
	if ( !app_gc_tmp_m[scrn] )
	    app_gc_tmp_m[scrn] = create_gc( scrn, TRUE );
	gc = app_gc_tmp_m[scrn];
    } else {					// create standard GC
	if ( !app_gc_tmp )
	    memset( (app_gc_tmp = new GC[appScreenCount]), 0, appScreenCount * sizeof( GC ) );
	if ( !app_gc_tmp[scrn] )
	    app_gc_tmp[scrn] = create_gc( scrn, FALSE );
	gc = app_gc_tmp[scrn];
    }
    return gc;
}


/*****************************************************************************
  Platform specific QApplication members
 *****************************************************************************/

/*!
    \fn QWidget *QApplication::mainWidget() const

    Returns the main application widget, or 0 if there is no main
    widget.

    \sa setMainWidget()
*/

/*!
    Sets the application's main widget to \a mainWidget.

    In most respects the main widget is like any other widget, except
    that if it is deleted, the application exits.

    You need not have a main widget; connecting lastWindowClosed() to
    quit() is an alternative.

    For X11, this function also resizes and moves the main widget
    according to the \e -geometry command-line option, so you should
    set the default geometry (using \l QWidget::setGeometry()) before
    calling setMainWidget().

    \sa mainWidget(), exec(), quit()
*/

void QApplication::setMainWidget( QWidget *mainWidget )
{
#if defined(QT_CHECK_STATE)
    if ( mainWidget && mainWidget->parentWidget() &&
	 ! mainWidget->parentWidget()->isDesktop() )
	qWarning( "QApplication::setMainWidget(): New main widget (%s/%s) "
		  "has a parent!",
		  mainWidget->className(), mainWidget->name() );
#endif
    main_widget = mainWidget;
    if ( main_widget ) {			// give WM command line
	XSetWMProperties( main_widget->x11Display(), main_widget->winId(),
			  0, 0, app_argv, app_argc, 0, 0, 0 );
	if ( mwTitle )
	    XStoreName( main_widget->x11Display(), main_widget->winId(), (char*)mwTitle );
	if ( mwGeometry ) {			// parse geometry
	    int x, y;
	    int w, h;
	    int m = XParseGeometry( (char*)mwGeometry, &x, &y, (uint*)&w, (uint*)&h );
	    QSize minSize = main_widget->minimumSize();
	    QSize maxSize = main_widget->maximumSize();
	    if ( (m & XValue) == 0 )
		x = main_widget->geometry().x();
	    if ( (m & YValue) == 0 )
		y = main_widget->geometry().y();
	    if ( (m & WidthValue) == 0 )
		w = main_widget->width();
	    if ( (m & HeightValue) == 0 )
		h = main_widget->height();
	    w = QMIN(w,maxSize.width());
	    h = QMIN(h,maxSize.height());
	    w = QMAX(w,minSize.width());
	    h = QMAX(h,minSize.height());
	    if ( (m & XNegative) ) {
		x = desktop()->width()  + x - w;
		qt_widget_tlw_gravity = NorthEastGravity;
	    }
	    if ( (m & YNegative) ) {
		y = desktop()->height() + y - h;
		qt_widget_tlw_gravity = (m & XNegative) ? SouthEastGravity : SouthWestGravity;
	    }
	    main_widget->setGeometry( x, y, w, h );
	}
    }
}

#ifndef QT_NO_CURSOR

/*****************************************************************************
  QApplication cursor stack
 *****************************************************************************/

typedef QPtrList<QCursor> QCursorList;

static QCursorList *cursorStack = 0;

/*!
    \fn QCursor *QApplication::overrideCursor()

    Returns the active application override cursor.

    This function returns 0 if no application cursor has been defined
    (i.e. the internal cursor stack is empty).

    \sa setOverrideCursor(), restoreOverrideCursor()
*/

/*!
    Sets the application override cursor to \a cursor.

    Application override cursors are intended for showing the user
    that the application is in a special state, for example during an
    operation that might take some time.

    This cursor will be displayed in all the application's widgets
    until restoreOverrideCursor() or another setOverrideCursor() is
    called.

    Application cursors are stored on an internal stack.
    setOverrideCursor() pushes the cursor onto the stack, and
    restoreOverrideCursor() pops the active cursor off the stack.
    Every setOverrideCursor() must eventually be followed by a
    corresponding restoreOverrideCursor(), otherwise the stack will
    never be emptied.

    If \a replace is TRUE, the new cursor will replace the last
    override cursor (the stack keeps its depth). If \a replace is
    FALSE, the new stack is pushed onto the top of the stack.

    Example:
    \code
	QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );
	calculateHugeMandelbrot();              // lunch time...
	QApplication::restoreOverrideCursor();
    \endcode

    \sa overrideCursor(), restoreOverrideCursor(), QWidget::setCursor()
*/

void QApplication::setOverrideCursor( const QCursor &cursor, bool replace )
{
    if ( !cursorStack ) {
	cursorStack = new QCursorList;
	Q_CHECK_PTR( cursorStack );
	cursorStack->setAutoDelete( TRUE );
    }
    app_cursor = new QCursor( cursor );
    Q_CHECK_PTR( app_cursor );
    if ( replace )
	cursorStack->removeLast();
    cursorStack->append( app_cursor );

    QWidget* amw = activeModalWidget();
    QWidgetIntDictIt it( *((QWidgetIntDict*)QWidget::mapper) );
    register QWidget *w;
    while ( (w=it.current()) ) {		// for all widgets that have
	if ( w->testWState(WState_OwnCursor) &&
	     ( !amw || !w->isVisible() || w->topLevelWidget() == amw ) )	//   set a cursor
	    XDefineCursor( w->x11Display(), w->winId(), app_cursor->handle() );
	++it;
    }
    XFlush( appDpy );				// make X execute it NOW
}

/*!
    Undoes the last setOverrideCursor().

    If setOverrideCursor() has been called twice, calling
    restoreOverrideCursor() will activate the first cursor set.
    Calling this function a second time restores the original widgets'
    cursors.

    \sa setOverrideCursor(), overrideCursor().
*/

void QApplication::restoreOverrideCursor()
{
    if ( !cursorStack )				// no cursor stack
	return;
    cursorStack->removeLast();
    app_cursor = cursorStack->last();
    if ( QWidget::mapper != 0 && !closingDown() ) {
	QWidgetIntDictIt it( *((QWidgetIntDict*)QWidget::mapper) );
	register QWidget *w;
	while ( (w=it.current()) ) {		// set back to original cursors
	    if ( w->testWState(WState_OwnCursor) )
		XDefineCursor( w->x11Display(), w->winId(),
			       app_cursor ? app_cursor->handle()
			       : w->cursor().handle() );
	    ++it;
	}
	XFlush( appDpy );
    }
    if ( !app_cursor ) {
	delete cursorStack;
	cursorStack = 0;
    }
}

#endif

/*!
    \fn bool QApplication::hasGlobalMouseTracking()

    Returns TRUE if global mouse tracking is enabled; otherwise
    returns FALSE.

    \sa setGlobalMouseTracking()
*/

/*!
    Enables global mouse tracking if \a enable is TRUE, or disables it
    if \a enable is FALSE.

    Enabling global mouse tracking makes it possible for widget event
    filters or application event filters to get all mouse move events,
    even when no button is depressed. This is useful for special GUI
    elements, e.g. tooltips.

    Global mouse tracking does not affect widgets and their
    mouseMoveEvent(). For a widget to get mouse move events when no
    button is depressed, it must do QWidget::setMouseTracking(TRUE).

    This function uses an internal counter. Each
    setGlobalMouseTracking(TRUE) must have a corresponding
    setGlobalMouseTracking(FALSE):
    \code
	// at this point global mouse tracking is off
	QApplication::setGlobalMouseTracking( TRUE );
	QApplication::setGlobalMouseTracking( TRUE );
	QApplication::setGlobalMouseTracking( FALSE );
	// at this point it's still on
	QApplication::setGlobalMouseTracking( FALSE );
	// but now it's off
    \endcode

    \sa hasGlobalMouseTracking(), QWidget::hasMouseTracking()
*/

void QApplication::setGlobalMouseTracking( bool enable )
{
    bool tellAllWidgets;
    if ( enable ) {
	tellAllWidgets = (++app_tracking == 1);
    } else {
	tellAllWidgets = (--app_tracking == 0);
    }
    if ( tellAllWidgets ) {
	QWidgetIntDictIt it( *((QWidgetIntDict*)QWidget::mapper) );
	register QWidget *w;
	while ( (w=it.current()) ) {
	    if ( app_tracking > 0 ) {		// switch on
		if ( !w->testWState(WState_MouseTracking) ) {
		    w->setMouseTracking( TRUE );
		}
	    } else {				// switch off
		if ( w->testWState(WState_MouseTracking) ) {
		    w->setMouseTracking( FALSE );
		}
	    }
	    ++it;
	}
    }
}


/*****************************************************************************
  Routines to find a Qt widget from a screen position
 *****************************************************************************/

Window qt_x11_findClientWindow( Window win, Atom property, bool leaf )
{
    Atom   type = None;
    int	   format, i;
    ulong  nitems, after;
    uchar *data;
    Window root, parent, target=0, *children=0;
    uint   nchildren;
    if ( XGetWindowProperty( appDpy, win, property, 0, 0, FALSE, AnyPropertyType,
			     &type, &format, &nitems, &after, &data ) == Success ) {
	if ( data )
	    XFree( (char *)data );
	if ( type )
	    return win;
    }
    if ( !XQueryTree(appDpy,win,&root,&parent,&children,&nchildren) ) {
	if ( children )
	    XFree( (char *)children );
	return 0;
    }
    for ( i=nchildren-1; !target && i >= 0; i-- )
	target = qt_x11_findClientWindow( children[i], property, leaf );
    if ( children )
	XFree( (char *)children );
    return target;
}


/*!
    Returns a pointer to the widget at global screen position \a
    (x, y), or 0 if there is no Qt widget there.

    If \a child is FALSE and there is a child widget at position \a
    (x, y), the top-level widget containing it is returned. If \a child
    is TRUE the child widget at position \a (x, y) is returned.

    This function is normally rather slow.

    \sa QCursor::pos(), QWidget::grabMouse(), QWidget::grabKeyboard()
*/

QWidget *QApplication::widgetAt( int x, int y, bool child )
{
    int lx, ly;

    Window target;
    if ( !XTranslateCoordinates(appDpy,
				QPaintDevice::x11AppRootWindow(),
				QPaintDevice::x11AppRootWindow(),
				x, y, &lx, &ly, &target) ) {
	return 0;
    }
    if ( !target || target == QPaintDevice::x11AppRootWindow() )
	return 0;
    QWidget *w, *c;
    w = QWidget::find( (WId)target );

    if ( !w ) {
	qt_ignore_badwindow();
	target = qt_x11_findClientWindow( target, qt_wm_state, TRUE );
	if (qt_badwindow() )
	    return 0;
	w = QWidget::find( (WId)target );
#if 0
	if ( !w ) {
	    // Perhaps the widgets at (x,y) is inside a foreign application?
	    // Search all toplevel widgets to see if one is within target
	    QWidgetList *list   = topLevelWidgets();
	    QWidget     *widget = list->first();
	    while ( widget && !w ) {
		Window	ctarget = target;
		if ( widget->isVisible() && !widget->isDesktop() ) {
		    Window wid = widget->winId();
		    while ( ctarget && !w ) {
			XTranslateCoordinates(appDpy, QPaintDevice::x11AppRootWindow(),
					      ctarget, x, y, &lx, &ly, &ctarget);
			if ( ctarget == wid ) {
			    // Found
			    w = widget;
			    XTranslateCoordinates(appDpy,
						  QPaintDevice::x11AppRootWindow(),
						  ctarget, x, y, &lx, &ly, &ctarget);
			}
		    }
		}
		widget = list->next();
	    }
	    delete list;
	}
#endif
    }
    if ( child && w ) {
	if ( (c = w->childAt( w->mapFromGlobal(QPoint(x, y ) ) ) ) )
	    return c;
    }
    return w;
}

/*!
    \overload QWidget *QApplication::widgetAt( const QPoint &pos, bool child )

    Returns a pointer to the widget at global screen position \a pos,
    or 0 if there is no Qt widget there.

    If \a child is FALSE and there is a child widget at position \a
    pos, the top-level widget containing it is returned. If \a child
    is TRUE the child widget at position \a pos is returned.
*/


/*!
    Flushes the X event queue in the X11 implementation. This normally
    returns almost immediately. Does nothing on other platforms.

    \sa syncX()
*/

void QApplication::flushX()
{
    if ( appDpy )
	XFlush( appDpy );
}

/*!
    Flushes the window system specific event queues.

    If you are doing graphical changes inside a loop that does not
    return to the event loop on asynchronous window systems like X11
    or double buffered window systems like MacOS X, and you want to
    visualize these changes immediately (e.g. Splash Screens), call
    this function.

    \sa flushX() sendPostedEvents() QPainter::flush()
*/

void QApplication::flush()
{
    flushX();
}

/*!
    Synchronizes with the X server in the X11 implementation. This
    normally takes some time. Does nothing on other platforms.

    \sa flushX()
*/

void QApplication::syncX()
{
    if ( appDpy )
	XSync( appDpy, False );			// don't discard events
}


/*!
    Sounds the bell, using the default volume and sound.
*/

void QApplication::beep()
{
    if ( appDpy )
	XBell( appDpy, 0 );
}



/*****************************************************************************
  Special lookup functions for windows that have been reparented recently
 *****************************************************************************/

static QWidgetIntDict *wPRmapper = 0;		// alternative widget mapper

void qPRCreate( const QWidget *widget, Window oldwin )
{						// QWidget::reparent mechanism
    if ( !wPRmapper ) {
	wPRmapper = new QWidgetIntDict;
	Q_CHECK_PTR( wPRmapper );
    }
    wPRmapper->insert( (long)oldwin, widget );	// add old window to mapper
    QETWidget *w = (QETWidget *)widget;
    w->setWState( Qt::WState_Reparented );	// set reparented flag
}

void qPRCleanup( QWidget *widget )
{
    QETWidget *etw = (QETWidget *)widget;
    if ( !(wPRmapper && etw->testWState(Qt::WState_Reparented)) )
	return;					// not a reparented widget
    QWidgetIntDictIt it(*wPRmapper);
    QWidget *w;
    while ( (w=it.current()) ) {
	int key = it.currentKey();
	++it;
	if ( w == etw ) {                       // found widget
	    etw->clearWState( Qt::WState_Reparented ); // clear flag
	    wPRmapper->remove( key );// old window no longer needed
	    if ( wPRmapper->count() == 0 ) {	// became empty
		delete wPRmapper;		// then reset alt mapper
		wPRmapper = 0;
		return;
	    }
	}
    }
}

QETWidget *qPRFindWidget( Window oldwin )
{
    return wPRmapper ? (QETWidget*)wPRmapper->find((long)oldwin) : 0;
}

/*!
    \internal
*/
int QApplication::x11ClientMessage(QWidget* w, XEvent* event, bool passive_only)
{
    QETWidget *widget = (QETWidget*)w;
    if ( event->xclient.format == 32 && event->xclient.message_type ) {
	if ( event->xclient.message_type == qt_wm_protocols ) {
	    Atom a = event->xclient.data.l[0];
	    if ( a == qt_wm_delete_window ) {
		if ( passive_only ) return 0;
		widget->translateCloseEvent(event);
	    }
	    else if ( a == qt_wm_take_focus ) {
		QWidget * amw = activeModalWidget();
		if ( (ulong) event->xclient.data.l[1] > qt_x_time )
		    qt_x_time = event->xclient.data.l[1];
		if ( amw && amw != widget ) {
		    QWidget* groupLeader = widget;
		    while ( groupLeader && !groupLeader->testWFlags( Qt::WGroupLeader ) )
			groupLeader = groupLeader->parentWidget();
		    if ( !groupLeader ) {
			amw->raise(); //  help broken window managers
			amw->setActiveWindow();
		    }
		}
#ifndef QT_NO_WHATSTHIS
	    } else if ( a == qt_net_wm_context_help ) {
		QWhatsThis::enterWhatsThisMode();
#endif // QT_NO_WHATSTHIS
	    }
	} else if ( event->xclient.message_type == qt_qt_scrolldone ) {
	    widget->translateScrollDoneEvent(event);
	} else if ( event->xclient.message_type == qt_xdnd_position ) {
	    qt_handle_xdnd_position( widget, event, passive_only );
	} else if ( event->xclient.message_type == qt_xdnd_enter ) {
	    qt_handle_xdnd_enter( widget, event, passive_only );
	} else if ( event->xclient.message_type == qt_xdnd_status ) {
	    qt_handle_xdnd_status( widget, event, passive_only );
	} else if ( event->xclient.message_type == qt_xdnd_leave ) {
	    qt_handle_xdnd_leave( widget, event, passive_only );
	} else if ( event->xclient.message_type == qt_xdnd_drop ) {
	    qt_handle_xdnd_drop( widget, event, passive_only );
	} else if ( event->xclient.message_type == qt_xdnd_finished ) {
	    qt_handle_xdnd_finished( widget, event, passive_only );
	} else {
	    if ( passive_only ) return 0;
	    // All other are interactions
	}
    } else {
	qt_motifdnd_handle_msg( widget, event, passive_only );
    }

    return 0;
}

/*!
    This function does the core processing of individual X
    \a{event}s, normally by dispatching Qt events to the right
    destination.

    It returns 1 if the event was consumed by special handling, 0 if
    the \a event was consumed by normal handling, and -1 if the \a
    event was for an unrecognized widget.

    \sa x11EventFilter()
*/
int QApplication::x11ProcessEvent( XEvent* event )
{
    switch ( event->type ) {
    case ButtonPress:
    case ButtonRelease:
	qt_x_time = event->xbutton.time;
	break;
    case MotionNotify:
	qt_x_time = event->xmotion.time;
	break;
    case XKeyPress:
    case XKeyRelease:
	qt_x_time = event->xkey.time;
	break;
    case PropertyNotify:
	qt_x_time = event->xproperty.time;
	break;
    case EnterNotify:
    case LeaveNotify:
	qt_x_time = event->xcrossing.time;
	qt_x_time = event->xcrossing.time;
	break;
    default:
	break;
    }

    if ( qt_x11EventFilter(event) )		// send through app filter
	return 1;

    QETWidget *widget = (QETWidget*)QWidget::find( (WId)event->xany.window );

    if ( wPRmapper ) {				// just did a widget reparent?
	if ( widget == 0 ) {			// not in std widget mapper
	    switch ( event->type ) {		// only for mouse/key events
	    case ButtonPress:
	    case ButtonRelease:
	    case MotionNotify:
	    case XKeyPress:
	    case XKeyRelease:
		widget = qPRFindWidget( event->xany.window );
		break;
	    }
	}
	else if ( widget->testWState(WState_Reparented) )
	    qPRCleanup( widget );		// remove from alt mapper
    }

    QETWidget *keywidget=0;
    bool grabbed=FALSE;
    if ( event->type==XKeyPress || event->type==XKeyRelease ) {
	keywidget = (QETWidget*)QWidget::keyboardGrabber();
	if ( keywidget ) {
	    grabbed = TRUE;
	} else {
	    if ( focus_widget )
		keywidget = (QETWidget*)focus_widget;
	    if ( !keywidget ) {
		if ( inPopupMode() ) // no focus widget, see if we have a popup
		    keywidget = (QETWidget*) activePopupWidget();
		else if ( widget )
		    keywidget = (QETWidget*)widget->topLevelWidget();
	    }
	}
    }

    int xkey_keycode = event->xkey.keycode;
    extern int qt_compose_keycode; // in qinputcontext_x11.cpp
    qt_compose_keycode = xkey_keycode;
    if ( XFilterEvent( event, keywidget ? keywidget->topLevelWidget()->winId() : None ) ) {
	if ( keywidget )
	    composingKeycode = xkey_keycode; // ### not documented in xlib
	return 1;
    }

    if ( event->type == MappingNotify ) {	// keyboard mapping changed
	XRefreshKeyboardMapping( &event->xmapping );
	return 0;
    }

    if ( event->type == PropertyNotify ) {	// some properties changed
	if ( event->xproperty.window == QPaintDevice::x11AppRootWindow() ) {
	    // root properties
	    if ( event->xproperty.atom == qt_clipboard_sentinel ) {
		if (qt_check_clipboard_sentinel( event ) )
		    emit clipboard()->dataChanged();
	    } else if ( event->xproperty.atom == qt_selection_sentinel ) {
		if (qt_check_selection_sentinel( event ) )
		    emit clipboard()->selectionChanged();
	    } else if ( event->xproperty.atom == qt_input_encoding ) {
		qt_set_input_encoding();
	    } else if ( event->xproperty.atom == qt_net_supported ) {
		qt_get_net_supported();
	    } else if ( event->xproperty.atom == qt_net_virtual_roots ) {
		qt_get_net_virtual_roots();
	    } else if ( obey_desktop_settings ) {
		if ( event->xproperty.atom == qt_resource_manager )
		    qt_set_x11_resources();
		else if ( event->xproperty.atom == qt_settings_timestamp )
		    QApplication::x11_apply_settings();
	    }
	} else if ( widget ) {
	    if (event->xproperty.window == widget->winId()) { // widget properties
		if (widget->isTopLevel()) { // top level properties
		    Atom ret;
		    int format, e;
		    unsigned char *data = 0;
		    unsigned long nitems, after;

		    if (event->xproperty.atom == qt_net_wm_frame_strut) {
			e = XGetWindowProperty(appDpy, event->xproperty.window,
					       qt_net_wm_frame_strut,
					       0, 4, // struts are 4 longs
					       False, XA_CARDINAL, &ret, &format,
					       &nitems, &after, &data);

			if (e == Success && ret == XA_CARDINAL &&
			    format == 32 && nitems == 4) {
			    long *strut = (long *) data;
			    widget->topData()->fleft   = strut[0];
			    widget->topData()->fright  = strut[1];
			    widget->topData()->ftop    = strut[2];
			    widget->topData()->fbottom = strut[3];
			    widget->fstrut_dirty = 0;
			} else {
			    // if failed, zero the strut and mark it dirty
			    widget->topData()->fleft = widget->topData()->fright =
			     widget->topData()->ftop = widget->topData()->fbottom = 0;
			    widget->fstrut_dirty = 1;
			}

			if (data)
			    XFree(data);
		    } else if (event->xproperty.atom == qt_net_wm_state) {
			// using length of 1024 should be safe for all current
			// and possible NET states...
			e = XGetWindowProperty(appDpy, event->xproperty.window,
					       qt_net_wm_state, 0, 1024, False,
					       XA_ATOM, &ret, &format, &nitems,
					       &after, &data);

			bool isMaximized = FALSE;
			if (e == Success && ret == XA_ATOM && format == 32 &&
			    nitems > 0) {
			    Atom *states = (Atom *) data;

			    unsigned long i;
			    for (i = 0; i < nitems; i++) {
				if (states[i] == qt_net_wm_state_max_v ||
				    states[i] == qt_net_wm_state_max_h) {
				    isMaximized = TRUE;
				    break;
				}
			    }
			}

			if (isMaximized)
			    widget->setWState(WState_Maximized);
			else
			    widget->clearWState(WState_Maximized);

			if (data)
			    XFree(data);
		    } else if (event->xproperty.atom == qt_wm_state) {
			// the widget frame strut should also be invalidated
			widget->topData()->fleft = widget->topData()->fright =
			 widget->topData()->ftop = widget->topData()->fbottom = 0;
			widget->fstrut_dirty = 1;

			if (event->xproperty.state == PropertyDelete) {
			    // the window manager has removed the WM State property,
			    // so it is now in the withdrawn state (ICCCM 4.1.3.1) and
			    // we are free to reuse this window
			    widget->topData()->parentWinId = 0;
			    // map the window if we were waiting for a
			    // transition to withdrawn
			    if ( qt_deferred_map_contains( widget ) ) {
				qt_deferred_map_take( widget );
				XMapWindow( appDpy, widget->winId() );
			    }
			} else if (widget->topData()->parentWinId !=
				   QPaintDevice::x11AppRootWindow()) {
			    // the window manager has changed the WM State property...
			    // we are  wanting to see if we are withdrawn so that we
			    // can reuse this window... we only do this check *IF* we
			    // haven't been reparented to root -
			    // (the parentWinId != QPaintDevice::x11AppRootWindow()) check above

			    e = XGetWindowProperty(appDpy, widget->winId(), qt_wm_state,
						   0, 2, False, qt_wm_state, &ret,
						   &format, &nitems, &after, &data );

			    if (e == Success && ret == qt_wm_state &&
				format == 32 && nitems > 0) {
				long *state = (long *) data;
				switch (state[0]) {
				case WithdrawnState:
				    // if we are in the withdrawn state, we are free to
				    // reuse this window provided we remove the
				    // WM_STATE property (ICCCM 4.1.3.1)
				    XDeleteProperty(appDpy, widget->winId(),
						    qt_wm_state);

				    // set the parent id to zero, so that show() will
				    // work again
				    widget->topData()->parentWinId = 0;
				    // map the window if we were waiting for a
				    // transition to withdrawn
				    if ( qt_deferred_map_contains( widget ) ) {
					qt_deferred_map_take( widget );
					XMapWindow( appDpy, widget->winId() );
				    }
				    break;
				default:
				    break;
				}
			    }

			    if (data)
				XFree(data);
			}
		    }
		}
	    }
	}
	return 0;
    }

    if ( !widget ) {				// don't know this windows
	QWidget* popup = QApplication::activePopupWidget();
	if ( popup ) {

	    /*
	      That is more than suboptimal. The real solution should
	      do some keyevent and buttonevent translation, so that
	      the popup still continues to work as the user expects.
	      Unfortunately this translation is currently only
	      possible with a known widget. I'll change that soon
	      (Matthias).
	    */

	    // Danger - make sure we don't lock the server
	    switch ( event->type ) {
	    case ButtonPress:
	    case ButtonRelease:
	    case XKeyPress:
	    case XKeyRelease:
		do {
		    popup->close();
		} while ( (popup = qApp->activePopupWidget()) );
		return 1;
	    }
	}
	return -1;
    }

    if ( app_do_modal )				// modal event handling
	if ( !qt_try_modal(widget, event) ) {
	    if ( event->type == ClientMessage )
		x11ClientMessage( widget, event, TRUE );
	    return 1;
	}


    if ( widget->x11Event(event) )		// send through widget filter
	return 1;
#if defined (QT_TABLET_SUPPORT)
    // Right now I'm only caring about the valuator (MOTION) events, so I'll
    // check them and let the rest go through as mouse events...
    if ( event->type == xinput_motion ||
	 event->type == xinput_button_release ||
	 event->type == xinput_button_press ) {
	widget->translateXinputEvent( event );
	return 0;
    }
#endif

    switch ( event->type ) {

    case ButtonPress:			// mouse event
	ignoreNextMouseReleaseEvent = FALSE;
	// fallthrough intended

    case ButtonRelease:
	if ( ignoreNextMouseReleaseEvent ) {
	    ignoreNextMouseReleaseEvent = FALSE;
	    break;
	}
	// fall through intended

    case MotionNotify:
#if defined(QT_TABLET_SUPPORT)
	if ( !chokeMouse ) {
#endif
	    widget->translateMouseEvent( event );
#if defined(QT_TABLET_SUPPORT)
	} else {
	    chokeMouse = FALSE;
	}
#endif
	break;

    case XKeyPress:				// keyboard event
    case XKeyRelease: {
	if ( keywidget && keywidget->isEnabled() ) // should always exist
	    keywidget->translateKeyEvent( event, grabbed );
    }
	break;

    case GraphicsExpose:
    case Expose:				// paint event
	widget->translatePaintEvent( event );
	break;

    case ConfigureNotify:			// window move/resize event
	if ( event->xconfigure.event == event->xconfigure.window )
	    widget->translateConfigEvent( event );
	break;

    case XFocusIn: {				// got focus
	if ( widget->isDesktop() )
	    break;
	if ( inPopupMode() ) // some delayed focus event to ignore
	    break;
	if ( !widget->isTopLevel() )
	    break;
	if ( event->xfocus.detail != NotifyAncestor &&
	     event->xfocus.detail != NotifyInferior &&
	     event->xfocus.detail != NotifyNonlinear )
	    break;
	widget->createInputContext();
	setActiveWindow( widget );
	if ( qt_focus_model == FocusModel_PointerRoot ) {
	    // We got real input focus from somewhere, but we were in PointerRoot
	    // mode, so we don't trust this event.  Check the focus model to make
	    // sure we know what focus mode we are using...
	    qt_check_focus_model();
	}
    }
	break;

    case XFocusOut:				// lost focus
	if ( widget->isDesktop() )
	    break;
	if ( !widget->isTopLevel() )
	    break;
	if ( event->xfocus.mode != NotifyNormal )
	    break;
	if ( event->xfocus.detail != NotifyAncestor &&
	     event->xfocus.detail != NotifyNonlinearVirtual &&
	     event->xfocus.detail != NotifyNonlinear )
	    break;
	if ( !inPopupMode() && widget == active_window )
	    setActiveWindow( 0 );
	break;

    case EnterNotify: {			// enter window
	if ( QWidget::mouseGrabber()  && widget != QWidget::mouseGrabber() )
	    break;
	if ( inPopupMode() && widget->topLevelWidget() != activePopupWidget() )
	    break;
	if ( event->xcrossing.mode != NotifyNormal ||
	     event->xcrossing.detail == NotifyVirtual  ||
	     event->xcrossing.detail == NotifyNonlinearVirtual )
	    break;
	if ( event->xcrossing.focus &&
	     !widget->isDesktop() && !widget->isActiveWindow() ) {
	    if ( qt_focus_model == FocusModel_Unknown ) // check focus model
		qt_check_focus_model();
	    if ( qt_focus_model == FocusModel_PointerRoot ) // PointerRoot mode
		setActiveWindow( widget );
	}
	qt_dispatchEnterLeave( widget, QWidget::find( curWin ) );
	curWin = widget->winId();
	widget->translateMouseEvent( event ); //we don't get MotionNotify, emulate it
    }
	break;

    case LeaveNotify: {			// leave window
	if ( QWidget::mouseGrabber()  && widget != QWidget::mouseGrabber() )
	    break;
	if ( curWin && widget->winId() != curWin )
	    break;
	if ( event->xcrossing.mode != NotifyNormal )
	    break;
	if ( !widget->isDesktop() )
	    widget->translateMouseEvent( event ); //we don't get MotionNotify, emulate it

	QWidget* enter = 0;
	XEvent ev;
	while ( XCheckMaskEvent( widget->x11Display(), EnterWindowMask | LeaveWindowMask , &ev )
		&& !qt_x11EventFilter( &ev ) && !widget->x11Event( &ev ) ) {
	    if ( ev.type == LeaveNotify && ev.xcrossing.mode == NotifyNormal ){
		if ( !enter )
		    enter = QWidget::find( ev.xcrossing.window );
		XPutBackEvent( widget->x11Display(), &ev );
		break;
	    }
	    if (  ev.xcrossing.mode != NotifyNormal ||
		  ev.xcrossing.detail == NotifyVirtual  ||
		  ev.xcrossing.detail == NotifyNonlinearVirtual )
		continue;
	    enter = QWidget::find( ev.xcrossing.window );
	    if ( ev.xcrossing.focus &&
		 enter && !enter->isDesktop() && !enter->isActiveWindow() ) {
		if ( qt_focus_model == FocusModel_Unknown ) // check focus model
		    qt_check_focus_model();
		if ( qt_focus_model == FocusModel_PointerRoot ) // PointerRoot mode
		    setActiveWindow( enter );
	    }
	    break;
	}

	if ( ( ! enter || enter->isDesktop() ) &&
	     event->xcrossing.focus && widget == active_window &&
	     qt_focus_model == FocusModel_PointerRoot // PointerRoot mode
	     ) {
	    setActiveWindow( 0 );
	}

	if ( !curWin )
	    qt_dispatchEnterLeave( widget, 0 );

	qt_dispatchEnterLeave( enter, widget );
	curWin = enter ? enter->winId() : 0;
    }
	break;

    case UnmapNotify:			// window hidden
	if ( widget->isTopLevel() && widget->isVisible() && !widget->isPopup() ) {
	    widget->clearWState( WState_Visible );
	    QHideEvent e;
	    QApplication::sendSpontaneousEvent( widget, &e );
	    widget->sendHideEventsToChildren( TRUE );
	}
	break;

    case MapNotify:				// window shown
	if ( widget->isTopLevel() && !widget->isVisible()
	     && !widget->isHidden() ) {	    widget->setWState( WState_Visible );
	    widget->sendShowEventsToChildren( TRUE );
	    QShowEvent e;
	    QApplication::sendSpontaneousEvent( widget, &e );
	}
	break;

    case ClientMessage:			// client message
	return x11ClientMessage(widget,event,False);

    case ReparentNotify:			// window manager reparents
	while ( XCheckTypedWindowEvent( widget->x11Display(),
					widget->winId(),
					ReparentNotify,
					event ) )
	    ;	// skip old reparent events
	if ( event->xreparent.parent == QPaintDevice::x11AppRootWindow() ) {
	    if ( widget->isTopLevel() ) {
		widget->topData()->parentWinId = event->xreparent.parent;
		if ( qt_deferred_map_contains( widget ) ) {
		    qt_deferred_map_take( widget );
		    XMapWindow( appDpy, widget->winId() );
		}
	    }
	} else
	    // store the parent. Useful for many things, embedding for instance.
	    widget->topData()->parentWinId = event->xreparent.parent;
	if ( widget->isTopLevel() ) {
	    // the widget frame strut should also be invalidated
	    widget->topData()->fleft = widget->topData()->fright =
	     widget->topData()->ftop = widget->topData()->fbottom = 0;

	    if ( qt_focus_model != FocusModel_Unknown ) {
		// toplevel reparented...
		QWidget *newparent = QWidget::find( event->xreparent.parent );
		if ( ! newparent || newparent->isDesktop() ) {
		    // we dont' know about the new parent (or we've been
		    // reparented to root), perhaps a window manager
		    // has been (re)started?  reset the focus model to unknown
		    qt_focus_model = FocusModel_Unknown;
		}
	    }
	}
	break;

    case SelectionRequest: {
	XSelectionRequestEvent *req = &event->xselectionrequest;
	if (! req)
	    break;

	if ( qt_xdnd_selection && req->selection == qt_xdnd_selection ) {
	    qt_xdnd_handle_selection_request( req );

	} else if (qt_clipboard) {
	    QCustomEvent e( QEvent::Clipboard, event );
	    QApplication::sendSpontaneousEvent( qt_clipboard, &e );
	}
	break;
    }
    case SelectionClear: {
	XSelectionClearEvent *req = &event->xselectionclear;
	// don't deliver dnd events to the clipboard, it gets confused
	if (! req || qt_xdnd_selection && req->selection == qt_xdnd_selection)
	    break;

	if (qt_clipboard) {
	    QCustomEvent e( QEvent::Clipboard, event );
	    QApplication::sendSpontaneousEvent( qt_clipboard, &e );
	}
	break;
    }

    case SelectionNotify: {
	XSelectionEvent *req = &event->xselection;
	// don't deliver dnd events to the clipboard, it gets confused
	if (! req || qt_xdnd_selection && req->selection == qt_xdnd_selection)
	    break;

	if (qt_clipboard) {
	    QCustomEvent e( QEvent::Clipboard, event );
	    QApplication::sendSpontaneousEvent( qt_clipboard, &e );
	}
	break;
    }

    default:
	break;
    }

    return 0;
}


/*!
    \overload

    Processes pending events for \a maxtime milliseconds or until
    there are no more events to process, whichever is shorter.

    You can call this function occasionally when you program is busy
    doing a long operation (e.g. copying a file).

    \sa exec(), QTimer
*/
void QApplication::processEvents( int maxtime )
{
    QTime start = QTime::currentTime();
    QTime now;
    while ( !quit_now && processNextEvent(FALSE) ) {
	now = QTime::currentTime();
	if ( start.msecsTo(now) > maxtime )
	    break;
    }
}

/*!
    This virtual function is only implemented under X11.

    If you create an application that inherits QApplication and
    reimplement this function, you get direct access to all X events
    that the are received from the X server.

    Return TRUE if you want to stop the event from being processed.
    Return FALSE for normal event dispatching.

    \sa x11ProcessEvent()
*/

bool QApplication::x11EventFilter( XEvent * )
{
    return FALSE;
}



/*****************************************************************************
  Modal widgets; Since Xlib has little support for this we roll our own
  modal widget mechanism.
  A modal widget without a parent becomes application-modal.
  A modal widget with a parent becomes modal to its parent and grandparents..

  qt_enter_modal()
	Enters modal state
	Arguments:
	    QWidget *widget	A modal widget

  qt_leave_modal()
	Leaves modal state for a widget
	Arguments:
	    QWidget *widget	A modal widget
 *****************************************************************************/

bool qt_modal_state()
{
    return app_do_modal;
}

void qt_enter_modal( QWidget *widget )
{
    if ( !qt_modal_stack ) {			// create modal stack
	qt_modal_stack = new QWidgetList;
	Q_CHECK_PTR( qt_modal_stack );
    }
    qt_modal_stack->insert( 0, widget );
    app_do_modal = TRUE;
    QWidget *w = QWidget::find( (WId)curWin );
    if ( w ) { // send synthetic leave event
	QEvent e( QEvent::Leave );
	QApplication::sendEvent( w, &e );
    }
    ignoreNextMouseReleaseEvent = FALSE;
}


void qt_leave_modal( QWidget *widget )
{
    if ( qt_modal_stack && qt_modal_stack->removeRef(widget) ) {
	if ( qt_modal_stack->isEmpty() ) {
	    delete qt_modal_stack;
	    qt_modal_stack = 0;
	    QPoint p( QCursor::pos() );
	    QWidget* w = QApplication::widgetAt( p.x(), p.y(), TRUE );
	    qt_dispatchEnterLeave( w, QWidget::find( curWin ) ); // send synthetic enter event
	    curWin = w? w->winId() : 0;
	}
    }
    app_do_modal = qt_modal_stack != 0;
    ignoreNextMouseReleaseEvent = TRUE;
}


static bool qt_try_modal( QWidget *widget, XEvent *event )
{
    if ( qApp->activePopupWidget() )
	return TRUE;

    QWidget *modal=0, *top=QApplication::activeModalWidget();

    QWidget* groupLeader = widget;
    widget = widget->topLevelWidget();

    if ( widget->testWFlags(Qt::WShowModal) )	// widget is modal
	modal = widget;
    if ( !top || modal == top )			// don't block event
	return TRUE;

    while ( groupLeader && !groupLeader->testWFlags( Qt::WGroupLeader ) )
	groupLeader = groupLeader->parentWidget();

    if ( groupLeader ) {
	// Does groupLeader have a child in qt_modal_stack?
	bool unrelated = TRUE;
	modal = qt_modal_stack->first();
	while (modal && unrelated) {
	    QWidget* p = modal->parentWidget();
	    while ( p && p != groupLeader && !p->testWFlags( Qt::WGroupLeader) ) {
		p = p->parentWidget();
	    }
	    modal = qt_modal_stack->next();
	    if ( p == groupLeader ) unrelated = FALSE;
	}

	if ( unrelated )
	    return TRUE;		// don't block event
    }

    bool block_event  = FALSE;
    switch ( event->type ) {
	case ButtonPress:			// disallow mouse/key events
	case ButtonRelease:
	case MotionNotify:
	case XKeyPress:
	case XKeyRelease:
	case EnterNotify:
	case LeaveNotify:
	    block_event	 = TRUE;
	    break;
    	default:
            break;
    }

    return !block_event;
}


/*****************************************************************************
  Popup widget mechanism

  openPopup()
	Adds a widget to the list of popup widgets
	Arguments:
	    QWidget *widget	The popup widget to be added

  closePopup()
	Removes a widget from the list of popup widgets
	Arguments:
	    QWidget *widget	The popup widget to be removed
 *****************************************************************************/


static int openPopupCount = 0;
void QApplication::openPopup( QWidget *popup )
{
    openPopupCount++;
    if ( !popupWidgets ) {			// create list
	popupWidgets = new QWidgetList;
	Q_CHECK_PTR( popupWidgets );
	if ( !activeBeforePopup )
	    activeBeforePopup = new QGuardedPtr<QWidget>;
	(*activeBeforePopup) = active_window;
    }
    popupWidgets->append( popup );		// add to end of list

    if ( popupWidgets->count() == 1 && !qt_nograb() ){ // grab mouse/keyboard
	int r = XGrabKeyboard( popup->x11Display(), popup->winId(), TRUE,
			       GrabModeSync, GrabModeAsync, CurrentTime );
	if ( (popupGrabOk = (r == GrabSuccess)) ) {
	    r = XGrabPointer( popup->x11Display(), popup->winId(), TRUE,
			      (uint)(ButtonPressMask | ButtonReleaseMask |
				     ButtonMotionMask | EnterWindowMask |
				     LeaveWindowMask | PointerMotionMask),
			      GrabModeSync, GrabModeAsync,
			      None, None, CurrentTime );

	    if ( (popupGrabOk = (r == GrabSuccess)) )
		XAllowEvents( popup->x11Display(), SyncPointer, CurrentTime );
	    else
		XUngrabKeyboard( popup->x11Display(), CurrentTime );
	}
    } else if ( popupGrabOk ) {
	XAllowEvents(  popup->x11Display(), SyncPointer, CurrentTime );
    }

    // popups are not focus-handled by the window system (the first
    // popup grabbed the keyboard), so we have to do that manually: A
    // new popup gets the focus
    QFocusEvent::setReason( QFocusEvent::Popup );
    if ( popup->focusWidget())
	popup->focusWidget()->setFocus();
    else
	popup->setFocus();
    QFocusEvent::resetReason();
}

void QApplication::closePopup( QWidget *popup )
{
    if ( !popupWidgets )
	return;
    popupWidgets->removeRef( popup );
    if (popup == popupOfPopupButtonFocus) {
	popupButtonFocus = 0;
	popupOfPopupButtonFocus = 0;
    }
    if ( popupWidgets->count() == 0 ) {		// this was the last popup
	popupCloseDownMode = TRUE;		// control mouse events
	delete popupWidgets;
	popupWidgets = 0;
	if ( !qt_nograb() && popupGrabOk ) {	// grabbing not disabled
	    if ( mouseButtonState != 0
		 || popup->geometry(). contains(QPoint(mouseGlobalXPos, mouseGlobalYPos) ) )
		{	// mouse release event or inside
		    XAllowEvents( popup->x11Display(), AsyncPointer,
				  CurrentTime );
	    } else {				// mouse press event
		mouseButtonPressTime -= 10000;	// avoid double click
		XAllowEvents( popup->x11Display(), ReplayPointer,CurrentTime );
	    }
	    XUngrabPointer( popup->x11Display(), CurrentTime );
	    XFlush( popup->x11Display() );
	}
	active_window = (*activeBeforePopup);
	// restore the former active window immediately, although
	// we'll get a focusIn later from X
	if ( active_window ) {
	    QFocusEvent::setReason( QFocusEvent::Popup );
	    if (active_window->focusWidget())
		active_window->focusWidget()->setFocus();
	    else
		active_window->setFocus();
	    QFocusEvent::resetReason();
	}
    }
     else {
	// popups are not focus-handled by the window system (the
	// first popup grabbed the keyboard), so we have to do that
	// manually: A popup was closed, so the previous popup gets
	// the focus.
	 QFocusEvent::setReason( QFocusEvent::Popup );
	 QWidget* aw = popupWidgets->getLast();
	 if (aw->focusWidget())
	     aw->focusWidget()->setFocus();
	 else
	     aw->setFocus();
	 QFocusEvent::resetReason();
	 if ( popupWidgets->count() == 1 && !qt_nograb() ){ // grab mouse/keyboard
	     int r = XGrabKeyboard( aw->x11Display(), aw->winId(), TRUE,
				    GrabModeSync, GrabModeAsync, CurrentTime );
	     if ( (popupGrabOk = (r == GrabSuccess)) ) {
		 r = XGrabPointer( aw->x11Display(), aw->winId(), TRUE,
				   (uint)(ButtonPressMask | ButtonReleaseMask |
					  ButtonMotionMask | EnterWindowMask |
					  LeaveWindowMask | PointerMotionMask),
				   GrabModeSync, GrabModeAsync,
				   None, None, CurrentTime );

		 if ( (popupGrabOk = (r == GrabSuccess)) )
		     XAllowEvents( aw->x11Display(), SyncPointer, CurrentTime );
	     }
	 }
     }
}

/*****************************************************************************
  Event translation; translates X11 events to Qt events
 *****************************************************************************/

//
// Mouse event translation
//
// Xlib doesn't give mouse double click events, so we generate them by
// comparing window, time and position between two mouse press events.
//

//
// Keyboard event translation
//

static int translateButtonState( int s )
{
    int bst = 0;
    if ( s & Button1Mask )
	bst |= Qt::LeftButton;
    if ( s & Button2Mask )
	bst |= Qt::MidButton;
    if ( s & Button3Mask )
	bst |= Qt::RightButton;
    if ( s & ShiftMask )
	bst |= Qt::ShiftButton;
    if ( s & ControlMask )
	bst |= Qt::ControlButton;
    if ( s & qt_alt_mask )
	bst |= Qt::AltButton;
    if ( s & qt_meta_mask )
	bst |= Qt::MetaButton;
    return bst;
}

bool QETWidget::translateMouseEvent( const XEvent *event )
{
    static bool manualGrab = FALSE;
    QEvent::Type type;				// event parameters
    QPoint pos;
    QPoint globalPos;
    int button = 0;
    int state;
    XEvent nextEvent;

    if ( sm_blockUserInput ) // block user interaction during session management
	return TRUE;

    static int x_root_save = -1, y_root_save = -1;

    if ( event->type == MotionNotify ) { // mouse move
	if (event->xmotion.root != RootWindow(appDpy, x11Screen()) &&
	    ! qt_xdnd_dragging )
	    return FALSE;

	XMotionEvent lastMotion = event->xmotion;
	while( XPending( appDpy ) )  { // compres mouse moves
	    XNextEvent( appDpy, &nextEvent );
	    if ( nextEvent.type == ConfigureNotify
		 || nextEvent.type == PropertyNotify
		 || nextEvent.type == Expose
		 || nextEvent.type == NoExpose ) {
		qApp->x11ProcessEvent( &nextEvent );
		continue;
	    } else if ( nextEvent.type != MotionNotify ||
			nextEvent.xmotion.window != event->xmotion.window ||
			nextEvent.xmotion.state != event->xmotion.state ) {
		XPutBackEvent( appDpy, &nextEvent );
		break;
	    }
	    if ( !qt_x11EventFilter(&nextEvent)
		 && !x11Event( &nextEvent ) ) // send event through filter
		lastMotion = nextEvent.xmotion;
	    else
		break;
	}
	type = QEvent::MouseMove;
	pos.rx() = lastMotion.x;
	pos.ry() = lastMotion.y;
	globalPos.rx() = lastMotion.x_root;
	globalPos.ry() = lastMotion.y_root;
	state = translateButtonState( lastMotion.state );
	if ( qt_button_down && (state & (LeftButton |
					 MidButton |
					 RightButton ) ) == 0 )
	    qt_button_down = 0;

	// throw away mouse move events that are sent multiple times to the same
	// position
	bool throw_away = FALSE;
	if ( x_root_save == globalPos.x() &&
	     y_root_save == globalPos.y() )
	    throw_away = TRUE;
	x_root_save = globalPos.x();
	y_root_save = globalPos.y();
	if ( throw_away )
	    return TRUE;
    } else if ( event->type == EnterNotify || event->type == LeaveNotify) {
	XEvent *xevent = (XEvent *)event;
	//unsigned int xstate = event->xcrossing.state;
	type = QEvent::MouseMove;
	pos.rx() = xevent->xcrossing.x;
	pos.ry() = xevent->xcrossing.y;
	globalPos.rx() = xevent->xcrossing.x_root;
	globalPos.ry() = xevent->xcrossing.y_root;
	state = translateButtonState( xevent->xcrossing.state );
	if ( qt_button_down && (state & (LeftButton |
					 MidButton |
					 RightButton ) ) == 0 )
	    qt_button_down = 0;
	if ( !qt_button_down )
	    state = state & ~(LeftButton | MidButton | RightButton );

	// throw away mouse move events that are sent multiple times to the same
	// position
	bool throw_away = FALSE;
	if ( x_root_save == globalPos.x() &&
	     y_root_save == globalPos.y() )
	    throw_away = TRUE;
	x_root_save = globalPos.x();
	y_root_save = globalPos.y();
	if ( throw_away )
	    return TRUE;
    } else {					// button press or release
	pos.rx() = event->xbutton.x;
	pos.ry() = event->xbutton.y;
	globalPos.rx() = event->xbutton.x_root;
	globalPos.ry() = event->xbutton.y_root;
	state = translateButtonState( event->xbutton.state );
	switch ( event->xbutton.button ) {
	case Button1: button = LeftButton;   goto DoFocus;
	case Button2: button = MidButton;    goto DoFocus;
	case Button3: button = RightButton;       DoFocus:
	    if ( isEnabled() && event->type == ButtonPress ) {
		QWidget* w = this;
		while ( w->focusProxy() )
		    w = w->focusProxy();
		if ( w->focusPolicy() & QWidget::ClickFocus ) {
		    QFocusEvent::setReason( QFocusEvent::Mouse);
		    w->setFocus();
		    QFocusEvent::resetReason();
		}
	    }
	    break;
	case Button4:
	case Button5:
	    // the fancy mouse wheel.

	    // take care about grabbing. We do this here since it
	    // is clear that we return anyway
	    if ( qApp->inPopupMode() && popupGrabOk )
		XAllowEvents( x11Display(), SyncPointer, CurrentTime );

	    // We are only interested in ButtonPress.
	    if (event->type == ButtonPress ){

		// compress wheel events (the X Server will simply
		// send a button press for each single notch,
		// regardless whether the application can catch up
		// or not)
		int delta = 1;
		XEvent xevent;
		while ( XCheckTypedWindowEvent(x11Display(),winId(),
					       ButtonPress,&xevent) ){
		    if (xevent.xbutton.button != event->xbutton.button){
			XPutBackEvent(x11Display(), &xevent);
			break;
		    }
		    delta++;
		}

		// the delta is defined as multiples of
		// WHEEL_DELTA, which is set to 120. Future wheels
		// may offer a finer-resolution. A positive delta
		// indicates forward rotation, a negative one
		// backward rotation respectively.
		int btn = event->xbutton.button;
		delta *= 120 * ( (btn == Button4) ? 1 : -1 );
		translateWheelEvent( globalPos.x(), globalPos.y(), delta, state, (state&AltButton)?Horizontal:Vertical );
	    }
	    return TRUE;
	}
	if ( event->type == ButtonPress ) {	// mouse button pressed
#if defined(Q_OS_IRIX) && defined(QT_TABLET_SUPPORT)
	    XEvent myEv;
	    XPeekEvent( appDpy, &myEv );
	    if ( myEv.type == xinput_button_press ) {
		XNextEvent( appDpy, &myEv );
		if ( translateXinputEvent( &myEv ) ) {
		    //Spontaneous event sent.  Check if we need to continue.
		    if ( chokeMouse ) {
			chokeMouse = FALSE;
			return FALSE;
		    }
		}
	    }
#endif
	    qt_button_down = childAt( pos );	//magic for masked widgets
	    if ( !qt_button_down || !qt_button_down->testWFlags(WMouseNoMask) )
		qt_button_down = this;
	    if ( mouseActWindow == event->xbutton.window &&
		 mouseButtonPressed == button &&
		 (long)event->xbutton.time -(long)mouseButtonPressTime
		 < QApplication::doubleClickInterval() &&
		 QABS(event->xbutton.x - mouseXPos) < 5 &&
		 QABS(event->xbutton.y - mouseYPos) < 5 ) {
		type = QEvent::MouseButtonDblClick;
		mouseButtonPressTime -= 2000;	// no double-click next time
	    } else {
		type = QEvent::MouseButtonPress;
		mouseButtonPressTime = event->xbutton.time;
	    }
	    mouseButtonPressed = button;	// save event params for
	    mouseXPos = pos.x();		// future double click tests
	    mouseYPos = pos.y();
	    mouseGlobalXPos = globalPos.x();
	    mouseGlobalYPos = globalPos.y();
	} else {				// mouse button released
#if defined(Q_OS_IRIX) && defined(QT_TABLET_SUPPORT)
	    XEvent myEv;
	    XPeekEvent( appDpy, &myEv );
	    if ( myEv.type == xinput_button_release ) {
		XNextEvent( appDpy, &myEv );
		if ( translateXinputEvent( &myEv ) ) {
		    //Spontaneous event sent.  I guess we just check if we should continue.
		    if ( chokeMouse ) {
			chokeMouse = FALSE;
			return FALSE;
		    }
		}
	    }
#endif
	    if ( manualGrab ) {			// release manual grab
		manualGrab = FALSE;
		XUngrabPointer( x11Display(), CurrentTime );
		XFlush( x11Display() );
	    }

	    type = QEvent::MouseButtonRelease;
	}
    }
    mouseActWindow = winId();			// save some event params
    mouseButtonState = state;
    if ( type == 0 )				// don't send event
	return FALSE;

    if ( qApp->inPopupMode() ) {			// in popup mode
	QWidget *popup = qApp->activePopupWidget();
	if ( popup != this ) {
	    if ( testWFlags(WType_Popup) && rect().contains(pos) )
		popup = this;
	    else				// send to last popup
		pos = popup->mapFromGlobal( globalPos );
	}
	bool releaseAfter = FALSE;
	QWidget *popupChild  = popup->childAt( pos );
	QWidget *popupTarget = popupChild ? popupChild : popup;

	if (popup != popupOfPopupButtonFocus){
	    popupButtonFocus = 0;
	    popupOfPopupButtonFocus = 0;
	}

	if ( !popupTarget->isEnabled() ) {
	    if ( popupGrabOk )
		XAllowEvents( x11Display(), SyncPointer, CurrentTime );
	}

	switch ( type ) {
	case QEvent::MouseButtonPress:
	case QEvent::MouseButtonDblClick:
	    popupButtonFocus = popupChild;
	    popupOfPopupButtonFocus = popup;
	    break;
	case QEvent::MouseButtonRelease:
	    releaseAfter = TRUE;
	    break;
	default:
	    break;				// nothing for mouse move
	}

	Display* dpy = x11Display(); // store display, send() may destroy us


	int oldOpenPopupCount = openPopupCount;

	if ( popupButtonFocus ) {
	    QMouseEvent e( type, popupButtonFocus->mapFromGlobal(globalPos),
			   globalPos, button, state );
	    QApplication::sendSpontaneousEvent( popupButtonFocus, &e );
	    if ( releaseAfter ) {
		popupButtonFocus = 0;
		popupOfPopupButtonFocus = 0;
	    }
	} else if ( popupChild ) {
	    QMouseEvent e( type, popupChild->mapFromGlobal(globalPos),
			   globalPos, button, state );
	    QApplication::sendSpontaneousEvent( popupChild, &e );
	} else {
	    QMouseEvent e( type, pos, globalPos, button, state );
	    QApplication::sendSpontaneousEvent( popup, &e );
	}

	if ( type == QEvent::MouseButtonPress && button == RightButton && ( openPopupCount == oldOpenPopupCount ) ) {
	    QWidget *popupEvent = popup;
	    if(popupButtonFocus)
		popupEvent = popupButtonFocus;
	    else if(popupChild)
		popupEvent = popupChild;
	    QContextMenuEvent e( QContextMenuEvent::Mouse, pos, globalPos, state );
	    QApplication::sendSpontaneousEvent( popupEvent, &e );
	}

	if ( releaseAfter )
	    qt_button_down = 0;

	if ( qApp->inPopupMode() ) { // still in popup mode
	    if ( popupGrabOk )
		XAllowEvents( dpy, SyncPointer, CurrentTime );
	} else {
	    if ( type != QEvent::MouseButtonRelease && state != 0 &&
		 QWidget::find((WId)mouseActWindow) ) {
		manualGrab = TRUE;		// need to manually grab
		XGrabPointer( dpy, mouseActWindow, False,
			      (uint)(ButtonPressMask | ButtonReleaseMask |
				     ButtonMotionMask |
				     EnterWindowMask | LeaveWindowMask),
			      GrabModeAsync, GrabModeAsync,
			      None, None, CurrentTime );
	    }
	}

    } else {
	QWidget *widget = this;
	QWidget *w = QWidget::mouseGrabber();
	if ( !w )
	    w = qt_button_down;
	if ( w && w != this ) {
	    widget = w;
	    pos = w->mapFromGlobal( globalPos );
	}

	if ( popupCloseDownMode ) {
	    popupCloseDownMode = FALSE;
	    if ( testWFlags(WType_Popup) )	// ignore replayed event
		return TRUE;
	}

	if ( type == QEvent::MouseButtonRelease &&
	     (state & (~button) & ( LeftButton |
				    MidButton |
				    RightButton)) == 0 ) {
	    qt_button_down = 0;
	}

	int oldOpenPopupCount = openPopupCount;

	QMouseEvent e( type, pos, globalPos, button, state );
	QApplication::sendSpontaneousEvent( widget, &e );

	if ( type == QEvent::MouseButtonPress && button == RightButton && ( openPopupCount == oldOpenPopupCount ) ) {
	    QContextMenuEvent e( QContextMenuEvent::Mouse, pos, globalPos, state );
	    QApplication::sendSpontaneousEvent( widget, &e );
	}
    }
    return TRUE;
}


//
// Wheel event translation
//
bool QETWidget::translateWheelEvent( int global_x, int global_y, int delta, int state, Orientation orient )
{
    QWidget* w = this;

    while ( w->focusProxy() )
	w = w->focusProxy();
    if ( w->focusPolicy() == QWidget::WheelFocus ) {
	QFocusEvent::setReason( QFocusEvent::Mouse);
	w->setFocus();
	QFocusEvent::resetReason();
    }

    // send the event to the widget or its ancestors
    {
	QWidget* popup = qApp->activePopupWidget();
	if ( popup && w->topLevelWidget() != popup )
	    popup->close();
	QWheelEvent e( w->mapFromGlobal(QPoint( global_x, global_y)),
		       QPoint(global_x, global_y), delta, state, orient );
	if ( QApplication::sendSpontaneousEvent( w, &e ) )
	    return TRUE;
    }

    // send the event to the widget that has the focus or its ancestors, if different
    if ( w != qApp->focusWidget() && ( w = qApp->focusWidget() ) ) {
	QWidget* popup = qApp->activePopupWidget();
	if ( popup && w != popup )
	    popup->hide();
	QWheelEvent e( w->mapFromGlobal(QPoint( global_x, global_y)),
		       QPoint(global_x, global_y), delta, state, orient );
	if ( QApplication::sendSpontaneousEvent( w, &e ) )
	    return TRUE;
    }
    return FALSE;
}


//
// XInput Translation Event
//
#if defined (QT_TABLET_SUPPORT)
bool QETWidget::translateXinputEvent( const XEvent *ev )
{
#if defined (Q_OS_IRIX)
    // Wacom has put defines in their wacom.h file so it would be quite wise
    // to use them, need to think of a decent way of not using
    // it when it doesn't exist...
    XDeviceState *s;
    XInputClass *iClass;
    XValuatorState *vs;
    int j;
#endif
    QWidget *w = this;
    QPoint global,
	curr;
    static int pressure = 0;
    static int xTilt = 0,
	       yTilt = 0;
    int deviceType = QTabletEvent::NoDevice;
    QPair<int, int> tId;
    XDevice *dev;
    XDeviceMotionEvent *motion = 0;
    XDeviceButtonEvent *button = 0;
    QEvent::Type t;

    if ( ev->type == xinput_motion ) {
	motion = (XDeviceMotionEvent*)ev;
	t = QEvent::TabletMove;
	curr = QPoint( motion->x, motion->y );
/*
	qDebug( "\n\nXInput Crazy Motion Event" );
	qDebug( "serial:\t%d", motion->serial );
	qDebug( "send_event:\t%d", motion->send_event );
	qDebug( "display:\t%p", motion->display );
	qDebug( "window:\t%d", motion->window );
	qDebug( "deviceID:\t%d", motion->deviceid );
	qDebug( "root:\t%d", motion->root );
	qDebug( "subwindot:\t%d", motion->subwindow );
	qDebug( "x:\t%d", motion->x );
	qDebug( "y:\t%d", motion->y );
	qDebug( "x_root:\t%d", motion->x_root );
	qDebug( "y_root:\t%d", motion->y_root );
	qDebug( "state:\t%d", motion->state );
	qDebug( "is_hint:\t%d", motion->is_hint );
	qDebug( "same_screen:\t%d", motion->same_screen );
	qDebug( "time:\t%d", motion->time );
*/
    } else {
	if ( ev->type == xinput_button_press ) {
	    t = QEvent::TabletPress;
        } else {
	    t = QEvent::TabletRelease;
	}
	button = (XDeviceButtonEvent*)ev;
/*
	qDebug( "\n\nXInput Button Event" );
	qDebug( "serial:\t%d", button->serial );
	qDebug( "send_event:\t%d", button->send_event );
	qDebug( "display:\t%p", button->display );
	qDebug( "window:\t%d", button->window );
	qDebug( "deviceID:\t%d", button->deviceid );
	qDebug( "root:\t%d", button->root );
	qDebug( "subwindot:\t%d", button->subwindow );
	qDebug( "x:\t%d", button->x );
	qDebug( "y:\t%d", button->y );
	qDebug( "x_root:\t%d", button->x_root );
	qDebug( "y_root:\t%d", button->y_root );
	qDebug( "state:\t%d", button->state );
	qDebug( "button:\t%d", button->button );
	qDebug( "same_screen:\t%d", button->same_screen );
	qDebug( "time:\t%d", button->time );
*/


	curr = QPoint( button->x, button->y );
    }
#if defined(Q_OS_IRIX)
    // default...
    dev = devStylus;
#else
    if ( ev->type == xinput_motion ) {
	if ( motion->deviceid == devStylus->device_id ) {
	    dev = devStylus;
	    deviceType = QTabletEvent::Stylus;
	} else if ( motion->deviceid == devEraser->device_id ) {
	    dev = devEraser;
	    deviceType = QTabletEvent::Eraser;
	}
    } else {
	if ( button->deviceid == devStylus->device_id ) {
	    dev = devStylus;
	    deviceType = QTabletEvent::Stylus;
	} else if ( button->deviceid == devEraser->device_id ) {
	    dev = devEraser;
	    deviceType = QTabletEvent::Eraser;
	}
    }
#endif

    const int PRESSURE_LEVELS = 255;
    // we got the maximum pressure at start time, since various tablets have
    // varying levels of distinguishing pressure changes, let's standardize and
    // scale everything to 256 different levels...
    static int scaleFactor = -1;
    if ( scaleFactor == -1 ) {
	if ( max_pressure > PRESSURE_LEVELS )
	    scaleFactor = max_pressure / PRESSURE_LEVELS;
	else
	    scaleFactor = PRESSURE_LEVELS / max_pressure;
    }
#if defined (Q_OS_IRIX)
    s = XQueryDeviceState( appDpy, dev );
    if ( s == NULL )
        return FALSE;
    iClass = s->data;
    for ( j = 0; j < s->num_classes; j++ ) {
        if ( iClass->c_class == ValuatorClass ) {
            vs = (XValuatorState *)iClass;
            // figure out what device we have, based on bitmasking...
            if ( vs->valuators[WAC_TRANSDUCER_I]
                 & WAC_TRANSDUCER_PROX_MSK ) {
                switch ( vs->valuators[WAC_TRANSDUCER_I]
                         & WAC_TRANSDUCER_MSK ) {
                case WAC_PUCK_ID:
                    deviceType = QTabletEvent::Puck;
                    break;
                case WAC_STYLUS_ID:
                    deviceType = QTabletEvent::Stylus;
                    break;
                case WAC_ERASER_ID:
                    deviceType = QTabletEvent::Eraser;
                    break;
                }
                // Get a Unique Id for the device, Wacom gives us this ability
                tId.first = vs->valuators[WAC_TRANSDUCER_I] & WAC_TRANSDUCER_ID_MSK;
                tId.second = vs->valuators[WAC_SERIAL_NUM_I];
            } else
                deviceType = QTabletEvent::NoDevice;
            // apparently Wacom needs a cast for the +/- values to make sense
            xTilt = short(vs->valuators[WAC_XTILT_I]);
            yTilt = short(vs->valuators[WAC_YTILT_I]);
            if ( max_pressure > PRESSURE_LEVELS )
                pressure = vs->valuators[WAC_PRESSURE_I] / scaleFactor;
            else
                pressure = vs->valuators[WAC_PRESSURE_I] * scaleFactor;
	    global = QPoint( vs->valuators[WAC_XCOORD_I],
                             vs->valuators[WAC_YCOORD_I] );
	    break;
	}
	iClass = (XInputClass*)((char*)iClass + iClass->length);
    }
    XFreeDeviceState( s );
#else
    if ( motion ) {
	xTilt = short(motion->axis_data[3]);
	yTilt = short(motion->axis_data[4]);
	if ( max_pressure > PRESSURE_LEVELS )
	    pressure = motion->axis_data[2] / scaleFactor;
	else
	    pressure = motion->axis_data[2] * scaleFactor;
	global = QPoint( motion->axis_data[0], motion->axis_data[1] );
    } else {
	xTilt = short(button->axis_data[3]);
	yTilt = short(button->axis_data[4]);
	if ( max_pressure > PRESSURE_LEVELS )
	    pressure = button->axis_data[2]  / scaleFactor;
	else
	    pressure = button->axis_data[2] * scaleFactor;
	global = QPoint( button->axis_data[0], button->axis_data[1] );
    }
    // The only way to get these Ids is to scan the XFree86 log, which I'm not going to do.
    tId.first = tId.second = -1;
#endif

    QTabletEvent e( t, curr, global, deviceType, pressure, xTilt, yTilt, tId );
    QApplication::sendSpontaneousEvent( w, &e );
    return TRUE;
}
#endif

#ifndef XK_ISO_Left_Tab
#define	XK_ISO_Left_Tab					0xFE20
#endif

// the next lines are taken from XFree > 4.0 (X11/XF86keysyms.h), defining some special
// multimedia keys. They are included here as not every system has them.
#define XF86XK_Standby		0x1008FF10
#define XF86XK_AudioLowerVolume	0x1008FF11
#define XF86XK_AudioMute	0x1008FF12
#define XF86XK_AudioRaiseVolume	0x1008FF13
#define XF86XK_AudioPlay	0x1008FF14
#define XF86XK_AudioStop	0x1008FF15
#define XF86XK_AudioPrev	0x1008FF16
#define XF86XK_AudioNext	0x1008FF17
#define XF86XK_HomePage		0x1008FF18
#define XF86XK_Calculator	0x1008FF1D
#define XF86XK_Mail		0x1008FF19
#define XF86XK_Start		0x1008FF1A
#define XF86XK_Search		0x1008FF1B
#define XF86XK_AudioRecord	0x1008FF1C
#define XF86XK_Back		0x1008FF26
#define XF86XK_Forward		0x1008FF27
#define XF86XK_Stop		0x1008FF28
#define XF86XK_Refresh		0x1008FF29
#define XF86XK_Favorites	0x1008FF30
#define XF86XK_AudioPause	0x1008FF31
#define XF86XK_AudioMedia	0x1008FF32
#define XF86XK_MyComputer	0x1008FF33
#define XF86XK_OpenURL		0x1008FF38
#define XF86XK_Launch0		0x1008FF40
#define XF86XK_Launch1		0x1008FF41
#define XF86XK_Launch2		0x1008FF42
#define XF86XK_Launch3		0x1008FF43
#define XF86XK_Launch4		0x1008FF44
#define XF86XK_Launch5		0x1008FF45
#define XF86XK_Launch6		0x1008FF46
#define XF86XK_Launch7		0x1008FF47
#define XF86XK_Launch8		0x1008FF48
#define XF86XK_Launch9		0x1008FF49
#define XF86XK_LaunchA		0x1008FF4A
#define XF86XK_LaunchB		0x1008FF4B
#define XF86XK_LaunchC		0x1008FF4C
#define XF86XK_LaunchD		0x1008FF4D
#define XF86XK_LaunchE		0x1008FF4E
#define XF86XK_LaunchF		0x1008FF4F
// end of XF86keysyms.h



static const KeySym KeyTbl[] = {		// keyboard mapping table
    XK_Escape,		Qt::Key_Escape,		// misc keys
    XK_Tab,		Qt::Key_Tab,
    XK_ISO_Left_Tab,    Qt::Key_Backtab,
    XK_BackSpace,	Qt::Key_Backspace,
    XK_Return,		Qt::Key_Return,
    XK_Insert,		Qt::Key_Insert,
    XK_KP_Insert,	Qt::Key_Insert,
    XK_Delete,		Qt::Key_Delete,
    XK_KP_Delete,	Qt::Key_Delete,
    XK_Clear,		Qt::Key_Delete,
    XK_Pause,		Qt::Key_Pause,
    XK_Print,		Qt::Key_Print,
    XK_KP_Begin,	Qt::Key_Clear,
    0x1005FF60,		Qt::Key_SysReq,		// hardcoded Sun SysReq
    0x1007ff00,		Qt::Key_SysReq,		// hardcoded X386 SysReq
    XK_Home,		Qt::Key_Home,		// cursor movement
    XK_End,		Qt::Key_End,
    XK_Left,		Qt::Key_Left,
    XK_Up,		Qt::Key_Up,
    XK_Right,		Qt::Key_Right,
    XK_Down,		Qt::Key_Down,
    XK_Prior,		Qt::Key_Prior,
    XK_Next,		Qt::Key_Next,
    XK_KP_Home,		Qt::Key_Home,
    XK_KP_End,		Qt::Key_End,
    XK_KP_Left,		Qt::Key_Left,
    XK_KP_Up,		Qt::Key_Up,
    XK_KP_Right,	Qt::Key_Right,
    XK_KP_Down,		Qt::Key_Down,
    XK_KP_Prior,	Qt::Key_Prior,
    XK_KP_Next,		Qt::Key_Next,
    XK_Shift_L,		Qt::Key_Shift,		// modifiers
    XK_Shift_R,		Qt::Key_Shift,
    XK_Shift_Lock,	Qt::Key_Shift,
    XK_Control_L,	Qt::Key_Control,
    XK_Control_R,	Qt::Key_Control,
    XK_Meta_L,		Qt::Key_Meta,
    XK_Meta_R,		Qt::Key_Meta,
    XK_Alt_L,		Qt::Key_Alt,
    XK_Alt_R,		Qt::Key_Alt,
    XK_Caps_Lock,	Qt::Key_CapsLock,
    XK_Num_Lock,	Qt::Key_NumLock,
    XK_Scroll_Lock,	Qt::Key_ScrollLock,
    XK_KP_Space,	Qt::Key_Space,		// numeric keypad
    XK_KP_Tab,		Qt::Key_Tab,
    XK_KP_Enter,	Qt::Key_Enter,
    XK_KP_Equal,	Qt::Key_Equal,
    XK_KP_Multiply,	Qt::Key_Asterisk,
    XK_KP_Add,		Qt::Key_Plus,
    XK_KP_Separator,	Qt::Key_Comma,
    XK_KP_Subtract,	Qt::Key_Minus,
    XK_KP_Decimal,	Qt::Key_Period,
    XK_KP_Divide,	Qt::Key_Slash,
    XK_Super_L,		Qt::Key_Super_L,
    XK_Super_R,		Qt::Key_Super_R,
    XK_Menu,		Qt::Key_Menu,
    XK_Hyper_L,		Qt::Key_Hyper_L,
    XK_Hyper_R,		Qt::Key_Hyper_R,
    XK_Help,		Qt::Key_Help,
    0x1000FF74,         Qt::Key_BackTab,     // hardcoded HP backtab

    // Special multimedia keys
    // currently only tested with MS internet keyboard

    // browsing keys
    XF86XK_Back,	Qt::Key_Back,
    XF86XK_Forward,	Qt::Key_Forward,
    XF86XK_Stop,	Qt::Key_Stop,
    XF86XK_Refresh,	Qt::Key_Refresh,
    XF86XK_Favorites,	Qt::Key_Favorites,
    XF86XK_AudioMedia,	Qt::Key_LaunchMedia,
    XF86XK_OpenURL,	Qt::Key_OpenUrl,
    XF86XK_HomePage,	Qt::Key_HomePage,
    XF86XK_Search,	Qt::Key_Search,

    // media keys
    XF86XK_AudioLowerVolume, Qt::Key_VolumeDown,
    XF86XK_AudioMute,	Qt::Key_VolumeMute,
    XF86XK_AudioRaiseVolume, Qt::Key_VolumeUp,
    XF86XK_AudioPlay,	Qt::Key_MediaPlay,
    XF86XK_AudioStop,	Qt::Key_MediaStop,
    XF86XK_AudioPrev,	Qt::Key_MediaPrev,
    XF86XK_AudioNext,	Qt::Key_MediaNext,
    XF86XK_AudioRecord,	Qt::Key_MediaRecord,

    // launch keys
    XF86XK_Mail,	Qt::Key_LaunchMail,
    XF86XK_MyComputer,	Qt::Key_Launch0,
    XF86XK_Calculator,	Qt::Key_Launch1,
    XF86XK_Standby, 	Qt::Key_Standby,

    XF86XK_Launch0,	Qt::Key_Launch2,
    XF86XK_Launch1,	Qt::Key_Launch3,
    XF86XK_Launch2,	Qt::Key_Launch4,
    XF86XK_Launch3,	Qt::Key_Launch5,
    XF86XK_Launch4,	Qt::Key_Launch6,
    XF86XK_Launch5,	Qt::Key_Launch7,
    XF86XK_Launch6,	Qt::Key_Launch8,
    XF86XK_Launch7,	Qt::Key_Launch9,
    XF86XK_Launch8,	Qt::Key_LaunchA,
    XF86XK_Launch9,	Qt::Key_LaunchB,
    XF86XK_LaunchA,	Qt::Key_LaunchC,
    XF86XK_LaunchB,	Qt::Key_LaunchD,
    XF86XK_LaunchC,	Qt::Key_LaunchE,
    XF86XK_LaunchD,	Qt::Key_LaunchF,

    0,			0
};


static QIntDict<void>    *keyDict  = 0;
static QIntDict<void>    *textDict = 0;

static void deleteKeyDicts()
{
    if ( keyDict )
	delete keyDict;
    keyDict = 0;
    if ( textDict )
	delete textDict;
    textDict = 0;
}

#if !defined(QT_NO_XIM)
static const unsigned short cyrillicKeysymsToUnicode[] = {
    0x0000, 0x0452, 0x0453, 0x0451, 0x0454, 0x0455, 0x0456, 0x0457,
    0x0458, 0x0459, 0x045a, 0x045b, 0x045c, 0x0000, 0x045e, 0x045f,
    0x2116, 0x0402, 0x0403, 0x0401, 0x0404, 0x0405, 0x0406, 0x0407,
    0x0408, 0x0409, 0x040a, 0x040b, 0x040c, 0x0000, 0x040e, 0x040f,
    0x044e, 0x0430, 0x0431, 0x0446, 0x0434, 0x0435, 0x0444, 0x0433,
    0x0445, 0x0438, 0x0439, 0x043a, 0x043b, 0x043c, 0x043d, 0x043e,
    0x043f, 0x044f, 0x0440, 0x0441, 0x0442, 0x0443, 0x0436, 0x0432,
    0x044c, 0x044b, 0x0437, 0x0448, 0x044d, 0x0449, 0x0447, 0x044a,
    0x042e, 0x0410, 0x0411, 0x0426, 0x0414, 0x0415, 0x0424, 0x0413,
    0x0425, 0x0418, 0x0419, 0x041a, 0x041b, 0x041c, 0x041d, 0x041e,
    0x041f, 0x042f, 0x0420, 0x0421, 0x0422, 0x0423, 0x0416, 0x0412,
    0x042c, 0x042b, 0x0417, 0x0428, 0x042d, 0x0429, 0x0427, 0x042a
};

static const unsigned short greekKeysymsToUnicode[] = {
    0x0000, 0x0386, 0x0388, 0x0389, 0x038a, 0x03aa, 0x0000, 0x038c,
    0x038e, 0x03ab, 0x0000, 0x038f, 0x0000, 0x0000, 0x0385, 0x2015,
    0x0000, 0x03ac, 0x03ad, 0x03ae, 0x03af, 0x03ca, 0x0390, 0x03cc,
    0x03cd, 0x03cb, 0x03b0, 0x03ce, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0391, 0x0392, 0x0393, 0x0394, 0x0395, 0x0396, 0x0397,
    0x0398, 0x0399, 0x039a, 0x039b, 0x039c, 0x039d, 0x039e, 0x039f,
    0x03a0, 0x03a1, 0x03a3, 0x0000, 0x03a4, 0x03a5, 0x03a6, 0x03a7,
    0x03a8, 0x03a9, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x03b1, 0x03b2, 0x03b3, 0x03b4, 0x03b5, 0x03b6, 0x03b7,
    0x03b8, 0x03b9, 0x03ba, 0x03bb, 0x03bc, 0x03bd, 0x03be, 0x03bf,
    0x03c0, 0x03c1, 0x03c3, 0x03c2, 0x03c4, 0x03c5, 0x03c6, 0x03c7,
    0x03c8, 0x03c9, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
};

// ### add keysym conversion for the missing ranges.
static QChar keysymToUnicode(unsigned char byte3, unsigned char byte4)
{
    if ( byte3 == 0x06 ) {
	// russian, use lookup table
	if ( byte4 > 0xa0 )
	    return QChar( cyrillicKeysymsToUnicode[byte4 - 0xa0] );
    }
    if ( byte3 == 0x07 ) {
	// greek
	if ( byte4 > 0xa0 )
	    return QChar( greekKeysymsToUnicode[byte4 - 0xa0] );
    }
    return QChar(0x0);
}
#endif


bool QETWidget::translateKeyEventInternal( const XEvent *event, int& count,
					   QString& text,
					   int& state,
					   char& ascii, int& code, QEvent::Type &type )
{
    QTextCodec *mapper = input_mapper;
    // some XmbLookupString implementations don't return buffer overflow correctly,
    // so we increase the input buffer to allow for long strings...
    // 256 chars * 2 bytes + 1 null-term == 513 bytes
    QCString chars(513);
    QChar converted;
    KeySym key = 0;

    if ( !keyDict ) {
	keyDict = new QIntDict<void>( 13 );
	keyDict->setAutoDelete( FALSE );
	textDict = new QIntDict<void>( 13 );
	textDict->setAutoDelete( FALSE );
	qAddPostRoutine( deleteKeyDicts );
    }

    QWidget* tlw = topLevelWidget();

    XKeyEvent xkeyevent = event->xkey;

    // save the modifier state, we will use the keystate uint later by passing
    // it to translateButtonState
    uint keystate = event->xkey.state;
    // remove the modifiers where mode_switch exists... HPUX machines seem
    // to have alt *AND* mode_switch both in Mod1Mask, which causes
    // XLookupString to return things like '�' (aring) for ALT-A.  This
    // completely breaks modifiers.  If we remove the modifier for Mode_switch,
    // then things work correctly...
    xkeyevent.state &= ~qt_mode_switch_remove_mask;

    type = (event->type == XKeyPress)
           ? QEvent::KeyPress : QEvent::KeyRelease;
#if defined(QT_NO_XIM)

    count = XLookupString( &xkeyevent, chars.data(), chars.size(), &key, 0 );

    if ( count == 1 )
	ascii = chars[0];

#else
    // Implementation for X11R5 and newer, using XIM

    int	       keycode = event->xkey.keycode;
    Status     status;

    if ( type == QEvent::KeyPress ) {
	bool mb=FALSE;
	if ( qt_xim ) {
	    QTLWExtra*  xd = tlw->topData();
	    QInputContext *qic = (QInputContext *) xd->xic;
	    if ( qic ) {
		mb=TRUE;
		count = qic->lookupString(&xkeyevent, chars, &key, &status);
	    }
	}
	if ( !mb ) {
	    count = XLookupString( &xkeyevent,
				   chars.data(), chars.size(), &key, 0 );
	}
	if ( count && !keycode ) {
	    keycode = composingKeycode;
	    composingKeycode = 0;
	}
	if ( key )
	    keyDict->replace( keycode, (void*)key );
	// all keysyms smaller than that are actally keys that can be mapped
	// to unicode chars
	if ( count == 0 && key < 0xff00 ) {
	    unsigned char byte3 = (unsigned char )(key >> 8);
	    int mib = -1;
	    switch( byte3 ) {
	    case 0: // Latin 1
	    case 1: // Latin 2
	    case 2: //latin 3
	    case 3: // latin4
		mib = byte3 + 4; break;
	    case 4: // kana
		break;
	    case 5: // arabic
		mib = 82; break;
	    case 6: // Cyrillic
	    case 7: //greek
		mib = -1; // manual conversion
		mapper = 0;
		converted = keysymToUnicode( byte3, key & 0xff );
	    case 8: // technical, no mapping here at the moment
	    case 9: // Special
	    case 10: // Publishing
	    case 11: // APL
		break;
	    case 12: // Hebrew
		mib = 85; break;
	    case 13: // Thai
		mib = 2259; break;
	    case 14: // Korean, no mapping
		break;
	    case 0x20:
		// currency symbols
		if ( key >= 0x20a0 && key <= 0x20ac ) {
		    mib = -1; // manual conversion
		    mapper = 0;
		    converted = (uint)key;
		}
		break;
	    default:
		break;
	    }
	    if ( mib != -1 ) {
		mapper = QTextCodec::codecForMib( mib );
		chars[0] = (unsigned char) (key & 0xff); // get only the fourth bit for conversion later
		count++;
	    }
	} else if ( key >= 0x1000000 && key <= 0x100ffff ) {
	    converted = (ushort) (key - 0x1000000);
	    mapper = 0;
	}
	if ( count < (int)chars.size()-1 )
	    chars[count] = '\0';
	if ( count == 1 ) {
	    ascii = chars[0];
	    // +256 so we can store all eight-bit codes, including ascii 0,
	    // and independent of whether char is signed or not.
	    textDict->replace( keycode, (void*)(256+ascii) );
	}
	tlw = 0;
    } else {
	key = (int)(long)keyDict->find( keycode );
	if ( key )
	    keyDict->take( keycode );
	long s = (long)textDict->find( keycode );
	if ( s ) {
	    textDict->take( keycode );
	    ascii = (char)(s-256);
	}
    }
#endif // !QT_NO_XIM

    state = translateButtonState( keystate );

    static int directionKeyEvent = 0;
    if ( qt_use_rtl_extensions && type == QEvent::KeyRelease ) {
	if (directionKeyEvent == Key_Direction_R || directionKeyEvent == Key_Direction_L ) {
	    type = QEvent::KeyPress;
	    code = directionKeyEvent;
	    chars[0] = 0;
	    directionKeyEvent = 0;
	    return TRUE;
	} else {
	    directionKeyEvent = 0;
	}
    }

    // Commentary in X11/keysymdef says that X codes match ASCII, so it
    // is safe to use the locale functions to process X codes in ISO8859-1.
    //
    // This is mainly for compatibility - applications should not use the
    // Qt keycodes between 128 and 255, but should rather use the
    // QKeyEvent::text().
    //
    if ( key < 128 || (key < 256 && (!input_mapper || input_mapper->mibEnum()==4)) ) {
	code = isprint((int)key) ? toupper((int)key) : 0; // upper-case key, if known
	directionKeyEvent = Key_Space;
    } else if ( key >= XK_F1 && key <= XK_F35 ) {
	code = Key_F1 + ((int)key - XK_F1);	// function keys
	directionKeyEvent = Key_Space;
    } else if ( key >= XK_KP_0 && key <= XK_KP_9) {
	code = Key_0 + ((int)key - XK_KP_0);	// numeric keypad keys
	state |= Keypad;
	directionKeyEvent = Key_Space;
    } else {
	int i = 0;				// any other keys
	while ( KeyTbl[i] ) {
	    if ( key == KeyTbl[i] ) {
		code = (int)KeyTbl[i+1];
		break;
	    }
	    i += 2;
	}
	switch ( key ) {
	case XK_KP_Insert:
	case XK_KP_Delete:
	case XK_KP_Home:
	case XK_KP_End:
	case XK_KP_Left:
	case XK_KP_Up:
	case XK_KP_Right:
	case XK_KP_Down:
	case XK_KP_Prior:
	case XK_KP_Next:
	case XK_KP_Space:
	case XK_KP_Tab:
	case XK_KP_Enter:
	case XK_KP_Equal:
	case XK_KP_Multiply:
	case XK_KP_Add:
	case XK_KP_Separator:
	case XK_KP_Subtract:
	case XK_KP_Decimal:
	case XK_KP_Divide:
	    state |= Keypad;
	    break;
	default:
	    break;
	}

	if ( code == Key_Tab &&
	     (state & ShiftButton) == ShiftButton ) {
            // map shift+tab to shift+backtab, QAccel knows about it
            // and will handle it.
	    code = Key_Backtab;
	    chars[0] = 0;
	}

	if ( qt_use_rtl_extensions && type  == QEvent::KeyPress ) {
	    if ( directionKeyEvent ) {
		if ( key == XK_Shift_L && directionKeyEvent == XK_Control_L ||
		     key == XK_Control_L && directionKeyEvent == XK_Shift_L ) {
		    directionKeyEvent = Key_Direction_L;
		} else if ( key == XK_Shift_R && directionKeyEvent == XK_Control_R ||
			    key == XK_Control_R && directionKeyEvent == XK_Shift_R ) {
		    directionKeyEvent = Key_Direction_R;
		}
	    } else if ( !directionKeyEvent )
		directionKeyEvent = key;
	    else if ( directionKeyEvent == Key_Direction_L || directionKeyEvent == Key_Direction_R ) {
		directionKeyEvent = Key_Space; // invalid
	    }
	}
    }

#if 0
#ifndef Q_EE
    static int c  = 0;
    extern void qt_dialog_default_key();
#define Q_EE(x) c = (c == x || (!c && x == 0x1000) )? x+1 : 0
    if ( tlw && state == '0' ) {
	switch ( code ) {
	case 0x4f: Q_EE(Key_Backtab); break;
	case 0x52: Q_EE(Key_Tab); break;
	case 0x54: Q_EE(Key_Escape); break;
	case 0x4c:
	    if (c == Key_Return )
		qt_dialog_default_key();
	    else
		Q_EE(Key_Backspace);
	    break;
	}
    }
#undef Q_EE
#endif
#endif

    // convert chars (8bit) to text (unicode).
    if ( mapper )
	text = mapper->toUnicode(chars,count);
    else if ( !mapper && converted.unicode() != 0x0 )
	text = converted;
    else
	text = chars;
    return TRUE;
}


bool QETWidget::translateKeyEvent( const XEvent *event, bool grab )
{
    int	   code = -1;
    int	   count = 0;
    int	   state;
    char   ascii = 0;

    if ( sm_blockUserInput ) // block user interaction during session management
	return TRUE;

    Display *dpy = x11Display();

    if ( !isEnabled() )
	return TRUE;

    QEvent::Type type;
    bool    autor = FALSE;
    QString text;

    translateKeyEventInternal( event, count, text, state, ascii, code, type );

    // once chained accelerators are possible in Qt, the accelavailable code should
    // be removed completely from Qt, and instead we should use an internal
    // QAccelManager which we feed key events, and it will tell us if the key
    // is an accelerator (in which case we return, doing nothing) or a normal
    // key event

    // process accelerators before doing key compression
    bool isAccel = FALSE;
    if ( !grab ) { // test for accel if the keyboard is not grabbed
	QKeyEvent a( QEvent::AccelAvailable, code, ascii, state, text, FALSE,
		     QMAX(count, int(text.length())) );
	a.ignore();
	QApplication::sendSpontaneousEvent( topLevelWidget(), &a );
	isAccel = a.isAccepted();
    }
    if ( type == QEvent::KeyPress && !grab ) {
	// send accel events if the keyboard is not grabbed
	QKeyEvent aa( QEvent::AccelOverride, code, ascii, state, text, autor,
		      QMAX(count, int(text.length())) );
	aa.ignore();
	QApplication::sendSpontaneousEvent( this, &aa );
	if ( !aa.isAccepted() ) {
	    QKeyEvent a( QEvent::Accel, code, ascii, state, text, autor,
			 QMAX(count, int(text.length())) );
	    a.ignore();
	    QApplication::sendSpontaneousEvent( topLevelWidget(), &a );
	    if ( a.isAccepted() )
		return TRUE;
	}
    }

    long save = 0;
    if ( qt_mode_switch_remove_mask != 0 ) {
	save = qt_mode_switch_remove_mask;
	qt_mode_switch_remove_mask = 0;

	// translate the key event again, but this time apply any Mode_switch
	// modifiers
	translateKeyEventInternal( event, count, text, state, ascii, code, type );
    }

    // compress keys
    if ( !isAccel && !text.isEmpty() && testWState(WState_CompressKeys) &&
	 // do not compress keys if the key event we just got above matches
	 // one of the key ranges used to compute stopCompression
	 ! ( ( code >= Key_Escape && code <= Key_SysReq ) ||
	     ( code >= Key_Home && code <= Key_Next ) ||
	     ( code >= Key_Super_L && code <= Key_Direction_R ) ||
	     ( ( code == 0 ) && ( ascii == '\n' ) ) ) ) {
	// the widget wants key compression so it gets it
	int	codeIntern = -1;
	int	countIntern = 0;
	int	stateIntern;
	char	asciiIntern = 0;
	XEvent	evRelease;
	XEvent	evPress;
	for (;;) {
	    QString textIntern;
	    if ( !XCheckTypedWindowEvent(dpy,event->xkey.window,
					 XKeyRelease,&evRelease) )
		break;
	    if ( !XCheckTypedWindowEvent(dpy,event->xkey.window,
					 XKeyPress,&evPress) ) {
		XPutBackEvent(dpy, &evRelease);
		break;
	    }
	    QEvent::Type t;
	    translateKeyEventInternal( &evPress, countIntern, textIntern,
				       stateIntern, asciiIntern, codeIntern, t );
	    // use stopCompression to stop key compression for the following
	    // key event ranges:
	    bool stopCompression =
		// 1) misc keys
		( codeIntern >= Key_Escape && codeIntern <= Key_SysReq ) ||
		// 2) cursor movement
		( codeIntern >= Key_Home && codeIntern <= Key_Next ) ||
		// 3) extra keys
		( codeIntern >= Key_Super_L && codeIntern <= Key_Direction_R ) ||
		// 4) something that a) doesn't translate to text or b) translates
		//    to newline text
		((codeIntern == 0) && (asciiIntern == '\n'));
	    if (stateIntern == state && !textIntern.isEmpty() && !stopCompression) {
		if (!grab) { // test for accel if the keyboard is not grabbed
		    QKeyEvent a( QEvent::AccelAvailable, codeIntern,
				 asciiIntern, stateIntern, textIntern, FALSE,
				 QMAX(countIntern, int(textIntern.length())) );
		    a.ignore();
		    QApplication::sendSpontaneousEvent( topLevelWidget(), &a );
		    if ( a.isAccepted() ) {
			XPutBackEvent(dpy, &evRelease);
			XPutBackEvent(dpy, &evPress);
			break;
		    }
		}
		text += textIntern;
		count += countIntern;
	    } else {
		XPutBackEvent(dpy, &evRelease);
		XPutBackEvent(dpy, &evPress);
		break;
	    }
	}
    }

    if ( save != 0 )
	qt_mode_switch_remove_mask = save;

    // was this the last auto-repeater?
    static uint curr_autorep = 0;
    if ( event->type == XKeyPress ) {
	if ( curr_autorep == event->xkey.keycode ) {
	    autor = TRUE;
	    curr_autorep = 0;
	}
    } else {
	// look ahead for auto-repeat
	XEvent nextpress;
	if ( XCheckTypedWindowEvent(dpy,event->xkey.window,
				    XKeyPress,&nextpress) ) {
	    // check for event pairs with delta t <= 10 msec.
	    autor = ( nextpress.xkey.keycode == event->xkey.keycode &&
		      (nextpress.xkey.time - event->xkey.time) <= 10 );

	    // Put it back... we COULD send the event now and not need
	    // the static curr_autorep variable.
	    XPutBackEvent(dpy,&nextpress);
	}
	curr_autorep = autor ? event->xkey.keycode : 0;
    }

    // autorepeat compression makes sense for all widgets (Windows
    // does it automatically .... )
    if ( event->type == XKeyPress && text.length() <= 1 ) {
	XEvent evPress = *event;
	XEvent evRelease;
	for (;;) {
	    if (!XCheckTypedWindowEvent(dpy,event->xkey.window,XKeyRelease,
					&evRelease) )
		break;
	    if (evRelease.xkey.keycode != event->xkey.keycode ) {
		XPutBackEvent(dpy, &evRelease);
		break;
	    }
	    if (!XCheckTypedWindowEvent(dpy,event->xkey.window,XKeyPress,
					&evPress)) {
		XPutBackEvent(dpy, &evRelease);
		break;
	    }
	    if ( evPress.xkey.keycode != event->xkey.keycode ||
		 evRelease.xkey.keycode != event->xkey.keycode ||
		 (evPress.xkey.time - evRelease.xkey.time) > 10){
		XPutBackEvent(dpy, &evRelease);
		XPutBackEvent(dpy, &evPress);
		break;
	    }
	    count++;
	    if (!text.isEmpty())
		text += text[0];
	}
    }

    if (code == 0 && ascii == '\n') {
	code = Key_Return;
	ascii = '\r';
	text = "\r";
    }

    // try the menukey first
    if ( type == QEvent::KeyPress && code == Qt::Key_Menu ) {
	QContextMenuEvent e( QContextMenuEvent::Keyboard, QPoint( 5, 5 ), mapToGlobal( QPoint( 5, 5 ) ), 0 );
	QApplication::sendSpontaneousEvent( this, &e );
	if( e.isAccepted() )
	    return TRUE;
    }

    QKeyEvent e( type, code, ascii, state, text, autor,
		 QMAX(count, int(text.length())) );

#ifndef QT_NO_XIM
    if (qt_xim_style & XIMPreeditCallbacks) {
	QWidget *tlw = topLevelWidget();
	QInputContext *qic = (QInputContext *) tlw->topData()->xic;

	if (qic && qic->composing && ! qic->lastcompose.isNull() &&
	    qic->lastcompose == text) {
	    // keyevent with same text as last compose, skip it
	    return TRUE;
	}
    }
#endif // QT_NO_XIM
    return QApplication::sendSpontaneousEvent( this, &e );
}


//
// Paint event translation
//
// When receiving many expose events, we compress them (union of all expose
// rectangles) into one event which is sent to the widget.

struct PaintEventInfo {
    Window window;
};

#if defined(Q_C_CALLBACKS)
extern "C" {
#endif

static Bool isPaintOrScrollDoneEvent( Display *, XEvent *ev, XPointer a )
{
    PaintEventInfo *info = (PaintEventInfo *)a;
    if ( ev->type == Expose || ev->type == GraphicsExpose
      ||    ev->type == ClientMessage
	 && ev->xclient.message_type == qt_qt_scrolldone )
    {
	if ( ev->xexpose.window == info->window )
	    return True;
    }
    return False;
}

#if defined(Q_C_CALLBACKS)
}
#endif


// declared above: static QPtrList<QScrollInProgress> *sip_list = 0;

void qt_insert_sip( QWidget* scrolled_widget, int dx, int dy )
{
    if ( !sip_list ) {
	sip_list = new QPtrList<QScrollInProgress>;
	sip_list->setAutoDelete( TRUE );
    }

    QScrollInProgress* sip = new QScrollInProgress( scrolled_widget, dx, dy );
    sip_list->append( sip );

    XClientMessageEvent client_message;
    client_message.type = ClientMessage;
    client_message.window = scrolled_widget->winId();
    client_message.format = 32;
    client_message.message_type = qt_qt_scrolldone;
    client_message.data.l[0] = sip->id;

    XSendEvent( appDpy, scrolled_widget->winId(), False, NoEventMask,
	(XEvent*)&client_message );
}

int qt_sip_count( QWidget* scrolled_widget )
{
    if ( !sip_list )
	return 0;

    int sips=0;

    for (QScrollInProgress* sip = sip_list->first();
	sip; sip=sip_list->next())
    {
	if ( sip->scrolled_widget == scrolled_widget )
	    sips++;
    }

    return sips;
}

static
bool translateBySips( QWidget* that, QRect& paintRect )
{
    if ( sip_list ) {
	int dx=0, dy=0;
	int sips=0;
	for (QScrollInProgress* sip = sip_list->first();
	    sip; sip=sip_list->next())
	{
	    if ( sip->scrolled_widget == that ) {
		if ( sips ) {
		    dx += sip->dx;
		    dy += sip->dy;
		}
		sips++;
	    }
	}
	if ( sips > 1 ) {
	    paintRect.moveBy( dx, dy );
	    return TRUE;
	}
    }
    return FALSE;
}

bool QETWidget::translatePaintEvent( const XEvent *event )
{
    setWState( WState_Exposed );
    QRect  paintRect( event->xexpose.x,	   event->xexpose.y,
		      event->xexpose.width, event->xexpose.height );
    bool   merging_okay = !testWFlags(WPaintClever);
    XEvent xevent;
    PaintEventInfo info;
    info.window = winId();
    bool should_clip = translateBySips( this, paintRect );

    QRegion paintRegion( paintRect );

    if ( merging_okay ) {
	// WARNING: this is O(number_of_events * number_of_matching_events)
	while ( XCheckIfEvent(x11Display(),&xevent,isPaintOrScrollDoneEvent,
			      (XPointer)&info) &&
		!qt_x11EventFilter(&xevent)  &&
		!x11Event( &xevent ) ) // send event through filter
	{
	    if ( xevent.type == Expose || xevent.type == GraphicsExpose ) {
		QRect exposure(xevent.xexpose.x,
			       xevent.xexpose.y,
			       xevent.xexpose.width,
			       xevent.xexpose.height);
		if ( translateBySips( this, exposure ) )
		    should_clip = TRUE;
		paintRegion = paintRegion.unite( exposure );
	    } else {
		translateScrollDoneEvent( &xevent );
	    }
	}
    }

    if ( should_clip ) {
	paintRegion = paintRegion.intersect( rect() );
	if ( paintRegion.isEmpty() )
	    return TRUE;
    }

    QPaintEvent e( paintRegion );
    setWState( WState_InPaintEvent );
    if ( !isTopLevel() && backgroundOrigin() != WidgetOrigin )
	erase( paintRegion );
    qt_set_paintevent_clipping( this, paintRegion );
    QApplication::sendSpontaneousEvent( this, &e );
    qt_clear_paintevent_clipping();
    clearWState( WState_InPaintEvent );
    return TRUE;
}

//
// Scroll-done event translation.
//

bool QETWidget::translateScrollDoneEvent( const XEvent *event )
{
    if ( !sip_list ) return FALSE;

    long id = event->xclient.data.l[0];

    // Remove any scroll-in-progress record for the given id.
    for (QScrollInProgress* sip = sip_list->first(); sip; sip=sip_list->next()) {
	if ( sip->id == id ) {
	    sip_list->remove( sip_list->current() );
	    return TRUE;
	}
    }

    return FALSE;
}


//
// ConfigureNotify (window move and resize) event translation

bool QETWidget::translateConfigEvent( const XEvent *event )
{
    // config pending is only set on resize, see qwidget_x11.cpp, internalSetGeometry()
    bool was_resize = testWState( WState_ConfigPending );

    clearWState(WState_ConfigPending);

    if ( isTopLevel() ) {
	QPoint newCPos( geometry().topLeft() );
	QSize  newSize( event->xconfigure.width, event->xconfigure.height );

	bool trust = (topData()->parentWinId == None ||
		      topData()->parentWinId == QPaintDevice::x11AppRootWindow());

	if (event->xconfigure.send_event || trust ) {
	    // if a ConfigureNotify comes from a real sendevent request, we can
	    // trust its values.
	    newCPos.rx() = event->xconfigure.x + event->xconfigure.border_width;
	    newCPos.ry() = event->xconfigure.y + event->xconfigure.border_width;
	}

	if ( isVisible() )
	    QApplication::syncX();

	// ConfigureNotify compression for faster opaque resizing
	XEvent otherEvent;
	while ( XCheckTypedWindowEvent( x11Display(), winId(), ConfigureNotify,
					&otherEvent ) ) {
	    if ( qt_x11EventFilter( &otherEvent ) )
		continue;

	    if (x11Event( &otherEvent ) )
		continue;

	    if ( otherEvent.xconfigure.event != otherEvent.xconfigure.window )
		continue;

	    newSize.setWidth( otherEvent.xconfigure.width );
	    newSize.setHeight( otherEvent.xconfigure.height );

	    if ( otherEvent.xconfigure.send_event || trust ) {
		newCPos.rx() = otherEvent.xconfigure.x +
			       otherEvent.xconfigure.border_width;
		newCPos.ry() = otherEvent.xconfigure.y +
			       otherEvent.xconfigure.border_width;
	    }
	}

	QRect cr ( geometry() );
	if ( newSize != cr.size() ) { // size changed
	    was_resize = TRUE;
	    QSize oldSize = size();
	    cr.setSize( newSize );
	    crect = cr;

	    if ( isVisible() ) {
		QResizeEvent e( newSize, oldSize );
		QApplication::sendSpontaneousEvent( this, &e );
	    } else {
		QResizeEvent * e = new QResizeEvent( newSize, oldSize );
		QApplication::postEvent( this, e );
	    }
	}

	if ( newCPos != cr.topLeft() ) { // compare with cpos (exluding frame)
	    QPoint oldPos = geometry().topLeft();
	    cr.moveTopLeft( newCPos );
	    crect = cr;
	    if ( isVisible() ) {
		QMoveEvent e( newCPos, oldPos ); // pos (including frame), not cpos
		QApplication::sendSpontaneousEvent( this, &e );
	    } else {
		QMoveEvent * e = new QMoveEvent( newCPos, oldPos );
		QApplication::postEvent( this, e );
	    }
	}
    } else {
	XEvent xevent;
	while ( XCheckTypedWindowEvent(x11Display(),winId(), ConfigureNotify,&xevent) &&
		!qt_x11EventFilter(&xevent)  &&
		!x11Event( &xevent ) ) // send event through filter
	    ;
    }

    bool transbg = backgroundOrigin() != WidgetOrigin;
    // we ignore NorthWestGravity at the moment for reversed layout
    if ( transbg ||
	 (!testWFlags( WStaticContents ) &&
	  testWState( WState_Exposed ) && was_resize ) ||
	 QApplication::reverseLayout() ) {
	// remove unnecessary paint events from the queue
	XEvent xevent;
	while ( XCheckTypedWindowEvent( x11Display(), winId(), Expose, &xevent ) &&
		! qt_x11EventFilter( &xevent )  &&
		! x11Event( &xevent ) ) // send event through filter
	    ;
	repaint( visibleRect(), !testWFlags(WResizeNoErase) || transbg );
    }

    return TRUE;
}


//
// Close window event translation.
//
bool QETWidget::translateCloseEvent( const XEvent * )
{
    return close(FALSE);
}


/*!
    Sets the text cursor's flash (blink) time to \a msecs
    milliseconds. The flash time is the time required to display,
    invert and restore the caret display. Usually the text cursor is
    displayed for \a msecs/2 milliseconds, then hidden for \a msecs/2
    milliseconds, but this may vary.

    Note that on Microsoft Windows, calling this function sets the
    cursor flash time for all windows.

    \sa cursorFlashTime()
*/
void  QApplication::setCursorFlashTime( int msecs )
{
    cursor_flash_time = msecs;
}


/*!
    Returns the text cursor's flash (blink) time in milliseconds. The
    flash time is the time required to display, invert and restore the
    caret display.

    The default value on X11 is 1000 milliseconds. On Windows, the
    control panel value is used.

    Widgets should not cache this value since it may be changed at any
    time by the user changing the global desktop settings.

    \sa setCursorFlashTime()
*/
int QApplication::cursorFlashTime()
{
    return cursor_flash_time;
}

/*!
    Sets the time limit that distinguishes a double click from two
    consecutive mouse clicks to \a ms milliseconds.

    Note that on Microsoft Windows, calling this function sets the
    double click interval for all windows.

    \sa doubleClickInterval()
*/

void QApplication::setDoubleClickInterval( int ms )
{
    mouse_double_click_time = ms;
}


/*!
    Returns the maximum duration for a double click.

    The default value on X11 is 400 milliseconds. On Windows, the
    control panel value is used.

    \sa setDoubleClickInterval()
*/

int QApplication::doubleClickInterval()
{
    return mouse_double_click_time;
}


/*!
    Sets the number of lines to scroll when the mouse wheel is rotated
    to \a n.

    If this number exceeds the number of visible lines in a certain
    widget, the widget should interpret the scroll operation as a
    single page up / page down operation instead.

    \sa wheelScrollLines()
*/
void QApplication::setWheelScrollLines( int n )
{
    wheel_scroll_lines = n;
}

/*!
    Returns the number of lines to scroll when the mouse wheel is
    rotated.

    \sa setWheelScrollLines()
*/
int QApplication::wheelScrollLines()
{
    return wheel_scroll_lines;
}

/*!
    Enables the UI effect \a effect if \a enable is TRUE, otherwise
    the effect will not be used.

    \sa isEffectEnabled(), Qt::UIEffect, setDesktopSettingsAware()
*/
void QApplication::setEffectEnabled( Qt::UIEffect effect, bool enable )
{
    switch (effect) {
    case UI_AnimateMenu:
	animate_menu = enable;
	break;
    case UI_FadeMenu:
	if ( enable )
	    animate_menu = TRUE;
	fade_menu = enable;
	break;
    case UI_AnimateCombo:
	animate_combo = enable;
	break;
    case UI_AnimateTooltip:
	animate_tooltip = enable;
	break;
    case UI_FadeTooltip:
	if ( enable )
	    animate_tooltip = TRUE;
	fade_tooltip = enable;
	break;
    default:
	animate_ui = enable;
	break;
    }
}

/*!
    Returns TRUE if \a effect is enabled; otherwise returns FALSE.

    By default, Qt will try to use the desktop settings. Call
    setDesktopSettingsAware(FALSE) to prevent this.

    \sa setEffectEnabled(), Qt::UIEffect
*/
bool QApplication::isEffectEnabled( Qt::UIEffect effect )
{
    if ( !animate_ui )
	return FALSE;

    switch( effect ) {
    case UI_AnimateMenu:
	return animate_menu;
    case UI_FadeMenu:
	    if ( QColor::numBitPlanes() < 16 )
		return FALSE;
	    return fade_menu;
    case UI_AnimateCombo:
	return animate_combo;
    case UI_AnimateTooltip:
	return animate_tooltip;
    case UI_FadeTooltip:
	if ( QColor::numBitPlanes() < 16 )
	    return FALSE;
	return fade_tooltip;
    default:
	return animate_ui;
    }
}

/*****************************************************************************
  Session management support
 *****************************************************************************/

#ifndef QT_NO_SM_SUPPORT

#include <X11/SM/SMlib.h>

class QSessionManagerData
{
public:
    QSessionManagerData( QSessionManager* mgr, QString& id, QString& key )
	: sm( mgr ), sessionId( id ), sessionKey( key ) {}
    QSessionManager* sm;
    QStringList restartCommand;
    QStringList discardCommand;
    QString& sessionId;
    QString& sessionKey;
    QSessionManager::RestartHint restartHint;
};

class QSmSocketReceiver : public QObject
{
    Q_OBJECT
public:
    QSmSocketReceiver( int socket )
	: QObject(0,0)
	{
	    QSocketNotifier* sn = new QSocketNotifier( socket, QSocketNotifier::Read, this );
	    connect( sn, SIGNAL( activated(int) ), this, SLOT( socketActivated(int) ) );
	}

public slots:
     void socketActivated(int);
};


static SmcConn smcConnection = 0;
static bool sm_interactionActive;
static bool sm_smActive;
static int sm_interactStyle;
static int sm_saveType;
static bool sm_cancel;
// static bool sm_waitingForPhase2;  ### never used?!?
static bool sm_waitingForInteraction;
static bool sm_isshutdown;
// static bool sm_shouldbefast;  ### never used?!?
static bool sm_phase2;
static bool sm_in_phase2;

static QSmSocketReceiver* sm_receiver = 0;

static void resetSmState();
static void sm_setProperty( const char* name, const char* type,
			    int num_vals, SmPropValue* vals);
static void sm_saveYourselfCallback( SmcConn smcConn, SmPointer clientData,
				  int saveType, Bool shutdown , int interactStyle, Bool fast);
static void sm_saveYourselfPhase2Callback( SmcConn smcConn, SmPointer clientData ) ;
static void sm_dieCallback( SmcConn smcConn, SmPointer clientData ) ;
static void sm_shutdownCancelledCallback( SmcConn smcConn, SmPointer clientData );
static void sm_saveCompleteCallback( SmcConn smcConn, SmPointer clientData );
static void sm_interactCallback( SmcConn smcConn, SmPointer clientData );
static void sm_performSaveYourself( QSessionManagerData* );

static void resetSmState()
{
//    sm_waitingForPhase2 = FALSE; ### never used?!?
    sm_waitingForInteraction = FALSE;
    sm_interactionActive = FALSE;
    sm_interactStyle = SmInteractStyleNone;
    sm_smActive = FALSE;
    sm_blockUserInput = FALSE;
    sm_isshutdown = FALSE;
//    sm_shouldbefast = FALSE; ### never used?!?
    sm_phase2 = FALSE;
    sm_in_phase2 = FALSE;
}


// theoretically it's possible to set several properties at once. For
// simplicity, however, we do just one property at a time
static void sm_setProperty( const char* name, const char* type,
			    int num_vals, SmPropValue* vals)
{
    if (num_vals ) {
      SmProp prop;
      prop.name = (char*)name;
      prop.type = (char*)type;
      prop.num_vals = num_vals;
      prop.vals = vals;

      SmProp* props[1];
      props[0] = &prop;
      SmcSetProperties( smcConnection, 1, props );
    }
    else {
      char* names[1];
      names[0] = (char*) name;
      SmcDeleteProperties( smcConnection, 1, names );
    }
}

static void sm_setProperty( const QString& name, const QString& value)
{
    SmPropValue prop;
    prop.length = value.length();
    prop.value = (SmPointer) value.latin1();
    sm_setProperty( name.latin1(), SmARRAY8, 1, &prop );
}

static void sm_setProperty( const QString& name, const QStringList& value)
{
    SmPropValue *prop = new SmPropValue[ value.count() ];
    int count = 0;
    for ( QStringList::ConstIterator it = value.begin(); it != value.end(); ++it ) {
      prop[ count ].length = (*it).length();
      prop[ count ].value = (char*)(*it).latin1();
      ++count;
    }
    sm_setProperty( name.latin1(), SmLISTofARRAY8, count, prop );
    delete [] prop;
}


// workaround for broken libsm, see below
struct QT_smcConn {
    unsigned int save_yourself_in_progress : 1;
    unsigned int shutdown_in_progress : 1;
};

static void sm_saveYourselfCallback( SmcConn smcConn, SmPointer clientData,
				  int saveType, Bool shutdown , int interactStyle, Bool /*fast*/)
{
    if (smcConn != smcConnection )
	return;
    sm_cancel = FALSE;
    sm_smActive = TRUE;
    sm_isshutdown = shutdown;
    sm_saveType = saveType;
    sm_interactStyle = interactStyle;
//    sm_shouldbefast = fast; ### never used?!?

    // ugly workaround for broken libSM. libSM should do that _before_
    // actually invoking the callback in sm_process.c
    ( (QT_smcConn*)smcConn )->save_yourself_in_progress = TRUE;
    if ( sm_isshutdown )
	( (QT_smcConn*)smcConn )->shutdown_in_progress = TRUE;

    sm_performSaveYourself( (QSessionManagerData*) clientData );
    if ( !sm_isshutdown ) // we cannot expect a confirmation message in that case
	resetSmState();
}

static void sm_performSaveYourself( QSessionManagerData* smd )
{
    if ( sm_isshutdown )
	sm_blockUserInput = TRUE;

    QSessionManager* sm = smd->sm;

    // generate a new session key
    timeval tv;
    gettimeofday( &tv, 0 );
    smd->sessionKey  = QString::number( tv.tv_sec ) + "_" + QString::number(tv.tv_usec);

    // tell the session manager about our program in best POSIX style
    sm_setProperty( SmProgram, QString( qApp->argv()[0] ) );
    // tell the session manager about our user as well.
    struct passwd* entry = getpwuid( geteuid() );
    if ( entry )
	sm_setProperty( SmUserID, QString::fromLatin1( entry->pw_name ) );

    // generate a restart and discard command that makes sense
    QStringList restart;
    restart  << qApp->argv()[0] << "-session" << smd->sessionId + "_" + smd->sessionKey;
    sm->setRestartCommand( restart );
    QStringList discard;
    sm->setDiscardCommand( discard );

    switch ( sm_saveType ) {
    case SmSaveBoth:
	qApp->commitData( *sm );
	if ( sm_isshutdown && sm_cancel)
	    break; // we cancelled the shutdown, no need to save state
    // fall through
    case SmSaveLocal:
	qApp->saveState( *sm );
	break;
    case SmSaveGlobal:
	qApp->commitData( *sm );
	break;
    default:
	break;
    }

    if ( sm_phase2 && !sm_in_phase2 ) {
	SmcRequestSaveYourselfPhase2( smcConnection, sm_saveYourselfPhase2Callback, (SmPointer*) sm );
	sm_blockUserInput = FALSE;
    }
    else {
	// close eventual interaction monitors and cancel the
	// shutdown, if required. Note that we can only cancel when
	// performing a shutdown, it does not work for checkpoints
	if ( sm_interactionActive ) {
	    SmcInteractDone( smcConnection, sm_isshutdown && sm_cancel);
	    sm_interactionActive = FALSE;
	}
	else if ( sm_cancel && sm_isshutdown ) {
	    if ( sm->allowsErrorInteraction() ) {
		SmcInteractDone( smcConnection, True );
		sm_interactionActive = FALSE;
	    }
	}

	// set restart and discard command in session manager
	sm_setProperty( SmRestartCommand, sm->restartCommand() );
	sm_setProperty( SmDiscardCommand, sm->discardCommand() );

	// set the restart hint
	SmPropValue prop;
	prop.length = sizeof( int );
	int value = sm->restartHint();
	prop.value = (SmPointer) &value;
	sm_setProperty( SmRestartStyleHint, SmCARD8, 1, &prop );

	// we are done
	SmcSaveYourselfDone( smcConnection, !sm_cancel );
    }
}

static void sm_dieCallback( SmcConn smcConn, SmPointer /* clientData  */)
{
    if (smcConn != smcConnection )
	return;
    resetSmState();
    qApp->quit();
}

static void sm_shutdownCancelledCallback( SmcConn smcConn, SmPointer /* clientData */)
{
    if (smcConn != smcConnection )
	return;
    if ( sm_waitingForInteraction )
	qApp->exit_loop();
    resetSmState();
}

static void sm_saveCompleteCallback( SmcConn smcConn, SmPointer /*clientData */)
{
    if (smcConn != smcConnection )
	return;
    resetSmState();
}

static void sm_interactCallback( SmcConn smcConn, SmPointer /* clientData */ )
{
    if (smcConn != smcConnection )
	return;
    if ( sm_waitingForInteraction )
	qApp->exit_loop();
}

static void sm_saveYourselfPhase2Callback( SmcConn smcConn, SmPointer clientData )
{
    if (smcConn != smcConnection )
	return;
    sm_in_phase2 = TRUE;
    sm_performSaveYourself( (QSessionManagerData*) clientData );
}


void QSmSocketReceiver::socketActivated(int)
{
    IceProcessMessages( SmcGetIceConnection( smcConnection ), 0, 0);
}


#undef Bool
#include "qapplication_x11.moc"

QSessionManager::QSessionManager( QApplication * app, QString &id, QString& key )
    : QObject( app, "session manager" )
{
    d = new QSessionManagerData( this, id, key );
    d->restartHint = RestartIfRunning;

    resetSmState();
    char cerror[256];
    char* myId = 0;
    char* prevId = (char*)id.latin1(); // we know what we are doing

    SmcCallbacks cb;
    cb.save_yourself.callback = sm_saveYourselfCallback;
    cb.save_yourself.client_data = (SmPointer) d;
    cb.die.callback = sm_dieCallback;
    cb.die.client_data = (SmPointer) d;
    cb.save_complete.callback = sm_saveCompleteCallback;
    cb.save_complete.client_data = (SmPointer) d;
    cb.shutdown_cancelled.callback = sm_shutdownCancelledCallback;
    cb.shutdown_cancelled.client_data = (SmPointer) d;

    // avoid showing a warning message below
    const char* session_manager = getenv("SESSION_MANAGER");
    if ( !session_manager || !session_manager[0] )
	return;

    smcConnection = SmcOpenConnection( 0, 0, 1, 0,
				       SmcSaveYourselfProcMask |
				       SmcDieProcMask |
				       SmcSaveCompleteProcMask |
				       SmcShutdownCancelledProcMask,
				       &cb,
				       prevId,
				       &myId,
				       256, cerror );

    id = QString::fromLatin1( myId );
    ::free( myId ); // it was allocated by C

    QString error = cerror;
    if (!smcConnection ) {
	qWarning("Session management error: %s", error.latin1() );
    }
    else {
	sm_receiver = new QSmSocketReceiver(  IceConnectionNumber( SmcGetIceConnection( smcConnection ) ) );
    }
}

QSessionManager::~QSessionManager()
{
    if ( smcConnection )
      SmcCloseConnection( smcConnection, 0, 0 );
    smcConnection = 0;
    delete sm_receiver;
    delete d;
}

QString QSessionManager::sessionId() const
{
    return d->sessionId;
}

QString QSessionManager::sessionKey() const
{
    return d->sessionKey;
}


void* QSessionManager::handle() const
{
    return (void*) smcConnection;
}


bool QSessionManager::allowsInteraction()
{
    if ( sm_interactionActive )
	return TRUE;

    if ( sm_waitingForInteraction )
	return FALSE;

    if ( sm_interactStyle == SmInteractStyleAny ) {
	sm_waitingForInteraction =  SmcInteractRequest( smcConnection, SmDialogNormal,
							sm_interactCallback, (SmPointer*) this );
    }
    if ( sm_waitingForInteraction ) {
	qApp->enter_loop();
	sm_waitingForInteraction = FALSE;
	if ( sm_smActive ) { // not cancelled
	    sm_interactionActive = TRUE;
	    sm_blockUserInput = FALSE;
	    return TRUE;
	}
    }
    return FALSE;
}

bool QSessionManager::allowsErrorInteraction()
{
    if ( sm_interactionActive )
	return TRUE;

    if ( sm_waitingForInteraction )
	return FALSE;

    if ( sm_interactStyle == SmInteractStyleAny || sm_interactStyle == SmInteractStyleErrors ) {
	sm_waitingForInteraction =  SmcInteractRequest( smcConnection, SmDialogError,
							sm_interactCallback, (SmPointer*) this );
    }
    if ( sm_waitingForInteraction ) {
	qApp->enter_loop();
	sm_waitingForInteraction = FALSE;
	if ( sm_smActive ) { // not cancelled
	    sm_interactionActive = TRUE;
	    sm_blockUserInput = FALSE;
	    return TRUE;
	}
    }
    return FALSE;
}

void QSessionManager::release()
{
    if ( sm_interactionActive ) {
	SmcInteractDone( smcConnection, False );
	sm_interactionActive = FALSE;
	if ( sm_smActive && sm_isshutdown )
	    sm_blockUserInput = TRUE;
    }
}

void QSessionManager::cancel()
{
    sm_cancel = TRUE;
}

void QSessionManager::setRestartHint( QSessionManager::RestartHint hint)
{
    d->restartHint = hint;
}

QSessionManager::RestartHint QSessionManager::restartHint() const
{
    return d->restartHint;
}

void QSessionManager::setRestartCommand( const QStringList& command)
{
    d->restartCommand = command;
}

QStringList QSessionManager::restartCommand() const
{
    return d->restartCommand;
}

void QSessionManager::setDiscardCommand( const QStringList& command)
{
    d->discardCommand = command;
}

QStringList QSessionManager::discardCommand() const
{
    return d->discardCommand;
}

void QSessionManager::setManagerProperty( const QString& name, const QString& value)
{
    SmPropValue prop;
    prop.length = value.length();
    prop.value = (SmPointer) value.utf8().data();
    sm_setProperty( name.latin1(), SmARRAY8, 1, &prop );
}

void QSessionManager::setManagerProperty( const QString& name, const QStringList& value)
{
    SmPropValue *prop = new SmPropValue[ value.count() ];
    int count = 0;
    for ( QStringList::ConstIterator it = value.begin(); it != value.end(); ++it ) {
      prop[ count ].length = (*it).length();
      prop[ count ].value = (char*)(*it).utf8().data();
      ++count;
    }
    sm_setProperty( name.latin1(), SmLISTofARRAY8, count, prop );
    delete [] prop;
}

bool QSessionManager::isPhase2() const
{
    return sm_in_phase2;
}

void QSessionManager::requestPhase2()
{
    sm_phase2 = TRUE;
}


#endif // QT_NO_SM_SUPPORT
