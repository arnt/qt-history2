/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qapplication_mac.cpp
**
** Implementation of Mac startup routines and event handling
**
** Created : 001018
**
** Copyrigght (C) 1992-2000 Trolltech AS.  All rights reserved.
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
** licenses for Unix/X11/FIXME may use this file in accordance with the Qt Commercial
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

// NOT REVISED

#define select		_qt_hide_select
#define gettimeofday	_qt_hide_gettimeofday

#include <stdio.h>
#include "qglobal.h"

// FIXME: These mac includes can be replaced by a single Carbon.h include
#include <Events.h>
#include <Quickdraw.h>
#include <Menus.h>
#include <Fonts.h>
#include <MacTypes.h>
#include <ToolUtils.h>
#include <MacWindows.h>
#include <Timer.h>

#include "qapplication.h"
#include "qapplication_p.h"
#include "qcolor_p.h"
#include "qwidget.h"
#include "qwidget_p.h"
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
#include "qguardedptr.h"
#include "qclipboard.h"
#include "qwhatsthis.h" // ######## dependency
#include "qwindowsstyle.h" // ######## dependency
#include "qmotifplusstyle.h" // ######## dependency
#include "qpaintdevicemetrics.h"

#define	 GC GC_QQQ

#if defined(QT_THREAD_SUPPORT)
#include "qthread.h"
#endif


/*****************************************************************************
  Internal variables and functions
 *****************************************************************************/

static int mouse_button_state = 0;
static char    *appName;                        // application name
QObject	       *qt_clipboard = 0;
QWidget	       *qt_button_down	 = 0;		// widget got last button-down


// one day in the future we will be able to have static objects in libraries....
struct QScrollInProgress {
    static long serial;
    QScrollInProgress( QWidget* w, int x, int y ) :
    id( serial++ ), scrolled_widget( w ), dx( x ), dy( y ) {}
    long id;
    QWidget* scrolled_widget;
    int dx, dy;
};
long QScrollInProgress::serial=0;

class QETWidget : public QWidget		// event translator widget
{
public:
    void setWState( WFlags f )		{ QWidget::setWState(f); }
    void clearWState( WFlags f )	{ QWidget::clearWState(f); }
    void setWFlags( WFlags f )		{ QWidget::setWFlags(f); }
    void clearWFlags( WFlags f )	{ QWidget::clearWFlags(f); }
};


UnsignedWide thesecs;

void qt_init( int* /* argcptr */, char **argv, QApplication::Type )
{
    qDebug( "QApplication::qt_init" );
    char *p;

    // Set application name
   
    p = strrchr( argv[0], '/' );
    appName = p ? p + 1 : argv[0];

// FIXME: which of these are needed and why?
//	InitGraf(&qd.thePort);
//	InitWindows();
	InitCursor();
//	InitMenus();
//	InitFonts();
    if ( qt_is_gui_used ) {
        QColor::initialize();
        QFont::initialize();
        QCursor::initialize();
        QPainter::initialize();
    }
    Microseconds( &thesecs );
    if ( qt_is_gui_used ) {
        qApp->setName( appName );   
    }
}



/*****************************************************************************
  qt_cleanup() - cleans up when the application is finished
 *****************************************************************************/

void qt_cleanup()
{
    qDebug( "qt_cleanup()" );
}

bool qt_set_socket_handler( int, int, QObject *, bool )
{
    qDebug( "qt_set_socket_handler" );
    return FALSE;
}

/*****************************************************************************
  Platform specific global and internal functions
 *****************************************************************************/

void qt_save_rootinfo()				// save new root info
{
    qDebug( "qt_save_rootinfo" );
}

void qt_updated_rootinfo()
{
    qDebug( "qt_update_rootinfo" );
}

bool qt_wstate_iconified( WId )
{
    qDebug( "qt_wstate_iconified" );
    return FALSE;
}

/*!
  \relates QApplication

  Adds a global routine that will be called from the QApplication
  destructor.  This function is normally used to add cleanup routines
  for program-wide functionality.

  The function given by \a p should take no arguments and return
  nothing, like this:
  \code
    static int *global_ptr = 0;

    static void cleanup_ptr()
    {
	delete [] global_ptr;
	global_ptr = 0;
    }

    void init_ptr()
    {
	global_ptr = new int[100];	// allocate data
	qAddPostRoutine( cleanup_ptr );	// delete later
    }
  \endcode

  Note that for an application- or module-wide cleanup,
  qAddPostRoutine() is often not suitable.  People have a tendency to
  make such modules dynamically loaded, and then unload those modules
  long before the QApplication destructor is called, for example.

  For modules and libraries, using a reference-counted initialization
  manager or Qt' parent-child delete mechanism may be better.  Here is
  an example of a private class which uses the parent-child mechanism
  to call a cleanup function at the right time:

  \code
    class MyPrivateInitStuff: public QObject {
    private:
        MyPrivateInitStuff( QObject * parent ): QObject( parent) {
	    // initialization goes here
	}
	MyPrivateInitStuff * p;

    public:
        static MyPrivateInitStuff * initStuff( QObject * parent ) {
	    if ( !p )
	        p = new MyPrivateInitStuff( parent );
	    return p;
	}

        ~MyPrivateInitStuff() {
	    // cleanup (the "post routine") goes here
	}
    }
  \endcode

  By selecting the right parent widget/object, this can often be made
  to clean up the module's data at the exact right moment.
*/

void qAddPostRoutine( Q_CleanUpFunction )
{
    qDebug( "qAddPostRoutine" );
}


void qRemovePostRoutine( Q_CleanUpFunction )
{
    qDebug( "qRemovePostRoutine" );
}


/*****************************************************************************
  Platform specific QApplication members
 *****************************************************************************/

/*!
  \fn QWidget *QApplication::mainWidget() const

  Returns the main application widget, or a null pointer if there is
  not a defined main widget.

  \sa setMainWidget()
*/

/*!
  Sets the main widget of the application.

  The main widget is like any other, in most respects except that if
  it is deleted, the application exits.

  You need not have a main widget; connecting lastWindowClosed() to quit() is
  another alternative.

  For X11, this function also resizes and moves the main widget
  according to the \e -geometry command-line option, so you should set
  the default geometry (using \l QWidget::setGeometry()) before
  calling setMainWidget().

  \sa mainWidget(), exec(), quit()
*/
extern QWidget * mac_mouse_grabber;
extern QWidget * mac_keyboard_grabber;

void QApplication::setMainWidget( QWidget *mainWidget )
{
    qDebug( "QApplication::setMainWidget" );
    main_widget = mainWidget;
}

#ifndef QT_NO_CURSOR

/*****************************************************************************
  QApplication cursor stack
 *****************************************************************************/

/*!
  \fn QCursor *QApplication::overrideCursor()

  Returns the active application override cursor.

  This function returns 0 if no application cursor has been defined
  (i.e. the internal cursor stack is empty).

  \sa setOverrideCursor(), restoreOverrideCursor()
*/

/*!
  Sets the application override cursor to \a cursor.

  Application override cursors are intended for showing the user that
  the application is in a special state, for example during an
  operation that might take some time.

  This cursor will be displayed in all the widgets of the application
  until restoreOverrideCursor() or another setOverrideCursor() is
  called.

  Application cursors are stored on an internal stack.
  setOverrideCursor() pushes the cursor onto the stack, and
  restoreOverrideCursor() pops the active cursor off the stack. Every
  setOverrideCursor() must eventually be followed by a corresponding
  restoreOverrideCursor(), otherwise the stack will never be emptied.

  If \a replace is TRUE, the new cursor will replace the last override
  cursor (the stack keeps its depth). If \a replace is FALSE, the new
  stack is pushed onto the top of the stack.

  Example:
  \code
    QApplication::setOverrideCursor( Qt::waitCursor );
    calculateHugeMandelbrot();			// lunch time...
    QApplication::restoreOverrideCursor();
  \endcode

  \sa overrideCursor(), restoreOverrideCursor(), QWidget::setCursor()
*/

void QApplication::setOverrideCursor( const QCursor &, bool )
{
    qDebug( "QApplication::setOverrideCursor" );
}

/*!
  Undoes the last setOverrideCursor().

  If setOverrideCursor() has been called twice, calling
  restoreOverrideCursor() will activate the first cursor set. Calling
  this function a second time restores the original widgets cursors.

  \sa setOverrideCursor(), overrideCursor().
*/

void QApplication::restoreOverrideCursor()
{
    qDebug( "QApplication::restoreOverrideCursor" );
}

#endif

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

  This function uses an internal counter.  Each
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

void QApplication::setGlobalMouseTracking( bool )
{
    qDebug( "QApplication::setGlobalMouseTracking" );
}


static QWidget *recursive_match(QWidget *widg, int x, int y)
{
  // Keep looking until we find ourselves in a widget with no kiddies
  // where the x,y is
  if(!widg)
    return 0;

  const QObjectList *objl=widg->children();
  if(!objl) // No children
    return widg;
  for(QObjectListIt it(*objl); it.current(); ++it) {
    if((*it)->inherits("QWidget")) {
      QWidget *curwidg=(QWidget *)(*it);
      int wx=curwidg->x(), wy=curwidg->y();
      int wx2=wx+curwidg->width(), wy2=wy+curwidg->height();
      qDebug("recursive_match %d %d  %d %d  %d %d",curwidg->x(), curwidg->y(),wx,wy,wx2,wy2);
      if(x>=wx && y>=wy && x<=wx2 && y<=wy2) {
	return recursive_match(curwidg,x-wx,y-wy);
      }
    }
  }
  // If we get here, it's within a widget that has children, but isn't in any
  // of the children
  return widg;
}

/*!
  Returns a pointer to the widget at global screen position \a (x,y), or a
  null pointer if there is no Qt widget there.

  If \a child is FALSE and there is a child widget at position \a
  (x,y), the top-level widget containing it is returned. If \a child
  is TRUE the child widget at position \a (x,y) is returned.

  This function is normally rather slow.

  \sa QCursor::pos(), QWidget::grabMouse(), QWidget::grabKeyboard()
*/
QWidget *QApplication::widgetAt( int x, int y, bool child)
{
    // Need to handle child/top-level stuff - right now only do top-levels
    Point p;
    p.h=x;
    p.v=y;
    WindowPtr wp;
    FindWindow(p,&wp);
    QWidget * widget=QWidget::find((WId)wp);
    if(!widget) {
	qWarning("Couldn't find %d",wp);
	return 0;
    }
    if(!child) {
	return widget;
    } else {
	return recursive_match(widget,x,y);
    }
}

/*!
  \overload QWidget *QApplication::widgetAt( const QPoint &pos, bool child )
*/


/*!
  Sounds the bell, using the default volume and sound.
*/

void QApplication::beep()
{
    qDebug( "QApplication::beep" );
}


/*****************************************************************************
  Main event loop
 *****************************************************************************/

/*!
  Enters the main event loop and waits until exit() is called or the
  main widget is destroyed, and Returns the value that was set via to
  exit() (which is 0 if exit() is called via quit()).

  It is necessary to call this function to start event handling. The
  main event loop receives events from the window system and
  dispatches these to the application widgets.

  Generally speaking, no user interaction can take place before
  calling exec(). As a special case, modal widgets like QMessageBox
  can be used before calling exec(), because modal widgets call exec()
  to start a local event loop.

  To make your application perform idle processing, i.e. executing a
  special function whenever there are no pending events, use a QTimer
  with 0 timeout. More advanced idle processing schemes can be
  achieved by using processEvents() and processOneEvent().

  \sa quit(), exit(), processEvents(), setMainWidget()
*/

int QApplication::exec()
{
    qDebug( "QApplication::exec" );
    quit_now = FALSE;
    quit_code = 0;

#if defined(QT_THREAD_SUPPORT)
    qApp->unlock(FALSE);
#endif

    enter_loop();

    return quit_code;
}


/*!
  Processes the next event and returns TRUE if there was an event
  (excluding posted events or zero-timer events) to process.

  This function returns immediately if \a canWait is FALSE. It might go
  into a sleep/wait state if \a canWait is TRUE.

  \sa processEvents()
*/

bool QApplication::processNextEvent( bool )
{
    qDebug( "QApplication::processNextEvent" );
    EventRecord event;
    sendPostedEvents();
    do {
	WaitNextEvent ( everyEvent, &event, 15L, nil);
	if ( event.what == nullEvent ) {
	    macProcessEvent( (MSG *)(&event) );
	}
    } while( event.what == nullEvent );
    if ( macProcessEvent( (MSG *)(&event)) == 1 )
	return TRUE;
    if ( quit_now || app_exit_loop )
	return FALSE;
    sendPostedEvents();
    return TRUE;
}


static const int KeyTbl[]={
    144667,          Qt::Key_Escape,         // misc keys
    143369,             Qt::Key_Tab,
    144136,       Qt::Key_Backspace,
    140301,          Qt::Key_Return,
    160513,            Qt::Key_Home,           // cursor movement
    162588,            Qt::Key_Left,
    163358,              Qt::Key_Up,
    162845,           Qt::Key_Right,
    163103,            Qt::Key_Down,
    160779,           Qt::Key_Prior,
    162060,            Qt::Key_Next,
    /*
      0,         Qt::Key_Shift,          // modifiers
      0,       Qt::Key_Control,
      0,       Qt::Key_Control,
      0,          Qt::Key_Meta,
      0,          Qt::Key_Meta,
      0,           Qt::Key_Alt,
      0,           Qt::Key_Alt,
      0,       Qt::Key_CapsLock,
      0,        Qt::Key_NumLock,
      0,     Qt::Key_ScrollLock,
    */
    143648,        Qt::Key_Space,          // numeric keypad
    150531,        Qt::Key_Enter,
    151869,        Qt::Key_Equal,
    148266,     Qt::Key_Asterisk,
    148779,          Qt::Key_Plus,
    151085,     Qt::Key_Minus,
    147758,      Qt::Key_Period,
    150319,       Qt::Key_Slash,
    162320, Qt::Key_F1,       // Function keys
    161808, Qt::Key_F2,
    156432, Qt::Key_F3,
    161296, Qt::Key_F4,
    155664, Qt::Key_F5,
    155920, Qt::Key_F6,
    156176, Qt::Key_F7,
    156688, Qt::Key_F8,
    156944, Qt::Key_F9,
    158992, Qt::Key_F10,
    157456, Qt::Key_F11,
    159504, Qt::Key_F12,
    149275, Qt::Key_NumLock,
    133796, Qt::Key_plusminus,
    0,0
};

int get_key(int key)
{
  for(int i = 0; KeyTbl[i]; i+=2) {
	if(key==KeyTbl[i]) {
	    return KeyTbl[i+1];
	}
    }
    return 0;
}

bool mouse_down=false;
extern WId myactive;

bool QApplication::do_mouse_down( EventRecord* es )
{
  qDebug("QApplication::do_mouse_down");
  EventRecord *er = (EventRecord *)es;
  WindowPtr wp;
  short windowPart;
  Point wherePoint = er->where;
  windowPart = FindWindow( er->where, &wp );
  QWidget *widget;
  int growWindowSize = 0;
  bool in_widget = FALSE;

  switch( windowPart ) {
  case inGoAway:
    widget=QWidget::find( (WId)wp );
    if( widget ) {
      if( widget->close( FALSE ) ) {
	widget->hide();
      } else {
	qDebug( "do_mouse_down: widget not found" );
      }
    } else {
      qWarning("Close for unknown widget");
    }
    break;
  case inDrag:
    DragWindow( wp, er->where, 0 );
    break;
  case inContent:
    in_widget = TRUE;
    break;
  case inGrow:
    qDebug("inGrow");
    Rect limits;
    widget = QWidget::find( (WId)wp );
   
    if( widget ) {
      qDebug( "Widget found" );
      if ( widget->extra ) {
	SetRect( &limits, widget->extra->minw, widget->extra->minh,
		 widget->extra->maxw, widget->extra->maxh);
      }
    }
    growWindowSize = GrowWindow( wp, wherePoint, &limits);
    if( growWindowSize) {
      // nw/nh might not match the actual size if setSizeIncrement
      // is used
      int nw = LoWord( growWindowSize );
      int nh = HiWord( growWindowSize );

      // FIXME: Replace hard coded constants with display size 
      if( nw < 1600 && nw > 0 && nh < 1200 && nh > 0 ) {
	if( widget ) {
	  int ow = widget->width(), oh = widget->height();
	  widget->resize( nw, nh );

	  QResizeEvent qre( QSize( widget->width(), widget->height() ), QSize( ow, oh) );
	  QApplication::sendEvent( widget, &qre );
	  widget->resizeEvent( &qre );
	}
      }
    }
    break;
  case inZoomIn:
  case inZoomOut:
    if( TrackBox( wp, er->where, windowPart ) == true ) {
      Rect bounds;
      SetPortWindowPort( wp );
      GetPortBounds( GetWindowPort( wp ), &bounds );
      EraseRect( &bounds );
      ZoomWindow( wp, windowPart, false);
      InvalWindowRect( wp, &bounds );
    }
    break;
  }
  return in_widget;
}

RgnHandle cliprgn=0;
bool ignorecliprgn=true;

int QApplication::macProcessEvent(MSG * m)
{
  WindowPtr wp;
  EventRecord *er = (EventRecord *)m;
  QWidget *twidget = QWidget::find( (WId)er->message );
  Point p2 = er->where;
  QWidget * widget = QApplication::widgetAt(p2.h,p2.v,true);
  if(!er->what)
    return 0;

  if ( er->what == updateEvt ) {
    qDebug( "Update Event" );
    wp = (WindowPtr)er->message;
    SetPortWindowPort(wp);
    SetOrigin( 0, 0 );

    if(!twidget) {
      qWarning("Couldn't find paint widget for %d!",(int)wp);
    } else {
      int metricWidth = twidget->metric (QPaintDeviceMetrics::PdmWidth );
      int metricHeight = twidget->metric( QPaintDeviceMetrics::PdmHeight );
      twidget->crect.setWidth( metricWidth - 1 );
      twidget->crect.setHeight( metricHeight - 1 );
      ignorecliprgn = false;
      twidget->propagateUpdates( 0, 0, twidget->width(), twidget->height() );
      ignorecliprgn = true;
    }
    qDebug( "~Update Event" );

  } else if( er->what == mouseDown ) {
    short part = FindWindow( er->where, &wp );
    if( part == inContent ) {
      if( mac_mouse_grabber ) {
	widget = mac_mouse_grabber;
      } else {
	Point pp2 = er->where;
	GlobalToLocal( &pp2 );
	widget = QApplication::widgetAt( pp2.h, pp2.v, true );
      }
      if ( widget ) {
	SetPortWindowPort( wp );

	QPoint p( er->where.h, er->where.v );
	QPoint pglob(widget->mapFromGlobal( p ));
	QMouseEvent qme( QEvent::MouseButtonPress, pglob, p, QMouseEvent::LeftButton, 0);
	QApplication::sendEvent( widget, &qme );
      }
      mouse_button_state = 1;
    }
    else do_mouse_down( er ); //do resize/move stuff
  } else if( er->what == mouseUp ) {
    short part = FindWindow( er->where, &wp );
    if( part == inContent ) {
      if( mac_mouse_grabber ) {
	widget = mac_mouse_grabber;
      } else {
	Point pp2 = er->where;
	GlobalToLocal( &pp2 );
	widget = QApplication::widgetAt( pp2.h, pp2.v, true );
      }
      if ( widget ) {
	SetPortWindowPort( wp );

	QPoint p( er->where.h, er->where.v );
	QPoint pglob(widget->mapFromGlobal( p ));
	QMouseEvent qme( QEvent::MouseButtonRelease, pglob, p, QMouseEvent::LeftButton, 0);
	QApplication::sendEvent( widget, &qme );

      }
    }
    mouse_button_state = 0;
  } else if(er->what == keyDown) {
    short part = FindWindow( er->where, &wp );
    if( part == inContent ) {
      if( mac_keyboard_grabber ) {
	widget = mac_keyboard_grabber;
      } else {
	Point pp2 = er->where;
	GlobalToLocal( &pp2 );
	widget = QApplication::widgetAt( pp2.h, pp2.v, true );
      }
    }
    if(!widget) {
      qWarning("Can't find %d!",myactive);
      return 0;
    }

    short mychar=er->message & charCodeMask;
    QKeyEvent ke(QEvent::KeyPress,get_key(er->message),mychar,0,QString(QChar(mychar)));
    //QApplication::sendEvent(twidget,&ke);
    QApplication::sendEvent(widget,&ke);
    qDebug("Key down.. %c", mychar);
  } else if(er->what == keyUp) {
    short part = FindWindow( er->where, &wp );
    if( part == inContent ) {
      if( mac_keyboard_grabber ) {
	widget = mac_keyboard_grabber;
      } else {
	Point pp2 = er->where;
	GlobalToLocal( &pp2 );
	widget = QApplication::widgetAt( pp2.h, pp2.v, true );
      }
    }
    if(!widget) {
      qWarning("Can't find %d!",myactive);
      return 0;
    }

    short mychar=er->message & charCodeMask;
    QKeyEvent ke(QEvent::KeyRelease,get_key(er->message),mychar,0,QString(QChar(mychar)));
    //QApplication::sendEvent(twidget,&ke);
    QApplication::sendEvent(widget,&ke);
    qDebug("Key up.. %c", mychar);
  } else if(er->what == osEvt) {
    if(((er->message >> 24) & 0xFF) == mouseMovedMessage) {
      short part = FindWindow( er->where, &wp );
      if( part == inContent ) {
	if( mac_mouse_grabber ) {
	  widget = mac_mouse_grabber;
	} else {
	  Point pp2 = er->where;
	  //GlobalToLocal( &pp2 );
	  widget = QApplication::widgetAt( pp2.h, pp2.v, true );
	}
	if ( widget && (mouse_button_state || widget->hasMouseTracking() || hasGlobalMouseTracking())) {
	  SetPortWindowPort( wp );

	  QPoint p( er->where.h, er->where.v );
	  QPoint pglob(widget->mapFromGlobal( p ));
	  QMouseEvent qme( QEvent::MouseMove, pglob, p, QMouseEvent::LeftButton, 0);
	  QApplication::sendEvent( widget, &qme );
	  qDebug("Mouse has Moved..");
	}	
      }
    }
    else
      printf("Damn!\n");
  } else {
    qWarning("  Type %d",er->what);
  }
  return 0;
}

void QApplication::wakeUpGuiThread()
{
    qDebug( "QApplication::wakeUpGuiThread" );
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

void qt_enter_modal( QWidget * )
{
    qDebug( "qt_enter_modal" );
}


void qt_leave_modal( QWidget * )
{
    qDebug( "qt_leave_modal" );
}


/*!
  Processes pending events for \a maxtime milliseconds or until there
  are no more events to process, whichever is shorter.

  You can call this function occasionally when you program is busy doing a
  long operation (e.g. copying a file).

  \sa processOneEvent(), exec(), QTimer
*/
void QApplication::processEvents( int )
{
    qDebug( "QApplication::processEvents" );
    processNextEvent(FALSE);
}

bool QApplication::macEventFilter( void ** )
{
    qDebug( "QApplication::macEventFilter" );
    return 0;
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

void QApplication::openPopup( QWidget * )
{
    qDebug( "QApplication::openPopup" );
}

void QApplication::closePopup( QWidget * )
{
    qDebug( "QApplication::closePopup" );
}

int qStartTimer( int, QObject * )
{
    qDebug( "qStartTimer" );
    return 0;
}

bool qKillTimer( int )
{
    qDebug( "qKillTimer" );
    return FALSE;
}

bool qKillTimer( QObject * )
{
    qDebug( "qKillTimer QObject" );
    return FALSE;
}

/*****************************************************************************
  Event translation; translates X11 events to Qt events
 *****************************************************************************/


/*!
  Sets the text cursor's flash time to \a msecs milliseconds.  The
  flash time is the time required to display, invert and restore the
  caret display: A full flash cycle.  Usually, the text cursor is
  displayed for \a msecs/2 milliseconds, then hidden for \a msecs/2
  milliseconds, but this may vary.

  Note that on Microsoft Windows, calling this function sets the
  cursor flash time for all windows.

  \sa cursorFlashTime()
 */
void  QApplication::setCursorFlashTime( int msecs )
{
    qDebug( "QApplication::setCursorFlashTime" );
    cursor_flash_time = msecs;
}


/*!
  Returns the text cursor's flash time in milliseconds. The flash time
  is the time required to display, invert and restore the caret
  display.

  The default value on X11 is 1000 milliseconds. On Windows, the
  control panel value is used.

  Widgets should not cache this value since it may vary any time the
  user changes the global desktop settings.

  \sa setCursorFlashTime()
 */
int QApplication::cursorFlashTime()
{
    qDebug( "QApplication::cursorFlashTime" );
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
    qDebug( "QApplication::setDoubleClickInterval" );
    mouse_double_click_time = ms;
}


//FIXME: What is the default value on the Mac?
/*!
  Returns the maximum duration for a double click.

  The default value on X11 is 400 milliseconds. On Windows, the control
  panel value is used.

  \sa setDoubleClickInterval()
*/

int QApplication::doubleClickInterval()
{
    qDebug( "QApplication::doubleClickInterval" );
    return mouse_double_click_time;
}


/*!
  Sets the number of lines to scroll when the mouse wheel is
  rotated.

  If this number exceeds the number of visible lines in a certain
  widget, the widget should interpret the scroll operation as a single
  page up / page down operation instead.

  \sa wheelScrollLines()
 */
void QApplication::setWheelScrollLines( int n )
{
    qDebug( "QApplication::setWheelScrollLines" );
    wheel_scroll_lines = n;
}

/*!
  Returns the number of lines to scroll when the mouse wheel is rotated.

  \sa setWheelScrollLines()
 */
int QApplication::wheelScrollLines()
{
    qDebug( "QApplication::wheelScrollLines" );
    return wheel_scroll_lines;
}

/*!
  Enables the UI effect \a effect if \a enable is TRUE, otherwise
  the effect will not be used.

  \sa isEffectEnabled(), Qt::UIEffect, setDesktopSettingsAware()
*/
void QApplication::setEffectEnabled( Qt::UIEffect effect, bool enable )
{
    qDebug( "QApplication::setEffectEnabled" );
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
  Returns TRUE if \a effect is enabled, otherwise FALSE.

  By default, Qt will try to use the desktop settings, and
  setDesktopSettingsAware() must be called to prevent this.

  sa\ setEffectEnabled(), Qt::UIEffect
*/
bool QApplication::isEffectEnabled( Qt::UIEffect )
{
    qDebug( "QApplication::isEffectEnabled" );
    return FALSE;
}

/*****************************************************************************
  Session management support
 *****************************************************************************/

#ifdef QT_NO_SM_SUPPORT

class QSessionManagerData
{
public:
    QStringList restartCommand;
    QStringList discardCommand;
    QString sessionId;
    QSessionManager::RestartHint restartHint;
};

QSessionManager::QSessionManager( QApplication * app, QString &session )
    : QObject( app, "qt_sessionmanager" )
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

void* QSessionManager::handle() const
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

#else // QT_NO_SM_SUPPORT


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


// workaround for broken libsm, see below
struct QT_smcConn {
    unsigned int save_yourself_in_progress : 1;
    unsigned int shutdown_in_progress : 1;
};

void QSmSocketReceiver::socketActivated(int)
{
}

#include "qapplication_mac.moc"

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
    return "";
}

bool QSessionManager::allowsInteraction()
{
    return FALSE;
}

bool QSessionManager::allowsErrorInteraction()
{
    return FALSE;
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

void QSessionManager::setRestartCommand( const QStringList& )
{
}

QStringList QSessionManager::restartCommand() const
{
    return QStringList();
}

void QSessionManager::setDiscardCommand( const QStringList& )
{
}

QStringList QSessionManager::discardCommand() const
{
    return QStringList();
}

void QSessionManager::setProperty( const QString&, const QString& )
{
}

void QSessionManager::setProperty( const QString&, const QStringList& )
{
}

bool QSessionManager::isPhase2() const
{
    return true;
}

void QSessionManager::requestPhase2()
{
}


#endif // QT_NO_SM_SUPPORT
