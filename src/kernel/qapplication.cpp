/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qapplication.cpp#31 $
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
static char ident[] = "$Id: //depot/qt/main/src/kernel/qapplication.cpp#31 $";
#endif


/*!
\class QApplication qapp.h
\brief The QApplication class manages the application event queue.

The QApplication class is central to Qt.  It receives events from
the underlying window system and sends them to the destination widgets.
An application object must be created before any widgets can be created!

Only one single QApplication object should be created, and this is
normally done in the main() function.  When a QApplication object has
been created, \c qApp (defined as <code>extern QApplication
*qApp</code>) will refer to this object.

Here is a complete Qt application:
\code
#include <qapp.h>
#include <qpushbt.h>

int main( int argc, char **argv )
{
    QApplication a( argc, argv );       \/ create app object
    QPushButton  hi( "Hello, world" );  \/ create main widget
    hi.show();                          \/ show widget
    return a.exec( &hi );               \/ main event loop
}
\endcode

Notice that the QApplication object must be created before the widget is
defined!

\header qkeycode.h
\header qwindefs.h
\header qglobal.h */


void qt_init( int *, char ** );			// defined in qapp_???.cpp
void qt_cleanup();

QApplication *qApp = 0;				// global application object
QWidget  *QApplication::main_widget   = 0;	// main application widget
QWidget  *QApplication::focus_widget  = 0;	// has keyboard input focus
QCursor  *QApplication::appCursor     = 0;	// default application cursor
QPalette *QApplication::appPal	      = 0;	// default application palette
QFont    *QApplication::appFont       = 0;	// default application font
bool	  QApplication::starting_up   = TRUE;	// app starting up
bool	  QApplication::closing_down  = FALSE;	// app closing down


#if defined(_WS_MAC_)
GUIStyle QApplication::appStyle = MacStyle;	// default style for Mac
#elif defined(_WS_WIN_)
GUIStyle QApplication::appStyle = WindowsStyle;	// default style for Windows
#elif defined(_WS_PM_)
GUIStyle QApplication::appStyle = PMStyle;	// default style for OS/2 PM
#elif defined(_WS_X11_)
GUIStyle QApplication::appStyle = MotifStyle;	// default style for X Windows
#endif


static QPalette *motifPalette = 0;

static void create_palettes()			// creates default palettes
{
    QColor lightBlue = blue.light();
    QColorGroup motif_nor( black, lightGray, lightGray.light(),
			   lightGray.dark(), gray, black, white );
    QColorGroup motif_dis( black, lightGray, lightGray.light(),
			   lightGray.dark(), gray, black, white );
    QColorGroup motif_act( black, lightGray, lightGray.light(),
			   lightGray.dark(), gray, black, white );
    motifPalette = new QPalette( motif_nor, motif_dis, motif_act );
}

static void destroy_palettes()
{
    delete motifPalette;
}


/*!
Creates an application object with the command line parameters \e argc
and \e argv.  The <code>qApp</code> will be set to point to this
application object.

Only one application object should be created.
This application object must be constructed before any paint devices
(includes widgets, pixmaps, bitmaps etc.)

The UNIX/X-Windows version of Qt recognizes these command line options:
<ul>
<li> -display \e display, Sets the X display (default is $DISPLAY).
<li> -fn or -font \e font, Defines the application font.
<li> -bg or -background \e color, Sets the default background color.
<li> -fg or -foreground \e color, Sets the default foreground color.
<li> -name \e name, Sets the application name.
<li> -title \e title, Sets the application title (caption).
<li> -sync, Turns X into synchronous mode for debugging.
</ul>
*/

QApplication::QApplication( int &argc, char **argv )
{
#if defined(CHECK_STATE)
    if ( qApp )
	warning( "QApplication: There should be only one application object" );
#endif
    qt_init( &argc, argv );
    quit_now = FALSE;
    quit_code = 0;
    qApp = this;
    if ( !appPal ) {				// palette not already set
	create_palettes();
	appPal = new QPalette( *motifPalette );
	CHECK_PTR( appPal );
    }
    if ( !appFont ) {				// font not already set
	appFont = new QFont;
	CHECK_PTR( appFont );
    }
    QWidget::createMapper();			// create widget mapper
    starting_up = FALSE;			// no longer starting up
}

/*!
Closes all widgets and cleans up all window system resources.
Sets <code>qApp</code> to 0.
*/

QApplication::~QApplication()
{
    destroy_palettes();
    delete appPal;
    appPal = 0;
    delete appFont;
    appFont = 0;
    delete appCursor;
    appCursor = 0;
    qApp = 0;
    closing_down = TRUE;
    QWidget::destroyMapper();			// destroy widget mapper
    delete objectDict;
    qt_cleanup();
}


/*!
\fn GUIStyle QApplication::style()
Returns the GUI style of the application.

\sa setStyle().
*/


/*!
Sets the application GUI style to \e s.

The style parameter can be MacStyle, WindowsStyle, PMStyle or MotifStyle.
Only MotifStyle is fully supported in the current version of Qt.

\sa style().
*/

void QApplication::setStyle( GUIStyle s )	// set application GUI style
{
    appStyle = s;
}


/*!
Returns a pointer to the default application palette.  There will always
be an application palette.
*/

QPalette *QApplication::palette()		// get application palette
{
    return appPal;
}

/*!
Changes the default application palette to \e p.

If \e updateAllWidgets is TRUE, then this palette will be set for
all existing widgets.
If \e updateAllWidgets is FALSE (default), then only new widgets
become this palette.
*/

void QApplication::setPalette( const QPalette &p, bool updateAllWidgets )
{						// set application palette
    delete appPal;
    appPal = new QPalette( p.copy() );
    CHECK_PTR( appPal );
    if ( updateAllWidgets && !(starting_up || closing_down) ) {
	QWidgetIntDictIt it( *((QWidgetIntDict*)QWidget::mapper) );
	register QWidget *w;
	while ( (w=it.current()) ) {		// for all widgets...
	    ++it;
	    if ( !w->testFlag(WType_Desktop) )	// (except desktop)
		w->setPalette( *appPal );
	}
    }
}


/*!
\fn QCursor *QApplication::cursor()
Returns the application cursor.  This function returns 0 if no application
cursor has been defined.

\sa setCursor().
*/

/*!
\fn void QApplication::setCursor( const QCursor &c )
Sets the application cursor to \e c.

This cursor will be displayed in all application widgets until restoreCursor()
is called.

Example of use:
\code
QApplication::setCursor( hourGlassCursor );
calculate_mandelbrot();		\/ this takes time...
QApplication::restoreCursor();
\endcode

\sa cursor(), restoreCursor().
*/

/*!
\fn void QApplication::restoreCursor()
Restores after some application cursor was set.

\sa setCursor().
*/


/*!
\fn QFont *QApplication::font()
Returns the default application font.  There will always be an application
font.

\sa setFont().
*/


/*!
Changes the default application font to \e f.

If \e updateAllWidgets is TRUE, then this font will be set for
all existing widgets.
If \e updateAllWidgets is FALSE (default), then only new widgets
become this font.
*/

void QApplication::setFont( const QFont &f,  bool updateAllWidgets )
{						// set application font
    if ( appFont )
	delete appFont;
    appFont = new QFont( f );
    CHECK_PTR( appFont );
    QFont::setDefaultFont( *appFont );
    if ( updateAllWidgets ) {			// set for all widgets now
	QWidgetIntDictIt it( *((QWidgetIntDict*)QWidget::mapper) );
	register QWidget *w;
	while ( (w=it.current()) ) {		// for all widgets...
	    ++it;
	    if ( !w->testFlag(WType_Desktop) )	// (except desktop)
		w->setFont( *appFont );
	}
    }
}


/*!
\fn QWidget *QApplication::desktop()
Returns the desktop widget (also called root window).

The desktop widget is useful for obtaining the size of the screen.
It can also be used to draw on the desktop.

\code
QWidget *d = QApplication::desktop();
d->height();			\/ height of display
d->width();			\/ width of display
d->setBackgroundColor( red );
\endcode
*/


/*! \fn int QApplication::exec( QWidget *mainWidget )

  Enters the main event loop and waits until quit() is called or \e 
  mainWidget is destroyed. Returns the value that was specified to quit().

  (As a special case, modal widgets like QMessageBox can be used even
  without calling exec().  This can e.g. be used to write a very short
  "hello world" but in general, all programs must call exec().) */

/*!
\fn int QApplication::enter_loop()
This function enters the main event loop (recursively).
Do not call it unless you are an expert.

\sa exit_loop().
*/

/*!
\fn void QApplication::exit_loop()
This function leaves from a recursive call to the main event loop.
Do not call it unless you are an expert.

\sa enter_loop().
*/


/*!
\fn QWidget *QApplication::mainWidget() const
Returns the widget that was specified to exec().
*/


/*!
Tells the application to quit.

After quit has been called, the application leaves the main event
loop an returns from the call to exec(). The exec() function
returns \e retcode.

\sa exec().
*/

void QApplication::quit( int retcode )		// quit application
{
    if ( !qApp )				// no global app object
	return;
    if ( qApp->quit_now )			// don't overwrite quit code
	return;
    qApp->quit_now = TRUE;
    qApp->quit_code = retcode;
}


/*!
\fn bool QApplication::sendEvent( QObject *receiver, QEvent *event )
Sends an event directly to a receiver, using the notify() function.
Returns the value that was returned from the event handler.

\sa postEvent().
*/

/*!
\fn void QApplication::postEvent( QObject *receiver, QEvent *event )
Stores the event in a queue and returns immediatly.

Back in the main event loop, all events that are stored in the queue
will be sent using the notify() function.

\sa sendEvent().
*/


/*!
Sends \e event to \e receiver: <code>receiver->event( event );</code>
Returns the value that was returned from the event handler.

All Qt events are sent using the notify function. Since this function
is virtual, you can make a subclass of QApplication and reimplement
notify() to get total control of Qt events.

\sa QObject::event().
*/

bool QApplication::notify( QObject *receiver, QEvent *event )
{						// send event to object
#if defined(CHECK_NULL)
    if ( receiver == 0 )			// fatal error
	warning( "QApplication::notify: Unexpected NULL receiver" );
#endif
    return receiver->event( event );
}


/*!
Returns TRUE if an application object has not been created yet.
*/

bool QApplication::startingUp()			// is application starting up?
{
    return starting_up;
}

/*!
Returns TRUE if the application objects is being destroyed.
*/

bool QApplication::closingDown()		// is application closing down?
{
    return closing_down;
}


/*!
\fn void QApplication::flushX()
Flushes the X event queue in the X-Windows implementation.
Does nothing on other platforms.
*/

/*!
\fn void QApplication::syncX()
Synchronizes with the X server in the X-Windows implementation.
Does nothing on other platforms.
*/


#if !defined(_WS_X11_)

// The X implementation of these functions is in qapp_x11.cpp

void QApplication::flushX()	{}		// do nothing

void QApplication::syncX()	{}		// do nothing

#endif

/*! \page index.html

<title>Qt toolkit - documentation home page</title>
</head><body>

<h1>Qt documentation home page</h1>

The Qt toolkit documentation is organized as one HTML page per class,
header file, example program, or topical documentation files.
<p>
There are several different index pages: <ul>
<li><a href=classes.html>Alphabetical class list</a>
<li><a href=hierarchy.html>The classes according to inheritance</a>
<li><a href=headers.html>All the header files</a>
<li><a href=enums.html>All the enum types</a>
<li><a href=examples.html>All the example programs</a>
</ul>

<p>There are also pages on these topics:<ul>
<li><a href=fontmatch.html>Font matching</a>
<li><a href=handleclass.html>Handle classes</a>
<li><a href=metaobjects.html>Signals, slots and the Meta Object Compiler</a>
</ul> */

/*! \example wheel.cpp

  <h1>Color Wheel</h1>

  This example draws a color wheel.  It shows how to use color, world
  transformation and QPainter. */

