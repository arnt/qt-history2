/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qwidget.cpp#718 $
**
** Implementation of QWidget class
**
** Created : 931031
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
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
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


#include "qobjectlist.h"
#include "qwidget.h"
#include "qwidgetlist.h"
#include "qwidgetintdict.h"
#include "qptrdict.h"
#include "qfocusdata.h"
#include "qcursor.h"
#include "qpixmap.h"
#include "qapplication.h"
#include "qapplication_p.h"
#include "qbrush.h"
#include "qlayout.h"
#include "qstylefactory.h"
#include "qcleanuphandler.h"
#if defined(QT_ACCESSIBILITY_SUPPORT)
#include "qaccessiblewidget.h"
#endif
#if defined(Q_WS_WIN)
#include "qt_windows.h"
#endif
#if defined(Q_WS_QWS)
#include "qwsmanager_qws.h"
#endif

/*!
  \class QWidget qwidget.h
  \brief The QWidget class is the base class of all user interface objects.

  \ingroup abstractwidgets

  The widget is the atom of the user interface: It receives mouse,
  keyboard and other events from the window system, and paints a
  representation of itself on the screen.  Every widget is
  rectangular, and they are sorted in a Z-order.  A widget is clipped
  by its parent and by the widgets in front of it.

  A widget that isn't embedded in a parent widget is called a
  top-level widget. Usually, top-level widgets are windows with a
  frame and a title bar (though it is also possible to create top
  level widgets without such decoration by the use of <a
  href="qt.html#WidgetFlags">widget flags</a>).  In Qt, QMainWindow and the
  various subclasses of QDialog are the most common top-level windows.

  A widget without a parent widget is always a top-level widget.

  The opposite of top-level widgets are child widgets. Those are child
  windows in their parent widgets.  You usually cannot distinguish a
  child widget from its parent visually.  Most other widgets in Qt are
  useful only as child widgets.  (You \e can make a e.g. button into a
  top-level widget, but most people prefer to put their buttons in
  e.g. dialogs.)

  QWidget has many member functions, but some of them have little
  direct functionality - for example it has a font but never uses it
  itself. There are many subclasses which provide real functionality,
  as diverse as QPushButton, QListBox and QTabDialog.

  <strong>Groups of functions:</strong>
  <ul>

  <li> Window functions:
	show(),
	hide(),
	raise(),
	lower(),
	close().

  <li> Top level windows:
	caption(),
	setCaption(),
	icon(),
	setIcon(),
	iconText(),
	setIconText(),
	isActiveWindow(),
	setActiveWindow(),
	showMinimized().
	showMaximized(),
	showFullScreen(),
	showNormal().

  <li> Window contents:
	update(),
	repaint(),
	erase(),
	scroll(),
	updateMask().

  <li> Geometry:
	pos(),
	size(),
	rect(),
	x(),
	y(),
	width(),
	height(),
	sizePolicy(),
	setSizePolicy(),
	sizeHint(),
	updateGeometry(),
	layout(),
	move(),
	resize(),
	setGeometry(),
	frameGeometry(),
	geometry(),
	childrenRect(),
	adjustSize(),
	mapFromGlobal(),
	mapFromParent()
	mapToGlobal(),
	mapToParent(),
	maximumSize(),
	minimumSize(),
	sizeIncrement(),
	setMaximumSize(),
	setMinimumSize(),
	setSizeIncrement(),
	setBaseSize(),
	setFixedSize()

  <li> Mode:
	isVisible(),
	isVisibleTo(),
	visibleRect(),
	isMinimized(),
	isDesktop(),
	isEnabled(),
	isEnabledTo(),
	isModal(),
	isPopup(),
	isTopLevel(),
	setEnabled(),
	hasMouseTracking(),
	setMouseTracking(),
	isUpdatesEnabled(),
	setUpdatesEnabled(),

  <li> Look and feel:
	style(),
	setStyle(),
	cursor(),
	setCursor()
	font(),
	setFont(),
	palette(),
	setPalette(),
	backgroundMode(),
	setBackgroundMode(),
	backgroundPixmap(),
	setBackgroundPixmap(),
	backgroundColor(),
	colorGroup(),
	fontMetrics(),
	fontInfo().

  <li> Keyboard focus functions:
	isFocusEnabled(),
	setFocusPolicy(),
	focusPolicy(),
	hasFocus(),
	setFocus(),
	clearFocus(),
	setTabOrder(),
	setFocusProxy().

  <li> Mouse and keyboard grabbing:
	grabMouse(),
	releaseMouse(),
	grabKeyboard(),
	releaseKeyboard(),
	mouseGrabber(),
	keyboardGrabber().

  <li> Event handlers:
	event(),
	mousePressEvent(),
	mouseReleaseEvent(),
	mouseDoubleClickEvent(),
	mouseMoveEvent(),
	keyPressEvent(),
	keyReleaseEvent(),
	focusInEvent(),
	focusOutEvent(),
	wheelEvent(),
	enterEvent(),
	leaveEvent(),
	paintEvent(),
	moveEvent(),
	resizeEvent(),
	closeEvent(),
	dragEnterEvent(),
	dragMoveEvent(),
	dragLeaveEvent(),
	dropEvent(),
	childEvent(),
	showEvent(),
	hideEvent(),
	customEvent().

  <li> Change handlers:
	backgroundColorChange(),
	backgroundPixmapChange(),
	enabledChange(),
	fontChange(),
	paletteChange(),
	styleChange(),
	windowActivationChange().

  <li> System functions:
	parentWidget(),
	topLevelWidget(),
	reparentolish((),
	winId(),
	find(),
	metric().

  <li> Internal kernel functions:
	focusNextPrevChild(),
	wmapper(),
	clearWFlags(),
	getWFlags(),
	setWFlags(),
	testWFlags().

  <li> What's this help:
	customWhatsThis()
  </ul>

  Every widget's constructor accepts two or three standard arguments:
  <ul>
  <li><code>QWidget *parent = 0</code> is the parent of the new widget.
  If it is 0 (the default), the new widget will be a top-level window.
  If not, it will be a child of \e parent, and be constrained by \e
  parent's geometry (Unless you specify \c WType_TopLevel as
  widget flag).
  <li><code>const char *name = 0</code> is the widget name of the new
  widget.  You can access it using name().  The widget name is little
  used by programmers but is quite useful with GUI builders such as the
  Qt Designer (you can name a widget in the builder, and connect() to
  it by name in your code).  The dumpObjectTree() debugging function also
  uses it.
  <li><code>WFlags f = 0</code> (where available) sets the <a
  href="qt.html#WidgetFlags">widget flags</a>; the default is good for almost
  all widgets, but to get e.g. top-level widgets without a window
  system frame you must use special flags.
  </ul>

  The tictac/tictac.cpp example program is good example of a simple
  widget.  It contains a few event handlers (as all widgets must), a
  few custom routines that are peculiar to it (as all useful widgets
  must), and has a few children and connections.  Everything it does
  is done in response to an event: This is by far the most common way
  to design GUI applications.

  You will need to supply the content for your widgets yourself, but
  here is a brief run-down of the events, starting with the most common
  ones: <ul>

  <li> paintEvent() - called whenever the widget needs to be
  repainted.  Every widget which displays output must implement it,
  and it is sensible to \e never paint on the screen outside
  paintEvent().

  <li> resizeEvent() - called when the widget has been resized.

  <li> mousePressEvent() - called when a mouse button is pressed.
  There are six mouse-related events, mouse press and mouse release
  events are by far the most important.  A widget receives mouse press
  events when the widget is inside it, or when it has grabbed the
  mouse using grabMouse().

  <li> mouseReleaseEvent() - called when a mouse button is released.
  A widget receives mouse release events when it has received the
  corresponding mouse press event.  This means that if the user
  presses the mouse inside \e your widget, then drags the mouse to
  somewhere else, then releases, \e your widget receives the release
  event.  There is one exception, however: If a popup menu appears
  while the mouse button is held down, that popup steals the mouse
  events at once.

  <li> mouseDoubleClickEvent() - not quite as obvious as it might seem.
  If the user double-clicks, the widget receives a mouse press event
  (perhaps a mouse move event or two if he/she does not hold the mouse
  quite steady), a mouse release event and finally this event.  It is \e
  not \e possible to distinguish a click from a double click until you've
  seen whether the second click arrives.  (This is one reason why most GUI
  books recommend that double clicks be an extension of single clicks,
  rather than trigger a different action.)
  </ul>

  If your widget only contains child widgets, you probably do not need to
  implement any event handlers.

  Widgets that accept keyboard input need to reimplement a few more
  event handlers: <ul>

  <li> keyPressEvent() - called whenever a key is pressed, and again
  when a key has been held down long enough for it to auto-repeat.
  Note that the Tab and shift-Tab keys are only passed to the widget
  if they are not used by the focus-change mechanisms.  To force those
  keys to be processed by your widget, you must reimplement
  QWidget::event().

  <li> focusInEvent() - called when the widget gains keyboard focus
  (assuming you have called setFocusPolicy(), of course). Well
  written widgets indicate that they own the keyboard focus in a clear
  but discreet way.

  <li> focusOutEvent() - called when the widget loses keyboard
  focus.
  </ul>

  Some widgets will need to reimplement some more obscure event
  handlers, too: <ul>

  <li> mouseMoveEvent() - called whenever the mouse moves while a
  button is held down.  This is useful for e.g. dragging.  If you call
  setMouseTracking(TRUE), you get mouse move events even when no
  buttons are held down.  (Note that applications which make use of
  mouse tracking are often not very useful on low-bandwidth X
  connections.)

  <li> keyReleaseEvent() - called whenever a key is released, and also
  while it is held down if the key is auto-repeating.  In that case
  the widget receives a key release event and immediately a key press
  event for every repeat.  Note that the Tab and shift-Tab keys are
  only passed to the widget if they are not used by the focus-change
  mechanisms.  To force those keys to be processed by your widget, you
  must reimplement QWidget::event().

  <li> wheelEvent() -- called whenever the user turns the mouse wheel
  while the widget has the focus.

  <li> enterEvent() - called when the mouse enters the widget's screen
  space.  (This excludes screen space owned by any children of the
  widget.)

  <li> leaveEvent() - called when the mouse leaves the widget's screen
  space.

  <li> moveEvent() - called when the widget has been moved relative to its
  parent.

  <li> closeEvent() - called when the user closes the widget (or when
  close() is called).
  </ul>

  There are also some \e really obscure events.  They are listed in
  qevent.h and you need to reimplement event() to handle them.  The
  default implementation of event() handles Tab and shift-Tab (to move
  the keyboard focus), and passes on most other events to one of the
  more specialized handlers above.

  When writing a widget, there are a few more things to look out
  for. <ul>

  <li> In the constructor, be sure to set up your member variables
  early on, before there's any chance that you might receive an event.

  <li>It is almost always useful to reimplement sizeHint() and to set
  the correct size policy with setSizePolicy(), so users of your class
  can set up layout management more easily.  A size policy lets you
  supply good defaults for the layout management handling, so that
  other widgets can contain and manage yours easily.  sizeHint()
  indicates a "good" size for the widget.

  <li>If your widget is a top-level window, setCaption() and setIcon() set
  the title bar and icon respectively.

  </ul>

  \sa QEvent, QPainter, QGridLayout, QBoxLayout
*/


/*****************************************************************************
  Internal QWidgetMapper class

  The purpose of this class is to map widget identifiers to QWidget objects.
  All QWidget objects register themselves in the QWidgetMapper when they
  get an identifier. Widgets unregister themselves when they change ident-
  ifier or when they are destroyed. A widget identifier is really a window
  handle.

  The widget mapper is created and destroyed by the main application routines
  in the file qapp_xxx.cpp.
 *****************************************************************************/

#ifdef Q_WS_QWS
static const int WDictSize = 163; // plenty for small devices
#else
static const int WDictSize = 1123; // plenty for 5 big complex windows
#endif

class QWidgetMapper : public QWidgetIntDict
{						// maps ids -> widgets
public:
    QWidgetMapper();
   ~QWidgetMapper();
    QWidget *find( WId id );		// find widget
    void     insert( const QWidget * );		// insert widget
    bool     remove( WId id );		// remove widget
private:
    WId	     cur_id;
    QWidget *cur_widget;
};

QWidgetMapper *QWidget::mapper = 0;		// app global widget mapper


QWidgetMapper::QWidgetMapper() : QWidgetIntDict(WDictSize)
{
    cur_id = 0;
    cur_widget = 0;
}

QWidgetMapper::~QWidgetMapper()
{
    clear();
}

inline QWidget *QWidgetMapper::find( WId id )
{
    if ( id != cur_id ) {			// need to lookup
	cur_widget = QWidgetIntDict::find((long)id);
	if ( cur_widget )
	    cur_id = id;
	else
	    cur_id = 0;
    }
    return cur_widget;
}

inline void QWidgetMapper::insert( const QWidget *widget )
{
    QWidgetIntDict::insert((long)widget->winId(),widget);
}

inline bool QWidgetMapper::remove( WId id )
{
    if ( cur_id == id ) {			// reset current widget
	cur_id = 0;
	cur_widget = 0;
    }
    return QWidgetIntDict::remove((long)id);
}


/*****************************************************************************
  QWidget member functions
 *****************************************************************************/

/*
    Widget state flags:
  <dl compact>
  <dt>WState_Created<dd> The widget has a valid winId().
  <dt>WState_Disabled<dd> The widget does not receive any mouse
       or keyboard events.
  <dt>WState_ForceDisabled<dd> The widget is explicitly disabled, i.e. it will remain
	disabled even when all its ancestors are set to enabled
	state. This implies WState_Disabled.
  <dt>WState_Visible<dd> The widget is currently visible.
  <dt>WState_ForceHide<dd> The widget is explicitly hidden, i.e. it won't become
      visible unless you call show() on it.  ForceHide implies !WState_Visible
  <dt>WState_OwnCursor<dd> A cursor has been set for this widget.
  <dt>WState_MouseTracking<dd> Mouse tracking is enabled.
  <dt>WState_CompressKeys<dd> Compress keyboard events.
  <dt>WState_BlockUpdates<dd> Repaints and updates are disabled.
  <dt>WState_InPaintEvent<dd> Currently processing a paint event.
  <dt>WState_Reparented<dd> The widget has been reparented.
  <dt>WState_ConfigPending<dd> A config (resize/move) event is pending.
  <dt>WState_Resized<dd> The widget has been resized.
  <dt>WState_AutoMask<dd> The widget has an automatic mask, see setAutoMask().
  <dt>WState_Polished<dd> The widget has been "polished" (i.e. late initializated ) by a QStyle.
  <dt>WState_DND<dd> The widget supports drag and drop, see setAcceptDrops().
  <dt>WState_Exposed<dd> the widget was finally exposed (x11 only,
      helps avoiding paint event doubling).
  </dl>
*/


/*! \enum Qt::WidgetFlags

\keyword widget flags

This enum type is used to specify various window-system properties
of the widget.  They are fairly unusual but necessary in a
few cases.

The main types are

\value WType_TopLevel  indicates that this widget is a top-level
widget, usually with a window-system frame and so on.

\value WType_Dialog  indicates that this widget is a secondary
top-level widget.  In combination with \c WShowModal, the dialog becomes
a modal dialog ie. it prevents widgets in all other top-level widget
from getting any input. \c WType_Dialog implies \c WType_TopLevel.

\value WType_Popup  indicates that this widget is a popup top-level
window, ie., that it is modal, but has a window system frame appropriate
for popup menus.\c WType_Popup implies WType_TopLevel.

\value WType_Desktop  indicates that this widget is the desktop.
See also \c WPaintDesktop below. \c WType_Desktop implies \c WType_TopLevel.

There are also a number of flags to let you customize the appearance
of top-level windows.  These have no effect on other windows:

\value WStyle_Customize  indicates that the \c WStyle_* flags should be
used to build the window instead of the default.

\value WStyle_NormalBorder  gives the window a normal border. Cannot
be combined with \c WStyle_DialogBorder or \c WStyle_NoBorder.

\value WStyle_DialogBorder  gives the window a thin dialog border.
Cannot be combined with \c WStyle_NormalBorder or \c WStyle_NoBorder.

\value WStyle_NoBorder  gives a borderless window.  Note that the
user cannot move or resize a borderless window via the window system.
Cannot be combined with \c WStyle_NormalBorder or \c WStyle_DialogBorder.
On Windows, the flag works fine. On X11, it bypasses the window manager
completely. This results in a borderless window, but also in a window that
is not managed at all (i.e. for example no keyboard focus unless you call
QWidget::setActiveWindow() manually.) For compatibility, the flag was not changed
for Qt-2.1. We suggest using \c WStyle_NoBorderEx instead.

\value WStyle_NoBorderEx  gives a borderless window.  Note that the user
cannot move or resize a borderless window via the window system.  Cannot
be combined with \c WStyle_NormalBorder or \c WStyle_DialogBorder. On X11,
the result of the flag is depending on the window manager and its ability
to understand MOTIF hints to some \c WStyle_DialogBorder. On X11 the result
of the flag is depending on the window manager and its ability to
understand MOTIF hints to some degree.  Most existing modern window
managers do this. With \c WX11BypassWM, you can bypass the window manager
completely. This results in a borderless window for sure, but also in a
window that is not managed at all (i.e., no keyboard input unless you call
setActiveWindow() manually).

\value WStyle_Title  gives the window a title bar.

\value WStyle_SysMenu  adds a window system menu.

\value WStyle_Minimize  adds a minimize button.  Note that on Windows
this has to be combined with \c WStyle_SysMenu for it to work.

\value WStyle_Maximize  adds a maximize button.  Note that on Windows
this has to be combined with \c WStyle_SysMenu for it to work.

\value WStyle_MinMax  is equal to \c WStyle_Minimize|WStyle_Maximize.
Note that on Windows this has to be combined with \c WStyle_SysMenu to work.

\value WStyle_ContextHelp  adds a context help button to dialogs.

\value WStyle_Tool  makes the window a tool window.  A tool window
is a small window that lives for a short time, and it is typically used
for creating popup windows.  It there is a parent, the tool window
will always be kept on top of it.  If there isn't a parent, you may
consider passing \c WStyle_StaysOnTop as well.  If the window system
supports it, a tool window can be decorated with a somewhat lighter
frame.  It can also be combined with \c WStyle_NoBorder.

\value WStyle_StaysOnTop  informs the window system that the window
should stay on top of all other windows.

\value WStyle_Dialog  indicates that the window is a logical subwindow
of its parent (in other words, a dialog).  The window will not get its own
taskbar entry and be kept on top of its parent by the window system.
Usually it will also be minimized when the parent is minimized.  If not
customized, the window is decorated with a slightly simpler title bar.
This is the flag QDialog uses.

Finally, there are some modifier flags:

\value WDestructiveClose  makes Qt delete this object when the object has
accepted closeEvent(), or when the widget tried to ignore closeEvent() but
could not.

\value WPaintDesktop  gives this widget paint events for the desktop.

\value WPaintUnclipped  makes all painters operating on this widget
unclipped.  Children of this widget or other widgets in front of it
do not clip the area the painter can paint on.

\value WPaintClever  indicates that Qt should not try to optimize
repainting for the widget, but instead pass on window system repaint
events directly.  (This tends to produce more events and smaller
repaint regions.)

\value WResizeNoErase  indicates that resizing the widget should not
erase it. This allows smart-repainting to avoid flicker.

\value WMouseNoMask  indicates that even if the widget has a mask,
it wants mouse events for its entire rectangle.

\value WNorthWestGravity  indicates that the widget contents are
north-west aligned and static. On resize, such a widget will receive
paint events only for the newly visible part of itself.

\value WRepaintNoErase  indicates that the widget paints all its
pixels.  Updating, scrolling and focus changes should therefore not
erase the widget.  This allows smart-repainting to avoid flicker.

\value WGroupLeader  makes this widget or window a group
leader. Modality of secondary windows only affects windows within the
same group.

*/


/*!
  Constructs a widget which is a child of \a parent, with the name \a name and
  widget flags set to \a f.

  If \a parent is 0, the new widget becomes a top-level window.  If \a
  parent is another widget, this widget becomes a child window inside
  \a parent.  The new widget is deleted when \a parent is.

  The \a name is sent to the QObject constructor.

  The widget flags argument \a f is normally 0, but it can be set to
  customize the window frame of a top-level widget (i.e. \a parent must be
  zero). To customize the frame, set the \c WStyle_Customize flag OR'ed with
  any of the Qt::WidgetFlags.

  Note that the X11 version of Qt may not be able to deliver all
  combinations of style flags on all systems.  This is because on X11,
  Qt can only ask the window manager, and the window manager can
  override the application's settings.  On Windows, Qt can set
  whatever flags you want.

  Example:
  \code
    QLabel *spashScreen = new QLabel( 0, "mySplashScreen",
				  WStyle_Customize | WStyle_NoBorder |
				  WStyle_Tool );
  \endcode
*/

QWidget::QWidget( QWidget *parent, const char *name, WFlags f )
    : QObject( parent, name ), QPaintDevice( QInternal::Widget )
{
    fstrut_dirty = 1;

    isWidget = TRUE;				// is a widget
    winid = 0;					// default attributes
    widget_state = 0;
    widget_flags = f;
    focus_policy = 0;
    own_font = 0;
    own_palette = 0;
    sizehint_forced = 0;
    is_closing = 0;
    in_show = 0;
#ifndef QT_NO_LAYOUT
    lay_out = 0;
#endif
    extra = 0;					// no extra widget info
#ifndef QT_NO_PALETTE
    bg_col = pal.active().background();		// default background color
#endif
    create();					// platform-dependent init
#ifndef QT_NO_PALETTE
    pal = isTopLevel() ? QApplication::palette() : parentWidget()->palette();
#endif
    fnt = isTopLevel() ? QApplication::font() : parentWidget()->font();

    if ( !isDesktop() )
	setBackgroundFromMode(); //### parts of this are done in create but not all (see reparent(...) )
    // make sure move/resize events are sent to all widgets
    QApplication::postEvent( this, new QMoveEvent( crect.topLeft(),
						   crect.topLeft() ) );
    QApplication::postEvent( this, new QResizeEvent(crect.size(),
						    crect.size()) );
    if ( isTopLevel() ) {
	setWState( WState_ForceHide );
	QFocusData *fd = focusData( TRUE );
	if ( fd->focusWidgets.findRef(this) < 0 )
	    fd->focusWidgets.append( this );
    } else {
	// propagate enabled state
	if ( !parentWidget()->isEnabled() )
	    setWState( WState_Disabled );
	// new widgets do not show up in already visible parents
	if ( parentWidget()->isVisibleTo( 0 ) )
	    setWState( WState_ForceHide );
    }
}

static bool noMoreToplevels();

/*!
  Destroys the widget.

  All children of this widget are deleted first.
  The application exits if this widget is (was) the main widget.
*/

QWidget::~QWidget()
{
#if defined (QT_CHECK_STATE)
    if ( paintingActive() )
	qWarning( "%s (%s): deleted while being painted", className(), name() );
#endif

    // Remove myself and all children from the can-take-focus list
    QFocusData *f = focusData( FALSE );
    if ( f ) {
	QListIterator<QWidget> it(f->focusWidgets);
	QWidget *w;
	while ( (w = it.current()) ) {
	    ++it;
	    QWidget * p = w;
	    while( p && p != this )
		p = p->parentWidget();
	    if ( p ) // my descendant
		f->focusWidgets.removeRef( w );
	}
    }

    if ( QApplication::main_widget == this ) {	// reset main widget
	QApplication::main_widget = 0;
	if (qApp)
	    qApp->quit();
    }

#ifndef QT_NO_STYLE
    extern const QWidget *qt_style_global_context;
    if(qt_style_global_context == this)
	qt_style_global_context = NULL;
#endif

    if ( focusWidget() == this )
	clearFocus();
    if ( QApplication::focus_widget == this )
	QApplication::focus_widget = 0;

    if ( isTopLevel() && !isHidden() && winId() )
	hide();

    // A parent widget must destroy all its children before destroying itself
    if ( childObjects ) {			// delete children objects
	QObjectListIt it(*childObjects);
	QObject *obj;
	while ( (obj=it.current()) ) {
	    ++it;
	    obj->parentObj = 0;
	    // ### nest line is a QGList workaround - remove in 3.0
	    childObjects->removeRef( obj );
	    delete obj;
	}
	delete childObjects;
	childObjects = 0;
    }

    QApplication::removePostedEvents( this );
    if ( extra )
	deleteExtra();

    destroy();					// platform-dependent cleanup
}


/*!
  \internal
  Creates the global widget mapper.
  The widget mapper converts window handles to widget pointers.
  \sa destroyMapper()
*/

void QWidget::createMapper()
{
    mapper = new QWidgetMapper;
    Q_CHECK_PTR( mapper );
}

/*!
  \internal
  Destroys the global widget mapper.
  \sa createMapper()
*/

void QWidget::destroyMapper()
{
    if ( !mapper )				// already gone
	return;
    QWidgetIntDictIt it( *((QWidgetIntDict*)mapper) );
    QWidgetMapper * myMapper = mapper;
    mapper = 0;
    register QWidget *w;
    while ( (w=it.current()) ) {		// remove parents widgets
	++it;
	if ( !w->parentObj )			// widget is a parent
	    w->destroy( TRUE, TRUE );
    }
    delete myMapper;
}


static QWidgetList *wListInternal( QWidgetMapper *mapper, bool onlyTopLevel )
{
    QWidgetList *list = new QWidgetList;
    Q_CHECK_PTR( list );
    if ( mapper ) {
	QWidget *w;
	QWidgetIntDictIt it( *((QWidgetIntDict*)mapper) );
	while ( (w=it.current()) ) {
	    ++it;
	    if ( !onlyTopLevel || w->isTopLevel() )
		list->append( w );
	}
    }
    return list;
}

/*!
  \internal
  Returns a list of all widgets.
  \sa tlwList(), QApplication::allWidgets()
*/

QWidgetList *QWidget::wList()
{
    return wListInternal( mapper, FALSE );
}

/*!
  \internal
  Returns a list of all top level widgets.
  \sa wList(), QApplication::topLevelWidgets()
*/

QWidgetList *QWidget::tlwList()
{
    return wListInternal( mapper, TRUE );
}


void QWidget::setWinId( WId id )		// set widget identifier
{
    if ( !mapper )				// mapper destroyed
	return;
    if ( winid )
	mapper->remove( winid );
    winid = id;
#if defined(Q_WS_X11)
    hd = id;					// X11: hd == ident
#endif
    if ( id )
	mapper->insert( this );
}


/*!
  \internal
  Returns a pointer to the block of extra widget data.
*/

QWExtra *QWidget::extraData()
{
    return extra;
}


/*!
  \internal
  Returns a pointer to the block of extra top level widget data.

  This data is guaranteed to exist for top level widgets.
*/

QTLWExtra *QWidget::topData()
{
    createTLExtra();
    return extra->topextra;
}


void QWidget::createTLExtra()
{
    if ( !extra )
	createExtra();
    if ( !extra->topextra ) {
	QTLWExtra* x = extra->topextra = new QTLWExtra;
#ifndef QT_NO_WIDGET_TOPEXTRA
	x->icon = 0;
#endif
	x->focusData = 0;
	x->fleft = x->fright = x->ftop = x->fbottom = 0;
	x->incw = x->inch = 0;
	x->basew = x->baseh = 0;
	x->iconic = 0;
	x->fullscreen = 0;
	x->showMode = 0;
	x->normalGeometry = QRect(0,0,-1,-1);
#if defined(Q_WS_X11)
	x->embedded = 0;
	x->parentWinId = 0;
	x->dnd = 0;
	x->uspos = 0;
	x->ussize = 0;
#endif
#if defined(Q_WS_QWS) && !defined(QT_NO_QWS_MANAGER)
	x->decor_allocated_region = QRegion();
	x->qwsManager = 0;
#endif
	createTLSysExtra();
    }
}

/*!
  \internal
  Creates the widget extra data.
*/

void QWidget::createExtra()
{
    if ( !extra ) {				// if not exists
	extra = new QWExtra;
	Q_CHECK_PTR( extra );
	extra->minw = extra->minh = 0;
	extra->maxw = extra->maxh = QWIDGETSIZE_MAX;
	extra->bg_pix = 0;
	extra->focus_proxy = 0;
#ifndef QT_NO_CURSOR
	extra->curs = 0;
#endif
	extra->topextra = 0;
	extra->bg_mode = PaletteBackground;
	extra->bg_origin = WidgetOrigin;
#ifndef QT_NO_STYLE
	extra->style = 0;
#endif
	extra->size_policy = QSizePolicy( QSizePolicy::Preferred,
					  QSizePolicy::Preferred );
	createSysExtra();
    }
}


/*!
  \internal
  Deletes the widget extra data.
*/

void QWidget::deleteExtra()
{
    if ( extra ) {				// if exists
	delete extra->bg_pix;
#ifndef QT_NO_CURSOR
	delete extra->curs;
#endif
	deleteSysExtra();
	if ( extra->topextra ) {
	    deleteTLSysExtra();
#ifndef QT_NO_WIDGET_TOPEXTRA
	    delete extra->topextra->icon;
#endif
	    delete extra->topextra->focusData;
#if defined(Q_WS_QWS) && !defined(QT_NO_QWS_MANAGER)
	    delete extra->topextra->qwsManager;
#endif
	    delete extra->topextra;
	}
#if defined(DEUBG)
#endif
	delete extra;
	// extra->xic destroyed in QWidget::destroy()
	extra = 0;
    }
}


/*!
  \internal
  This function is called when a widget is hidden or destroyed.
  It resets some application global pointers that should only refer active,
  visible widgets.
*/

void QWidget::deactivateWidgetCleanup()
{
    extern QWidget *qt_button_down;
    // If this was the active application window, reset it
    if ( this == QApplication::active_window )
	qApp->setActiveWindow( 0 );
    // If the is the active mouse press widget, reset it
    if ( qt_button_down == this )
	qt_button_down = 0;
}


/*!
  Returns a pointer to the widget with window identifer/handle \a id.

  The window identifier type depends by the underlying window system,
  see qwindowdefs.h for the actual definition.
  If there is no widget with this identifier, a null pointer is returned.

  \sa wmapper(), id()
*/

QWidget *QWidget::find( WId id )
{
    return mapper ? mapper->find( id ) : 0;
}

/*!
  \fn QWidgetMapper *QWidget::wmapper()
  \internal
  Returns a pointer to the widget mapper.

  The widget mapper is an internal dictionary that is used to map from
  window identifiers/handles to widget pointers.
  \sa find(), id()
*/

/*!
  \fn WFlags QWidget::getWFlags() const

  Returns the widget flags for this this widget.

  Widget flags are a combination of Qt::WidgetFlags.

  \sa testWFlags(), setWFlags(), clearWFlags()
*/

/*!
  \fn void QWidget::setWFlags( WFlags f )

  Sets the widget flags \a f.

  Widget flags are a combination of Qt::WidgetFlags.

  \sa testWFlags(), getWFlags(), clearWFlags()
*/

/*!
  \fn void QWidget::clearWFlags( WFlags f )

  Clears the widget flags \a f.

  Widget flags are a combination of Qt::WidgetFlags.

  \sa testWFlags(), getWFlags(), setWFlags()
*/



/*!
  \fn WId QWidget::winId() const

  Returns the window system identifier of the widget.

  Portable in principle, but if you use it you are probably about to do
  something non-portable. Be careful.

  \sa find()
*/

#ifndef QT_NO_STYLE
/*!
  Returns the GUI style for this widget

  \sa QWidget::setStyle(), QApplication::setStyle(),
  QApplication::style()
*/

QStyle& QWidget::style() const
{
    extern const QWidget *qt_style_global_context;
    if ( extra && extra->style ) {
	qt_style_global_context = this;
	return *extra->style;
    }
    QStyle &ret = qApp->style();
    qt_style_global_context = this;
    return ret;
}

/*!
  Sets the widget's GUI style to \a style. Ownership of the style
  object is not transferred.

  If no style is set, the widget uses the application's style
  QApplication::style() instead.

  Setting a widget's style has no effect on existing or future
  child widgets.

  \warning This function is particularly useful for demonstration
  purposes, where you want to show Qt's styling capabilities.  Real
  applications should stay away from it and use one consistent GUI
  style instead.

  \sa style(), QStyle, QApplication::style(), QApplication::setStyle()
*/

void QWidget::setStyle( QStyle *style )
{
    QStyle& old  = QWidget::style();
    createExtra();
    extra->style = style;
    if ( !testWFlags(WType_Desktop) // (except desktop)
	 && testWState(WState_Polished)) { // (and have been polished)
	old.unPolish( this );
	QWidget::style().polish( this );
    }
    styleChange( old );
}

/*!
  \overload

  Sets the widget's GUI style to \a style using the QStyleFactory.
*/
QStyle* QWidget::setStyle( const QString &style )
{
    QStyle *s = QStyleFactory::create( style );
    setStyle( s );
    return s;
}

/*!
  This virtual function is called when the style of the widgets.
  changes.\a oldStyle is the
  previous GUI style; you can get the new style from style().

  Reimplement this function if your widget needs to know when its GUI
  style changes.  You will almost certainly need to update the widget
  using update().

  The default implementation updates the widget including its
  geometry.

  \sa QApplication::setStyle(), style(), update(), updateGeometry()
*/

void QWidget::styleChange( QStyle& /* oldStyle */ )
{
    update();
    updateGeometry();
}

#endif

/*! \property QWidget::isTopLevel
    \brief whether the widget is a top-level widget

  A top-level widget is a widget which usually has a frame and a \l
  caption (title). \link isPopup() Popup\endlink and \link
  isDesktop() desktop\endlink widgets are also top-level widgets.

  A top-level widget can have a \link parentWidget() parent
  widget\endlink. It will then be grouped with its parent: deleted
  when the parent is deleted, minimized when the parent is minimized
  etc. If supported by the window manager, it will also have a common
  taskbar entry with its parent.

  QDialog and QMainWindow widgets are by default top-level, even if a
  parent widget is specified in the constructor. This behavior is
  specified by the \c WType_TopLevel widget flag.

  Child widgets are the opposite of top-level widgets.

  \sa topLevelWidget(), isDialog(), isModal(), isPopup(), isDesktop(), parentWidget()
*/

/*! \property QWidget::isDialog
    \brief whether the widget is a dialog widget

  A dialog widget is a secondary top-level widget.

  \sa isTopLevel(), QDialog
*/

/*! \property QWidget::isPopup
    \brief whether the widget is a popup widget

  A popup widget is created by specifying the widget flag \c
  WType_Popup to the widget constructor. A popup widget is also a
  top-level widget.

  \sa isTopLevel()
*/

/*! \property QWidget::isDesktop
    \brief whether the widget is a desktop widget

  A desktop widget is also a top-level widget.

  \sa isTopLevel(), QApplication::desktop()
*/

/*! \property QWidget::isModal
    \brief whether the widget is a modal widget

  This property only makes sense for top-level widgets. A modal
  widget prevents widgets in all other top-level widget from getting
  any input.

  \sa isTopLevel(), isDialog(), QDialog
*/

/*!
  Returns TRUE if this widget would become enabled if \a ancestor is
  enabled.

  This is the case if neither the widget itself nor every parent up to
  but excluding \a ancestor has been explicitly disabled.

  isEnabledTo(0) is equivalent to isEnabled().

  \sa setEnabled() isEnabled()
*/

bool QWidget::isEnabledTo( QWidget* ancestor ) const
{
    const QWidget * w = this;
    while ( w && !w->testWFlags(WState_ForceDisabled)
	    && !w->isTopLevel()
	    && w->parentWidget()
	    && w->parentWidget() != ancestor )
	w = w->parentWidget();
    return !w->testWFlags( WState_ForceDisabled );
}


/*!
  \fn bool QWidget::isEnabledToTLW() const
  \obsolete

  This function is deprecated. It is equivalent to isEnabled()
*/

/*! \property QWidget::enabled
    \brief whether the widget is enabled

  An enabled widget receives keyboard and mouse events; a disabled
  widget does not. Incidentally, an enabled widget receives keyboard
  events only when it is in focus.

  Some widgets display themselves differently when they are disabled.
  For example a button might draw its label grayed out. If your widget
  needs to know when it becomes enabled or disabled, you can
  reimplement the enabledChange() function.

  Disabling a widget implicitly disables all its children. Enabling
  respectively enables all child widgets unless they have been
  explicitly disabled.

  \sa isEnabled(), isEnabledTo(), QKeyEvent, QMouseEvent, enabledChange()
*/
void QWidget::setEnabled( bool enable )
{
    if ( enable )
	clearWState( WState_ForceDisabled );
    else
	setWState( WState_ForceDisabled );

    if ( !isTopLevel() && parentWidget() &&
	 !parentWidget()->isEnabled() && enable )
	return; // nothing we can do

    if ( enable ) {
	if ( testWState(WState_Disabled) ) {
	    clearWState( WState_Disabled );
	    setBackgroundFromMode();
	    enabledChange( !enable );
	    if ( children() ) {
		QObjectListIt it( *children() );
		QWidget *w;
		while( (w = (QWidget *)it.current()) != 0 ) {
		    ++it;
		    if ( w->isWidgetType() &&
			 !w->testWState( WState_ForceDisabled ) )
			w->setEnabled( TRUE );
		}
	    }
	}
    } else {
	if ( !testWState(WState_Disabled) ) {
	    if ( focusWidget() == this )
		focusNextPrevChild( TRUE );
	    setWState( WState_Disabled );
	    setBackgroundFromMode();
	    enabledChange( !enable );
	    if ( children() ) {
		QObjectListIt it( *children() );
		QWidget *w;
		while( (w = (QWidget *)it.current()) != 0 ) {
		    ++it;
		    if ( w->isWidgetType() && w->isEnabled() ) {
			w->setEnabled( FALSE );
			w->clearWState( WState_ForceDisabled );
		    }
		}
	    }
	}
    }
}

/*!
  Disables widget input events if \a disable is TRUE, otherwise enables
  input events.

  See the \l enabled documentation for more information.

  \sa isEnabledTo(), QKeyEvent, QMouseEvent, enabledChange()
*/
void QWidget::setDisabled( bool disable )
{
    setEnabled( !disable );
}

/*!
  \fn void QWidget::enabledChange( bool oldEnabled )

  This virtual function is called from setEnabled(). \a oldEnabled is the
  previous setting; you can get the new setting from isEnabled().

  Reimplement this function if your widget needs to know when it becomes
  enabled or disabled. You will almost certainly need to update the widget
  using update().

  The default implementation repaints the visible part of the widget.

  \sa setEnabled(), isEnabled(), repaint(), update(), visibleRect()
*/

void QWidget::enabledChange( bool )
{
    update();
#if defined(QT_ACCESSIBILITY_SUPPORT)
    emit accessibilityChanged( QAccessible::StateChanged );
#endif
}

/*!
  \fn void QWidget::windowActivationChange( bool oldActive )

  This virtual function is called for a widget when its window is activated or
  deactivated by the windows system. \a oldActive is the previous state; you can
  get the new setting from isActiveWindow().

  Reimplement this function if your widget needs to know when its window becomes
  activated or deactivated.

  The default implementation updates the visible part of the widget if the inactive
  and the active colorgroup are different for colors other than the highlight and link
  colors.

  \sa setActiveWindow(), isActiveWindow(), update(), palette()
*/

void QWidget::windowActivationChange( bool )
{
#ifndef QT_NO_PALETTE
    if ( !isVisible() )
	return;

    const QColorGroup acg = palette().active();
    const QColorGroup icg = palette().inactive();

    if ( acg != icg &&
       ( acg.background() != icg.background() ||
	 acg.base() != icg.base() ||
	 acg.text() != icg.text() ||
	 acg.foreground() != icg.foreground() ||
	 acg.button() != icg.button() ||
	 acg.buttonText() != icg.buttonText() ||
	 acg.brightText() != icg.brightText() ||
	 acg.dark() != icg.dark() ||
	 acg.light() != icg.light() ||
	 acg.mid() != icg.mid() ||
	 acg.midlight() != icg.midlight() ||
	 acg.shadow() != icg.shadow() ) )
	update();
#endif
}

/*! \property QWidget::frameGeometry
    \brief geometry of the widget relative to its parent including
    any window frame

  See the \link geometry.html Window Geometry documentation\endlink
  for an overview of geometry issues with top-level widgets.

  \sa geometry() x() y() pos()
*/
QRect QWidget::frameGeometry() const
{
    if (isTopLevel() && ! isPopup()) {
	if (fstrut_dirty)
	    updateFrameStrut();
	QWidget *that = (QWidget *) this;
	QTLWExtra *top = that->topData();
	return QRect(crect.x() - top->fleft,
		     crect.y() - top->ftop,
		     crect.width() + top->fleft + top->fright,
		     crect.height() + top->ftop + top->fbottom);
    }
    return crect;
}

/*! \property QWidget::x
    \brief the x coordinate of the widget relative to its parent including
    any window frame

  See the \link geometry.html Window Geometry documentation\endlink
  for an overview of geometry issues with top-level widgets.

  \sa frameGeometry, y, pos
*/
int QWidget::x() const
{
    if (isTopLevel() && ! isPopup()) {
	if (fstrut_dirty)
	    updateFrameStrut();
	QWidget *that = (QWidget *) this;
	return crect.x() - that->topData()->fleft;
    }
    return crect.x();
}

/*! \property QWidget::y
    \brief the y coordinate of the widget relative to its parent and
    including any window frame

  See the \link geometry.html Window Geometry documentation\endlink
  for an overview of geometry issues with top-level widgets.

  \sa frameGeometry, x, pos
*/
int QWidget::y() const
{
    if (isTopLevel() && ! isPopup()) {
	if (fstrut_dirty)
	    updateFrameStrut();
	QWidget *that = (QWidget *) this;
	return crect.y() - that->topData()->ftop;
    }
    return crect.y();
}

/*! \property QWidget::pos
    \brief the position of the widget in its parent widget

  If the widget is a top-level widget, the position is that of the
  widget on the desktop, including the frame.

  When changing the position, the widget, if visible, receives a move
  event (moveEvent()) immediately. If the widget is not visible yet,
  it is guaranteed to receive an event before it is shown.

  move() is virtual, and all other overloaded move() implementations
  in Qt call it.

  \warning If you call move() or setGeometry() from moveEvent(), you
  may see infinite recursion.

  See the \link geometry.html Window Geometry documentation\endlink
  for an overview of geometry issues with top-level widgets.

  See the \link geometry.html Window Geometry documentation\endlink
  for an overview of geometry issues with top-level widgets.

  \sa frameGeometry, size x(), y()
*/
QPoint QWidget::pos() const
{
    if (isTopLevel() && ! isPopup()) {
	if (fstrut_dirty)
	    updateFrameStrut();
	QWidget *that = (QWidget *) this;
	QTLWExtra *top = that->topData();
	return QPoint(crect.x() - top->fleft, crect.y() - top->ftop);
    }
    return crect.topLeft();
}

/*! \property QWidget::geometry
    \brief the geometry of the widget relative to its parent and
    excluding the window frame

  When changing the geometry, the widget, if visible, receives a move
  event (moveEvent()) and/or a resize event (resizeEvent())
  immediately. If the widget is not visible yet, it is guaranteed to
  receive appropriate events before it is shown.

  The size component is adjusted if it lies outside the range defined
  by minimumSize() and maximumSize().

  setGeometry() is virtual, and all other overloaded setGeometry()
  implementations in Qt call it.

  \warning If you call setGeometry() from resizeEvent() or
  moveEvent(), you may see infinite recursion.

  See the \link geometry.html Window Geometry documentation\endlink
  for an overview of geometry issues with top-level widgets.

  \sa frameGeometry(), rect(), move(), resize(), moveEvent(),
      resizeEvent(), minimumSize(), maximumSize()
*/

/*! \property QWidget::size
    \brief the size of the widget excluding any window frame

  When resizing, the widget, if visible, receives a resize event
  (resizeEvent()) immediately. If the widget is not visible yet, it
  is guaranteed to receive an event before it is shown.

  The size is adjusted if it lies outside the range defined by
  minimumSize() and maximumSize(). Furthermore, the size is always at
  least QSize(1, 1).

  resize() is virtual, and all other overloaded resize()
  implementations in Qt call it.

  \warning If you call resize() or setGeometry() from resizeEvent(),
  you may see infinite recursion.

  \sa pos, geometry, minimumSize, maximumSize, resizeEvent()
*/

/*! \property QWidget::width
    \brief the width of the widget excluding any window frame

  See the \link geometry.html Window Geometry documentation\endlink
  for an overview of geometry issues with top-level widgets.

  \sa geometry, height, size
*/

/*! \property QWidget::height
    \brief the height of the widget excluding any window frame

  See the \link geometry.html Window Geometry documentation\endlink
  for an overview of geometry issues with top-level widgets.

  \sa geometry, width, size
*/

/*! \property QWidget::rect
    \brief the internal geometry of the widget excluding any window
    frame

  The rect property equals QRect(0, 0, width(), height()).

  See the \link geometry.html Window Geometry documentation\endlink
  for an overview of geometry issues with top-level widgets.

  \sa size
*/

/*! \property QWidget::childrenRect
    \brief the bounding rectangle of the widget's children

  Hidden children are excluded.

  \sa childrenRegion() geometry()
*/

QRect QWidget::childrenRect() const
{
    QRect r( 0, 0, 0, 0 );
    if ( !children() )
	return r;
    QObjectListIt it( *children() );
    QObject *obj;
    while ( (obj = it.current()) ) {
	++it;
	if ( obj->isWidgetType() && !((QWidget*)obj)->isHidden() )
	    r = r.unite( ((QWidget*)obj)->geometry() );
    }
    return r;
}

/*! \property QWidget::childrenRegion
    \brief the combined region occupied by the widget's children

  Hidden children are excluded.

  \sa childrenRect() geometry()
*/

QRegion QWidget::childrenRegion() const
{
    QRegion r;
    if ( !children() )
	return r;
    QObjectListIt it( *children() );		// iterate over all children
    QObject *obj;
    while ( (obj=it.current()) ) {
	++it;
	if ( obj->isWidgetType() && !((QWidget*)obj)->isHidden() )
	    r = r.unite( ((QWidget*)obj)->geometry() );
    }
    return r;
}


/*! \property QWidget::minimumSize
    \brief the widget's minimum size

  The widget cannot be resized to a smaller size than the minimum widget
  size. The widget's size is forced to the minimum size if the current
  size is smaller.

  If you use a layout inside the widget, the minimum size will be set
  by the layout and not by setMinimumSize(), unless you set the
  layouts resize mode to QLayout::FreeResize.

  \sa minimumWidth, minimumHeight, maximumSize, sizeIncrement
      QLayout::setResizeMode()
*/

QSize QWidget::minimumSize() const
{
    return extra ? QSize( extra->minw, extra->minh ) : QSize( 0, 0 );
}

/*! \property QWidget::maximumSize
    \brief the widget's maximum size

  The widget cannot be resized to a larger size than the maximum widget
  size.

  \sa maximumWidth(), maximumHeight(), setMaximumSize(),
  minimumSize(), sizeIncrement()
*/

QSize QWidget::maximumSize() const
{
    return extra ? QSize( extra->maxw, extra->maxh )
		 : QSize( QWIDGETSIZE_MAX, QWIDGETSIZE_MAX );
}


/*! \property QWidget::minimumWidth
    \brief the widget's minimum width

  This property corresponds to minimumSize().width().

  \sa minimumSize, minimumHeight
*/

/*! \property QWidget::minimumHeight
    \brief the widget's minimum height

  This property corresponds to minimumSize().height().

  \sa minimumSize, minimumWidth
*/

/*! \property QWidget::maximumWidth
    \brief the widget's maximum width

  This property corresponds to maximumSize().width().

  \sa maximumSize, maximumHeight
*/

/*! \property QWidget::maximumHeight
    \brief the widget's maximum height

  This property corresponds to maximumSize().height().

  \sa maximumSize, maximumWidth
*/

/*! \property QWidget::sizeIncrement
    \brief the size increment of the widget

  When the user resizes the window, the size will move in steps of
  sizeIncrement().width() pixels horizontally and
  sizeIncrement.height() pixels vertically, with baseSize() as basis.
  Preferred widget sizes are therefore for nonnegative integers \e i
  and \e j:
  \code
    width = baseSize().width() + i * sizeIncrement().width();
    height = baseSize().height() + j * sizeIncrement().height();
  \endcode

  Note that while you can set the size increment for all widgets, it
  has no effect except for top-level widgets.

  \warning The size increment has no effect under Windows, and may be
  disregarded by the window manager on X.

  \sa size, minimumSize, maximumSize
*/
QSize QWidget::sizeIncrement() const
{
    return ( extra && extra->topextra )
	? QSize( extra->topextra->incw, extra->topextra->inch )
	: QSize( 0, 0 );
}

/*! \property QWidget::baseSize
    \brief the base size of the widget

  The base size is used to calculate a proper widget size in case the
  widget defines sizeIncrement().

  \sa setSizeIncrement()
*/

QSize QWidget::baseSize() const
{
    return ( extra != 0 && extra->topextra != 0 )
	? QSize( extra->topextra->basew, extra->topextra->baseh )
	: QSize( 0, 0 );
}

/*!
  Sets both the minimum and maximum sizes of the widget to \a s,
  thereby preventing it from ever growing or shrinking.

  \sa setMaximumSize() setMinimumSize()
*/

void QWidget::setFixedSize( const QSize & s)
{
    setMinimumSize( s );
    setMaximumSize( s );
    resize( s );
}


/*!
  \overload void QWidget::setFixedSize( int w, int h )
*/

void QWidget::setFixedSize( int w, int h )
{
    setMinimumSize( w, h );
    setMaximumSize( w, h );
    resize( w, h );
}

void QWidget::setMinimumWidth( int w )
{
    setMinimumSize( w, minimumSize().height() );
}

void QWidget::setMinimumHeight( int h )
{
    setMinimumSize( minimumSize().width(), h );
}

void QWidget::setMaximumWidth( int w )
{
    setMaximumSize( w, maximumSize().height() );
}

void QWidget::setMaximumHeight( int h )
{
    setMaximumSize( maximumSize().width(), h );
}

/*!
  Sets both the minimum and maximum width of the widget to \a w
  without changing the heights.  Provided for convenience.

  \sa sizeHint() minimumSize() maximumSize() setFixedSize()
*/

void QWidget::setFixedWidth( int w )
{
    setMinimumSize( w, minimumSize().height() );
    setMaximumSize( w, maximumSize().height() );
}


/*!
  Sets both the minimum and maximum heights of the widget to \a h
  without changing the widths.  Provided for convenience.

  \sa sizeHint() minimumSize() maximumSize() setFixedSize() and more
*/

void QWidget::setFixedHeight( int h )
{
    setMinimumSize( minimumSize().width(), h );
    setMaximumSize( maximumSize().width(), h );
}


/*! Translates the widget coordinate \a pos to the coordinate system
  of \a parent, which must be non-null and be a parent widget of this.

  \sa mapFrom() mapToParent() mapToGlobal()
*/

QPoint QWidget::mapTo( QWidget * parent, const QPoint & pos ) const
{
    QPoint p = pos;
    if ( parent ) {
	const QWidget * w = this;
	while ( w != parent ) {
	    p = w->mapToParent( p );
	    w = w->parentWidget();
	}
    }
    return p;
}


/*! Translates the widget coordinate \a pos from the coordinate system
  of \a parent to this widget's coordinate system, which must be non-null
  and be a parent widget of this.

  \sa mapTo() mapFromParent() mapFromGlobal()
*/

QPoint QWidget::mapFrom( QWidget * parent, const QPoint & pos ) const
{
    QPoint p( pos );
    if ( parent ) {
	const QWidget * w = this;
	while ( w != parent ) {
	    p = w->mapFromParent( p );
	    w = w->parentWidget();
	}
    }
    return p;
}


/*!
  Translates the widget coordinate \a pos to a coordinate in the parent widget.

  Same as mapToGlobal() if the widget has no parent.

  \sa mapFromParent() mapTo() mapToGlobal()
*/

QPoint QWidget::mapToParent( const QPoint &pos ) const
{
    return pos + crect.topLeft();
}

/*!
  Translates the parent widget coordinate \a pos to widget coordinates.

  Same as mapFromGlobal() if the widget has no parent.

  \sa mapToParent() mapFrom() mapFromGlobal()
*/

QPoint QWidget::mapFromParent( const QPoint &pos ) const
{
    return pos - crect.topLeft();
}


/*!

  Returns the top-level widget for this widget, i.e. the next ancestor
  widget that has a window-system frame (or at least may have one).

  If the widget is a top-level, the widget itself is returned.

  Typical usage is changing the window caption:

  \code
    aWidget->topLevelWidget()->setCaption( "New Caption" );
  \endcode

  \sa isTopLevel()
*/

QWidget *QWidget::topLevelWidget() const
{
    QWidget *w = (QWidget *)this;
    QWidget *p = w->parentWidget();
    while ( !w->testWFlags(WType_TopLevel) && p ) {
	w = p;
	p = p->parentWidget();
    }
    return w;
}

void QWidget::setBackgroundColorForMode( BackgroundMode mode, const QColor &color )
{
#ifndef QT_NO_PALETTE
    switch( mode ) {
    case FixedColor:
    case FixedPixmap :
    case NoBackground:
    case X11ParentRelative:
	setEraseColor( color );
	break;
    default:
	QPalette pal = palette();
	pal.setBackgroundColorForMode( QPalette::Active, mode, color );
	pal.setBackgroundColorForMode( QPalette::Inactive, mode, color );
	pal.setBackgroundColorForMode( QPalette::Disabled, mode, color );
	setPalette( pal );
	break;
    }
#else
    setEraseColor( color );
#endif
}

/*! \property QWidget::foregroundColor
    \brief the foreground color of the widget

  setForegroundColor() is a convenience function that creates and
  sets a modified QPalette with setPalette(). The palette is modified
  according to the widget's \e {background mode}. For example, if the
  background mode is PaletteButton the palette entry
  QColorGroup::ButtonText is set to color.

  \sa setPalette() QApplication:setPalette() backgroundMode()
      foregroundColor() setBackgroundMode() setEraseColor()
*/
const QColor &QWidget::foregroundColor() const
{
#ifndef QT_NO_PALETTE
    return foregroundColorForMode( backgroundMode() );
#else
    return black; //###
#endif
}

void QWidget::setForegroundColor( const QColor & color )
{
    setForegroundColorForMode( backgroundMode(), color );
}

void QWidget::setForegroundColorForMode( BackgroundMode mode, const QColor & color )
{
#ifndef QT_NO_PALETTE
    QPalette pal = palette();
    pal.setForegroundColorForMode( QPalette::Active, mode, color );
    pal.setForegroundColorForMode( QPalette::Inactive, mode, color );
    pal.setForegroundColorForMode( QPalette::Disabled, mode, color );
    setPalette( pal );
#endif
}

/*! \fn const QColor& QWidget::eraseColor() const

  Returns the erase color of the widget.

  \sa setEraseColor() setErasePixmap() backgroundColor
*/

/*!
  Sets the erase color of the widget to \a color.

  The erase color is the color the widget is to be cleared to before
  paintEvent() is called. If there is an erase pixmap (set using
  setErasePixmap()), then this property has an indeterminate value.

  \sa erasePixmap(), backgroundColor, backgroundMode, palette
*/
void QWidget::setEraseColor( const QColor & color )
{
    setBackgroundModeDirect( FixedColor );
    setBackgroundColorDirect( color );
}

/*! \property QWidget::backgroundPixmap
    \brief the widget's background pixmap

  If the widget has backgroundMode() NoBackground, the
  backgroundPixmap() returns a pixmap for which QPixmap:isNull() is
  true.  If the widget has no pixmap is the background,
  backgroundPixmap() returns a null pointer.

  setBackgroundPixmap() is a convenience function that creates and
  sets a modified QPalette with setPalette(). The palette is modified
  according to the widget's background mode. For example, if the
  background mode is PaletteButton the pixmap used for the palette's
  QColorGroup::Button brush entry is set.

  \sa backgroundPixmapChange(), backgroundColor,
      erasePixmap, palette, QApplication::setPalette()
*/
const QPixmap *QWidget::backgroundPixmap() const
{
    return backgroundPixmapForMode(backgroundMode());
}

void QWidget::setBackgroundPixmap( const QPixmap &pixmap )
{
    setBackgroundPixmapForMode( backgroundMode(), pixmap );
}

void QWidget::setBackgroundPixmapForMode( BackgroundMode mode, const QPixmap &pixmap )
{
#ifndef QT_NO_PALETTE
    switch( mode ) {
    case FixedColor:
    case FixedPixmap :
    case NoBackground:
    case X11ParentRelative:
	setErasePixmap( pixmap );
	break;
    default:
	QPalette pal = palette();
	pal.setBackgroundPixmapForMode( QPalette::Active, mode, pixmap );
	pal.setBackgroundPixmapForMode( QPalette::Inactive, mode, pixmap );
	pal.setBackgroundPixmapForMode( QPalette::Disabled, mode, pixmap );
	setPalette( pal );
	break;
    }
#else
    setErasePixmap( pixmap );
#endif
}

/*!
  Returns the widget's erase pixmap.

  \sa setErasePixmap() eraseColor()
*/
const QPixmap *QWidget::erasePixmap() const
{
    return ( extra && extra->bg_pix ) ? extra->bg_pix : 0;
}

/*!
  Sets the widget's erase pixmap to \a pixmap.

  This pixmap is used to clear the widget before paintEvent() is called.
*/
void QWidget::setErasePixmap( const QPixmap &pixmap )
{
    // This function is called with a null pixmap by setBackgroundEmpty().
    setBackgroundPixmapDirect( pixmap );
    setBackgroundModeDirect( FixedPixmap );
}

void QWidget::setBackgroundFromMode()
{
#ifndef QT_NO_PALETTE
    QColorGroup::ColorRole r = QColorGroup::Background;
    if ( extra ) {
	int i = (BackgroundMode)extra->bg_mode;
	if ( i == FixedColor || i == FixedPixmap || i == NoBackground ) {
	    // Mode is for fixed color, not one based on palette,
	    // so nothing to do.
	    return;
	}
	switch( i ) {
	case PaletteForeground:
	    r = QColorGroup::Foreground;
	    break;
	case PaletteButton:
	    r = QColorGroup::Button;
	    break;
	case PaletteLight:
	    r = QColorGroup::Light;
	    break;
	case PaletteMidlight:
	    r = QColorGroup::Midlight;
	    break;
	case PaletteDark:
	    r = QColorGroup::Dark;
	    break;
	case PaletteMid:
	    r = QColorGroup::Mid;
	    break;
	case PaletteText:
	    r = QColorGroup::Text;
	    break;
	case PaletteBrightText:
	    r = QColorGroup::BrightText;
	    break;
	case PaletteBase:
	    r = QColorGroup::Base;
	    break;
	case PaletteBackground:
	    r = QColorGroup::Background;
	    break;
	case PaletteShadow:
	    r = QColorGroup::Shadow;
	    break;
	case PaletteHighlight:
	    r = QColorGroup::Highlight;
	    break;
	case PaletteHighlightedText:
	    r = QColorGroup::HighlightedText;
	    break;
	case PaletteButtonText:
	    r = QColorGroup::ButtonText;
	    break;
	case X11ParentRelative:
#if defined(Q_WS_X11)
	    setBackgroundX11Relative();
#endif
	    return;
	}
    }
    QPixmap * p = palette().active().brush( r ).pixmap();
    if ( p )
	setBackgroundPixmapDirect( *p );
    else
	setBackgroundColorDirect( palette().active().color( r ) );
#endif
}

/*! \enum Qt::BackgroundMode

  This enum describes how the background of a widget changes, as the
  widget's palette changes.

  The background is what the widget contains when paintEvent() is
  called.  To minimize flicker, this should be the most common color
  or pixmap in the widget.  For \c PaletteBackground, use
  colorGroup().brush( \c QColorGroup::Background ), and so on.  There
  are also three special values, listed at the end:

  \value PaletteForeground
  \value PaletteBackground
  \value PaletteButton
  \value PaletteLight
  \value PaletteMidlight
  \value PaletteDark
  \value PaletteMid
  \value PaletteText
  \value PaletteBrightText
  \value PaletteButtonText
  \value PaletteBase
  \value PaletteShadow
  \value PaletteHighlight
  \value PaletteHighlightedText
  \value NoBackground the widget is not cleared before paintEvent().
  If the widget's paint event always draws on all the pixels, using
  this mode can be both fast and flicker-free.
  \value FixedColor the widget is cleared to a fixed color,
  normally different from all the ones in the palette().  Set using
  setBackgroundColor().
  \value FixedPixmap the widget is cleared to a fixed pixmap,
  normally different from all the ones in the palette().  Set using
  setBackgroundPixmap().

  \c FixedColor and \c FixedPixmap sometimes are just the right
  thing, but if you use them, make sure that your application looks
  right when the desktop color scheme has been changed.  (On X11, a
  quick way to test is e.g. "./yourapp -bg paleblue".  On Windows, you
  have to use the control panel.)

  \sa QWidget::setBackgroundMode() QWidget::backgroundMode() QWidget::setBackgroundPixmap()
  QWidget::setBackgroundColor()
*/

/*! \property QWidget::backgroundMode
    \brief the color role used for painting the background of the widget

  setBackgroundColor() reads this property to determine which entry of
  the \l palette to set.

  For most widgets the default suffices (PaletteBackground, typically
  gray), but some need to use PaletteBase (the background color for
  text output, typically white) or another role.

  QListBox, which is "sunken" and uses the base color to contrast with
  its environment, does this in its constructor:

  \code
    setBackgroundMode( PaletteBase );
  \endcode

  You will never need to set the background mode of a built-in widget
  in Qt, but you might consider setting it in your custom widgets, so
  that setBackgroundColor() works as expected.

  Note that two of the BackgroundMode values make no sense for
  setBackgroundMode(), namely FixedPixmap and FixedColor. You have to
  call setBackgroundPixmap() and setBackgroundColor() instead.
*/
Qt::BackgroundMode QWidget::backgroundMode() const
{
    return extra ? (BackgroundMode) extra->bg_mode : PaletteBackground;
}

void QWidget::setBackgroundMode( BackgroundMode m )
{
    if ( m == NoBackground ) {
	setBackgroundEmpty();
    } else if ( m == FixedColor || m == FixedPixmap ) {
#if defined(QT_DEBUG)
	qWarning( "QWidget::setBackgroundMode: FixedColor or FixedPixmap makes"
		  " no sense" );
#endif
	return;
    }
    setBackgroundModeDirect(m);
}

/*!
  \internal
*/
void QWidget::setBackgroundModeDirect( BackgroundMode m )
{
    if ( m == PaletteBackground && !extra )
	return;

    createExtra();
    if ( (BackgroundMode)extra->bg_mode != m ) {
	extra->bg_mode = m;
	setBackgroundFromMode();
    }
}

/*! \property QWidget::backgroundColor
    \brief the background color of the widget

  The background color is usually set implicitly by
  setBackgroundMode(), although it can also be set explicitly by
  setBackgroundColor().

  If there is a background pixmap (set using setBackgroundPixmap()),
  then the return value of this function is indeterminate.

  \sa foregroundColor, palette, colorGroup()
*/
const QColor & QWidget::backgroundColor() const
{
    return backgroundColorForMode( backgroundMode() );
}

void QWidget::setBackgroundColor( const QColor &color )
{
    setBackgroundColorForMode( backgroundMode(), color );
}

const QColor & QWidget::backgroundColorForMode( BackgroundMode mode ) const
{
#ifndef QT_NO_PALETTE
    switch( mode ) {
    case FixedColor:
    case FixedPixmap :
    case NoBackground:
    case X11ParentRelative:
	return eraseColor();
    default:
	QPalette pal = palette();
	return  pal.backgroundColorForMode( QPalette::Normal, mode );
    }
#else
    return eraseColor();
#endif
}

const QColor &QWidget::foregroundColorForMode( BackgroundMode mode ) const
{
#ifndef QT_NO_PALETTE
    QPalette pal = palette();
    return pal.foregroundColorForMode( QPalette::Normal, mode );
#else
    return Qt::black; //###
#endif
}


/*!
  \fn void QWidget::backgroundColorChange( const QColor &oldBackgroundColor )

  This virtual function is called from setBackgroundColor().
  \e oldBackgroundColor is the previous background color; you can get the new
  background color from backgroundColor().

  Reimplement this function if your widget needs to know when its
  background color changes.  You will almost certainly need to call
  this implementation of the function.

  \sa setBackgroundColor(), backgroundColor(), setPalette(), repaint(),
  update()
*/

void QWidget::backgroundColorChange( const QColor & )
{
    update();
}

const QPixmap *QWidget::backgroundPixmapForMode( BackgroundMode mode ) const
{
#ifndef QT_NO_PALETTE
    switch( mode ) {
    case FixedColor:
    case FixedPixmap :
    case NoBackground:
    case X11ParentRelative:
	return erasePixmap();
    default:
	QPalette pal = palette();
	return pal.backgroundPixmapForMode( QPalette::Normal, mode );
    }
#else
    return erasePixmap();
#endif
}

/*!
  \fn void QWidget::backgroundPixmapChange( const QPixmap & oldBackgroundPixmap )

  This virtual function is called from setBackgroundPixmap().
  \e oldBackgroundPixmap is the previous background pixmap; you can get the
  new background pixmap from backgroundPixmap().

  Reimplement this function if your widget needs to know when its
  background pixmap changes.  You will almost certainly need to call
  this implementation of the function.

  \sa setBackgroundPixmap(), backgroundPixmap(), repaint(), update()
*/

void QWidget::backgroundPixmapChange( const QPixmap & )
{
    update();
}


/*! \property QWidget::colorGroup
    \brief the current color group of the widget palette

  The color group is determined by the state of the widget. A
  disabled widget has the QPalette::disabled() color group, a widget
  with keyboard focus has the QPalette::active() color group, and an
  inactive widget has the QPalette::inactive() color group.

  \sa palette(), setPalette()
*/
#ifndef QT_NO_PALETTE
const QColorGroup &QWidget::colorGroup() const
{
    if ( !isEnabled() )
	return palette().disabled();
    else if ( isActiveWindow() )
	return palette().active();
    else
	return palette().inactive();
}
#endif

/*! \property QWidget::palette
    \brief the widget's palette

  As long as no special palette has been set, or after unsetPalette()
  has been called, this is either a special palette for the widget
  class, the parent's palette or (if this widget is a top level
  widget) the default application palette.

  \sa ownPalette, colorGroup(), QApplication::palette()
*/

#ifndef QT_NO_PALETTE
void QWidget::setPalette( const QPalette &palette )
{
    own_palette = TRUE;
    if ( pal == palette )
	return;
    QPalette old = pal;
    pal = palette;
    setBackgroundFromMode();
    paletteChange( old );
    QEvent ev( QEvent::PaletteChange );
    QApplication::sendEvent( this, &ev );
    if ( children() ) {
	QEvent e( QEvent::ParentPaletteChange );
	QObjectListIt it( *children() );
	QWidget *w;
	while( (w=(QWidget *)it.current()) != 0 ) {
	    ++it;
	    if ( w->isWidgetType() )
		QApplication::sendEvent( w, &e );
	}
    }
    update();
}

void QWidget::unsetPalette()
{
    if ( own_palette ) {
	if ( !isTopLevel() && QApplication::palette( this ).isCopyOf( QApplication::palette() ) )
	    setPalette( parentWidget()->palette() );
	else
	    setPalette( QApplication::palette( this ) );
	own_palette = FALSE;
    }
}

/*!
  \fn void QWidget::setPalette( const QPalette&, bool )
  \obsolete

  Use setPalette( const QPalette& p ) instead.
*/

/*!
  \fn void QWidget::paletteChange( const QPalette &oldPalette )

  This virtual function is called from setPalette().  \e oldPalette is the
  previous palette; you can get the new palette from palette().

  Reimplement this function if your widget needs to know when its
  palette changes.  You will almost certainly need to call this
  implementation of the function.

  \sa setPalette(), palette()
*/

void QWidget::paletteChange( const QPalette & )
{
}
#endif // QT_NO_PALETTE

/*! \property QWidget::font
    \brief the font currently set for the widget

  The fontInfo() function reports the actual font that is being used by the
  widget.

  As long as no special font has been set, or after unsetFont() is
  called, this is either a special font for the widget class, the
  parent's font or (if this widget is a top level widget) the default
  application font.

  This code fragment sets a 12 point helvetica bold font:
  \code
    QFont f( "Helvetica", 12, QFont::Bold );
    setFont( f );
  \endcode

  Apart from setting the font, setFont() informs all children about
  the change.

  \sa fontChange() fontInfo() fontMetrics() ownFont()
*/
void QWidget::setFont( const QFont &font )
{
    own_font = TRUE;
    if ( fnt == font )
	return;
    QFont old = fnt;
    fnt = font;
    fnt.handle(); // force load font
    fontChange( old );
    if ( children() ) {
	QEvent e( QEvent::ParentFontChange );
	QObjectListIt it( *children() );
	QWidget *w;
	while( (w=(QWidget *)it.current()) != 0 ) {
	    ++it;
	    if ( w->isWidgetType() )
		QApplication::sendEvent( w, &e );
	}
    }
    if ( hasFocus() )
	setFontSys();
}

void QWidget::unsetFont()
{
    if ( own_font ) {
	if ( !isTopLevel() && QApplication::font( this ).isCopyOf( QApplication::font() ) )
	    setFont( parentWidget()->font() );
	else
	    setFont( QApplication::font( this ) );
	own_font = FALSE;
    }
}

/*!
  \fn void QWidget::setFont( const QFont&, bool )
  \obsolete

  Use setFont(const QFont& font) instead.
*/

/*!
  \fn void QWidget::fontChange( const QFont &oldFont )

  This virtual function is called from setFont().  \e oldFont is the
  previous font; you can get the new font from font().

  Reimplement this function if your widget needs to know when its font
  changes.  You will almost certainly need to update the widget using
  update().

  The default implementation updates the widget including its
  geometry.

  \sa setFont(), font(), update(), updateGeometry()
*/

void QWidget::fontChange( const QFont & )
{
    update();
    updateGeometry();
}


/*!
  \fn QFontMetrics QWidget::fontMetrics() const

  Returns the font metrics for the widget's current font.
  Equivalent to QFontMetrics(widget->font()).

  \sa font(), fontInfo(), setFont()
*/

/*!
  \fn QFontInfo QWidget::fontInfo() const

  Returns the font info for the widget's current font.
  Equivalent to QFontInto(widget->font()).

  \sa font(), fontMetrics(), setFont()
*/


/*! \property QWidget::cursor
    \brief the cursor shape for this widget

  The mouse cursor will assume this shape when it's over this widget.
  See a list of predefined cursor objects with a range of useful
  shapes in the QCursor documentation.

  An editor widget would for example use an I-beam cursor:
  \code
    setCursor( ibeamCursor );
  \endcode

  If no cursor has been set, or after a call to unsetCursor(), the
  parent's cursor is used. The function unsetCursor() has no effect
  on top-level widgets.

  \sa QApplication::setOverrideCursor()
*/

#ifndef QT_NO_CURSOR
const QCursor &QWidget::cursor() const
{
    if ( testWState(WState_OwnCursor) )
	return (extra && extra->curs)
	    ? *extra->curs
	    : arrowCursor;
    else
	return isTopLevel() ? arrowCursor : parentWidget()->cursor();
}
#endif
#ifndef QT_NO_WIDGET_TOPEXTRA
/*! \property QWidget::caption
    \brief the window caption (title)

  This property only makes sense for top-level widgets. If no caption
  has been set, the caption is QString::null.

  \sa icon() iconText()
*/
QString QWidget::caption() const
{
    return extra && extra->topextra
	? extra->topextra->caption
	: QString::null;
}

/*! \property QWidget::icon
    \brief the widget icon pixmap

  This property makes sense only for top-level widgets. If no icon
  has been set, icon() returns a null pointer.

  \sa iconText, caption,
      \link appicon.html Setting the Application Icon\endlink
*/
const QPixmap *QWidget::icon() const
{
    return ( extra && extra->topextra ) ? extra->topextra->icon : 0;
}

/*! \property QWidget::iconText
    \brief the widget icon text

  This property makes sense only for top-level widgets. If no icon
  text has been set, this functions returns QString::null.

  \sa icon, caption
*/

QString QWidget::iconText() const
{
    return ( extra && extra->topextra ) ? extra->topextra->iconText
	: QString::null;
}
#endif //QT_NO_WIDGET_TOPEXTRA

/*! \property QWidget::mouseTracking
    \brief whether mouse tracking is enabled for this widget

  If mouse tracking is disabled (the default), this widget only
  receives mouse move events when at least one mouse button is
  pressed down while the mouse is being moved.

  If mouse tracking is enabled, this widget receives mouse move
  events even if no buttons are pressed down.

  \sa mouseMoveEvent(), QApplication::setGlobalMouseTracking()
*/
#if !defined(Q_WS_X11)
void QWidget::setMouseTracking( bool enable )
{
    if ( enable )
	setWState( WState_MouseTracking );
    else
	clearWState( WState_MouseTracking );
    return;
}
#endif // Q_WS_X11

/*!  Sets this widget's focus proxy to \a w. If \a w is 0, this
  function resets this widget to not have any focus proxy.

  Some widgets, such as QComboBox, can "have focus," but create a
  child widget to actually handle the focus.  QComboBox, for example,
  creates a QLineEdit.

  setFocusProxy() sets the widget which will actually get focus when
  "this widget" gets it.  If there is a focus proxy, focusPolicy(),
  setFocusPolicy(), setFocus() and hasFocus() all operate on the focus
  proxy.

  \sa focusProxy()
*/

void QWidget::setFocusProxy( QWidget * w )
{
    if ( !w && !extra )
	return;

    createExtra();

    if ( extra->focus_proxy ) {
	disconnect( extra->focus_proxy, SIGNAL(destroyed()),
		    this, SLOT(focusProxyDestroyed()) );
	extra->focus_proxy = 0;
    }

    if ( w ) {
	setFocusPolicy( w->focusPolicy() );
	connect( w, SIGNAL(destroyed()),
		 this, SLOT(focusProxyDestroyed()) );
    }
    extra->focus_proxy = w;
}


/*!  Returns a pointer to the focus proxy, or 0 if there is no focus
  proxy.

  \sa setFocusProxy()
*/

QWidget * QWidget::focusProxy() const
{
    return extra ? extra->focus_proxy : 0;
}


/*!  Internal slot used to clean up if the focus proxy is destroyed.
  \sa setFocusProxy()
*/

void QWidget::focusProxyDestroyed()
{
    if ( extra )
	extra->focus_proxy = 0;
    setFocusPolicy( NoFocus );
}

/*! \property QWidget::focus
    \brief whether this widget (or its focus proxy) has the keyboard
    input focus

  Equivalent to \c {qApp->focusWidget() == this}.

  \sa setFocus(), clearFocus(), setFocusPolicy(), QApplication::focusWidget()
*/
bool QWidget::hasFocus() const
{
    const QWidget* w = this;
    while ( w->focusProxy() )
	w = w->focusProxy();
    return qApp->focusWidget() == w;
}

/*!  Gives the keyboard input focus to this widget (or its focus
  proxy).

  First, a focus out event is sent to the focus widget (if any) to
  tell it that it is about to lose the focus. Then a focus in event is
  sent to this widget to tell it that it just received the focus. (Of
  course, if the two are the same nothing happens.

  setFocus() gives focus to a widget regardless of its focus policy,
  but does not clear any keyboard grab (see grabKeyboard()).

  \warning If you call setFocus() in a function which may itself be
  called from focusOutEvent() or focusInEvent(), you may see infinite
  recursion.

  \sa hasFocus() clearFocus() focusInEvent() focusOutEvent()
  setFocusPolicy() QApplication::focusWidget() grabKeyboard()
  grabMouse()
*/

void QWidget::setFocus()
{
    if ( !isEnabled() )
	return;

    if ( focusProxy() ) {
	focusProxy()->setFocus();
	return;
    }

    QFocusData * f = focusData(TRUE);
    if ( f->it.current() == this && qApp->focusWidget() == this )
	return;

    f->it.toFirst();
    while ( f->it.current() != this && !f->it.atLast() )
	++f->it;
    // at this point, the iterator should point to 'this'.  if it
    // does not, 'this' must not be in the list - an error, but
    // perhaps possible.  fix it.
    if ( f->it.current() != this ) {
	f->focusWidgets.append( this );
	f->it.toLast();
    }

    if ( isActiveWindow() ) {
	QWidget * prev = qApp->focus_widget;
	qApp->focus_widget = this;
#if defined(Q_WS_X11)
	focusInputContext();
#endif

	if ( prev != this ) {
	    if ( prev ) {
		QFocusEvent out( QEvent::FocusOut );
		QApplication::sendEvent( prev, &out );
	    }

	    QFocusEvent in( QEvent::FocusIn );
	    QApplication::sendEvent( this, &in );
	}
#if defined(Q_WS_WIN)
	if ( !isPopup() )
	    SetFocus( winId() );
	else {
#endif
#if defined(QT_ACCESSIBILITY_SUPPORT)
	    emit accessibilityChanged( QAccessible::Focus );
#endif
#if defined(Q_WS_WIN)
	}
#endif
    }
}

/*!
  Takes keyboard input focus from the widget.

  If the widget has active focus, a \link focusOutEvent() focus out
  event\endlink is sent to this widget to tell it that it is about to
  lose the focus.

  This widget must enable focus setting in order to get the keyboard input
  focus, i.e. it must call setFocusPolicy().

  \sa hasFocus(), setFocus(), focusInEvent(), focusOutEvent(),
  setFocusPolicy(), QApplication::focusWidget()
*/

void QWidget::clearFocus()
{
    if ( focusProxy() ) {
	focusProxy()->clearFocus();
	return;
    } else if ( hasFocus() ) {
	QWidget* w = qApp->focusWidget();
	// clear active focus
	qApp->focus_widget = 0;
	QFocusEvent out( QEvent::FocusOut );
	QApplication::sendEvent( w, &out );
#if defined(Q_WS_WIN)
	if ( !isPopup() )
	    SetFocus( 0 );
	else {
#endif
#if defined(QT_ACCESSIBILITY_SUPPORT)
	    emit accessibilityChanged( QAccessible::Focus );
#endif
#if defined(Q_WS_WIN)
	}
#endif
    }
}


/*!
  Finds a new widget to give the keyboard focus to, as appropriate for
  Tab/shift-Tab, and returns TRUE if is can find a new widget and
  FALSE if it can't,

  If \a next is true, this function searches "forwards", if \a next is
  FALSE, "backwards".

  Sometimes, you will want to reimplement this function.  For example,
  a web browser might reimplement it to move its "current active link"
  forwards or backwards, and call QWidget::focusNextPrevChild() only
  when it reaches the last/first.

  Child widgets call focusNextPrevChild() on their parent widgets, and
  only the top-level widget will thus make the choice of where to redirect
  focus.  By overriding this method for an object, you thus gain control
  of focus traversal for all child widgets.

  \sa focusData()
*/

bool QWidget::focusNextPrevChild( bool next )
{
    QWidget* p = parentWidget();
    if ( !testWFlags(WType_TopLevel) && p )
	return p->focusNextPrevChild(next);

    QFocusData *f = focusData( TRUE );

    QWidget *startingPoint = f->it.current();
    QWidget *candidate = 0;
    QWidget *w = next ? f->focusWidgets.last() : f->focusWidgets.first();
    do {
	if ( w && w != startingPoint &&
	     ( ( w->focusPolicy() & TabFocus ) == TabFocus )
	     && !w->focusProxy() && w->isVisible() && w->isEnabled())
	    candidate = w;
	w = next ? f->focusWidgets.prev() : f->focusWidgets.next();
    } while( w && !(candidate && w==startingPoint) );

    if ( !candidate )
	return FALSE;

    candidate->setFocus();
    return TRUE;
}

/*!
  Returns the focus widget in this widget's window.  This
  is not the same as QApplication::focusWidget(), which returns the
  focus widget in the currently active window.
*/

QWidget *QWidget::focusWidget() const
{
    QWidget *that = (QWidget *)this;		// mutable
    QFocusData *f = that->focusData( FALSE );
    if ( f && f->focusWidgets.count() && f->it.current() == 0 )
	f->it.toFirst();
    return ( f && f->it.current() && f->it.current()->focusPolicy() != NoFocus ) ? f->it.current() : 0;
}


/*!
  Returns a pointer to the focus data for this widget's top-level
  widget.

  Focus data always belongs to the top-level widget.  The focus data
  list contains all the widgets in this top-level widget that can
  accept focus, in tab order.  An iterator points to the current focus
  widget (focusWidget() returns a pointer to this widget).

  This information is useful for implementing advanced versions
  of focusNextPrevChild().
*/
QFocusData * QWidget::focusData()
{
    return focusData(TRUE);
}

/*!
  Internal function which lets us not create it too.
*/
QFocusData * QWidget::focusData( bool create )
{
    QWidget * tlw = topLevelWidget();
    QWExtra * ed = tlw->extraData();
    if ( !ed || !ed->topextra ) {
	if ( !create )
	    return 0;
	tlw->createTLExtra();
	ed = tlw->extraData();
    }
    if ( create && !ed->topextra->focusData )
	ed->topextra->focusData = new QFocusData;

    return ed->topextra->focusData;
}


/*!
  Enables key event compression, if \a compress is TRUE, and disables it
  if \a compress is FALSE.

  By default key compression is off, so widgets receive one key press
  event for each key press (or more, since autorepeat is usually on).
  If you turn it on and your program doesn't keep up with key input,
  Qt tries to compress key events so that more than one character can
  be processed in each event.

  For example, a word processor widget might receive 2, 3 or more
  characters in each QKeyEvent::text(), if the layout recalculation
  takes too long for the CPU.

  If a widget supports multiple character unicode input, it is always
  safe to turn the compression on.

  \sa QKeyEvent::text();
*/

void QWidget::setKeyCompression(bool compress)
{
    if ( compress )
	setWState( WState_CompressKeys );
    else
	clearWState( WState_CompressKeys );
}

/*! \property QWidget::isActiveWindow
    \brief whether this widget is the active window or a child of it

  The active window is the window that has keyboard focus.

  When popup windows are visible, this property is TRUE for both the
  active window and the popup.

  \sa setActiveWindow(), QApplication::activeWindow()
*/
bool QWidget::isActiveWindow() const
{
    return (topLevelWidget() == qApp->activeWindow() )||
	     ( isVisible() && topLevelWidget()->isPopup() );
}

/*!
  Moves the \a second widget around the ring of focus widgets
  so that keyboard focus moves from \a first widget to \a second
  widget when Tab is pressed.

  Note that since the tab order of the \e second widget is changed,
  you should order a chain like this:

  \code
    setTabOrder(a, b ); // a to b
    setTabOrder(b, c ); // a to b to c
    setTabOrder(c, d ); // a to b to c to d
  \endcode

  not like this:

  \code
    setTabOrder(c, d); // c to d
    setTabOrder(a, b); // a to b AND c to d
    setTabOrder(b, c); // a to b to c, but not c to d
  \endcode

  If either \a first or \a second has a focus proxy, setTabOrder()
  substitutes its/their proxies.

  \sa setFocusPolicy(), setFocusProxy()
*/
void QWidget::setTabOrder( QWidget* first, QWidget *second )
{
    if ( !first || !second ||
	first->focusPolicy() == NoFocus || second->focusPolicy() == NoFocus )
	return;

    while ( first->focusProxy() )
	first = first->focusProxy();
    while ( second->focusProxy() )
	second = second->focusProxy();

    QFocusData *f = first->focusData( TRUE );
    bool focusThere = (f->it.current() == second );
    f->focusWidgets.removeRef( second );
    if ( f->focusWidgets.findRef( first ) >= 0 )
	f->focusWidgets.insert( f->focusWidgets.at() + 1, second );
    else
	f->focusWidgets.append( second );
    if ( focusThere ) { // reset iterator so tab will work appropriately
	f->it.toFirst();
	while( f->it.current() && f->it.current() != second )
	    ++f->it;
    }
}

/*!\internal

  Moves the relevant subwidgets of this widget from the \a oldtlw's
  tab chain to that of the new parent, if there's anything to move and
  we're really moving

  This function is called from QWidget::reparent() *after* the widget
  has been reparented.

  \sa reparent()
*/

void QWidget::reparentFocusWidgets( QWidget * oldtlw )
{
    if ( oldtlw == topLevelWidget() )
	return; // nothing to do

    QFocusData * from = oldtlw ? oldtlw->topData()->focusData : 0;
    QFocusData * to;
    to = focusData();

    if ( from ) {
	from->focusWidgets.first();
	do {
	    QWidget * pw = from->focusWidgets.current();
	    while( pw && pw != this )
		pw = pw->parentWidget();
	    if ( pw == this ) {
		QWidget * w = from->focusWidgets.take();
		if ( w == from->it.current() )
		    // probably best to clear keyboard focus, or
		    // the user might become rather confused
		    w->clearFocus();
		if ( !isTopLevel() && w->focusPolicy() != NoFocus )
		    to->focusWidgets.append( w );
	    } else {
		from->focusWidgets.next();
	    }
	} while( from->focusWidgets.current() );
    }

    if ( to->focusWidgets.findRef(this) < 0 )
	to->focusWidgets.append( this );

    if ( !isTopLevel() && extra && extra->topextra && extra->topextra->focusData ) {
	// this widget is no longer a top-level widget, so get rid
	// of old focus data
	delete extra->topextra->focusData;
	extra->topextra->focusData = 0;
    }
}

/*!
  \fn void QWidget::recreate( QWidget *parent, WFlags f, const QPoint & p, bool showIt )

  \obsolete

  This method is provided to aid porting from Qt 1.0 to 2.0.  It has
  been renamed reparent() in Qt 2.0.
*/

/*! \property QWidget::frameSize
    \brief the size of the widget including any window frame
*/
QSize QWidget::frameSize() const
{
    if ( isTopLevel() && !isPopup() ) {
	if ( fstrut_dirty )
	    updateFrameStrut();
	QWidget *that = (QWidget *) this;
	QTLWExtra *top = that->topData();
	return QSize( crect.width() + top->fleft + top->fright,
		      crect.height() + top->ftop + top->fbottom );
    }
    return crect.size();
}

/*!
  \overload

  This corresponds to move( QSize(\a x, \a y) ).
*/
void QWidget::move( int x, int y )
{
    QPoint oldp(pos());
    internalSetGeometry( x + geometry().x() - QWidget::x(),
			 y + geometry().y() - QWidget::y(),
			 width(), height(), TRUE );
    if ( isVisible() && !isTopLevel() && !testWFlags(Qt::WSubWindow) && oldp != pos() && children() ) {
	QObjectListIt it(*children());
	register QObject *object;
	QWidget *widget;
	while ( it ) {
	    object = it.current();
	    ++it;
	    if ( object->isWidgetType() ) {
		widget = (QWidget*)object;
		if ( !widget->isHidden() && !widget->isTopLevel() && !widget->testWFlags(Qt::WSubWindow) &&
		     widget->backgroundOrigin() == WindowOrigin && widget->backgroundPixmap() )
		    widget->update();
	    }
	}
    }
}

/*!
  \overload

  This corresponds to resize( QSize(\a w, \a h) ).
*/
void QWidget::resize( int w, int h )
{
    internalSetGeometry( geometry().x(), geometry().y(), w, h, FALSE );
    setWState( WState_Resized );
}

/*!
  \overload

  This corresponds to setGeometry( QRect(\a x, \a y, \a w, \a h) ).
*/
void QWidget::setGeometry( int x, int y, int w, int h )
{
    QPoint oldp( pos( ));
    internalSetGeometry( x, y, w, h, TRUE );
    setWState( WState_Resized );
    if ( isVisible() && !isTopLevel() && !testWFlags(Qt::WSubWindow) && oldp != pos() && children() ) {
	QObjectListIt it(*children());
	register QObject *object;
	QWidget *widget;
	while ( it ) {
	    object = it.current();
	    ++it;
	    if ( object->isWidgetType() ) {
		widget = (QWidget*)object;
		if ( !widget->isHidden() && !widget->isTopLevel() && !widget->testWFlags(Qt::WSubWindow) &&
		     widget->backgroundOrigin() == WindowOrigin && widget->backgroundPixmap() )
		    widget->update();
	    }
	}
    }
}

/*! \property QWidget::focusEnabled
    \brief whether the widget accepts keyboard focus

  Keyboard focus is initially disabled (i.e., focusPolicy() ==
  QWidget::NoFocus).

  You must enable keyboard focus for a widget if it processes keyboard
  events.  This is normally done from the widget's constructor.  For
  instance, the QLineEdit constructor calls
  setFocusPolicy(QWidget::StrongFocus).

  \sa setFocusPolicy(), focusInEvent(), focusOutEvent(), keyPressEvent(),
      keyReleaseEvent(), isEnabled()
*/

/*! \enum QWidget::FocusPolicy

  This enum type defines the various policies a widget can have with
  respect to acquiring keyboard focus.

  The \e policy can be:

  \value TabFocus  the widget accepts focus by tabbing.
  \value ClickFocus  the widget accepts focus by clicking.
  \value StrongFocus  the widget accepts focus by both tabbing
  and clicking.
  \value WheelFocus  like StrongFocus plus the widget accepts
  focus by using the mouse wheel.
  \value NoFocus  the widget does not accept focus.

*/

/*! \property QWidget::focusPolicy
    \brief the way the widget accepts keyboard focus

  The policy is QWidget::TabFocus if the widget accepts keyboard
  focus by tabbing, QWidget::ClickFocus if the widget accepts focus
  by clicking, QWidget::StrongFocus if it accepts both and
  QWidget::NoFocus if it does not accept focus at all (the default
  for QWidget).

  You must enable keyboard focus for a widget if it processes
  keyboard events. This is normally done from the widget's
  constructor. For instance, the QLineEdit constructor calls
  setFocusPolicy(QWidget::StrongFocus).

  \sa focusEnabled, focusInEvent(), focusOutEvent(), keyPressEvent(),
      keyReleaseEvent(), enabled
*/

void QWidget::setFocusPolicy( FocusPolicy policy )
{
    if ( focusProxy() )
	focusProxy()->setFocusPolicy( policy );
    if ( policy != NoFocus ) {
	QFocusData * f = focusData( TRUE );
	if ( f->focusWidgets.findRef( this ) < 0 )
	    f->focusWidgets.append( this );
    } else {
	QFocusData *f = focusData( FALSE );
	if ( f )
	    f->focusWidgets.removeRef( this );
    }
    focus_policy = (uint) policy;
}

/*! \property QWidget::updatesEnabled
    \brief whether updates are enabled

  Calling update() and repaint() has no effect if updates are disabled.
  Paint events from the window system are processed normally even if
  updates are disabled.

  setUpdatesEnabled() normally used to disable updates for a short
  period of time, for instance to avoid screen flicker during large
  changes.

  Example:
  \code
    setUpdatesEnabled( FALSE );
    bigVisualChanges();
    setUpdatesEnabled( TRUE );
    repaint();
  \endcode

  \sa update(), repaint(), paintEvent()
*/
void QWidget::setUpdatesEnabled( bool enable )
{
    if ( enable )
	clearWState( WState_BlockUpdates );
    else
	setWState( WState_BlockUpdates );
}

/*
  Returns TRUE if there's no non-withdrawn top level window left
  (except the desktop, dialogs or popups).  This is an internal
  function used by QWidget::close() to decide whether to emit
  QApplication::lastWindowClosed() or not.
*/

static bool noMoreToplevels()
{
    QWidgetList *list   = qApp->topLevelWidgets();
    QWidget     *widget = list->first();
    while ( widget ) {
	if ( !widget->isHidden()
	     && !widget->isDesktop()
	     && !widget->isPopup()
	     && !widget->testWFlags( Qt::WStyle_Dialog) )
	    break;
	widget = list->next();
    }
    delete list;
    return widget == 0;
}


/*!
  Shows the widget and its child widgets.

  If its size or position has changed, Qt guarantees that a widget gets
  move and resize events just before the widget is shown.

  You almost never have to reimplement this function. If you need to
  change some settings before a widget is shown, use showEvent()
  instead. If you need to do some delayed initialization use polish().

  \sa showEvent(), hide(), showMinimized(), showMaximized(), showNormal(),
  isVisible(), polish()
*/

void QWidget::show()
{
    bool sendLayoutHint = !isTopLevel() && isHidden();
    clearWState( WState_ForceHide );

    if ( testWState(WState_Visible) )
	return; // nothing to do
    if ( !isTopLevel() && !parentWidget()->isVisibleTo( 0 ) ){
	// we should become visible, but our parents are explicitly
	// hidden. Don' worry, since we cleared the ForceHide flag,
	// our immediate parent will call show() on us again during
	// his own processing of show().
	if ( sendLayoutHint ) {
	    QEvent e( QEvent::ShowToParent );
	    QApplication::sendEvent( this, &e );
	}
	return;
    }

    in_show = TRUE;

    QApplication::sendPostedEvents( this, QEvent::ChildInserted );

    if ( isTopLevel() && !testWState( WState_Resized ) )  {
	// do this before sending the posted resize events. Otherwise
	// the layout would catch the resize event and may expand the
	// minimum size.
	QSize s = sizeHint();
	QSizePolicy::ExpandData exp;
#ifndef QT_NO_LAYOUT
	if ( layout() ) {
	    if ( layout()->hasHeightForWidth() )
		s.setHeight( layout()->totalHeightForWidth( s.width() ) );
	    exp =  layout()->expanding();
	} else
#endif
	    {
		if ( sizePolicy().hasHeightForWidth() )
		    s.setHeight( heightForWidth( s.width() ) );
		exp = sizePolicy().expanding();
	    }
	if ( exp & QSizePolicy::Horizontal )
	    s.setWidth( QMAX( s.width(), 200 ) );
	if ( exp & QSizePolicy::Vertical )
	    s.setHeight( QMAX( s.height(), 150 ) );
	QRect screen = QApplication::desktop()->screenGeometry( QApplication::desktop()->screenNumber( pos() ) );
	s.setWidth( QMIN( s.width(), screen.width()*2/3 ) );
	s.setHeight( QMIN( s.height(), screen.height()*2/3 ) );
	if ( !s.isEmpty() )
	    resize( s );
    }

    QApplication::sendPostedEvents( this, QEvent::Move );
    QApplication::sendPostedEvents( this, QEvent::Resize );

    setWState( WState_Visible );

    if ( parentWidget() )
	QApplication::sendPostedEvents( parentWidget(),
					QEvent::ChildInserted );

    if ( extra ) {
	int w = crect.width();
	int h = crect.height();
	if ( w < extra->minw || h < extra->minh ||
	     w > extra->maxw || h > extra->maxh ) {
	    w = QMAX( extra->minw, QMIN( w, extra->maxw ));
	    h = QMAX( extra->minh, QMIN( h, extra->maxh ));
	    resize( w, h );			// deferred resize
	}
    }

    if ( testWFlags(WStyle_Tool) || isPopup() ) {
	raise();
    } else if ( testWFlags(WType_TopLevel) ) {
	while ( QApplication::activePopupWidget() )
	    QApplication::activePopupWidget()->close();
    }

    if ( !testWState(WState_Polished) )
	polish();

    if ( children() ) {
	QObjectListIt it(*children());
	register QObject *object;
	QWidget *widget;
	while ( it ) {				// show all widget children
	    object = it.current();		//   (except popups and other toplevels)
	    ++it;
	    if ( object->isWidgetType() ) {
		widget = (QWidget*)object;
		if ( !widget->isHidden() && !widget->isTopLevel() )
		    widget->show();
	    }
	}
    }


    bool sendShowWindowRequest = FALSE;

    if ( !isTopLevel() && !parentWidget()->isVisible() ) {
	// we should become visible, but somehow our parent is not
	// visible, so we can't do that. Since it is not explicitly
	// hidden (that we checked above with isVisibleTo(0) ), our
	// window is not withdrawn, but may for example be iconfied or
	// on another virtual desktop. Therefore we have to prepare
	// for simply receiving a show event without show() beeing
	// called again (see the call to sendShowEventsToChildren() in
	// qapplication).
	showWindow();
	clearWState( WState_Visible );
	if ( sendLayoutHint ) {
	    QEvent e( QEvent::ShowToParent );
	    QApplication::sendEvent( this, &e );
	}
    } else {

	QShowEvent e;
	QApplication::sendEvent( this, &e );

	if ( testWFlags(WShowModal) ) {
	    // qt_enter_modal *before* show, otherwise the initial
	    // stacking might be wrong
	    qt_enter_modal( this );
	}

	// do not show the window directly, but post a showWindow
	// request to reduce flicker with laid out widgets
	if ( !isTopLevel() && !parentWidget()->in_show )
	    sendShowWindowRequest = TRUE;
	else
	    showWindow();

	if ( testWFlags(WType_Popup) )
	    qApp->openPopup( this );
    }

    if ( sendLayoutHint )
	QApplication::postEvent( parentWidget(),
				 new QEvent( QEvent::LayoutHint) );
    if ( sendShowWindowRequest )
	QApplication::postEvent( this, new QEvent( QEvent::ShowWindowRequest ) );

#if defined(QT_ACCESSIBILITY_SUPPORT)
    emit accessibilityChanged( QAccessible::ObjectShow );
#endif

    in_show = FALSE;
}

/*! \fn void QWidget::iconify() 
    \obsolete
*/

/*!
  Hides the widget.

  You almost never have to reimplement this function. If you need to
  do something after a widget is hidden, use hideEvent() instead.

  \sa isHhideEvent(), isHidden(), show(), showMinimized(), isVisible(), close()
*/

void QWidget::hide()
{
    if ( testWState(WState_ForceHide) )
	return;
    setWState( WState_ForceHide );

    if ( testWFlags(WType_Popup) )
	qApp->closePopup( this );

#if defined(Q_WS_WIN)
    if ( isTopLevel() && !isPopup() && parentWidget() && isActiveWindow() )
	parentWidget()->setActiveWindow();	// Activate parent
#endif

    hideWindow();

    if ( !testWState(WState_Visible) ) {
	QEvent e( QEvent::HideToParent );
	QApplication::sendEvent( this, &e );
	// post layout hint for non toplevels. The parent widget check is
	// necessary since the function is called in the destructor
	if ( !isTopLevel() && parentWidget() )
	    QApplication::postEvent( parentWidget(),
				     new QEvent( QEvent::LayoutHint) );
	return;
    }
    clearWState( WState_Visible );

    // next bit tries to move the focus if the focus widget is now
    // hidden.
    if ( qApp && qApp->focusWidget() == this )
	focusNextPrevChild( TRUE );

    QHideEvent e;
    QApplication::sendEvent( this, &e );

    // post layout hint for non toplevels. The parent widget check is
    // necessary since the function is called in the destructor
    if ( !isTopLevel() && parentWidget() )
	QApplication::postEvent( parentWidget(),
				 new QEvent( QEvent::LayoutHint) );

    sendHideEventsToChildren( FALSE );

    if ( testWFlags(WType_Modal) )
	qt_leave_modal( this );

#if defined(QT_ACCESSIBILITY_SUPPORT)
    emit accessibilityChanged( QAccessible::ObjectHide );
#endif
}


void QWidget::sendShowEventsToChildren( bool spontaneous )
{
     if ( children() ) {
	QObjectListIt it(*children());
	register QObject *object;
	QWidget *widget;
	while ( it ) {
	    object = it.current();
	    ++it;
	    if ( object->isWidgetType() ) {
		widget = (QWidget*)object;
		if ( !widget->isTopLevel() && !widget->isVisible() && !widget->isHidden() ) {
		    widget->setWState( WState_Visible );
		    widget->sendShowEventsToChildren( spontaneous );
		    QShowEvent e;
		    if ( spontaneous )
			QApplication::sendSpontaneousEvent( widget, &e );
		    else
			QApplication::sendEvent( widget, &e );
		}
	    }
	}
    }
}

void QWidget::sendHideEventsToChildren( bool spontaneous )
{
     if ( children() ) {
	QObjectListIt it(*children());
	register QObject *object;
	QWidget *widget;
	while ( it ) {
	    object = it.current();
	    ++it;
	    if ( object->isWidgetType() ) {
		widget = (QWidget*)object;
		if ( !widget->isTopLevel() && widget->isVisible() ) {
		    widget->clearWState( WState_Visible );
		    widget->sendHideEventsToChildren( spontaneous );
		    QHideEvent e;
		    if ( spontaneous )
			QApplication::sendSpontaneousEvent( widget, &e );
		    else
			QApplication::sendEvent( widget, &e );
		}
	    }
	}
    }
}


/*!
  Delayed initialization of a widget.

  This function will be called \e after a widget has been fully created
  and \e before it is shown the very first time.

  Polishing is useful for final initialization depending on an
  instantiated widget. This is something a constructor cannot
  guarantee since the initialization of the subclasses might not be
  finished.

  After this function, the widget has a proper font and palette and
  QApplication::polish() has been called.

  Remember to call QWidget's implementation when reimplementing this
  function.

  \sa constPolish(), QApplication::polish()
*/

void QWidget::polish()
{
    if ( !testWState(WState_Polished) ) {
	if ( !own_font && !QApplication::font( this ).isCopyOf( QApplication::font() ) ) {
	    setFont( QApplication::font( this ) );
	    own_font = FALSE;
	}
#ifndef QT_NO_PALETTE
	if ( !own_palette && !QApplication::palette( this ).isCopyOf( QApplication::palette() ) ) {
	    setPalette( QApplication::palette( this ) );
	    own_palette = FALSE;
	}
#endif
	setWState(WState_Polished);
	qApp->polish( this );
	QApplication::sendPostedEvents( this, QEvent::ChildInserted );
    }
}


/*!
  \fn void QWidget::constPolish() const

  Ensures that the widget is properly initialized by calling polish().

  Call constPolish() from functions like sizeHint() that depends on
  the widget being initialized, and that may be called before show().

  \warning Do not call constPolish() on a widget from inside that
  widget's constructor.

  \sa polish()
*/

/*!
  Closes this widget. Returns TRUE if the widget was closed, otherwise
  FALSE.

  If \a alsoDelete is TRUE or the widget has the \c WDestructiveClose
  widget flag, the widget is also deleted.  The widget can prevent
  itself from being closed by rejecting the QCloseEvent it gets.

  The QApplication::lastWindowClosed() signal is emitted when the last
  visible top level widget is closed.

  Note that closing the \l QApplication::mainWidget() terminates the
  application.

  \sa closeEvent(), QCloseEvent, hide(), QApplication::quit(),
  QApplication::setMainWidget(), QApplication::lastWindowClosed()
*/

bool QWidget::close( bool alsoDelete )
{
    if ( is_closing )
	return TRUE;
    is_closing = 1;
    WId id	= winId();
    bool isMain = qApp->mainWidget() == this;
    bool checkLastWindowClosed = isTopLevel() && !isPopup();
    QCloseEvent e;
    QApplication::sendEvent( this, &e );
    bool deleted = !QWidget::find(id);
    if ( !deleted && !e.isAccepted() ) {
	is_closing = 0;
	return FALSE;
    }
    if ( !deleted )
	hide();
    if ( checkLastWindowClosed
	 && qApp->receivers(SIGNAL(lastWindowClosed()))
	 && noMoreToplevels() )
	emit qApp->lastWindowClosed();
    if ( isMain )
	qApp->quit();
    if ( deleted )
	return TRUE;
    is_closing = 0;
    if ( alsoDelete || testWFlags(WDestructiveClose) )
	delete this;
    return TRUE;
}


/*!
  \fn bool QWidget::close()
  Closes this widget. Returns TRUE if the widget was closed, otherwise
  FALSE.

  First it sends the widget a QCloseEvent. The widget is \link hide()
  hidden\endlink if it \link QCloseEvent::accept() accepts\endlink the
  close event. The default implementation of QWidget::closeEvent()
  accepts the close event.

  The QApplication::lastWindowClosed() signal is emitted when the last
  visible top level widget is closed.

  \sa close()
*/

/*! \property QWidget::visible
    \brief whether the widget is visible

  Calling show() sets the widget to visible status if all its parent
  widgets up to the top-level widget are visible. If an ancestor is
  not visible, the widget won't become visible until all its
  ancestors are shown.

  Calling hide() hides a widget explicitly. An explicitly hidden
  widget will never become visible, even if all its ancestors become
  visible.

  Iconified top-level widgets also have hidden status, as well as
  having isMinimized() return TRUE. Windows that live on another
  virtual desktop (on platforms that support this concept) also have
  hidden status.

  A widget that happens to be obscured by other windows on the screen
  is considered visible.

  A widget receives show and hide events when its visibility status
  changes. Between a hide and a show event, there is no need in
  wasting any CPU on preparing or displaying information to the
  user. A video application, for example, might simply stop generating
  new frames.

  \sa show(), hide(), isHidden(), isVisibleTo(), isMinimized(),
  showEvent(), hideEvent()
*/


/*!
  Returns TRUE if this widget would become visible if \a ancestor is
  shown.

  This is the case if neither the widget itself nor every parent up to
  but excluding \a ancestor has been explicitly hidden.

  This function returns TRUE if the widget it is obscured by other
  windows on the screen, but would be visible if moved.

  isVisibleTo(0) is very similar to isVisible(), with the exception
  that it does not cover the iconfied-case or the situation where the
  window lives on another virtual desktop.

  \sa show() hide() isVisible()
*/

bool QWidget::isVisibleTo(QWidget* ancestor) const
{
    const QWidget * w = this;
    while ( w
	    && !w->isHidden()
	    && !w->isTopLevel()
	    && w->parentWidget()
	    && w->parentWidget()!=ancestor )
	w = w->parentWidget();
    return !w->isHidden();
}


/*!
  \fn bool QWidget::isVisibleToTLW() const
  \obsolete

  This function is deprecated. It is equivalent to isVisible()
*/

/*! \property QWidget::hidden
    \brief whether the widget is explicitly hidden

  If FALSE, the widget is visible or would become visible if all its
  ancestors became visible.

  \sa hide(), show(), isVisible(), isVisibleTo()
*/

/*! \property QWidget::visibleRect
    \brief the currently visible rectangle of the widget

  This property is useful to optimize immediate repainting of a
  widget. Typical usage is
  \code
    repaint( w->visibleRect() );
  \endcode
  or
  \code
    repaint( w->visibleRect(), FALSE );
  \endcode

  If nothing is visible, the rectangle returned is empty.
*/
QRect QWidget::visibleRect() const
{
    QRect r = rect();
    const QWidget * w = this;
    int ox = 0;
    int oy = 0;
    while ( w
	    && w->isVisible()
	    && !w->isTopLevel()
	    && w->parentWidget() ) {
	ox -= w->x();
	oy -= w->y();
	w = w->parentWidget();
	r = r.intersect( QRect( ox, oy, w->width(), w->height() ) );
    }
    if ( !w->isVisible() )
	return QRect();
    else
	return r;
}


/*!
  Adjusts the size of the widget to fit the contents.

  Uses sizeHint() if valid (i.e if the size hint's width and height are
  equal to or greater than 0), otherwise sets the size to the children
  rectangle (the union of all child widget geometries).

  \sa sizeHint(), childrenRect()
*/

void QWidget::adjustSize()
{
    if ( !testWState(WState_Polished) )
	polish();
    QSize s = sizeHint();
    if ( s.isValid() ) {
	resize( s );
	return;
    }
    QRect r = childrenRect();			// get children rectangle
    if ( r.isNull() )				// probably no widgets
	return;
    resize( r.width() + 2 * r.x(), r.height() + 2 * r.y() );
}


/*! \property QWidget::sizeHint
    \brief the recommended size for the widget

  If the value of this property is an invalid size, no size is
  recommended.

  The default implementation of sizeHint() returns an invalid size if
  there is no layout for this widget, the layout's preferred size
  otherwise.

  \sa QSize::isValid(), minimumSizeHint(), sizePolicy(), setMinimumSize(),
      updateGeometry()
*/

QSize QWidget::sizeHint() const
{
#ifndef QT_NO_LAYOUT
    if ( layout() )
	return layout()->totalSizeHint();
#endif
    constPolish();
    return QSize( -1, -1 );
}

/*! \property QWidget::minimumSizeHint
    \brief the recommended minimum size for the widget

  If the value of this property is an invalid size, no minimum size
  is recommended.

  The default implementation of minimumSizeHint() returns an invalid
  size if there is no layout for this widget, the layout's minimum
  size otherwise. Most built-in widgets reimplement minimumSizeHint().

  \l QLayout will never resize a widget to a size smaller than
  minimumSizeHint.

  \sa QSize::isValid(), resize(), setMinimumSize(), sizePolicy()
*/
QSize QWidget::minimumSizeHint() const
{
#ifndef QT_NO_LAYOUT
    if ( layout() )
	return layout()->totalMinimumSize();
#endif
    constPolish();
    return QSize( -1, -1 );
}


/*!
  \fn QWidget *QWidget::parentWidget( bool sameWindow ) const

  Returns a pointer to the parent of this widget, or a null pointer if
  it does not have any parent widget.
*/

/*!
  \fn WFlags QWidget::testWFlags( WFlags f ) const

  Returns the logical AND of the widget flags and \a f.

  Widget flags are a combination of Qt::WidgetFlags.

  \sa getWFlags(), setWFlags(), clearWFlags()
*/

/*!
  \fn WState QWidget::testWState( WState s ) const
  \internal

  Returns the logical AND of the widget states and \a s.
*/

/*!
  \fn uint QWidget::getWState() const
  \internal
  Returns the current widget state.
*/
/*!
  \fn void QWidget::clearWState( uint n )
  \internal

  Clears the widgets states \a n.
*/
/*!
  \fn void QWidget::setWState( uint n )
  \internal
  Sets the widgets states \a n.
*/



/*****************************************************************************
  QWidget event handling
 *****************************************************************************/


/*!
  This is the main event handler. You may reimplement this function
  in a subclass, but we recommend using one of the specialized event
  handlers instead.

  The main event handler first passes an event through all \link
  QObject::installEventFilter() event filters\endlink that have been
  installed.  If none of the filters intercept the event, it calls one
  of the specialized event handlers.

  Key press/release events are treated differently from other events.
  event() checks for Tab and shift-Tab and tries to move the focus
  appropriately.  If there is no widget to move the focus to (or the
  key press is not Tab or shift-Tab), event() calls keyPressEvent().

  This function returns TRUE if it is able to pass the event over to
  someone, or FALSE if nobody wanted the event.

  \sa closeEvent(), focusInEvent(), focusOutEvent(), enterEvent(),
  keyPressEvent(), keyReleaseEvent(), leaveEvent(),
  mouseDoubleClickEvent(), mouseMoveEvent(), mousePressEvent(),
  mouseReleaseEvent(), moveEvent(), paintEvent(),
  resizeEvent(), QObject::event(), QObject::timerEvent()
*/

bool QWidget::event( QEvent *e )
{
    if ( eventFilters ) {			// try filters
	if ( activate_filters(e) )		// stopped by a filter
	    return TRUE;
    }

    switch ( e->type() ) {

	case QEvent::Timer:
	    timerEvent( (QTimerEvent*)e );
	    break;

	case QEvent::MouseMove:
	    mouseMoveEvent( (QMouseEvent*)e );
	    if ( ! ((QMouseEvent*)e)->isAccepted() )
		return FALSE;
	    break;

	case QEvent::MouseButtonPress:
	    resetInputContext();
	    mousePressEvent( (QMouseEvent*)e );
	    if ( ! ((QMouseEvent*)e)->isAccepted() )
		return FALSE;
	    break;

	case QEvent::MouseButtonRelease:
	    mouseReleaseEvent( (QMouseEvent*)e );
	    if ( ! ((QMouseEvent*)e)->isAccepted() )
		return FALSE;
	    break;

	case QEvent::MouseButtonDblClick:
	    mouseDoubleClickEvent( (QMouseEvent*)e );
	    if ( ! ((QMouseEvent*)e)->isAccepted() )
		return FALSE;
	    break;
#ifndef QT_NO_WHEELEVENT
	case QEvent::Wheel:
	    wheelEvent( (QWheelEvent*)e );
	    if ( ! ((QWheelEvent*)e)->isAccepted() )
		return FALSE;
	    break;
#endif
	case QEvent::KeyPress: {
	    QKeyEvent *k = (QKeyEvent *)e;
	    bool res = FALSE;
	    if ( k->key() == Key_Backtab ||
		 (k->key() == Key_Tab &&
		  (k->state() & ShiftButton)) ) {
		QFocusEvent::setReason( QFocusEvent::Backtab );
		res = focusNextPrevChild( FALSE );
		QFocusEvent::resetReason();

	    } else if ( k->key() == Key_Tab ) {
		QFocusEvent::setReason( QFocusEvent::Tab );
		res = focusNextPrevChild( TRUE );
		QFocusEvent::resetReason();
	    }
	    if ( res )
		break;
	    keyPressEvent( k );
	    if ( !k->isAccepted() )
		return FALSE;
	    }
	    break;

	case QEvent::KeyRelease:
	    keyReleaseEvent( (QKeyEvent*)e );
	    if ( ! ((QKeyEvent*)e)->isAccepted() )
		return FALSE;
	    break;

	case QEvent::IMStart: {
	    QIMEvent *i = (QIMEvent *) e;
	    imStartEvent(i);
	    if (! i->isAccepted())
		return FALSE;
	    }
	    break;

	case QEvent::IMCompose: {
	    QIMEvent *i = (QIMEvent *) e;
	    imComposeEvent(i);
	    if (! i->isAccepted())
		return FALSE;
	    }
	    break;

	case QEvent::IMEnd: {
	    QIMEvent *i = (QIMEvent *) e;
	    imEndEvent(i);
	    if (! i->isAccepted())
		return FALSE;
	    }
	    break;

	case QEvent::FocusIn:
	    resetInputContext();
	    focusInEvent( (QFocusEvent*)e );
	    setFontSys();
	    break;

	case QEvent::FocusOut:
	    focusOutEvent( (QFocusEvent*)e );
	    break;

	case QEvent::Enter:
	    enterEvent( e );
	    break;

	case QEvent::Leave:
	     leaveEvent( e );
	    break;

	case QEvent::Paint:
	    // At this point the event has to be delivered, regardless
	    // whether the widget isVisible() or not because it
	    // already went through the filters
	    paintEvent( (QPaintEvent*)e );
	    break;

	case QEvent::Move:
	    moveEvent( (QMoveEvent*)e );
	    break;

	case QEvent::Resize:
	    resizeEvent( (QResizeEvent*)e );
	    break;

	case QEvent::Close: {
	    QCloseEvent *c = (QCloseEvent *)e;
	    closeEvent( c );
	    if ( !c->isAccepted() )
		return FALSE;
	    }
	    break;
	case QEvent::ContextMenu: {
	    QContextMenuEvent *c = (QContextMenuEvent *)e;
	    contextMenuEvent( c );
	    if ( !c->isAccepted() )
		return FALSE;
	    }
	    break;
#ifndef QT_NO_DRAGANDDROP
	case QEvent::Drop:
	    dropEvent( (QDropEvent*) e);
	    break;
	case QEvent::DragEnter:
	    dragEnterEvent( (QDragEnterEvent*) e);
	    break;
	case QEvent::DragMove:
	    dragMoveEvent( (QDragMoveEvent*) e);
	    break;
	case QEvent::DragLeave:
	    dragLeaveEvent( (QDragLeaveEvent*) e);
	    break;
#endif
	case QEvent::Show:
	    showEvent( (QShowEvent*) e);
	    break;
	case QEvent::Hide:
	    hideEvent( (QHideEvent*) e);
	    break;
	case QEvent::ShowWindowRequest:
	    if ( !isHidden() )
		showWindow();
	    break;
	case QEvent::ChildInserted:
	case QEvent::ChildRemoved:
	    childEvent( (QChildEvent*) e);
	    break;
	case QEvent::ParentFontChange:
	    if ( isTopLevel() )
		break;
	    // FALL THROUGH
	case QEvent::ApplicationFontChange:
	    if ( !own_font && !isDesktop() ) {
		if ( !isTopLevel() && QApplication::font( this ).isCopyOf( QApplication::font() ) )
		    setFont( parentWidget()->font() );
		else
		    setFont( QApplication::font( this ) );
		own_font = FALSE;
	    }
	    break;
#ifndef QT_NO_PALETTE
	case QEvent::ParentPaletteChange:
	    if ( isTopLevel() )
		break;
	    // FALL THROUGH
	case QEvent::ApplicationPaletteChange:
	    if ( !own_palette && !isDesktop() ) {
		if ( !isTopLevel() && QApplication::palette( this ).isCopyOf( QApplication::palette() ) )
		    setPalette( parentWidget()->palette() );
		else
		    setPalette( QApplication::palette( this ) );
		own_palette = FALSE;
	    }
	    break;
#endif
	case QEvent::WindowActivate:
	case QEvent::WindowDeactivate:
	    windowActivationChange( e->type() == QEvent::WindowActivate );
	    if ( children() ) {
		QObjectListIt it( *children() );
		QObject *o;
		while( ( o = it.current() ) != 0 ) {
		    ++it;
		    QApplication::sendEvent( o, e );
		}
	    }
	    break;
	default:
	    if ( e->type() >= QEvent::User ) {
		customEvent( (QCustomEvent*) e );
		return TRUE;
	    }
	    return FALSE;
    }
    return TRUE;
}


/*!
  This event handler can be reimplemented in a subclass to receive
  mouse move events for the widget.

  If mouse tracking is switched off, mouse move events only occur if a
  mouse button is down while the mouse is being moved.	If mouse
  tracking is switched on, mouse move events occur even if no mouse
  button is down.

  QMouseEvent::pos() reports the position of the mouse cursor, relative to
  this widget.  For press and release events, the position is usually
  the same as the position of the last mouse move event, but it might be
  different if the user moves and clicks the mouse fast.  This is
  a feature of the underlying window system, not Qt.

  \sa setMouseTracking(), mousePressEvent(), mouseReleaseEvent(),
  mouseDoubleClickEvent(), event(), QMouseEvent
*/

void QWidget::mouseMoveEvent( QMouseEvent * e)
{
    e->ignore();
}

/*!
  This event handler can be reimplemented in a subclass to receive
  mouse press events for the widget.

  If you create new widgets in the mousePressEvent() the
  mouseReleaseEvent() may not end up where you expect, depending on the
  underlying window system (or X11 window manager), the widgets'
  location and maybe more.

  The default implementation implements the closing of popup widgets
  when you click outside the window. For other widget types it does
  nothing.

  \sa mouseReleaseEvent(), mouseDoubleClickEvent(),
  mouseMoveEvent(), event(),  QMouseEvent
*/

void QWidget::mousePressEvent( QMouseEvent *e )
{
    e->ignore();
    if ( isPopup() ) {
	e->accept();
	QWidget* w;
	while ( (w = qApp->activePopupWidget() ) && w != this ){
	    w->close();
	    if (qApp->activePopupWidget() == w) // widget does not want to dissappear
		w->hide(); // hide at least
	}
	if (!rect().contains(e->pos()) ){
	    close();
	}
    }
}

/*!
  This event handler can be reimplemented in a subclass to receive
  mouse release events for the widget.

  \sa mouseReleaseEvent(), mouseDoubleClickEvent(),
  mouseMoveEvent(), event(),  QMouseEvent
*/

void QWidget::mouseReleaseEvent( QMouseEvent * e )
{
    e->ignore();
}

/*!
  This event handler can be reimplemented in a subclass to receive
  mouse double click events for the widget.

  The default implementation generates a normal mouse press event.

  Note that the widgets gets a mousePressEvent() and a mouseReleaseEvent()
  before the mouseDoubleClickEvent().

  \sa mousePressEvent(), mouseReleaseEvent()
  mouseMoveEvent(), event(),  QMouseEvent
*/

void QWidget::mouseDoubleClickEvent( QMouseEvent *e )
{
    mousePressEvent( e );			// try mouse press event
}

#ifndef QT_NO_WHEELEVENT
/*!
  This event handler can be reimplemented in a subclass to receive
  wheel events for the widget.

  If you reimplement this handler, it is very important that you \link
  QWheelEvent ignore()\endlink the event if you do not handle it, so
  that the widget's parent can interpret it.

  The default implementation ignores the event.

  \sa QWheelEvent::ignore(), QWheelEvent::accept(),
  event(), QWheelEvent
*/

void QWidget::wheelEvent( QWheelEvent *e )
{
    e->ignore();
}
#endif

/*!
  This event handler can be reimplemented in a subclass to receive key
  press events for the widget.

  A widget must call setFocusPolicy() to accept focus initially and
  have focus in order to receive a key press event.

  If you reimplement this handler, it is very important that you
  ignore() the event if you do not understand it, so that the widget's
  parent can interpret it.

  The default implementation closes popup widgets if you hit
  escape.  Otherwise the event is ignored.

  \sa keyReleaseEvent(), QKeyEvent::ignore(), setFocusPolicy(),
  focusInEvent(), focusOutEvent(), event(), QKeyEvent
*/

void QWidget::keyPressEvent( QKeyEvent *e )
{
    if ( isPopup() && e->key() == Key_Escape ) {
	e->accept();
	close();
    } else {
	e->ignore();
    }
}

/*!
  This event handler can be reimplemented in a subclass to receive
  key release events for the widget.

  A widget must \link setFocusPolicy() accept focus\endlink initially
  and \link hasFocus() have focus\endlink in order to receive a key
  release event.

  If you reimplement this handler, it is very important that you \link
  QKeyEvent ignore()\endlink the release if you do not understand it,
  so that the widget's parent can interpret it.

  The default implementation ignores the event.

  \sa keyPressEvent(), QKeyEvent::ignore(), setFocusPolicy(),
  focusInEvent(), focusOutEvent(), event(), QKeyEvent
*/

void QWidget::keyReleaseEvent( QKeyEvent *e )
{
    e->ignore();
}

/*!
  This event handler can be reimplemented in a subclass to receive
  keyboard focus events (focus received) for the widget.

  A widget normally must setFocusPolicy() to something other than
  NoFocus in order to receive focus events.  (Note that the
  application programmer can call setFocus() on any widget, even those
  that do not normally accept focus.)

  The default implementation updates the widget if it accepts
  focus (see focusPolicy()).  It also calls setMicroFocusHint(), hinting any
  system-specific input tools about the focus of the user's attention.

  \sa focusOutEvent(), setFocusPolicy(),
  keyPressEvent(), keyReleaseEvent(), event(), QFocusEvent
*/

void QWidget::focusInEvent( QFocusEvent * )
{
    if ( focusPolicy() != NoFocus || !isTopLevel() ) {
	update();
	if ( testWState(WState_AutoMask) )
	    updateMask();
	setMicroFocusHint(width()/2, 0, 1, height(), FALSE);
    }
}

/*!
  This event handler can be reimplemented in a subclass to receive
  keyboard focus events (focus lost) for the widget.

  A widget normally must setFocusPolicy() to something other than
  NoFocus in order to receive focus events.  (Note that the
  application programmer can call setFocus() on any widget, even those
  that do not normally accept focus.)

  The default implementation calls repaint() since the widget's
  colorGroup() changes from active to normal, so the widget probably
  needs repainting.  It also calls setMicroFocusHint(), hinting any
  system-specific input tools about the focus of the user's attention.

  \sa focusInEvent(), setFocusPolicy(),
  keyPressEvent(), keyReleaseEvent(), event(), QFocusEvent
*/

void QWidget::focusOutEvent( QFocusEvent * )
{
    if ( focusPolicy() != NoFocus || !isTopLevel() ){
	update();
	if ( testWState(WState_AutoMask) )
	    updateMask();
    }
}

/*! \property QWidget::microFocusHint
    \brief the currently set micro focus hint for this widget.

  See the documentation of setMicroFocusHint() for more information.
*/
QRect QWidget::microFocusHint() const
{
    if ( !extra || extra->micro_focus_hint.isEmpty() )
	return QRect(width()/2, 0, 1, height() );
    else
	return extra->micro_focus_hint;
}

/*!
  This event handler can be reimplemented in a subclass to receive
  widget enter events.

  An event is sent to the widget when the mouse cursor enters the widget.

  \sa leaveEvent(), mouseMoveEvent(), event()
*/

void QWidget::enterEvent( QEvent * )
{
}

/*!
  This event handler can be reimplemented in a subclass to receive
  widget leave events.

  A leave event is sent to the widget when the mouse cursor leaves
  the widget.

  \sa enterEvent(), mouseMoveEvent(), event()
*/

void QWidget::leaveEvent( QEvent * )
{
}

/*!
  This event handler can be reimplemented in a subclass to receive
  paint events.

  A paint event is a request to repaint all or part of the widget.  It
  can happen as a result of repaint() or update(), or because the
  widget was obscured and has now been uncovered, or for many other
  reasons.

  Many widgets can simply repaint their entire surface when asked to,
  but some slow widgets need to optimize by painting only the
  requested region - QPaintEvent::region(). This speed optimization
  does not change the result, as painting is clipped to that region
  during event processing.  QListView and QCanvas do this, for
  example.

  Qt also tries to speed up painting by merging multiple paint events
  into one.  When update() is called several times or the window
  system sends several paint events, Qt merges these events into one
  event with a larger region (see QRegion::unite()).  repaint() does
  not permit this optimization, so we suggest using update() when
  possible.

  When the paint event occurs, the update region normally has been
  erased, so that you're painting on the widget's background. There
  are a couple of exceptions, though, and QPaintEvent::erased() tells
  you whether the widget has been erased or not.

  The background is settable using setBackgroundMode(),
  setBackgroundColor() or setBackgroundPixmap(). The documentation for
  setBackgroundMode() elaborates on the background; we recommend
  reading it.

  \sa event(), repaint(), update(), QPainter, QPixmap, QPaintEvent
*/

void QWidget::paintEvent( QPaintEvent * )
{
}


/*!
  This event handler can be reimplemented in a subclass to receive
  widget move events.  When the widget receives this event, it is
  already at the new position.

  The old position is accessible through QMoveEvent::oldPos().

  \sa resizeEvent(), event(), move(), QMoveEvent
*/

void QWidget::moveEvent( QMoveEvent * )
{
}


/*!
  This event handler can be reimplemented in a subclass to receive
  widget resize events. When resizeEvent() is called, the widget
  already has its new geometry. The old size is accessible through
  QResizeEvent::oldSize(), though.

  The widget will be erased and receive a paint event immediately
  after processing the resize event. No drawing has to (and should) be
  done inside this handler.

  Widgets that have been created with the \c WResizeNoErase flag will not
  be erased. Nevertheless, they will receive a paint event for their
  entire area afterwards. Again, no drawing needs to be done inside
  this handler.

  The default implementation calls updateMask() if the widget
  has \link QWidget::setAutoMask() automatic masking\endlink
  enabled.

  \sa moveEvent(), event(), resize(), QResizeEvent, paintEvent()
*/

void QWidget::resizeEvent( QResizeEvent * )
{
    if ( testWState(WState_AutoMask) )
	updateMask();
}

/*!
  This event handler can be reimplemented in a subclass to receive
  widget close events.

  The default implementation calls e->accept(), which hides this widget.
  See the QCloseEvent documentation for more details.

  \sa event(), hide(), close(), QCloseEvent
*/

void QWidget::closeEvent( QCloseEvent *e )
{
    e->accept();
}


/*!
  This event handler can be reimplemented in a subclass to receive
  widget context menu events.

  The default implementation calls e->ignore(), which rejects the context
  event.
  See the QContextMenuEvent documentation for more details.

  \sa event(), QContextMenuEvent
*/

void QWidget::contextMenuEvent( QContextMenuEvent *e )
{
    e->ignore();
}


/*!
  This event handler can be reimplemented in a subclass to receive
  Input Method composition events.  This handler is called when
  the user begins inputting text via an Input Method.

  The default implementation calls e->ignore(), which rejects the
  Input Method event.
  See the QIMEvent documentation for more details.

  \sa event(), QIMEvent
*/
void QWidget::imStartEvent( QIMEvent *e )
{
    e->ignore();
}

/*!
  This event handler can be reimplemented in a subclass to receive
  Input Method composition events.  This handler is called when
  the user has entered some text via an Input Method.

  The default implementation calls e->ignore(), which rejects the
  Input Method event.
  See the QIMEvent documentation for more details.

  \sa event(), QIMEvent
*/
void QWidget::imComposeEvent( QIMEvent *e )
{
    e->ignore();
}


/*!
  This event handler can be reimplemented in a subclass to receive
  Input Method composition events.  This handler is called when
  the user has finished inputting text via an Input Method.

  The default implementation calls e->ignore(), which rejects the
  Input Method event.
  See the QIMEvent documentation for more details.

  \sa event(), QIMEvent
*/
void QWidget::imEndEvent( QIMEvent *e )
{
    e->ignore();
}


#ifndef QT_NO_DRAGANDDROP

/*!
  This event handler is called when a drag is in progress and the
  mouse enters this widget.

  See the \link dnd.html Drag-and-drop documentation\endlink for
  an overview of how to provide drag-and-drop in your application.

  \sa QTextDrag, QImageDrag, QDragEnterEvent
*/
void QWidget::dragEnterEvent( QDragEnterEvent * )
{
}

/*!
  This event handler is called when a drag is in progress and the
  mouse enters this widget, and whenever it moves within
  the widget.

  See the \link dnd.html Drag-and-drop documentation\endlink for
  an overview of how to provide drag-and-drop in your application.

  \sa QTextDrag, QImageDrag, QDragMoveEvent
*/
void QWidget::dragMoveEvent( QDragMoveEvent * )
{
}

/*!
  This event handler is called when a drag is in progress and the
  mouse leaves this widget.

  See the \link dnd.html Drag-and-drop documentation\endlink for
  an overview of how to provide drag-and-drop in your application.

  \sa QTextDrag, QImageDrag, QDragLeaveEvent
*/
void QWidget::dragLeaveEvent( QDragLeaveEvent * )
{
}

/*!
  This event handler is called when the drag is dropped on this
  widget.

  See the \link dnd.html Drag-and-drop documentation\endlink for
  an overview of how to provide drag-and-drop in your application.

  \sa QTextDrag, QImageDrag, QDropEvent
*/
void QWidget::dropEvent( QDropEvent * )
{
}

#endif // QT_NO_DRAGANDDROP

/*!
  This event handler can be reimplemented in a subclass to receive
  widget show events.

  Non-spontaneous show events are sent to widgets right before they are
  shown. Spontaneous show events of top level widgets are delivered
  afterwards, naturally.

  \sa event(), QShowEvent
  */
void QWidget::showEvent( QShowEvent * )
{
    if ( focusWidget() )
	return;
    else if ( focusPolicy() )
	setFocus();
    else
	focusNextPrevChild( TRUE );
}

/*!
  This event handler can be reimplemented in a subclass to receive
  widget hide events.

  Hide events are sent to widgets right after they have been hidden.

  \sa event(), QHideEvent
  */
void QWidget::hideEvent( QHideEvent * )
{
}

#if defined(Q_WS_MAC)

/*!
  This special event handler can be reimplemented in a subclass to receive
  native Macintosh events.

  If the event handler returns FALSE, this native event is passed back to
  Qt, which translates the event into a Qt event and sends it to the
  widget.  If the event handler returns TRUE, the event is stopped.

  \warning This function is not portable.

  \sa QApplication::macEventFilter()
*/

bool QWidget::macEvent( MSG * )
{
    return FALSE;
}

#elif defined(Q_WS_WIN)

/*!
  This special event handler can be reimplemented in a subclass to receive
  native Windows events.

  If the event handler returns FALSE, this native event is passed back to
  Qt, which translates the event into a Qt event and sends it to the
  widget.  If the event handler returns TRUE, the event is stopped.

  \warning This function is not portable.

  \sa QApplication::winEventFilter()
*/

bool QWidget::winEvent( MSG * )
{
    return FALSE;
}

#elif defined(Q_WS_X11)

/*!
  This special event handler can be reimplemented in a subclass to receive
  native X11 events.

  If the event handler returns FALSE, this native event is passed back to
  Qt, which translates the event into a Qt event and sends it to the
  widget.  If the event handler returns TRUE, the event is stopped.

  \warning This function is not portable.

  \sa QApplication::x11EventFilter()
*/

bool QWidget::x11Event( XEvent * )
{
    return FALSE;
}

#elif defined(Q_WS_QWS)

/*!
  This special event handler can be reimplemented in a subclass to receive
  native Qt/Embedded events.

  If the event handler returns FALSE, this native event is passed back to
  Qt, which translates the event into a Qt event and sends it to the
  widget.  If the event handler returns TRUE, the event is stopped.

  \warning This function is not portable.

  \sa QApplication::qwsEventFilter()
*/

bool QWidget::qwsEvent( QWSEvent * )
{
    return FALSE;
}

#endif

/*! \property QWidget::autoMask
    \brief whether the auto mask feature is enabled for the widget

  Transparent widgets use a mask to define their visible region.
  QWidget has some built-in support to make the task of recalculating
  the mask easier. When setting auto mask to TRUE, updateMask() will
  be called whenever the widget is resized or changes its focus
  state.

  Note: When you re-implement resizeEvent(), focusInEvent() or
  focusOutEvent() in your custom widgets and still want to ensure
  that the auto mask calculation works, you will have to add

  \code
    if ( autoMask() )
	updateMask();
  \endcode

  at the end of your event handlers. The same holds for all member
  functions that change the appearance of the widget in a way that a
  recalculation of the mask is necessary.

  While being a technically appealing concept, masks have one big
  drawback: when using complex masks that cannot be expressed easily
  with relatively simple regions, they tend to be very slow on some
  window systems. The classic example is a transparent label. The
  complex shape of its contents makes it necessary to represent its
  mask by a bitmap, which consumes both memory and time.  If all you
  want is to blend the background of several neighboring widgets
  together seamlessly, you may probably want to use
  setBackgroundOrigin() rather than a mask.

  \sa autoMask() updateMask() setMask() clearMask() setBackgroundOrigin()
*/

bool QWidget::autoMask() const
{
    return testWState(WState_AutoMask);
}

void QWidget::setAutoMask( bool enable )
{
    if ( enable == autoMask() )
	return;

    if ( enable ) {
	setWState(WState_AutoMask);
	updateMask();
    } else {
	clearWState(WState_AutoMask);
	clearMask();
    }
}

/*! \enum QWidget::BackgroundOrigin

  This enum defines the origin used to draw a widget's background
  pixmap.

  \value WidgetOrigin  the pixmap is drawn in the widget's coordinate system.
  \value ParentOrigin  the pixmap is drawn in the parent's coordinate system.
  \value WindowOrigin  the pixmap is drawn in the toplevel window's coordinate system.
*/

/*! \property QWidget::backgroundOrigin
    \brief the origin of the widget's background

  The origin is either WidgetOrigin (the default), ParentOrigin or
  WindowOrigin.

  This makes a difference only if the widget has a background pixmap,
  in which case positioning matters. Using WindowOrigin for several
  neighboring widgets makes the background blend together seamlessly.

  \sa backgroundPixmap(), setBackgroundMode()
*/
QWidget::BackgroundOrigin QWidget::backgroundOrigin() const
{
    return extra ? (BackgroundOrigin)extra->bg_origin : WidgetOrigin;
}

void QWidget::setBackgroundOrigin( BackgroundOrigin origin )
{
    if ( origin == backgroundOrigin() )
	return;
    createExtra();
    extra->bg_origin = origin;
    update();
}

/*!
  This function can be reimplemented in a subclass to support
  transparent widgets. It is supposed to be called whenever a widget
  changes state in a way that the shape mask has to be recalculated.

  \sa setAutoMask(), updateMask(), setMask(), clearMask()
  */
void QWidget::updateMask()
{
}


/*!
  \fn QLayout* QWidget::layout () const

  Returns a pointer to the layout engine that manages the geometry of
  this widget's children.

  If the widget does not have a layout, layout() returns a null pointer.

  \sa  sizePolicy()
*/


/*!  Sets this widget to use \a l to manage the geometry of its
  children.

  If there already was a layout for this widget, the old layout is
  forgotten.  (Note that it is not deleted.)

  \sa layout() QLayout sizePolicy()
*/
#ifndef QT_NO_LAYOUT
void QWidget::setLayout( QLayout *l )
{
    lay_out = l;
}
#endif

/*! \property QWidget::sizePolicy
    \brief the default layout behavior of the widget

  If there is a QLayout that manages this widget's children, the size
  policy specified by that layout is used. If there is no such
  QLayout, the result of this function is used.

  The default policy is Preferred/Preferred, which means that the
  widget can be freely resized, but prefers to be the size sizeHint()
  returns. Button-like widgets set the size policy to specify that
  they may stretch horizontally, but are fixed vertically. The same
  applies to lineedit controls (such as QLineEdit, QSpinBox or an
  editable QComboBox) and other horizontally orientated widgets (such
  as QProgressBar).  A QToolButton on the other hand wants to be
  squared, therefore it allows growth in both directions. Widgets that
  support different directions (such as QSlider, QScrollBar or
  QHeader) specify stretching in the respective direction
  only. Widgets that can provide scrollbars (usually subclasses of
  QScrollView) tend to specify that they can use additional space, and
  that they can survive on less than sizeHint().

  \sa sizeHint() QLayout QSizePolicy updateGeometry()
*/
QSizePolicy QWidget::sizePolicy() const
{
    return extra ? extra->size_policy
	: QSizePolicy( QSizePolicy::Preferred, QSizePolicy::Preferred );
}

void QWidget::setSizePolicy( QSizePolicy policy )
{
    if ( policy == sizePolicy() )
	return;
    createExtra();
    extra->size_policy = policy;
    updateGeometry();
}


/*!
  Returns the preferred height for this widget, given the width \a w.
  The default implementation returns 0, indicating that the preferred
  height does not depend on the width.

  \warning Does not look at the widget's layout
*/

int QWidget::heightForWidth( int w ) const
{
    (void)w;
    return 0;
}

/*! \property QWidget::customWhatsThis
    \brief whether the widget wants to handle What's This help
    manually

  The default implementation of customWhatsThis() returns FALSE,
  which means the widget will not receive any events in Whats This
  mode.

  The widget may leave What's This mode by calling
  QWhatsThis::leaveWhatsThisMode(), with or without actually
  displaying any help text.

  You may also reimplement customWhatsThis() if your widget is a
  "passive interactor" supposed to work under all circumstances.
  Simply don't call QWhatsThis::leaveWhatsThisMode() in that case.

  \sa QWhatsThis::inWhatsThisMode() QWhatsThis::leaveWhatsThisMode()
*/
bool QWidget::customWhatsThis() const
{
    return FALSE;
}

/*!  Returns the visible child widget at pixel position \a (x,y) in
  the widget's own coordinate system.

  If \a includeThis is TRUE, and there is no child visible at \a
  (x,y), the widget itself is returned.
*/
QWidget  *QWidget::childAt( int x, int y, bool includeThis ) const
{
    if ( !rect().contains( x, y ) )
	return 0;
    if ( children() ) {
	QObjectListIt it( *children() );
	it.toLast();
	QWidget *w, *t;
	while( (w=(QWidget *)it.current()) != 0 ) {
	    --it;
	    if ( w->isWidgetType() && !w->isTopLevel() && !w->isHidden() ) {
		if ( ( t = w->childAt( x - w->x(), y - w->y(), TRUE ) ) )
		    return t;
	    }
	}
    }
    if ( includeThis )
	return (QWidget*)this;
    return 0;
}

/*!\overload */
QWidget  *QWidget::childAt( const QPoint & p, bool includeThis ) const
{
    return childAt( p.x(), p.y(), includeThis );
}


/*!
  Notifies the layout system that this widget has changed and may need
  to change geometry.

  Call this function if the sizeHint() or sizePolicy() have changed.

  For explicitly hidden widgets, updateGeometry() is a no-op. The
  layout system will be notified as soon as the widget is shown.
*/

void QWidget::updateGeometry()
{
    if ( !isTopLevel() && !isHidden() )
	QApplication::postEvent( parentWidget(),
				 new QEvent( QEvent::LayoutHint ) );
}

/*!\overload

  A convenience version of reparent that does not take widget
  flags as argument.

  Calls reparent(\a parent, getWFlags()&~WType_Mask, \a p, \a showIt )
*/
void  QWidget::reparent( QWidget *parent, const QPoint & p,
			 bool showIt )
{
    reparent( parent, getWFlags() & ~WType_Mask, p, showIt );
}



/*!
  Shows the widget in full-screen mode.

  Calling this function has no effect for other than top-level
  widgets.

  To return from full-screen mode, call showNormal().

  Full-screen mode works fine under Windows, but has certain problems
  under X.  These problems are due to limitations of the ICCCM
  protocol that specifies the communication between X11 clients and
  the window manager.  ICCCM simply does not know the concept of
  non-decorated full-screen windows. Therefore, the best we can do is
  to request a borderless window and place and resize it to fill the
  entire screen. Depending on the window manager, this may or may not
  work. The borderless window is requested using MOTIF hints, which
  are at least partially supported by virtually all modern window
  managers.

  An alternative would be to bypass the window manager at all and to
  create a window with the WX11BypassWM flag. This has other severe
  problems, though, like totally broken keyboard focus and very
  strange effects on desktop changes or when the user raises other
  windows.

  Future window managers that follow modern post-ICCCM specifications
  may support full-screen mode properly.

  \sa showNormal(), showMaximized(), show(), hide(), isVisible()
*/
void QWidget::showFullScreen()
{
    if ( !isTopLevel() )
	return;
    if ( topData()->fullscreen ) {
	show();
	raise();
	return;
    }
    if ( topData()->normalGeometry.width() < 0 )
	topData()->normalGeometry = QRect( pos(), size() );
    reparent( 0, WType_TopLevel | WStyle_Customize | WStyle_NoBorderEx |
	      // preserve some widget flags
	      (getWFlags() & 0xffff0000),
	      QPoint(0,0) );
    topData()->fullscreen = 1;
    move(0, 0);
    resize( qApp->desktop()->size() );
    raise();
    show();
#if defined(Q_WS_X11)
    extern void qt_wait_for_window_manager( QWidget* w ); // defined in qwidget_x11.cpp
    qt_wait_for_window_manager( this );
#endif

    setActiveWindow();
}

/*!
  \fn bool QWidget::isMaximized() const

  Returns TRUE if this widget is a top-level widget that is maximized,
  or else FALSE.

  Note that due to limitations in some window-systems,
  this does not always report expected results (eg. if the user on X11
  maximizes the window via the window manager, Qt has no way of telling
  this from any other resize). This will improve as window manager
  protocols advance.

  \sa showMaximized()
*/

/*! \property QWidget::ownCursor
    \brief whether the widget uses its own cursor

  If FALSE, the widget uses its parent widget's cursor

  \sa setCursor(), unsetCursor()
*/

/*! \property QWidget::ownFont
    \brief whether the widget uses its own font

  If FALSE, the widget uses its parent widget's font

  \sa setFont(), unsetFont()
*/

/*! \property QWidget::ownPalette
    \brief whether the widget uses its own palette

  If FALSE, the widget uses its parent widget's palette

  \sa setPalette(), unsetPalette()
*/

/*!
  \reimp
*/
#if defined(QT_ACCESSIBILITY_SUPPORT)
QAccessibleInterface *QWidget::accessibleInterface()
{
    return new QAccessibleWidget( this );
}
#endif


