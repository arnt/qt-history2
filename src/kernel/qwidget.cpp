/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qwidget.cpp#96 $
**
** Implementation of QWidget class
**
** Author  : Haavard Nord
** Created : 931031
**
** Copyright (C) 1993-1996 by Troll Tech AS.  All rights reserved.
**
** --------------------------------------------------------------------------
** IMPORTANT NOTE: Widget identifier should only be set with the set_id()
** function, otherwise widget mapping will not work.
*****************************************************************************/

#include "qobjcoll.h"
#include "qwidget.h"
#include "qwidcoll.h"
#include "qpixmap.h"
#include "qkeycode.h"
#include "qapp.h"

RCSTAG("$Id: //depot/qt/main/src/kernel/qwidget.cpp#96 $")


/*----------------------------------------------------------------------------
  \class QWidget qwidget.h
  \brief The QWidget class is the base class of all user interface objects.

  \ingroup abstractwidgets

  It receives mouse, keyboard and other events from the window system.
  It has a number of virtual functions which can be reimplemented in
  order to respond to these events.

  By far the most important is paintEvent() which is called whenever
  the widget needs to update its on-screen representation.  Other
  commonly implemented events include resizeEvent(), keyPressEvent(),
  mousePressEvent() etc.

  A widget without a parent, called a top level widget, is a window
  with a frame and title bar (depending on the widget style specified
  by the widget flags). A widget with a parent is a child window in
  its parent.

  \sa QEvent, QPainter
 ----------------------------------------------------------------------------*/


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


/*****************************************************************************
  QWidget member functions
 *****************************************************************************/

/*----------------------------------------------------------------------------
  Constructs a widget which is a child of \e parent, with the name \e name and
  widget flags set to \e f.

  If \e parent is 0, the new widget becomes a top level window. If \e
  parent is another widget, the new widget becomes a child window inside \e
  parent.

  The \e name is sent to the QObject constructor.

  The widget flags are strictly internal.  You are strongly advised to use 0.
 ----------------------------------------------------------------------------*/

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

/*----------------------------------------------------------------------------
  Destroys the widget.

  All children of this widget are deleted first.
  The application exits if this widget is (was) the main widget.
 ----------------------------------------------------------------------------*/

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
#if defined(_WS_WIN_)
	    if ( obj->isWidgetType() ) {	// is child widget
		QWidget *w = (QWidget *)obj;
		w->clearWFlags( WState_Created );
		w->set_id( 0 );			// Windows destroys children
	    }
#endif
	    obj->parentObj = 0;			// object should not remove
	    delete obj;				//   itself from the list
	    obj = childObjects->next();
	}
	delete childObjects;
	childObjects = 0;
    }
    destroy();					// platform-dependent cleanup
    if ( extra )
	deleteExtra();
}


/*----------------------------------------------------------------------------
  \internal
  Creates the global widget mapper.
  The widget mapper converts window handles to widget pointers.
  \sa destroyMapper()
 ----------------------------------------------------------------------------*/

void QWidget::createMapper()
{
    mapper = new QWidgetMapper;
    CHECK_PTR( mapper );
}

/*----------------------------------------------------------------------------
  \internal
  Destroys the global widget mapper.
  \sa createMapper()
 ----------------------------------------------------------------------------*/

void QWidget::destroyMapper()
{
    if ( !mapper )				// already gone
	return;
    register QWidget *w;
    QWidgetIntDictIt it( *((QWidgetIntDict*)mapper) );
    while ( (w=it.current()) ) {		// remove parents widgets
	++it;
	if ( !w->parentObj )			// widget is a parent
	    delete w;
    }
#if defined(DEBUG)
    ASSERT( it.count() == 0 );
#endif
#if 0
    w = it.toFirst();				// shouln't be any more widgets
    while ( w ) {				// delete the rest
	delete w;
	w = ++it;
    }
#endif
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


/*----------------------------------------------------------------------------
  \internal
  Returns a pointer to the block of extra widget data.
 ----------------------------------------------------------------------------*/

QWExtra *QWidget::extraData()
{
    return extra;
}

/*----------------------------------------------------------------------------
  \internal
  Creates the widget extra data.
 ----------------------------------------------------------------------------*/

void QWidget::createExtra()
{
    if ( !extra ) {				// if not exists
	extra = new QWExtra;
	CHECK_PTR( extra );
	extra->guistyle = QApplication::style();// default style
	extra->minw = extra->minh = -1;
	extra->maxw = extra->maxh = -1;
	extra->incw = extra->inch = -1;
	extra->caption = extra->iconText = 0;
	extra->icon = extra->bg_pix = 0;
    }
}

/*----------------------------------------------------------------------------
  \internal
  Deletes the widget extra data.
 ----------------------------------------------------------------------------*/

void QWidget::deleteExtra()
{
    if ( extra ) {				// if exists
	delete [] extra->caption;
	delete [] extra->iconText;
	delete extra->icon;
	delete extra->bg_pix;
	delete extra;
	extra = 0;
    }
}


/*----------------------------------------------------------------------------
  Returns a pointer to the widget with window identifer/handle \e id.

  The window identifier type depends by the underlying window system,
  see qwindefs.h for the actual definition.
  If there is no widget with this identifier, a null pointer is returned.

  \sa wmapper(), id()
 ----------------------------------------------------------------------------*/

QWidget *QWidget::find( WId id )
{
    return mapper ? mapper->find( id ) : 0;
}

/*----------------------------------------------------------------------------
  \fn QWidgetMapper *QWidget::wmapper()
  \internal
  Returns a pointer to the widget mapper.

  The widget mapper is an internal dictionary that is used to map from
  window identifiers/handles to widget pointers.
  \sa find(), id()
 ----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
  \fn WFlags QWidget::getWFlags() const
  \internal
  Returns the widget flags for this this widget.

  Widget flags are internal, not meant for public use.
  \sa testWFlags(), setWFlags(), clearWFlags()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn void QWidget::setWFlags( WFlags f )
  \internal
  Sets the widget flags \e f.

  Widget flags are internal, not meant for public use.
  \sa testWFlags(), getWFlags(), clearWFlags()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn void QWidget::clearWFlags( WFlags f )
  \internal
  Clears the widget flags \e f.

  Widget flags are internal, not meant for public use.
  \sa testWFlags(), getWFlags(), setWFlags()
 ----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
  \fn void QWidget::destroyed()
  This signal is emitted immediately before the widget is destroyed.

  All the widget's children are destroyed immediately after this signal
  is emitted.
 ----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
  \fn WId QWidget::id() const
  Returns the window system identifier of the widget.

  Portable in principle, but if you use it you are probably about to do
  something non-portable. Be careful.

  \sa find()
 ----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
  Returns the GUI style for this widget.

  \sa setStyle(), QApplication::style()
 ----------------------------------------------------------------------------*/

GUIStyle QWidget::style() const
{
    return extra ? extra->guistyle : QApplication::style();
}

/*----------------------------------------------------------------------------
  Sets the GUI style for this widget.  The valid values are listed
  in qglobal.h, but everything except \c MotifStyle is masked out in
  the free linux version.

  \sa style(), QApplication::setStyle()
 ----------------------------------------------------------------------------*/

void QWidget::setStyle( GUIStyle style )
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


/*----------------------------------------------------------------------------
  Enables widget input events.

  An enabled widget receives keyboard and mouse events; a disabled
  widget does not.  Note that an enabled widget receives keyboard
  events only when it is in focus.

  \sa disable(), setEnabled(), isEnabled(), isDisabled(),
  QKeyEvent, QMouseEvent
 ----------------------------------------------------------------------------*/

void QWidget::enable()
{
    if ( testWFlags(WState_Disabled) ) {
	clearWFlags( WState_Disabled );
	update();
    }
}

/*----------------------------------------------------------------------------
  Disables widget input events.

  An enabled widget receives keyboard and mouse events; a disabled
  widget does not.  Note that an enabled widget receives keyboard
  events only when it is in focus.

  \sa enable(), setEnabled(), isEnabled(), isDisabled(),
  QKeyEvent, QMouseEvent
 ----------------------------------------------------------------------------*/

void QWidget::disable()
{
    if ( !testWFlags(WState_Disabled) ) {
	setWFlags( WState_Disabled );
	update();
    }
}

/*----------------------------------------------------------------------------
  Enables widget input events if \e enable is TRUE, otherwise disables
  input events.

  An enabled widget receives keyboard and mouse events; a disabled
  widget does not.  Note that an enabled widget receives keyboard
  events only when it is in focus.

  \sa enable(), disable(), isEnabled(), isDisabled(),
  QKeyEvent, QMouseEvent
 ----------------------------------------------------------------------------*/

void QWidget::setEnabled( bool enable )
{
    if ( enable )
	this->enable();
    else
	disable();
}

/*----------------------------------------------------------------------------
  \fn bool QWidget::isEnabled() const
  Returns TRUE if the widget is enabled, or FALSE if it is disabled.
  \sa enable(), disable(), setEnabled(), isDisabled()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn bool QWidget::isDisabled() const
  Returns TRUE if the widget is disabled, or FALSE if it is enabled.
  \sa enable(), disable(), setEnabled(), isEnabled()
 ----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
  \fn const QRect &QWidget::frameGeometry() const
  Returns the geometry of the widget, relative to its parent and
  including the window frame.
  \sa geometry(), x(), y(), pos()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn const QRect &QWidget::geometry() const
  Returns the geometry of the widget, relative to its parent widget
  and excluding the window frame.
  \sa frameGeometry(), size(), rect()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn int QWidget::x() const
  Returns the x coordinate of the widget, relative to its parent
  widget and including the window frame.
  \sa frameGeometry(), y(), pos()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn int QWidget::y() const
  Returns the y coordinate of the widget, relative to its parent
  widget and including the window frame.
  \sa frameGeometry(), x(), pos()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn QPoint QWidget::pos() const
  Returns the postion of the widget in its parent widget, including
  the window frame.
  \sa frameGeometry(), x(), y()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn QSize QWidget::size() const
  Returns the size of the widget, excluding the window frame.
  \sa geometry(), width(), height()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn int QWidget::width() const
  Returns the width of the widget, excluding the window frame.
  \sa geometry(), height(), size()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn int QWidget::height() const
  Returns the height of the widget, excluding the window frame.
  \sa geometry(), width(), size()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn QRect QWidget::rect() const
  Returns the the internal geometry of the widget, excluding the window frame.
  rect() equals QRect(0,0,width(),height()).
  \sa size()
 ----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
  Returns the bounding rectangle of the widget's children.
 ----------------------------------------------------------------------------*/

QRect QWidget::childrenRect() const
{
    QRect r( 0, 0, 0, 0 );
    if ( !children() )
	return r;
    QObjectListIt it( *children() );		// iterate over all children
    QObject *obj;
    while ( (obj=it.current()) ) {
	++it;
	if ( obj->isWidgetType() ) {
	    if ( r.isNull() )
		r = ((QWidget*)obj)->geometry();
	    else
		r = r.unite( ((QWidget*)obj)->geometry() );
	}
    }
    return r;
}


/*----------------------------------------------------------------------------
  Returns TRUE if a minimum widget size has been set, or FALSE if
  it has not been set.

  If a minimum size has been set, it is returned in \e *w and \e *h.

  Note that while you can set the minimum size for all widgets, it has
  no effect except for top-level widgets.

  \sa setMinimumSize()
 ----------------------------------------------------------------------------*/

bool QWidget::minimumSize( int *w, int *h ) const
{
    if ( extra && extra->minw >= 0 && w && h) {
	*w = extra->minw;
	*h = extra->minh;
	return TRUE;
    }
    return FALSE;
}

/*----------------------------------------------------------------------------
  Returns TRUE if a maximum widget size has been set, or FALSE if
  it has not been set.

  If a mazimum size has been set, it returned in \e *w and \e *h.

  Note that while you can set the minimum size for all widgets, it has
  no effect except for top-level widgets.

  \sa setMaximumSize()
 ----------------------------------------------------------------------------*/

bool QWidget::maximumSize( int *w, int *h ) const
{
    if ( extra && extra->maxw >= 0 && w && h ) {
	*w = extra->maxw;
	*h = extra->maxh;
	return TRUE;
    }
    return FALSE;
}

/*----------------------------------------------------------------------------
  Returns TRUE if a widget size increment has been set, or FALSE if
  it has not been set.

  If a size increment has been set, it returned in \e *w and \e *h.

  \sa setSizeIncrement()
 ----------------------------------------------------------------------------*/

bool QWidget::sizeIncrement( int *w, int *h ) const
{
    if ( extra && extra->incw >= 0 && w && h ) {
	*w = extra->incw;
	*h = extra->inch;
	return TRUE;
    }
    return FALSE;
}


/*----------------------------------------------------------------------------
  Returns the top level widget for this widget.

  A top level widget is an overlapping widget that has no parent widget.
 ----------------------------------------------------------------------------*/

QWidget *QWidget::topLevelWidget() const
{
    QWidget *w = (QWidget *)this;
    QWidget *p = w->parentWidget();
    while ( !w->testWFlags(WType_Modal) && p ) {
	w = p;
	p = p->parentWidget();
    }
    return w;
}


/*----------------------------------------------------------------------------
  \fn const QColor &QWidget::backgroundColor() const

  Returns the background color of this widget.

  The background color is independent of the color group.

  Setting a new palette overwrites the background color.

  \sa setBackgroundColor(), foregroundColor(), colorGroup(), setPalette()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  Returns the foreground color of this widget.

  The foreground color equals <code>colorGroup().foreground()</code>.

  \sa backgroundColor(), colorGroup()
 ----------------------------------------------------------------------------*/

const QColor &QWidget::foregroundColor() const
{
    return colorGroup().foreground();
}


/*----------------------------------------------------------------------------
  Returns the background pixmap, or null if no background pixmap has not
  been set.

  \sa setBackgroundPixmap()
 ----------------------------------------------------------------------------*/

const QPixmap *QWidget::backgroundPixmap() const
{
    return (extra && extra->bg_pix) ? extra->bg_pix : 0;
}



/*----------------------------------------------------------------------------
  Returns the current color group of the widget palette.

  The color group is determined by the state of the widget.

  A disabled widget returns the QPalette::disabled() color group, a
  widget in focus returns the QPalette::active() color group and a
  normal widget returns the QPalette::normal() color group.

  \sa palette(), setPalette()
 ----------------------------------------------------------------------------*/

const QColorGroup &QWidget::colorGroup() const
{
    if ( testWFlags(WState_Disabled) )
	return pal.disabled();
    else if ( qApp->focus_widget == this )
	return pal.active();
    else
	return pal.normal();
}

/*----------------------------------------------------------------------------
  \fn const QPalette &QWidget::palette() const
  Returns the widget palette.
  \sa setPalette(), colorGroup()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  Sets the widget palette to \e p. The widget background color is set to
  <code>colorGroup().background()</code>.

  \sa palette(), colorGroup(), setBackgroundColor()
 ----------------------------------------------------------------------------*/

void QWidget::setPalette( const QPalette &p )
{
    pal = p;
    setBackgroundColor( colorGroup().background() );
    update();
}


/*----------------------------------------------------------------------------
  \fn const QFont &QWidget::font() const

  Returns the font currently set for the widget.

  fontInfo() tells you what font is actually being used.

  \sa setFont(), fontInfo(), fontMetrics()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  Sets the font for the widget.

  The fontInfo() function reports the actual font that is being used by the
  widget.

  This code fragment sets a 12 point helvetica bold font:
  \code
    QFont f("Helvetica", 12, QFont::Bold);
    setFont( f );
  \endcode

  \sa font(), fontInfo(), fontMetrics()
 ----------------------------------------------------------------------------*/

void QWidget::setFont( const QFont &font )
{
    fnt = font;
    fnt.handle();				// force load font
    update();
}


/*----------------------------------------------------------------------------
  \fn QFontMetrics QWidget::fontMetrics() const
  Returns the font metrics for the widget.
  \sa font(), fontInfo(), setFont()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn QFontInfo QWidget::fontInfo() const
  Returns the font info for the widget.
  \sa font(), fontMetrics(), setFont()
 ----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
  Returns the widget cursor.
  \sa setCursor()
 ----------------------------------------------------------------------------*/

const QCursor &QWidget::cursor() const
{
    return curs;
}


/*----------------------------------------------------------------------------
  Returns the widget caption, or null if no caption has been set.
  \sa setCaption(), icon(), iconText()
 ----------------------------------------------------------------------------*/

const char *QWidget::caption() const
{
    return extra ? extra->caption : 0;
}

/*----------------------------------------------------------------------------
  Returns the widget icon pixmap, or null if no icon has been set.
  \sa setIcon(), iconText(), caption()
 ----------------------------------------------------------------------------*/

const QPixmap *QWidget::icon() const
{
    return extra ? extra->icon : 0;
}

/*----------------------------------------------------------------------------
  Returns the widget icon text, or null if no icon text has been set.
  \sa setIconText(), icon(), caption()
 ----------------------------------------------------------------------------*/

const char *QWidget::iconText() const
{
    return extra ? extra->iconText : 0;
}


/*----------------------------------------------------------------------------
  \fn bool QWidget::setMouseTracking( bool enable )
  Enables or disables mouse tracking and returns the previous setting.

  If mouse tracking is disabled (default), the widget only receives
  mouse move events when at least one mouse button is pressed down while
  the mouse is being moved.

  If mouse tracking is enabled, the widget receives mouse move events
  even if no buttons are pressed down.

  \sa mouseMoveEvent()
 ----------------------------------------------------------------------------*/

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


/*----------------------------------------------------------------------------
  Returns TRUE if this widget (not one of its children) has the
  keyboard input focus, otherwise FALSE.

  Equivalent with <code>qApp->focusWidget() == this</code>.

  \sa setFocus(), setAcceptFocus(), QApplication::focusWidget()
 ----------------------------------------------------------------------------*/

bool QWidget::hasFocus() const
{
    return qApp->focusWidget() == this;
}


/*----------------------------------------------------------------------------
  \internal
  Sets the frame rectangle and recomputes the client rectangle.

  The frame rectangle is the geometry of this widget including any
  decorative borders, in its parent's coordinate system.

  The client rectangle is the geometry of just this widget in its
  parent's coordinate system.
 ----------------------------------------------------------------------------*/

void QWidget::setFRect( const QRect &r )
{
    crect.setLeft( crect.left() + r.left() - frect.left() );
    crect.setTop( crect.top() + r.top() - frect.top() );
    crect.setRight( crect.right() + r.right() - frect.right() );
    crect.setBottom( crect.bottom() + r.bottom() - frect.bottom() );
    frect = r;
}

/*----------------------------------------------------------------------------
  \internal
  Sets the client rectangle and recomputes the frame rectangle.

  The client rectangle is the geometry of just this widget in its
  parent's coordinate system.

  The frame rectangle is the geometry of this widget including any
  decorative borders, in its parent's coordinate system.
 ----------------------------------------------------------------------------*/

void QWidget::setCRect( const QRect &r )
{
    frect.setLeft( frect.left() + r.left() - crect.left() );
    frect.setTop( frect.top() + r.top() - crect.top() );
    frect.setRight( frect.right() + r.right() - crect.right() );
    frect.setBottom( frect.bottom() + r.bottom() - crect.bottom() );
    crect = r;
}

/*----------------------------------------------------------------------------
  \fn bool QWidget::acceptFocus() const
  Returns TRUE if the widget accepts keyboard focus events, or FALSE if
  it does not.

  Focus events are initially disabled, so the widget cannot receive
  keyboard input.  Call setAcceptFocus(TRUE) to enable focus events
  and keyboard events.

  \sa setAcceptFocus(), focusInEvent(), focusOutEvent(), keyPressEvent(),
  keyReleaseEvent(), enable(), disable()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  Enables or disables the keyboard input focus events for the widget.

  Focus events are initially disabled. Enabling focus is normally done
  from a widget's constructor. For instance, the QLineEdit constructor
  does setAcceptFocus(TRUE).

  \sa setAcceptFocus(), focusInEvent(), focusOutEvent(), keyPressEvent(),
  keyReleaseEvent(), enable(), disable()
 ----------------------------------------------------------------------------*/

void QWidget::setAcceptFocus( bool enable )
{
    if ( enable )
	setWFlags( WState_AcceptFocus );
    else
	clearWFlags( WState_AcceptFocus );
}


/*----------------------------------------------------------------------------
  Translates the widget coordinate \e pos to a coordinate in the parent widget.

  Same as mapToGlobal() if the widget has no parent.
  \sa mapFromParent()
 ----------------------------------------------------------------------------*/

QPoint QWidget::mapToParent( const QPoint &p ) const
{
    return p + crect.topLeft();
}

/*----------------------------------------------------------------------------
  Translates the parent widget coordinate \e pos to widget coordinates.

  Same as mapFromGlobal() if the widget has no parent.

  \sa mapToParent()
 ----------------------------------------------------------------------------*/

QPoint QWidget::mapFromParent( const QPoint &p ) const
{
    return p - crect.topLeft();
}


/*----------------------------------------------------------------------------
  Closes this widget.

  First it sends the widget a QCloseEvent, then, if the widget did accept
  that, or \e forceKill is TRUE, it deletes the widget and all its children.

  The application is terminated if the main widget is closed.

  \sa closeEvent(), QApplication::setMainWidget(), QApplication::quit()
 ----------------------------------------------------------------------------*/

bool QWidget::close( bool forceKill )
{
    QCloseEvent event;
    bool accept = QApplication::sendEvent( this, &event );
    if ( accept || forceKill ) {
	hide();
	if ( qApp->mainWidget() == this )
	    qApp->quit();
	else
	    delete this;
    }
    return accept;
}

/*----------------------------------------------------------------------------
  \fn bool QWidget::isVisible() const
  Returns TRUE if the widget is visible, or FALSE if the widget is invisible.

  Calling show() makes the widget visible. Calling hide() makes the widget
  invisible.

  A widget is considered visible even if it is obscured by other windows on the
  screen.
 ----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
  Virtual function that adjusts the size of the widget to fit the contents.

  The default implementation adjusts the size to the children rectangle.

  \sa childrenRect()
 ----------------------------------------------------------------------------*/

void QWidget::adjustSize()
{
    QRect r = childrenRect();			// get children rectangle
    if ( r.isNull() )				// probably no widgets
	return;
    resize( r.width()+2*r.x(), r.height()+2*r.y() );
}


/*----------------------------------------------------------------------------
  \fn QWidget *QWidget::parentWidget() const
  Returns a pointer to the parent of this widget, or a null pointer if
  it does not have any parent widget.
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
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
  <dt>WMouseTracking <dd> The widget receives mouse move events.
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
 ----------------------------------------------------------------------------*/


/*****************************************************************************
  QWidget event handling
 *****************************************************************************/


/*----------------------------------------------------------------------------
  This is the main event handler. You may reimplement this function
  in a subclass, but we recommend using one of the specialized event
  handlers instead.

  The main event handler first passes an event through all \link
  QObject::installEventFilter() event filters\endlink that have been
  installed.  If none of the filters intercept the event, it calls one
  of the specialized event handlers.

  Key press/release events are treated differently from other events.
  First event() checks if there exists an \link QAccel accelerator
  \endlink that wants the key press (accelerators do not get key release
  events).  If not, it sends the event to the widget that has the \link
  QApplication::focusWidget() keyboard focus\endlink. If there is no
  widget in focus or the focus widget did not want the key, the event is
  sent to the top level widget.

  This function returns TRUE if it is able to pass the event over to
  someone, or FALSE if nobody wanted the event.

  \sa closeEvent(), focusInEvent(), focusOutEvent(), keyPressEvent(),
  keyReleaseEvent(), mouseDoubleClickEvent(), mouseMoveEvent(),
  mousePressEvent(), mouseReleaseEvent(), moveEvent(), paintEvent(),
  resizeEvent(), QObject::event(), QObject::timerEvent()
 ----------------------------------------------------------------------------*/

bool QWidget::event( QEvent *e )		// receive event(),
{
    if ( eventFilters ) {			// try filters
	if ( activate_filters(e) )		// stopped by a filter
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
	    QKeyEvent *k = (QKeyEvent *)e;
#if 0
	    bool res = FALSE;
	    if ( k->key() == Key_Tab )
		res = focusNextChild();
	    else if ( k->key() == Key_Backtab )
		res = focusPrevChild();
	    if ( res )
		break;
#endif
	    keyPressEvent( k );
	    }
	    break;

	case Event_KeyRelease:
	    keyReleaseEvent( (QKeyEvent*)e );
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

	default:
	    return FALSE;
    }
    return TRUE;
}

/*----------------------------------------------------------------------------
  This event handler can be reimplemented in a subclass to receive
  mouse move events for the widget.

  If mouse tracking is switched off, mouse move events only occur if a
  mouse button is down while the mouse is being moved.  If mouse
  tracking is switched on, mouse move events occur even if no mouse
  button is down.

  The default implementation does nothing.

  \sa setMouseTracking(), mousePressEvent(), mouseReleaseEvent(),
  mouseDoubleClickEvent(), event(),  QMouseEvent
 ----------------------------------------------------------------------------*/

void QWidget::mouseMoveEvent( QMouseEvent * )
{
}

/*----------------------------------------------------------------------------
  This event handler can be reimplemented in a subclass to receive
  mouse press events for the widget.

  The default implementation does nothing.

  If you create new widgets in the mousePressEvent() the
  mouseReleaseEvent() may not end up where you expect, depending on the
  underlying window system (or X-Windows window manager), the widgets'
  location and maybe more.

  \sa mouseReleaseEvent(), mouseDoubleClickEvent(),
  mouseMoveEvent(), event(),  QMouseEvent
 ----------------------------------------------------------------------------*/

void QWidget::mousePressEvent( QMouseEvent * )
{
}

/*----------------------------------------------------------------------------
  This event handler can be reimplemented in a subclass to receive
  mouse release events for the widget.

  The default implementation does nothing.

  \sa mouseReleaseEvent(), mouseDoubleClickEvent(),
  mouseMoveEvent(), event(),  QMouseEvent
 ----------------------------------------------------------------------------*/

void QWidget::mouseReleaseEvent( QMouseEvent * )
{
}

/*----------------------------------------------------------------------------
  This event handler can be reimplemented in a subclass to receive
  mouse double click events for the widget.

  The default implementation generates a normal mouse press event.

  Note that the widgets gets a mousePressEvent() and a mouseReleaseEvent()
  before the mouseDoubleClickEvent().

  \sa mousePressEvent(), mouseReleaseEvent()
  mouseMoveEvent(), event(),  QMouseEvent
 ----------------------------------------------------------------------------*/

void QWidget::mouseDoubleClickEvent( QMouseEvent *e )
{
    mousePressEvent( e );			// try mouse press event
}


/*----------------------------------------------------------------------------
  This event handler can be reimplemented in a subclass to receive
  key press events for the widget.

  A widget must \link setAcceptFocus() accept focus\endlink initially
  and \link hasFocus() have focus\endlink in order to receive a key press
  event.

  If you reimplement this handler, it is very important that you \link
  QKeyEvent ignore()\endlink the press if you do not understand it, so
  that the widget's parent can interpret it.

  The default implementation ignores the event.

  \sa keyReleaseEvent(), QKeyEvent::ignore(), setAcceptFocus(),
  focusInEvent(), focusOutEvent(), event(), QKeyEvent
 ----------------------------------------------------------------------------*/

void QWidget::keyPressEvent( QKeyEvent *e )
{
    e->ignore();
}

/*----------------------------------------------------------------------------
  This event handler can be reimplemented in a subclass to receive
  key release events for the widget.

  A widget must \link setAcceptFocus() accept focus\endlink initially
  and \link hasFocus() have focus\endlink in order to receive a key
  release event.

  If you reimplement this handler, it is very important that you \link
  QKeyEvent ignore()\endlink the release if you do not understand it,
  so that the widget's parent can interpret it.

  The default implementation ignores the event.

  \sa keyPressEvent(), QKeyEvent::ignore(), setAcceptFocus(),
  focusInEvent(), focusOutEvent(), event(), QKeyEvent
 ----------------------------------------------------------------------------*/

void QWidget::keyReleaseEvent( QKeyEvent *e )
{
    e->ignore();
}

/*----------------------------------------------------------------------------
  This event handler can be reimplemented in a subclass to receive
  keyboard focus events (focus received) for the widget.

  A widget must \link setAcceptFocus() accept focus\endlink initially in
  order to receive focus events.

  The default implementation does nothing.

  \sa focusOutEvent(), setAcceptFocus(),
  keyPressEvent(), keyReleaseEvent(), event(), QFocusEvent
 ----------------------------------------------------------------------------*/

void QWidget::focusInEvent( QFocusEvent * )
{
}

/*----------------------------------------------------------------------------
  This event handler can be reimplemented in a subclass to receive
  keyboard focus events (focus lost) for the widget.

  A widget must \link setAcceptFocus() accept focus\endlink initially in
  order to receive focus events.

  The default implementation does nothing.

  \sa focusInEvent(), setAcceptFocus(),
  keyPressEvent(), keyReleaseEvent(), event(), QFocusEvent
 ----------------------------------------------------------------------------*/

void QWidget::focusOutEvent( QFocusEvent * )
{
}

/*----------------------------------------------------------------------------
  This event handler can be reimplemented in a subclass to receive
  widget paint events.	Actually, it more or less \e must be
  reimplemented.

  The default implementation does nothing.

  When the paint event occurs, the update rectangle QPaintEvent::rect()
  normally has been cleared to the background color or pixmap. An
  exception is repaint() with erase=FALSE.

  For many widgets it is sufficient to redraw the entire widget each time,
  but some need to consider the update rectangle to avoid flicker or slow
  update.

  Pixmaps can also be used to implement flicker-free update.

  update() and repaint() can be used to force a paint event.

  \sa event(), repaint(), update(), QPainter, QPixmap, QPaintEvent
 ----------------------------------------------------------------------------*/

void QWidget::paintEvent( QPaintEvent * )
{
}


/*----------------------------------------------------------------------------
  This event handler can be reimplemented in a subclass to receive
  widget move events.  When the widget receives this event, it is
  already at the new position.

  The old position is accessible through QMoveEvent::oldPos().

  The default implementation does nothing.

  \sa resizeEvent(), event(), move(), QMoveEvent
 ----------------------------------------------------------------------------*/

void QWidget::moveEvent( QMoveEvent * )
{
}


/*----------------------------------------------------------------------------
  This event handler can be reimplemented in a subclass to receive
  widget resize events.  When resizeEvent() is called, the widget
  already has its new geometry.

  The old size is accessible through QResizeEvent::oldSize().

  The default implementation does nothing.

  \sa moveEvent(), event(), resize(), QResizeEvent
 ----------------------------------------------------------------------------*/

void QWidget::resizeEvent( QResizeEvent * )
{
}

/*----------------------------------------------------------------------------
  This event handler can be reimplemented in a subclass to receive
  widget close events.

  The default implementation does nothing.

  \sa event(), close(), destroyed(), QCloseEvent
 ----------------------------------------------------------------------------*/

void QWidget::closeEvent( QCloseEvent * )
{
}


#if defined(_WS_MAC_)

/*----------------------------------------------------------------------------
  This special event handler can be reimplemented in a subclass to receive
  raw Macintosh events.

  It must return FALSE to pass the event to Qt, or TRUE to stop the event.

  \warning This function is not portable.

  QApplication::macEventFilter()
 ----------------------------------------------------------------------------*/

bool QWidget::macEvent( MSG * )
{
    return FALSE;
}

#elif defined(_WS_WIN_)

/*----------------------------------------------------------------------------
  This special event handler can be reimplemented in a subclass to receive
  raw Windows events.

  It must return FALSE to pass the event to Qt, or TRUE to stop the event.

  \warning This function is not portable.

  QApplication::winEventFilter()
 ----------------------------------------------------------------------------*/

bool QWidget::winEvent( MSG * )
{
    return FALSE;
}

#elif defined(_WS_PM_)

/*----------------------------------------------------------------------------
  This special event handler can be reimplemented in a subclass to receive
  raw OS/2 Presentation Manager events.

  It must return FALSE to pass the event to Qt, or TRUE to stop the event.

  \warning This function is not portable.

  QApplication::pmEventFilter()
 ----------------------------------------------------------------------------*/

bool QWidget::pmEvent( QMSG * )
{
    return FALSE;
}

#elif defined(_WS_X11_)

/*----------------------------------------------------------------------------
  This special event handler can be reimplemented in a subclass to receive
  raw X-Windows events.

  It must return FALSE to pass the event to Qt, or TRUE to stop the event.

  \warning This function is not portable.

  QApplication::x11EventFilter()
 ----------------------------------------------------------------------------*/

bool QWidget::x11Event( XEvent * )
{
    return FALSE;
}

#endif
