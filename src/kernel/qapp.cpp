/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qapp.cpp#94 $
**
** Implementation of QApplication class
**
** Created : 931107
**
** Copyright (C) 1993-1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qapp.h"
#include "qobjcoll.h"
#include "qwidget.h"
#include "qwidcoll.h"
#include "qpalette.h"

RCSTAG("$Id: //depot/qt/main/src/kernel/qapp.cpp#94 $");


/*!
  \class QApplication qapp.h
  \brief The QApplication class manages the application event queue.

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
    #include <qapp.h>				// defines QApplication
    #include <qpushbt.h>			// defines QPushButton

    int main( int argc, char **argv )
    {
	QApplication app( argc, argv );		// create application object
	QPushButton  hello( "Hello, world!" );	// create a push button
	app.setMainWidget( &hello );		// define as main widget
	connect( &hello, SIGNAL(clicked()),	// clicking the button
		 &app, SLOT(quit()) );		//   quits the application
	hello.show();				// show button
	return a.exec();			// run main event loop
    }
  \endcode

  <strong>Important</strong><br>
  Notice that the QApplication object must  be created before any widget can
  be defined!

  Note also that for X11, setMainWidget() may change the main widget
  according to the \e -geometry option.	 To preserve this functionality,
  you must set your defaults before setMainWidget() and any overrides
  after.

  \header qkeycode.h
  \header qwindefs.h
  \header qglobal.h
*/


void qt_init( int *, char ** );			// defined in qapp_xyz.cpp
void qt_cleanup();

QApplication *qApp = 0;				// global application object
QPalette *QApplication::app_pal	       = 0;	// default application palette
QFont	 *QApplication::app_font       = 0;	// default application font
QCursor	 *QApplication::app_cursor     = 0;	// default application cursor
bool	  QApplication::is_app_running = FALSE;	// app starting up if FALSE
bool	  QApplication::is_app_closing = FALSE;	// app closing down if TRUE
int	  QApplication::loop_level     = 0;	// event loop level
QWidget	 *QApplication::main_widget    = 0;	// main application widget
QWidget	 *QApplication::focus_widget   = 0;	// has keyboard input focus


#if defined(_WS_MAC_)
GUIStyle QApplication::app_style = MacStyle;	// default style for Mac
#elif defined(_WS_WIN_)
GUIStyle QApplication::app_style = WindowsStyle;// default style for Windows
#elif defined(_WS_PM_)
GUIStyle QApplication::app_style = PMStyle;	// default style for OS/2 PM
#elif defined(_WS_X11_)
GUIStyle QApplication::app_style = MotifStyle;	// default style for X Windows
#endif

int	 QApplication::app_cspec = QApplication::NormalColor;


static QPalette *stdPalette = 0;

static void create_palettes()			// creates default palettes
{
    QColorGroup std_nor( black, lightGray,
			 white, lightGray.dark(), gray,
			 black, white );
    QColorGroup std_dis( darkGray, lightGray,
			 white, lightGray.dark(), gray,
			 darkGray, std_nor.background() );
    QColorGroup std_act( black, lightGray,
			 white, lightGray.dark(), gray,
			 black, white );
    stdPalette = new QPalette( std_nor, std_dis, std_act );
}

static void destroy_palettes()
{
    delete stdPalette;
}


/*!
  Constructs an application object with the command line arguments \e argc
  and \e argv.

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
  See <a href=debug.html> Debugging Techniques</a> for
  a more detailed explanation.

  The X11 version of Qt support a few more command line options:
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
  <li> \c -style= \e style, sets the application GUI style. Possible values
       are \c motif and \c windows 
  </ul>

  \sa argc(), argv()
*/

QApplication::QApplication( int &argc, char **argv )
{
#if defined(CHECK_STATE)
    if ( qApp )
	warning( "QApplication: There should be only one application object" );
#endif
    qApp = this;
    qt_init( &argc, argv );
    initMetaObject();
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
	CHECK_PTR( app_font );
    }
    QWidget::createMapper();			// create widget mapper
    is_app_running = TRUE;			// no longer starting up
}

/*!
  Closes all widgets and cleans up all window system resources.
  Sets \c qApp to 0.
*/

QApplication::~QApplication()
{
    is_app_closing = TRUE;
    QWidget::destroyMapper();			// destroy widget mapper
    destroy_palettes();
    delete app_pal;
    app_pal = 0;
    delete app_font;
    app_font = 0;
    delete app_cursor;
    app_cursor = 0;
    qt_cleanup();
    delete objectDict;
    qApp = 0;
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

    #include <qapp.h>
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
  \fn GUIStyle QApplication::style()
  Returns the GUI style of the application.
  \sa setStyle()
*/

/*!
  Sets the application GUI style to \e style.

  The style parameter can be \c MacStyle, \c WindowsStyle, \c PMStyle
  or \c MotifStyle.  Only \c MotifStyle and \c WindowsStyle is
  supported in the current version of Qt.

  \sa style(), QWidget::setStyle()
*/

void QApplication::setStyle( GUIStyle style )
{
    app_style = style;
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
  <li> \c QApplication::NormalColor. This is the default color allocation
  strategy. The application allocates system global colors. This work fine
  for most applications under X11, but Windows dithers to the 20 standard
  colors unless the display has true color support (more than 256 colors).

  <li> \c QApplication::CustomColor. Under X11 this is the same as \c
  NormalColor. Under Windows, Qt creates a Windows palette if the display
  supports 256 colors.

  <li> \c QApplication::PrivateColor. Under Windows this is the same as \c
  CustomColor. Under X11, Qt uses a private colormap for the application.

  <li> \c QApplication::TrueColor. Under Windows, this is equal to
  \c NormalColor. Under X11, this option makes the application use
  a true color visual if one exists but is not the default visual.
  Silicon Graphics X servers have this feature. They provide an 8
  bit visual as default but can deliver true color when asked.
  </ul>

  The settings are bit-coded and can be combined using | or +.

  If you have an application that uses buttons, menus, texts and
  pixmaps with few colors, you can probably use the \c NormalColor
  specification (i.e. keep the default setting).

  If your application needs some custom colors you should specify \c
  CustomColor.  This only makes a difference on Windows - the application
  gets more colors when it is active, but the background windows look
  less good.

  If the application is color hungry (e.g. it wants 200 colors), \c
  PrivateColor is best.  On Windows, it is the same as \c CustomColor.
  Under X, you get a private colormap.

  If your application is very color hungry (e.g. it wants thousands of
  colors), you can specify a combination such as \c
  (TrueColor+PrivateColor) or \c (TrueColor+CustomColor).  These
  combinations mean to use 24-bit color if that is available, and fall
  back to \c PrivateColor or \c CustomColor respectively if not.
  
  Example:
  \code
  int main( int argc, char **argv )
  {
      QApplication::setColorSpec( QApplication::PrivateColor );
      QApplication a( argc, argv );
      ...
  }
  \endcode

  QColor provides more functionality for controlling color allocation and
  freeing up certains colors. See QColor::enterAllocContext() for more
  information.

  To see what mode you end up with, you can call QColor::numBitPlanes()
  once the QApplication object exists.  A value greater than 8 (typically
  16,24 or 32) means true color.

  \warning The X.h header file for X11 contains <code>#define TrueColor
  </code>. If you include the X headers files (which is necessary only
  when you bypass Qt and write Xlib-specific code), you must <code>#undef
  TrueColor</code> to be able to specify \c QApplication::TrueColor.

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
  \sa setPalette(), QWidget::palette()
*/

QPalette *QApplication::palette()
{
    return app_pal;
}

/*!
  Changes the default application palette to \e palette.

  If \e updateAllWidgets is TRUE, then the palette of all existing
  widgets is set to \e palette.

  Widgets created after this call get \e palette as their \link
  QWidget::palette() palette\endlink.

  \sa QWidget::setPalette(), palette()
*/

void QApplication::setPalette( const QPalette &palette, bool updateAllWidgets )
{
    delete app_pal;
    app_pal = new QPalette( palette.copy() );
    CHECK_PTR( app_pal );
    if ( updateAllWidgets && is_app_running && !is_app_closing ) {
	QWidgetIntDictIt it( *((QWidgetIntDict*)QWidget::mapper) );
	register QWidget *w;
	while ( (w=it.current()) ) {		// for all widgets...
	    ++it;
	    if ( !w->testWFlags(WType_Desktop) )// (except desktop)
		w->setPalette( *app_pal );
	}
    }
}


/*!
  \fn QFont *QApplication::font()
  Returns the default application font.	 There is always an application
  font, i.e. the returned pointer is guaranteed to be non-null.
  \sa setFont(), fontMetrics(), QWidget::font()
*/

/*!
  Changes the default application font to \e font.

  The default font depends on the X server in use.

  If \e updateAllWidgets is TRUE, then the font of all existing
  widgets is set to \e font.

  Widgets created after this call get \e font as their \link
  QWidget::font() font\endlink.

  \sa font(), fontMetrics(), QWidget::setFont()
*/

void QApplication::setFont( const QFont &font,	bool updateAllWidgets )
{
    if ( app_font )
	delete app_font;
    app_font = new QFont( font );
    CHECK_PTR( app_font );
    QFont::setDefaultFont( *app_font );
    if ( updateAllWidgets ) {			// set for all widgets now
	QWidgetIntDictIt it( *((QWidgetIntDict*)QWidget::mapper) );
	register QWidget *w;
	while ( (w=it.current()) ) {		// for all widgets...
	    ++it;
	    if ( !w->testWFlags(WType_Desktop) )// (except desktop)
		w->setFont( *app_font );
	}
    }
}


/*!
  Returns a list of the top level widgets in the application.

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

  \sa QWidget::isTopLevel(), QWidget::isVisible(), QList::isEmpty()
*/

QWidgetList *QApplication::topLevelWidgets()
{
    return QWidget::tlwList();
}


/*!
  \fn QWidget *QApplication::focusWidget() const
  Returns the application widget that has the keyboard input focus, or null
  if no application widget has the focus.
  \sa QWidget::setFocus(), QWidget::hasFocus()
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

  \sa quit(), exec()
*/

void QApplication::exit( int retcode )
{
    if ( !qApp )				// no global app object
	return;
    if ( qApp->quit_now )			// don't overwrite quit code
	return;
    qApp->quit_now = TRUE;
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

  All Qt events are sent using the notify function. Since this function
  is virtual, you can make a subclass of QApplication and reimplement
  notify() to get total control of Qt events.

  Installing an event filter on \c qApp is another way of making an
  application-global event hook.

  \sa QObject::event(), installEventFilter()
*/

bool QApplication::notify( QObject *receiver, QEvent *event )
{
    if ( receiver == 0 ) {			// serious error
#if defined(CHECK_NULL)
	warning( "QApplication::notify: Unexpected null receiver" );
#endif
	return FALSE;
    }
    if ( eventFilters ) {
	QObjectListIt it( *eventFilters );
	register QObject *obj = it.current();
	while ( obj ) {				// send to all filters
	    ++it;				//   until one returns TRUE
	    if ( obj->eventFilter(receiver,event) )
		return TRUE;
	    obj = it.current();
	}
    }
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


#if !defined(_WS_X11_)

// The doc and X implementation of these functions is in qapp_x11.cpp

void QApplication::flushX()	{}		// do nothing

void QApplication::syncX()	{}		// do nothing

#endif

