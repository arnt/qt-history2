/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qwidget.cpp#64 $
**
** Implementation of QWidget class
**
** Author  : Haavard Nord
** Created : 931031
**
** Copyright (C) 1993-1995 by Troll Tech AS.  All rights reserved.
**
** --------------------------------------------------------------------------
** IMPORTANT NOTE: Widget identifier should only be set with the set_id()
** function, otherwise widget mapping will not work.
*****************************************************************************/

#include "qobjcoll.h"
#include "qwidget.h"
#include "qwidcoll.h"
#include "qpalette.h"
#include "qkeycode.h"
#include "qapp.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qwidget.cpp#64 $";
#endif

/*!
\class QWidget qwidget.h
\brief The QWidget class is the base class of all user interface objects.
\ingroup abstractwidgets

A widget without a parent, called a top level widget, is a window with
a frame and title bar (depends on the widget style specified by the widget
flags).

A widget with a parent becomes a child window in the parent's window.
*/

// --------------------------------------------------------------------------
// Internal QWidgetMapper class
//
// The purpose of this class is to map widget identifiers to QWidget objects.
// All QWidget objects register themselves in the QWidgetMapper when they
// get an identifier. Widgets unregister themselves when they change ident-
// ifier or when they are destroyed. A widget identifier is really a window
// handle.
//
// The widget mapper is created and destroyed by the main application routines
// in the file qapp_xxx.cpp.
//

static const WDictSize = 101;

class QWidgetMapper : public QWidgetIntDict
{						// maps ids -> widgets
public:
    QWidgetMapper();
   ~QWidgetMapper();
    QWidget *find( WId id );			// find widget
    void     insert( const QWidget * );		// insert widget
    bool     remove( WId id );			// remove widget
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
    QWidgetIntDict::insert((long)widget->id(),widget);
}

inline bool QWidgetMapper::remove( WId id )
{
    if ( cur_id == id ) {			// reset current widget
	cur_id = 0;
	cur_widget = 0;
    }
    return QWidgetIntDict::remove((long)id);
}


// --------------------------------------------------------------------------
// QWidget member functions
//

/*!
Constructs a widget which is a child of \e parent, with the name \e name and
widget flags set to \e f.

If \e parent is 0, the new widget will be a top level window. If \e parent
is another widget, the new widget will be a child window inside \e parent.

The \e name is sent to the QObject constructor.

The widget flags are strictly internal.	 You are strongly advised to use 0.
*/

QWidget::QWidget( QWidget *parent, const char *name, WFlags f )
    : QObject( parent, name ), QPaintDevice( PDT_WIDGET ),
      pal( *qApp->palette() )			// use application palette
{
    initMetaObject();				// initialize meta object
    isWidget = TRUE;				// is a widget
    ident = 0;					// default attributes
    flags = f;
    extra = 0;					// no extra widget info
    focusChild = 0;				// no child has focus
    create();					// platform-dependent init
}

/*!
Destroys the widget.

All children of this widget are deleted first.
The application exits if this widget is (was) the main widget.
*/

QWidget::~QWidget()
{
    if ( QApplication::main_widget == this )	// reset main widget
	QApplication::main_widget = 0;
    if ( testWFlags(WFont_Metrics) )		// remove references to this
	QFontMetrics::reset( this );
    if ( testWFlags(WFont_Info) )		// remove references to this
	QFontInfo::reset( this );
    if ( childObjects ) {			// widget has children
	register QObject *obj = childObjects->first();
	while ( obj ) {				// delete all child objects
	    obj->parentObj = 0;			// object should not remove
	    delete obj;				//   itself from the list
	    obj = childObjects->next();
	}
	delete childObjects;
	childObjects = 0;
    }
    destroy();					// platform-dependent cleanup
    delete extra;
}


void QWidget::createMapper()			// create widget mapper
{
    mapper = new QWidgetMapper;
    CHECK_PTR( mapper );
}

void QWidget::destroyMapper()			// destroy widget mapper
{
    if ( !mapper )				// already gone
	return;
    register QWidget *w;
    QWidgetIntDictIt it( *((QWidgetIntDict*)mapper) );
    w = it.current();
    while ( w ) {				// remove child widgets first
	if ( !w->parentObj ) {			// widget is a parent
	    delete w;
	    w = it.current();			// w will be next widget
	}
	else					// skip child widgets now
	    w = ++it;
    }
    w = it.toFirst();
    while ( w ) {				// delete the rest
	delete w;
	w = ++it;
    }
    delete mapper;
    mapper = 0;
}

void QWidget::set_id( WId id )			// set widget identifier
{
    if ( !mapper )				// mapper destroyed
	return;
    if ( ident )
	mapper->remove( ident );
    ident = id;
#if defined(_WS_X11_)
    hd = id;					// X11: hd == ident
#endif
    if ( id )
	mapper->insert( this );
}

void QWidget::createExtra()			// create extra data
{
    if ( !extra ) {				// if not exists
	extra = new QWExtra;
	CHECK_PTR( extra );
	extra->guistyle = QApplication::style();// initialize
	extra->minw = extra->minh = -1;
	extra->maxw = extra->maxh = -1;
	extra->incw = extra->inch = -1;
    }
}

/*!
  Returns a pointer to the widget with window identifer/handle \e id.

  The window identifier type depends by the underlying window system,
  see qwindefs.h for the actual definition.
  If there is no widget with this identifier, a null pointer is returned.
  \sa wmapper(), id()
*/

QWidget *QWidget::find( WId id )		// find widget with id
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
  \internal
  Returns the widget flags for this this widget.

  Widget flags are internal, not meant for public use.
  \sa testWFlags(), setWFlags(), clearWFlags() */

/*! \fn void QWidget::setWFlags( WFlags f )
  \internal
  Sets the widget flags \e f.

  Widget flags are internal, not meant for public use.
  \sa testWFlags(), getWFlags(), clearWFlags()
*/

/*!
  \fn void QWidget::clearWFlags( WFlags f )
  \internal
  Clears the widget flags \e f.

  Widget flags are internal, not meant for public use.
  \sa testWFlags(), getWFlags(), setWFlags()
*/


/*! \fn void QWidget::destroyed()
This signal is emitted immediately before the widget is destroyed.

All the widget's children will be destroyed before the signal is
emitted.
*/


/*!
  \fn WId QWidget::id() const
  Returns the window system identifier of the widget.

  Portable in principle, but if you use it you are probably about to do
  something non-portable. Be careful.
  \sa find()
*/


/*!
  Returns the GUI style for this widget.

  \sa setStyle(), QApplication::style().
*/

GUIStyle QWidget::style() const			// get widget GUI style
{
    return extra ? extra->guistyle : QApplication::style();
}

/*!
  Sets the GUI style for this widget.

  Only \c MotifStyle is allowed in this version of Qt.

  \sa style(), QApplication::setStyle().
*/

void QWidget::setStyle( GUIStyle style )	// set widget GUI style
{
#if defined(LINUX_RESTRICTED)
    if ( style != MotifStyle ) {
	warning( "QWidget::setStyle: Only Motif style is supported" );
	return;
    }
#endif
    createExtra();
    extra->guistyle = style;
}


/*!
  Enables the widget so that it can receive mouse and keyboard events.
  \sa disable(), isDisabled()
*/

void QWidget::enable()				// enable events
{
    clearWFlags( WState_Disabled );
}

/*!
  Disables the widget so that it will not receive mouse and keyboard events.
  \sa enable(), isDisabled()
*/

void QWidget::disable()				// disable events
{
    setWFlags( WState_Disabled );
}

/*! \fn bool QWidget::isDisabled() const
  Returns TRUE if the widget is disabled, or FALSE if it is enabled.
  \sa enable(), disable()
*/


/*!
  \fn const QRect &QWidget::frameGeometry() const
  Returns the geometry of the widget, relative to its parent and
  including the window frame.
  \sa geometry(), x(), y(), pos()
*/

/*!
  \fn const QRect &QWidget::geometry() const
  Returns the geometry of the widget, relative to its parent widget
  and excluding the window frame.
  \sa frameGeometry(), size(), rect()
*/

/*!
  \fn int QWidget::x() const
  Returns the x coordinate of the widget, relative to its parent
  widget and including the window frame.
  \sa frameGeometry(), y(), pos()
*/

/*!
  \fn int QWidget::y() const
  Returns the y coordinate of the widget, relative to its parent
  widget and including the window frame.
  \sa frameGeometry(), x(), pos()
*/

/*!
  \fn QPoint QWidget::pos() const
  Returns the postion of the widget in its parent widget, including
  the window frame.
  \sa frameGeometry(), x(), y()
*/

/*!
  \fn QSize QWidget::size() const
  Returns the size of the widget, excluding the window frame.
  \sa geometry(), width(), height()
*/

/*!
  \fn int QWidget::width() const
  Returns the width of the widget, excluding the window frame.
  \sa geometry(), height(), size()
*/

/*!
  \fn int QWidget::height() const
  Returns the height of the widget, excluding the window frame.
  \sa geometry(), width(), size()
*/

/*!
  \fn QRect QWidget::rect() const
  Returns the the internal geometry of the widget, excluding the window frame.
  rect() equals QRect(0,0,width(),height()).
  \sa size()
*/

/*!
  \fn QFontMetrics QWidget::fontMetrics() const
  Returns the font metrics for the widget.
*/

/*!
  \fn QFontInfo QWidget::fontInfo() const
  Returns the font info for the widget.
*/

/*!
  Returns the background color of this widget.

  The background color is independent of the color group.
  The background color will be overwritten when setting a new palette.
  \sa setBackgroundColor(), foregroundColor(), colorGroup()
*/

const QColor &QWidget::backgroundColor() const
{
    return bg_col;
}

/*!
  Returns the foreground color of this widget.

  The foreground color equals <code>colorGroup().foreground()</code>.
  \sa backgroundColor(), colorGroup()
*/

const QColor &QWidget::foregroundColor() const
{
    return colorGroup().foreground();
}

/*!
  Returns the current color group of the widget palette.

  The color group is determined by the state of the widget.

  A disabled widget returns the QPalette::disabled() color group.<br>
  A widget in focus returns the QPalette::active() color group.<br>
  A normale widget returns the QPalette::normal() color group.<br>

  \sa palette(), setPalette()
*/

const QColorGroup &QWidget::colorGroup() const	// get current colors
{
    if ( testWFlags(WState_Disabled) )
	return pal.disabled();
    else if ( qApp->focus_widget == this )
	return pal.active();
    else
	return pal.normal();
}

/*!
  Returns the widget palette.
  \sa setPalette(), colorGroup().
*/

const QPalette &QWidget::palette() const	// get widget palette
{
    return pal;
}

/*!
  Sets the widget palette to \e p. The widget background color is set to
  <code>colorGroup().background()</code>.
  \sa palette(), colorGroup().
*/

void QWidget::setPalette( const QPalette &p )	// set widget palette
{
    pal = p;
    setBackgroundColor( colorGroup().background() );
    update();
}


/*!
Returns the font currently set for the widget.

QFontInfo will tell you what font is actually being used.

\sa setFont(), fontInfo(), fontMetrics()
*/

const QFont &QWidget::font()
{
    return fnt;
}

/*!
Sets the font for the widget.

The fontInfo() function reports the actual font that is being used by the
widget.

This code fragment sets a 12 point helvetica bold font:
\code
  QFont f("Helvetica", 12, QFont::Bold);
  setFont( f );
\endcode

\sa font(), fontInfo(), fontMetrics()
*/

void QWidget::setFont( const QFont &font )
{
    fnt = font;
    fnt.handle();				// force load font
    update();
}


/*!
Returns the widget cursor shape.
\sa setCursor()
*/

const QCursor &QWidget::cursor() const
{
    return curs;
}


/*! \fn bool QWidget::setMouseTracking( bool enable )
Enables or disables mouse tracking and returns the previous setting.

If mouse tracking is enabled, the widget will always receive mouse
move events, even when no mouse button is pressed down.

If mouse tracking is disabled (default), the widget will only receive
mouse move events when a mouse button is pressed down.

\sa mouseMoveEvent().
*/

#if !defined(_WS_X11_)
bool QWidget::setMouseTracking( bool enable )
{
    bool v = testWFlags( WMouseTracking );
    if ( enable )
	setWFlags( WMouseTracking );
    else
	clearWFlags( WMouseTracking );
    return v;
}
#endif // _WS_X11_


/*!
Returns TRUE if the widget (not one of its children) has the
keyboard focus.
*/

bool QWidget::hasFocus() const
{
    return qApp->focusWidget() == this;
}


/*!
\internal
Sets the frame rectangle and recomputes the client rectangle.

The frame rectangle is the geometry of this widget including any
decorative borders, in its parent's coordinate system.

The client rectangle is the geometry of just this widget in its
parent's coordinate system.
*/

void QWidget::setFRect( const QRect &r )	// set frect, update crect
{
    crect.setLeft( crect.left() + r.left() - frect.left() );
    crect.setTop( crect.top() + r.top() - frect.top() );
    crect.setRight( crect.right() + r.right() - frect.right() );
    crect.setBottom( crect.bottom() + r.bottom() - frect.bottom() );
    frect = r;
}

/*!
\internal
Sets the client rectangle and recomputes the frame rectangle.

The client rectangle is the geometry of just this widget in its
parent's coordinate system.

The frame rectangle is the geometry of this widget including any
decorative borders, in its parent's coordinate system.
*/

void QWidget::setCRect( const QRect &r )	// set crect, update frect
{
    frect.setLeft( frect.left() + r.left() - crect.left() );
    frect.setTop( frect.top() + r.top() - crect.top() );
    frect.setRight( frect.right() + r.right() - crect.right() );
    frect.setBottom( frect.bottom() + r.bottom() - crect.bottom() );
    crect = r;
}

/*!
Enables or disables the keyboard input focus events for the widget.

Focus events are initially disabled.
*/

void QWidget::setAcceptFocus( bool enable )
{
    if ( enable )
	setWFlags( WState_AcceptFocus );
    else
	clearWFlags( WState_AcceptFocus );
}


/*!
Translates the widget coordinate \e pos to global screen coordinates.
\sa mapFromGlobal().
*/

QPoint QWidget::mapToGlobal( const QPoint &pos ) const
{						// map to global coordinates
    register QWidget *w = (QWidget*)this;
    QPoint p = pos;
    while ( w ) {
	p += w->crect.topLeft();
	w = w->parentWidget();
    }
    return p;
}

/*!
Translates the global screen coordinate \e pos to widget coordinates.
\sa mapToGlobal().
*/

QPoint QWidget::mapFromGlobal( const QPoint &pos ) const
{						// map from global coordinates
    register QWidget *w = (QWidget*)this;
    QPoint p = pos;
    while ( w ) {
	p -= w->crect.topLeft();
	w = w->parentWidget();
    }
    return p;
}

/*!
Translates the widget coordinate \e pos to a coordinate in the parent widget.

Same as mapToGlobal() if the widget has no parent.
\sa mapFromParent().
*/

QPoint QWidget::mapToParent( const QPoint &p ) const
{						// map to parent coordinates
    return p + crect.topLeft();
}

/*!
Translates the parent widget coordinate \e pos to widget coordinates.

Same as mapFromGlobal() if the widget has no parent.
\sa mapToParent().
*/

QPoint QWidget::mapFromParent( const QPoint &p ) const
{						// map from parent coordinate
    return p - crect.topLeft();
}

/*!
Closes this widget.  First it sends the widget a QCloseEvent, then,
if the widget did accept that, it does an explicit delete of the
widget and all its children.
*/

bool QWidget::close( bool forceKill )		// close widget
{
    QCloseEvent event;
    QApplication::sendEvent( this, &event );
    if ( event.isAccepted() || forceKill )
	delete this;
    return event.isAccepted();
}

/*!
\fn bool QWidget::isVisible() const
Returns TRUE if the widget is visible, or FALSE if the widget is invisible.

Calling show() makes the widget visible. Calling hide() makes the widget
invisible.

A widget is considered visible even if it is obscured by other windows on the
screen.
*/

/*! \fn QWidget *QWidget::parentWidget() const
Returns a pointer to the parent of this widget, or a null pointer if
it does not have any parent widget.
*/

/*!
\fn bool QWidget::testWFlags( WFlags n ) const

Returns non-zero if any of the widget flags in \e n are set. The
widget flags are listed in qwindefs.h, and are strictly for
internal use.

\internal

Here are the flags and what they mean:<dl compact>
<dt>WState_Created <dd> Means that the widget has a valid id().
<dt>WState_Disabled <dd> Mouse and keyboard events disabled.
<dt>WState_Visible <dd> Visible (may be hidden by other windows).
<dt>WState_Active <dd> NOT USED!
<dt>WState_Paint <dd> Being painted.
<dt>WState_MGrab <dd> Currently grabbing the mouse pointer.
<dt>WState_KGrab <dd> Currently grabbing the keyboard input.
<dt>WState_Focus <dd> NOT USED!
<dt>WType_Overlap <dd> Overlapping/top level
<dt>WType_Modal <dd> Modal widget.
<dt>WType_Popup <dd> Popup widget.
<dt>WType_Desktop <dd> Desktop widget (root window).
<dt>WStyle_Title <dd> NOT USED!
<dt>WStyle_Border <dd> NOT USED!
<dt>WStyle_Close <dd> NOT USED!
<dt>WStyle_Resize <dd> NOT USED!
<dt>WStyle_Minimize <dd> NOT USED!
<dt>WStyle_Maximize <dd> NOT USED!
<dt>WStyle_MinMax <dd> NOT USED!
<dt>WStyle_All <dd> All style flags set.
<dt>WMouseTracking <dd> The widget receives mouse move events (not only drag).
<dt>WHasAccel <dd> The widget has an associated QAccel.
<dt>WConfigPending <dd> Config event pending.
<dt>WResizeNoErase <dd> Widget resizing should not erase the widget.
<dt>WExplicitHide <dd> Flags that hide() has been called before first show().
<dt>WCursorSet <dd> Flags that a cursor has been set.
<dt>WPaintDesktop <dd> Widget wants desktop paint events.
<dt>WPaintUnclipped <dd> Paint without clipping child widgets.
<dt>WPaintClever <dd> Widget wants every update rectangle.
<dt>WNoUpdates <dd> Do not update the widget.
<dt>WRecreated <dd> The widet has been recreated.
</dl>
*/

// --------------------------------------------------------------------------
// QWidget event handling
//

class QKeyEventFriend : public QKeyEvent {	// trick to use accel flag
public:
    QKeyEventFriend() : QKeyEvent(0,0,0,0) {}
    bool didAccel() const { return accel != 0; }
    void setAccel()	  { accel = TRUE; }
};

/*!
This is the main event handler. You may reimplement this function in
a sub class, but we recommend using one of the specialized event
handlers instead.

The main event handler first passes an event through all event
filters that have been installed (see QObject::installEventFilter()).
If none of the filters intercept the event, it calls one of the
specialized event handlers.

Key press/release events are treated differently from other events.
First it checks if there exists an accelerator object (QAccel)
that want the key press (accelerators do not get key release events).
If not, it sends the event to the widget that has the keyboard focus.
If there is no widget in focus or the focus widget did not want the key,
the event is sent to the top level widget.

This function returns TRUE if it is able to pass the event over to someone,
or FALSE if nobody wanted the event.

\sa QObject::event(), closeEvent(),
focusInEvent(), focusOutEvent(), keyPressEvent(), keyReleaseEvent(),
mouseDoubleClickEvent(), mouseMoveEvent(), mousePressEvent(),
mouseReleaseEvent(), moveEvent(), paintEvent(), resizeEvent() and
timerEvent().
*/

bool QWidget::event( QEvent *e )		// receive event(),
{
    if ( eventFilters ) {			// pass through event filters
	if ( activate_filters( e ) )		// stopped by a filter
	    return TRUE;
    }
    switch ( e->type() ) {

	case Event_Timer:
	    timerEvent( (QTimerEvent*)e );
	    break;

	case Event_MouseMove:
	    mouseMoveEvent( (QMouseEvent*)e );
	    break;

	case Event_MouseButtonPress:
	    mousePressEvent( (QMouseEvent*)e );
	    break;

	case Event_MouseButtonRelease:
	    mouseReleaseEvent( (QMouseEvent*)e );
	    break;

	case Event_MouseButtonDblClick:
	    mouseDoubleClickEvent( (QMouseEvent*)e );
	    break;

	case Event_KeyPress: {
	    QKeyEventFriend *k = (QKeyEventFriend*)e;
	    QWidget *w = this;
	    if ( !k->didAccel() ) {
		k->setAccel();			// flag that we tried accel
		while ( w ) {			// try all parents
		    bool has_accel = w->testWFlags(WHasAccel);
		    if ( has_accel && w->children() ) {
			QObjectListIt it( *w->children() );
			QObject *obj;
			bool found_accel = FALSE;
			while ( (obj=it.current()) ) {
			    if ( !obj->highPriority() )
				break;
			    if ( !obj->inherits("QAccel") )
				break;
			    found_accel = TRUE;
			    if ( obj->event( k ) )
				return TRUE;	// accel wanted it
			    ++it;
			}
			if ( !found_accel )	// accel probably removed
			    w->clearWFlags( WHasAccel );
		    }
		    else if ( has_accel )	// accel but not children???
			w->clearWFlags( WHasAccel );
		    w = w->parentWidget();
		}
	    }
#if 0
	    bool res = FALSE;
	    if ( k->key() == Key_Tab )
		res = focusNextChild();
	    else if ( k->key() == Key_Backtab )
		res = focusPrevChild();
	    if ( res )
		break;
#endif
	    if ( qApp->focusWidget() ) {	// send to focus widget
		w = qApp->focusWidget();
	    }
	    else {
		w = this;			// search for parent widget
		while ( w->parentWidget() )
		    w = w->parentWidget();
	    }
	    w->keyPressEvent( k );
#if defined(_WS_X11_)
	    if ( !k->isAccepted() && w->parentWidget() ) {
		while ( w->parentWidget() )
		    w = w->parentWidget();
		w->keyPressEvent( k );		// pass event to top level
	    }
#endif
	    }
	    break;

	case Event_KeyRelease: {
	    QKeyEvent *k = (QKeyEvent*)e;
	    keyReleaseEvent( k );
#if defined(_WS_X11_)
	    if ( !k->isAccepted() && !testWFlags(WType_Overlap) && parentObj )
		return parentObj->event( e );	// pass event to parent
#endif
	    }
	    break;

	case Event_FocusIn:
	    focusInEvent( (QFocusEvent*)e );
	    break;

	case Event_FocusOut:
	    focusOutEvent( (QFocusEvent*)e );
	    break;

	case Event_Paint:
	    paintEvent( (QPaintEvent*)e );
	    break;

	case Event_Move:
	    moveEvent( (QMoveEvent*)e );
	    break;

	case Event_Resize:
	    resizeEvent( (QResizeEvent*)e );
	    break;

	case Event_Close: {
	    QCloseEvent *c = (QCloseEvent *)e;
	    closeEvent( c );
	    if ( !c->isAccepted() )
		return FALSE;
	    }
	    break;

	case Event_AccelInserted:
	    setWFlags( WHasAccel );
	    break;

	default:
	    return FALSE;
    }
    return TRUE;
}

/*!
This event handler can be reimplemented in a sub class to receive
timer events for the widget.

The default implementation does nothing.

\sa QObject::startTimer(), QObject::killTimer() and event().
*/

void QWidget::timerEvent( QTimerEvent * )
{
}

/*!
This event handler can be reimplemented in a sub class to receive
mouse move events for the widget.

If mouse tracking is switched off, mouse move events will only occur
if a mouse button is down.  If mouse tracking is swithed on, mouse
move events will occur even if no mouse button is down.

The default implementation does nothing.

\sa setMouseTracking() and event().
*/

void QWidget::mouseMoveEvent( QMouseEvent * )
{
}

/*!
This event handler can be reimplemented in a sub class to receive
mouse press events for the widget.

The default implementation does nothing.

\sa mouseReleaseEvent() and event().
*/

void QWidget::mousePressEvent( QMouseEvent * )
{
}

/*!
This event handler can be reimplemented in a sub class to receive
mouse release events for the widget.

The default implementation does nothing.

\sa mousePressEvent() and event().
*/

void QWidget::mouseReleaseEvent( QMouseEvent * )
{
}

/*!
This event handler can be reimplemented in a sub class to receive
mouse double click events for the widget.

The default implementation generates a normal mouse press event.

\sa mousePressEvent() and event().
*/

void QWidget::mouseDoubleClickEvent( QMouseEvent *e )
{
    mousePressEvent( e );			// try mouse press event
}

/*!
This event handler can be reimplemented in a sub class to receive
key press events for the widget.

The default implementation ignores the key.

\sa keyReleaseEvent() and event().
*/

void QWidget::keyPressEvent( QKeyEvent *e )
{
    e->ignore();
}

/*!
This event handler can be reimplemented in a sub class to receive
key release events for the widget.

The default implementation ignores the key.

\sa keyPressEvent() and event().
*/

void QWidget::keyReleaseEvent( QKeyEvent *e )
{
    e->ignore();
}

/*!
This event handler can be reimplemented in a sub class to receive
keyboard focus events (focus received) for the widget.

The default implementation does nothing.

\sa focusOutEvent() and event().
*/

void QWidget::focusInEvent( QFocusEvent * )
{
}

/*!
This event handler can be reimplemented in a sub class to receive
keyboard focus events (focus lost) for the widget.

The default implementation does nothing.

\sa focusInEvent() and event().
*/

void QWidget::focusOutEvent( QFocusEvent * )
{
}

/*!
This event handler can be reimplemented in a sub class to receive
widget paint events.

The default implementation does nothing.

\sa event().
*/

void QWidget::paintEvent( QPaintEvent * )
{
}

/*!
This event handler can be reimplemented in a sub class to receive
widget move events.

The default implementation does nothing.

\sa resizeEvent() and event().
*/

void QWidget::moveEvent( QMoveEvent * )
{
}

/*!
This event handler can be reimplemented in a sub class to receive
widget resize events.

The default implementation does nothing.

\sa moveEvent() and event().
*/

void QWidget::resizeEvent( QResizeEvent * )
{
}

/*!
This event handler can be reimplemented in a sub class to receive
widget close events.

The default implementation does nothing.

\sa event().
*/

void QWidget::closeEvent( QCloseEvent * )
{
}


#if defined(_WS_MAC_)

bool QWidget::macEvent( MSG * )			// Macintosh event
{
    return FALSE;
}

#elif defined(_WS_WIN_)

bool QWidget::winEvent( MSG * )			// Windows (+NT) event
{
    return FALSE;
}

#elif defined(_WS_PM_)

bool QWidget::pmEvent( QMSG * )			// OS/2 PM event
{
    return FALSE;
}

#elif defined(_WS_X11_)

bool QWidget::x11Event( XEvent * )		// X11 event
{
    return FALSE;
}

#endif
