/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qapplication.cpp#208 $
**
** Implementation of QApplication class
**
** Created : 931107
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
#include "qdeveloper.h"
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

/*!
  \class QApplication qapplication.h
  \brief The QApplication class manages the application event queue.

  \ingroup kernel

  The QApplication class is central to Qt.  It receives events from
  the underlying window system and sends them to the destination widgets.
  An application object must be created before any widgets can be created!

  Only one single QApplication object should be created.  In fact Qt
  complains if you create more than one, and this is normally done
  in the main() function.  Once a QApplication object has been
  created, \c qApp (defined as <code>extern QApplication *qApp</code>)
  refers to this object.

  Example (a complete Qt application):
  \code
    #include <qapplication.h>				// defines QApplication
    #include <qpushbutton.h>			// defines QPushButton

    int main( int argc, char **argv )
    {
	QApplication app( argc, argv );		// create application object
	QPushButton  hello( "Hello, world!" );	// create a push button
	app.setMainWidget( &hello );		// define as main widget
	connect( &hello, SIGNAL(clicked()),	// clicking the button
		 &app, SLOT(quit()) );		//   quits the application
	hello.show();				// show button
	return app.exec();			// run main event loop
    }
  \endcode

  <strong>Important</strong><br> Notice that the QApplication object must
  be created before any window-system functionality of Qt is used, this
  includes widgets, colors, fonts etc.

  Note also that for X11, setMainWidget() may change the main widget
  according to the \e -geometry option.	 To preserve this functionality,
  you must set your defaults before setMainWidget() and any overrides
  after.

  While Qt is not optimized or designed for writing non-GUI programs,
  it's possible to use <a href="tools.html">some of its classes</a>
  without creating a QApplication.  This can be very useful if you
  wish to share code between a non-GUI server and a GUI client.

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

QApplication *qApp = 0;				// global application object
QStyle *QApplication::app_style	       = 0;	// default application style
QPalette *QApplication::app_pal	       = 0;	// default application palette
QFont	 *QApplication::app_font       = 0;	// default application font
QDict<QPalette>* QApplication::app_palettes = 0; // default application palettes
QDict<QFont>* QApplication::app_fonts = 0;// default application fonts
QCursor	 *QApplication::app_cursor     = 0;	// default application cursor
int	  QApplication::app_tracking   = 0;	// global mouse tracking
bool	  QApplication::is_app_running = FALSE;	// app starting up if FALSE
bool	  QApplication::is_app_closing = FALSE;	// app closing down if TRUE
int	  QApplication::loop_level     = 0;	// event loop level
QWidget	 *QApplication::main_widget    = 0;	// main application widget
QWidget	 *QApplication::focus_widget   = 0;	// has keyboard input focus
QWidget	 *QApplication::active_window  = 0;	// toplevel that has keyboard input focus
QWidgetList *QApplication::popupWidgets= 0;	// has keyboard input focus
static bool makeqdevel = FALSE;		// developer tool needed?
static QDeveloper* qdevel = 0;		// developer tool
static QWidget *desktopWidget	= 0;		// root window widget


int	 QApplication::app_cspec = QApplication::NormalColor;


static QPalette *stdPalette = 0;
static QColor * winHighlightColor = 0;
static int mouseDoubleClickInterval = 400;

static void create_palettes()			// creates default palettes
{
    QColor standardLightGray( 192, 192, 192 );
    QColor light( 255, 255, 255 );
    QColor dark( standardLightGray.dark( 150 ) );
    QColorGroup std_nor( Qt::black, standardLightGray,
			 light, dark, Qt::gray,
			 Qt::black, Qt::white );
    QColorGroup std_dis( Qt::darkGray, standardLightGray,
			 light, dark, Qt::gray,
			 Qt::darkGray, std_nor.background() );
    QColorGroup std_act( Qt::black, standardLightGray,
			 light, dark, Qt::gray,
			 Qt::black, Qt::white );
    stdPalette = new QPalette( std_nor, std_dis, std_act );
}

static void destroy_palettes()
{
    delete stdPalette;
}

static
void process_cmdline( int* argcptr, char ** argv )
{
    // process platform-indep command line

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
	    qApp->setStyle( new QWindowsStyle );
	} else if ( stricmp(arg, "-style=motif") == 0 ) {
	    qApp->setStyle( new QMotifStyle );
	} else if ( stricmp(arg, "-style=platinum") == 0 ) {
	    qApp->setStyle( new QPlatinumStyle );
	} else if ( stricmp(arg, "-style=cde") == 0 ) {
	    qApp->setStyle( new QCDEStyle );
	} else if ( strcmp(arg,"-style") == 0 && i < argc-1 ) {
	    QCString s = argv[++i];
	    s = s.lower();
	    if ( s == "windows" )
		qApp->setStyle( new QWindowsStyle );
	    else if ( s == "motif" )
		qApp->setStyle( new QMotifStyle );
	    else if ( s == "platinum" )
		qApp->setStyle( new QPlatinumStyle );
	    else if ( s == "cde" )
		qApp->setStyle( new QCDEStyle );
	} else
	    argv[j++] = argv[i];
    }
    *argcptr = j;
}

/*!
  Initializes the window system and constructs an application object
  with the command line arguments \e argc and \e argv.

  The global \c qApp pointer refers to this application object. Only
  one application object should be created.

  This application object must be constructed before any \link
  QPaintDevice paint devices\endlink (includes widgets, pixmaps, bitmaps
  etc.)

  Notice that \e argc and \e argv might be changed.  Qt removes
  command line arguments that it recognizes.  \e argc and \e argv are
  can be accessed later by \c qApp->argc() and \c qApp->argv().	 The
  documentation for argv() contains a detailed description of how to
  process command line arguments.

  Qt debugging options:
  <ul>
  <li> \c -nograb, tells Qt to never grab the mouse or the keyboard.
  <li> \c -sync (only under X11), switches to synchronous mode for
	debugging.
  </ul>

  See <a href="debug.html">Debugging Techniques</a> for a more
  detailed explanation.

  All Qt programs automatically support the following command line options:
  <ul>
  <li> \c -style= \e style, sets the application GUI style. Possible values
       are \c motif, \c windows, and \c platinum.
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

QApplication::QApplication( int &argc, char **argv )
{
    init_precmdline();
    static char *empty = "";
    if ( argc == 0 || argv == 0 ) {
	argc = 0;
	argv = &empty;
    }

    qt_init( &argc, argv );
    process_cmdline( &argc, argv );

    initialize( argc, argv );
}


#if defined(_WS_X11_)

/*!
  Create an application, given an already open display.  This is
  available only on X11.
*/

QApplication::QApplication( Display* dpy )
{
    init_precmdline();
    // ... no command line.
    qt_init( dpy );
    initialize( 0, 0 );
}

#endif // _WS_X11_

void QApplication::init_precmdline()
{
    translators = 0;
#if defined(CHECK_STATE)
    if ( qApp )
	warning( "QApplication: There should be only one application object" );
#endif

    qApp = this;
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
    if ( !app_pal ) {				// palette not already set
	create_palettes();
	app_pal = new QPalette( *stdPalette );
	CHECK_PTR( app_pal );
    }
    if ( !app_font ) {				// font not already set
	app_font = new QFont;
	app_font->setCharSet( QFont::defaultFont().charSet() );
	CHECK_PTR( app_font );
    }

    QWidget::createMapper();			// create widget mapper
    is_app_running = TRUE;			// no longer starting up

    if (!app_style) {
#if defined(_WS_WIN_)
	app_style = new QWindowsStyle;// default style for Windows
#elif defined(_WS_X11_)
	app_style = new QMotifStyle;// default style for X Windows
#endif
    }

    qInitPngIO();

    app_style->polish( *app_pal );
    app_style->polish( this ); //##### wrong place, still inside the qapplication constructor...grmbl....

    if ( makeqdevel ) {
	qdevel = new QDeveloper;
	qdevel->show();
    }
}


/*!
  Cleans up any window system resources that were allocated by this
  application.  Sets the global variable \c qApp to null. Unlike
  former versions of Qt the destructor does \e not delete all
  remaining widgets.
*/

QApplication::~QApplication()
{
    is_app_closing = TRUE;
    QWidget::destroyMapper();
    destroy_palettes();
    delete app_pal;
    app_pal = 0;
    delete app_font;
    app_font = 0;
    delete app_palettes;
    app_palettes = 0;
    delete app_fonts;
    app_fonts = 0;
    delete app_cursor;
    app_cursor = 0;
    qt_cleanup();
    delete winHighlightColor;
    winHighlightColor = 0;
    delete objectDict;
    qApp = 0;
    //can not delete codecs until after QDict destructors
    //QTextCodec::deleteAllCodecs();
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
  Sets the application GUI style to \e style.

  \sa style(), QStyle
*/

void QApplication::setStyle( QStyle *style )
{
    QStyle* old = app_style;
    app_style = style;

    if ( startingUp() ) {
	delete old;
	return;
    }

    if (old) {
	if ( is_app_running && !is_app_closing ) {
	    QWidgetIntDictIt it( *((QWidgetIntDict*)QWidget::mapper) );
	    register QWidget *w;
	    while ( (w=it.current()) ) {		// for all widgets...
		++it;
		if ( !w->testWFlags(WType_Desktop) // (except desktop)
		     && w->testWState(WState_Polished) ) { // (and have been polished)
		    old->unPolish(w);
		}
	    }
	}
	old->unPolish( qApp );
    }
    app_style->polish( qApp );
    if (old) {
	if ( is_app_running && !is_app_closing ) {
	    QWidgetIntDictIt it( *((QWidgetIntDict*)QWidget::mapper) );
	    register QWidget *w;
	    while ( (w=it.current()) ) {		// for all widgets...
		++it;
		if ( !w->testWFlags(WType_Desktop) // (except desktop)
		     && w->testWState(WState_Polished)) { // (and have been polished)
		    app_style->polish(w);
		    w->styleChange(old->guiStyle());
		    if (w->isVisible()){
			w->update();
		    }
		}
	    }
	}
    }
    delete old;
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

  The color specification controls how your application allocates
  colors. You must set the color specification before you create the
  QApplication object.

  The choices are:
  <ul>
  <li> \c QApplication::NormalColor.
    This is the default color allocation strategy.
    Use this choice if your application uses buttons, menus,
    texts and pixmaps with few colors.
    With this choice, the application allocates system global colors.
    This work fine for most applications under X11, but Windows dithers to
    the 20 standard colors unless the display has true color support (more
    than 256 colors).

  <li> \c QApplication::CustomColor.
    Use this choice if your application needs a small number of
    custom colors.  This choice only makes a difference on Windows
    - the application gets more colors when it is active, but the
    background windows look less good.
    Under X11 this is the same as \c
    NormalColor. Under Windows, Qt creates a Windows palette if the display
    supports 256 colors.

  <li> \c QApplication::ManyColor.
    Use this choice if your application is very color hungry
    (e.g. it wants thousands of colors).
    Under Windows, this is equal to \c CustomColor.
    Under X11 the effect is:
    <ul>
      <li> For 256-color displays which have at best a 256 color true color
	    visual, the default visual is used, and a colors are allocated
	    from a color cube.
	    The color cube is the 6x6x6 (216 color) "Web palette", but the
	    number of colors can be changed by the \e -ncols option.
	    The user can force the application to use the true color visual by
	    the \link QApplication::QApplication() -visual \endlink
	    option.
      <li> For 256-color displays which have a true color visual with more
	    than 256 colors, use that visual.  Silicon Graphics X servers
	    have this feature. They provide an 8 bit visual as default but
	    can deliver true color when asked.
    </ul>
  </ul>

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

  \sa colorSpec(), QColor::numBitPlanes(), QColor::enterAllocContext()
*/

void QApplication::setColorSpec( int spec )
{
#if defined(CHECK_STATE)
    if ( qApp ) {
	warning( "QApplication::setColorSpec: This function must be "
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

QPalette *QApplication::palette(const QWidget* w)
{
#if defined(CHECK_STATE)
    if ( !qApp ) {
	warning( "QApplication::palette: This function can only be "
		 "called after the QApplication object has been created" );
    }
#endif
    if (w && app_palettes) {
	QDictIterator<QPalette> it(*app_palettes);
	const char* name;
	while ( (name=(const char*)(void*)it.currentKeyLong()) ) {
	    if ( w->isA(name) ) {
		return it.current();
	    }
	    ++it;
	}
	(void) it.toFirst();
	while ( (name=(const char*)(void*)it.currentKeyLong()) ) {
	    if ( w->inherits( name ) ) {
		return it.current();
	    }
	    ++it;
	}
    }
    return app_pal;
}


/*!
  Changes the default application palette to \e palette.

  If \e updateAllWidgets is TRUE, then the palette of all existing
  widgets is set to \e palette.

  If a className is passed, then the palette is only set for widgets
  that inherit this class in the sense of QObject::inherits()

  Widgets created after this call get \e   palette as their
  \link QWidget::palette() palette\endlink when they
  access it.

  The palette may be changed according to the current GUI style in
  QStyle::polish().

  \sa QWidget::setPalette(), palette(), QStyle::polish()
*/

void QApplication::setPalette( const QPalette &palette, bool updateAllWidgets, const char* className )
{
    QPalette* pal = new QPalette( palette );

    if ( !startingUp() )
	qApp->style().polish( *pal );


    if (!className) {
	QPalette* old =  app_pal;
	app_pal = pal;
	CHECK_PTR( app_pal );
	delete old;
    }
    else {
	if (!app_palettes){
	    app_palettes = new QDict<QPalette>;
	    CHECK_PTR( app_palettes );
	    app_palettes->setAutoDelete( TRUE );
	}
	CHECK_PTR( pal );
	app_palettes->insert(className, pal);
    }
    if ( updateAllWidgets && is_app_running && !is_app_closing ) {
	QWidgetIntDictIt it( *((QWidgetIntDict*)QWidget::mapper) );
	register QWidget *w;
	while ( (w=it.current()) ) {		// for all widgets...
	    ++it;
	    if ( !w->testWFlags(WType_Desktop) // (except desktop)
		 && !w->testWState(WState_PaletteFixed) ) {// (and except fixed palettes)
		w->setPalette( *QApplication::palette( w ) );
	    }
	}
    }
}



/*!
  \fn QFont *QApplication::font(const QWidget* = 0)
  Returns the default application font.	 There is always an application
  font, i.e. the returned pointer is guaranteed to be non-null.
  \sa setFont(), fontMetrics(), QWidget::font()
*/

QFont *QApplication::font( const QWidget* w)
{
    if (w && app_fonts) {
	QDictIterator<QFont> it(*app_fonts);
	const char* name;
	while ( (name=(const char*)(void*)it.currentKeyLong()) ) {
	    if ( w->isA(name) ) {
		return it.current();
	    }
	    ++it;
	}
	(void) it.toFirst();
	while ( (name=(const char*)(void*)it.currentKeyLong()) ) {
	    if ( w->inherits( name ) ) {
		return it.current();
	    }
	    ++it;
	}
    }
    return app_font;
}



/*!
  Changes the default application font to \e font.

  The default font depends on the X server in use.

  If \e updateAllWidgets is TRUE, then the font of all existing
  widgets is set to \e font.

  If a className is passed, then the palette is only set for widgets
  that inherit this class in the sense of QObject::inherits()

  Widgets created after this call get \e font as their \link
  QWidget::font() font\endlink when they access it.

  \sa font(), fontMetrics(), QWidget::setFont()
*/

void QApplication::setFont( const QFont &font,	bool updateAllWidgets, const char* className )
{
    if (!className) {
	delete app_font;
	app_font = new QFont( font );
	CHECK_PTR( app_font );
	QFont::setDefaultFont( *app_font );
    }
    else {
	if (!app_fonts){
	    app_fonts = new QDict<QFont>;
	    CHECK_PTR( app_fonts );
	    app_fonts->setAutoDelete( TRUE );
	}
	QFont* fnt = new QFont(font);
	CHECK_PTR( fnt );
	app_fonts->insert(className, fnt);
    }
    if ( updateAllWidgets && is_app_running && !is_app_closing ) {
	QWidgetIntDictIt it( *((QWidgetIntDict*)QWidget::mapper) );
	register QWidget *w;
	while ( (w=it.current()) ) {		// for all widgets...
	    ++it;
	    if ( !w->testWFlags(WType_Desktop) // (except desktop)
		 && !w->testWState(WState_FontFixed) ) // (and except fixed fonts)
		w->setFont( *QApplication::font( w ) );
	}
    }
}

/*!
  Polishing of widgets.

  Usually widgets call this automatically when they are polished.  It
  may be used to do some style-based central customization of widgets.

  Note that you are not limited to public functions of \llink QWidget.
  Instead, based on meta information like \link QObject::className()
  you are able to customize any kind of widgets.

  The default implementation calls QStyle::polish().

  \sa QStyle::polish(), QWidget::polish()
*/

void QApplication::polish(QWidget* w)
{
    app_style->polish( w );
}



/*!
  Returns a list of the top level widgets in the application.

  The list is created using new and must be deleted by the caller.

  The list is \link QList::isEmpty() empty \endlink if there are no
  top level widgets.

  Note that some of the top level widgets may be hidden.

  Example:
  \code
    //
    // Shows all hidden top level widgets.
    //
    QWidgetList	 *list = QApplication::topLevelWidgets();
    QWidgetListIt it( *list );		// iterate over the widgets
    while ( it.current() ) {		// for each top level widget...
	if ( !it.current()->isVisible() )
	    it.current()->show();
	++it;
    }
    delete list;			// delete the list, not the widgets
  \endcode

  The QWidgetList class is defined in the qwidcoll.h header file.

  \warning
  Delete the list away as soon you have finished using it.
  You can get in serious trouble if you for instance try to access
  a widget that has been deleted.

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
  returns \e retcode.

  By convention, \e retcode 0 means success, any non-zero value indicates
  an error.

  Note that unlike the C library exit function, this function \e does
  returns to the caller - it is event processing that stops.

  \sa quit(), exec()
*/

void QApplication::exit( int retcode )
{
    if ( !qApp )				// no global app object
	return;
    if ( qApp->quit_now )			// don't overwrite quit code
	return;
    qApp->quit_now  = TRUE;
    qApp->quit_code = retcode;
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

  \sa exit()
*/

void QApplication::quit()
{
    QApplication::exit( 0 );
}


/*!
  \fn void QApplication::lastWindowClosed()

  This signal is emitted when the user has closed a top level widget
  and there are no more visible top level widgets left.

  The signal is very useful when your application has many top level
  widgets but no main widget. You can then connect it to the quit() slot.

  \sa mainWidget(), topLevelWidgets(), QWidget::isTopLevel()
*/


/*!
  \fn bool QApplication::sendEvent( QObject *receiver, QEvent *event )

  Sends an event directly to a receiver, using the notify() function.
  Returns the value that was returned from the event handler.

  \sa postEvent(), notify()
*/

/*!
  Sends \e event to \e receiver: <code>receiver->event( event )</code>
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
	warning( "QApplication::notify: Unexpected null receiver" );
#endif
	return FALSE;
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
    QPalette p( *palette() );
    p.setColor( QColorGroup::Highlight, c );
    setPalette( p, TRUE);
}


/*!\obsolete
  Returns the color used to mark selections in windows style.
  \sa setWinStyleHighlightColor()
*/
const QColor& QApplication::winStyleHighlightColor()
{
    return palette()->normal().highlight();
}


/*!
  Sets the time limit that distinguishes a double click from two
  consecutive mouse clicks to \a ms milliseconds. This value is
  ignored under Windows (the control panel value is used.)

  The default value is 400 milliseconds.

  \sa doubleClickInterval()
*/

void QApplication::setDoubleClickInterval( int ms )
{
    mouseDoubleClickInterval = ms;
}


/*!
  Returns the maximum duration for a double click.

  \sa setDoubleClickInterval()
*/

int QApplication::doubleClickInterval()
{
    return mouseDoubleClickInterval;
}


/*!
  \fn Qt::WindowsVersion QApplication::winVersion()

  Returns the version of the Windows operating system running:

  <ul>
  <li> \c WV_NT Windows NT.
  <li> \c WV_95 Windows 95.
  <li> \c WV_32s Win32s.
  </ul>

  Note that this function is implemented for the Windows version
  of Qt only.
*/


/*!
  Tells the -qdevel widget, if any, about a new top-level widget.
*/

void QApplication::noteTopLevel( QWidget* tlw )
{
    if ( qdevel )
	qdevel->addTopLevelWidget(tlw);
}


/*!  Adds \a mf to the list of message files to be used for
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


/*!  Removes \a mf from the list of message files used by this
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


/*!  Returns the best available translation for \a key in \a scope, by
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
	return key;
    // scope can be null, for global stuff

    if ( translators ) {
	uint h = QTranslator::hash( scope, key );
	QListIterator<QTranslator> it( *translators );
	QTranslator * mf;
	QString result;
	while( (mf=it.current()) != 0 ) {
	    ++it;
	    result = mf->find( h, scope, key );
	    if ( result != QString::null )
		return result;
	}
    }
    return key;
}


/*****************************************************************************
  QApplication management of posted events
 *****************************************************************************/

struct QPostEvent {
    QPostEvent( QObject *r, QEvent *e ) { receiver=r; event=e; }
   ~QPostEvent()			{ delete event; }
    QObject  *receiver;
    QEvent   *event;
};

typedef QList<QPostEvent> QPostEventList;
typedef QListIterator<QPostEvent> QPostEventListIt;
static QPostEventList *postedEvents = 0;	// list of posted events

static void cleanupPostedEvents();

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
	warning( "QApplication::postEvent: Unexpected null receiver" );
#endif
	return;
    }

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
		delete event;
		return;
	    }
	}
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


/*!
  Immediately dispatches all events which have been previously enqueued
  with QApplication::postEvent() and which are for the object \a receiver
  and have the \a event_type.

  If \a receiver is 0, all objects get their events.  If \a event_type is
  0, all types of events are dispatched.

  Some event compression may occur.  Note that events from the
  window system are \e not dispatched by this function.
*/

void QApplication::sendPostedEvents( QObject *receiver, int event_type )
{
    if ( !postedEvents )
	return;
    QPostEventListIt it(*postedEvents);
    QPostEvent *pe;

    // For accumulating compressed events
    QPoint oldpos, newpos;
    QSize oldsize, newsize;
    QRegion paintRegion;
    QRegion erasePaintRegion;
    bool first=TRUE;

    while ( (pe = it.current()) ) {
	++it;

	if ( pe->event && pe->receiver == receiver
	     && pe->event->type() == event_type ) {
	    postedEvents->take( postedEvents->findRef( pe ) );
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
	    pe->event->posted = FALSE;
	    delete pe;
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
	    if (! erasePaintRegion.isEmpty() ) {
		QPaintEvent e( erasePaintRegion, TRUE );
		if ( receiver->isWidgetType() )
		    ((QWidget*)receiver)->erase(erasePaintRegion);
		sendEvent( receiver, &e );
	    }
	    if (! paintRegion.isEmpty() ) {
		QPaintEvent e( paintRegion, FALSE );
		sendEvent( receiver, &e );
	    }
	}
    }
}


static void cleanupPostedEvents()		// cleanup list
{
    delete postedEvents;
    postedEvents = 0;
}




/*!  Removes all events posted using postEvent() for \a receiver.

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


/*!  Removes \a event from the queue of posted events, and emits a
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
	    const char *n = 0;
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
	    warning( "QEvent: Warning: %s event deleted while posted to %s %s",
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



/*! Returns the desktop widget (also called the root window).

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
