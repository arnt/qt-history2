/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qapplication.cpp#297 $
**
** Implementation of QApplication class
**
** Created : 931107
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
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

#include "qapplication.h"
#include "qobjectlist.h"
#include "qobjectdict.h"
#include "qwidget.h"
#include "qwidgetlist.h"
#include "qwidgetintdict.h"
#include "qwindowsstyle.h"
#include "qmotifstyle.h"
#include "qplatinumstyle.h"
#include "qcdestyle.h"
#include "qtranslator.h"
#include "qtextcodec.h"
#include "qpngio.h"
#include "qsessionmanager.h"

#if defined(QT_MAKEDLL)
#define QApplication QBaseApplication
#endif


// BEING REVISED: hanord (QSessionManager)

/*!
  \class QApplication qapplication.h
  \brief The QApplication class manages the GUI application's control flow and main settings.

  \ingroup environment

  It contains the main event loop, where all events from the window
  system and other sources are processed and dispatched.  It also
  handles the application initialization and finalization, and
  provides session management.  Finally, it handles most system-wide
  and application-wide settings.

  For any GUI application that uses Qt, there is precisely one
  QApplication object, no matter whether the application has 0, 1, 2
  or even more windows at the moment.

  This object (you can access is using the global variable \c qApp)
  does a great many things, most importantly: <ul>

  <li> It initializes the application to the user's desktop settings
  like palette(), font() or the doubleClickInterval(). It keeps track
  of these properties in case the user changes the desktop globally
  in some kind of control panel.

  <li> It performs event handling, meaning that it receives events
  from the underlying window system and sends them to the destination
  widgets.  By using sendEvent() and postEvent() you can send your own
  events to widgets.

  <li> It parses common command line arguments and sets its internal
  state accordintly. See the constructor documentation below for more
  details about this.

  <li> It defines the application's look and feel, which is
  encapsulated in a QStyle object. This can be changed during runtime
  with setStyle().

  <li> It specifies how the application is to allocate colors.
  See setColorSpec() for details.

  <li> It specifies the default text encoding (see setDefaultCodec() )
  and provides localization of strings that are visible to the user via
  translate().

  <li> It provides some magic objects like the desktop() and the
  clipboard().

  <li> It knows about the application's windows, and lets you ask
  which widget is at a certain position using widgetAt(), lets you
  closeAllWindows(), gives you a list of topLevelWidgets(), etc.

  <li> It manages the application's mouse cursor handling,
  see setOverrideCursor() and setGlobalMouseTracking().

  <li> On the X window system, it provides functions to flush and sync
  the communication stream, see flushX() and syncX().

  <li> It provides support to implement sophisticated \link
  session.html session management \endlink. This makes it possible
  for applications to terminate gracefully when the user logs out, to
  cancel a shutdown process if termination isn't possible and even to
  preserve the entire application state for a future session. See
  isSessionRestored(), sessionId() and commitData() and saveState()
  for details.

  </ul>

  The <a href="simple-application.html">Application walktrough
  example</a> contains a typical complete main() that does the usual
  things with QApplication.

  Since the QApplication object does so much initialization, it is
  absolutely necessary to create it before any other objects related
  to the user interface are created.

  Since it also deals with common command line arguments, it is
  usually a good idea to create it \e before any interpretation or
  modification of \c argv is done in the application itself.  (Note
  also that for X11, setMainWidget() may change the main widget
  according to the \c -geometry option.  To preserve this
  functionality, you must set your defaults before setMainWidget() and
  any overrides after.)

  <strong>Groups of functions:</strong>
  <ul>
     <li> System settings:
	desktopSettingsAware(),
	setDesktopSettingsAware(),
	cursorFlashTime(),
	setCursorFlashTime(),
	doubleClickInterval(),
	setDoubleClickInterval(),
	wheelScrollLines(),
	setWheelScrollLines(),
	palette(),
	setPalette(),
	font(),
	setFont(),
	fontMetrics().

     <li> Event handling:
	exec(),
	processEvents(),
	processOneEvent(),
	enter_loop(),
	exit_loop(),
	exit(),
	quit().
	sendEvent(),
	postEvent(),
	sendPostedEvents(),
	removePostedEvents(),
	notify(),
	x11EventFilter(),
	x11ProcessEvent(),
	winEventFilter().

     <li> GUI Styles:
	style(),
	setStyle(),
	polish().

     <li> Color usage:
	colorSpec(),
	setColorSpec().

     <li> Text handling:
	setDefaultCodec(),
	installTranslator(),
	removeTranslator()
	translate().

     <li> Certain widgets:
	mainWidget(),
	setMainWidget(),
	allWidgets(),
	topLevelWidgets(),
	desktop(),
	activePopupWidget(),
	activeModalWidget(),
	clipboard(),
	focusWidget(),
	activeWindow(),
	widgetAt().

     <li> Advanced cursor handling:
	hasGlobalMouseTracking(),
	setGlobalMouseTracking(),
	overrideCursor(),
	setOverrideCursor(),
	restoreOverrideCursor().

     <li> X Window System synchronization:
	flushX(),
	syncX().

     <li> Session management:
	isSessionRestored(),
	sessionId(),
	commitData(),
	saveState()

     <li> Misc:
	closeAllWindows(),
	startingUp(),
	closingDown(),
  </ul>

  <strong>Non-GUI programs</strong><br> While Qt is not optimized or
  designed for writing non-GUI programs, it's possible to use <a
  href="tools.html">some of its classes</a> without creating a
  QApplication.  This can be useful if you wish to share code between
  a non-GUI server and a GUI client.

  \header qnamespace.h
  \header qwindowdefs.h
  \header qglobal.h
*/

/*
  The qt_init() and qt_cleanup() functions are implemented in the
  qapplication_xyz.cpp file.
*/

void qt_init( int *, char ** );
void qt_cleanup();
#if defined(_WS_X11_)
void qt_init( Display* dpy );
#endif

QNonBaseApplication *qApp = 0;			// global application object

QStyle   *QApplication::app_style      = 0;	// default application style
int	  QApplication::app_cspec = QApplication::NormalColor;
QPalette *QApplication::app_pal	       = 0;	// default application palette
QFont	 *QApplication::app_font       = 0;	// default application font
QCursor	 *QApplication::app_cursor     = 0;	// default application cursor
int	  QApplication::app_tracking   = 0;	// global mouse tracking
bool	  QApplication::is_app_running = FALSE;	// app starting up if FALSE
bool	  QApplication::is_app_closing = FALSE;	// app closing down if TRUE
bool	  QApplication::app_exit_loop  = FALSE;	// flag to exit local loop
int	  QApplication::loop_level     = 0;	// event loop level
QWidget	 *QApplication::main_widget    = 0;	// main application widget
QWidget	 *QApplication::focus_widget   = 0;	// has keyboard input focus
QWidget	 *QApplication::active_window  = 0;	// toplevel with keyboard focus
bool	  QApplication::obey_desktop_settings = TRUE;	// use winsys resources
int	  QApplication::cursor_flash_time = 1000;	// text caret flash time
int	  QApplication::mouse_double_click_time = 400;	// mouse dbl click limit
int	  QApplication::wheel_scroll_lines = 3;		// number of lines to scroll
bool	  qt_is_gui_used;
static int drag_time = 250;
static int drag_distance = 4;

// Default application palettes and fonts (per widget type)
QAsciiDict<QPalette> *QApplication::app_palettes = 0;
QAsciiDict<QFont>    *QApplication::app_fonts = 0;

QWidgetList *QApplication::popupWidgets = 0;	// has keyboard input focus

static bool	   makeqdevel	 = FALSE;	// developer tool needed?
//static QDeveloper* qdevel = 0;		// developer tool
static QWidget	  *desktopWidget = 0;		// root window widget
static QTextCodec *default_codec = 0;		// root window widget

QWidgetList * qt_modal_stack=0; 		// stack of modal widgets

// Definitions for posted events
struct QPostEvent {
    QPostEvent( QObject *r, QEvent *e ) { receiver=r; event=e; }
   ~QPostEvent()			{ delete event; }
    QObject  *receiver;
    QEvent   *event;
};

typedef QList<QPostEvent>	  QPostEventList;
typedef QListIterator<QPostEvent> QPostEventListIt;
static QPostEventList *postedEvents = 0;	// list of posted events

static void cleanupPostedEvents();


QPalette *qt_std_pal = 0;

void qt_create_std_palette()
{
    if ( qt_std_pal )
	delete qt_std_pal;

    QColor standardLightGray( 192, 192, 192 );
    QColor light( 255, 255, 255 );
    QColor dark( standardLightGray.dark( 150 ) );
    QColorGroup std_act( Qt::black, standardLightGray,
			 light, dark, Qt::gray,
			 Qt::black, Qt::white );
    QColorGroup std_dis( Qt::darkGray, standardLightGray,
			 light, dark, Qt::gray,
			 Qt::darkGray, std_act.background() );
    QColorGroup std_inact( Qt::black, standardLightGray,
			   light, dark, Qt::gray,
			   Qt::black, Qt::white );
    qt_std_pal = new QPalette( std_act, std_dis, std_inact );
}

static void qt_fix_tooltips()
{
    // No resources for this yet (unlike on Windows).
    QColorGroup cg( Qt::black, QColor(255,255,220),
		    QColor(96,96,96), Qt::black, Qt::black,
		    Qt::black, QColor(255,255,220) );
    QPalette pal( cg, cg, cg );
    QApplication::setPalette( pal, TRUE, "QTipLabel");
}


void QApplication::process_cmdline( int* argcptr, char ** argv )
{
    // process platform-indep command line
    if ( !qt_is_gui_used )
	return;

    int argc = *argcptr;
    int i, j;

    j = 1;
    for ( i=1; i<argc; i++ ) {
	if ( argv[i] && *argv[i] != '-' ) {
	    argv[j++] = argv[i];
	    continue;
	}
	QCString arg = argv[i];
	if ( arg == "-qdevel" || arg == "-qdebug") {
	    makeqdevel = !makeqdevel;
	} else if ( stricmp(arg, "-style=windows") == 0 ) {
	    setStyle( new QWindowsStyle );
	} else if ( stricmp(arg, "-style=motif") == 0 ) {
	    setStyle( new QMotifStyle );
	} else if ( stricmp(arg, "-style=platinum") == 0 ) {
	    setStyle( new QPlatinumStyle );
	} else if ( stricmp(arg, "-style=cde") == 0 ) {
	    setStyle( new QCDEStyle );
	} else if ( strcmp(arg,"-style") == 0 && i < argc-1 ) {
	    QCString s = argv[++i];
	    s = s.lower();
	    if ( s == "windows" )
		setStyle( new QWindowsStyle );
	    else if ( s == "motif" )
		setStyle( new QMotifStyle );
	    else if ( s == "platinum" )
		setStyle( new QPlatinumStyle );
	    else if ( s == "cde" )
		setStyle( new QCDEStyle );
	} else if ( strcmp(arg,"-session") == 0 && i < argc-1 ) {
	    QCString s = argv[++i];
	    if ( !s.isEmpty() ) {
		session_id = QString::fromLatin1( s );
		is_session_restored = TRUE;
	    }
	} else
	    argv[j++] = argv[i];
    }
    *argcptr = j;
}

/*!
  Initializes the window system and constructs an application object
  with the command line arguments \a argc and \a argv.

  The global \c qApp pointer refers to this application object. Only
  one application object should be created.

  This application object must be constructed before any \link
  QPaintDevice paint devices\endlink (includes widgets, pixmaps, bitmaps
  etc.)

  Notice that \a argc and \a argv might be changed.  Qt removes
  command line arguments that it recognizes.  \a argc and \a argv are
  can be accessed later by \c qApp->argc() and \c qApp->argv().	 The
  documentation for argv() contains a detailed description of how to
  process command line arguments.

  Qt debugging options (not available if Qt was compiled with the
  NO_DEBUG flag defined):
  <ul>
  <li> \c -nograb, tells Qt to never grab the mouse or the keyboard.
  <li> \c -dograb (only under X11), running under a debugger can cause
  an implicit -nograb, use -dograb to override.
  <li> \c -sync (only under X11), switches to synchronous mode for
	debugging.
  </ul>

  See <a href="debug.html">Debugging Techniques</a> for a more
  detailed explanation.

  All Qt programs automatically support the following command line options:
  <ul>
  <li> \c -style= \e style, sets the application GUI style. Possible values
       are \c motif, \c windows, and \c platinum.
  <li> \c -session= \e session, restores the application from an earlier
       \link session.html session \endlink.
  <li> \c -qdevel activates the Application Builder window, which allows
       run-time inspection of the program.
  <li> \c -qtranslate activates the Application Translator window, which allows
       translation of the texts shown in the program.
  </ul>

  The X11 version of Qt also supports some traditional X11
  command line options:
  <ul>
  <li> \c -display \e display, sets the X display (default is $DISPLAY).
  <li> \c -geometry \e geometry, sets the client geometry of the
	\link setMainWidget() main widget\endlink.
  <li> \c -fn or \c -font \e font, defines the application font.
  <li> \c -bg or \c -background \e color, sets the default background color
	and an application palette (light and dark shades are calculated).
  <li> \c -fg or \c -foreground \e color, sets the default foreground color.
  <li> \c -btn or \c -button \e color, sets the default button color.
  <li> \c -name \e name, sets the application name.
  <li> \c -title \e title, sets the application title (caption).
  <li> \c -visual \c TrueColor, forces the application to use a TrueColor visual
       on an 8-bit display.
  <li> \c -ncols \e count, limits the number of colors allocated in the
       color cube on a 8-bit display, if the application is using the
       \c QApplication::ManyColor color specification.  If \e count is
       216 then a 6x6x6 color cube is used (ie. 6 levels of red, 6 of green,
       and 6 of blue); for other values, a cube
       approximately proportional to a 2x3x1 cube is used.
  <li> \c -cmap, causes the application to install a private color map
       on an 8-bit display.
  </ul>

  \sa argc(), argv()
*/

//######### BINARY COMPATIBILITY constructor
QApplication::QApplication( int &argc, char **argv )
{
    qt_is_gui_used = TRUE;
    init_precmdline();
    static char *empty = (char*)"";
    if ( argc == 0 || argv == 0 ) {
	argc = 0;
	argv = &empty;
    }
    qt_init( &argc, argv );   // Must be called before initialize()
    process_cmdline( &argc, argv );

    initialize( argc, argv );
}


/*!
  Constructs an application object with the command line arguments \a
  argc and \a argv. If \a GUIenabled is TRUE, a normal application is
  constructed, otherwise a non-GUI application is created.

  Set \a GUIenabled to FALSE for programs without a graphical user
  interface that should be able to run without a window system.

  On X11, the window system is initialized if \a GUIenabled is TRUE.
  If \a GUIenabled is FALSE, the application does not connect to the
  X-server.

  On Windows, currently the window system is always initialized,
  regardless of the value of GUIenabled. This may change in future
  versions of Qt.
  
  The following example shows how to create an application that
  uses a graphical interface when available.
\code  
  int main( int argc, char **argv )
  {
#ifdef _WS_X11_
    bool useGUI = getenv( "DISPLAY" ) != 0;
#else
    bool useGUI = TRUE;
#endif
    QApplication app(argc, argv, useGUI);

    if ( useGUI ) {
       //start GUI version
       ...
    } else {
       //start non-GUI version
       ...
    }
    return app.exec();
  }
\endcode
*/

QApplication::QApplication( int &argc, char **argv, bool GUIenabled  )
{
    qt_is_gui_used = GUIenabled;
    init_precmdline();
    static char *empty = (char*)"";
    if ( argc == 0 || argv == 0 ) {
	argc = 0;
	argv = &empty;
    }
    qt_init( &argc, argv );   // Must be called before initialize()
    process_cmdline( &argc, argv );
    initialize( argc, argv );
}


#if defined(_WS_X11_)
// note: #ifdef'ed stuff is NOT documented.
/*!
  Create an application, given an already open display.  This is
  available only on X11.
*/

QApplication::QApplication( Display* dpy )
{
    qt_is_gui_used = TRUE;
    init_precmdline();
    // ... no command line.
    qt_init( dpy );
    initialize( 0, 0 );
}

#endif // _WS_X11_


void QApplication::init_precmdline()
{
    translators = 0;
    is_session_restored = FALSE;
#if defined(CHECK_STATE)
    if ( qApp )
	qWarning( "QApplication: There should be max one application object" );
#endif
    qApp = (QNonBaseApplication*)this;
}

/*!
  Initializes the QApplication object, called from the constructors.
*/

void QApplication::initialize( int argc, char **argv )
{
    app_argc = argc;
    app_argv = argv;
    quit_now = FALSE;
    quit_code = 0;
    QWidget::createMapper(); // create widget mapper
    (void) palette();  // trigger creation of application palette
    is_app_running = TRUE; // no longer starting up


    if (!app_style) {
#if defined(_WS_WIN_)
	app_style = new QWindowsStyle; // default style for Windows
#elif defined(_WS_X11_)
	app_style = new QMotifStyle; // default style for X Windows
#elif defined(_WS_MAC_)
	app_style = new QPlatinumStyle;
#else
#error "Toto... I have a feeling we're not in Kansas anymore."
#endif
    }

    qInitPngIO();

    app_style->polish( *app_pal );
    app_style->polish( qApp ); //##### wrong place, still inside the qapplication constructor...grmbl....
#if 0
    if ( makeqdevel ) {
	qdevel = new QDeveloper;
	qdevel->show();
    }
#endif
    // connect to the session manager
    session_manager = new QSessionManager( qApp, session_id );
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

  \sa activeModalWidget(), topLevelWidgets()
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

  \sa activePopupWidget(), topLevelWidgets()
*/

QWidget *QApplication::activeModalWidget()
{
    if ( !qt_modal_stack )
	return 0;
    QWidget* w = qt_modal_stack->getFirst();
    if ( w->testWState( WState_Modal ) )
	return w;
    // find the real one
    QWidgetListIt it( *qt_modal_stack );
    while ( it.current() ) {
	if ( it.current()->testWState( WState_Modal ) )
	    return it.current();
	--it;
    }
    return 0;
}



/*!
  Cleans up any window system resources that were allocated by this
  application.  Sets the global variable \c qApp to null. Unlike
  former versions of Qt the destructor does \e not delete all
  remaining widgets.
*/

QApplication::~QApplication()
{
    delete desktopWidget;
    desktopWidget = 0;
    is_app_closing = TRUE;
    QWidget::destroyMapper();
    delete qt_std_pal;
    qt_std_pal = 0;
    delete app_pal;
    app_pal = 0;
    delete app_font;
    app_font = 0;
    delete app_palettes;
    app_palettes = 0;
    delete app_fonts;
    app_fonts = 0;
    delete app_style;
    app_style = 0;
    delete app_cursor;
    app_cursor = 0;
    qt_cleanup();
    delete objectDict;
    objectDict = 0;
    qApp = 0;
    is_app_running = FALSE;
    // Cannot delete codecs until after QDict destructors
    // QTextCodec::deleteAllCodecs()
}


/*!
  \fn int QApplication::argc() const
  Returns the number of command line arguments.

  The documentation for argv() contains a detailed description of how to
  process command line arguments.

  \sa argv(), QApplication::QApplication()
*/

/*!
  \fn char **QApplication::argv() const
  Returns the command line argument vector.

  \c argv()[0] is the program name, \c argv()[1] is the first argument and
  \c argv()[argc()-1] is the last argument.

  A QApplication object is constructed by passing \e argc and \e argv from
  the \c main() function.  Some of the arguments may be recognized as Qt
  options removed from the argument vector.  For example, the X11
  version of Qt knows about \c -display, \c -font and a few more options.

  Example:
  \code
    // showargs.cpp - displays program arguments in a list box

    #include <qapplication.h>
    #include <qlistbox.h>

    int main( int argc, char **argv )
    {
	QApplication a( argc, argv );
	QListBox b;
	a.setMainWidget( &b );
	for ( int i=0; i<a.argc(); i++ )	// a.argc() == argc
	    b.insertItem( a.argv()[i] );	// a.argv()[i] == argv[i]
	b.show();
	return a.exec();
    }
  \endcode

  If you run <tt>showargs -display unix:0 -font 9x15bold hello
  world</tt> under X11, the list box contains the three strings
  "showargs", "hello" and "world".

  \sa argc(), QApplication::QApplication()
*/


/*!
  \fn QStyle& QApplication::style()
  Returns the style object of the application.
  \sa setStyle(), QStyle
*/

/*!
  Sets the application GUI style to \a style.  Ownership of the style
  object is not transfered.

  When switching application styles, the color palette is set back to
  the initital colors or the system defaults. This is necessary since
  certain styles have to adapt the color palette to be fully
  style-guide compliant.

  \sa style(), QStyle, setPalette(), desktopSettingsAware()
*/

void QApplication::setStyle( QStyle *style )
{
    QStyle* old = app_style;
    app_style = style;

    if ( startingUp() ) {
	delete old;
	return;
    }


    // clean up the old style
    if (old) {
	if ( is_app_running && !is_app_closing ) {
	    QWidgetIntDictIt it( *((QWidgetIntDict*)QWidget::mapper) );
	    register QWidget *w;
	    while ( (w=it.current()) ) {		// for all widgets...
		++it;
		if ( !w->testWFlags(WType_Desktop) &&	// except desktop
		     w->testWState(WState_Polished) ) { // has been polished
		    old->unPolish(w);
		}
	    }
	}
	old->unPolish( qApp );
    }

    // take care of possible palette requirements of certain gui
    // styles. Do it before polishing the application since the style
    // might call QApplication::setStyle() itself
    if ( !qt_std_pal )
	qt_create_std_palette();
    QPalette tmpPal = *qt_std_pal;
    app_style->polish( tmpPal );
    if ( tmpPal != *app_pal )
	setPalette( tmpPal, TRUE );

    // initialize the application with the new style
    app_style->polish( qApp );

    // re-polish existing widgets if necessary
    if (old) {
	if ( is_app_running && !is_app_closing ) {
	    QWidgetIntDictIt it( *((QWidgetIntDict*)QWidget::mapper) );
	    register QWidget *w;
	    while ( (w=it.current()) ) {		// for all widgets...
		++it;
		if ( !w->testWFlags(WType_Desktop) ) {	// except desktop
		    if ( w->testWState(WState_Polished) )
			app_style->polish(w);		// repolish
		    w->styleChange( *old );
		    if ( w->isVisible() ){
			w->update();
		    }
		}
	    }
	}
    }
}


#if 1  /* OBSOLETE */

QApplication::ColorMode QApplication::colorMode()
{
    return (QApplication::ColorMode)app_cspec;
}

void QApplication::setColorMode( QApplication::ColorMode mode )
{
    app_cspec = mode;
}
#endif


/*!
  Returns the color specification.
  \sa QApplication::setColorSpec()
 */

int QApplication::colorSpec()
{
    return app_cspec;
}

/*!
  Sets the color specification for the application to \a spec.

  The color specification controls how your application allocates colors
  whn run on a display with a limited amount of colors, i.e. 8 bit / 256
  color displays.

  The color specification must be set before you create the QApplication
  object.

  The choices are:
  <ul>
  <li> \c QApplication::NormalColor.
    This is the default color allocation strategy. Use this choice if
    your application uses buttons, menus, texts and pixmaps with few
    colors. With this choice, the application uses system global
    colors. This works fine for most applications under X11, but on
    Windows machines it may cause dithering of non-standard colours.
  <li> \c QApplication::CustomColor.
    Use this choice if your application needs a small number of custom
    colors. On X11, this choice is the same as NormalColor. On Windows, Qt
    creates a Windows palette, and allocates colors in it on demand.
  <li> \c QApplication::ManyColor.
    Use this choice if your application is very color hungry
    (e.g. it wants thousands of colors).
    Under X11 the effect is: <ul>
      <li> For 256-color displays which have at best a 256 color true color
	    visual, the default visual is used, and colors are allocated
	    from a color cube.
	    The color cube is the 6x6x6 (216 color) "Web palette", but the
	    number of colors can be changed by the \e -ncols option.
	    The user can force the application to use the true color visual by
	    the \link QApplication::QApplication() -visual \endlink
	    option.
      <li> For 256-color displays which have a true color visual with more
	    than 256 colors, use that visual.  Silicon Graphics X
	    servers have this feature, for example.  They provide an 8
	    bit visual by default but can deliver true color when
	    asked.
    </ul>
    On Windows, Qt creates a Windows palette, and fills it with a color cube.
  </ul>

  Be aware that the CustomColor and ManyColor choices may lead to colormap
  flashing: The foreground application gets (most) of the available
  colors, while the background windows will look less good.

  Example:
  \code
  int main( int argc, char **argv )
  {
      QApplication::setColorSpec( QApplication::ManyColor );
      QApplication a( argc, argv );
      ...
  }
  \endcode

  QColor provides more functionality for controlling color allocation and
  freeing up certain colors. See QColor::enterAllocContext() for more
  information.

  To see what mode you end up with, you can call QColor::numBitPlanes()
  once the QApplication object exists.  A value greater than 8 (typically
  16, 24 or 32) means true color.

  The color cube used by Qt are all those colors with red, green, and blue
  components of either 0x00, 0x33, 0x66, 0x99, 0xCC, or 0xFF.

  \sa colorSpec(), QColor::numBitPlanes(), QColor::enterAllocContext() */

void QApplication::setColorSpec( int spec )
{
#if defined(CHECK_STATE)
    if ( qApp ) {
	qWarning( "QApplication::setColorSpec: This function must be "
		 "called before the QApplication object is created" );
    }
#endif
    app_cspec = spec;
}


/*!
  Returns a pointer to the default application palette.	 There is
  always an application palette, i.e. the returned pointer is
  guaranteed to be non-null.

  If a widget is passed as argument, the default palette for the
  widget's class is returned. This may or may not be the application
  palette, but in most cases there won't be a special palette for
  certain types of widgets. An exception is the popup menu under
  Windows, when the user defined a special background color for menus
  in the display settings.

  \sa setPalette(), QWidget::palette()
*/

QPalette QApplication::palette(const QWidget* w)
{
#if defined(CHECK_STATE)
    if ( !qApp ) {
	qWarning( "QApplication::palette: This function can only be "
		  "called after the QApplication object has been created" );
    }
#endif
    if ( !app_pal ) {
	if ( !qt_std_pal )
	    qt_create_std_palette();
	app_pal = new QPalette( *qt_std_pal );
	qt_fix_tooltips();
    }

    if ( w && app_palettes ) {
	QPalette* wp = app_palettes->find( w->className() );
	if ( wp )
	    return *wp;
	QAsciiDictIterator<QPalette> it( *app_palettes );
	const char* name;
	while ( (name=it.currentKey()) != 0 ) {
	    if ( w->inherits( name ) )
		return *it.current();
	    ++it;
	}
    }
    return *app_pal;
}



/*!
  Changes the default application palette to \a palette.  If \a informWidgets
  is TRUE, then existing widgets are informed about the change and thus
  may adjust themselves to the new application setting.  Otherwise the
  change only affects newly created widgets.  If \a className is
  passed, the change applies only to classes that inherit \a className
  (as reported by QObject::inherits()).

  The palette may be changed according to the current GUI style in
  QStyle::polish().

  \sa QWidget::setPalette(), palette(), QStyle::polish()
*/

void QApplication::setPalette( const QPalette &palette, bool informWidgets,
			       const char* className )
{
    QPalette pal = palette;
    if ( !startingUp() )
	qApp->style().polish( pal );	// NB: non-const reference

    bool all = FALSE;
    if ( !className ) {
	if ( !app_pal ) {
	    app_pal = new QPalette( pal );
	    CHECK_PTR( app_pal );
	} else {
	    *app_pal = pal;
	}
	all = app_palettes != 0;
	delete app_palettes;
	app_palettes = 0;
	qt_fix_tooltips();		// ### Doesn't (always) work
    } else {
	if ( !app_palettes ) {
	    app_palettes = new QAsciiDict<QPalette>;
	    CHECK_PTR( app_palettes );
	    app_palettes->setAutoDelete( TRUE );
	}
	app_palettes->insert( className, new QPalette( pal ) );
    }
    if ( informWidgets && is_app_running && !is_app_closing ) {
	QCustomEvent e( QEvent::ApplicationPaletteChange, 0 );
	QWidgetIntDictIt it( *((QWidgetIntDict*)QWidget::mapper) );
	register QWidget *w;
	while ( (w=it.current()) ) {		// for all widgets...
	    ++it;
	    if ( all || (!className && w->isTopLevel() ) || w->inherits(className) ) // matching class
		sendEvent( w, &e );
	}
    }
}

/*!
  Returns the default font for a widget. Basically this function uses the
  class name of the widget to get a font for it.

  If \a w is 0 the default application font is returned.

  \sa setFont(), fontMetrics(), QWidget::font()
*/

QFont QApplication::font( const QWidget *w )
{
    if ( w && app_fonts ) {
	QFont* wf = app_fonts->find( w->className() );
	if ( wf )
	    return *wf;
	QAsciiDictIterator<QFont> it( *app_fonts );
	const char* name;
	while ( (name=it.currentKey()) != 0 ) {
	    if ( w->inherits( name ) )
		return *it.current();
	    ++it;
	}
    }
    if ( !app_font ) {
	app_font = new QFont( "Helvetica" );
	CHECK_PTR( app_font );
    }
    return *app_font;
}

/*!  Changes the default application font to \a font.  If \a
  informWidgets is TRUE, then existing widgets are informed about the
  change and thus may adjust themselves to the new application
  setting.  Otherwise the change only affects newly created widgets.
  If \a className is passed, the change applies only to classes that
  inherit \a className (as reported by QObject::inherits()).

  On application start-up, the default font depends on the window
  system.  It can vary both with window system version and with
  locale.  This function lets you override it.  Note that overriding
  it may be a bad idea, for example some locales need extra-big fonts
  to support their special characters.

  \sa font(), fontMetrics(), QWidget::setFont()
*/

void QApplication::setFont( const QFont &font, bool informWidgets,
			    const char* className )
{
    bool all = FALSE;
    if ( !className ) {
	if ( !app_font ) {
	    app_font = new QFont( font );
	    CHECK_PTR( app_font );
	} else {
	    *app_font = font;
	}
	all = app_fonts != 0;
	delete app_fonts;
	app_fonts = 0;
    } else {
	if (!app_fonts){
	    app_fonts = new QAsciiDict<QFont>;
	    CHECK_PTR( app_fonts );
	    app_fonts->setAutoDelete( TRUE );
	}
	QFont* fnt = new QFont(font);
	CHECK_PTR( fnt );
	app_fonts->insert(className, fnt);
    }
    if ( informWidgets && is_app_running && !is_app_closing ) {
	QCustomEvent e( QEvent::ApplicationFontChange, 0 );
	QWidgetIntDictIt it( *((QWidgetIntDict*)QWidget::mapper) );
	register QWidget *w;
	while ( (w=it.current()) ) {		// for all widgets...
	    ++it;
	    if ( all || (!className && w->isTopLevel() ) || w->inherits(className) ) // matching class
		sendEvent( w, &e );
	}
    }
}


/*!
  Polishing of widgets.

  Usually widgets call this automatically when they are polished.  It
  may be used to do some style-based central customization of widgets.

  Note that you are not limited to public functions of QWidget.
  Instead, based on meta information like QObject::className()
  you are able to customize any kind of widgets.

  \sa QStyle::polish(), QWidget::polish(), setPalette(), setFont()
*/

void QApplication::polish( QWidget *w )
{

#if 0
    if ( qdevel && w->isTopLevel() )
	qdevel->addTopLevelWidget(tlw);
#endif

    app_style->polish( w );
}


/*!
  Returns a list of the top level widgets in the application.

  The list is created using new and must be deleted by the caller.

  The list is \link QList::isEmpty() empty \endlink if there are no
  top level widgets.

  Note that some of the top level widgets may be hidden, for example
  all the popup menus of a menubar.

  Example:
  \code
    //
    // Shows all hidden top level widgets.
    //
    QWidgetList	 *list = QApplication::topLevelWidgets();
    QWidgetListIt it( *list );	// iterate over the widgets
    while ( it.current() ) {	// for each top level widget...
	if ( !it.current()->isVisible() )
	    it.current()->show();
	++it;
    }
    delete list;		// delete the list, not the widgets
  \endcode

  \warning Delete the list away as soon you have finished using it.
  The widgets in the list may be deleted by someone else at any time.

  \sa allWidgets(), QWidget::isTopLevel(), QWidget::isVisible(),
      QList::isEmpty()
*/

QWidgetList *QApplication::topLevelWidgets()
{
    return QWidget::tlwList();
}

/*!
  Returns a list of all the widgets in the application.

  The list is created using new and must be deleted by the caller.

  The list is \link QList::isEmpty() empty \endlink if there are no
  widgets.

  Note that some of the widgets may be hidden.

  Example:
  \code
    //
    // Updates all widgets.
    //
    QWidgetList	 *list = QApplication::allWidgets();
    QWidgetListIt it( *list );		// iterate over the widgets
    while ( it.current() ) {		// for each top level widget...
	it.current()->update();
	++it;
    }
    delete list;			// delete the list, not the widgets
  \endcode

  The QWidgetList class is defined in the qwidcoll.h header file.

  \warning
  Delete the list away as soon you have finished using it.
  You can get in serious trouble if you for instance try to access
  a widget that has been deleted.

  \sa topLevelWidgets(), QWidget::isVisible(), QList::isEmpty(),
*/

QWidgetList *QApplication::allWidgets()
{
    return QWidget::wList();
}

/*!
  \fn QWidget *QApplication::focusWidget() const
  Returns the application widget that has the keyboard input focus, or null
  if no application widget has the focus.
  \sa QWidget::setFocus(), QWidget::hasFocus(), activeWindow()
*/

/*!
  \fn QWidget *QApplication::activeWindow() const

  Returns the application toplevel window that has the keyboard input
  focus, or null if no application window has the focus. Note that
  there might be an activeWindow even if there is no focusWidget, if
  no widget in that window accepts key events.

  \sa QWidget::setFocus(), QWidget::hasFocus(), focusWidget()
*/

/*!
  Returns display (screen) font metrics for the application font.
  \sa font(), setFont(), QWidget::fontMetrics(), QPainter::fontMetrics()
*/

QFontMetrics QApplication::fontMetrics()
{
    return desktop()->fontMetrics();
}


/*!
  Tells the application to exit with a return code.

  After this function has been called, the application leaves the main
  event loop and returns from the call to exec(). The exec() function
  returns \a retcode.

  By convention, \a retcode 0 means success, any non-zero value indicates
  an error.

  Note that unlike the C library exit function, this function \e does
  returns to the caller - it is event processing that stops.

  \sa quit(), exec()
*/

void QApplication::exit( int retcode )
{
    if ( !qApp )				// no global app object
	return;
    if ( ((QApplication*)qApp)->quit_now )	// don't overwrite quit code...
	return;
    ((QApplication*)qApp)->quit_code = retcode;	// here
    ((QApplication*)qApp)->quit_now = TRUE;
    ((QApplication*)qApp)->app_exit_loop = TRUE;
}


/*!
  Tells the application to exit with return code 0 (success).
  Equivalent to calling QApplication::exit( 0 ).

  This function is a \link metaobjects.html slot\endlink, i.e. you
  may connect any signal to activate quit().

  Example:
  \code
    QPushButton *quitButton = new QPushButton( "Quit" );
    connect( quitButton, SIGNAL(clicked()), qApp, SLOT(quit()) );
  \endcode

  \sa exit(), aboutToQuit()
*/

void QApplication::quit()
{
    QApplication::exit( 0 );
}


/*!
  A convenience function that closes all toplevel windows.

  The function is particularly useful for applications with many
  toplevel windows. It could for example be connected to a "Quit"
  entry in the file menu as shown in the following code example:

  \code
    // the "Quit" menu entry should try to close all windows
    QPopupMenu* file = new QPopupMenu( this );
    file->insertItem( tr("&Quit"), qApp, SLOT(closeAllWindows()), CTRL+Key_Q );

    // when the last window was closed, the application should quit
    connect( qApp, SIGNAL( lastWindowClosed() ), qApp, SLOT( quit() ) );
  \endcode

  The windows are closed in random order, until one window does not
  accept the close event.

  \sa QWidget::close(), QWidget::closeEvent(), lastWindowClosed(),
  quit(), topLevelWidgets(), QWidget::isTopLevel()

 */
void QApplication::closeAllWindows()
{
    QWidgetList *list = QApplication::topLevelWidgets();
    bool did_close = TRUE;
    QWidget* w = list->first();
    while ( did_close && w ) {
	if ( !w->testWState( WState_ForceHide ) ) {
	    did_close = w->close();
	    delete list;
	    list = QApplication::topLevelWidgets();
	    w = list->first();
	}
	else
	    w = list->next();
    }
    delete list;
}


/*!
  \fn void QApplication::lastWindowClosed()

  This signal is emitted when the user has closed the last remaining
  top level window.

  The signal is very useful when your application has many top level
  widgets but no main widget. You can then connect it to the quit() slot.

  For convenience, transient toplevel widgets such as popup menus and
  dialogs are omitted.

  \sa mainWidget(), topLevelWidgets(), QWidget::isTopLevel(), QWidget::close()
*/

/*!
  \fn void QApplication::aboutToQuit()

  This signal is emitted when the application is about to quit the
  main event loop.  This may happen either after a call to quit() from
  inside the applicaton or when the users shuts down the entire
  desktop session.

  The signal is particularly useful if your application has to do some
  last-second cleanups. Note that no user interaction is possible at
  this state.

  \sa quit()
*/


/*!
  \fn void QApplication::guiThreadAwake()

  This signal is emitted when the GUI threads is about to process a cycle
  of the event loop.

  \sa wakeUpGuiThread()
*/

/*!
  \fn bool QApplication::sendEvent( QObject *receiver, QEvent *event )

  Sends an event directly to a receiver, using the notify() function.
  Returns the value that was returned from the event handler.

  \sa postEvent(), notify()
*/

/*!
  Sends \a event to \a receiver: <code>receiver->event( event )</code>
  Returns the value that is returned from the receiver's event handler.

  Reimplementing this virtual function is one of five ways to process
  an event: <ol> <li> Reimplementing this function.  Very powerful,
  you get \e complete control, but of course only one subclass can be
  qApp.

  <li> Installing an event filter on qApp.  Such an event filter gets
  to process all events for all widgets, so it's just as powerful as
  reimplementing notify(), and in this way it's possible to have more
  than one application-global event filter.  Global event filter get
  to see even mouse events for \link QWidget::isEnabled() disabled
  widgets, \endlink and if \link setGlobalMouseTracking() global mouse
  tracking \endlink is enabled, mouse move events for all widgets.

  <li> Reimplementing QObject::event() (as QWidget does).  If you do
  this you get tab key-presses, and you get to see the events before
  any widget-specific event filters.

  <li> Installing an event filter on the object.  Such an even filter
  gets all the events except Tab and Shift-Tab key presses.

  <li> Finally, reimplementing paintEvent(), mousePressEvent() and so
  on.  This is the normal, easiest and least powerful way. </ol>

  \sa QObject::event(), installEventFilter()
*/

bool QApplication::notify( QObject *receiver, QEvent *event )
{
    // no events are delivered after ~QApplication has started
    if ( is_app_closing )
	return FALSE;

    if ( receiver == 0 ) {			// serious error
#if defined(CHECK_NULL)
	qWarning( "QApplication::notify: Unexpected null receiver" );
#endif
	return FALSE;
    }

#if 0
    if ( qdevel && event->type() == QEvent::Reparent
	 && receiver->isWidgetType()
	 && ((QWidget*)receiver)->isTopLevel() )
	qdevel->addTopLevelWidget( (QWidget*)receiver );
#endif


    if ( receiver->pendEvent && event->type() == QEvent::ChildRemoved ) {
	// if this is a child remove event an the child insert hasn't been
	// dispatched yet, kill that insert and return.
	QObject * c = ((QChildEvent*)event)->child();
	QPostEventListIt it( *postedEvents );
	QPostEvent * pe;
	while( ( pe = it.current()) != 0 ) {
	    ++it;
	    if ( pe->event && pe->receiver == receiver &&
		 pe->event->type() == QEvent::ChildInserted &&
		 ((QChildEvent*)pe->event)->child() == c ) {
		postedEvents->take( postedEvents->findRef( pe ) );
		pe->event->posted = FALSE;
		delete pe;
		return TRUE;
	    }
	}
    }

    if ( eventFilters ) {
	QObjectListIt it( *eventFilters );
	register QObject *obj;
	while ( (obj=it.current()) != 0 ) {	// send to all filters
	    ++it;				//   until one returns TRUE
	    if ( obj->eventFilter(receiver,event) )
		return TRUE;
	}
    }

    // throw away mouse events to disabled widgets
    if ( event->type() <= QEvent::MouseMove &&
	 event->type() >= QEvent::MouseButtonPress &&
	 ( receiver->isWidgetType() &&
	   !((QWidget *)receiver)->isEnabled() ) )
	return FALSE;

    // throw away any mouse-tracking-only mouse events
    if ( event->type() == QEvent::MouseMove &&
	 (((QMouseEvent*)event)->state()&QMouseEvent::MouseButtonMask) == 0 &&
	 ( receiver->isWidgetType() &&
	   !((QWidget *)receiver)->hasMouseTracking() ) )
	return TRUE;

    return receiver->event( event );
}


/*!
  Returns TRUE if an application object has not been created yet.
  \sa closingDown()
*/

bool QApplication::startingUp()
{
    return !is_app_running;
}

/*!
  Returns TRUE if the application objects are being destroyed.
  \sa startingUp()
*/

bool QApplication::closingDown()
{
    return is_app_closing;
}


/*!
  Processes pending events, for 3 seconds or until there
  are no more events to process, then return.

  You can call this function occasionally when your program is busy doing a
  long operation (e.g. copying a file).

  \sa processOneEvent(), exec(), QTimer
*/

void QApplication::processEvents()
{
    processEvents( 3000 );
}

/*!
  Waits for an event to occur, processes it, then returns.

  This function is useful for adapting Qt to situations where the event
  processing must be grafted into existing program loops.  Beware
  that using this function in new applications may be an indication
  of design problems.

  \sa processEvents(), exec(), QTimer
*/

void QApplication::processOneEvent()
{
    processNextEvent(TRUE);
}


#if !defined(_WS_X11_)

// The doc and X implementation of these functions is in qapplication_x11.cpp

void QApplication::flushX()	{}		// do nothing

void QApplication::syncX()	{}		// do nothing

#endif



/*!\obsolete
  Sets the color used to mark selections in windows style for all widgets
  in the application. Will repaint all widgets if the color is changed.

  The default color is \c darkBlue.
  \sa winStyleHighlightColor()
*/

void QApplication::setWinStyleHighlightColor( const QColor &c )
{
    QPalette p( palette() );
    p.setColor( QColorGroup::Highlight, c );
    setPalette( p, TRUE);
}


/*!\obsolete
  Returns the color used to mark selections in windows style.
  \sa setWinStyleHighlightColor()
*/
const QColor& QApplication::winStyleHighlightColor()
{
    return palette().normal().highlight();
}


/*!
  \fn Qt::WindowsVersion QApplication::winVersion()

  Returns the version of the Windows operating system running:

  <ul>
  <li> \c Qt::WV_95 Windows 95.
  <li> \c Qt::WV_98 Windows 98.
  <li> \c Qt::WV_NT Windows NT (NT4).
  <li> \c Qt::WV_2000 Windows 2000 (NT5).
  </ul>

  Note that this function is implemented for the Windows version
  of Qt only.
*/


/*!
  Adds \a mf to the list of message files to be used for
  localization.  Message files are searched starting with the most
  recently added file.

  \sa removeTranslator() translate() QObject::tr()
*/

void QApplication::installTranslator( QTranslator * mf )
{
    if ( !translators )
	translators = new QList<QTranslator>;
    if ( mf )
	translators->insert( 0, mf );
}


/*!
  Removes \a mf from the list of message files used by this
  application.  Does not, of course, delete mf.

  \sa installTranslator() translate(), QObject::tr()
*/

void QApplication::removeTranslator( QTranslator * mf )
{
    if ( !translators || !mf )
	return;
    translators->first();
    while( translators->current() && translators->current() != mf )
	translators->next();
    translators->take();
}


/*!
  If the literal quoted text in the program is not in the Latin1
  encoding, this function can be used to
  set the appropriate encoding.  For example, software developed
  by Korean programmers might use eucKR for all the text in the program,
  in which case main() would be:

  \code
    main(int argc, char** argv)
    {
	QApplication app(argc, argv);
	... install any additional codecs ...
	app.setDefaultCodec( QTextCodec::codecForName("eucKR") );
	...
    }
  \endcode

  Note that this is \e not the way to select the encoding that the
  \e user has chosen.  For example, to convert an application containing
  literal English strings to Korean, all that is needed is for the English
  strings to be passed through tr() and for translation files to be
  loaded.  For details of internationalization, see the \link i18n.html
  Qt Internationalization documentation\endlink.

  Note also that some Qt built-in classes call tr() with various
  strings.  These strings are in English, so for a full translation, a
  codec would be required for these strings.
*/

void QApplication::setDefaultCodec( QTextCodec* codec )
{
    default_codec = codec;
}

/*!
  Returns the default codec (see setDefaultCodec()).
  Returns 0 by default (no codec).
*/

QTextCodec* QApplication::defaultCodec() const
{
    return default_codec;
}

/*!
  Returns the best available translation for \a key in \a scope, by
  querying the installed messages files.  The message file that was
  installed last is asked first.

  QObject::tr() offers a more convenient way to use this functionality.

  \a scope is typically a class name (e.g. \c MyDialog) and \a is
  either English text or a short marker text, if the output text will
  be very long (as for help texts).

  If none of the message files contain a translation for \a key in \a
  scope, this function returns \a key.

  This function is not virtual, but you can add alternative translation
  techniques by installing subclasses of QTranslator.

  \sa QObject::tr() installTranslator() removeTranslator() QTranslator
*/

QString QApplication::translate( const char * scope, const char * key ) const
{
    if ( !key )
	return QString::null;
    // scope can be null, for global stuff

    if ( translators ) {
	QListIterator<QTranslator> it( *translators );
	QTranslator * mf;
	QString result;
	while( (mf=it.current()) != 0 ) {
	    ++it;
	    result = mf->find( scope, key );
	    if ( !result.isNull() )
		return result;
	}
    }
    if ( default_codec ) {
	return default_codec->toUnicode(key);
    } else {
	return QString::fromLatin1(key);
    }
}


/*****************************************************************************
  QApplication management of posted events
 *****************************************************************************/

//see also notify(), which does the removal of ChildInserted when ChildRemoved.

/*!
  Stores the event in a queue and returns immediately.

  The event must be allocated on the heap, as it is deleted when the event
  has been posted.

  When control returns to the main event loop, all events that are
  stored in the queue will be sent using the notify() function.

  \sa sendEvent()
*/

void QApplication::postEvent( QObject *receiver, QEvent *event )
{
    if ( !postedEvents ) {			// create list
	postedEvents = new QList<QPostEvent>;
	CHECK_PTR( postedEvents );
	postedEvents->setAutoDelete( TRUE );
	qAddPostRoutine( cleanupPostedEvents );
    }
    if ( receiver == 0 ) {
#if defined(CHECK_NULL)
	qWarning( "QApplication::postEvent: Unexpected null receiver" );
#endif
	return;
    }

    receiver->pendEvent = TRUE;
    event->posted = TRUE;
    postedEvents->append( new QPostEvent(receiver,event) );
}


/*!  Dispatches all posted events. */
void QApplication::sendPostedEvents()
{
    if ( !postedEvents )
	return;
    int abortAfter = 16*postedEvents->count();
    QPostEventListIt it( * postedEvents );
    QPostEvent *pe;
    while ( (pe=it.current()) ) {
	++it;
	if ( pe->event &&
	     ( pe->event->type() == QEvent::LayoutHint ||
	       pe->event->type() == QEvent::ChildInserted ||
	       pe->event->type() == QEvent::Move ||
	       pe->event->type() == QEvent::Resize ||
	       pe->event->type() == QEvent::Paint ) ) {
	    // uglehack: get rid of this sort of event now, by calling
	    // the more-specific function
	    sendPostedEvents( pe->receiver, pe->event->type() );
	} else {
	    postedEvents->take( postedEvents->findRef( pe ) );
	    QApplication::sendEvent( pe->receiver, pe->event );
	    pe->event->posted = FALSE;
	    delete pe;
	}
	if ( abortAfter-- < 1 )
	    return; // if a posted events generates another, don't freeze.
    }
}



/*##############

  The following was in the documentation. The code did not implement it.

  If \a receiver is 0, all objects get their events.  If \a event_type is
  0, all types of events are dispatched.

*/
/*!
  Immediately dispatches all events which have been previously enqueued
  with QApplication::postEvent() and which are for the object \a receiver
  and have the \a event_type.

  Some event compression may occur.  Note that events from the
  window system are \e not dispatched by this function.
*/

void QApplication::sendPostedEvents( QObject *receiver, int event_type )
{
    if ( receiver == 0 ) {			// serious error
#if defined(CHECK_NULL)
	qWarning( "QApplication::sendPostedEvents: Unexpected null receiver" );
#endif
	return;
    }
    
    if ( !receiver->pendEvent || !postedEvents )
	return;
    QPostEventListIt it(*postedEvents);
    QPostEvent *pe;

    // For accumulating compressed events
    QPoint oldpos, newpos;
    QSize oldsize, newsize;
    QRegion paintRegion;
    QRegion erasePaintRegion;
    bool first=TRUE;
    bool pendEvent = FALSE;
    receiver->pendEvent = FALSE;

    while ( (pe = it.current()) ) {
	++it;

	if ( pe->event && pe->receiver == receiver ) {
	    if ( pe->event->type() != event_type ) {
		pendEvent = TRUE;
	    } else {
		postedEvents->take( postedEvents->findRef( pe ) );
		pe->event->posted = FALSE;
		switch ( event_type ) {
		case QEvent::Move:
		    if ( first ) {
			oldpos = ((QMoveEvent*)pe->event)->oldPos();
			first = FALSE;
		    }
		    newpos = ((QMoveEvent*)pe->event)->pos();
		    break;
		case QEvent::Resize:
		    if ( first ) {
			oldsize = ((QResizeEvent*)pe->event)->oldSize();
			first = FALSE;
		    }
		    newsize = ((QResizeEvent*)pe->event)->size();
		    break;
		case QEvent::LayoutHint:
		    first = FALSE;
		    break;
		case QEvent::Paint:
		    if ( ((QPaintEvent*)pe->event)->erased() )
			erasePaintRegion = erasePaintRegion.unite(  ((QPaintEvent*)pe->event)->region() );
		    else
			paintRegion = paintRegion.unite(  ((QPaintEvent*)pe->event)->region() );
		    first = FALSE;
		    break;
		default:
		    sendEvent( receiver, pe->event );
		}
		delete pe;
	    }
	}
    }

    if ( !first ) {
	if ( event_type == QEvent::LayoutHint ) {
	    QEvent e( QEvent::LayoutHint );
	    sendEvent( receiver, &e );
	} else if ( event_type == QEvent::Move ) {
	    QMoveEvent e(newpos, oldpos);
	    sendEvent( receiver, &e );
	} else if ( event_type == QEvent::Resize ) {
	    QResizeEvent e(newsize, oldsize);
	    sendEvent( receiver, &e );
	} else if ( event_type == QEvent::Paint ) {
	    if ( receiver->isWidgetType() && ((QWidget*)receiver)->isVisible() ) {
		QWidget* w = (QWidget*)receiver;
		if ( !erasePaintRegion.isEmpty() )
		    w->repaint( erasePaintRegion, TRUE );
		if ( !paintRegion.isEmpty() )
		    w->repaint( paintRegion, FALSE );
	    }
	}
    }
    
    receiver->pendEvent = receiver->pendEvent || pendEvent;
}


static void cleanupPostedEvents()		// cleanup list
{
    delete postedEvents;
    postedEvents = 0;
}


/*!
  Removes all events posted using postEvent() for \a receiver.

  The events are \e not dispatched, simply removed from the queue.
  You should never need to call this function.
*/

void QApplication::removePostedEvents( QObject *receiver )
{
    if ( !postedEvents || !receiver || !receiver->pendEvent )
	return;

    QPostEventListIt it( *postedEvents );
    QPostEvent * pe;
    while( (pe = it.current()) != 0 ) {
	++it;
	if ( pe->receiver == receiver ) {
	    postedEvents->take( postedEvents->findRef( pe ) );
	    pe->event->posted = FALSE;
	    delete pe;
	}
    }
}


/*!
  Removes \a event from the queue of posted events, and emits a
  warning message if appropriate.
*/

void QApplication::removePostedEvent( QEvent *  event )
{
    if ( !event || !event->posted || !postedEvents )
	return;

    QPostEventListIt it( *postedEvents );
    QPostEvent * pe;
    while( (pe = it.current()) != 0 ) {
	++it;
	if ( pe->event == event ) {
	    postedEvents->take( postedEvents->findRef( pe ) );
	    event->posted = FALSE;
#if defined(DEBUG)
	    const char *n;
	    switch ( event->type() ) {
	    case QEvent::Timer:
		n = "Timer";
		break;
	    case QEvent::MouseButtonPress:
		n = "MouseButtonPress";
		break;
	    case QEvent::MouseButtonRelease:
		n = "MouseButtonRelease";
		break;
	    case QEvent::MouseButtonDblClick:
		n = "MouseButtonDblClick";
		break;
	    case QEvent::MouseMove:
		n = "MouseMove";
		break;
	    case QEvent::Wheel:
		n = "Wheel";
		break;
	    case QEvent::KeyPress:
		n = "KeyPress";
		break;
	    case QEvent::KeyRelease:
		n = "KeyRelease";
		break;
	    case QEvent::FocusIn:
		n = "FocusIn";
		break;
	    case QEvent::FocusOut:
		n = "FocusOut";
		break;
	    case QEvent::Enter:
		n = "Enter";
		break;
	    case QEvent::Leave:
		n = "Leave";
		break;
	    case QEvent::Paint:
		n = "Paint";
		break;
	    case QEvent::Move:
		n = "Move";
		break;
	    case QEvent::Resize:
		n = "Resize";
		break;
	    case QEvent::Create:
		n = "Create";
		break;
	    case QEvent::Destroy:
		n = "Destroy";
		break;
	    case QEvent::Close:
		n = "Close";
		break;
	    case QEvent::Quit:
		n = "Quit";
		break;
	    default:
		n = "<other>";
		break;
	    }
	    qWarning("QEvent: Warning: %s event deleted while posted to %s %s",
		     n,
		     pe->receiver ? pe->receiver->className() : "null ",
		     pe->receiver ? pe->receiver->name() : "object" );
	    // note the beautiful uglehack if !pe->receiver :)
#endif
	    delete pe;
	    return;
	}
    }
}

/*!\internal

  Sets the active window as a reaction on a system event. Call this
  from the platform specific event handlers.

  It sets the activeWindow() and focusWidget() attributes and sends
  proper WindowActivate/WindowDeactivate and FocusIn/FocusOut events
  to all appropriate widgets.

  \sa activeWindow()
 */
void QApplication::setActiveWindow( QWidget* act )
{
    QWidget* window = act?act->topLevelWidget():0;

    if ( active_window == window )
	return;

    QWidget* old_active = active_window;

    // first the activation / deactivation events
    if ( old_active ) {
	active_window = 0;
	QCustomEvent e( QEvent::WindowDeactivate, 0 );
	QApplication::sendEvent( old_active, &e );
    }
    active_window = window;
    if ( active_window ) {
	QCustomEvent e( QEvent::WindowActivate, 0 );
	QApplication::sendEvent( active_window, &e );
    }

    QWidgetIntDictIt it( *((QWidgetIntDict*)QWidget::mapper) );
    register QWidget *w;
    while ( (w=it.current()) ) {		// for all widgets...
	++it;
	if ( (w->topLevelWidget() == old_active || w->topLevelWidget()==active_window )
	     && w->palette().active() != w->palette().inactive() )
	    w->update();
    }

    // then focus events
    QFocusEvent::setReason( QFocusEvent::ActiveWindow );
    if ( !active_window && focus_widget ) {
	QFocusEvent out( QEvent::FocusOut );
	QWidget *tmp = focus_widget;
	focus_widget = 0;
	QApplication::sendEvent( tmp, &out );
    } else if ( active_window ) {
	QWidget *w = active_window->focusWidget();
	if ( w )
	    w->setFocus();
	else
	    active_window->focusNextPrevChild( TRUE );
    }
    QFocusEvent::resetReason();
}



/*!
  Returns the desktop widget (also called the root window).

  The desktop widget is useful for obtaining the size of the screen.
  It can also be used to draw on the desktop.

  \code
    QWidget *d = QApplication::desktop();
    int w=d->width();			// returns screen width
    int h=d->height();			// returns screen height
    d->setBackgroundColor( red );	// makes desktop red
  \endcode
*/

QWidget *QApplication::desktop()
{
    if ( !desktopWidget ||			// not created yet
	 !desktopWidget->testWFlags( WType_Desktop ) ) { // recreated away
	desktopWidget = new QWidget( 0, "desktop", WType_Desktop );
	CHECK_PTR( desktopWidget );
    }
    return desktopWidget;
}


/*!
  By default, Qt will try to get the current standard colors, fonts
  etc. from the underlying window system's desktop settings (resources),
  and use them for all relevant widgets. This behavior can be switched off
  by calling this function with \a on set to FALSE.

  This static function must be called before creating the QApplication
  object, like this:

  \code
  int main( int argc, char** argv ) {
    QApplication::setDesktopSettingsAware( FALSE ); // I know better than the user
    QApplication myApp( argc, argv );           // gimme default fonts & colors
    ...
  }
  \endcode

  \sa desktopSettingsAware()
*/

void QApplication::setDesktopSettingsAware( bool on )
{
    obey_desktop_settings = on;
}

/*!
  Returns the value set by setDesktopSettingsAware(), by default TRUE.

  \sa setDesktopSettingsAware()
*/

bool QApplication::desktopSettingsAware()
{
    return obey_desktop_settings;
}


/*!
  This function enters the main event loop (recursively).
  Do not call it unless you really know what you are doing.
  \sa exit_loop(), loopLevel()
*/

int QApplication::enter_loop()
{
    loop_level++;

    bool old_app_exit_loop = app_exit_loop;
    app_exit_loop = FALSE;

    while ( !app_exit_loop )
	processNextEvent( TRUE );

    app_exit_loop = old_app_exit_loop || quit_now;
    loop_level--;

    if ( app_exit_loop && !loop_level )
	emit aboutToQuit();

    return 0;
}


/*!
  This function leaves from a recursive call to the main event loop.
  Do not call it unless you are an expert.
  \sa enter_loop(), loopLevel()
*/

void QApplication::exit_loop()
{
    app_exit_loop = TRUE;
}


/*!
  Returns the current loop level

  \sa enter_loop(), exit_loop()
*/

int QApplication::loopLevel() const
{
    return loop_level;
}


/*!
  \fn bool QApplication::isSessionRestored() const

  Returns whether the application has been restored from an earlier
  \link session.html session \endlink.

  \sa sessionId(), commitData(), saveState()
*/


/*!
  \fn QString QApplication::sessionId() const

  Returns the identifier of the current \link session.html session
  \endlink.

  If the application has been restored from an earlier session, this
  identifier is the same as it was in that previous session.

  The session identifier is guaranteed to be unique for both different
  applications and different instances of the same application.

  \sa isSessionRestored(), commitData(), saveState()
 */


/*!
  \fn void QApplication::commitData( QSessionManager& sm )

  This function deals with \link session.html session management
  \endlink. It is invoked when the \link QSessionManager session
  manager \endlink wants the application to commit all its data.

  Usually this means saving of all open files, after getting
  permission from the user. Furthermore you may want to provide the
  user a way to cancel the shutdown.

  Note that you should not exit the application within this function.
  Instead, the session manager may or may not do this afterwards,
  depending on the context.

  <strong>Important</strong><br> Within this function, no user
  interaction is possible, \e unless you ask the session manager \a sm
  for explicit permission. See QSessionManager::allowsInteraction()
  and QSessionManager::allowsErrorInteraction() for details and
  example usage.

  Details about session management in general can be found \link
  session.html here \endlink.

  \sa isSessionRestored(), sessionId(), saveState()
*/

void QApplication::commitData( QSessionManager& /* sm */ )
{
}


/*!
  \fn void QApplication::saveState( QSessionManager& sm )

  This function deals with \link session.html session management
  \endlink It is invoked when the \link QSessionManager session
  manager \endlink wants the application to preserve its state for a
  future session.

  For a text editor this would mean creating a temporary file that
  includes the current contents of the edit buffers, the location of
  the cursor and other aspects of the current editing session.

  Note that you should never exit the application within this
  function.  Instead, the session manager may or may not do this
  afterwards, depending on the context. Futhermore, most session
  managers will very likely request a saved state immediately after
  the application has been started. This permits the session manager
  to learn about the application's restart policy.

  <strong>Important</strong><br> Within this function, no user
  interaction is possible, \e unless you ask the session manager \a sm
  for explicit permission. See QSessionManager::allowsInteraction()
  and QSessionManager::allowsErrorInteraction() for details.

  Details about session management in general can be found \link
  session.html here \endlink

  \sa isSessionRestored(), sessionId(), commitData()
*/

void QApplication::saveState( QSessionManager& /* sm */ )
{
}

/*!
  Sets the time after which a drag should start.

  \sa startDragTime()
*/

void QApplication::setStartDragTime( int ms )
{
    drag_time = ms;
}

/*!
  If you support drag'n'drop in you application and a drag should
  start after a mouse click and after a certain time elapsed, you
  should use the value which this method returns as delay (in ms).

  Qt internally uses also this delay e.g. in QMultiLineEdit for starting
  a drag.

  The defaul value is set to 250 ms.

  \sa setStartDragTime(), startDragDistance()
*/

int QApplication::startDragTime()
{
    return drag_time;
}

/*!
  Sets the distance after which a drag should start.

  \sa startDragDistance()
*/

void QApplication::setStartDragDistance( int l )
{
    drag_distance = l;
}

/*!
  If you support drag'n'drop in you application and a drag should
  start after a mouse click and after moving the mouse a certain distance,
  you should use the value which this method returns as the distance. So
  if the mouse position of the click is stored in \c startPos and the current
  position (e.g. in the mouse move event) is \c currPos, you can find out if a
  drag should be started with a code like this:

  \code
  if ( ( startPos - currPos ).manhattanLength() > QApplication::startDragDistance() )
      startTheDrag();
  \endcode

  Qt internally uses this value too, e.g. in the QFileDialog.

  The default value is set to 4 pixels.

  \sa setStartDragDistance(), startDragDistance(), QPoint::manhattanLength()
*/

int QApplication::startDragDistance()
{
    return drag_distance;
}

/*!
  \class QSessionManager qsessionmanager.h
  \brief The QSessionManager class provides access to the session manager.

  \ingroup environment

  The QSessionManager class provides access to the session manager
  during a \link session.html session management \endlink action.  In
  Qt, session management actions are handled in the two virtual
  functions QApplication::commitData() and QApplication::saveState().
  Both functions provide a reference to a session manager object as
  argument, thus allowing the application to communicate with the
  session manager.

  During a session management action, i.e. within one of the two
  mentioned functions, no user interaction is possible, \e unless the
  application got explicit permission from the session manager.  An
  ordinary permission can be requested with allowsInteraction().  With
  allowsErrorInteraction(), applications can ask for a higher priority
  interaction permission in case an error occured.

  Another important function is cancel(), which asks the session
  manager to cancel the shutdown process.

  For sophisticated session managers as provided on Unix/X11,
  QSessionManager offers further possibilites to fine-tune an
  application's session management behaviour: setRestartCommand(),
  setDiscardCommand(), setRestartHint(), setProperty(),
  requestPhase2().  Please see the respective function descriptions
  for further details.
*/

/*! \enum QSessionManager::RestartHint

  This enum type defines the circumstances under which this
  application wants to be restarted by the session manager.  The
  current values are: <ul>

  <li> \c RestartIfRunning - if the application still runs by the time
  the session is shut down, it wants to be restarted at the start of
  the next session.

  <li> \c RestartAnyway - the application wants to be started at the
  start of the next session, no matter what.  (This is useful for
  utilities that run just after startup, then quit.)

  <li> \c RestartImmediately - the application wants to be started
  immediately whenever it is not running.

  <li> \c RestartNever - the application does not want to be restarted
  automatically.

  </ul>

  The default hint is \c RestartIfRunning.
*/


/*!
  \fn QString QSessionManager::sessionId() const

  Returns the identifier of the current \link session.html session
  \endlink.

  If the application has been restored from an earlier session, this
  identifier is the same as it was in that previous session.

  \sa QApplication::sessionId()
 */


// ### Note: This function is undocumented, since it is #ifdef'd.

/*!
  \fn HANDLE QSessionManager::handle() const

  X11 only: returns a handle to the current \c SmcConnection.
*/


/*!
  \fn bool QSessionManager::allowsInteraction()

  Asks the session manager for permission to interact with the
  user.  Returns TRUE if the interaction was granted, FALSE
  otherwise.

  The rationale behind this is to make it possible to synchronize user
  interaction during a shutdown.  Advanced session managers on
  multitasking operating systems may ask all applications
  simultaniously to commit their data, which results in a much faster
  shutdown.

  After the interaction we recommend releasing the user interaction
  semaphore with a call to release().  This way, other applications
  may get the chance to interact with the user while your application
  is still busy saving data.

  If the user decides to cancel the shutdown process during the
  interaction phase, you must tell the session manager by calling to
  cancel().

  Here's an example usage of the mentioned functions that may occur
  in the QApplication::commitData() function of an application:

\code
void MyApplication::commitData( QSessionManager& sm ) {
    if ( sm.allowsInteraction() ) {
	switch ( QMessageBox::warning( yourMainWindow, "Application Name",
					"Save changes to Document Foo?",
					tr("&Yes"),
					tr("&No"),
					tr("Cancel"),
					0, 2) ) {
	case 0: // yes
	    sm.release();
	    // save document here. If saving fails, call sm.cancel()
	    break;
	case 1: // no
	    break;
	default: // cancel
	    sm.cancel();
	    break;
	}
    } else {
	// we did not get permission to interact, then
	// do something reasonable instead.
    }
}
\endcode

  If an error occured within the application while saving its data,
  you may want to try allowsErrorInteraction() instead.

   \sa QApplication::commitData(), release(), cancel()
*/


/*!
  \fn bool QSessionManager::allowsErrorInteraction()

  Like allowsInteraction() but tells the session manager in addition
  that an error occured.  Session managers may treat error interaction
  requests with higher priority.  That means it is more likely that an
  error interaction is granted.  However, you are still not guaranteed
  that the session manager will grant your request.

  \sa allowsInteraction(), release(), cancel()
*/

/*!
  \fn void QSessionManager::release()

  Releases the session manager's interaction semaphore after an
  interaction phase.

  \sa allowsInteraction(), allowsErrorInteraction()
*/

/*!
  \fn void QSessionManager::cancel()

  Tells the session manager to cancel the shutdown process.
  Applications should not call this function without asking the user
  first.

  \sa allowsInteraction(), allowsErrorInteraction()

*/

/*!
  \fn void QSessionManager::setRestartHint( RestartHint hint )

  Sets the application's restart hint to \a hint.  On application
  start-up the hint is set to \c RestartIfRunning.

  Note that these flags are only hints, a session manager may or may
  not obey them.

  We recommend setting the restart hint in QApplication::saveState()
  since most session managers perform a checkpoint shortly after an
  application's startup.

  \sa restartHint()
*/

/*!
  \fn QSessionManager::RestartHint QSessionManager::restartHint() const

  Returns the application's current restart hint. The default is
  \c RestartIfRunning.

  \sa setRestartHint()
*/

/*!
  \fn void QSessionManager::setRestartCommand( const QStringList& command)

  If the session manager is capable of restoring sessions, it will
  execute \a command in order to restore the application.  The command
  defaults to

  \code
	       appname -session id
  \endcode

  The \c -session option is mandatory, otherwise QApplication can not
  tell whether it has been restored or what the current session
  identifier is.  See QApplication::isSessionRestored() and
  QApplication::sessionId() for details.  If your application is very
  simple, it may be possible to store the entire application state in
  additional command line options.  In general, this is a very bad
  idea, since command lines are often limited to a few hundred bytes.
  Instead, use temporary files or a database for this purpose.  By
  marking the data with the unique sessionId(), you will be able to
  restore the application in a future session.

  \sa restartCommand(), setDiscardCommand(), setRestartHint()
*/

/*!
  \fn QStringList QSessionManager::restartCommand() const

  Returns the currently set restart command.

  \sa setRestartCommand(), restartHint()
*/

/*!
  \fn void QSessionManager::setDiscardCommand( const QStringList& )

  \sa discardCommand(), setRestartCommand()
*/


/*!
  \fn QStringList QSessionManager::discardCommand() const

  Returns the currently set discard command.

  \sa setDiscardCommand(), restartCommand(), setRestartCommand()
*/

/*!
  \fn void QSessionManager::setProperty( const QString& name, const QString& value )

  Low-level write access to the application's identification and state
  record kept in the session manager.
*/

/*!
  \fn void QSessionManager::setProperty( const QString& name, const QStringList& value )

  Low-level write access to the application's identification and state
  record kept in the session manager.
*/

/*!
  \fn bool QSessionManager::isPhase2() const

  Returns whether the session manager is currently performing a second
  session management phase.

  \sa requestPhase2()
*/

/*!
  \fn void QSessionManager::requestPhase2()

  Requests a second session management phase for the application.  The
  application may then simply return from the
  QApplication::commitData() or QApplication::saveState() function.
  The respective function will be called again after the first session
  management phase has been finished, this time with isPhase2()
  returning TRUE.

  The two phases are useful for applications like X11 window manager,
  that need to store informations about other application's windows
  and therefore have to wait until these applications finished their
  respective session management tasks.

  \sa isPhase2()
*/
