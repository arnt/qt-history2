/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qapplication.cpp#41 $
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
static char ident[] = "$Id: //depot/qt/main/src/kernel/qapplication.cpp#41 $";
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
<li> -geometry \e geometry, Sets the client geometry of the 
      \link exec main widget. \endlink
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
#if defined(LINUX_RESTRICTED)
    if ( s != MotifStyle ) {
	warning( "QApplication::setStyle: Only Motif style is supported" );
	return;
    }
#endif
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
If \e updateAllWidgets is FALSE (default), then only widgets that
are created after this call will have the palette.
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
If \e updateAllWidgets is FALSE (default), then only widgets
created after this call will have this font.
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

<h1>Qt Documentation Home Page</h1>

<strong> This documentation is far from complete.  Remember, Qt is
still not at version 1.0. </strong>

<p>

Here are the main index pages: <ul>
<li><a href=hierarchy.html>Class overview</a>
<li><a href=classes.html>Alphabetical list</a>
<li><a href=topicals.html>Other index pages</a>
</ul>

<p>

Here are some introductory pages and pages about the central concepts
in Qt: <ul>
<li><a href=faq.html>FAQ</a>
<li><a href=metaobjects.html>Signals, slots and the Meta Object Compiler</a>
<li><a href=fontmatch.html>Font matching</a>
</ul>

<p>

And, finally, some meta-information: <ul>
<li><a href=troll.html>Troll Tech contact information</a>
<li><a href=license.html>License statement for Qt</a>
</ul>

<p>

There is a mailing list for Qt users.  Send a message containing the
single word "subscribe" to <a
href=mailto:qt-interest-request@nvg.unit.no>qt-interest-request@nvg.unit.no</a>
to join the list.  You will receive a receipt from the list server
within a few minutes. */


/*! \page topicals.html

<title>Qt toolkit - topical index pages</title>
</head><body>

<h1>Topical Index pages</h1>

<ul>
<li><a href=tools.html>General utility classes</a>
<li><a href=headers.html>Header files</a>
<li><a href=enums.html>enum types</a>
<li><a href=examples.html>Example programs</a>
</ul>

*/

/*! \page license.html

<title>Qt License Statement</title>

<h1>All Rights Reserved</h1>

Copyright 1992-1995 Troll Tech AS.  All rights reserved.

<p>

Qt is a product of Troll Tech AS and is provided for use on computers
running the Linux operating system.

<p>

Users may copy this beta version of the Qt toolkit provided that the
entire archive is distributed as a whole, including this notice.

<p>

Users may use the Qt toolkit to create programs provided that these
programs are either for internal/own use or freely distributable.  THIS
BETA VERSION OF QT MAY NOT BE USED IN COMMERCIAL PROGRAMS.

<p>

Troll Tech makes no obligation to support or upgrade Qt, or assist in
use of Qt.

<p>

In no event shall Troll Tech be liable for any lost revenue or profits or
other special, indirect or consequential damages, even if Troll Tech has
been advised of the possibility of such damages.

<p><strong>

QT IS PROVIDED AS IS WITH NO WARRANTY OF ANY KIND, INCLUDING THE WARRANY
OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.

</strong>

*/

/*! \page troll.html

<title>Troll Tech Contact Information</title>

<h1>Contact Information for Troll Tech</h1>

<h2>Snail mail</h2>

Troll Tech<br>
Postboks 6133 Etterstad<br>
N-0602 Oslo<br>
Norway<br>

<h2>Electronically</h2>

Fax: +47 22 64 69 49.

<p>

E-mail: <a href=mailto:info@troll.no>info@troll.no</a> for general
enquiries, <a href=mailto:www@troll.no>www@troll.no</a> for comments
about our WWW pages, or <a
href=mailto:qt-bugs@troll.no>qt-bugs@troll.no</a> for Qt bug reports
and other Qt-related mail. */

/*! \page qt.html

<title>Qt toolkit - overview</title>

This is the announcement of Qt 0.9, the first public beta version.

<h1>Qt announcement</h1>

Troll Tech is proud to relase a free beta-version of Qt (pronounced:
"cute") for X-Windows/Linux.

<p>

Qt is an object-oriented framework for developing graphical user interface
applications.

<p>

It is ftp'able from sunsite.unc.edu in /pub/Linux/Incoming, hopefully
soon /pub/Linux/devel/c++, and from ftp.nvg.unit.no in /pub/linux/qt.

<p>

The documentation can be browsed on the web at <a href=http://www.troll.no/>
http://www.troll.no/</a> where you will also find other Qt-related links.

<p>

About Qt:

<p>

Qt consists of a rich C++ library (around 100 classes) and a meta-object
system that extends C++ with new concepts called signals and slots.
Signals and slots define clean and natural object interfaces for creating
independent objects.
This makes Qt very suitable for true component programming.

<p>

A "hello world" application under Qt is only 8 lines of C++ code:

<pre>
    #include <qmsgbox.h>
    #include <qapp.h>

    int main( int argc, char **argv )
    {
        QApplication a( argc, argv );
        return QMessageBox::message( "Attention", "Hello, world!" );
    }
</pre>

Qt dramatically cuts down on development time and complexity in
writing user interface software for X-Windows/Linux.
It allows the programmer to focus directly on the programming task,
and not mess around with low-level X11 code.

<p>

Qt is very fast and compact because it is based direcly on Xlib and not
Motif or X Intrinsics.
Qt's widgets (user interface objects) emulate Motif look and feel.

<p>

Qt supports advanced features such as drawing transformed graphics,
including drawing rotated text and pixmaps.
Pixmaps can be loaded and saved using several image formats.
An image class makes it easy to implement image processing algorithms.

<p>

Qt is definitely not a toy. It is a professional product that compares
well to any commercial GUI class library.

<p>

If you find any bugs, send a report to qt-bugs@troll.no.

<p>

If you create a nice Qt program and want us to distribute it as a
contribution to Qt, send us a line at info@troll.no.

<p>


Here are the main features of Qt:

<ul>
<li> Qt Widgets:
<ul>
<li>    Button
<li>    Button group
<li>    Check box
<li>    Combo box
<li>    Dialog
<li>    Frame
<li>    Group box
<li>    Label
<li>    LCD number
<li>    Line editor
<li>    List box
<li>    Menu bar
<li>    Message box
<li>    Popup menu
<li>    Push button
<li>    Radio button
<li>    Scroll bar
<li>    Table
<li>    View
</ul>


<li> User Interface Functionality:
<ul>
<li>   Accelerators
<li>   Color support:
<ul>
<li>	RGB-based color
<li>	HSV conversions
<li>	Color group
<li>	Palette
</ul>
<li>   Cursor
<li>   Font
<li>   Painter
<li>   Paint devices:
<ul>
<li>	Widget
<li>	Pixmap/Bitmap
<li>	Picture (meta-file)
</ul>
<li>   Painter tools:
<ul>
<li>	Pen
<li>	Brush
<li>	Region
<li>	Transform Matrix
</ul>
<li>   Image (abstract pixmap)
<li>   Data types:
<ul>
<li>	Point
<li>	Size
<li>	Rect
<li>	Point array
</ul>
<li>   Events:
<ul>
<li>	Mouse events
<li>	Keyboard events
<li>	Timer events
<li>	and more...
</ul>
</ul>
<li> General Toolkit:
<ul>
<li>   Array
<li>   Bit array
<li>   Byte array
<li>   String
<li>   Date and time classes
<li>   Template/macro-based collections and iterators:
<ul>
<li>	List and List iterator
<li>	Dict and Dict iterator
<li>	Queue
<li>	Stack
<li>	Vector
<li>	Cache and Cache iterator
</ul>
<li>   IO devices:
<ul>
<li>	Buffer
<li>	File
</ul>
<li>   Binary IO stream (for serialization)
<li>   Text IO stream
<li>   Regular expression parsing
</ul>
</ul>
*/

/*! \defgroup tools.html

  <h1>The Tool Classes</h1>

  The tool classes are general-purpose classes that may be used
  independently of the GUI classes.

  */

/*! \example wheel.cpp

  <h1>Color Wheel</h1>

  This example draws a color wheel.  It shows how to use color, world
  transformation and QPainter. */

/*! \example connect.cpp

  <h1>A graphical hello world</h1>

  This example shows very simple mouse-based user interaction and
  painting without any world transform matrix or other advanced
  features.  Run the program, click the button, move the mouse,
  release the button, and watch the lines get drawn. */

/*! \example cursor.cpp

  <h1>Cursors</h1>

  This example shows how to do tricks with the mouse cursor.
*/

