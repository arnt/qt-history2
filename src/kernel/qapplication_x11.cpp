/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qapplication_x11.cpp#553 $
**
** Implementation of X11 startup routines and event handling
**
** Created : 931029
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#define select		_qt_hide_select
#define gettimeofday	_qt_hide_gettimeofday

#include "qglobal.h"
#if defined(_OS_WIN32_)
#undef select
#include <windows.h>
#define HANDLE QT_HANDLE
#endif
#include "qapplication.h"
#include "qwidget.h"
#include "qobjectlist.h"
#include "qobjectdict.h"
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
#include <stdlib.h>
#include <ctype.h>
#include <locale.h>
#include <errno.h>
#define	 GC GC_QQQ

#if defined(_OS_LINUX_) && defined(DEBUG)
#include "qfile.h"
#include <unistd.h>
#endif

#if defined(_OS_WIN32_)
#undef gettimeofday
#endif

#include "qt_x11.h"

#ifndef X11R4
#include <X11/Xlocale.h>
#endif


#if defined(_OS_IRIX_)
#include <bstring.h>
#endif

#if defined(_OS_AIX_) && defined(_CC_GNU_)
#include <sys/time.h>
#include <sys/select.h>
#include <unistd.h>
#endif

#if defined(_OS_QNX_)
#include <sys/select.h>
#endif

#if defined(_CC_MSVC_)
#pragma warning(disable: 4018)
#undef open
#undef close
#endif

#if defined(_OS_WIN32_) && defined(gettimeofday)
#undef gettimeofday
#include <sys/timeb.h>
inline void gettimeofday( struct timeval *t, struct timezone * )
{
    struct _timeb tb;
    _ftime( &tb );
    t->tv_sec  = tb.time;
    t->tv_usec = tb.millitm * 1000;
}
#else
#undef gettimeofday
extern "C" int gettimeofday( struct timeval *, struct timezone * );
#endif // _OS_WIN32 etc.
#if !defined(_OS_WIN32_)
#undef select
extern "C" int select( int, void *, void *, void *, struct timeval * );
#endif

//#define X_NOT_BROKEN
#ifdef X_NOT_BROKEN
// Some X libraries are built with setlocale #defined to _Xsetlocale,
// even though library users are then built WITHOUT such a definition.
// This creates a problem - Qt might setlocale() one value, but then
// X looks and doesn't see the value Qt set.  The solution here is to
// implement _Xsetlocale just in case X calls it - redirecting it to
// the real libC version.
//
#ifndef setlocale
extern "C" char *_Xsetlocale(int category, const char *locale);
char *_Xsetlocale(int category, const char *locale)
{
    //qDebug("_Xsetlocale(%d,%s),category,locale");
    return setlocale(category,locale);
}
#endif
#endif

// resolve the conflict between X11's FocusIn and QEvent::FocusIn
const int XFocusOut = FocusOut;
const int XFocusIn = FocusIn;
#undef FocusOut
#undef FocusIn

const int XKeyPress = KeyPress;
const int XKeyRelease = KeyRelease;
#undef KeyPress
#undef KeyRelease


/*****************************************************************************
  Internal variables and functions
 *****************************************************************************/

static char    *appName;			// application name
static char    *appFont		= 0;		// application font
static char    *appBGCol	= 0;		// application bg color
static char    *appFGCol	= 0;		// application fg color
static char    *appBTNCol	= 0;		// application btn color
static char    *mwGeometry	= 0;		// main widget geometry
static char    *mwTitle		= 0;		// main widget title
static bool	mwIconic	= FALSE;	// main widget iconified
static Display *appDpy		= 0;		// X11 application display
static char    *appDpyName	= 0;		// X11 display name
static bool     appForeignDpy	= FALSE;        // we didn't create display
static bool	appSync		= FALSE;	// X11 synchronization
#if defined(DEBUG)
static bool	appNoGrab	= FALSE;	// X11 grabbing enabled
static bool	appDoGrab	= FALSE;	// X11 grabbing override (gdb)
#endif
static int	appScreen;			// X11 screen number
static Window	appRootWin;			// X11 root window
static bool	app_save_rootinfo = FALSE;	// save root info

static bool	app_do_modal	= FALSE;	// modal mode
static int	app_Xfd;			// X network socket
static fd_set	app_readfds;			// fd set for reading
static fd_set	app_writefds;			// fd set for writing
static fd_set	app_exceptfds;			// fd set for exceptions

static GC	app_gc_ro	= 0;		// read-only GC
static GC	app_gc_tmp	= 0;		// temporary GC
static GC	app_gc_ro_m	= 0;		// read-only GC (monochrome)
static GC	app_gc_tmp_m	= 0;		// temporary GC (monochrome)
static Atom	qt_wm_protocols;		// window manager protocols
Atom		qt_wm_delete_window;		// delete window protocol
static Atom	qt_qt_scrolldone;		// scroll synchronization

Atom	qt_embedded_window;
Atom	qt_embedded_window_take_focus;
Atom	qt_embedded_window_focus_in;
Atom	qt_embedded_window_focus_out;
Atom	qt_embedded_window_tab_focus;
Atom	qt_embedded_window_support_tab_focus;
Atom	qt_wheel_event;
Atom	qt_unicode_key_press;
Atom	qt_unicode_key_release;

static Atom	qt_xsetroot_id;
Atom		qt_selection_property;
Atom		qt_wm_state;
static Atom 	qt_desktop_properties;   	// Qt desktop properties
static Atom 	qt_resource_manager;		// X11 Resource manager
Atom 		qt_sizegrip;			// sizegrip
Atom 		qt_wm_client_leader;
Atom 		qt_window_role;
Atom 		qt_sm_client_id;

static Window	mouseActWindow	     = 0;	// window where mouse is
static int	mouseButtonPressed   = 0;	// last mouse button pressed
static int	mouseButtonState     = 0;	// mouse button state
static Time	mouseButtonPressTime = 0;	// when was a button pressed
static short	mouseXPos, mouseYPos;		// mouse position in act window

static QWidgetList *modal_stack  = 0;		// stack of modal widgets
static QWidget     *popupButtonFocus = 0;
static QWidget     *popupOfPopupButtonFocus = 0;
static bool	    popupCloseDownMode = FALSE;
static bool	    popupGrabOk;

static bool sm_blockUserInput = FALSE;		// session management

typedef void  (*VFPTR)();
typedef QList<void> QVFuncList;
static QVFuncList *postRList = 0;		// list of post routines

static void	initTimers();
static void	cleanupTimers();
static timeval	watchtime;			// watch if time is turned back
timeval        *qt_wait_timer();
int	        qt_activate_timers();

#if !defined(NO_XIM)
XIM	qt_xim = 0;
XIMStyle qt_xim_style = 0;
static XIMStyle xim_preferred_style = XIMPreeditPosition | XIMStatusNothing;
#endif
static int composingKeycode=0;
static QTextCodec * input_mapper = 0;

QObject	       *qt_clipboard = 0;
Time		qt_x_clipboardtime = CurrentTime;

static void	qt_save_rootinfo();
static bool	qt_try_modal( QWidget *, XEvent * );
void		qt_reset_color_avail();		// defined in qcolor_x11.cpp

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
static QList<QScrollInProgress> *sip_list = 0;


// stuff in tq_xdnd.cpp
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
// client message atoms
extern Atom qt_xdnd_enter;
extern Atom qt_xdnd_position;
extern Atom qt_xdnd_status;
extern Atom qt_xdnd_leave;
extern Atom qt_xdnd_drop;
extern Atom qt_xdnd_finished;
// xdnd selection atom
extern Atom qt_xdnd_selection;


// Paint event clipping magic
extern void qt_set_paintevent_clipping( QPaintDevice* dev, const QRegion& region);
extern void qt_clear_paintevent_clipping();


// Palette handling
extern QPalette *qt_std_pal;
extern void qt_create_std_palette();

void qt_x11_intern_atom( const char *, Atom * );
void qt_xembed_tab_focus( QWidget*, bool next );

static QList<QWidget>* deferred_map_list = 0;
static void qt_deferred_map_cleanup()
{
    delete deferred_map_list;
    deferred_map_list = 0;
}
void qt_deferred_map_add( QWidget* w)
{
    if ( !deferred_map_list ) {
	deferred_map_list = new QList<QWidget>;
	qAddPostRoutine( qt_deferred_map_cleanup );
    }
    deferred_map_list->append( w );
};
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
    bool translateKeyEventInternal( const XEvent *, int& count, QString& text, int& state, char& ascii, int &code );
    bool translateKeyEvent( const XEvent *, bool grab );
    bool translatePaintEvent( const XEvent * );
    bool translateConfigEvent( const XEvent * );
    bool translateCloseEvent( const XEvent * );
    bool translateScrollDoneEvent( const XEvent * );
    bool translateWheelEvent( int global_x, int global_y, int delta, int state );
    void embeddedWindowTabFocus( bool );
};


static void close_xim()
{
#if !defined(NO_XIM)
    // Calling XCloseIM gives a Purify FMR error
    // XCloseIM( qt_xim );
    // We prefer a less serious memory leak
    qt_xim = 0;
#endif
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
    qFatal( "X Error: %s %d\n  Major opcode:  %d", errstr, err->error_code, err->request_code );
    return 0;
}


static int qt_xio_errhandler( Display * )
{
    qWarning( "%s: Fatal IO error: client killed", appName );
    exit( 1 );
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
  quickly.  If the application is running, the atom is created here.

  Neither argument may point to temporary variables.
 *****************************************************************************/

void qt_x11_intern_atom( const char *name, Atom *result)
{
    if ( !name || !result || *result )
	return;

    if ( create_atoms_now ) {
	*result = XInternAtom(appDpy, name, FALSE );
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
	XInternAtoms( appDpy, names, i, FALSE, res );
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
	    *result = XInternAtom(appDpy, name, FALSE );
	}
#endif
	delete atoms_to_be_created;
	atoms_to_be_created = 0;
	create_atoms_now = TRUE;
    }
}


// read the _QT_DESKTOP_PROPERTIES property and apply the settings to
// the application
static bool qt_set_desktop_properties()
{

    if ( !qt_std_pal )
	qt_create_std_palette();

    Atom type;
    int format;
    ulong  nitems, after = 1;
    long offset = 0;
    const char *data;

    int e = XGetWindowProperty( appDpy, appRootWin, qt_desktop_properties, 0, 1,
			     FALSE, AnyPropertyType, &type, &format, &nitems,
			     &after,  (unsigned char**)&data );
    if ( data )
	XFree(  (unsigned char*)data );
    if ( e != Success || !nitems )
	return FALSE;

    QBuffer  properties;
    properties.open( IO_WriteOnly );
    while (after > 0) {
	XGetWindowProperty( appDpy, appRootWin, qt_desktop_properties,
			    offset, 256, FALSE, AnyPropertyType,
			    &type, &format, &nitems, &after, (unsigned char**) &data );
	properties.writeBlock(data, nitems);
	offset += nitems;
	XFree(  (unsigned char*)data );
    }

    QDataStream d( properties.buffer(), IO_ReadOnly );

    QPalette pal;
    QFont font;
    d >> pal >> font;
    if ( pal != *qt_std_pal && pal != QApplication::palette() )
	QApplication::setPalette( pal, TRUE );
    *qt_std_pal = pal;
    font.setCharSet(QFont::charSetForLocale());
    if ( font != QApplication::font() ) {
	QApplication::setFont( font, TRUE );
    }
    return TRUE;
}


// set font, foreground and background from x11 resources. The
// arguments may override the resource settings.
static void qt_set_x11_resources( const char* font = 0, const char* fg = 0,
				  const char* bg = 0, const char* button = 0 )
{
    if ( !qt_std_pal )
	qt_create_std_palette();

    QCString resFont, resFG, resBG;

    if ( QApplication::desktopSettingsAware() && !qt_set_desktop_properties() ) {
	int format;
	ulong  nitems, after = 1;
	QCString res;
	long offset = 0;
	Atom type = None;

	while (after > 0) {
	    char *data;
	    XGetWindowProperty( appDpy, appRootWin, qt_resource_manager,
				offset, 8192, FALSE, AnyPropertyType,
				&type, &format, &nitems, &after,
				(unsigned char**) &data );
	    res += data;
	    offset += 8192;
	    XFree(data);
	}

	QCString item, key, value;
	int l = 0, r, i;

	while( (unsigned) l < res.length()) {
	    r = res.find( "\n", l );
	    if ( r < 0 )
		r = res.length();
	    while ( isspace(res[l]) )
		l++;
	    if ( res[l] == '*'
	      && (res[l+1] == 'f' || res[l+1] == 'b') )
	    {
		// OPTIMIZED, since we only want "*[fb].."

		item = res.mid( l, r - l ).simplifyWhiteSpace();
		i = item.find( ":" );
		key = item.left( i ).stripWhiteSpace();
		value = item.right( item.length() - i - 1 ).stripWhiteSpace();
		if ( !font && key == "*font")
		    resFont = value.copy();
		else if  ( !fg &&  key == "*foreground" )
		    resFG = value.copy();
		else if ( !bg && key == "*background")
		    resBG = value.copy();
		// NOTE: if you add more, change the [fb] stuff above
	    }
	    l = r + 1;
	}
    }

    if ( resFont.isEmpty() )
	resFont = font;
    if ( resFG.isEmpty() )
	resFG = fg;
    if ( resBG.isEmpty() )
	resBG = bg;

    if ( !resFont.isEmpty() ) {				// set application font
	QFont fnt;
	fnt.setRawName( resFont );

	// override requested charset, unless given on the command-line
	if ( !font )
	    fnt.setCharSet( QFont::charSetForLocale() );

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
	    bg = qt_std_pal->normal().background();
	if ( !resFG.isEmpty() )
	    fg = QColor(QString(resFG));
	else
	    fg = qt_std_pal->normal().foreground();
	if ( button )
	    btn = QColor( button );
	else if ( !resBG.isEmpty() )
	    btn = bg;
	else
	    btn = qt_std_pal->normal().button();

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
	}
	else {
	    cg.setColor( QColorGroup::HighlightedText, Qt::white );
	    cg.setColor( QColorGroup::Highlight, Qt::darkBlue );
	}
	QColor disabled( (fg.red()+btn.red())/2,
			 (fg.green()+btn.green())/2,
			 (fg.blue()+btn.blue())/2);
	QColorGroup dcg( disabled, btn, btn.light( 125 ), btn.dark(), btn.dark(150),
			 disabled, Qt::white, Qt::white, bg );
	QPalette pal( cg, dcg, cg );
	if ( pal != *qt_std_pal && pal != QApplication::palette() )
	    QApplication::setPalette( pal, TRUE );
	*qt_std_pal = pal;
    }
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

void qt_init_internal( int *argcptr, char **argv, Display *display )
{
    if ( display ) {
	// Qt part of other application

	appForeignDpy = TRUE;
	appName = "Qt-subapplication";
	appDpy  = display;
	app_Xfd = XConnectionNumber( appDpy );

    } else {
	// Qt controls everything (default)

	char *p;
	int argc = *argcptr;
	int j;

	// Install default error handlers

	XSetErrorHandler( qt_x_errhandler );
	XSetIOErrorHandler( qt_xio_errhandler );

	// Set application name

	p = strrchr( argv[0], '/' );
	appName = p ? p + 1 : argv[0];

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
#if !defined(NO_XIM)
	    } else if ( arg == "-inputstyle" ) {
		if ( ++i < argc ) {
		    QCString s = QCString(argv[i]).lower();
		    if ( s == "overthespot" ) {
			xim_preferred_style =
					     XIMPreeditPosition | XIMStatusNothing;
		    } else if ( s == "offthespot" ) {
			xim_preferred_style =
					     XIMPreeditArea | XIMStatusArea;
		    } else if ( s == "root" ) {
			xim_preferred_style =
					     XIMPreeditNothing | XIMStatusNothing;
		    }
		}
#endif
	    } else if ( arg == "-cmap" ) {    // xv uses this name
		qt_cmap_option = TRUE;
	    }
#if defined(DEBUG)
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

#if defined(DEBUG) && defined(_OS_LINUX_)
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

	if( QApplication::is_gui_used ) {
	    if ( ( appDpy = XOpenDisplay(appDpyName) ) == 0 ) {
		qWarning( "%s: cannot connect to X server %s", appName,
			  XDisplayName(appDpyName) );
		exit( 1 );
	    }
	    app_Xfd = XConnectionNumber( appDpy );	// set X network socket

	    if ( appSync )				// if "-sync" argument
		XSynchronize( appDpy, TRUE );
	}
    }
    // Common code, regardless of whether display is foreign.

    // Get X parameters

    if( QApplication::is_gui_used ) {
	appScreen  = DefaultScreen(appDpy);
	appRootWin = RootWindow(appDpy,appScreen);

	// Set X paintdevice parameters

	Visual *vis = DefaultVisual(appDpy,appScreen);
	QPaintDevice::x_appdisplay     = appDpy;
	QPaintDevice::x_appscreen      = appScreen;
	QPaintDevice::x_appdepth       = DefaultDepth(appDpy,appScreen);
	QPaintDevice::x_appcells       = DisplayCells(appDpy,appScreen);
	QPaintDevice::x_appvisual      = vis;
	QPaintDevice::x_appdefvisual   = TRUE;

	if ( qt_visual_option == TrueColor ||	// find custom visual
	     QApplication::colorSpec() == QApplication::ManyColor ) {
	    vis = find_truecolor_visual( appDpy, &QPaintDevice::x_appdepth,
					 &QPaintDevice::x_appcells );
	    QPaintDevice::x_appdefvisual =
	       (XVisualIDFromVisual(vis) ==
		XVisualIDFromVisual(DefaultVisual(appDpy,appScreen)));
	    QPaintDevice::x_appvisual = vis;
	}

	if ( vis->c_class == TrueColor ) {
	    QPaintDevice::x_appdefcolormap = QPaintDevice::x_appdefvisual;
	} else {
	    QPaintDevice::x_appdefcolormap = !qt_cmap_option;
	}
	if ( QPaintDevice::x_appdefcolormap ) {
	    QPaintDevice::x_appcolormap = DefaultColormap(appDpy,appScreen);
	} else {
	    QPaintDevice::x_appcolormap = XCreateColormap(appDpy, appRootWin,
							  vis, AllocNone);
	}

	// Support protocols

	qt_x11_intern_atom( "WM_PROTOCOLS", &qt_wm_protocols );
	qt_x11_intern_atom( "WM_DELETE_WINDOW", &qt_wm_delete_window );
	qt_x11_intern_atom( "_XSETROOT_ID", &qt_xsetroot_id );
	qt_x11_intern_atom( "_QT_SCROLL_DONE", &qt_qt_scrolldone );
	qt_x11_intern_atom( "_QT_SELECTION", &qt_selection_property );
	qt_x11_intern_atom( "WM_STATE", &qt_wm_state );
	qt_x11_intern_atom( "RESOURCE_MANAGER", &qt_resource_manager );
	qt_x11_intern_atom( "_QT_DESKTOP_PROPERTIES", &qt_desktop_properties );
	qt_x11_intern_atom( "_QT_SIZEGRIP", &qt_sizegrip );
	qt_x11_intern_atom( "WM_CLIENT_LEADER", &qt_wm_client_leader);
	qt_x11_intern_atom( "WINDOW_ROLE", &qt_window_role);
	qt_x11_intern_atom( "SM_CLIENT_ID", &qt_sm_client_id);

	qt_x11_intern_atom( "_QT_EMBEDDED_WINDOW", &qt_embedded_window );
	qt_x11_intern_atom( "_QT_EMBEDDED_WINDOW_TAKE_FOCUS",
			    &qt_embedded_window_take_focus );
	qt_x11_intern_atom( "_QT_EMBEDDED_WINDOW_FOCUS_IN",
			    &qt_embedded_window_focus_in );
	qt_x11_intern_atom( "_QT_EMBEDDED_WINDOW_FOCUS_OUT",
			    &qt_embedded_window_focus_out );
	qt_x11_intern_atom( "_QT_EMBEDDED_WINDOW_SUPPORT_TAB_FOCUS",
			    &qt_embedded_window_support_tab_focus );
	qt_x11_intern_atom( "_QT_EMBEDDED_WINDOW_TAB_FOCUS",
			    &qt_embedded_window_tab_focus );
	qt_x11_intern_atom( "_QT_WHEEL_EVENT", &qt_wheel_event );
	qt_x11_intern_atom( "_QT_UNICODE_KEY_PRESS", &qt_unicode_key_press );
	qt_x11_intern_atom( "_QT_UNICODE_KEY_RELEASE",
			    &qt_unicode_key_release );


	qt_xdnd_setup();

	// Finally create all atoms
	qt_x11_process_intern_atoms();

	// Misc. initialization

	QColor::initialize();
	QFont::initialize();
	QCursor::initialize();
	QPainter::initialize();
    }
    gettimeofday( &watchtime, 0 );

    if( QApplication::is_gui_used ) {
	qApp->setName( appName );

	XSelectInput( appDpy, appRootWin,
		      KeyPressMask | KeyReleaseMask |
		      KeymapStateMask |
		      EnterWindowMask | LeaveWindowMask |
		      FocusChangeMask | PropertyChangeMask
		      );
    }
    qt_xim = 0;
    setlocale( LC_ALL, "" );		// use correct char set mapping
    setlocale( LC_NUMERIC, "C" );	// make sprintf()/scanf() work
    if ( QApplication::is_gui_used ) {
#if !defined(NO_XIM)

	if ( !XSupportsLocale() )
	    qDebug("Qt: Locales not supported on X server");
	else if ( XSetLocaleModifiers ("") == NULL )
	    qDebug("Qt: Cannot set locale modifiers");
	else
	    qt_xim = XOpenIM( appDpy, 0, 0, 0 );

	if ( qt_xim ) {
	    XIMStyles *styles=0;
	    XGetIMValues(qt_xim, XNQueryInputStyle, &styles, NULL, NULL);
	    if ( styles ) {
		bool done = FALSE;
		int i;
		for ( i = 0; !done && i < styles->count_styles; i++ ) {
		    if ( styles->supported_styles[i] == xim_preferred_style ) {
			qt_xim_style = xim_preferred_style;
			done = TRUE;
		    }
		}
		// if the preferred input style couldn't be found, look for
		// Nothing and failing that, None.
		for ( i = 0; !done && i < styles->count_styles; i++ ) {
		    if ( styles->supported_styles[i] == (XIMPreeditNothing |
							 XIMStatusNothing) ) {
			qt_xim_style = XIMPreeditNothing | XIMStatusNothing;
			done = TRUE;
		    }
		}
		for ( i = 0; !done && i < styles->count_styles; i++ ) {
		    if ( styles->supported_styles[i] == (XIMPreeditNone |
							 XIMStatusNone) ) {
			qt_xim_style = XIMPreeditNone | XIMStatusNone;
			done = TRUE;
		    }
		}
		for ( i = 0; i < styles->count_styles; i++) {
		    if (styles->supported_styles[i] == xim_preferred_style) {
			qt_xim_style = xim_preferred_style;
			break;
		    } else if (styles->supported_styles[i] ==
			       (XIMPreeditNone | XIMStatusNone) ||
			       styles->supported_styles[i] ==
			       (XIMPreeditNothing | XIMStatusNothing) ) {
			// Either of these will suffice as a default
			if ( !qt_xim_style )
			    qt_xim_style = styles->supported_styles[i];
		    }
		}
		XFree(styles);
	    }
	    if ( !qt_xim_style ) {
		// Give up
		qWarning( "Input style unsupported."
			  "  See InputMethod documentation.");
		close_xim();
	    }
	}
#endif
	// Always use the locale codec, since we have no examples of non-local
	// XIMs, and since we cannot get a sensible answer about the encoding
	// from the XIM.
	input_mapper = QTextCodec::codecForLocale();

	// pick default character set (now that we have done setlocale stuff)
	QFont::locale_init();
	QFont f;
	if ( QPaintDevice::x11AppDpiX() < 95 )
	    f=QFont( "Helvetica", 12 ); // default font
	else
	    f=QFont( "Helvetica", 11 ); // default font
	f.setCharSet( QFont::charSetForLocale() ); // must come after locale_init()
	QApplication::setFont( f );

	qt_set_x11_resources( appFont, appFGCol, appBGCol, appBTNCol);
    }
}

void qt_init( int *argcptr, char **argv )
{
    qt_init_internal( argcptr, argv, 0 );
}

void qt_init( Display *display )
{
    qt_init_internal( 0, 0, display );
}


/*****************************************************************************
  qt_cleanup() - cleans up when the application is finished
 *****************************************************************************/

void qt_cleanup()
{
    if ( postRList ) {
	VFPTR f = (VFPTR)postRList->first();
	while ( f ) {				// call post routines
	    (*f)();
	    postRList->remove();
	    f = (VFPTR)postRList->first();
	}
	delete postRList;
	postRList = 0;
    }

    if ( app_save_rootinfo )			// root window must keep state
	qt_save_rootinfo();
    cleanupTimers();
    QPixmapCache::clear();
    QPainter::cleanup();
    QCursor::cleanup();
    QFont::cleanup();
    QColor::cleanup();

#if !defined(NO_XIM)
    if ( qt_xim ) {
	close_xim();
    }
#endif

    if ( QApplication::is_gui_used && !QPaintDevice::x11AppDefaultColormap() )
	XFreeColormap( QPaintDevice::x11AppDisplay(),
		       QPaintDevice::x11AppColormap() );

#define CLEANUP_GC(g) if (g) XFreeGC(appDpy,g)
    CLEANUP_GC(app_gc_ro);
    CLEANUP_GC(app_gc_ro_m);
    CLEANUP_GC(app_gc_tmp);
    CLEANUP_GC(app_gc_tmp_m);

    if ( sip_list ) {
	delete sip_list;
	sip_list = 0;
    }

    if ( QApplication::is_gui_used && !appForeignDpy )
	XCloseDisplay( appDpy );		// close X display
    appDpy = 0;
}


/*****************************************************************************
  Platform specific global and internal functions
 *****************************************************************************/

void qt_save_rootinfo()				// save new root info
{
    Atom type;
    int format;
    unsigned long length, after;
    unsigned char *data;

    if ( qt_xsetroot_id ) {			// kill old pixmap
	if ( XGetWindowProperty( appDpy, appRootWin, qt_xsetroot_id, 0, 1,
				 TRUE, AnyPropertyType, &type, &format,
				 &length, &after, &data ) == Success ) {
	    if ( type == XA_PIXMAP && format == 32 && length == 1 &&
		 after == 0 && data ) {
		XKillClient( appDpy, *((Pixmap*)data) );
		XFree( (char *)data );
	    }
	    Pixmap dummy = XCreatePixmap( appDpy, appRootWin, 1, 1, 1 );
	    XChangeProperty( appDpy, appRootWin, qt_xsetroot_id, XA_PIXMAP, 32,
			     PropModeReplace, (uchar *)&dummy, 1 );
	    XSetCloseDownMode( appDpy, RetainPermanent );
	}
    }
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
    unsigned char *data;
    int r = XGetWindowProperty( appDpy, winid, qt_wm_state, 0, 2,
				 FALSE, AnyPropertyType, &type, &format,
				 &length, &after, &data );
    bool iconic = FALSE;
    if ( r == Success && data && format == 32 ) {
	Q_UINT32 *wstate = (Q_UINT32*)data;
	iconic = (*wstate == IconicState );
	XFree( (char *)data );
    }
    return iconic;
}

/*!
  \relates QApplication
  Adds a global routine that will be called from the QApplication destructor.
  This function is normally used to add cleanup routines.


  The function given by \a p should take no arguments and return nothing.

  Example of use:
  \code
    static int *global_ptr = 0;

    void cleanup_ptr()
    {
	delete [] global_ptr;
    }

    void init_ptr()
    {
	global_ptr = new int[100];		// allocate data
	qAddPostRoutine( cleanup_ptr );		// delete later
    }
  \endcode
*/

void qAddPostRoutine( Q_CleanUpFunction p )
{
    if ( !postRList ) {
	postRList = new QVFuncList;
	CHECK_PTR( postRList );
    }
    postRList->insert( 0, (void *)p );		// store at list head
}


char *qAppName()				// get application name
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

WId qt_xrootwin()				// get X root window
{
    return appRootWin;
}

bool qt_nograb()				// application no-grab option
{
#if defined(DEBUG)
    return appNoGrab;
#else
    return FALSE;
#endif
}

static GC create_gc( bool monochrome )
{
    GC gc;
    if ( monochrome ) {
	Pixmap pm = XCreatePixmap( appDpy, appRootWin, 8, 8, 1 );
	gc = XCreateGC( appDpy, pm, 0, 0 );
	XFreePixmap( appDpy, pm );
    } else {
	if ( QPaintDevice::x11AppDefaultVisual() ) {
	    gc = XCreateGC( appDpy, appRootWin, 0, 0 );
	} else {
	    Window w;
	    XSetWindowAttributes a;
	    a.background_pixel = Qt::black.pixel();
	    a.border_pixel = Qt::black.pixel();
	    a.colormap = QPaintDevice::x11AppColormap();
	    w = XCreateWindow( appDpy, appRootWin, 0, 0, 100, 100,
			       0, QPaintDevice::x11AppDepth(), InputOutput,
			       (Visual*)QPaintDevice::x11AppVisual(),
			       CWBackPixel|CWBorderPixel|CWColormap, &a );
	    gc = XCreateGC( appDpy, w, 0, 0 );
	    XDestroyWindow( appDpy, w );
	}
    }
    XSetGraphicsExposures( appDpy, gc, FALSE );
    return gc;
}

GC qt_xget_readonly_gc( bool monochrome )	// get read-only GC
{
    GC gc;
    if ( monochrome ) {
	if ( !app_gc_ro_m )			// create GC for bitmap
	    app_gc_ro_m = create_gc(TRUE);
	gc = app_gc_ro_m;
    } else {					// create standard GC
	if ( !app_gc_ro )			// create GC for bitmap
	    app_gc_ro = create_gc(FALSE);
	gc = app_gc_ro;
    }
    return gc;
}

GC qt_xget_temp_gc( bool monochrome )		// get temporary GC
{
    GC gc;
    if ( monochrome ) {
	if ( !app_gc_tmp_m )			// create GC for bitmap
	    app_gc_tmp_m = create_gc(TRUE);
	gc = app_gc_tmp_m;
    } else {					// create standard GC
	if ( !app_gc_tmp )			// create GC for bitmap
	    app_gc_tmp = create_gc(FALSE);
	gc = app_gc_tmp;
    }
    return gc;
}


/*****************************************************************************
  Platform specific QApplication members
 *****************************************************************************/

/*!
  \fn QWidget *QApplication::mainWidget() const
  Returns the main application widget, or 0 if there is not a defined
  main widget.
  \sa setMainWidget()
*/

/*!
  Sets the main widget of the application.

  The special thing about the main widget is that destroying the main
  widget (i.e. the program calls QWidget::close() or the user
  double-clicks the window close box) will leave the main event loop and
  \link QApplication::quit() exit the application\endlink.

  For X11, this function also resizes and moves the main widget
  according to the \e -geometry command-line option, so you should
  \link QWidget::setGeometry() set the default geometry\endlink before
  calling setMainWidget().

  \sa mainWidget(), exec(), quit()
*/

void QApplication::setMainWidget( QWidget *mainWidget )
{
    extern int qwidget_tlw_gravity;		// in qwidget_x11.cpp
    main_widget = mainWidget;
    if ( main_widget ) {			// give WM command line
	XSetWMProperties( main_widget->x11Display(), main_widget->winId(),
			  0, 0, app_argv, app_argc, 0, 0, 0 );
	if ( mwTitle )
	    XStoreName( appDpy, main_widget->winId(), mwTitle );
	if ( mwGeometry ) {			// parse geometry
	    int x, y;
	    int w, h;
	    int m = XParseGeometry( mwGeometry, &x, &y, (uint*)&w, (uint*)&h );
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
		qwidget_tlw_gravity = 3;
	    }
	    if ( (m & YNegative) ) {
		y = desktop()->height() + y - h;
		qwidget_tlw_gravity = (m & XNegative) ? 9 : 7;
	    }
	    main_widget->setGeometry( x, y, w, h );
	}
    }
}


/*****************************************************************************
  QApplication cursor stack
 *****************************************************************************/

typedef QList<QCursor> QCursorList;

static QCursorList *cursorStack = 0;

/*!
  \fn QCursor *QApplication::overrideCursor()
  Returns the active application override cursor.

  This function returns 0 if no application cursor has been defined (i.e. the
  internal cursor stack is empty).

  \sa setOverrideCursor(), restoreOverrideCursor()
*/

/*!
  Sets the application override cursor to \e cursor.

  Application override cursor are intended for showing the user that the
  application is in a special state, for example during an operation that
  might take some time.

  This cursor will be displayed in all application widgets until
  restoreOverrideCursor() or another setOverrideCursor() is called.

  Application cursors are stored on an internal stack. setOverrideCursor()
  pushes the cursor onto the stack, and restoreOverrideCursor() pops the
  active cursor off the stack.	Every setOverrideCursor() must have an
  corresponding restoreOverrideCursor(), otherwise the stack will get out
  of sync. overrideCursor() returns 0 if the cursor stack is empty.

  If \e replace is TRUE, the new cursor will replace the last override
  cursor.

  Example:
  \code
    QApplication::setOverrideCursor( waitCursor );
    calculateHugeMandelbrot();			// lunch time...
    QApplication::restoreOverrideCursor();
  \endcode

  \sa overrideCursor(), restoreOverrideCursor(), QWidget::setCursor()
*/

void QApplication::setOverrideCursor( const QCursor &cursor, bool replace )
{
    if ( !cursorStack ) {
	cursorStack = new QCursorList;
	CHECK_PTR( cursorStack );
	cursorStack->setAutoDelete( TRUE );
    }
    app_cursor = new QCursor( cursor );
    CHECK_PTR( app_cursor );
    if ( replace )
	cursorStack->removeLast();
    cursorStack->append( app_cursor );
    QWidgetIntDictIt it( *((QWidgetIntDict*)QWidget::mapper) );
    register QWidget *w;
    while ( (w=it.current()) ) {		// for all widgets that have
	if ( w->testWState(WState_OwnCursor) )	//   set a cursor
	    XDefineCursor( w->x11Display(), w->winId(), app_cursor->handle() );
	++it;
    }
    XFlush( appDpy );				// make X execute it NOW
}

/*!
  Restores the effect of setOverrideCursor().

  If setOverrideCursor() has been called twice, calling
  restoreOverrideCursor() will activate the first cursor set.  Calling
  this function a second time restores the original widgets cursors.

  Application cursors are stored on an internal stack. setOverrideCursor()
  pushes the cursor onto the stack, and restoreOverrideCursor() pops the
  active cursor off the stack.	Every setOverrideCursor() must have an
  corresponding restoreOverrideCursor(), otherwise the stack will get out
  of sync. overrideCursor() returns 0 if the cursor stack is empty.

  \sa setOverrideCursor(), overrideCursor().
*/

void QApplication::restoreOverrideCursor()
{
    if ( !cursorStack )				// no cursor stack
	return;
    cursorStack->removeLast();
    app_cursor = cursorStack->last();
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
    if ( !app_cursor ) {
	delete cursorStack;
	cursorStack = 0;
    }
}


/*!
  \fn bool QApplication::hasGlobalMouseTracking()
  Returns TRUE if global mouse tracking is enabled, otherwise FALSE.

  \sa setGlobalMouseTracking()
*/

/*!
  Enables global mouse tracking if \a enable is TRUE or disables it
  if \a enable is FALSE.

  Enabling global mouse tracking makes it possible for widget event
  filters or application event filters to get all mouse move events, even
  when no button is depressed.  This is useful for special GUI elements,
  e.g. tool tips.

  Global mouse tracking does not affect widgets and their
  mouseMoveEvent().  For a widget to get mouse move events when no button
  is depressed, it must do QWidget::setMouseTracking(TRUE).

  This function has an internal counter.  Each
  setGlobalMouseTracking(TRUE) must have a corresponding
  setGlobalMouseTracking(FALSE).

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
		    w->clearWState(WState_MouseTracking);
		}
	    } else {				// switch off
		if ( !w->testWState(WState_MouseTracking) ) {
		    w->setWState(WState_MouseTracking);
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

static QWidget *findChildWidget( const QWidget *p, const QPoint &pos )
{
    if ( p->children() ) {
	QWidget *w;
	QObjectListIt it( *p->children() );
	it.toLast();
	while ( it.current() ) {
	    if ( it.current()->isWidgetType() ) {
		w = (QWidget*)it.current();
		if ( w->isVisible() && w->geometry().contains(pos) ) {
		    QWidget *c = findChildWidget( w, w->mapFromParent(pos) );
		    return c ? c : w;
		}
	    }
	    --it;
	}
    }
    return 0;
}

Window qt_x11_findClientWindow( Window win, Atom property, bool leaf )
{
    Atom   type = None;
    int	   format, i;
    ulong  nitems, after;
    uchar *data;
    Window root, parent, target=0, *children=0;
    uint   nchildren;
    XGetWindowProperty( appDpy, win, property, 0, 0, FALSE, AnyPropertyType,
			&type, &format, &nitems, &after, &data );
    if ( data )
	XFree( (char *)data );
    if ( type )
	return win;
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
  Returns a pointer to the widget at global screen position \a (x,y), or a
  null pointer if there is no Qt widget there.

  If \a child is FALSE and there is a child widget at position \a (x,y),
  the top-level widget containing it is returned.  If \a child is TRUE
  the child widget at position \a (x,y) is returned.

  \sa QCursor::pos(), QWidget::grabMouse(), QWidget::grabKeyboard()
*/

QWidget *QApplication::widgetAt( int x, int y, bool child )
{
    int lx, ly;

    Window target;
    if ( !XTranslateCoordinates(appDpy, appRootWin, appRootWin,
				x, y, &lx, &ly, &target) )
	return 0;
    if ( !target || target == appRootWin )
	return 0;
    QWidget *w, *c;
    w = QWidget::find( (WId)target );
    if ( child && w ) {
	c = findChildWidget( w, w->mapFromParent(QPoint(lx,ly)) );
	if ( c )
	    return c;
    }

    if ( !qt_wm_state )
	return w;

    //########### why so complex? Why not findChildWidget(...)->topLevelWidget();? me


    target = qt_x11_findClientWindow( target, qt_wm_state, TRUE );
    c = QWidget::find( (WId)target );
    if ( !c ) {
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
			XTranslateCoordinates(appDpy, appRootWin, ctarget,
			    x, y, &lx, &ly, &ctarget);
			if ( ctarget == wid ) {
			    // Found
			    w = widget;
			    XTranslateCoordinates(appDpy, appRootWin, ctarget,
				x, y, &lx, &ly, &ctarget);
			}
		    }
		}
		widget = list->next();
	    }
	    delete list;
	}
	c = w;
	if ( !w )
	    return w;
    }
    if ( child ) {
	c = findChildWidget( c, c->mapFromParent(QPoint(lx,ly)) );
	if ( !c )
	    c = w;
    }
    return c;
}

/*!
  \overload QWidget *QApplication::widgetAt( const QPoint &pos, bool child )
*/


/*!
  Flushes the X event queue in the X11 implementation.
  Does nothing on other platforms.
  \sa syncX()
*/

void QApplication::flushX()
{
    if ( appDpy )
	XFlush( appDpy );
}

/*!
  Synchronizes with the X server in the X11 implementation.
  Does nothing on other platforms.
  \sa flushX()
*/

void QApplication::syncX()
{
    if ( appDpy )
	XSync( appDpy, FALSE );			// don't discard events
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
	CHECK_PTR( wPRmapper );
    }
    wPRmapper->insert( (long)oldwin, widget );	// add old window to mapper
    QETWidget *w = (QETWidget *)widget;
    w->setWState( Qt::WState_Reparented );	// set reparented flag
}

void qPRCleanup( QETWidget *widget )
{
    if ( !(wPRmapper && widget->testWState(Qt::WState_Reparented)) )
	return;					// not a reparented widget
    QWidgetIntDictIt it(*wPRmapper);
    QWidget *w;
    while ( (w=it.current()) ) {
	if ( w == widget ) {			// found widget
	    widget->clearWState( Qt::WState_Reparented ); // clear flag
	    wPRmapper->remove( it.currentKey());// old window no longer needed
	    if ( wPRmapper->count() == 0 ) {	// became empty
		delete wPRmapper;		// then reset alt mapper
		wPRmapper = 0;
	    }
	    return;
	}
	++it;
    }
}

QETWidget *qPRFindWidget( Window oldwin )
{
    return wPRmapper ? (QETWidget*)wPRmapper->find((long)oldwin) : 0;
}


/*****************************************************************************
  Socket notifier (type: 0=read, 1=write, 2=exception)

  The QSocketNotifier class (qsocketnotifier.h) provides installable callbacks
  for select() through the internal function qt_set_socket_handler().
 *****************************************************************************/

struct QSockNot {
    QObject *obj;
    int	     fd;
    fd_set  *queue;
};

typedef QList<QSockNot> QSNList;
typedef QListIterator<QSockNot> QSNListIt;

static int	sn_highest = -1;
static QSNList *sn_read	   = 0;
static QSNList *sn_write   = 0;
static QSNList *sn_except  = 0;

static fd_set	sn_readfds;			// fd set for reading
static fd_set	sn_writefds;			// fd set for writing
static fd_set	sn_exceptfds;			// fd set for exceptions
static fd_set	sn_queued_read;
static fd_set	sn_queued_write;
static fd_set	sn_queued_except;

static struct SN_Type {
    QSNList **list;
    fd_set   *fdspec;
    fd_set   *fdres;
    fd_set   *queue;
} sn_vec[3] = {
    { &sn_read,	  &sn_readfds,	 &app_readfds,   &sn_queued_read },
    { &sn_write,  &sn_writefds,	 &app_writefds,  &sn_queued_write },
    { &sn_except, &sn_exceptfds, &app_exceptfds, &sn_queued_except } };


static QSNList *sn_act_list = 0;


static void sn_cleanup()
{
    delete sn_act_list;
    sn_act_list = 0;
    for ( int i=0; i<3; i++ ) {
	delete *sn_vec[i].list;
	*sn_vec[i].list = 0;
    }
}


static void sn_init()
{
    if ( !sn_act_list ) {
	sn_act_list = new QSNList;
	CHECK_PTR( sn_act_list );
	qAddPostRoutine( sn_cleanup );
    }
}


bool qt_set_socket_handler( int sockfd, int type, QObject *obj, bool enable )
{
    if ( sockfd < 0 || type < 0 || type > 2 || obj == 0 ) {
#if defined(CHECK_RANGE)
	qWarning( "QSocketNotifier: Internal error" );
#endif
	return FALSE;
    }

    QSNList  *list = *sn_vec[type].list;
    fd_set   *fds  =  sn_vec[type].fdspec;
    QSockNot *sn;

    if ( enable ) {				// enable notifier
	if ( !list ) {
	    sn_init();
	    list = new QSNList;			// create new list
	    CHECK_PTR( list );
	    list->setAutoDelete( TRUE );
	    *sn_vec[type].list = list;
	    FD_ZERO( fds );
	    FD_ZERO( sn_vec[type].queue );
	}
	sn = new QSockNot;
	CHECK_PTR( sn );
	sn->obj = obj;
	sn->fd	= sockfd;
	sn->queue = sn_vec[type].queue;
	if ( list->isEmpty() ) {
	    list->insert( 0, sn );
	} else {				// sort list by fd, decreasing
	    QSockNot *p = list->first();
	    while ( p && p->fd > sockfd )
		p = list->next();
#if defined(CHECK_STATE)
	    if ( p && p->fd == sockfd ) {
		static const char *t[] = { "read", "write", "exception" };
		qWarning( "QSocketNotifier: Multiple socket notifiers for "
			 "same socket %d and type %s", sockfd, t[type] );
	    }
#endif
	    if ( p )
		list->insert( list->at(), sn );
	    else
		list->append( sn );
	}
	FD_SET( sockfd, fds );
	sn_highest = QMAX(sn_highest,sockfd);

    } else {					// disable notifier

	if ( list == 0 )
	    return FALSE;			// no such fd set
	QSockNot *sn = list->first();
	while ( sn && !(sn->obj == obj && sn->fd == sockfd) )
	    sn = list->next();
	if ( !sn )				// not found
	    return FALSE;
	FD_CLR( sockfd, fds );			// clear fd bit
	FD_CLR( sockfd, sn->queue );
	if ( sn_act_list )
	    sn_act_list->removeRef( sn );	// remove from activation list
	list->remove();				// remove notifier found above
	if ( sn_highest == sockfd ) {		// find highest fd
	    sn_highest = -1;
	    for ( int i=0; i<3; i++ ) {
		if ( *sn_vec[i].list && (*sn_vec[i].list)->count() )
		    sn_highest = QMAX(sn_highest,  // list is fd-sorted
				      (*sn_vec[i].list)->getFirst()->fd);
	    }
	}
    }

    return TRUE;
}


//
// We choose a random activation order to be more fair under high load.
// If a constant order is used and a peer early in the list can
// saturate the IO, it might grab our attention completely.
// Also, if we're using a straight list, the callback routines may
// delete other entries from the list before those other entries are
// processed.
//

static int sn_activate()
{
    if ( !sn_act_list )
	sn_init();
    int i, n_act = 0;
    for ( i=0; i<3; i++ ) {			// for each list...
	if ( *sn_vec[i].list ) {		// any entries?
	    QSNList  *list = *sn_vec[i].list;
	    fd_set   *fds  = sn_vec[i].fdres;
	    QSockNot *sn   = list->first();
	    while ( sn ) {
		if ( FD_ISSET( sn->fd, fds ) &&	// store away for activation
		     !FD_ISSET( sn->fd, sn->queue ) ) {
		    sn_act_list->insert( (rand() & 0xff) %
					 (sn_act_list->count()+1),
					 sn );
		    FD_SET( sn->fd, sn->queue );
		}
		sn = list->next();
	    }
	}
    }
    if ( sn_act_list->count() > 0 ) {		// activate entries
	QEvent event( QEvent::SockAct );
	QSNListIt it( *sn_act_list );
	QSockNot *sn;
	while ( (sn=it.current()) ) {
	    ++it;
	    sn_act_list->removeRef( sn );
	    if ( FD_ISSET(sn->fd, sn->queue) ) {
		FD_CLR( sn->fd, sn->queue );
		QApplication::sendEvent( sn->obj, &event );
		n_act++;
	    }
	}
    }
    return n_act;
}


/*****************************************************************************
  Main event loop
 *****************************************************************************/

/*!
  Enters the main event loop and waits until exit() is called or
  the \link setMainWidget() main widget\endlink is destroyed.
  Returns the value that was specified to exit(), which is 0 if
  exit() is called via quit().

  It is necessary to call this function to start event handling.
  The main event loop receives \link QWidget::event() events\endlink from
  the window system and dispatches these to the application widgets.

  Generally, no user interaction can take place before calling exec().
  As a special case, modal widgets like QMessageBox can be used before
  calling exec(), because modal widget have a local event loop.

  To make your application perform idle processing, i.e. executing a
  special function whenever there are no pending events, use a QTimer
  with 0 timeout. More advanced idle processing schemes can be
  achieved by using processEvents() and processOneEvent().

  \sa quit(), exit(), processEvents(), setMainWidget()
*/

int QApplication::exec()
{
    quit_now = FALSE;
    quit_code = 0;
    enter_loop();
    return quit_code;
}


/*!
  Processes the next event and returns TRUE if there was an event
  (excluding posted events or zero-timer events) to process.

  This function returns immediately if \e canWait is FALSE. It might go
  into a sleep/wait state if \e canWait is TRUE.

  \sa processEvents()
*/

bool QApplication::processNextEvent( bool canWait )
{
    XEvent event;
    int	   nevents = 0;

    if (is_gui_used ) {
	sendPostedEvents();

	while ( XPending(appDpy) ) {		// also flushes output buffer
	    if ( app_exit_loop )		// quit between events
		return FALSE;
	    XNextEvent( appDpy, &event );	// get next event
	    nevents++;

	    if ( x11ProcessEvent( &event ) == 1 )
		return TRUE;
	}
    }
    if ( app_exit_loop )			// break immediately
	return FALSE;

    sendPostedEvents();

    static timeval zerotm;
    timeval *tm = qt_wait_timer();		// wait for timer or X event
    if ( !canWait ) {
	if ( !tm )
	    tm = &zerotm;
	tm->tv_sec  = 0;			// no time to wait
	tm->tv_usec = 0;
    }
    if ( sn_highest >= 0 ) {			// has socket notifier(s)
	if ( sn_read )
	    app_readfds = sn_readfds;
	else
	    FD_ZERO( &app_readfds );
	if ( sn_write )
	    app_writefds = sn_writefds;
	if ( sn_except )
	    app_exceptfds = sn_exceptfds;
    } else {
	FD_ZERO( &app_readfds );
    }

    if ( is_gui_used ) {
	FD_SET( app_Xfd, &app_readfds );
	XFlush( appDpy );
    }
    int nsel;

#if defined(_OS_WIN32_)
#define FDCAST (fd_set*)
#else
#define FDCAST (void*)
#endif

    nsel = select( is_gui_used ? ( QMAX(app_Xfd,sn_highest)+1) : (sn_highest+1) ,
		   FDCAST (&app_readfds),
		   FDCAST (sn_write  ? &app_writefds  : 0),
		   FDCAST (sn_except ? &app_exceptfds : 0),
		   tm );
#undef FDCAST

    if ( nsel == -1 ) {
	if ( errno == EINTR || errno == EAGAIN ) {
	    errno = 0;
	    return (nevents > 0);
	} else {
	    ; // select error
	}
    } else if ( nsel > 0 && sn_highest >= 0 ) {
	nevents += sn_activate();
    }

    nevents += qt_activate_timers();		// activate timers
    qt_reset_color_avail();			// color approx. optimization

    return (nevents > 0);
}

int QApplication::x11ClientMessage(QWidget* w, XEvent* event, bool passive_only)
{
    QETWidget *widget = (QETWidget*)w;
    if ( event->xclient.format == 32 && event->xclient.message_type ) {
	if ( event->xclient.message_type == qt_wm_protocols ) {
	    if ( passive_only ) return 0;
	    long *l = event->xclient.data.l;
	    if ( *l == (long)qt_wm_delete_window )
		widget->translateCloseEvent(event);
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
	    if ( event->xclient.message_type == qt_embedded_window_take_focus ) {
		widget->setFocus();
	    } else if ( event->xclient.message_type == qt_embedded_window_focus_in ) {
		active_window = widget->topLevelWidget();
		QWidget *w = widget->focusWidget();
		while ( w && w->focusProxy() )
		    w = w->focusProxy();
		if ( w && w->isFocusEnabled() )
		    w->setFocus();
		else
		    widget->focusNextPrevChild( TRUE );
	    } else if ( event->xclient.message_type == qt_embedded_window_focus_out ) {
		active_window = 0;
		if ( focus_widget && !inPopupMode() ) {
		    QFocusEvent out( QEvent::FocusOut );
		    QWidget *widget = focus_widget;
		    focus_widget = 0;
		    QApplication::sendEvent( widget, &out );
		}
	    } else if ( event->xclient.message_type == qt_wheel_event ) {
		return widget->translateWheelEvent( event->xclient.data.l[0],
						    event->xclient.data.l[1],
						    event->xclient.data.l[2],
						    event->xclient.data.l[3] );
	    }
	}
    }
    else if ( event->xclient.format == 16 ) {
	if ( passive_only ) return 0; // all below are interactions
	if ( event->xclient.message_type == qt_unicode_key_press
	     || event->xclient.message_type == qt_unicode_key_release ) {

	    QWidget *g = QWidget::keyboardGrabber();
	    if ( g )
		widget = (QETWidget*)g;
	    else if ( focus_widget )
		widget = (QETWidget*)focus_widget;
	    else
		widget = (QETWidget*)widget->topLevelWidget();

	    if ( !widget || !widget->isEnabled() )
		return 0;
	    bool grab = g != 0;

	    QEvent::Type type = event->xclient.message_type == qt_unicode_key_press?
				QEvent::KeyPress : QEvent::KeyRelease;

	    short *s = event->xclient.data.s;
	    QChar c(s[6],s[5]);
	    QString text;
	    if (c != QChar::null)
		text = c;
	    if (!grab && type == QEvent::KeyPress) {
		// test accelerators first
		QKeyEvent a (QEvent::Accel, s[0], s[1], s[2], text, s[3], s[4]);
		a.ignore();
		QApplication::sendEvent( widget->topLevelWidget(), &a );
		if ( a.isAccepted() )
		    return 1;
	    }
	    QKeyEvent kev(type, s[0], s[1], s[2], text, s[3], s[4]);
	    QApplication::sendEvent( widget, &kev );
	    return 1;
	}
    }
    return 0;
}


/*!
  Returns
  1 if the event was consumed by special handling,
  0 if the event was consumed by normal handling, and
  -1 if the event was for an unrecognized widget.

  \internal

  This documentation is unclear.
*/
int QApplication::x11ProcessEvent( XEvent* event )
{
    if ( x11EventFilter(event) )		// send through app filter
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
	    else if ( widget )
		keywidget = (QETWidget*)widget->topLevelWidget();
	}
    }
    int xkey_keycode = event->xkey.keycode;
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
	qt_x_clipboardtime = event->xproperty.time;
	if ( event->xproperty.window == appRootWin ) { // root properties
	    if ( obey_desktop_settings ) {
		if ( event->xproperty.atom == qt_resource_manager )
		    qt_set_x11_resources();
		else if ( event->xproperty.atom == qt_desktop_properties )
		    qt_set_desktop_properties();
	    }
	} else if ( widget ) { // widget properties
	    if ( event->xproperty.atom == qt_wm_state ) {
		widget->createTLExtra();
		widget->extra->topextra->wmstate = 1;
		if ( qt_deferred_map_contains( widget ) ) {
		    qt_deferred_map_take( widget );
		    XMapWindow( appDpy, widget->winId() );
		}
	    }
	    else if ( event->xproperty.atom == qt_embedded_window ) {
		Atom type;
		int format;
		unsigned long length, after;
		unsigned char *data;
		if ( XGetWindowProperty( appDpy, widget->winId(), qt_embedded_window, 0, 1,
					 FALSE, XA_CARDINAL, &type, &format,
					 &length, &after, &data ) == Success ) {
		    if (data ) {
			widget->createTLExtra();
			widget->extra->topextra->embedded = ((long*)data[0])?1:0;
			XFree( data );
			if ( widget->extra->topextra->embedded ) {
			    // we support tab focus, inform the embedding widget about it
			    XClientMessageEvent client_message;
			    client_message.type = ClientMessage;
			    client_message.window = widget->extra->topextra->parentWinId;
			    client_message.format = 32;
			    client_message.message_type = qt_embedded_window_support_tab_focus;
			    XSendEvent( appDpy, client_message.window, FALSE, NoEventMask,
					(XEvent*)&client_message );
			}
		    }
		}
	    }
	}
	return 0;
    }

    if ( !widget ) {				// don't know this window
	if ( (widget=(QETWidget*)QApplication::activePopupWidget()) )
	    {
		// Danger - make sure we don't lock the server
		switch ( event->type ) {
		case ButtonPress:
		case ButtonRelease:
		case XKeyPress:
		case XKeyRelease:
		    widget->close();
		    return 1;
		}
	    } else {
		void qt_np_process_foreign_event(XEvent*); // in qnpsupport.cpp
		qt_np_process_foreign_event( event );
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

    switch ( event->type ) {

    case ButtonPress:			// mouse event
    case ButtonRelease:
    case MotionNotify:
	qt_x_clipboardtime = (event->type == MotionNotify) ?
			     event->xmotion.time : event->xbutton.time;
	widget->translateMouseEvent( event );
	break;

    case XKeyPress:				// keyboard event
    case XKeyRelease: {
	qt_x_clipboardtime = event->xkey.time;
	if ( keywidget ) // should always exist
	    keywidget->translateKeyEvent( event, grabbed );
    }
    break;

    case GraphicsExpose:
    case Expose:				// paint event
	if ( widget->testWState(WState_ForceHide) ) {
	  //widget->setWState( WState_Visible );
	  //widget->hide();
	} else {
	    widget->translatePaintEvent( event );
	}
	break;

    case ConfigureNotify:			// window move/resize event
	widget->translateConfigEvent( event );
	break;

    case XFocusIn: {				// got focus
	if ( widget == desktop() )
	    return TRUE; // not interesting
 	if ( inPopupMode() ) // some delayed focus event to ignore
 	    break;
// 	if ( event->xfocus.mode == NotifyUngrab )
// 	    break;
	QWidget* old_active_window = active_window;
	active_window = widget->topLevelWidget();
	if (active_window && active_window->extra &&
	    active_window->extra->topextra &&
	    active_window->extra->topextra->embedded) {
	    // we are embedded. Refuse focus, the out app will send us
	    // qew_focus_in if that shall be necessary
	    ((XEvent*)event)->xfocus.window
		= active_window->extra->topextra->parentWinId;
	    XSendEvent( appDpy, active_window->extra->topextra->parentWinId,
			NoEventMask, FALSE, (XEvent*)event);
	    active_window = old_active_window;
	    return TRUE;
	}

	QWidget *w = widget->focusWidget();
	while ( w && w->focusProxy() )
	    w = w->focusProxy();
	if ( w && w->isFocusEnabled() )
	    w->setFocus();
	else
	    widget->focusNextPrevChild( TRUE );
	if ( !focus_widget ) {
	    if ( widget->focusWidget() )
		widget->focusWidget()->setFocus();
	    else
		widget->topLevelWidget()->setFocus();
	}
    }
    break;

    case XFocusOut:				// lost focus
	if ( widget == desktop() )
	    return TRUE; // not interesting
// 	if ( event->xfocus.mode == NotifyGrab )
// 	    break;
	active_window = 0;
	if ( focus_widget && !inPopupMode() ) {
	    QFocusEvent out( QEvent::FocusOut );
	    QWidget *widget = focus_widget;
	    focus_widget = 0;
	    QApplication::sendEvent( widget, &out );
	}
	break;

    case EnterNotify:			// enter window
    case LeaveNotify: {			// leave window
	qt_x_clipboardtime = event->xcrossing.time;
	if ( event->xcrossing.detail == NotifyNormal )
	    widget->translateMouseEvent( event ); //we don't get MotionNotify
	QEvent e( event->type == EnterNotify ? QEvent::Enter : QEvent::Leave );
	QApplication::sendEvent( widget, &e );
    }
    break;

    case UnmapNotify:			// window hidden
	if ( widget->isVisible() ) {
	    widget->clearWState( WState_Visible );
	    widget->clearWState( WState_Withdrawn );
	    QHideEvent e( TRUE );
	    QApplication::sendEvent( widget, &e );
	    widget->sendHideEventsToChildren( TRUE );
	}
	break;

    case MapNotify:				// window shown
	if ( !widget->isVisible() )  {
	    if ( widget->testWState( WState_Withdrawn ) ) {
		// this cannot happen in normal applications but might happen with embedding
		widget->show();
	    }
	    else {
		widget->setWState( WState_Visible );
		widget->clearWState( WState_Withdrawn );
		widget->sendShowEventsToChildren( TRUE );
		QShowEvent e( TRUE );
		QApplication::sendEvent( widget, &e );
	    }
	}
	break;

    case ClientMessage:			// client message
	return x11ClientMessage(widget,event,FALSE);
	//break;

    case ReparentNotify:			// window manager reparents
	while ( XCheckTypedWindowEvent( widget->x11Display(),
					widget->winId(),
					ReparentNotify,
					event ) )
		    ;	// skip old reparent events
	if ( event->xreparent.parent == appRootWin ) {

	    QTLWExtra*  x = widget->extra? widget->extra->topextra : 0;
	    if ( x )
		x->parentWinId = appRootWin;
	}
	else if (!QWidget::find((WId)event->xreparent.parent) )
	    {
		Window parent = event->xreparent.parent;

		// We can drop the entire crect calculation here (it's
		// already done in translateConfigEvent()
		// anyway). QWidget::frameGeometry() should do this
		// calculation instead - Matthias

		int x = event->xreparent.x;
		int y = event->xreparent.y;
		XWindowAttributes a;
		qt_ignore_badwindow();
		XGetWindowAttributes( widget->x11Display(), parent,
				      &a );
		if (qt_badwindow())
		    break;

		QRect& r = widget->crect;
		QRect frect ( r );

		if ( x == 0 && y == 0 && a.width == r.width() && a.height == r.height() ) {
		    // multi reparenting window manager, parent is just a shell
		    Window root_return, parent_return, *children_return;
		    unsigned int nchildren;
		    if ( XQueryTree( widget->x11Display(), parent,
				     &root_return, &parent_return,
				     &children_return, &nchildren) ) {
			if ( children_return )
			    XFree( (void*) children_return );
			XWindowAttributes a2;
			qt_ignore_badwindow();
			XGetWindowAttributes( widget->x11Display(), parent_return,
					      &a2 );
			if (qt_badwindow())
			    break;
			x += a.x;
			y += a.y;
			frect.setRect(r.left() - x - a2.border_width,
				      r.top() - y - a2.border_width,
				      a2.width + 2*a2.border_width,
				      a2.height + 2*a2.border_width);
		    }
		} else {
		    // single reparenting window manager
		    frect.setRect(r.left() - x - a.border_width,
				  r.top() - y - a.border_width,
				  a.width + 2*a.border_width,
				  a.height + 2*a.border_width);
		}

		widget->createTLExtra();
		widget->fpos = frect.topLeft();
		widget->extra->topextra->fsize = frect.size();

		// store the parent. Useful for many things, embedding for instance.
		widget->extra->topextra->parentWinId = parent;
	    }
	break;

    case SelectionRequest:
	if ( qt_xdnd_selection ) {
	    XSelectionRequestEvent *req = &event->xselectionrequest;
	    if ( req && req->selection == qt_xdnd_selection ) {
		qt_xdnd_handle_selection_request( req );
		break;
	    }
	}
	// FALL THROUGH
    case SelectionClear:
    case SelectionNotify:
	if ( qt_clipboard ) {
	    QCustomEvent e( QEvent::Clipboard, event );
	    QApplication::sendEvent( qt_clipboard, &e );
	}
	break;

    default:
	break;
    }

    return 0;
}


/*!
  Processes pending events, for \a maxtime milliseconds or until there
  are no more events to process, then return.

  You can call this function occasionally when you program is busy doing a
  long operation (e.g. copying a file).

  \sa processOneEvent(), exec(), QTimer
*/
void QApplication::processEvents( int maxtime )
{
    QTime start = QTime::currentTime();
    QTime now;
    while ( !app_exit_loop && processNextEvent(FALSE) ) {
	now = QTime::currentTime();
	if ( start.msecsTo(now) > maxtime )
	    break;
    }
}


/*!
  This virtual function is only implemented under X11.

  If you create an application that inherits QApplication and reimplement this
  function, you get direct access to all X events that the are received
  from the X server.

  Return TRUE if you want to stop the event from being dispatched, or return
  FALSE for normal event dispatching.
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
    if ( !modal_stack ) {			// create modal stack
	modal_stack = new QWidgetList;
	CHECK_PTR( modal_stack );
    }
    modal_stack->insert( 0, widget );
    app_do_modal = TRUE;
}


void qt_leave_modal( QWidget *widget )
{
    if ( modal_stack && modal_stack->removeRef(widget) ) {
	if ( modal_stack->isEmpty() ) {
	    delete modal_stack;
	    modal_stack = 0;
	}
    }
    app_do_modal = modal_stack != 0;
}


static bool qt_try_modal( QWidget *widget, XEvent *event )
{
    if ( qApp->activePopupWidget() )
	return TRUE;
    if ( widget->testWFlags(Qt::WStyle_Tool) )	// allow tool windows
	return TRUE;

    QWidget *modal=0, *top=modal_stack->getFirst();

    widget = widget->topLevelWidget();
    if ( widget->testWFlags(Qt::WType_Modal) )	// widget is modal
	modal = widget;
    if ( modal == top )				// don't block event
	return TRUE;

#ifdef ALLOW_NON_APPLICATION_MODAL
    if ( top && top->parentWidget() ) {
	// Not application-modal
	// Does widget have a child in modal_stack?
	bool unrelated = TRUE;
	modal = modal_stack->first();
	while (modal && unrelated) {
	    QWidget* p = modal->parentWidget();
	    while ( p && p != widget ) {
		p = p->parentWidget();
	    }
	    modal = modal_stack->next();
	    if ( p ) unrelated = FALSE;
	}
	if ( unrelated ) return TRUE;		// don't block event
    }
#endif

    bool block_event  = FALSE;
    bool expose_event = FALSE;

    switch ( event->type ) {
	case ButtonPress:			// disallow mouse/key events
	case ButtonRelease:
	case MotionNotify:
	case XKeyPress:
	case XKeyRelease:
	case XFocusIn:
	case XFocusOut:
	case ClientMessage:
	    block_event	 = TRUE;
	    break;
	case Expose:
	    expose_event = TRUE;
	    break;
    }

    if ( top->parentWidget() == 0 && (block_event || expose_event) )
	XRaiseWindow( appDpy, top->winId() );	// raise app-modal widget

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

void QApplication::openPopup( QWidget *popup )
{
    if ( !popupWidgets ) {			// create list
	popupWidgets = new QWidgetList;
	CHECK_PTR( popupWidgets );
    }
    popupWidgets->append( popup );		// add to end of list
    if ( popupWidgets->count() == 1 && !qt_nograb() ){ // grab mouse/keyboard
	int r;
	r = XGrabKeyboard( popup->x11Display(), popup->winId(), TRUE,
			   GrabModeSync, GrabModeSync, CurrentTime );
	if ( (popupGrabOk = (r == GrabSuccess)) ) {
	    XAllowEvents( popup->x11Display(), SyncKeyboard, CurrentTime );
	    r = XGrabPointer( popup->x11Display(), popup->winId(), TRUE,
			      (uint)(ButtonPressMask | ButtonReleaseMask |
				     ButtonMotionMask | EnterWindowMask |
				     LeaveWindowMask | PointerMotionMask),
			      GrabModeSync, GrabModeAsync,
			      None, None, CurrentTime );
	    if ( (popupGrabOk = (r == GrabSuccess)) ) {
		XAllowEvents( popup->x11Display(), SyncPointer, CurrentTime );
	    } else {
		XUngrabKeyboard( popup->x11Display(), CurrentTime );
	    }
	}
    }

    // popups are not focus-handled by the window system (the first
    // popup grabbed the keyboard), so we have to do that manually: A
    // new popup gets the focus
    active_window = popup;
    if (active_window->focusWidget())
	active_window->focusWidget()->setFocus();
    else
	active_window->setFocus();
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
	    XUngrabKeyboard( popup->x11Display(), CurrentTime );
	    if ( mouseButtonState != 0 ) {	// mouse release event
		XAllowEvents( popup->x11Display(), AsyncPointer,
			      CurrentTime );
	    } else {				// mouse press event
		mouseButtonPressTime -= 10000;	// avoid double click
		XAllowEvents( popup->x11Display(), ReplayPointer,CurrentTime );
	    }
	    XFlush( popup->x11Display() );
	}
	active_window = 0;
    }
     else {
	// popups are not focus-handled by the window system (the
	// first popup grabbed the keyboard), so we have to do that
	// manually: A popup was closed, so the previous popup gets
	// the focus.
	 active_window = popupWidgets->getLast();
	 if (active_window->focusWidget())
	     active_window->focusWidget()->setFocus();
	 else
	     active_window->setFocus();
     }
}


/*****************************************************************************
  Functions returning the active popup and modal widgets.
 *****************************************************************************/

/*!
  Returns the active popup widget.

  A popup widget is a special top level widget that sets the WType_Popup
  widget flag, e.g. the QPopupMenu widget.  When the application opens a
  popup widget, all events are sent to the popup and normal widgets and
  modal widgets cannot be accessed before the popup widget is closed.

  Only other popup widgets may be opened when a popup widget is shown.
  The popup widgets are organized in a stack.
  This function returns the active popup widget on top of the stack.

  \sa currentModalWidget(), topLevelWidgets()
*/

QWidget *QApplication::activePopupWidget()
{
    return popupWidgets ? popupWidgets->getLast() : 0;
}


/*!
  Returns the active modal widget.

  A modal widget is a special top level widget which is a subclass of
  QDialog that specifies the modal parameter of the constructor to TRUE.
  A modal widget must be finished before the user can continue with other
  parts of the program.

  The modal widgets are organized in a stack.
  This function returns the active modal widget on top of the stack.

  \sa currentPopupWidget(), topLevelWidgets()
*/

QWidget *QApplication::activeModalWidget()
{
    return modal_stack ? modal_stack->getLast() : 0;
}


/*****************************************************************************
  Timer handling; Xlib has no application timer support so we'll have to
  make our own from scratch.

  NOTE: These functions are for internal use. QObject::startTimer() and
	QObject::killTimer() are for public use.
	The QTimer class provides a high-level interface which translates
	timer events into signals.

  qStartTimer( interval, obj )
	Starts a timer which will run until it is killed with qKillTimer()
	Arguments:
	    int interval	timer interval in milliseconds
	    QObject *obj	where to send the timer event
	Returns:
	    int			timer identifier, or zero if not successful

  qKillTimer( timerId )
	Stops a timer specified by a timer identifier.
	Arguments:
	    int timerId		timer identifier
	Returns:
	    bool		TRUE if successful

  qKillTimer( obj )
	Stops all timers that are sent to the specified object.
	Arguments:
	    QObject *obj	object receiving timer events
	Returns:
	    bool		TRUE if successful
 *****************************************************************************/

//
// Internal data structure for timers
//

struct TimerInfo {				// internal timer info
    int	     id;				// - timer identifier
    timeval  interval;				// - timer interval
    timeval  timeout;				// - when to sent event
    QObject *obj;				// - object to receive event
};

typedef QList<TimerInfo> TimerList;	// list of TimerInfo structs

static QBitArray *timerBitVec;			// timer bit vector
static TimerList *timerList	= 0;		// timer list


//
// Internal operator functions for timevals
//

static inline bool operator<( const timeval &t1, const timeval &t2 )
{
    return t1.tv_sec < t2.tv_sec ||
	  (t1.tv_sec == t2.tv_sec && t1.tv_usec < t2.tv_usec);
}

static inline timeval &operator+=( timeval &t1, const timeval &t2 )
{
    t1.tv_sec += t2.tv_sec;
    if ( (t1.tv_usec += t2.tv_usec) >= 1000000 ) {
	t1.tv_sec++;
	t1.tv_usec -= 1000000;
    }
    return t1;
}

static inline timeval operator+( const timeval &t1, const timeval &t2 )
{
    timeval tmp;
    tmp.tv_sec = t1.tv_sec + t2.tv_sec;
    if ( (tmp.tv_usec = t1.tv_usec + t2.tv_usec) >= 1000000 ) {
	tmp.tv_sec++;
	tmp.tv_usec -= 1000000;
    }
    return tmp;
}

static inline timeval operator-( const timeval &t1, const timeval &t2 )
{
    timeval tmp;
    tmp.tv_sec = t1.tv_sec - t2.tv_sec;
    if ( (tmp.tv_usec = t1.tv_usec - t2.tv_usec) < 0 ) {
	tmp.tv_sec--;
	tmp.tv_usec += 1000000;
    }
    return tmp;
}


//
// Internal functions for manipulating timer data structures.
// The timerBitVec array is used for keeping track of timer identifiers.
//

static int allocTimerId()			// find avail timer identifier
{
    int i = timerBitVec->size()-1;
    while ( i >= 0 && (*timerBitVec)[i] )
	i--;
    if ( i < 0 ) {
	i = timerBitVec->size();
	timerBitVec->resize( 4 * i );
	for( int j=timerBitVec->size()-1; j > i; j-- )
	    timerBitVec->clearBit( j );
    }
    timerBitVec->setBit( i );
    return i+1;
}

static void insertTimer( const TimerInfo *ti )	// insert timer info into list
{
    TimerInfo *t = timerList->first();
    int index = 0;
    while ( t && t->timeout < ti->timeout ) {	// list is sorted by timeout
	t = timerList->next();
	index++;
    }
    timerList->insert( index, ti );		// inserts sorted
}

static inline void getTime( timeval &t )	// get time of day
{
    gettimeofday( &t, 0 );
    while ( t.tv_usec >= 1000000 ) {		// NTP-related fix
	t.tv_usec -= 1000000;
	t.tv_sec++;
    }
    while ( t.tv_usec < 0 ) {
	if ( t.tv_sec > 0 ) {
	    t.tv_usec += 1000000;
	    t.tv_sec--;
	} else {
	    t.tv_usec = 0;
	    break;
	}
    }
}

static void repairTimer( const timeval &time )	// repair broken timer
{
    if ( !timerList )				// not initialized
	return;
    timeval diff = watchtime - time;
    register TimerInfo *t = timerList->first();
    while ( t ) {				// repair all timers
	t->timeout = t->timeout - diff;
	t = timerList->next();
    }
}


//
// Timer activation functions (called from the event loop)
//

/*
  Returns the time to wait for the next timer, or null if no timers are
  waiting.
*/

timeval *qt_wait_timer()
{
    static timeval tm;
    bool first = TRUE;
    timeval currentTime;
    if ( timerList && timerList->count() ) {	// there are waiting timers
	getTime( currentTime );
	if ( first ) {
	    if ( currentTime < watchtime )	// clock was turned back
		repairTimer( currentTime );
	    first = FALSE;
	    watchtime = currentTime;
	}
	TimerInfo *t = timerList->first();	// first waiting timer
	if ( currentTime < t->timeout ) {	// time to wait
	    tm = t->timeout - currentTime;
	} else {
	    tm.tv_sec  = 0;			// no time to wait
	    tm.tv_usec = 0;
	}
	return &tm;
    }
    return 0;					// no timers
}

/*
  Activates the timer events that have expired. Returns the number of timers
  (not 0-timer) that were activated.
*/

int qt_activate_timers()
{
    if ( !timerList || !timerList->count() )	// no timers
	return 0;
    bool first = TRUE;
    timeval currentTime;
    int maxcount = timerList->count();
    int n_act = 0;
    register TimerInfo *t;
    while ( maxcount-- ) {			// avoid starvation
	getTime( currentTime );			// get current time
	if ( first ) {
	    if ( currentTime < watchtime )	// clock was turned back
		repairTimer( currentTime );
	    first = FALSE;
	    watchtime = currentTime;
	}
	t = timerList->first();
	if ( !t || currentTime < t->timeout )	// no timer has expired
	    break;
	timerList->take();			// unlink from list
	t->timeout += t->interval;
	if ( t->timeout < currentTime )
	    t->timeout = currentTime + t->interval;
	insertTimer( t );			// relink timer
	if ( t->interval.tv_usec > 0 || t->interval.tv_sec > 0 )
	    n_act++;
	QTimerEvent e( t->id );
	QApplication::sendEvent( t->obj, &e );	// send event
    }
    return n_act;
}


//
// Timer initialization and cleanup routines
//

static void initTimers()			// initialize timers
{
    timerBitVec = new QBitArray( 128 );
    CHECK_PTR( timerBitVec );
    int i = timerBitVec->size();
    while( i-- > 0 )
	timerBitVec->clearBit( i );
    timerList = new TimerList;
    CHECK_PTR( timerList );
    timerList->setAutoDelete( TRUE );
}

static void cleanupTimers()			// cleanup timer data structure
{
    if ( timerList ) {
	delete timerList;
	timerList = 0;
	delete timerBitVec;
	timerBitVec = 0;
    }
}


//
// Main timer functions for starting and killing timers
//

int qStartTimer( int interval, QObject *obj )
{
    if ( !timerList )				// initialize timer data
	initTimers();
    int id = allocTimerId();			// get free timer id
    if ( id <= 0 ||
	 id > (int)timerBitVec->size() || !obj )// cannot create timer
	return 0;
    timerBitVec->setBit( id-1 );		// set timer active
    TimerInfo *t = new TimerInfo;		// create timer
    CHECK_PTR( t );
    t->id = id;
    t->interval.tv_sec  = interval/1000;
    t->interval.tv_usec = (interval%1000)*1000;
    timeval currentTime;
    getTime( currentTime );
    t->timeout = currentTime + t->interval;
    t->obj = obj;
    insertTimer( t );				// put timer in list
    return id;
}

bool qKillTimer( int id )
{
    register TimerInfo *t;
    if ( !timerList || id <= 0 ||
	 id > (int)timerBitVec->size() || !timerBitVec->testBit( id-1 ) )
	return FALSE;				// not init'd or invalid timer
    t = timerList->first();
    while ( t && t->id != id )			// find timer info in list
	t = timerList->next();
    if ( t ) {					// id found
	timerBitVec->clearBit( id-1 );		// set timer inactive
	return timerList->remove();
    }
    else					// id not found
	return FALSE;
}

bool qKillTimer( QObject *obj )
{
    register TimerInfo *t;
    if ( !timerList )				// not initialized
	return FALSE;
    t = timerList->first();
    while ( t ) {				// check all timers
	if ( t->obj == obj ) {			// object found
	    timerBitVec->clearBit( t->id-1 );
	    timerList->remove();
	    t = timerList->current();
	} else {
	    t = timerList->next();
	}
    }
    return TRUE;
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

int translateButtonState( int s )
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
    if ( s & Mod1Mask )
	bst |= Qt::AltButton;
    return bst;
}

bool QETWidget::translateMouseEvent( const XEvent *event )
{
    static bool manualGrab = FALSE;
    QEvent::Type type;				// event parameters
    QPoint pos;
    QPoint globalPos;
    int	   button = 0;
    int	   state;

    if ( sm_blockUserInput ) // block user interaction during session management
	return TRUE;

    if ( event->type == MotionNotify ) { // mouse move
	XEvent *xevent = (XEvent *)event;
	unsigned int xstate = event->xmotion.state;
	while ( XCheckTypedWindowEvent( appDpy, event->xmotion.window, MotionNotify, xevent ) ) {
	    // compress motion events
	    if ( xevent->xmotion.state != xstate ) {
		XPutBackEvent( appDpy, xevent );
		break;
	    }
	}
	type = QEvent::MouseMove;
	pos.rx() = xevent->xmotion.x;
	pos.ry() = xevent->xmotion.y;
	globalPos.rx() = xevent->xmotion.x_root;
	globalPos.ry() = xevent->xmotion.y_root;
	state = translateButtonState( xevent->xmotion.state );
	if ( qt_button_down && (state & (LeftButton |
					 MidButton |
					 RightButton ) ) == 0 )
	    qt_button_down = 0;
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
		if ( isEnabled() ) {
		    QWidget* w = this;
		    while ( w->focusProxy() )
			w = w->focusProxy();
		    if ( w->focusPolicy() & QWidget::ClickFocus ) {
			w->setFocus();
			// inform parent in case we are an embedded window
			QWidget* active_window = topLevelWidget();
			if (active_window && active_window->extra->topextra->embedded) {
			  XClientMessageEvent client_message;
			  client_message.type = ClientMessage;
			  client_message.window = active_window->extra->topextra->parentWinId;
			  client_message.format = 32;
			  client_message.message_type = qt_embedded_window_take_focus;
			  XSendEvent( appDpy, client_message.window, FALSE, NoEventMask,
				      (XEvent*)&client_message );
			}
		    }
		}
		break;
	    case Button4:
	    case Button5:
		// the fancy mouse wheel.

		// take care about grabbing.  We do this here since it
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
		    // may offer a finer-resolution.  A positive delta
		    // indicates forward rotation, a negative one
		    // backward rotation respectively.
		    delta *= 120*(event->xbutton.button == Button4?1:-1);

		    if ( !translateWheelEvent( globalPos.x(), globalPos.y(), delta, state ) ) {
			// we did not accept the wheel event because
			// we did not have focus. If we are embedded,
			// we'll send the event to our parent.

			QWidget* tlw = topLevelWidget();

			if ( tlw && tlw->extra && tlw->extra->topextra &&
			     tlw->extra->topextra->embedded ) {
			    XEvent ev;
			    memset( &ev, 0, sizeof(ev) );
			    ev.xclient.type = ClientMessage;
			    ev.xclient.window = tlw->extra->topextra->parentWinId;
			    ev.xclient.message_type = qt_wheel_event;
			    ev.xclient.format = 32;
			    ev.xclient.data.l[0] = globalPos.x();
			    ev.xclient.data.l[1] = globalPos.y();
			    ev.xclient.data.l[2] = delta;
			    ev.xclient.data.l[3] = state;
			    XSendEvent(qt_xdisplay(), ev.xclient.window,
				       FALSE, NoEventMask, &ev);
			}
		    }
		}
		return TRUE;
	}
	if ( event->type == ButtonPress ) {	// mouse button pressed
	    qt_button_down = findChildWidget( this, pos );	//magic for masked widgets
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
	    mouseButtonPressed = button; 	// save event params for
	    mouseXPos = pos.x();		// future double click tests
	    mouseYPos = pos.y();
	} else {				// mouse button released
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
	QWidget *popupChild  = findChildWidget( popup, pos );
	QWidget *popupTarget = popupChild ? popupChild : popup;

	if (popup != popupOfPopupButtonFocus){
	    popupButtonFocus = 0;
	    popupOfPopupButtonFocus = 0;
	}

	if ( !popupTarget->isEnabled() )
	    return FALSE;

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

	if ( popupButtonFocus ) {
	    QMouseEvent e( type, popupButtonFocus->mapFromGlobal(globalPos),
			   globalPos, button, state );
	    QApplication::sendEvent( popupButtonFocus, &e );
	    if ( releaseAfter ) {
		popupButtonFocus = 0;
		popupOfPopupButtonFocus = 0;
	    }
	} else {
	    QMouseEvent e( type, pos, globalPos, button, state );
	    QApplication::sendEvent( popup, &e );
	}

	if ( releaseAfter )
	    qt_button_down = 0;

	if ( qApp->inPopupMode() ) {			// still in popup mode
	    if ( popupGrabOk )
		XAllowEvents( x11Display(), SyncPointer, CurrentTime );
	} else {
	    if ( type != QEvent::MouseButtonRelease && state != 0 &&
		 QWidget::find((WId)mouseActWindow) ) {
		manualGrab = TRUE;		// need to manually grab
		XGrabPointer( x11Display(), mouseActWindow, FALSE,
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
	if ( !w && qt_button_down )
	    w = qt_button_down;
	if ( w && w != this ) {
	    widget = w;
	    pos = mapToGlobal( pos );
	    pos = w->mapFromGlobal( pos );
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

	QMouseEvent e( type, pos, globalPos, button, state );
	QApplication::sendEvent( widget, &e );
    }
    return TRUE;
}


//
// Wheel event translation
//
bool QETWidget::translateWheelEvent( int global_x, int global_y, int delta, int state )
{
    QWidget* w = this;

    while ( w->focusProxy() )
	w = w->focusProxy();
    if ( w->focusPolicy() == QWidget::WheelFocus )
	w->setFocus();

    // send the event to the widget that has the focus or its ancestors
    w = qApp->focusWidget();
    if (w){
	do {
	    QWheelEvent e( w->mapFromGlobal(QPoint( global_x, global_y)),
			   QPoint(global_x, global_y), delta, state );
	    e.ignore();
	    QApplication::sendEvent( w, &e );
	    if ( e.isAccepted() )
		return TRUE;
	    w = w->isTopLevel()?0:w->parentWidget();
	} while (w);
    }
    return FALSE;
}


//
// Keyboard event translation
//

#define XK_MISCELLANY
#define XK_LATIN1
#include <X11/keysymdef.h>

#ifndef XK_ISO_Left_Tab
#define	XK_ISO_Left_Tab					0xFE20
#endif
static KeySym KeyTbl[] = {			// keyboard mapping table
    XK_Escape,		Qt::Key_Escape,		// misc keys
    XK_Tab,		Qt::Key_Tab,
    XK_ISO_Left_Tab,    Qt::Key_Backtab,
    XK_BackSpace,	Qt::Key_Backspace,
    XK_Return,		Qt::Key_Return,
    XK_Insert,		Qt::Key_Insert,
    XK_KP_Insert,		Qt::Key_Insert,
    XK_Delete,		Qt::Key_Delete,
    XK_KP_Delete,		Qt::Key_Delete,
    XK_Clear,		Qt::Key_Delete,
    XK_Pause,		Qt::Key_Pause,
    XK_Print,		Qt::Key_Print,
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
    XK_KP_Right,		Qt::Key_Right,
    XK_KP_Down,		Qt::Key_Down,
    XK_KP_Prior,		Qt::Key_Prior,
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




bool QETWidget::translateKeyEventInternal( const XEvent *event, int& count,
					   QString& text, int& state,
					   char& ascii, int &code )
{
    QCString chars(64);
    KeySym key = 0;

    if ( !keyDict ) {
	keyDict = new QIntDict<void>( 13 );
	keyDict->setAutoDelete( FALSE );
	textDict = new QIntDict<void>( 13 );
	textDict->setAutoDelete( FALSE );
	qAddPostRoutine( deleteKeyDicts );
    }

    QWidget* tlw = topLevelWidget();

#if defined(NO_XIM)

    count = XLookupString( &((XEvent*)event)->xkey,
			   chars.data(), chars.size(), &key, 0 );

    if ( count == 1 )
	ascii = chars[0];

#else
    QEvent::Type type = (event->type == XKeyPress)
			? QEvent::KeyPress : QEvent::KeyRelease;
    // Implementation for X11R5 and newer, using XIM

    int	       keycode = event->xkey.keycode;
    Status     status;

    if ( type == QEvent::KeyPress ) {
	bool mb=FALSE;
	if ( qt_xim ) {
	    QTLWExtra*  xd = tlw->extraData()?tlw->extraData()->topextra:0;
	    if ( !xd ) {
		tlw->createTLExtra();
		xd = tlw->extraData()->topextra;
	    }
	    if ( xd->xic ) {
		mb=TRUE;
		count = XmbLookupString( (XIC)(xd->xic), &((XEvent*)event)->xkey,
					 chars.data(), chars.size(), &key, &status );
		if ( status == XBufferOverflow ) {
		    chars.resize(count+1);
		    count = XmbLookupString( (XIC)(xd->xic), &((XEvent*)event)->xkey,
					 chars.data(), chars.size(), &key, &status );
		}
	    }
	}
	if ( !mb ) {
	    count = XLookupString( &((XEvent*)event)->xkey,
				   chars.data(), chars.size(), &key, 0 );
	}
	if ( count && !keycode ) {
	    keycode = composingKeycode;
	    composingKeycode = 0;
	}
	if ( key )
	    keyDict->replace( keycode, (void*)key );
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
#endif // !NO_XIM

    state = translateButtonState( event->xkey.state );

    // Commentary in X11/keysymdef says that X codes match ASCII, so it
    // is safe to use the locale functions to process X codes in ISO8859-1.
    //
    // This is mainly for compatibility - applications should not use the
    // Qt keycodes between 128 and 255, but should rather use the
    // QKeyEvent::text().
    //
    if ( key < 128 || key < 256 && (!input_mapper || input_mapper->mibEnum()==4) ) {
	code = isprint((int)key) ? toupper((int)key) : 0; // upper-case key, if known
    } else if ( key >= XK_F1 && key <= XK_F35 ) {
	code = Key_F1 + ((int)key - XK_F1);	// function keys
    } else if ( key >= XK_KP_0 && key <= XK_KP_9){
	code = Key_0 + ((int)key - XK_KP_0);	// numeric keypad keys
    } else {
	int i = 0;				// any other keys
	while ( KeyTbl[i] ) {
	    if ( key == KeyTbl[i] ) {
		code = (int)KeyTbl[i+1];
		break;
	    }
	    i += 2;
	}
	if ( code == Key_Tab &&
	     (state & ShiftButton) == Qt::ShiftButton ) {
	    code = Key_Backtab;
	    chars[0] = 0;
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

    if ( qApp->inPopupMode() ) {			// in popup mode
	if ( popupGrabOk )
	    XAllowEvents( x11Display(), SyncKeyboard, CurrentTime );
    }

    // convert chars (8bit) to text (unicode).
    if ( input_mapper )
	text = input_mapper->toUnicode(chars,count);
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
    QWidget *tlw = topLevelWidget();

    if ( !grab && tlw && tlw->extra && tlw->extra->topextra &&
	 tlw->extra->topextra->embedded ) {
	((XEvent*)event)->xkey.window = tlw->extra->topextra->parentWinId;
	XSendEvent( dpy, tlw->extra->topextra->parentWinId, NoEventMask,
		    FALSE, (XEvent*)event );
	return TRUE;
    }

    if ( !isEnabled() )
	return TRUE;


    QEvent::Type type = (event->type == XKeyPress) ?
	QEvent::KeyPress : QEvent::KeyRelease;
    bool    autor = FALSE;
    QString text;

    translateKeyEventInternal( event, count, text, state, ascii, code );
    bool isAccel = FALSE;
    if (!grab) { // test for accel if the keyboard is not grabbed
	QKeyEvent a( QEvent::AccelAvailable, code, ascii, state, text, FALSE,
		     QMAX(count, int(text.length())) );
	a.ignore();
	QApplication::sendEvent( topLevelWidget(), &a );
	isAccel = a.isAccepted();
    }

    if ( !isAccel && !text.isEmpty() && testWState(WState_CompressKeys) ) {
	// the widget wants key compression so it gets it
	int	codeIntern = -1;
	int	countIntern = 0;
	int	stateIntern;
	char	asciiIntern = 0;
	XEvent	evRelease;
	XEvent	evPress;
	while (1) {
	    QString textIntern;
	    if ( !XCheckTypedWindowEvent(dpy,event->xkey.window,
					 XKeyRelease,&evRelease) )
		break;
	    if ( !XCheckTypedWindowEvent(dpy,event->xkey.window,
					 XKeyPress,&evPress) ) {
		XPutBackEvent(dpy, &evRelease);
		break;
	    }
	    translateKeyEventInternal( &evPress, countIntern, textIntern,
				       stateIntern, asciiIntern, codeIntern);
	    if ( stateIntern == state && !textIntern.isEmpty() ) {
		if (!grab) { // test for accel if the keyboard is not grabbed
		    QKeyEvent a( QEvent::AccelAvailable, codeIntern,
				 asciiIntern, stateIntern, textIntern, FALSE,
				 QMAX(countIntern, int(textIntern.length())) );
		    a.ignore();
		    QApplication::sendEvent( topLevelWidget(), &a );
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
	    autor = nextpress.xkey.time == event->xkey.time;
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
	while (1) {
	    if (!XCheckTypedWindowEvent(dpy,event->xkey.window,XKeyRelease,
					&evRelease) )
		break;
	    if (evRelease.xkey.keycode != event->xkey.keycode ) {
		XPutBackEvent(dpy, &evRelease);
		break;
	    }
	    if (!XCheckTypedWindowEvent(dpy,event->xkey.window,XKeyPress,
					&evPress))
		break;
	    if ( evPress.xkey.keycode != event->xkey.keycode ||
		 evRelease.xkey.time != evPress.xkey.time){
		XPutBackEvent(dpy, &evRelease);
		XPutBackEvent(dpy, &evPress);
		break;
	    }
	    count++;
	    if (!text.isEmpty())
		text += text[0];
	}
    }

    // process accelerates before popups
    QKeyEvent e( type, code, ascii, state, text, autor,
		 QMAX(count, int(text.length())) );
    if ( type == QEvent::KeyPress && !grab ) {
	// send accel event to tlw if the keyboard is not grabbed
	QKeyEvent a( QEvent::Accel, code, ascii, state, text, autor,
		     QMAX(count, int(text.length())) );
	a.ignore();
	QApplication::sendEvent( topLevelWidget(), &a );
	if ( a.isAccepted() )
	    return TRUE;
    }
    return QApplication::sendEvent( this, &e );
}


//
// Paint event translation
//
// When receiving many expose events, we compress them (union of all expose
// rectangles) into one event which is sent to the widget.
// Some X servers send expose events before resize (configure) events.
// We try to remedy that, too.
//

struct PaintEventInfo {
    Window window;
    int	   w, h;
    bool   check;
    int	   config;
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
	    return TRUE;
    } else if ( ev->type == ConfigureNotify && info->check ) {
	XConfigureEvent *c = (XConfigureEvent *)ev;
	if ( c->window == info->window &&
	     (c->width != info->w || c->height != info->h) ) {
	    info->w = c->width;
	    info->h = c->height;
	    info->config++;
	    return TRUE;
	}
    }
    return FALSE;
}

#if defined(Q_C_CALLBACKS)
}
#endif


// declared above: static QList<QScrollInProgress> *sip_list = 0;

void qt_insert_sip( QWidget* scrolled_widget, int dx, int dy )
{
    if ( !sip_list ) {
	sip_list = new QList<QScrollInProgress>;
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

    XSendEvent( appDpy, scrolled_widget->winId(), FALSE, NoEventMask,
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
    QRect  paintRect( event->xexpose.x,	   event->xexpose.y,
		      event->xexpose.width, event->xexpose.height );
    bool   merging_okay = !testWFlags(WPaintClever);
    XEvent xevent;
    PaintEventInfo info;

    info.window = winId();
    info.w	= width();
    info.h	= height();
    info.check	= testWFlags(WType_TopLevel);
    info.config = 0;
    bool should_clip = translateBySips( this, paintRect );

    QRegion paintRegion( paintRect );

    if ( merging_okay ) {
	// WARNING: this is O(number_of_events * number_of_matching_events)
	while ( XCheckIfEvent(x11Display(),&xevent,isPaintOrScrollDoneEvent,
			      (XPointer)&info) &&
		!qApp->x11EventFilter(&xevent)  &&
		!x11Event( &xevent ) ) // send event through filter
	{
	    if ( !info.config ) {
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
	    } // otherwise, discard all up to last config event
	}
    }

    if ( info.config ) {
	XConfigureEvent *c = (XConfigureEvent *)&xevent;
	c->window  = info.window;
	c->event  = info.window;
	c->width  = info.w;
	c->height = info.h;
	translateConfigEvent( (XEvent*)c );
    }

    if ( should_clip ) {
	paintRegion = paintRegion.intersect( rect() );
	if ( paintRegion.isEmpty() )
	    return TRUE;
    }

    QPaintEvent e( paintRegion );
    setWState( WState_InPaintEvent );
    qt_set_paintevent_clipping( this, paintRegion);
    QApplication::sendEvent( this, &e );
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
//
// The problem with ConfigureNotify is that one cannot trust x and y values
// in the xconfigure struct. Top level widgets are reparented by the window
// manager, and (x,y) is sometimes relative to the parent window, but not
// always!  It is safer (but slower) to translate the window coordinates.
//

bool QETWidget::translateConfigEvent( const XEvent *event )
{


    if ( !testWFlags( WType_TopLevel )
	 || testWFlags( WType_Popup )
	 || (!testWFlags( WStyle_DialogBorder ) &&
	     !testWFlags( WStyle_NormalBorder ) )
	 || testWFlags( WType_Desktop ) )
	return TRUE;			// child widget or override_redirect

    clearWState(WState_ConfigPending);

    QSize  newSize( event->xconfigure.width, event->xconfigure.height );
    int x = event->xconfigure.x;
    int y = event->xconfigure.y;

    if (event->xconfigure.send_event ||
	( extra && extra->topextra &&
	  ( extra->topextra->parentWinId == None ||
	    extra->topextra->parentWinId == appRootWin ) ) ) {
	// nothing to do, x and y is correct
    } else {
	Display *dpy = x11Display();
	Window child;
	// ### this slows down display of all top-level widgets, and most
	// ### don't care about the result.  can it be avoided?
	XTranslateCoordinates( dpy, winId(), DefaultRootWindow(dpy),
			       0, 0, &x, &y, &child );
    }


    XEvent otherEvent;
    while ( XCheckTypedWindowEvent( x11Display(),winId(),ConfigureNotify,&otherEvent ) ) {
	if ( qApp->x11EventFilter( &otherEvent ) )
	    break;
	if (x11Event( &otherEvent ) )
	    break;
	newSize.setWidth( otherEvent.xconfigure.width );
	newSize.setHeight( otherEvent.xconfigure.height );
	if ( otherEvent.xconfigure.send_event ) {
	    x = otherEvent.xconfigure.x;
	    y = otherEvent.xconfigure.y;
	}
    }

    QPoint newPos( x, y );
    QRect  r = geometry();
    if ( newSize != size() ) {			// size changed
	QSize oldSize = size();
	r.setSize( newSize );
	setCRect( r );
	if ( isVisible() ) {
	    QResizeEvent e( newSize, oldSize );
	    QApplication::sendEvent( this, &e );
	} else {
	    QResizeEvent * e = new QResizeEvent( newSize, oldSize );
	    QApplication::postEvent( this, e );
	}
	// visibleRect() is not really useful yet, since isTopLevel()
	// is always TRUE here
	if ( !testWFlags( WNorthWestGravity ) )
	    repaint( visibleRect(), !testWFlags(WResizeNoErase) );
    }
    if ( newPos != geometry().topLeft() ) {
	QPoint oldPos = pos();
	r.moveTopLeft( newPos );
	setCRect( r );
	if ( isVisible() ) {
	    QMoveEvent e( newPos, oldPos );
	    QApplication::sendEvent( this, &e );
	} else {
	    QMoveEvent * e = new QMoveEvent( newPos, oldPos );
	    QApplication::postEvent( this, e );
	}
    }
    return TRUE;
}


//
// Close window event translation.
//
// This class is a friend of QApplication because it needs to emit the
// lastWindowClosed() signal when the last top level widget is closed.
//

bool QETWidget::translateCloseEvent( const XEvent * )
{
    return close(FALSE);
}


/*
  If \w is embedded and the tab-focus wrapped, tell the embedding
  window about it so it may continue in its own focus chain.
 */
void qt_xembed_tab_focus( QWidget* w, bool next ) {
    ( (QETWidget*) w )->embeddedWindowTabFocus( next );
}

/*
  If \w is embedded and the tab-focus wrapped, tell the embedding
  window about it so it may continue in its own focus chain.
 */
void QETWidget::embeddedWindowTabFocus( bool next )
{
    QWidget* tlw = topLevelWidget();

    if ( tlw && tlw->extra && tlw->extra->topextra &&
	 tlw->extra->topextra->embedded ) {
	XEvent ev;
	memset( &ev, 0, sizeof(ev) );
	ev.xclient.type = ClientMessage;
	ev.xclient.window = tlw->extra->topextra->parentWinId;
	ev.xclient.message_type = qt_embedded_window_tab_focus;
	ev.xclient.format = 32;
	ev.xclient.data.l[0] = next;
	XSendEvent(qt_xdisplay(), ev.xclient.window,
		   FALSE, NoEventMask, &ev);
    }
}


/*!
  Sets the text cursor's flash time to \a msecs milliseconds.  The
  flash time is the time requried to display, invert and restore the
  caret display: A full flash cycle.  Usually, the text cursor is
  displayed for \a msecs/2 millisecnds, then hidden for \a msecs/2
  milliseconds.

  Under windows, calling this function sets the double click
  interval for all windows.

  \sa cursorFlashTime()
 */
void  QApplication::setCursorFlashTime( int msecs )
{
    cursor_flash_time = msecs;
}


/*!
  Returns the text cursor's flash time in milliseconds. The flash time is the
  time requried to display, invert and restore the caret display.

  The default value is 1000 milliseconds. Under Windows, the control
  panel value is used.

  Widgets should not cache this value since it may vary any time the
  user changes the global desktop settings.

  \sa setCursorFlashTime()
 */
int QApplication::cursorFlashTime()
{
    return cursor_flash_time;
}

/*!
  Sets the time limit that distinguishes a double click from two
  consecutive mouse clicks to \a ms milliseconds.

  Under windows, calling this function sets the double click
  interval for all windows.

  \sa doubleClickInterval()
*/

void QApplication::setDoubleClickInterval( int ms )
{
    mouse_double_click_time = ms;
}


/*!
  Returns the maximum duration for a double click.

  The default value is 400 milliseconds. Under Windows, the control
  panel value is used.

  \sa setDoubleClickInterval()
*/

int QApplication::doubleClickInterval()
{
    return mouse_double_click_time;
}



/*****************************************************************************
  Session management support (-D QT_SM_SUPPORT to enable it)
 *****************************************************************************/

#ifndef QT_SM_SUPPORT

class QSessionManagerData
{
public:
    QStringList restartCommand;
    QStringList discardCommand;
    QString sessionId;
    QSessionManager::RestartHint restartHint;
};

QSessionManager::QSessionManager( QApplication * app, QString &session )
    : QObject( app, "session manager" )
{
    d = new QSessionManagerData;
    d->sessionId = session;
    d->restartHint = RestartIfRunning;
}

QSessionManager::~QSessionManager()
{
    delete d;
}

QString QSessionManager::sessionId() const
{
    return d->sessionId;
}

HANDLE QSessionManager::handle() const
{
    return 0;
}

bool QSessionManager::allowsInteraction()
{
    return TRUE;
}

bool QSessionManager::allowsErrorInteraction()
{
    return TRUE;
}

void QSessionManager::release()
{
}

void QSessionManager::cancel()
{
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

void QSessionManager::setProperty( const QString&, const QString&)
{
}

void QSessionManager::setProperty( const QString&, const QStringList& )
{
}

bool QSessionManager::isPhase2() const
{
    return FALSE;
}

void QSessionManager::requestPhase2()
{
}

#else // QT_SM_SUPPORT


#include <X11/SM/SMlib.h>

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
static bool sm_waitingForPhase2;
static bool sm_waitingForInteraction;
static bool sm_isshutdown;
static bool sm_shouldbefast;
static bool sm_phase2;

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
static void sm_performSaveYourself( QSessionManager* );


static void resetSmState()
{
    sm_waitingForPhase2 = FALSE;
    sm_waitingForInteraction = FALSE;
    sm_interactionActive = FALSE;
    sm_interactStyle = SmInteractStyleNone;
    sm_smActive = FALSE;
    sm_blockUserInput = FALSE;
    sm_isshutdown = FALSE;
    sm_shouldbefast = FALSE;
    sm_phase2 = FALSE;
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


static void sm_saveYourselfCallback( SmcConn smcConn, SmPointer clientData,
				  int saveType, Bool shutdown , int interactStyle, Bool fast)
{
    if (smcConn != smcConnection )
	return;
    sm_cancel = FALSE;
    sm_smActive = TRUE;
    sm_isshutdown = shutdown;
    if ( sm_isshutdown )
	sm_blockUserInput = TRUE;
    sm_saveType = saveType;
    sm_interactStyle = interactStyle;
    sm_shouldbefast = fast;
    sm_performSaveYourself( (QSessionManager*) clientData );
    if ( !sm_isshutdown ) // we cannot expect a confirmation message in that case
	resetSmState();
}

static void sm_performSaveYourself( QSessionManager* sm )
{
    // tell the session manager about our program in best POSIX style
    sm_setProperty( SmProgram, QString( qApp->argv()[0] ) );
    // tell the session manager about our user as well.
    sm_setProperty( SmUserID, QString::fromLatin1( getlogin() ) );

    // generate a restart and discard command that makes sense
    QStringList restart;
    restart  << qApp->argv()[0] << "-session" << sm->sessionId();
    sm->setRestartCommand( restart );
    QStringList discard;
    sm->setDiscardCommand( discard );

    switch ( sm_saveType ) {
    case SmSaveBoth:
	qApp->commitData( *sm );
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

    if ( sm_phase2 ) {
	SmcRequestSaveYourselfPhase2( smcConnection, sm_saveYourselfPhase2Callback, (SmPointer*) sm );
    }
    else {
	// close eventual interaction monitors and cancel the
	// shutdown, if required.  Note that we can only cancel when
	// performing a shutdown, it does not work for checkpoints
	if ( sm_interactionActive ) {
	    SmcInteractDone( smcConnection, sm_isshutdown && sm_cancel);
	    sm_interactionActive = FALSE;
	}
	else if ( sm_cancel && sm_isshutdown ) {
	    if ( sm->allowsErrorInteraction() ) {
		SmcInteractDone( smcConnection, TRUE );
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
    sm_performSaveYourself( (QSessionManager*) clientData );
}


void QSmSocketReceiver::socketActivated(int)
{
    Bool reply_set;
    IceProcessMessages( SmcGetIceConnection( smcConnection ), 0, &reply_set );
}

const char *QSmSocketReceiver::className() const
{
    return "QSmSocketReceiver";
}

QMetaObject *QSmSocketReceiver::metaObj = 0;


static QMetaObjectInit init_QSmSocketReceiver(&QSmSocketReceiver::staticMetaObject);

void QSmSocketReceiver::initMetaObject()
{
    if ( metaObj )
	return;
    if ( strcmp(QObject::className(), "QObject") != 0 )
	badSuperclassWarning("QSmSocketReceiver","QObject");

    staticMetaObject();
}

QString QSmSocketReceiver::tr(const char* s)
{
    return qApp->translate("QSmSocketReceiver",s);
}

void QSmSocketReceiver::staticMetaObject()
{
    if ( metaObj )
	return;
    QObject::staticMetaObject();

    typedef void(QSmSocketReceiver::*m1_t0)(int);
    m1_t0 v1_0 = &QSmSocketReceiver::socketActivated;
    QMetaData *slot_tbl = QMetaObject::new_metadata(1);
    slot_tbl[0].name = "socketActivated(int)";
    slot_tbl[0].ptr = *((QMember*)&v1_0);
    metaObj = QMetaObject::new_metaobject(
	"QSmSocketReceiver", "QObject",
	slot_tbl, 1,
	0, 0 );
}

class QSessionManagerData
{
public:
    QStringList restartCommand;
    QStringList discardCommand;
    QString sessionId;
    QSessionManager::RestartHint restartHint;
};

QSessionManager::QSessionManager( QApplication * app, QString &session )
    : QObject( app, "session manager" )
{
    d = new QSessionManagerData;
    d->sessionId = session;
    d->restartHint = RestartIfRunning;

    resetSmState();
    char cerror[256];
    char* myId = 0;
    char* prevId = (char*)session.latin1(); // we know what we are doing

    SmcCallbacks cb;
    cb.save_yourself.callback = sm_saveYourselfCallback;
    cb.save_yourself.client_data = (SmPointer) this;
    cb.die.callback = sm_dieCallback;
    cb.die.client_data = (SmPointer) this;
    cb.save_complete.callback = sm_saveCompleteCallback;
    cb.save_complete.client_data = (SmPointer) this;
    cb.shutdown_cancelled.callback = sm_shutdownCancelledCallback;
    cb.shutdown_cancelled.client_data = (SmPointer) this;

    // avoid showing a warning message below
    if (!::getenv("SESSION_MANAGER") )
	return;

    smcConnection = SmcOpenConnection( 0, 0, 1, 0,
				       SmcSaveYourselfProcMask |
				       SmcDieProcMask |
				       SmcSaveCompleteProcMask |
				       SmcShutdownCancelledProcMask,
				       &cb,
				       prevId,
				       &myId,
				       256,
				       (char*)&cerror );

    d->sessionId = QString::fromLatin1( myId );
    ::free( myId ); // it was allocated by C
    session = d->sessionId;

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

HANDLE QSessionManager::handle() const
{
    return (HANDLE) smcConnection;
}


bool QSessionManager::allowsInteraction()
{
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
	SmcInteractDone( smcConnection, FALSE );
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

void QSessionManager::setProperty( const QString& name, const QString& value)
{
    SmPropValue prop;
    prop.length = value.length();
    prop.value = (SmPointer) value.utf8().data();
    sm_setProperty( name.latin1(), SmARRAY8, 1, &prop );
}

void QSessionManager::setProperty( const QString& name, const QStringList& value)
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
    return sm_phase2;
}

void QSessionManager::requestPhase2()
{
    sm_phase2 = TRUE;
}


#endif // QT_SM_SUPPORT
