/****************************************************************************
** $Id: //depot/qt/main/extensions/xembed/qxembed.cpp#8 $
**
** Implementation of QXEmbed class
**
** Created :
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

#include <qapplication.h>
#include "qxembed.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>


// defined in qapplication_x11.cpp
extern Atom	qt_embedded_window;
extern Atom	qt_embedded_window_take_focus;
extern Atom	qt_embedded_window_focus_in;
extern Atom	qt_embedded_window_focus_out;
extern Atom	qt_embedded_window_tab_focus;
extern Atom	qt_embedded_window_support_tab_focus;
extern Atom	qt_wheel_event;
extern Atom	qt_unicode_key_press;
extern Atom	qt_unicode_key_release;
extern Atom	qt_wm_delete_window;

class QXEmbedData
{
public:
    QXEmbedData(){};
    ~QXEmbedData(){};
};


/*!
  \class QXEmbed qxembed.h

  \brief The QXEmbed widget is a container that can embed an external X-Window.
  
  \extension XEmbed

  A QXEmbed widget serves as a container that can manage one single
  embedded X-window. These so-called client windows can be arbitrary
  QWidgets.

  There are two different ways of using QXembed, from the container or
  from the client side.  When using it from the container side, you
  already know the window identifier of the window that should be
  embedded. Simply call embed() with this identifier as parameter.
  
  Embedding from the client side requires that the client knows the
  window identifier of the respective container widget. Use either
  embedClientIntoWindow() or the high-level wrapper
  processClientCmdline(). 

  If a window has been embedded succesfully, embeddedWinId() returns
  its id.
  
  Reimplement the change handler windowChanged() to catch embedding or
  the destruction of embedded windows. In the latter case, the
  container also emits a signal embeddedWindowDestroyed() for
  convenience.
  
*/

/*!
  Constructs a xembed widget.

  The \e parent, \e name and \e f arguments are passed to the QFrame
  constructor.
 */
QXEmbed::QXEmbed(QWidget *parent, const char *name, WFlags f)
  : QWidget(parent, name, f)
{
    window = 0;
    setFocusPolicy(StrongFocus);

    //trick to create extraData();
    QCursor old = cursor();
    setCursor(Qt::blankCursor);
    setCursor(old);

    // we are interested int SubstructureNotify
    XSelectInput(qt_xdisplay(), winId(),
  		 KeyPressMask | KeyReleaseMask |
  		 ButtonPressMask | ButtonReleaseMask |
  		 KeymapStateMask |
   		 ButtonMotionMask |
   		 PointerMotionMask | // may need this, too
  		 EnterWindowMask | LeaveWindowMask |
  		 FocusChangeMask |
  		 ExposureMask |
		 StructureNotifyMask |
		 SubstructureRedirectMask |
		 SubstructureNotifyMask
  		 );
}

/*!
  Destructor. Cleans up the focus if necessary.
 */
QXEmbed::~QXEmbed()
{
    static Atom wm_protocols = 0;
    if (!wm_protocols )
	wm_protocols = XInternAtom( qt_xdisplay(), "WM_PROTOCOLS", False );
    
    if ( window != 0 ) {
 	XUnmapWindow(qt_xdisplay(), window );
 	XReparentWindow(qt_xdisplay(), window, qt_xrootwin(), 0, 0);
	 
 	XEvent ev;
 	memset(&ev, 0, sizeof(ev));
 	ev.xclient.type = ClientMessage;
 	ev.xclient.window = window;
 	ev.xclient.message_type = wm_protocols;
 	ev.xclient.format = 32;
 	ev.xclient.data.s[0] = qt_wm_delete_window;
 	XSendEvent(qt_xdisplay(), window, FALSE, NoEventMask, &ev);
     }
    window = 0;
}


/*!
  Reimplimented to resize the embedded window respectively.
 */
void QXEmbed::resizeEvent(QResizeEvent*)
{
    if (window != 0)
	XResizeWindow(qt_xdisplay(), window, width(), height());
}

/*!
  Reimplimented to ensure the embedded window will be visible as well.
 */
void QXEmbed::showEvent(QShowEvent*)
{
    if (window != 0)
	XMapRaised(qt_xdisplay(), window);

}

/*!
  Reimplimented to route the keyevents to the embedded window.
 */
void QXEmbed::keyPressEvent( QKeyEvent *e )
{
    if (!window)
	return;

    XEvent ev;
    QString text = e->text();
    int i = 1;
    int m = QMAX(1, text.length());
    do{
	memset(&ev, 0, sizeof(ev));
	ev.xclient.type = ClientMessage;
	ev.xclient.window = window;
	ev.xclient.message_type = qt_unicode_key_press;
	ev.xclient.format = 16;
	ev.xclient.data.s[0] = e->key();
	ev.xclient.data.s[1] = e->ascii();
	ev.xclient.data.s[2] = e->state();
	ev.xclient.data.s[3] = e->isAutoRepeat();
	ev.xclient.data.s[4] = !text.isEmpty()?1:e->count();
	ev.xclient.data.s[5] = !text.isEmpty()?text[i-1].row():QChar::null.row();
	ev.xclient.data.s[6] = !text.isEmpty()?text[i-1].cell():QChar::null.cell();
	ev.xclient.data.s[7] = i++;
	ev.xclient.data.s[8] = m;
	XSendEvent(qt_xdisplay(), window, FALSE, NoEventMask, &ev);
    } while ( i <= m);
}

/*!
  Reimplimented to route the keyevents to the embedded window.
 */
void QXEmbed::keyReleaseEvent( QKeyEvent *e )
{
    if (!window)
	return;

    XEvent ev;
    QString text = e->text();
    int i = 1;
    int m = QMAX(1, text.length());
    do{
	memset(&ev, 0, sizeof(ev));
	ev.xclient.type = ClientMessage;
	ev.xclient.window = window;
	ev.xclient.message_type = qt_unicode_key_release;
	ev.xclient.format = 16;
	ev.xclient.data.s[0] = e->key();
	ev.xclient.data.s[1] = e->ascii();
	ev.xclient.data.s[2] = e->state();
	ev.xclient.data.s[3] = e->isAutoRepeat();
	ev.xclient.data.s[4] = !text.isEmpty()?1:e->count();
	ev.xclient.data.s[5] = !text.isEmpty()?text[i-1].row():QChar::null.row();
	ev.xclient.data.s[6] = !text.isEmpty()?text[i-1].cell():QChar::null.cell();
	ev.xclient.data.s[7] = i++;
	ev.xclient.data.s[8] = m;
	XSendEvent(qt_xdisplay(), window, FALSE, NoEventMask, &ev);
    } while ( i <= m);
}

/*!
  Reimplimented to route the focus events to the embedded window.
 */
void QXEmbed::focusInEvent( QFocusEvent * ){
    if (!window)
	return;
    sendFocusIn();
}

/*!
  Reimplimented to route the focus events to the embedded window.
 */
void QXEmbed::focusOutEvent( QFocusEvent * ){
    if (!window)
	return;
    sendFocusOut();
}


/*!
  Reimplimented to route the wheel events to the embedded window.
 */
void QXEmbed::wheelEvent( QWheelEvent * e)
{
    if (!window)
	return;

    XEvent ev;
    memset(&ev, 0, sizeof(ev));
    ev.xclient.type = ClientMessage;
    ev.xclient.window = window;
    ev.xclient.message_type = qt_wheel_event;
    ev.xclient.format = 32;
    ev.xclient.data.l[0] = e->globalX();
    ev.xclient.data.l[1] = e->globalY();
    ev.xclient.data.l[2] = e->delta();
    ev.xclient.data.l[3] = e->state();
    XSendEvent(qt_xdisplay(), window, FALSE, NoEventMask, &ev);
}


/*!

  Embeds the window with the identifier \a w into this xembed widget.

  This function is useful if the server knows about the client window
  that should be embedded.  Often it is vice versa: the client knows
  about his target container. In that case, it is not necessary to
  call embed(). Instead, the client will call the static function
  embedClientIntoWindow().

  \sa embeddedWinId()
 */
void QXEmbed::embed(WId w)
{
    if (!w)
	return;

    bool has_window =  w == window;

    window = w;
    long a = 1;
    window_supports_tab_focus = FALSE;
    XChangeProperty(qt_xdisplay(), w,
		    qt_embedded_window, XA_CARDINAL, 32, PropModeReplace,
		    (const unsigned char*)&a, 1);
    if ( !has_window )
	XReparentWindow(qt_xdisplay(), w, winId(), 0, 0);
    QApplication::syncX();
    XResizeWindow(qt_xdisplay(), w, width(), height());
    XMapRaised(qt_xdisplay(), window);
    XAddToSaveSet( qt_xdisplay(), w );
    extraData()->xDndProxy = w;

    if ( parent() ) {
	QEvent * layoutHint = new QEvent( QEvent::LayoutHint );
	QApplication::postEvent( parent(), layoutHint );
    }
    windowChanged( window );

    if (this == qApp->focusWidget() )
	sendFocusIn();
    else
	sendFocusOut();
}


/*!
  Returns the window identifier of the embedded window, or 0 if no
  window is embedded yet.
 */
WId QXEmbed::embeddedWinId() const
{
    return window;
}

void QXEmbed::sendFocusIn()
{
    XClientMessageEvent client_message;
    client_message.type = ClientMessage;
    client_message.window = window;
    client_message.format = 32;
    client_message.message_type = qt_embedded_window_focus_in;
    XSendEvent( qt_xdisplay(), client_message.window, FALSE, NoEventMask,
		(XEvent*)&client_message );
}

void QXEmbed::sendFocusOut()
{
    XClientMessageEvent client_message;
    client_message.type = ClientMessage;
    client_message.window = window;
    client_message.format = 32;
    client_message.message_type = qt_embedded_window_focus_out;
    XSendEvent( qt_xdisplay(), client_message.window, FALSE, NoEventMask,
		(XEvent*)&client_message );
}


/*!\reimp
 */
bool QXEmbed::focusNextPrevChild( bool next )
{
    if ( window && window_supports_tab_focus )
	return FALSE;
    else
	return QWidget::focusNextPrevChild( next );
}


/*!
  Reimplemented to observe child window changes
 */
bool QXEmbed::x11Event( XEvent* e)
{
    switch ( e->type ) {
    case DestroyNotify:
	if ( e->xdestroywindow.window == window ) {
	    window = 0;
	    windowChanged( window );
	    emit embeddedWindowDestroyed();
	}
	break;
    case ReparentNotify:
	if ( window && e->xreparent.window == window &&
	     e->xreparent.parent != winId() ) {
	    // we lost the window
	    window = 0;
	    windowChanged( window );
	} else if ( e->xreparent.parent == winId() ){
	    // we got a window
	    window = e->xreparent.window;
	    embed( window );
	}
	break;
    case MapRequest:
	if ( window && e->xmaprequest.window == window )
	    XMapRaised(qt_xdisplay(), window );
	break;
    case ClientMessage:
	if ( e->xclient.format == 32 && e->xclient.message_type ) {
	    if  ( e->xclient.message_type == qt_embedded_window_support_tab_focus ) {
		window_supports_tab_focus = TRUE;
	    }
	    else if  ( e->xclient.message_type == qt_embedded_window_tab_focus ) {
		window_supports_tab_focus = TRUE;
		QWidget::focusNextPrevChild( e->xclient.data.l[0] );
	    }
	}
    default:
	break;
    }
    return FALSE;
}


/*!
  A change handler that indicates that the embedded window has been
  changed.  The window handle can also be retrieved with
  embeddedWinId().
 */
void QXEmbed::windowChanged( WId )
{
}


/*!
  A utility function for clients that embed theirselves. The widget \a
  client will be embedded in the window that is passed as
  \c -embed command line argument.

  The function returns TRUE on sucess or FALSE if no such command line
  parameter is specified.

  \sa embedClientIntoWindow()
 */
bool QXEmbed::processClientCmdline( QWidget* client, int& argc, char ** argv )
{
    int myargc = argc;
    WId window = 0;
    int i, j;

    j = 1;
    for ( i=1; i<myargc; i++ ) {
	if ( argv[i] && *argv[i] != '-' ) {
	    argv[j++] = argv[i];
	    continue;
	}
	QCString arg = argv[i];
	if ( strcmp(arg,"-embed") == 0 && i < myargc-1 ) {
	    QCString s = argv[++i];
	    window = s.toInt();
	} else
	    argv[j++] = argv[i];
    }
    argc = j;

    if ( window != 0 ) {
	embedClientIntoWindow( client, window );
	return TRUE;
    }

    return FALSE;
}


/*!
  A utility function for clients that embed theirselves. The widget \a
  client will be embedded in the window \a window. The application has
  to ensure that \a window is the handle of the window identifier of
  an QXEmbed widget.

  \sa processClientCmdline()
 */
void QXEmbed::embedClientIntoWindow(QWidget* client, WId window)
{
    XReparentWindow(qt_xdisplay(), client->winId(), window, 0, 0);
    client->show();
}



/*!
  Specifies that this widget can use additional space, and that it can
  survive on less than sizeHint().
*/

QSizePolicy QXEmbed::sizePolicy() const
{
    return QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
}


/*!
  Returns a size sufficient for the embedded window
*/
QSize QXEmbed::sizeHint() const
{
    return minimumSizeHint();
}

/*!
  \fn void QXEmbed::embeddedWindowDestroyed()

  This signal is emitted when the embedded window has been destroyed.
  
  \sa embeddedWinId()
*/

/*!
  Returns a size sufficient for one character, and scroll bars.
*/

QSize QXEmbed::minimumSizeHint() const
{
    int minw = 0;
    int minh = 0;
    if ( window ) {
	XSizeHints size;
	long msize;
	if (XGetWMNormalHints(qt_xdisplay(), window, &size, &msize)
	    && ( size.flags & PMinSize) ) {
	    minw = size.min_width;
	    minh = size.min_height;
	}
    }

    return QSize( minw, minh );
}

