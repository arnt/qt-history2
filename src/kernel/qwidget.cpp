/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qwidget.cpp#125 $
**
** Implementation of QWidget class
**
** Author  : Haavard Nord
** Created : 931031
**
** Copyright (C) 1993-1996 by Troll Tech AS.  All rights reserved.
**
** --------------------------------------------------------------------------
** IMPORTANT NOTE: Widget identifier should only be set with the setWinId()
** function, otherwise widget mapping will not work.
*****************************************************************************/

#include "qobjcoll.h"
#include "qwidget.h"
#include "qwidcoll.h"
#include "qpixmap.h"
#include "qkeycode.h"
#include "qapp.h"

RCSTAG("$Id: //depot/qt/main/src/kernel/qwidget.cpp#125 $")


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

/*----------------------------------------------------------------------------
  Constructs a widget which is a child of \e parent, with the name \e name and
  widget flags set to \e f.

  If \e parent is 0, the new widget becomes a \link isTopLevel() top level
  window\endlink. If \e parent is another widget, this widget becomes a child
  window inside \e parent.

  The \e name is sent to the QObject constructor.

  The widget flags argument \e f is normally 0, but it can be set to
  customize the window frame of a top level widget (i.e. \e parent must be
  zero). To customize the frame, set the \c WStyle_Customize flag OR'ed with
  any of these flags:

  <ul>
  <li> \c WStyle_NormalBorder gives the window a normal border. Cannot
    be combined with \c WStyle_DialogBorder or \c WStyle_NoBorder.
  <li> \c WStyle_DialogBorder gives the window a thin dialog border.
    Cannot be combined with \c WStyle_NormalBorder or \c WStyle_NoBorder.
  <li> \c WStyle_NoBorder gives a borderless window. Note that the
    user cannot move or resize a borderless window via the window system.
    Cannot be combined with \c WStyle_NormalBorder or \c WStyle_DialogBorder.
  <li> \c WStyle_Title gives the window a title bar.
  <li> \c WStyle_SysMenu adds a window system menu.
  <li> \c WStyle_Minimize adds a minimize button.
  <li> \c WStyle_Maximize adds a maximize button.
  <li> \c WStyle_MinMax is equal to <code>(WStyle_Minimize | WStyle_Maximize)
  </code>.
  <li> \c WStyle_Tool makes the window a tool window, usually
    combined with \c WStyle_NoBorder. A tool window is a small window that
    lives for a short time and it is typically used for creating popup
    windows.
  </ul>

  Note that X11 does not necessarily support all style flag combinations. X11
  window managers live their own lives and can only take hints. Win32
  supports all style flags.

  Example:
  \code
    QLabel *toolTip = new QLabel( 0, "myToolTip",
    				  WStyle_Customize | WStyle_NoBorder |
				  WStyle_Tool );
  \endcode

  The widget flags are defined in qwindefs.h (which is included by
  qwidget.h).
 ----------------------------------------------------------------------------*/

QWidget::QWidget( QWidget *parent, const char *name, WFlags f )
    : QObject( parent, name ), QPaintDevice( PDT_WIDGET ),
      pal( *qApp->palette() )			// use application palette
{
    initMetaObject();				// initialize meta object
    isWidget = TRUE;				// is a widget
    winid = 0;					// default attributes
    flags = f;
    extra = 0;					// no extra widget info
    focusChild = 0;				// no child has focus
    create();					// platform-dependent init
#if 0
 // Setting the WDestructiveClose flag deletes the widget instead of
 // hiding it. We don't want to do that any longer.
    if ( (flags & (WType_TopLevel | WType_Modal)) == WType_TopLevel ) {
	flags |= WDestructiveClose;
    }
#endif
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
    if ( testWFlags(WExportFontMetrics) )	// remove references to this
	QFontMetrics::reset( this );
    if ( testWFlags(WExportFontInfo) )		// remove references to this
	QFontInfo::reset( this );
    if ( childObjects ) {			// widget has children
	register QObject *obj = childObjects->first();
	while ( obj ) {				// delete all child objects
#if defined(_WS_WIN_)
	    if ( obj->isWidgetType() ) {	// is child widget
		QWidget *w = (QWidget *)obj;
		w->clearWFlags( WState_Created );
		w->setWinId( 0 );		// Windows destroys children
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

void QWidget::setWinId( WId id )		// set widget identifier
{
    if ( !mapper )				// mapper destroyed
	return;
    if ( winid )
	mapper->remove( winid );
    winid = id;
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
	extra->minw = extra->minh = 0;
	extra->maxw = extra->maxh = QCOORD_MAX;
	extra->incw = extra->inch = 0;
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
  \fn WId QWidget::winId() const
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
  in qwindefs.h.

  \sa style(), styleChange(), QApplication::setStyle()
 ----------------------------------------------------------------------------*/

void QWidget::setStyle( GUIStyle style )
{
    GUIStyle old = this->style();
    createExtra();
    extra->guistyle = style;
    styleChange( old );
}

/*----------------------------------------------------------------------------
  \fn void QWidget::styleChange( GUIStyle oldStyle )

  This virtual function is called from setStyle().  \e oldStyle is the
  previous style; you can get the new style from style().

  Reimplement this function if your widget needs to know when its GUI
  style changes.  You will almost certainly need to update the widget
  using either repaint(TRUE) or update().

  The default implementation calls update().

  \sa setStyle(), style(), repaint(), update()
 ----------------------------------------------------------------------------*/

void QWidget::styleChange( GUIStyle )
{
    update();
}


/*----------------------------------------------------------------------------
  \fn bool QWidget::isTopLevel() const
  Returns TRUE if the widget is a top level widget, otherwise FALSE.

  A top level widget is a widget which usually has a frame and a \link
  setCaption() caption\endlink (title bar). \link isPopup() Popup\endlink
  and \link isDesktop() desktop\endlink widgets are also top level
  widgets. Modal \link QDialog dialog\endlink widgets are the only top
  level widgets that can have \link parentWidget() parent widgets\endlink;
  all other top level widgets have null parents.  Child widgets are the
  opposite of top level widgets.

  \sa topLevelWidget(), isModal(), isPopup(), isDesktop(), parentWidget()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn bool QWidget::isModal() const
  Returns TRUE if the widget is a modal widget, otherwise FALSE.

  A modal widget is also a top level widget.

  \sa isTopLevel(), QDialog
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn bool QWidget::isPopup() const
  Returns TRUE if the widget is a popup widget, otherwise FALSE.

  A popup widget is created by specifying the widget flag \c WType_Popup
  to the widget constructor.

  A popup widget is also a top level widget.

  \sa isTopLevel()
 ----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
  \fn bool QWidget::isDesktop() const
  Returns TRUE if the widget is a desktop widget, otherwise FALSE.

  A desktop widget is also a top level widget.

  \sa isTopLevel(), QApplication::desktop()
 ----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
  \fn bool QWidget::isEnabled() const
  Returns TRUE if the widget is enabled, or FALSE if it is disabled.
  \sa setEnabled()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  Enables widget input events if \e enable is TRUE, otherwise disables
  input events.

  An enabled widget receives keyboard and mouse events; a disabled
  widget does not.  Note that an enabled widget receives keyboard
  events only when it is in focus.

  \sa isEnabled(), QKeyEvent, QMouseEvent
 ----------------------------------------------------------------------------*/

void QWidget::setEnabled( bool enable )
{
    if ( enable ) {
	if ( testWFlags(WState_Disabled) ) {
	    clearWFlags( WState_Disabled );
	    repaint();
	}
    } else {
	if ( !testWFlags(WState_Disabled) ) {
	    setWFlags( WState_Disabled );
	    repaint();
	}
    }
}


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
  Returns the minimum widget size.

  The widget cannot be resized to a smaller size than the minimum widget
  size.

  \sa setMinimumSize(), maximumSize(), sizeIncrement()
 ----------------------------------------------------------------------------*/

QSize QWidget::minimumSize() const
{
    return extra ? QSize(extra->minw,extra->minh) : QSize(0,0);
}

/*----------------------------------------------------------------------------
  Returns the maximum widget size.

  The widget cannot be resized to a larger size than the maximum widget
  size.

  \sa setMaximumSize(), minimumSize(), sizeIncrement()
 ----------------------------------------------------------------------------*/

QSize QWidget::maximumSize() const
{
    return extra ? QSize(extra->maxw,extra->maxh)
		 : QSize(QCOORD_MAX,QCOORD_MAX);
}

/*----------------------------------------------------------------------------
  Returns the widget size increment.

  \sa setSizeIncrement(), minimumSize(), maximumSize()
 ----------------------------------------------------------------------------*/

QSize QWidget::sizeIncrement() const
{
    return extra ? QSize(extra->incw,extra->inch) : QSize(0,0);
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
  Returns the top level widget for this widget.

  A top level widget is an overlapping widget. It usually has no parent.
  Modal \link QDialog dialog widgets\endlink are the only top level
  widgets that can have parent widgets.

  \sa isTopLevel()
 ----------------------------------------------------------------------------*/

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


/*----------------------------------------------------------------------------
  \fn const QColor &QWidget::backgroundColor() const

  Returns the background color of this widget.

  The background color is independent of the color group.

  Setting a new palette overwrites the background color.

  \sa setBackgroundColor(), foregroundColor(), colorGroup(), palette()
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
  \fn void QWidget::backgroundColorChange( const QColor &oldBackgroundColor )

  This virtual function is called from setBackgroundColor().
  \e oldBackgroundColor is the previous background color; you can get the new
  background color from backgroundColor().

  Reimplement this function if your widget needs to know when its
  background color changes.  You will almost certainly need to update the
  widget using either repaint(TRUE) or update().

  The default implementation calls update().

  \sa setBackgroundColor(), backgroundColor(), setPalette(), repaint(),
  update()
 ----------------------------------------------------------------------------*/

void QWidget::backgroundColorChange( const QColor & )
{
    update();
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
  \fn void QWidget::backgroundPixmapChange( const QPixmap & oldBackgroundPixmap )

  This virtual function is called from setBackgroundPixmap().
  \e oldBackgroundPixmap is the previous background pixmap; you can get the
  new background pixmap from backgroundPixmap().

  Reimplement this function if your widget needs to know when its
  background pixmap changes.  You will almost certainly need to update the
  widget using either repaint(TRUE) or update().

  The default implementation calls update().

  \sa setBackgroundPixmap(), backgroundPixmap(), repaint(), update()
 ----------------------------------------------------------------------------*/

void QWidget::backgroundPixmapChange( const QPixmap & )
{
    update();
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

  \sa palette(), paletteChange(), colorGroup(), setBackgroundColor()
 ----------------------------------------------------------------------------*/

void QWidget::setPalette( const QPalette &p )
{
    QPalette old = pal;
    pal = p;
    setBackgroundColor( colorGroup().background() );
    paletteChange( old );
}

/*----------------------------------------------------------------------------
  \fn void QWidget::paletteChange( const QPalette &oldPalette )

  This virtual function is called from setPalette().  \e oldPalette is the
  previous palette; you can get the new palette from palette().

  Reimplement this function if your widget needs to know when its palette
  changes.  You will almost certainly need to update the widget using
  either repaint(TRUE) or update().

  The default implementation calls update().

  \sa setPalette(), palette(), repaint(), update()
 ----------------------------------------------------------------------------*/

void QWidget::paletteChange( const QPalette & )
{
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

  \sa font(), fontChange(), fontInfo(), fontMetrics()
 ----------------------------------------------------------------------------*/

void QWidget::setFont( const QFont &font )
{
    QFont old = fnt;
    fnt = font;
    fnt.handle();				// force load font
    fontChange( old );
}


/*----------------------------------------------------------------------------
  \fn void QWidget::fontChange( const QFont &oldFont )

  This virtual function is called from setFont().  \e oldFont is the
  previous font; you can get the new font from font().

  Reimplement this function if your widget needs to know when its font
  changes.  You will almost certainly need to update the widget using
  either repaint(TRUE) or update().

  The default implementation calls update().

  \sa setFont(), font(), repaint(), update()
 ----------------------------------------------------------------------------*/

void QWidget::fontChange( const QFont & )
{
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
  \fn bool QWidget::hasMouseTracking() const
  Returns TRUE if mouse tracking is enabled for this widget, or FALSE
  if mouse tracking is disabled.
  \sa setMouseTracking()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn void QWidget::setMouseTracking( bool enable )
  Enables mouse tracking if \e enable is TRUE, or disables it if \e enable
  is FALSE.

  If mouse tracking is disabled (default), this widget only receives
  mouse move events when at least one mouse button is pressed down while
  the mouse is being moved.

  If mouse tracking is enabled, this widget receives mouse move events
  even if no buttons are pressed down.

  \sa hasMouseTracking(), mouseMoveEvent()
 ----------------------------------------------------------------------------*/

#if !defined(_WS_X11_)
void QWidget::setMouseTracking( bool enable )
{
    if ( enable )
	setWFlags( WState_TrackMouse );
    else
	clearWFlags( WState_TrackMouse );
    return;
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
  \fn bool QWidget::isUpdatesEnabled() const
  Returns TRUE if updates are enabled, otherwise FALSE.
  \sa setUpdatesEnabled()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  Enables widget updates if \e enable is TRUE, or disables widget updates
  if \e enable is FALSE.

  Calling update() and repaint() has no effect if updates are disabled.
  Paint events from the window system are processed as normally even if
  updates are disabled.

  This function is normally used to disable updates for a short period of
  time, for instance to avoid screen flicker during large changes.

  Example:
  \code
    setUpdatesEnabled( FALSE );
    bigVisualChanges();
    setUpdatesEnabled( TRUE );
    repaint();
  \endcode

  \sa isUpdatesEnabled(), update(), repaint(), paintEvent()
 ----------------------------------------------------------------------------*/

void QWidget::setUpdatesEnabled( bool enable )
{
    if ( enable )
	clearWFlags( WState_BlockUpdates );
    else
	setWFlags( WState_BlockUpdates );
}


/*----------------------------------------------------------------------------
  Closes this widget. Returns TRUE if the widget was closed, otherwise
  FALSE.

  First it sends the widget a QCloseEvent. The widget is \link hide()
  hidden\endlink if it \link QCloseEvent::accept() accepts\endlink the
  close event. The default implementation of QWidget::closeEvent()
  accepts the close event.

  If \e forceKill is TRUE, the widget is deleted whether it accepts the
  close event or not.

  The application is \link QApplication::quit() terminated\endlink when
  the \link QApplication::setMainWidget() main widget\endlink is closed.

  \sa closeEvent(), QCloseEvent, hide(), QApplication::quit(),
  QApplication::setMainWidget()
 ----------------------------------------------------------------------------*/

bool QWidget::close( bool forceKill )
{
    WId	 id	= winId();
    bool isMain = qApp->mainWidget() == this;
    QCloseEvent e;
    bool accept = QApplication::sendEvent( this, &e );
    if ( !QWidget::find(id) ) {			// widget was deleted
	if ( isMain )
	    qApp->quit();
	return TRUE;
    }
    if ( accept || forceKill ) {
	accept = TRUE;
	hide();
	if ( isMain )
	    qApp->quit();
	else if ( forceKill || testWFlags(WDestructiveClose) )
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

  Widget state flags:
  <dl compact>
  <dt>WState_Created<dd> The widget has a valid winId().
  <dt>WState_Disabled<dd> Disables mouse and keyboard events.
  <dt>WState_Visible<dd> show() has been called.
  <dt>WState_DoHide<dd> hide() has been called before first show().
  <dt>WState_AcceptFocus<dd> The widget can take keyboard focus.
  <dt>WState_TrackMouse<dd> Mouse tracking is enabled.
  <dt>WState_BlockUpdates<dd> Repaints and updates are disabled.
  <dt>WState_PaintEvent<dd> Currently processing a paint event.
  </dl>

  Widget type flags:
  <dl compact>
  <dt>WType_TopLevel<dd> Top level widget (not a child).
  <dt>WType_Modal<dd> Modal widget, implies \c WType_TopLevel.
  <dt>WType_Popup<dd> Popup widget, implies \c WType_TopLevel.
  <dt>WType_Desktop<dd> Desktop widget (root window), implies
	\c WType_TopLevel.
  </dl>

  Window style flags (for top level widgets):
  <dl compact>
  <dt>WStyle_Customize<dd> Custom window style (for top level widgets).
  <dt>WStyle_NormalBorder<dd> Normal window border.
  <dt>WStyle_DialogBorder<dd> Thin dialog border.
  <dt>WStyle_NoBorder<dd> No window border.
  <dt>WStyle_Title<dd> The window has a title.
  <dt>WStyle_SysMenu<dd> The window has a system menu
  <dt>WStyle_Minimize<dd> The window has a minimize box.
  <dt>WStyle_Maximize<dd> The window has a maximize box.
  <dt>WStyle_MinMax<dd> Equals (\c WStyle_Minimize | \c WStyle_Maximize).
  <dt>WStyle_Tool<dd> The window is a tool window.
  </dl>

  Misc. flags:
  <dl compact>
  <dt>WCursorSet<dd> Flags that a cursor has been set.
  <dt>WDestructiveClose<dd> The widget is deleted when its closed.
  <dt>WPaintDesktop<dd> The widget wants desktop paint events.
  <dt>WPaintUnclipped<dd> Paint without clipping child widgets.
  <dt>WPaintClever<dd> The widget wants every update rectangle.
  <dt>WConfigPending<dd> Config (resize,move) event pending.
  <dt>WResizeNoErase<dd> Widget resizing should not erase the widget.
  <dt>WRecreated<dd> The widet has been recreated.
  <dt>WExportFontMetrics<dd> Somebody refers the font's metrics.
  <dt>WExportFontInfo<dd> Somebody refers the font's info.
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
  event() checks for TAB and shift-TAB and tries to move the focus
  appropriately.  If there is no widget to move the focus to (or the
  key press is not TAB or shift-TAB), event() calls keyPressEvent().

  This function returns TRUE if it is able to pass the event over to
  someone, or FALSE if nobody wanted the event.

  \sa closeEvent(), focusInEvent(), focusOutEvent(), enterEvent(),
  keyPressEvent(), keyReleaseEvent(), leaveEvent(),
  mouseDoubleClickEvent(), mouseMoveEvent(), mousePressEvent(),
  mouseReleaseEvent(), moveEvent(), paintEvent(),
  resizeEvent(), QObject::event(), QObject::timerEvent()
 ----------------------------------------------------------------------------*/

bool QWidget::event( QEvent *e )
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
	    bool res = FALSE;
	    if ( k->key() == Key_Tab )
		res = focusNextChild();
	    else if ( k->key() == Key_Backtab )
		res = focusPrevChild();
	    if ( res )
		break;
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

	case Event_Enter:
	    enterEvent( e );
	    break;

	case Event_Leave:
	    leaveEvent( e );
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
  mouse button is down while the mouse is being moved.	If mouse
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

  The default implementation calls repaint() since the widget's \link
  QColorGroup color group\endlink changes from normal to active.  You
  may want to call repaint(FALSE) to reduce flicker in any reimplementation.

  \sa focusOutEvent(), setAcceptFocus(),
  keyPressEvent(), keyReleaseEvent(), event(), QFocusEvent
 ----------------------------------------------------------------------------*/

void QWidget::focusInEvent( QFocusEvent * )
{
    repaint();
}

/*----------------------------------------------------------------------------
  This event handler can be reimplemented in a subclass to receive
  keyboard focus events (focus lost) for the widget.

  A widget must \link setAcceptFocus() accept focus\endlink initially in
  order to receive focus events.

  The default implementation calls repaint() since the widget's \link
  QColorGroup color group\endlink changes from active to normal.  You
  may want to call repaint(FALSE) to reduce flicker in any reimplementation.

  \sa focusInEvent(), setAcceptFocus(),
  keyPressEvent(), keyReleaseEvent(), event(), QFocusEvent
 ----------------------------------------------------------------------------*/

void QWidget::focusOutEvent( QFocusEvent * )
{
    repaint();
}

/*----------------------------------------------------------------------------
  This event handler can be reimplemented in a subclass to receive
  widget enter events.

  An event is sent to the widget when the mouse cursor enters the widget.

  The default implementation does nothing.

  \sa leaveEvent(), mouseMoveEvent(), event()
 ----------------------------------------------------------------------------*/

void QWidget::enterEvent( QEvent * )
{
}

/*----------------------------------------------------------------------------
  This event handler can be reimplemented in a subclass to receive
  widget leave events.

  A leave event is sent to the widget when the mouse cursor leaves
  the widget.

  The default implementation does nothing.

  \sa enterEvent(), mouseMoveEvent(), event()
 ----------------------------------------------------------------------------*/

void QWidget::leaveEvent( QEvent * )
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
  widget resize events.	 When resizeEvent() is called, the widget
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

  The default implementation calls e->accept(), which hides this widget.
  See the QCloseEvent documentation for more details.

  \sa event(), hide(), close(), QCloseEvent
 ----------------------------------------------------------------------------*/

void QWidget::closeEvent( QCloseEvent *e )
{
    e->accept();
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
