/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qapplication.cpp#59 $
**
** Implementation of QApplication class
**
** Author  : Haavard Nord
** Created : 931107
**
** Copyright (C) 1993-1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qapp.h"
#include "qobjcoll.h"
#include "qwidget.h"
#include "qwidcoll.h"
#include "qpalette.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qapplication.cpp#59 $";
#endif


/*----------------------------------------------------------------------------
  \class QApplication qapp.h
  \brief The QApplication class manages the application event queue.

  The QApplication class is central to Qt.  It receives events from
  the underlying window system and sends them to the destination widgets.
  An application object must be created before any widgets can be created!

  Only one single QApplication object should be created, and this is
  normally done in the main() function.	 When a QApplication object has
  been created, \c qApp (defined as <code>extern QApplication
  *qApp</code>) will refer to this object.

  Here is a complete Qt application:
  \code
    #include <qapp.h>				// defines QApplication
    #include <qpushbt.h>			// defines QPushButton

    int main( int argc, char **argv )
    {
	QApplication app( argc, argv );		// create app object
	QPushButton  hi( "Hello, world" );	// create a push button
	app.setMainWidget( &hi );		// define as main widget
	hi.show();				// show button
	return a.exec();			// run main event loop
    }
  \endcode

  <strong>Important</strong><br>
  Notice that the QApplication object must be created before any widget can be
  defined!

  Note also that on X11, setMainWidget() may change the main widget
  according to the \e -geometry option.

  \header qkeycode.h
  \header qwindefs.h
  \header qglobal.h
 ----------------------------------------------------------------------------*/


void qt_init( int *, char ** );			// defined in qapp_???.cpp
void qt_cleanup();

QApplication *qApp = 0;				// global application object
QPalette *QApplication::app_pal	      = 0;	// default application palette
QFont	 *QApplication::app_font      = 0;	// default application font
QCursor	 *QApplication::app_cursor    = 0;	// default application cursor
bool	  QApplication::starting_up   = TRUE;	// app starting up
bool	  QApplication::closing_down  = FALSE;	// app closing down
QWidget	 *QApplication::main_widget   = 0;	// main application widget
QWidget	 *QApplication::focus_widget  = 0;	// has keyboard input focus


#if defined(_WS_MAC_)
GUIStyle QApplication::app_style = MacStyle;	// default style for Mac
#elif defined(_WS_WIN_)
GUIStyle QApplication::app_style = WindowsStyle;// default style for Windows
#elif defined(_WS_PM_)
GUIStyle QApplication::app_style = PMStyle;	// default style for OS/2 PM
#elif defined(_WS_X11_)
GUIStyle QApplication::app_style = MotifStyle;	// default style for X Windows
#endif


static QPalette *motifPalette = 0;

static void create_palettes()			// creates default palettes
{
    QColorGroup motif_nor( black, lightGray,
			   white, lightGray.dark(), gray,
			   black, white );
    QColorGroup motif_dis( darkGray, lightGray,
			   white, lightGray.dark(), gray,
			   darkGray, white );
    QColorGroup motif_act( black, lightGray,
			   white, lightGray.dark(), gray,
			   black, white );
    motifPalette = new QPalette( motif_nor, motif_dis, motif_act );
}

static void destroy_palettes()
{
    delete motifPalette;
}


/*----------------------------------------------------------------------------
  Creates an application object with the command line parameters \e argc
  and \e argv.

  The global \c qApp pointer will refer to this application
  object. Only one application object should be created.

  This application object must be constructed before any paint devices
  (includes widgets, pixmaps, bitmaps etc.)

  Notice that the \e argc and \e argv parameters might be changed. Qt
  will remove command line options that it recognizes.

  Qt accepts these debugging options:
  <ul>
  <li> -nograb, Tells Qt to never grab the mouse or the keyboard.
  <li> -memchk, Performs memory leak detection.
  <li> -membuf n, Sets the memory checking buffer size.
  <li> -memlog file, Sets the memory checking output file name.
  </ul>
  See <a href=#debuggingtechniques> Debugging Techniques</a> for
  a more detailed explanation.

  The X-Windows version of Qt accepts additional command line options:
  <ul>
  <li> -display \e display, Sets the X display (default is $DISPLAY).
  <li> -geometry \e geometry, Sets the client geometry of the
	\link setMainWidget() main widget\endlink.
  <li> -fn or -font \e font, Defines the application font.
  <li> -bg or -background \e color, Sets the default background color.
  <li> -fg or -foreground \e color, Sets the default foreground color.
  <li> -name \e name, Sets the application name.
  <li> -title \e title, Sets the application title (caption).
  <li> -sync, Turns X into synchronous mode for debugging.
  </ul>
 ----------------------------------------------------------------------------*/

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
	app_pal = new QPalette( *motifPalette );
	CHECK_PTR( app_pal );
    }
    if ( !app_font ) {				// font not already set
	app_font = new QFont;
	CHECK_PTR( app_font );
    }
    QWidget::createMapper();			// create widget mapper
    starting_up = FALSE;			// no longer starting up
}

/*----------------------------------------------------------------------------
  Closes all widgets and cleans up all window system resources.
  Sets \c qApp to 0.
 ----------------------------------------------------------------------------*/

QApplication::~QApplication()
{
    destroy_palettes();
    delete app_pal;
    app_pal = 0;
    delete app_font;
    app_font = 0;
    delete app_cursor;
    app_cursor = 0;
    closing_down = TRUE;
    QWidget::destroyMapper();			// destroy widget mapper
#if defined(CHECK_MEMORY)
    bool prev_mc = memchkSetReporting( FALSE );    
    objectDict->remove( "QObject" );
    memchkSetReporting( prev_mc );
    setName( 0 );
    objectDict->clear();
#endif
    qt_cleanup();
    delete objectDict;
    qApp = 0;
}


/*----------------------------------------------------------------------------
  \fn GUIStyle QApplication::style()
  Returns the GUI style of the application.

  \sa setStyle()
 ----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
  Sets the application GUI style to \e style.

  The style parameter can be \c MacStyle, \c WindowsStyle, \c PMStyle or
  \c MotifStyle.
  Only \c MotifStyle is supported in the current version of Qt.

  \sa style()
 ----------------------------------------------------------------------------*/

void QApplication::setStyle( GUIStyle style )	// set application GUI style
{
#if defined(LINUX_RESTRICTED)
    if ( style != MotifStyle ) {
	warning( "QApplication::setStyle: Only Motif style is supported" );
	return;
    }
#endif
    app_style = style;
}


/*----------------------------------------------------------------------------
  Returns a pointer to the default application palette.	 There will always
  be an application palette, i.e. the returned pointer will never be 0.
 ----------------------------------------------------------------------------*/

QPalette *QApplication::palette()
{
    return app_pal;
}

/*----------------------------------------------------------------------------
  Changes the default application palette to \e palette.

  If \e updateAllWidgets is TRUE, then this palette will be set for
  all existing widgets.
  If \e updateAllWidgets is FALSE (default), then only widgets that
  are created after this call will have the palette.
 ----------------------------------------------------------------------------*/

void QApplication::setPalette( const QPalette &palette, bool updateAllWidgets )
{
    delete app_pal;
    app_pal = new QPalette( palette.copy() );
    CHECK_PTR( app_pal );
    if ( updateAllWidgets && !(starting_up || closing_down) ) {
	QWidgetIntDictIt it( *((QWidgetIntDict*)QWidget::mapper) );
	register QWidget *w;
	while ( (w=it.current()) ) {		// for all widgets...
	    ++it;
	    if ( !w->testWFlags(WType_Desktop) )// (except desktop)
		w->setPalette( *app_pal );
	}
    }
}


/*----------------------------------------------------------------------------
  \fn QCursor *QApplication::cursor()
  Returns the application cursor.
  This function returns 0 if no application cursor has been defined.
  \sa setCursor()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn void QApplication::setCursor( const QCursor &c )
  Sets the application cursor to \e c.

  This cursor will be displayed in all application widgets until
  restoreCursor() is called.

  Example:
  \code
    QApplication::setCursor( waitCursor );
    calculate_mandelbrot();			// lunch time...
    QApplication::restoreCursor();
  \endcode

  \sa cursor(), restoreCursor()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn void QApplication::restoreCursor()
  Restores after some application cursor was set.

  \sa setCursor()
 ----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
  \fn QFont *QApplication::font()
  Returns the default application font.	 There will always be an application
  font, i.e. the returned pointer will never be 0.
  \sa setFont()
 ----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
  Changes the default application font to \e font.

  If \e updateAllWidgets is TRUE, then this font will be set for
  all existing widgets.
  If \e updateAllWidgets is FALSE (default), then only widgets
  created after this call will have this font.
 ----------------------------------------------------------------------------*/

void QApplication::setFont( const QFont &font,	bool updateAllWidgets )
{						// set application font
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


/*----------------------------------------------------------------------------
  Returns display (screen) font metrics for the application font.

  \sa font(), setFont()
 ----------------------------------------------------------------------------*/

QFontMetrics QApplication::fontMetrics()
{
    return desktop()->fontMetrics();
}

/*----------------------------------------------------------------------------
  \fn QWidget *QApplication::desktop()
  Returns the desktop widget (also called root window).

  The desktop widget is useful for obtaining the size of the screen.
  It can also be used to draw on the desktop.

  \code
    QWidget *d = QApplication::desktop();
    d->width();				// returns screen width
    d->height();			// returns screen height
    d->setBackgroundColor( red );	// makes desktop red
  \endcode
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn int QApplication::enter_loop()
  This function enters the main event loop (recursively).
  Do not call it unless you are an expert.
  \sa exit_loop()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn void QApplication::exit_loop()
  This function leaves from a recursive call to the main event loop.
  Do not call it unless you are an expert.
  \sa enter_loop()
 ----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
  \fn QWidget *QApplication::mainWidget() const
  Returns the main application widget.
  \sa setMainWidget()
 ----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
  Tells the application to quit.

  After quit has been called, the application leaves the main event
  loop and returns from the call to exec(). The exec() function
  returns \e retcode.

  By convention, \e retcode 0 means success, any non-zero value indicates
  an error.

  \sa quitApp(), exec()
 ----------------------------------------------------------------------------*/

void QApplication::quit( int retcode )		// quit application
{
    if ( !qApp )				// no global app object
	return;
    if ( qApp->quit_now )			// don't overwrite quit code
	return;
    qApp->quit_now = TRUE;
    qApp->quit_code = retcode;
}


/*----------------------------------------------------------------------------
  Tells the application to quit with exit code 0 (success).
  Equivalent to calling QApplication::quit(0).

  This function is a slot, i.e. you may connect any signal to
  activate quitApp().

  Example:
  \code
    QPushButton *quitButton = new QPushButton;
    connect( quitButton, SIGNAL(clicked()), qApp, SLOT(quitApp()) );
  \endcode  

  \sa quit()
 ----------------------------------------------------------------------------*/

void QApplication::quitApp()			// quit application
{
    quit( 0 );
}


/*----------------------------------------------------------------------------
  \fn bool QApplication::sendEvent( QObject *receiver, QEvent *event )
  Sends an event directly to a receiver, using the notify() function.
  Returns the value that was returned from the event handler.
  \sa postEvent()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn void QApplication::postEvent( QObject *receiver, QEvent *event )
  Stores the event in a queue and returns immediatly.

  Back in the main event loop, all events that are stored in the queue
  will be sent using the notify() function.

  \sa sendEvent()
 ----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
  Sends \e event to \e receiver: <code>receiver->event( event );</code>
  Returns the value that was returned from the event handler.

  All Qt events are sent using the notify function. Since this function
  is virtual, you can make a subclass of QApplication and reimplement
  notify() to get total control of Qt events.

  Installing an event filter on \c qApp is another way of making an
  application-global event hook.

  \sa QObject::event(), installEventFilter()
 ----------------------------------------------------------------------------*/

bool QApplication::notify( QObject *receiver, QEvent *event )
{						// send event to object
#if defined(CHECK_NULL)
    if ( receiver == 0 )			// fatal error
	warning( "QApplication::notify: Unexpected null receiver" );
#endif
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


/*----------------------------------------------------------------------------
  Returns TRUE if an application object has not been created yet.
  \sa closingDown()
 ----------------------------------------------------------------------------*/

bool QApplication::startingUp()			// is application starting up?
{
    return starting_up;
}

/*----------------------------------------------------------------------------
  Returns TRUE if the application objects is being destroyed.
  \sa startingUp()
 ----------------------------------------------------------------------------*/

bool QApplication::closingDown()		// is application closing down?
{
    return closing_down;
}


/*----------------------------------------------------------------------------
  \fn void QApplication::flushX()
  Flushes the X event queue in the X-Windows implementation.
  Does nothing on other platforms.
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn void QApplication::syncX()
  Synchronizes with the X server in the X-Windows implementation.
  Does nothing on other platforms.
 ----------------------------------------------------------------------------*/


#if !defined(_WS_X11_)

// The X implementation of these functions is in qapp_x11.cpp

void QApplication::flushX()	{}		// do nothing

void QApplication::syncX()	{}		// do nothing

#endif

