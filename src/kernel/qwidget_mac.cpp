/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qwidget_mac.cpp $
**
** Implementation of QWidget and QWindow classes for mac
**
** Created : 001018
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
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

#include "qt_mac.h"

#include "qapplication.h"
#include "qapplication_p.h"
#include "qpaintdevicemetrics.h"
#include "qpainter.h"
#include "qbitmap.h"
#include "qwidgetlist.h"
#include "qwidgetintdict.h"
#include "qobjectlist.h"
#include "qobjectdict.h"
#include "qaccel.h"
#include "qdragobject.h"
#include "qfocusdata.h"
#include "qabstractlayout.h"
#include "qtextcodec.h"

// NOT REVISED

/*****************************************************************************
  QWidget member functions
 *****************************************************************************/

/* THIS IS FOR PASCAL STYLE STRINGS, WE NEED TO FIGURE OUT IF THESE ARE FREED WHEN HANDED OVER
   TO THE OS FIXME FIXME FIXME FIXME */
//this function really sucks, re-write me when you'r edone figuring out
const unsigned char * p_str(const char * c)
{
    unsigned char * ret=new unsigned char[qstrlen(c)+2];
    ret[0]=qstrlen(c);
    qstrcpy(((char *)ret)+1,c);
    return ret;
}

/*!
  Creates a new widget window if \a window is null, otherwise sets the
  widget's window to \a window.

  Initializes the window (sets the geometry etc.) if \a initializeWindow
  is TRUE.  If \a initializeWindow is FALSE, no initialization is
  performed.  This parameter makes only sense if \a window is a valid
  window.

  Destroys the old window if \a destroyOldWindow is TRUE.  If \a
  destroyOldWindow is FALSE, you are responsible for destroying
  the window yourself (using platform native code).

  The QWidget constructor calls create(0,TRUE,TRUE) to create a window for
  this widget.
*/

WId parentw, destroyw = 0;
WId myactive = -1;

QWidget * get_top( QWidget * widg )
{
    // Return top-level window
    if ( !widg )
	return 0;
    QWidget *ret = widg;
    while ( ret->parentWidget() != 0 ) {
	ret = ret->parentWidget();
    }
    return ret;
}

//FIXME why not call this something mapLocalToGlobal?
void make_top( QWidget *widg, int &x, int &y )
{
    // Convert from local window to global coords
    // First get top left of actual window
    int xOffset = 0;
    int yOffset = 0;

    if ( !widg || !widg->parentWidget() )
	return;
    QWidget *ret = widg;
    while ( ret->parentWidget() ) {
	xOffset += ret->x();
	yOffset += ret->y();
	ret = ret->parentWidget();
    }
    x = x + xOffset;
    y = y + yOffset;
}


//FIXME How can I create translucent windows? (Need them for pull down menus)
//FIXME Is this even possible with the Carbon API? (You can't do it on OS9)
//FIXME Perhaps we need to access the lower level Quartz API?
//FIXME Documentation on Quartz, where is it?
void QWidget::create( WId window, bool initializeWindow, bool /* destroyOldWindow */ )
{
    qDebug( "QWidget::create" );
    bg_pix = 0;
    back_type = 1;
    WId root_win = 0;
    setWState( WState_Created );                        // set created flag

    if ( !parentWidget() )
	setWFlags( WType_TopLevel );            // top-level widget

    static int sw = -1, sh = -1;                // screen size
    bool topLevel = testWFlags( WType_TopLevel );
    bool popup = testWFlags( WType_Popup );
    bool modal = testWFlags( WType_Modal );
    bool desktop = testWFlags( WType_Desktop );
    WId    id;

    if ( !window )                              // always initialize
	initializeWindow=TRUE;

    //FIXME need to query the display for characteristics
    if ( sw < 0 ) {
	sw = 1024;    // Make it up
	sh = 768;
    }

    bg_col = pal.normal().background();

    if ( modal || popup || desktop ) {          // these are top-level, too
	topLevel = TRUE;
	setWFlags( WType_TopLevel );
    }

    Rect boundsRect;

    if ( desktop ) {                            // desktop widget
	modal = popup = FALSE;                  // force these flags off
	crect.setRect( 0, 0, sw, sh );
    } else if ( topLevel ) {                    // calc pos/size from screen
	crect.setRect( sw/4, 3*sh/10, sw/2, 4*sh/10 );
    } else {                                    // child widget
	crect.setRect( 0, 0, 100, 30 );
    }
    fpos = crect.topLeft();                     // default frame rect

    parentw = topLevel ? root_win : parentWidget()->winId();

    SetRect( &boundsRect, crect.left(), crect.top(), 
	     crect.right(), crect.bottom());

    char title[2];
    title[0]=0;
    title[1]='\0';
    unsigned char visible=0;
    short procid;
    if ( popup ) {
	procid = plainDBox;
    } else {
	procid = zoomDocProc;
    }

    WindowPtr behind = (WindowPtr)-1;
    unsigned char goaway=true;

    if( !parentWidget() || (popup || modal) ) {
	mytop = this;
	SetRect( &boundsRect, 50, 50, 600, 200 );
	id = (WId)NewCWindow( nil, &boundsRect, (const unsigned char*)title, 
			      visible, procid, behind, goaway, 0);
	hd = (void *)id;
    } else {
	mytop = get_top( this );
	id = (WId)mytop->hd;
	hd = mytop->hd;
    }

    bg_col = pal.normal().background();
    if ( !parentWidget() ) {
	qDebug( "Toplevel %d\n", id );
	setWinId( id );
    } else {
	qDebug( "Non-toplevel %d\n", id );
	winid = id;
    }

    const char *c = name();
    if( !parentWidget() ) {
	if ( c ) {
	    setCaption( QString( c ));
	}
    }

    setWState( WState_MouseTracking );
    setMouseTracking( FALSE );                  // also sets event mask
    clearWState(WState_Visible);
}


/*!
  Frees up window system resources.
  Destroys the widget window if \a destroyWindow is TRUE.

  destroy() calls itself recursively for all the child widgets,
  passing \a destroySubWindows for the \a destroyWindow parameter.
  To have more control over destruction of subwidgets,
  destroy subwidgets selectively first.

  This function is usually called from the QWidget destructor.
*/

void QWidget::destroy( bool, bool )
{
    qDebug( "QWidget::destroy" );
}

/*!
  Reparents the widget.  The widget gets a new \a parent, new widget
  flags (\a f, but as usual, use 0) at a new position in its new
  parent (\a p).

  If \a showIt is TRUE, show() is called once the widget has been
  reparented.

  If the new parent widget is in a different top-level widget, the
  reparented widget and its children are appended to the end of the
  \link setFocusPolicy() TAB chain \endlink of the new parent widget,
  in the same internal order as before.  If one of the moved widgets
  had keyboard focus, reparent() calls clearFocus() for that widget.

  If the new parent widget is in the same top-level widget as the old
  parent, reparent doesn't change the TAB order or keyboard focus.

  \warning Reparenting widgets should be a real exception. In normal
  applications, you will almost never need it. Dynamic masks can be
  achieved much easier and cleaner with classes like QWidgetStack or
  on a higher abstraction level, QWizard.

  \sa getWFlags()
*/

void QWidget::reparent( QWidget *, WFlags, const QPoint &,
			bool )
{
    qDebug( "QWidget::reparent" );
}


/*!
  Translates the widget coordinate \e pos to global screen coordinates.
  For example, \code mapToGlobal(QPoint(0,0))\endcode would give the
  global coordinates of the top-left pixel of the widget.
  \sa mapFromGlobal() mapTo() mapToParent()
*/

QPoint QWidget::mapToGlobal( const QPoint &pos ) const
{
  int x2=pos.x();
  int y2=pos.y();

  ((QWidget *)this)->lockPort();
  PixMapHandle pmh=GetPortPixMap(GetWindowPort((WindowPtr)handle()));
  x2=x2-(**pmh).bounds.left;
  y2=y2-(**pmh).bounds.top;
  ((QWidget *)this)->unlockPort();

  qDebug("QWidget::mapToGlobal(%d, %d) = (%d, %d)\n", pos.x(), pos.y(), x2, y2);
  QPoint p2(x2,y2);
  return p2;
}


/*!
  Translates the global screen coordinate \e pos to widget coordinates.
  \sa mapToGlobal() mapFrom() mapFromParent()
*/

QPoint QWidget::mapFromGlobal( const QPoint &pos ) const
{
  int x2=pos.x();
  int y2=pos.y();

  ((QWidget *)this)->lockPort();
  PixMapHandle pmh=GetPortPixMap(GetWindowPort((WindowPtr)handle()));
  x2=x2+(**pmh).bounds.left;
  y2=y2+(**pmh).bounds.top;
  for(const QWidget *widg=this; !widg->isTopLevel(); widg = widg->parentWidget()) {
    x2-=widg->x();
    y2-=widg->y();
  }
  ((QWidget *)this)->unlockPort();

  qDebug("QWidget::mapFromGlobal(%d, %d) = (%d, %d)\n", pos.x(), pos.y(), x2, y2);
  QPoint p2(x2,y2);
  return p2;
}

/*!
  When a widget gets focus, it should call setMicroFocusHint for some
  appropriate position and size - \a x, \a y and \a w by \a h.  This
  has no \e visual effect, it just provides hints to any
  system-specific input handling tools.

  The \a text argument should be TRUE if this is a position for text
  input.

  In the Windows version of Qt, this method sets the system caret, which is
  used for user Accessibility focus handling.  If \a text is TRUE, it also
  sets the IME composition window in Far East Asian language input systems.

  In the X11 version of Qt, if \a text is TRUE, this method sets the
  XIM "spot" point for complex language input handling.

  \sa microFocusHint()
*/
void QWidget::setMicroFocusHint(int, int, int, int, bool )
{
    qDebug( "QWidget::setMicroFocusHint" );
}

void QWidget::setFontSys()
{
    qDebug( "QWidget::setFontSys" );
}


void QWidget::setBackgroundColorDirect( const QColor &color )
{
    qDebug( "QWidget::setBackgroundColorDirect" );
    back_type = 1;
    bg_col = color;
    if ( bg_pix )
      delete bg_pix;
    erase( 0, 0, width(), height());
}

void QWidget::setBackgroundPixmapDirect( const QPixmap & )
{
    qDebug( "QWidget::setBackgroundPixmapDirect" );
}

void show_children( QWidget * w, int show )
{
    const QObjectList *child = w->children();
    if( child ) {
	QObjectListIt it( *child );
	QObject *sibling = it.toLast();
	do {
	    if( sibling->inherits( "QWidget" ) )
		if( !sibling->inherits( "QMenuBar" ) )
		    if (show)
			((QWidget *)sibling)->show();
		    else
			((QWidget *)sibling)->hide();
	    sibling = --it;
	} while ( sibling != 0 );
    }
}



void redraw_children(QWidget * w)
{
    //FIXME this erase blanks the window when it is resized smaller
    w->erase( 0, 0, w->width(), w->height());
    const QObjectList *child = w->children();
    if ( child ) {
	QObjectListIt it( *child );
	QObject *sibling = it.toLast();
	do {
	    if ( sibling->inherits( "QWidget" ))
		redraw_children( (QWidget *)sibling );
	    sibling = --it;
	} while ( sibling !=0 );
    }
}

/*!
  Sets the window-system background of the widget to nothing.

  Note that `nothing' is actually a pixmap that isNull(), thus you
  can check for an empty background by checking backgroundPixmap().

  \sa setBackgroundPixmap(), setBackgroundColor()

  This class should \e NOT be made virtual - it is an alternate usage
  of setBackgroundPixmap().
*/
void QWidget::setBackgroundEmpty()
{
    qDebug( "QWidget::setBackgroundEmpty" );
}


/*!
  Sets the widget cursor shape to \e cursor.

  The mouse cursor will assume this shape when it's over this widget.
  See a list of predefined cursor objects with a range of useful
  shapes in the QCursor documentation.

  An editor widget would for example use an I-beam cursor:
  \code
    setCursor( ibeamCursor );
  \endcode

  \sa cursor(), unsetCursor(), QApplication::setOverrideCursor()
*/

void QWidget::setCursor( const QCursor & )
{
    qDebug( "QApplication::setCursor" );
}


/*!
  Unset the cursor for this widget. The widget will use the cursor of
  its parent from now on.

  This functions does nothing for top-level windows.

  \sa cursor(), setCursor(), QApplication::setOverrideCursor()
 */

void QWidget::unsetCursor()
{
    qDebug( "QApplication::unsetCursor" );
}

/*!
  Sets the window caption (title) to \a caption.
  \sa caption(), setIcon(), setIconText()
*/

void QWidget::setCaption( const QString &cap )
{
    qDebug( "QWidget::setCaption" );

    SetWTitle((WindowPtr)winid, p_str(cap.latin1()));
}

/*!
  Sets the window icon to \a pixmap.
  \sa icon(), setIconText(), setCaption(),
      \link appicon.html Setting the Application Icon\endlink
*/

void QWidget::setIcon( const QPixmap & )
{
    qDebug( "QWidget::setIcon" );
}


/*!
  Sets the text of the window's icon to \e iconText.
  \sa iconText(), setIcon(), setCaption()
*/

void QWidget::setIconText( const QString & )
{
    qDebug( "QWidget::setIconText" );
}


/*!
  Grabs the mouse input.

  This widget will be the only one to receive mouse events until
  releaseMouse() is called.

  \warning Grabbing the mouse might lock the terminal.

  It is almost never necessary to grab the mouse when using Qt since
  Qt grabs and releases it sensibly.  In particular, Qt grabs the
  mouse when a button is pressed and keeps it until the last button is
  released.

  \sa releaseMouse(), grabKeyboard(), releaseKeyboard()
*/

QWidget *mac_mouse_grabber = 0;

void QWidget::grabMouse()
{
    qDebug( "QWidget::grabMouse" );
    mac_mouse_grabber=this;
}

/*!
  Grabs the mouse input and changes the cursor shape.

  The cursor will assume shape \e cursor (for as long as the mouse focus is
  grabbed) and this widget will be the only one to receive mouse events
  until releaseMouse() is called().

  \warning Grabbing the mouse might lock the terminal.

  \sa releaseMouse(), grabKeyboard(), releaseKeyboard(), setCursor()
*/

void QWidget::grabMouse( const QCursor & )
{
    qDebug( "QWidget::grabMouse" );
    mac_mouse_grabber=0;
}

/*!
  Releases the mouse grab.

  \sa grabMouse(), grabKeyboard(), releaseKeyboard()
*/

void QWidget::releaseMouse()
{
    qDebug( "QWidget::releaseMouse" );
}

/*!
  Grabs all keyboard input.

  This widget will receive all keyboard events, independent of the active
  window.

  \warning Grabbing the keyboard might lock the terminal.

  \sa releaseKeyboard(), grabMouse(), releaseMouse()
*/

QWidget *mac_keyboard_grabber = 0;

void QWidget::grabKeyboard()
{
    qDebug( "QWidget::grabKeyboard" );
    mac_keyboard_grabber = this;
}

/*!
  Releases the keyboard grab.

  \sa grabKeyboard(), grabMouse(), releaseMouse()
*/

void QWidget::releaseKeyboard()
{
    qDebug( "QWidget::releaseKeyboard" );
    mac_keyboard_grabber = 0;
}


/*!
  Returns a pointer to the widget that is currently grabbing the
  mouse input.

  If no widget in this application is currently grabbing the mouse, 0 is
  returned.

  \sa grabMouse(), keyboardGrabber()
*/

QWidget *QWidget::mouseGrabber()
{
    qDebug( "QWidget::mouseGrabber" );
    return 0;
}

/*!
  Returns a pointer to the widget that is currently grabbing the
  keyboard input.

  If no widget in this application is currently grabbing the keyboard, 0
  is returned.

  \sa grabMouse(), mouseGrabber()
*/

QWidget *QWidget::keyboardGrabber()
{
    qDebug( "QWidget::keyboardGrabber" );
    return 0;
}


/*!
  Sets the top-level widget containing this widget to be the active
  window.

  An active window is a visible top-level window that has the keyboard input
  focus.

  This function performs the same operation as clicking the mouse on
  the title bar of a top-level window, at least on Windows. On X11,
  the result depends on the Window Manager. If you want to ensure that
  the window is stacked on top as well, call raise() in addition. Note
  that the window has be to visible, otherwise setActiveWindow() has
  no effect.

  \sa isActiveWindow(), topLevelWidget(), show()
*/

void QWidget::setActiveWindow()
{
    qDebug( "QWidget::setActiveWindow" );
    QWidget *widget = QWidget::find( myactive );
    // FIXME: This is likely to flicker
    if ( widget && !widget->isPopup() )
        widget = 0;
    if ( !parentWidget() ) {
	SelectWindow( (WindowPtr)winid );  // FIXME: Also brings to front - naughty?
	erase( 0, 0, width(), height() );
	myactive = winid;
    }
    if ( !isPopup() && widget )
	widget->setActiveWindow();
}


/*!
  Updates the widget unless updates are disabled or the widget is hidden.

  Updating the widget will erase the widget contents and generate an
  appropriate paint event for the invalidated region. The paint event
  is processed after the program has returned to the main event loop.
  Calling update() many times in a row will generate a single paint
  event.

  If the widgets sets the WRepaintNoErase flag, update() will not erase
  its contents.

  \sa repaint(), paintEvent(), setUpdatesEnabled(), erase(), setWFlags()
*/

void QWidget::update()
{
    update( 0, 0, width(), height() );    
    qDebug( "QWidget::update" );
}

/*!
  Updates a rectangle (\e x, \e y, \e w, \e h) inside the widget
  unless updates are disabled or the widget is hidden.

  Updating the widget erases the widget area \e (x,y,w,h) and generate
  an appropriate paint event for the invalidated region. The paint
  event is processed after the program has returned to the main event
  loop.  Calling update() many times in a row will generate a single
  paint event.

  If \e w is negative, it is replaced with <code>width() - x</code>.
  If \e h is negative, it is replaced width <code>height() - y</code>.


  If the widgets sets the WRepaintNoErase flag, update() will not erase
  its contents.

  \sa repaint(), paintEvent(), setUpdatesEnabled(), erase()
*/

void QWidget::update( int x, int y, int w, int h )
{
    qDebug( "QWidget::update" );
    if ( testWFlags( WState_Created ) ) {
	if ( !isVisible() )
	    return;
	erase( x, y, w, h );
	Rect r;
	int x1 = x;
	int y1 = y;
	int x2 = x + w;
	int y2 = y + h;
	make_top( this, x1, y1 );
	make_top( this, x2, y2);
	SetRect( &r, x1, y1, x2, y2 );
	InvalWindowRect( (WindowRef)winId(), &r );
    }
}

/*!
  \overload void QWidget::update( const QRect &r )
*/

/*!
  \overload void QWidget::repaint( bool erase )

  This version repaints the entire widget.
*/

/*!
  \overload void QWidget::repaint()

  This version erases and repaints the entire widget.
*/

/*!
  Repaints the widget directly by calling paintEvent() directly,
  unless updates are disabled or the widget is hidden.

  Erases the widget area  \e (x,y,w,h) if \e erase is TRUE.

  If \e w is negative, it is replaced with <code>width() - x</code>.
  If \e h is negative, it is replaced width <code>height() - y</code>.

  Use repaint if your widget needs to be repainted immediately, for
  example when doing some animation. In all other cases, update() is
  to be preferred. Calling update() many times in a row will generate
  a single paint event.

  \warning If you call repaint() in a function which may itself be called
  from paintEvent(), you may see infinite recursion. The update() function
  never generates recursion.

  \sa update(), paintEvent(), setUpdatesEnabled(), erase()
*/

void QWidget::repaint( int x, int y, int w, int h, bool erase )
{
    qDebug( "QWidget::repaint" );
    if ( (widget_state & (WState_Visible|WState_BlockUpdates)) 
	 == WState_Visible ) {
	if ( w < 0 )
	    w = crect.width()  - x;
	if ( h < 0 )
	    h = crect.height() - y;
	QPaintEvent e( QRect(x, y, w, h ), erase );
	if ( erase && w != 0 && h != 0 )
	    this->erase( x, y, w, h );
	QApplication::sendEvent( this, &e );
    }
}

/*!
  Repaints the widget directly by calling paintEvent() directly,
  unless updates are disabled or the widget is hidden.

  Erases the widget region  \a reg if \a erase is TRUE.

  Use repaint if your widget needs to be repainted immediately, for
  example when doing some animation. In all other cases, update() is
  to be preferred. Calling update() many times in a row will generate
  a single paint event.

  \warning If you call repaint() in a function which may itself be called
  from paintEvent(), you may see infinite recursion. The update() function
  never generates recursion.

  \sa update(), paintEvent(), setUpdatesEnabled(), erase()
*/

void QWidget::repaint( const QRegion& , bool erase )
{
    qDebug( "QWidget::repaint QRegion" );
    repaint( 0, 0, width(), height(), erase );
}

/*!
  \overload void QWidget::repaint( const QRect &r, bool erase )
*/


/*!
  \internal
  Platform-specific part of QWidget::show().
*/

void QWidget::showWindow()
{
    qDebug( "QWidget::showWindow" );
    setWState( WState_Visible );
    clearWState( WState_ForceHide );
    QShowEvent e( FALSE );
    QApplication::sendEvent( this, &e );
    if ( !parentWidget() )
	ShowHide( (WindowPtr)winid, 1 );
    setActiveWindow();
    QApplication::postEvent( this, new QPaintEvent( rect() ) );
    if ( parentWidget() )
	parentWidget()->update();
    erase( 0, 0, width(), height() );
    show_children( this, 1 );
}


/*!
  \internal
  Platform-specific part of QWidget::hide().
*/

void QWidget::hideWindow()
{
    qDebug( "QWidget::hide" );
}


/*!
  Shows the widget minimized, as an icon.

  Calling this function has no effect for other than \link isTopLevel()
  top-level widgets\endlink.

  \sa showNormal(), showMaximized(), show(), hide(), isVisible(), isMinimized()
*/

void QWidget::showMinimized()
{
    qDebug( "QWidget::showMinimized" );
}

/*!
  Returns TRUE if this widget is a top-level widget that is minimized
  (iconified), or else FALSE.

  \sa showMinimized(), isVisible(), show(), hide(), showNormal()
 */
bool QWidget::isMinimized() const
{
    qDebug( "QWidget::isMinimized" );
    return false;
}

bool QWidget::isMaximized() const
{
    qDebug( "QWidget::isMaximized" );
    return false;
}


/*!
  Shows the widget maximized.

  Calling this function has no effect for other than \link isTopLevel()
  top-level widgets\endlink.

  On X11, this function may not work properly with certain window
  managers. See the \link geometry.html Window Geometry
  documentation\endlink for details on why.

  \sa showNormal(), showMinimized(), show(), hide(), isVisible()
*/

void QWidget::showMaximized()
{
    qDebug( "QWidget::showMaximized" );
}

/*!
  Restores the widget after it has been maximized or minimized.

  Calling this function has no effect for other than \link isTopLevel()
  top-level widgets\endlink.

  \sa showMinimized(), showMaximized(), show(), hide(), isVisible()
*/

void QWidget::showNormal()
{
    qDebug( "QWidget::showNormal" );
}


/*!
  Raises this widget to the top of the parent widget's stack.

  If there are any siblings of this widget that overlap it on the screen,
  this widget will be visually in front of its siblings afterwards.

  \sa lower(), stackUnder()
*/

void QWidget::raise()
{
    qDebug( "QWidget::raise" );
}

/*!
  Lowers the widget to the bottom of the parent widget's stack.

  If there are siblings of this widget that overlap it on the screen, this
  widget will be obscured by its siblings afterwards.

  \sa raise(), stackUnder()
*/

void QWidget::lower()
{
    qDebug( "QWidget::lower" );
}


/*!
  Places the widget under \a w in the parent widget's stack.

  To make this work, the widget itself and \a w have to be siblings.

  \sa raise(), lower()
*/
void QWidget::stackUnder( QWidget*)
{
    qDebug( "QWidget::stackUnder" );
}



/*
  The global variable qt_widget_tlw_gravity defines the window gravity of
  the next top level window to be created. We do this when setting the
  main widget's geometry and the "-geometry" command line option contains
  a negative position.
*/

void QWidget::internalSetGeometry( int x, int y, int w, int h, bool isMove )
{
    qDebug( QString( "QWidget::internalSetGeometry x=%1 y=%2 w=%3 h=%4" ).arg( x ).arg( y ).arg( w ).arg( h ) );
    if ( testWFlags(WType_Desktop) ) {
	return;
    }
    if ( w < 1 )                                // invalid size
        w = 1;
    if ( h < 1 )
        h = 1;
    QPoint oldp = pos();
    QSize  olds = size();
    // Deal with size increment
    if ( extra ) {
	if ( extra->topextra ) {
	    if ( extra->topextra->incw ) {
		w = w/extra->topextra->incw;
		w = w*extra->topextra->incw;
	    }
	    if ( extra->topextra->inch ) {
		h = h/extra->topextra->inch;
		h = h*extra->topextra->inch;
	    }
	}
    }
    QRect  r( x, y, w, h );
    if ( r.size() == olds && oldp == r.topLeft() && (isTopLevel() == FALSE ) ) { 
        return;
    }

    setCRect( r );

    if ( !parentWidget() && isMove && winid )
        MoveWindow((WindowPtr)winid,x,y,1);

    bool isResize = olds != r.size();
    if ( !parentWidget() && winid )
        SizeWindow((WindowPtr)winid,w,h,1);

    if ( isVisible() ) {
        if ( isMove ) {
            QMoveEvent e( r.topLeft(), oldp );
            QApplication::sendEvent( this, &e );
            repaint(TRUE);
        }
        if ( isResize ) {
            QResizeEvent e( r.size(), olds );
            QApplication::sendEvent( this, &e );
            if ( !testWFlags( WResizeNoErase ) )
                repaint( TRUE );
        }
    } else {
        if ( isMove )
            QApplication::postEvent( this,
                                     new QMoveEvent( r.topLeft(), oldp ) );
        if ( isResize )
            QApplication::postEvent( this,
                                     new QResizeEvent( r.size(), olds ) );
    }

    /* FIXME - this was erasing the window when I resized it to be smaller.
    if(parentWidget()) {
	redraw_children(parentWidget());
    } else {
        redraw_children(this);
    }
    */
}

/*!
  \overload void QWidget::setMinimumSize( const QSize &size )
*/

/*!
  Sets the minimum size of the widget to \e w by \e h pixels.

  The widget cannot be resized to a smaller size than the minimum widget
  size. The widget's size is forced to the minimum size if the current
  size is smaller.

  If you use a layout inside the widget, the minimum size will be set by the layout and
  not by setMinimumSize, unless you set the layouts resize mode to QLayout::FreeResize.

  \sa minimumSize(), setMaximumSize(), setSizeIncrement(), resize(), size(), QLayout::setResizeMode()
*/

void QWidget::setMinimumSize( int, int )
{
    qDebug( "QWidget::setMinimumSize" );
}

/*!
  \overload void QWidget::setMaximumSize( const QSize &size )
*/

/*!
  Sets the maximum size of the widget to \e w by \e h pixels.

  The widget cannot be resized to a larger size than the maximum widget
  size. The widget's size is forced to the maximum size if the current
  size is greater.

  \sa maximumSize(), setMinimumSize(), setSizeIncrement(), resize(), size()
*/

void QWidget::setMaximumSize( int, int )
{
    qDebug( "QWidget::setMaximumSize" );
}

/*!
  Sets the size increment of the widget.  When the user resizes the
  window, the size will move in steps of \e w pixels horizontally and
  \e h pixels vertically, with baseSize() as basis. Preferred widget sizes are therefore for
  non-negative integers \e i and \e j:
  \code
  width = baseSize().width() + i * sizeIncrement().width();
  height = baseSize().height() + j * sizeIncrement().height();
  \endcode

  Note that while you can set the size increment for all widgets, it
  has no effect except for top-level widgets.

  \warning The size increment has no effect under Windows, and may be
  disregarded by the window manager on X.

  \sa sizeIncrement(), setMinimumSize(), setMaximumSize(), resize(), size()
*/

void QWidget::setSizeIncrement( int, int )
{
    qDebug( "QWidget::setSizeIncrement" );
}
/*!
  \overload void QWidget::setSizeIncrement( const QSize& )
*/


/*!
  Sets the base size of the widget.  The base size is important only
  in combination with size increments. See setSizeIncrement() for details.

  \sa baseSize()
*/

void QWidget::setBaseSize( int, int )
{
    qDebug( "QWidget::setSizeIncrement" );
}



/*!
  \overload void QWidget::setBaseSize( const QSize& )
*/

/*!
  \overload void QWidget::erase()
  This version erases the entire widget.
*/

/*!
  \overload void QWidget::erase( const QRect &r )
*/

/*!
  Erases the specified area \e (x,y,w,h) in the widget without generating
  a \link paintEvent() paint event\endlink.

  If \e w is negative, it is replaced with <code>width() - x</code>.
  If \e h is negative, it is replaced width <code>height() - y</code>.

  Child widgets are not affected.

  \sa repaint()
*/

void QWidget::erase( int x, int y, int w, int h )
{
    qDebug( QString( "QWidget::erase %1 %2 %3 %4" ).arg( x ).arg( y ).arg( w ).arg( h ) );
    if ( back_type == 1 ) {
	// solid background
	Rect r;
	RGBColor rc;
	rc.red = bg_col.red()*256;
	rc.green = bg_col.green()*256;
	rc.blue = bg_col.blue()*256;
	this->lockPort();
	RGBForeColor( &rc );
	x--;
	y--;
	w += 2;
	h += 2;
	if ( x < 0 )
	    x = 0;
	if ( y < 0 )
	    y = 0;
	if ( w > width() )
	    w = width();
	if ( h > height() )
	    h = height();
	SetRect( &r, x, y, x + w, y + h );
	PaintRect( &r );
	this->unlockPort();
    } else if ( back_type == 2 ) {
	// pixmap
	if ( bg_pix ) {
	    QPainter p;
	    p.begin( this );
	    p.drawTiledPixmap( x, y, w, h, *bg_pix, 0, 0 );
	    p.end();
	}
    } else {
	// nothing
    }
}

/*!
  Erases the area defined by \a reg, without generating a
  \link paintEvent() paint event\endlink.

  Child widgets are not affected.
*/

void QWidget::erase( const QRegion& reg )
{
    qDebug( "QWidget::erase QRegion" );
    RGBColor rc;
    this->lockPort();
    rc.red = bg_col.red()*256;
    rc.green = bg_col.green()*256;
    rc.blue = bg_col.blue()*256;
    RGBForeColor( &rc );
    PaintRgn( (RgnHandle)reg.handle() );
    this->unlockPort();
}


/*! \overload

  This version of the function scrolls the entire widget and moves the
  widget's children along with the scroll.

  \sa bitBlt() QScrollView
*/

void QWidget::scroll( int, int )
{
    qDebug( "QWidget::scroll" );
}

/*! Scrolls \a r \a dx pixels to the right and \a dy downwards.  Both
  \a dx and \a dy may be negative.

  If \a r is empty or invalid, the result is undefined.

  After scrolling, scroll() sends a paint event for the the part of \a r
  that is read but not written.  For example, when scrolling 10 pixels
  rightwards, the leftmost ten pixels of \a r need repainting. The paint
  event may be delivered immediately or later, depending on some heuristics.

  This version of scroll() does not move the children of this widget.

  \sa QScrollView erase() bitBlt()
*/
void QWidget::scroll( int, int, const QRect& )
{
    qDebug( "QWidget::scroll" );
}


/*!
  \overload void QWidget::drawText( const QPoint &pos, const QString& str )
*/

/*!
  Writes \e str at position \e x,y.

  The \e y position is the base line position of the text.  The text is
  drawn using the default font and the default foreground color.

  This function is provided for convenience.  You will generally get
  more flexible results and often higher speed by using a a \link
  QPainter painter\endlink instead.

  \sa setFont(), foregroundColor(), QPainter::drawText()
*/

void QWidget::drawText( int, int, const QString & )
{
    qDebug( "QWidget::drawText" );
}


/*!
  Internal implementation of the virtual QPaintDevice::metric() function.

  Use the QPaintDeviceMetrics class instead.
*/

int QWidget::metric( int m ) const
{
    qDebug( "QPaintDevice::metric" );
    WindowPtr p = (WindowPtr)winid;
    if ( m == QPaintDeviceMetrics::PdmWidth ) {
	if ( parentWidget() ) {
	    return crect.width();
	} else {
  	    Rect windowBounds;
            GetPortBounds( GetWindowPort( p ), &windowBounds );
            return windowBounds.right;
	}
    } else if( m == QPaintDeviceMetrics::PdmHeight ) {
	if ( parentWidget() ) {
	    return crect.height();
	} else {
  	    Rect windowBounds;
            GetPortBounds( GetWindowPort( p ), &windowBounds );
            return windowBounds.bottom;
	}
    } else if ( m == QPaintDeviceMetrics::PdmWidthMM ) {
	return metric( QPaintDeviceMetrics::PdmWidth );
    } else if ( m == QPaintDeviceMetrics::PdmHeightMM ) {
	return metric( QPaintDeviceMetrics::PdmHeight );
    } else if ( m == QPaintDeviceMetrics::PdmNumColors ) {
	return 16;
    } else if ( m == QPaintDeviceMetrics::PdmDepth ) {
	// FIXME : this is a lie in most cases
	return 16;
    } else {
        // FIXME: Handle this case
	qWarning("QWidget::metric unhandled parameter %d",m);
    }
    return 0;
}

void QWidget::createSysExtra()
{
    qDebug( "QWidget::createSysExtra" );
}

void QWidget::deleteSysExtra()
{
    qDebug( "QWidget::deleteSysExtra" );
}

void QWidget::createTLSysExtra()
{
    qDebug( "QWidget::createTLSysExtra" );
}

void QWidget::deleteTLSysExtra()
{
    qDebug( "QWidget::deleteTLSysExtra" );
}


/*!
  Returns TRUE if drop events are enabled for this widget.

  \sa setAcceptDrops()
*/

bool QWidget::acceptDrops() const
{
    qDebug( "QWidget::acceptDrops" );
    return false;
}

/*!
  Announces to the system that this widget \e may be able to
  accept drop events.

  If the widgets is \link QWidget::isDesktop() the desktop\endlink,
  this may fail if another application is using the desktop - you
  can call acceptDrops() to test if this occurs.

  \sa acceptDrops()
*/

void QWidget::setAcceptDrops( bool )
{
    qDebug( "QWidget::isDesktop" );
}

/*!
  Causes only the parts of the widget which overlap \a region
  to be visible.  If the region includes pixels outside the
  rect() of the widget, window system controls in that area
  may or may not be visible, depending on the platform.

  Note that this effect can be slow if the region is particularly
  complex.

  \sa setMask(QBitmap), clearMask()
*/

void QWidget::setMask( const QRegion& )
{
    qDebug( "QWidget::setMask" );
}

/*!
  Causes only the pixels of the widget for which \a bitmap
  has a corresponding 1 bit
  to be visible.  If the region includes pixels outside the
  rect() of the widget, window system controls in that area
  may or may not be visible, depending on the platform.

  Note that this effect can be slow if the region is particularly
  complex.

  \sa setMask(const QRegion&), clearMask()
*/

void QWidget::setMask( const QBitmap & )
{
    qDebug( "QWidget::setMask" );
}

/*!
  Removes any mask set by setMask().

  \sa setMask()
*/

void QWidget::clearMask()
{
    qDebug( "QWidget::clearMask" );
}

/*!\reimp
 */
void QWidget::setName( const char * )
{
    qDebug( "QWidget::setName" );
}


//FIXME: untested
void QWidget::propagateUpdates(int x, int y, int x2, int y2)
{
    qDebug( "QWidget::propagateUpdates" );

    this->lockPort();
    erase( x, y, x2, y2 );
    QRect paintRect( x, y, x2, y2 );
    QRegion paintRegion( paintRect );
    QPaintEvent e( paintRegion );
    setWState( WState_InPaintEvent );
    QApplication::sendEvent( this, &e );
    clearWState( WState_InPaintEvent );

    int a, b, c, d;
    const QObjectList *childList = children();
    if ( childList ) {
	QObjectListIt it(*childList);
	QObject *child;
	QWidget *childWidget;
	child = it.toLast();
	do {
	    if ( child->inherits( "QWidget" ) ) {
		childWidget = (QWidget *)child;
		a = x;
		b = y;
		c = x2;
		d = y2;
		a -= childWidget->x();
		b -= childWidget->y();
		c -= childWidget->x();
		d -= childWidget->y();
		if ( a < childWidget->width() && b < childWidget->height() &&
		    c > 0 && d > 0 ) {
		    childWidget->propagateUpdates( a, b, c, d );
		}
	    }
	    child = --it;
	} while ( child != 0 );
    }
    this->unlockPort();
    qDebug( "leaving QWidget::propagateUpdates" );
}

//FIXME: I think function was used to define a clipping region
//FIXME: Basically in ensures that I widget doesn't draw over
//FIXME: The top of child widgets
//FIXME: Maybe we should use Qt/Embedded code for doing this.
void QWidget::lockPort()
{
    if ( !hd )
	return;

    int x = 0;
    int y = 0;
    SetPortWindowPort( (WindowPtr)hd );
    SetOrigin( 0, 0 );
    make_top( this, x, y );
    return; //FIXME: NO CLIPPING
}

void QWidget::unlockPort() { } //does nothing

BitMap
*QWidget::portBitMap() const
{
  return (BitMap *)*GetPortPixMap(GetWindowPort((WindowPtr)hd));
}
