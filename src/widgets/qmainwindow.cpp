/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qmainwindow.cpp#323 $
**
** Implementation of QMainWindow class
**
** Created : 980312
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
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

#include "qmainwindow.h"
#ifndef QT_NO_MAINWINDOW

#include "qtimer.h"
#include "qlayout.h"
#include "qobjectlist.h"
#include "qintdict.h"
#include "qapplication.h"
#include "qptrlist.h"
#include "qmap.h"
#include "qcursor.h"
#include "qpainter.h"
#include "qmenubar.h"
#include "qpopupmenu.h"
#include "qtoolbar.h"
#include "qstatusbar.h"
#include "qscrollview.h"
#include "qtooltip.h"
#include "qdatetime.h"
#include "qwhatsthis.h"
#include "qbitmap.h"
#include "qdockarea.h"
#include "qstringlist.h"
#if defined(QT_ACCESSIBILITY_SUPPORT)
#include "qaccessiblewidget.h"
#endif

/* QMainWindowLayout, respects widthForHeight layouts (like the left
  and right docks are)
*/

class QMainWindowLayout : public QLayout
{
    Q_OBJECT

public:
    QMainWindowLayout( QLayout* parent = 0 );
    ~QMainWindowLayout() {}

    void addItem( QLayoutItem * );
    void setLeftDock( QDockArea *l );
    void setRightDock( QDockArea *r );
    void setCentralWidget( QWidget *w );
    bool hasHeightForWidth() const { return FALSE; }
    QSize sizeHint() const;
    QSize minimumSize() const;
    QLayoutIterator iterator();
    QSizePolicy::ExpandData expanding() const { return QSizePolicy::BothDirections; }
    void invalidate() {}

protected:
    void setGeometry( const QRect &r ) {
	QLayout::setGeometry( r );
	layoutItems( r );
    }

private:
    int layoutItems( const QRect&, bool testonly = FALSE );
    QDockArea *left, *right;
    QWidget *central;

};

QSize QMainWindowLayout::sizeHint() const
{
    if ( !left && !right && !central )
	return QSize( 0, 0 );

    int w = 0, h = 0;
    if ( left ) {
	w = QMAX( w, left->sizeHint().width() );
	h = QMAX( h, left->sizeHint().height() );
    }
    if ( right ) {
	w = QMAX( w, right->sizeHint().width() );
	h = QMAX( h, right->sizeHint().height() );
    }
    if ( central ) {
	w = QMAX( w, central->sizeHint().width() );
	h = QMAX( h, central->sizeHint().height() );
    }

    return QSize( w, h );
}

QSize QMainWindowLayout::minimumSize() const
{
    if ( !left && !right && !central )
	return QSize( 0, 0 );

    int w = 0, h = 0;
    if ( left ) {
	w += left->minimumSize().width();
	h = QMAX( h, left->minimumSize().height() );
    }
    if ( right ) {
	w += right->minimumSize().width();
	h = QMAX( h, right->minimumSize().height() );
    }
    if ( central ) {
	QSize min = central->minimumSize().isNull() ?
		    central->minimumSizeHint() : central->minimumSize();
	w += min.width();
	h = QMAX( h, min.height() );
    }

    return QSize( w, h );
}

QMainWindowLayout::QMainWindowLayout( QLayout* parent )
    : QLayout( parent ), left( 0 ), right( 0 ), central( 0 )
{
}

void QMainWindowLayout::setLeftDock( QDockArea *l )
{
    left = l;
}

void QMainWindowLayout::setRightDock( QDockArea *r )
{
    right = r;
}

void QMainWindowLayout::setCentralWidget( QWidget *w )
{
    central = w;
}

int QMainWindowLayout::layoutItems( const QRect &r, bool testonly )
{
    if ( !left && !central && !right )
	return 0;

    int wl = 0, wr = 0;
    if ( left )
	wl = ( (QDockAreaLayout*)left->QWidget::layout() )->widthForHeight( r.height() );
    if ( right )
	wr = ( (QDockAreaLayout*)right->QWidget::layout() )->widthForHeight( r.height() );
    int w = r.width() - wr - wl;
    if ( w < 0 )
	w = 0;

    if ( !testonly ) {
	QRect g( geometry() );
	if ( left )
	    left->setGeometry( QRect( g.x(), g.y(), wl, r.height() ) );
	if ( right )
	    right->setGeometry( QRect( g.x() + g.width() - wr, g.y(), wr, r.height() ) );
	if ( central )
	    central->setGeometry( g.x() + wl, g.y() + 2, w, r.height() - 2 );
    }

    w = wl + wr;
    if ( central )
	w += central->minimumSize().width();
    return w;
}

void QMainWindowLayout::addItem( QLayoutItem * /*item*/ )
{
}


QLayoutIterator QMainWindowLayout::iterator()
{
    return 0;
}


/*
  QHideToolTip and QHideDock - minimized dock
*/

class QHideToolTip : public QToolTip
{
public:
    QHideToolTip( QWidget *parent ) : QToolTip( parent ) {}

    void maybeTip( const QPoint &pos );
};


class QHideDock : public QWidget
{
    Q_OBJECT

public:
    QHideDock( QMainWindow *parent ) : QWidget( parent, "qt_hide_dock" ) {
	hide();
	setFixedHeight( style().toolBarHandleExtent() );
	pressedHandle = -1;
	pressed = FALSE;
	setMouseTracking( TRUE );
	win = parent;
	tip = new QHideToolTip( this );
    }
    ~QHideDock() { delete tip; }

protected:
    void paintEvent( QPaintEvent *e ) {
	if ( !children() || children()->isEmpty() )
	    return;
	QPainter p( this );
	p.setClipRegion( e->rect() );
	p.fillRect( e->rect(), colorGroup().brush( QColorGroup::Background ) );
	int x = 0;
	int i = -1;
	QObjectListIt it( *children() );
	QObject *o;
	while ( ( o = it.current() ) ) {
	    ++it;
	    ++i;
	    if ( !o->inherits( "QDockWindow" ) )
		continue;
	    if ( !( (QDockWindow*)o )->isVisible() )
		continue;
	    style().drawToolBarHandle( &p, QRect( x, 0, 30, 10 ), Qt::Vertical,
				       i == pressedHandle, colorGroup(), TRUE );
	    x += 30;
	}
    }

    void mousePressEvent( QMouseEvent *e ) {
	pressed = TRUE;
	if ( !children() || children()->isEmpty() )
	    return;
	mouseMoveEvent( e );
	pressedHandle = -1;

	if ( e->button() == RightButton && win->isDockMenuEnabled() ) {
	    qDebug( "todo: hidedock menu" );
	} else {
	    mouseMoveEvent( e );
	}
    }

    void mouseMoveEvent( QMouseEvent *e ) {
	if ( !children() || children()->isEmpty() )
	    return;
	if ( !pressed )
	    return;
	int x = 0;
	int i = -1;
	if ( e->y() >= 0 && e->y() <= height() ) {
	    QObjectListIt it( *children() );
	    QObject *o;
	    while ( ( o = it.current() ) ) {
		++it;
		++i;
		if ( !o->inherits( "QDockWindow" ) )
		    continue;
		if ( !( (QDockWindow*)o )->isVisible() )
		    continue;
		if ( e->x() >= x && e->x() <= x + 30 ) {
		    int old = pressedHandle;
		    pressedHandle = i;
		    if ( pressedHandle != old )
			repaint( TRUE );
		    return;
		}
		x += 30;
	    }
	}
	int old = pressedHandle;
	pressedHandle = -1;
	if ( old != -1 )
	    repaint( TRUE );
    }

    void mouseReleaseEvent( QMouseEvent *e ) {
	pressed = FALSE;
	if ( pressedHandle == -1 )
	    return;
	if ( !children() || children()->isEmpty() )
	    return;
	if ( e->button() == LeftButton ) {
	    if ( e->y() >= 0 && e->y() <= height() ) {
		QObject *o = ( (QObjectList*)children() )->at( pressedHandle );
		if ( o && o->inherits( "QDockWindow" ) ) {
		    QDockWindow *dw = (QDockWindow*)o;
		    dw->show();
		    dw->dock();
		}
	    }
	}
	pressedHandle = -1;
	repaint( FALSE );
    }

    void childEvent( QChildEvent *e ) {
	QWidget::childEvent( e );
	if ( e->type() == QEvent::ChildInserted )
	    e->child()->installEventFilter( this );
	else
	    e->child()->removeEventFilter( this );
	updateState();
    }

    bool eventFilter( QObject *o, QEvent *e ) {
	if ( o == this || !o->isWidgetType() )
	    return QWidget::eventFilter( o, e );
	if ( e->type() == QEvent::Hide ||
	     e->type() == QEvent::Show )
	    updateState();
	return QWidget::eventFilter( o, e );
    }

    void updateState() {
	bool visible = TRUE;
	if ( !children() || children()->isEmpty() ) {
	    visible = FALSE;
	} else {
	    QObjectListIt it( *children() );
	    QObject *o;
	    while ( ( o = it.current() ) ) {
		++it;
		if ( !o->inherits( "QDockWindow" ) )
		    continue;
		if ( !( (QDockWindow*)o )->isVisible() )
		    continue;
		visible = TRUE;
		break;
	    }
	}

	if ( visible )
	    show();
	else
	    hide();
	win->triggerLayout( FALSE );
	update();
    }

private:
    QMainWindow *win;
    int pressedHandle;
    bool pressed;
    QHideToolTip *tip;

    friend class QHideToolTip;

};

void QHideToolTip::maybeTip( const QPoint &pos )
{
    if ( !parentWidget() )
	return;
    QHideDock *dock = (QHideDock*)parentWidget();

    if ( !dock->children() || dock->children()->isEmpty() )
	return;
    QObjectListIt it( *dock->children() );
    QObject *o;
    int x = 0;
    while ( ( o = it.current() ) ) {
	++it;
	if ( !o->inherits( "QDockWindow" ) )
	    continue;
	if ( !( (QDockWindow*)o )->isVisible() )
	    continue;
	if ( pos.x() >= x && pos.x() <= x + 30 ) {
	    QDockWindow *dw = (QDockWindow*)o;
	    if ( !dw->caption().isEmpty() )
		tip( QRect( x, 0, 30, dock->height() ), dw->caption() );
	    return;
	}
	x += 30;
    }
}




/*
 QMainWindowPrivate - private variables of QMainWindow
*/

class QMainWindowPrivate
{
public:
    QMainWindowPrivate()
	:  mb(0), sb(0), ttg(0), mc(0), tll(0), ubp( FALSE ), utl( FALSE ),
	   justify( FALSE ), movable( TRUE ), opaque( FALSE ), dockMenu( TRUE )
    {
	docks.insert( Qt::Top, TRUE );
	docks.insert( Qt::Bottom, TRUE );
	docks.insert( Qt::Left, TRUE );
	docks.insert( Qt::Right, TRUE );
	docks.insert( Qt::Minimized, TRUE );
	docks.insert( Qt::TornOff, TRUE );
    }

    ~QMainWindowPrivate()
    {
    }

#ifndef QT_NO_MENUBAR
    QMenuBar * mb;
#else
    QWidget * mb;
#endif
    QStatusBar * sb;
    QToolTipGroup * ttg;

    QWidget * mc;

    QBoxLayout * tll;

    bool ubp;
    bool utl;
    bool justify;

    bool movable;
    bool opaque;

    QDockArea *topDock, *bottomDock, *leftDock, *rightDock;

    QPtrList<QDockWindow> dockWindows;
    QMap<Qt::Dock, bool> docks;
    QStringList disabledDocks;
    bool dockMenu;
    QHideDock *hideDock;

    QGuardedPtr<QPopupMenu> rmbMenu, tbMenu, dwMenu;
    QMap<QDockWindow*, bool> appropriate;
    QMap<QPopupMenu*, QMainWindow::DockWindows> dockWindowModes;

};



/*! \class QMainWindow qmainwindow.h

  \brief The QMainWindow class provides a main application window, with
  a menu bar, dock windows (e.g. for toolbars), and a status bar.

  \ingroup application

    Main windows are most often used to provide menus, toolbars and a
    status bar around a large central widget, such as a text edit or
    drawing canvas. QMainWindow is usually subclassed since this makes
    it easier to encapsulate the central widget, menus and toolbars as
    well as the window's state. Subclassing makes it possible to create
    the slots that are called when the user clicks menu items or toolbar
    buttons. You can also create main windows using <i>Qt Designer</i>.
    We'll briefly review adding menu items and toolbar buttons then
    describe the facilities of QMainWindow itself.

    \code
    QMainWindow *mw = new QMainWindow;
    QTextEdit *edit = new QTextEdit( mw, "editor" );
    edit->setFocus();
    mw->setCaption( "Main Window" );
    mw->setCentralWidget( edit );
    mw->show();
    \endcode

    QMainWindows may be created in their own right as shown above.  The
    central widget is set with setCentralWidget(). Popup menus can be added
    to the default menu bar, widgets can be added to the status bar,
    toolbars and dock windows can be added to any of the dock areas.

    \walkthrough application/main.cpp
    \skipto ApplicationWindow
    \printuntil show

    In the extract above ApplicationWindow is a subclass of QMainWindow
    that we must write for ourselves; this is the usual approach to
    using QMainWindow. (The source for the extracts in this description
    are taken from \l application/main.cpp, \l
    application/application.cpp, \l action/main.cpp,
    and \l action/application.cpp )

    When subclassing we add the menu items and toolbars in the
    subclass's constructor. If we've created a QMainWindow instance
    directly we can add menu items and toolbars just as easily by
    passing the QMainWindow instance as the parent instead of the \e
    this pointer.

    \walkthrough application/application.cpp
    \skipto help = new
    \printuntil about

    Here we've added a new menu with one menu item. The menu has been
    inserted into the menu bar that QMainWindow provides by default and
    which is accessible through the menuBar() function. The slot will be
    called when the menu item is clicked.

    \walkthrough application/application.cpp
    \skipto fileTools
    \printuntil setLabel
    \skipto QToolButton
    \printuntil open file

    This extract shows the creation of a toolbar with one toolbar
    button. QMainWindow supplies four dock areas for toolbars. When a
    toolbar is created as a child of a QMainWindow (or derived class)
    instance it will be placed in a dock area (the \c Top dock area by
    default). The slot will be called when the toolbar button is
    clicked. Any dock window can be added to a dock area either using
    addDockWindow(), or by creating a dock window with the QMainWindow
    as the parent.

    \walkthrough application/application.cpp
    \skipto editor
    \printuntil statusBar

    Having created the menus and toolbar we create an instance of the
    large central widget, give it the focus and set it as the main
    window's central widget. In the example we've also set the status
    bar, accessed via the statusBar() function, to an initial message
    which will be displayed for two seconds. Note that you can add
    additional widgets to the status bar, for example labels, to show
    further status information. See the QStatusBar documentation for
    details, particularly the addWidget() function.

    Often we want to synchronize a toolbar button with a menu item.
    For example, if the user clicks a 'bold' toolbar button we want the
    'bold' menu item to be checked. This synchronization can be achieved
    automatically by creating actions and adding the actions to the
    toolbar and menu.

    \walkthrough action/application.cpp
    \skipto QAction * fileOpen
    \printline
    \skipto fileOpenAction
    \printuntil choose

    Here we create an action with an icon which will be used in any menu
    and toolbar that the action is added to. We've also given the action
    a menu name, '&Open', and a keyboard shortcut. The connection that we
    have made will be used when the user clicks either the menu item \e
    or the toolbar button.

    \walkthrough action/application.cpp
    \skipto QPopupMenu * file
    \printuntil menuBar
    \skipto fileOpen
    \printline

    The extract above shows the creation of a popup menu. We add the menu
    to the QMainWindow's menu bar and add our action.

    \walkthrough action/application.cpp
    \skipto QToolBar * fileTool
    \printuntil OpenAction

    Here we create a new toolbar as a child of the QMainWindow and add
    our action to the toolbar.

    We'll now explore the functionality offered by QMainWindow.

    The main window will take care of the dock areas, and the geometry
    of the central widget, but all other aspects of the central widget
    are left to you. QMainWindow automatically detects the creation of a
    menu bar or status bar if you specify the QMainWindow as parent, or
    you can use the provided menuBar() and statusBar() functions.
    The functions menuBar() and statusBar() create a suitable widget if one doesn't
    exist, and update the window's layout to make space.

    QMainWindow provides a QToolTipGroup connected to the status bar.
    The function toolTipGroup() provides access to the default QToolTipGroup.
    It isn't possible to set a different tool tip group.

    New dock windows and toolbars can be added to a QMainWindow using
    addDockWindow(). Dock windows can be moved using moveDockWindow() and
    removed with removeDockWindow(). QMainWindow allows default dock
    window (toolbar) docking in all its dock areas (top, left, right,
    bottom).  You can use setDockEnabled() to enable and disable docking
    areas for dock windows. When adding or moving dock windows you can
    specify their 'edge' (dock area). The currently available edges are:
    \c Top, \c Left, \c Right, \c Bottom, \c Minimized (effectively a
    'hidden' dock area) and \c TornOff (floating). See \l Qt::Dock for
    an explanation of these areas. Note that the *ToolBar functions are
    included for backward compatibility, all new code should use the
    *DockWindow functions. QToolbar is a subclass of QDockWindow so all
    functions that work with dock windows work on toolbars in the same
    way.  If the user minimizes a dock window by clicking the dock window's
    window handle then the dock window is moved to the \c Minimized dock
    area.  If the user clicks the close button, then the dock window is
    hidden and can only be shown again by using the
    <a href="#dwm">dock window menu</a>.

    Some functions change the appearance of a QMainWindow globally:
    <ul>
    <li>QDockWindow::setHorizontalStretchable() and
    QDockWindow::setVerticalStretchable() are used to make specific dock
    windows or toolbars stretchable.
    <li>setUsesBigPixmaps() is used to set whether tool buttons should
    draw small or large pixmaps (see QIconSet for more information).
    <li>setUsesTextLabel() is used to set whether tool buttons
    should display a textual label in addition to pixmaps
    (see QToolButton for more information).
    </ul>

    The user can drag dock windows into any enabled docking area. Dock
    windows can also be dragged \e within a docking area, for example to
    rearrange the order of some toolbars. Dock windows can also be
    dragged outside any docking area (undocked or 'floated'). Being
    able to drag dock windows can be enabled (the default) and disabled
    using setDockWindowsMovable(). <a name="dwm">If the user clicks the
    close button on a floating dock window then the dock window will
    disappear.</a> To get the dock window back the user must right click
    a dock area, to pop up the dock window menu, then click the name of
    the dock window they want to restore. Visible dock windows have a
    tick by their name in the dock window menu.
    The dock window menu is created automatically as required by
    createDockWindowMenu(). Since it may not always be appropriate for a
    dock window to appear on this menu the setAppropriate() function is
    used to inform the main window whether or not the dock window menu
    should include a particular dock window. Double clicking a dock
    window handle (usually on the left-hand side of the dock
    window) undocks (floats) the dock window. Double clicking a floating
    dock window's titlebar will dock the floating dock window.

    The \c Minimized edge is a hidden dock area. If this dock area is
    enabled the user can hide (minimize) a dock window or show (restore)
    a minimized dock window by clicking the dock window handle. If the
    user hovers the mouse cursor over one of the handles, the caption of
    the dock window is displayed in a tool tip (see
    QDockWindow::caption() or QToolBar::label()), so if you enable the
    \c Minimized dock area, it is best to specify a meaningful caption
    or label for each dock window. To minimize a dock window
    programmatically use moveDockWindow() with an edge of \c Minimized.

    Dock windows are moved transparently by default, i.e.
    during the drag an outline rectangle is drawn on the screen
    representing the position of the dock window as it moves. If you
    want the dock window to be shown normally whilst it is moved use
    setOpaqueMoving().

    The location of a dock window, i.e. its dock area and position
    within the dock area, can be determined by calling getLocation().
    Movable dock windows can be lined up to minimize wasted space with
    lineUpDockWindows(). Pointers to the dock areas are available from
    topDock(), leftDock(), rightDock() and bottomDock(). A customize
    menu item is added to the pop up dock window menu if
    isCustomizable() returns TRUE; it returns FALSE by default.
    Reimplement isCustomizable() and customize() if you want to offer
    this extra menu item, for example, to allow the user to change
    settings relating to the main window and its toolbars and dock
    windows.

    The main window's menu bar is fixed (at the top) by default. If you
    want a movable menu bar, create a QMenuBar as a stretchable widget
    inside its own movable dock window and restrict this
    dock window to only live within the \c Top or \c Bottom dock:

    \code
    QToolBar *tb = new QToolBar( this );
    addDockWindow( tb, tr( "Menubar" ), Top, FALSE );
    QMenuBar *mb = new QMenuBar( tb );
    mb->setFrameStyle( QFrame::NoFrame );
    tb->setStretchableWidget( mb );
    setDockEnabled( tb, Left, FALSE );
    setDockEnabled( tb, Right, FALSE );
    \endcode

  An application with multiple dock windows can choose to save the
  current dock window layout in order to restore it later, e.g. in the
  next session. You can do this by using the streaming operators for
  QMainWindow.

  To save the layout and positions of all the dock windows do this:

  \code
  QFile f( filename );
  if ( f.open( IO_WriteOnly ) ) {
      QTextStream ts( &f );
      ts << *mainWindow;
      f.close();
  }
  \endcode

  To restore the dock window positions and sizes (normally when the
  application is next started), do following:

  \code
  QFile f( filename );
  if ( f.open( IO_ReadOnly ) ) {
      QTextStream ts( &f );
      ts >> *mainWindow;
      f.close();
  }
  \endcode

    The QSettings class can be used in conjunction with the streaming
    operators to store the application's settings.

    QMainWindow's management of dock windows and toolbars is done
    transparently behind-the-scenes by QDockArea.

  For multi-document interfaces (MDI), use a QWorkspace as the central
  widget.

  Adding dock windows, e.g. toolbars, to QMainWindow's dock areas is
  straightforward. If the supplied dock areas are not sufficient for
  your application we suggest that you create a QWidget subclass and add
  your own dock areas (see \l QDockArea) to the subclass since
  QMainWindow provides functionality specific to the standard dock areas
  it provides.

  <img src=qmainwindow-m.png> <img src=qmainwindow-w.png>

  \sa QToolBar QDockWindow QStatusBar QAction QMenuBar QPopupMenu QToolTipGroup QDialog
*/

/*!
  \enum Qt::Dock

  Each dock window can be in one of the following positions:

  \value Top  above the central widget, below the menu bar.

  \value Bottom  below the central widget, above the status bar.

  \value Left  to the left of the central widget.

  \value Right to the right of the central widget.

  \value Minimized the dock window is not shown (this is effectively a
  'hidden' dock area); the handles of all minimized dock windows are drawn
  in one row below the menu bar.

  \value TornOff the dock window floats as its own top level window
  which always stays on top of the main window.

  \value Unmanaged not managed by a QMainWindow.
*/

/*!
    \enum QMainWindow::DockWindows

    Right-clicking a dock area will pop-up the dock window menu
    (createDockWindowMenu() is called automatically). When called in
    code you can specify what items should appear on the menu with this
    enum.

    \value OnlyToolBars The menu will list all the toolbars,
    but not any other dock windows.

    \value NoToolBars The menu will list dock windows but not toolbars.

    \value AllDockWindows The menu will list all toolbars and other dock
    windows. (This is the default.)

*/

/*!  Constructs an empty main window. */

QMainWindow::QMainWindow( QWidget * parent, const char * name, WFlags f )
    : QWidget( parent, name, f )
{
    d = new QMainWindowPrivate;
    d->opaque = FALSE;
    installEventFilter( this );
    d->topDock = new QDockArea( Horizontal, QDockArea::Normal, this, "qt_top_dock" );
    d->topDock->installEventFilter( this );
    d->bottomDock = new QDockArea( Horizontal, QDockArea::Reverse, this, "qt_bottom_dock" );
    d->bottomDock->installEventFilter( this );
    d->leftDock = new QDockArea( Vertical, QDockArea::Normal, this, "qt_left_dock" );
    d->leftDock->installEventFilter( this );
    d->rightDock = new QDockArea( Vertical, QDockArea::Reverse, this, "qt_right_dock" );
    d->rightDock->installEventFilter( this );
    d->hideDock = new QHideDock( this );
}


/*! Destroys the object and frees any allocated resources.

*/

QMainWindow::~QMainWindow()
{
    delete layout();
    delete d;
}

#ifndef QT_NO_MENUBAR
/*!  Sets this main window to use the menu bar \a newMenuBar.

  The existing menu bar (if any) is deleted along with its contents.

  \sa menuBar()
*/

void QMainWindow::setMenuBar( QMenuBar * newMenuBar )
{
    if ( !newMenuBar )
	return;
    if ( d->mb )
	delete d->mb;
    d->mb = newMenuBar;
    d->mb->installEventFilter( this );
    triggerLayout();
}


/*!  Returns the menu bar for this window.

    If there isn't one, then menuBar() creates an empty menu bar.

  \sa statusBar()
*/

QMenuBar * QMainWindow::menuBar() const
{
    if ( d->mb )
	return d->mb;

    QObjectList * l
	= ((QObject*)this)->queryList( "QMenuBar", 0, FALSE, FALSE );
    QMenuBar * b;
    if ( l && l->count() ) {
	b = (QMenuBar *)l->first();
    } else {
	b = new QMenuBar( (QMainWindow *)this, "automatic menu bar" );
	b->show();
    }
    delete l;
    d->mb = b;
    d->mb->installEventFilter( this );
    ((QMainWindow *)this)->triggerLayout();
    return b;
}
#endif // QT_NO_MENUBAR

/*!  Sets this main window to use the status bar \a newStatusBar.

  The existing status bar (if any) is deleted along with its contents.

  Note that \a newStatusBar \e must be a child of this main window, and
  that it is not automatically displayed.  If you call this function
  after show(), you will probably also need to call
  newStatusBar->show().

  \sa setMenuBar() statusBar()
*/

void QMainWindow::setStatusBar( QStatusBar * newStatusBar )
{
    if ( !newStatusBar || newStatusBar == d->sb )
	return;
    if ( d->sb )
	delete d->sb;
    d->sb = newStatusBar;
    // ### this code can cause unnecessary creation of a tool tip group
    connect( toolTipGroup(), SIGNAL(showTip(const QString&)),
	     d->sb, SLOT(message(const QString&)) );
    connect( toolTipGroup(), SIGNAL(removeTip()),
	     d->sb, SLOT(clear()) );
    d->sb->installEventFilter( this );
    triggerLayout();
}


/*!  Returns the status bar for this window.  If there isn't one,
  statusBar() creates an empty status bar, and if necessary
  a tool tip group too.

  \sa  menuBar() toolTipGroup()
*/

QStatusBar * QMainWindow::statusBar() const
{
    if ( d->sb )
	return d->sb;

    QObjectList * l
	= ((QObject*)this)->queryList( "QStatusBar", 0, FALSE, FALSE );
    QStatusBar * s;
    if ( l && l->count() ) {
	s = (QStatusBar *)l->first();
    } else {
	s = new QStatusBar( (QMainWindow *)this, "automatic status bar" );
	s->show();
    }
    delete l;
    ((QMainWindow *)this)->setStatusBar( s );
    ((QMainWindow *)this)->triggerLayout( TRUE );
    return s;
}


/*!  Sets this main window to use the tool tip group \a newToolTipGroup.

  The existing tool tip group (if any) is deleted along with its
  contents. All the tool tips connected to it lose the ability to
  display the group texts.

  \sa menuBar() toolTipGroup()
*/

void QMainWindow::setToolTipGroup( QToolTipGroup * newToolTipGroup )
{
    if ( !newToolTipGroup || newToolTipGroup == d->ttg )
	return;
    if ( d->ttg )
	delete d->ttg;
    d->ttg = newToolTipGroup;

    connect( toolTipGroup(), SIGNAL(showTip(const QString&)),
	     statusBar(), SLOT(message(const QString&)) );
    connect( toolTipGroup(), SIGNAL(removeTip()),
	     statusBar(), SLOT(clear()) );
}


/*!  Returns the tool tip group for this window.  If there isn't one,
  toolTipGroup() creates an empty tool tip groups.

  \sa menuBar() statusBar()
*/

QToolTipGroup * QMainWindow::toolTipGroup() const
{
    if ( d->ttg )
	return d->ttg;

    QToolTipGroup * t = new QToolTipGroup( (QMainWindow*)this,
					   "automatic tool tip group" );
    ((QMainWindowPrivate*)d)->ttg = t;
    return t;
}


/*!
    If \a enable is TRUE then users can dock windows in the \a dock
    area.  If \a enable is FALSE users cannot dock windows in the \a
    dock area.

      Users can dock (drag) dock windows into any enabled dock area.
*/

void QMainWindow::setDockEnabled( Dock dock, bool enable )
{
    d->docks.replace( dock, enable );
}


/*!  Returns TRUE if the \a dock dock area is enabled, i.e. it can accept
    user dragged dock windows; otherwise returns FALSE.

  \sa setDockEnabled()
*/

bool QMainWindow::isDockEnabled( Dock dock ) const
{
    return d->docks[ dock ];
}

/*!  \overload

  Returns TRUE if \a area is enabled, i.e. it can accept user dragged
 dock windows; otherwise returns FALSE.

  \sa setDockEnabled()
*/

bool QMainWindow::isDockEnabled( QDockArea *area ) const
{
    if ( area == d->leftDock )
	return d->docks[ Left ];
    if ( area == d->rightDock )
	return d->docks[ Right ];
    if ( area == d->topDock )
	return d->docks[ Top ];
    if ( area == d->bottomDock )
	return d->docks[ Bottom ];
    return FALSE;
}

/*!
    \overload

    If \a enable is TRUE then users can dock the \a dw dock window in
    the \a dock area.  If \a enable is FALSE users cannot dock the \a dw
    dock window in the \a dock area.

    In general users can dock (drag) dock windows into any enabled dock
    area. Using this function particular dock areas can be enabled (or
    disabled) as docking points for particular dock windows.
*/


void QMainWindow::setDockEnabled( QDockWindow *dw, Dock dock, bool enable )
{
    if ( d->dockWindows.find( dw ) == -1 ) {
	d->dockWindows.append( dw );
	connect( dw, SIGNAL( placeChanged( QDockWindow::Place ) ),
		 this, SLOT( slotPlaceChanged() ) );
    }
    QString s;
    s.sprintf( "%p_%d", dw, (int)dock );
    if ( enable )
	d->disabledDocks.remove( s );
    else if ( d->disabledDocks.find( s ) == d->disabledDocks.end() )
	d->disabledDocks << s;
    switch ( dock ) {
	case Top:
	    topDock()->setAcceptDockWindow( dw, enable );
	    break;
	case Left:
	    leftDock()->setAcceptDockWindow( dw, enable );
	    break;
	case Right:
	    rightDock()->setAcceptDockWindow( dw, enable );
	    break;
	case Bottom:
	    bottomDock()->setAcceptDockWindow( dw, enable );
	    break;
	default:
	    break;
    }
}

/*!  \overload

  Returns TRUE if \a area is enabled for the dock window \a dw,
 otherwise returns FALSE.

  \sa setDockEnabled()
*/

bool QMainWindow::isDockEnabled( QDockWindow *dw, QDockArea *area ) const
{
    Dock dock;
    if ( area == d->leftDock )
	dock = Left;
    else if ( area == d->rightDock )
	dock = Right;
    else if ( area == d->topDock )
	dock = Top;
    else if ( area == d->bottomDock )
	dock = Bottom;
    else
	return FALSE;
    return isDockEnabled( dw, dock );
}

/*!  \overload

  Returns TRUE if \a dock is enabled for the dock window \a tb,
 otherwise returns FALSE.

  \sa setDockEnabled()
*/

bool QMainWindow::isDockEnabled( QDockWindow *tb, Dock dock ) const
{
    QString s;
    s.sprintf( "%p_%d", tb, (int)dock );
    return d->disabledDocks.find( s ) == d->disabledDocks.end();
}



/*!  Adds \a dockWindow to the \a edge dock area.

    If \a newLine is FALSE (the default) then the \a dockWindow is added at
    the end of the \a edge. For vertical edges the end is at the bottom,
    for horizontal edges (including \c Minimized) the end is at the
    right.  If \a newLine is TRUE a new line of dock windows is started
    with \a dockWindow as the first (left-most and top-most) dock
    window.

  If \a dockWindow is managed by another main window, it is first
  removed from that window.
*/

void QMainWindow::addDockWindow( QDockWindow *dockWindow,
			      Dock edge, bool newLine )
{
    moveDockWindow( dockWindow, edge );
    dockWindow->setNewLine( newLine );
    if ( d->dockWindows.find( dockWindow ) == -1 )
	d->dockWindows.append( dockWindow );
    connect( dockWindow, SIGNAL( placeChanged( QDockWindow::Place ) ),
	     this, SLOT( slotPlaceChanged() ) );
    dockWindow->installEventFilter( this );
}


/*! \overload

    Adds \a dockWindow to the dock area with label \a label.

    If \a newLine is FALSE (the default) the \a dockWindow is added at
    the end of the \a edge. For vertical edges the end is at the bottom,
    for horizontal edges (including \c Minimized) the end is at the
    right.  If \a newLine is TRUE a new line of dock windows is started
    with \a dockWindow as the first (left-most and top-most) dock
    window.

  If \a dockWindow is managed by another main window, it is first
  removed from that window.
*/

void QMainWindow::addDockWindow( QDockWindow * dockWindow, const QString &label,
			      Dock edge, bool newLine )
{
    addDockWindow( dockWindow, edge, newLine );
    if ( dockWindow->inherits( "QToolBar" ) )
	( (QToolBar*)dockWindow )->setLabel( label );
}

/*!
  Moves \a dockWindow to the end of the \a edge.

    For vertical edges the end is at the bottom, for horizontal edges
    (including \c Minimized) the end is at the right.

  If \a dockWindow is managed by another main window, it is first
  removed from that window.
*/

void QMainWindow::moveDockWindow( QDockWindow * dockWindow, Dock edge )
{
    switch ( edge ) {
    case Top:
	dockWindow->removeFromDock( FALSE );
	d->topDock->moveDockWindow( dockWindow );
	break;
    case Bottom:
	dockWindow->removeFromDock( FALSE );
	d->bottomDock->moveDockWindow( dockWindow );
	break;
    case Right:
	dockWindow->removeFromDock( FALSE );
	d->rightDock->moveDockWindow( dockWindow );
	break;
    case Left:
	dockWindow->removeFromDock( FALSE );
	d->leftDock->moveDockWindow( dockWindow );
	break;
    case TornOff:
	dockWindow->undock();
	break;
    case Minimized:
	dockWindow->undock( d->hideDock );
	break;
    case Unmanaged:
	break;
    }
    if ( dockWindow->inherits( "QToolBar" ) )
	( (QToolBar*)dockWindow )->setOrientation( dockWindow->orientation() );
}

/*! \overload

    Moves \a dockWindow to position \a index of \a edge.

    Any dock windows with positions \a index or higher have
    their position number incremented and any of these on the same line
    are moved right (down for vertical dock areas) to make room.

    If \a nl is TRUE, a new dock window line is created below the line
    in which the moved dock window appears and the moved dock window,
    with any others with higher positions on the same line, is moved
    to this new line.

    The \a extraOffset is the space to put between the left side of the
    dock area (top side for vertical dock areas) and the dock window.
    (This is mostly used for restoring dock windows to the positions the
    user has dragged them to.)

  If \a dockWindow is managed by another main window, it is first
  removed from that window.
*/

void QMainWindow::moveDockWindow( QDockWindow * dockWindow, Dock edge, bool nl, int index, int extraOffset )
{
    dockWindow->setNewLine( nl );
    dockWindow->setOffset( extraOffset );
    switch ( edge ) {
    case Top:
	dockWindow->removeFromDock( FALSE );
	d->topDock->moveDockWindow( dockWindow, index );
	break;
    case Bottom:
	dockWindow->removeFromDock( FALSE );
	d->bottomDock->moveDockWindow( dockWindow, index );
	break;
    case Right:
	dockWindow->removeFromDock( FALSE );
	d->rightDock->moveDockWindow( dockWindow, index );
	break;
    case Left:
	dockWindow->removeFromDock( FALSE );
	d->leftDock->moveDockWindow( dockWindow, index );
	break;
    case TornOff:
	dockWindow->undock();
	break;
    case Minimized:
	dockWindow->undock( d->hideDock );
	break;
    case Unmanaged:
	break;
    }
    if ( dockWindow->inherits( "QToolBar" ) )
	( (QToolBar*)dockWindow )->setOrientation( dockWindow->orientation() );
}

/*!
  Removes \a dockWindow from the main window's docking area, provided \a
  dockWindow is non-null and managed by this main window.
*/

void QMainWindow::removeDockWindow( QDockWindow * dockWindow )
{
    dockWindow->hide();
    d->dockWindows.removeRef( dockWindow );
    disconnect( dockWindow, SIGNAL( placeChanged( QDockWindow::Place ) ),
		this, SLOT( slotPlaceChanged() ) );
    dockWindow->removeEventFilter( this );
}

/*!  Sets up the geometry management of the window. It is called
  automatically when needed, so you shouldn't need to call it.
*/

void QMainWindow::setUpLayout()
{
#ifndef QT_NO_MENUBAR
    if ( !d->mb ) {
	// slightly evil hack here.  reconsider this after 2.0
	QObjectList * l
	    = ((QObject*)this)->queryList( "QMenuBar", 0, FALSE, FALSE );
	if ( l && l->count() )
	    d->mb = menuBar();
	delete l;
    }
#endif
    if ( !d->sb ) {
	// as above.
	QObjectList * l
	    = ((QObject*)this)->queryList( "QStatusBar", 0, FALSE, FALSE );
	if ( l && l->count() )
	    d->sb = statusBar();
	delete l;
    }

    delete d->tll;
    d->tll = new QBoxLayout( this, QBoxLayout::Down );

#ifndef QT_NO_MENUBAR
    if ( d->mb && d->mb->isVisibleTo( this ) ) {
	d->tll->setMenuBar( d->mb );
    }
#endif

    d->tll->addWidget( d->hideDock );

    if ( style() == WindowsStyle )
	d->tll->addSpacing( d->movable ? 1  : 2 );
    d->tll->addWidget( d->topDock );

    QMainWindowLayout *mwl = new QMainWindowLayout( d->tll );

    mwl->setLeftDock( d->leftDock );
    if ( centralWidget() )
	mwl->setCentralWidget( centralWidget() );
    mwl->setRightDock( d->rightDock );

    d->tll->addWidget( d->bottomDock );

    if ( d->sb ) {
	d->tll->addWidget( d->sb, 0 );
	// make the sb stay on top of tool bars if there isn't enough space
	d->sb->raise();
    }
}


/*!  \reimp */
void QMainWindow::show()
{
    if ( !d->tll)
	setUpLayout();
    QWidget::show();
}


/*!  \reimp */
QSize QMainWindow::sizeHint() const
{
    QMainWindow* that = (QMainWindow*) this;
    // Workaround: because d->tll get's deleted in
    // totalSizeHint->polish->sendPostedEvents->childEvent->triggerLayout
    // [eg. canvas example on Qt/Embedded]
    QApplication::sendPostedEvents( that, QEvent::ChildInserted );
    if ( !that->d->tll )
	that->setUpLayout();
    return that->d->tll->totalSizeHint();
}

/*!  \reimp */
QSize QMainWindow::minimumSizeHint() const
{
    if ( !d->tll ) {
	QMainWindow* that = (QMainWindow*) this;
	that->setUpLayout();
    }
    return d->tll->totalMinimumSize();
}

/*!  Sets the central widget for this window to \a w.

    The central widget is surrounded by the left, top, right and bottom
    dock areas. The menu bar is above the top dock area.

    \sa centralWidget()
*/

void QMainWindow::setCentralWidget( QWidget * w )
{
    if ( d->mc )
	d->mc->removeEventFilter( this );
    d->mc = w;
    if ( d->mc )
	d->mc->installEventFilter( this );
    triggerLayout();
}


/*!  Returns a pointer to the main window's central widget.

    The central widget is surrounded by the left, top, right and bottom
    dock areas. The menu bar is above the top dock area.

  \sa setCentralWidget()
*/

QWidget * QMainWindow::centralWidget() const
{
    return d->mc;
}


/*! \reimp */

void QMainWindow::paintEvent( QPaintEvent * )
{
    if ( style() == WindowsStyle && d->mb && !d->topDock->isEmpty() ) {
	QPainter p( this );
	int y = d->mb->height() + 1;
	style().drawSeparator( &p, 0, y, width(), y, colorGroup() );
    }
}


bool QMainWindow::dockMainWindow( QObject *dock )
{
    while ( dock ) {
	if ( dock->parent() && dock->parent() == this )
	    return TRUE;
	if ( dock->parent() && dock->parent()->inherits( "QMainWindow" ) )
	    return FALSE;
	dock = dock->parent();
    }
    return FALSE;
}

/*!
  \reimp
*/

bool QMainWindow::eventFilter( QObject* o, QEvent *e )
{
    if ( e->type() == QEvent::Show && o == this ) {
	if ( !d->tll )
	    setUpLayout();
	d->tll->activate();
    }

    if ( e->type() == QEvent::ContextMenu && d->dockMenu &&
	 ( o->inherits( "QDockArea" ) && dockMainWindow( o ) || o == d->hideDock ) ) {
	if ( showDockMenu( ( (QMouseEvent*)e )->globalPos() ) ) {
	    ( (QContextMenuEvent*)e )->accept();
	    return TRUE;
	}
    }

    return QWidget::eventFilter( o, e );
}


/*!
  Monitors events to ensure the layout is updated.
*/
void QMainWindow::childEvent( QChildEvent* e)
{
    if ( e->type() == QEvent::ChildRemoved ) {
	if ( e->child() == 0 ||
	     !e->child()->isWidgetType() ||
	     ((QWidget*)e->child())->testWFlags( WType_TopLevel ) ) {
	    // nothing
	} else if ( e->child() == d->sb ) {
	    d->sb = 0;
	    triggerLayout();
	} else if ( e->child() == d->mb ) {
	    d->mb = 0;
	    triggerLayout();
	} else if ( e->child() == d->mc ) {
	    d->mc = 0;
	    triggerLayout();
	} else if ( e->child()->isWidgetType() ) {
	    removeDockWindow( (QDockWindow *)(e->child()) );
	    d->appropriate.remove( (QDockWindow*)e->child() );
	    triggerLayout();
	}
    } else if ( e->type() == QEvent::ChildInserted ) {
	if ( e->child()->inherits( "QStatusBar" ) ) {
	    d->sb = (QStatusBar*)e->child();
	    if ( d->tll ) {
		if ( !d->tll->findWidget( d->sb ) )
		    d->tll->addWidget( (QStatusBar*)e->child() );
	    } else {
		triggerLayout();
	    }
	}
    }
}

/*!\reimp
*/

bool QMainWindow::event( QEvent * e )
{
    if ( e->type() == QEvent::ChildRemoved && ( (QChildEvent*)e )->child() == d->mc ) {
	d->mc->removeEventFilter( this );
	d->mc = 0;
    }

    return QWidget::event( e );
}


/*! \property QMainWindow::usesBigPixmaps
    \brief whether big pixmaps are enabled

  If disabled, the tool buttons will use small
  pixmaps. If enabled, big pixmaps will be used.

  Tool buttons and other widgets that wish to respond to this setting
  are responsible for reading the correct state on startup, and for
  connecting to the main window's widget's pixmapSizeChanged() signal.
*/

bool QMainWindow::usesBigPixmaps() const
{
    return d->ubp;
}

void QMainWindow::setUsesBigPixmaps( bool enable )
{
    if ( d->ubp == enable )
	return;

    d->ubp = enable;
    emit pixmapSizeChanged( enable );

    QObjectList *l = queryList( "QLayout" );
    if ( !l || !l->first() ) {
	delete l;
	return;
    }
    for ( QLayout *lay = (QLayout*)l->first(); lay; lay = (QLayout*)l->next() )
	    lay->activate();
    delete l;
}

/*! \property QMainWindow::usesTextLabel
    \brief whether text labels for toolbar buttons are enabled

  If disabled (the default), the tool buttons will not use
  text labels. If enabled, text labels will be used.

  Tool buttons and other widgets that wish to respond to this setting
  are responsible for reading the correct state on startup, and for
  connecting to the main window's widget's usesTextLabelChanged()
  signal.

  \sa QToolButton::setUsesTextLabel()
*/

bool QMainWindow::usesTextLabel() const
{
    return d->utl;
}


void QMainWindow::setUsesTextLabel( bool enable )
{
    if ( d->utl == enable )
	return;

    d->utl = enable;
    emit usesTextLabelChanged( enable );

    QObjectList *l = queryList( "QLayout" );
    if ( !l || !l->first() ) {
	delete l;
	return;
    }
    for ( QLayout *lay = (QLayout*)l->first(); lay; lay = (QLayout*)l->next() )
	    lay->activate();
    delete l;
}


/*! \property QMainWindow::toolBarsMovable
    \brief If the toolbars are movable
    \obsolete

    This property is obsolete, use dockWindowsMovable now.

    \sa dockWindowsMovable()
*/


/*! \fn void QMainWindow::pixmapSizeChanged( bool )

  This signal is called whenever the setUsesBigPixmaps() is called
  with a value different to the current setting.  All
  widgets that should respond to such changes, e.g. toolbar buttons,
  must connect to this signal.
*/

/*! \fn void QMainWindow::usesTextLabelChanged( bool )

  This signal is called whenever the setUsesTextLabel() is called
  with a value different to the current setting.  All
  widgets that should respond to such changes, e.g. toolbar buttons,
  must connect to this signal.
*/

/*!
  \fn void QMainWindow::dockWindowPositionChanged( QDockWindow *dockWindow )

  This signal is emitted when the \a dockWindow has changed its position.
  A change in position occurs when a dock window is moved within its
  dock area or moved to another dock area (including the \c Minimized
  and \c TearOff dock areas).

  \sa getLocation()
*/

void QMainWindow::setRightJustification( bool enable )
{
    if ( enable == d->justify )
	return;
    d->justify = enable;
    triggerLayout( TRUE );
}


/*! \property QMainWindow::rightJustification
    \brief whether the main window right-justifies its dock windows

  If disabled (the default), stretchable dock windows
  are expanded, and non-stretchable dock windows are given the minimum
  space they need. Since most dock windows are not stretchable, this
  usually results in a unjustified right edge (or unjustified bottom edge for a
  vertical dock area). If enabled, the main window will
  right-justify its dock windows.

  \sa QDockWindow::setVerticalStretchable(), QDockWindow::setHorizontalStretchable()
*/

bool QMainWindow::rightJustification() const
{
    return d->justify;
}

/*! \internal
 */

void QMainWindow::triggerLayout( bool deleteLayout )
{
    if ( !deleteLayout && d->tll ) {
    } else {
	delete d->tll;
	d->tll = 0;
	setUpLayout();
    }
    QApplication::postEvent( this, new QEvent( QEvent::LayoutHint ) );
}

/*!
    Enters 'What's This?' question mode and returns immediately.

    This is the same as QWhatsThis::enterWhatsThisMode(), but
    implemented as a main window object's slot. This way it can easily
    be used for popup menus, for example:

  \code
    QPopupMenu * help = new QPopupMenu( this );
    help->insertItem( "What's &This", this , SLOT(whatsThis()), SHIFT+Key_F1);
  \endcode

  \sa QWhatsThis::enterWhatsThisMode()

 */
void QMainWindow::whatsThis()
{
    QWhatsThis::enterWhatsThisMode();
}


/*!
  \reimp
*/

void QMainWindow::styleChange( QStyle& old )
{
    QWidget::styleChange( old );
}

/*!
    Finds the location of the dock window \a dw.

    If the \a dw dock window is found in the main window the function
    returns TRUE and populates the \a dock variable with the dw's
    dock area and the \a index with the dw's position within the dock
    area. It also sets \a nl to TRUE if the \a dw begins a new line
    (otherwise FALSE), and \a extraOffset with the dw's offset.

    If the \a dw dock window is not found then the function returns FALSE and
    the state of \a dock, \a index, \a nl and \a extraOffset is
    undefined.

    If you want to save and restore dock window positions then use
    operator>>() and operator<<().

    \sa operator>>() operator<<()

*/

bool QMainWindow::getLocation( QDockWindow *dw, Dock &dock, int &index, bool &nl, int &extraOffset ) const
{
    dock = TornOff;
    if ( d->topDock->hasDockWindow( dw, &index ) )
	dock = Top;
    else if ( d->bottomDock->hasDockWindow( dw, &index ) )
	dock = Bottom;
    else if ( d->leftDock->hasDockWindow( dw, &index ) )
	dock = Left;
    else if ( d->rightDock->hasDockWindow( dw, &index ) )
	dock = Right;
    else if ( dw->parentWidget() == d->hideDock ) {
	index = 0;
	dock = Minimized;
    } else {
	index = 0;
    }
    nl = dw->newLine();
    extraOffset = dw->offset();
    return TRUE;
}

/*!  Returns a list of all the toolbars which are in the \a dock dock
 area, regardless of their state.

 For example, the \c TornOff dock area may contain closed toolbars but
 these are returned along with the visible toolbars.

  \sa dockWindows()
*/

QPtrList<QToolBar> QMainWindow::toolBars( Dock dock ) const
{
    QPtrList<QDockWindow> lst = dockWindows( dock );
    QPtrList<QToolBar> tbl;
    for ( QDockWindow *w = lst.first(); w; w = lst.next() ) {
	if ( w->inherits( "QToolBar" ) )
	    tbl.append( (QToolBar*)w );
    }
    return tbl;
}

/*!  Returns a list of all the dock windows which are in the \a dock
 dock area, regardless of their state.

 For example, the \c TornOff dock area may contain closed dock windows
 but these are returned along with the visible dock windows.
*/

QPtrList<QDockWindow> QMainWindow::dockWindows( Dock dock ) const
{
    QPtrList<QDockWindow> lst;
    switch ( dock ) {
    case Top:
	return d->topDock->dockWindowList();
    case Bottom:
	return d->bottomDock->dockWindowList();
    case Left:
	return d->leftDock->dockWindowList();
    case Right:
	return d->rightDock->dockWindowList();
    case TornOff: {
	for ( QDockWindow *w = d->dockWindows.first(); w; w = d->dockWindows.next() ) {
	    if ( !w->area() && w->place() == QDockWindow::OutsideDock )
		lst.append( w );
	}
    }
    return lst;
    case Minimized: {
	if ( d->hideDock->children() ) {
	    QObjectListIt it( *d->hideDock->children() );
	    QObject *o;
	    while ( ( o = it.current() ) ) {
		++it;
		if ( !o->inherits( "QDockWindow" ) )
		    continue;
		lst.append( (QDockWindow*)o );
	    }
	}
    }
    return lst;
    default:
	break;
    }
    return lst;
}

/* Returns the list of dock windows which belong to this main window,
    regardless of which dock area they are in or what their state is,
    (e.g. irrespective of whether they are visible or not).
*/

QPtrList<QDockWindow> QMainWindow::dockWindows() const
{
    return d->dockWindows;
}

void QMainWindow::setDockWindowsMovable( bool enable )
{
    d->movable = enable;
    QObjectList *l = queryList( "QDockWindow" );
    if ( l ) {
	for ( QObject *o = l->first(); o; o = l->next() )
	    ( (QDockWindow*)o )->setMovingEnabled( enable );
    }
    delete l;
}

/*! \property QMainWindow::dockWindowsMovable
    \brief whether the dock windows are movable

  If enabled, the user will be able to move Movable dock windows from
  one QMainWindow dock area to another, including the \c TearOff
  area (i.e. where the dock window floats freely as a window in its
  own right), and the \c Minimized area (where only the dock window's
  handle is shown below the menu bar). Moveable dock windows can also
  be moved within QMainWindow dock areas, i.e. to rearrange them within
  a dock area.

  If disabled the user will not be able to move any dock windows.

  By default dock windows are moved transparently (i.e. only an outline
  rectangle is shown during the drag), but this setting can be changed
  with setOpaqueMoving().

  \sa setDockEnabled(), setOpaqueMoving()

*/

bool QMainWindow::dockWindowsMovable() const
{
    return d->movable;
}

void QMainWindow::setOpaqueMoving( bool b )
{
    d->opaque = b;
    QObjectList *l = queryList( "QDockWindow" );
    if ( l ) {
	for ( QObject *o = l->first(); o; o = l->next() )
	    ( (QDockWindow*)o )->setOpaqueMoving( b );
    }
    delete l;
}

/*! \property QMainWindow::opaqueMoving
    \brief whether dock windows are moved opaquely

  If enabled the dock windows of the main window are shown
  opaquely (i.e. it shows the toolbar as it looks when docked) when
  moved. If disabled they are shown transparently,
  (i.e. as an outline rectangle).
*/

bool QMainWindow::opaqueMoving() const
{
    return d->opaque;
}

/*!
    This function will line up dock windows within the visible dock
    areas (\c Top, \c Left, \c Right and \c Bottom) as compactly as
    possible.

  If \a keepNewLines is TRUE, all dock windows stay on their original
  lines. If \a keepNewLines is FALSE then newlines may be removed to achieve
  the most compact layout possible.

  The method only works if dockWindowsMovable() returns TRUE.
*/

void QMainWindow::lineUpDockWindows( bool keepNewLines )
{
    if ( !dockWindowsMovable() )
	return;
    d->topDock->lineUp( keepNewLines );
    d->leftDock->lineUp( keepNewLines );
    d->rightDock->lineUp( keepNewLines );
    d->bottomDock->lineUp( keepNewLines );
}

/*!

  Returns TRUE, if the dock window menu is enabled; otherwise returns
  FALSE.

  The menu lists the (appropriate()) dock windows (which may be shown
  or hidden), and has a "Line Up Dock Windows" menu item. It will also have
  a "Customize" menu item if isCustomizable() returns TRUE.

  \sa setDockEnabled(), lineUpDockWindows() appropriate() setAppropriate()
*/

bool QMainWindow::isDockMenuEnabled() const
{
    return d->dockMenu;
}

/*!
    If \a b is TRUE then right clicking on a dock window or dock area
    will pop up the dock window menu. If \a b is FALSE right
    clicking a dock window or dock area will not pop up the menu.

  The menu lists the (appropriate()) dock windows (which may be shown or
  hidden), and has a line up dock window item. It will also have a
  Customize menu item if isCustomizable() returns TRUE.

  \sa lineUpDockWindows(), isDockMenuEnabled()
*/

void QMainWindow::setDockMenuEnabled( bool b )
{
    d->dockMenu = b;
}

/*!  Creates the dock window menu which contains all toolbars (if \a
    dockWindows is \c OnlyToolBars ), all dock windows (if \a
    dockWindows is \c NoToolBars) or all toolbars and dock windows ( if
    \a dockWindows is \c AllDockWindows - the default).

  This function is called internally when necessary, e.g. when the user
  right clicks a dock area (providing isDockMenuEnabled() returns TRUE).
  You may reimplement this function if you wish to customize the
  behaviour.

  The menu items representing the toolbars and dock windows are checkable.
  The visible dock windows are checked and the hidden dock windows are
  unchecked. The user can click a menu item to change its state (show
  or hide the dock window).

  The list and the state are always kept up-to-date.

  Toolbars and dock windows which are not appropriate in the current
  context (see setAppropriate()) are not listed in the menu.

  The menu also has a menu item for lining up the dock windows.

  If isCustomizable() returns TRUE, a Customize menu item is added to
  the menu, which if clicked will call customize(). The isCustomizable()
  function we provide returns FALSE and customize() does nothing, so
  they must be reimplemented in a subclass to be useful.
*/

QPopupMenu *QMainWindow::createDockWindowMenu( DockWindows dockWindows ) const
{
    QObjectList *l = queryList( "QDockWindow" );

    if ( !l || l->isEmpty() )
	return 0;

    delete l;

    QPopupMenu *menu = new QPopupMenu( (QMainWindow*)this );
    menu->setCheckable( TRUE );
    d->dockWindowModes.replace( menu, dockWindows );
    connect( menu, SIGNAL( aboutToShow() ), this, SLOT( menuAboutToShow() ) );
    return menu;
}

void QMainWindow::menuAboutToShow()
{
    QPopupMenu *menu = (QPopupMenu*)sender();
    QMap<QPopupMenu*, DockWindows>::Iterator it = d->dockWindowModes.find( menu );
    if ( it == d->dockWindowModes.end() )
	return;
    menu->clear();

    DockWindows dockWindows = *it;

    QObjectList *l = queryList( "QDockWindow" );

    bool empty = TRUE;
    if ( l && !l->isEmpty() ) {

	QObject *o = 0;
	if ( dockWindows == AllDockWindows || dockWindows == NoToolBars ) {
	    for ( o = l->first(); o; o = l->next() ) {
		QDockWindow *dw = (QDockWindow*)o;
		if ( !appropriate( dw ) || dw->inherits( "QToolBar" ) || !dockMainWindow( dw ) )
		    continue;
		QString label = dw->caption();
		if ( !label.isEmpty() ) {
		    int id = menu->insertItem( label, dw, SLOT( toggleVisible() ) );
		    menu->setItemChecked( id, dw->isVisible() );
		    empty = FALSE;
		}
	    }
	    if ( !empty )
		menu->insertSeparator();
	}

	if ( dockWindows == AllDockWindows || dockWindows == OnlyToolBars ) {
	    for ( o = l->first(); o; o = l->next() ) {
		QDockWindow *dw = (QDockWindow*)o;
		if ( !appropriate( dw ) || !dw->inherits( "QToolBar" ) || !dockMainWindow( dw ) )
		    continue;
		QString label = ( (QToolBar*)dw )->label();
		if ( !label.isEmpty() ) {
		    int id = menu->insertItem( label, dw, SLOT( toggleVisible() ) );
		    menu->setItemChecked( id, dw->isVisible() );
		    empty = FALSE;
		}
	    }
	} else {
	    empty = TRUE;
	}

    }

    delete l;

    if ( !empty )
	menu->insertSeparator();

    if ( dockWindowsMovable() )
	menu->insertItem( tr( "Line up" ), this, SLOT( doLineUp() ) );
    if ( isCustomizable() )
	menu->insertItem( tr( "Customize..." ), this, SLOT( customize() ) );
}

/*! Shows the dock menu at the position \a globalPos. The menu lists the
    dock windows so that they can be shown (or hidden), lined up, and
    possibly customized.

  The default implementation uses the dock window menu which gets
  created by createDockWindowMenu(). You can reimplement
  createDockWindowMenu() if you want to use your own specialized popup
  menu.
*/

bool QMainWindow::showDockMenu( const QPoint &globalPos )
{
    if ( !d->dockMenu )
	return FALSE;
    if ( !d->rmbMenu )
	d->rmbMenu = createDockWindowMenu();
    if ( !d->rmbMenu )
	return FALSE;

    d->rmbMenu->exec( globalPos );
    return TRUE;
}

void QMainWindow::slotPlaceChanged()
{
    if ( sender()->inherits( "QDockWindow" ) )
	emit dockWindowPositionChanged( (QDockWindow*)sender() );
    if ( sender()->inherits( "QToolBar" ) )
	emit toolBarPositionChanged( (QToolBar*)sender() );
}

/*!
    \internal
    For internal use of QDockWindow only.
 */

QDockArea *QMainWindow::dockingArea( const QPoint &p )
{
    int mh = d->mb ? d->mb->height() : 0;
    int sh = d->sb ? d->sb->height() : 0;
    if ( p.x() >= -5 && p.x() <= 20 && p.y() > mh && p.y() - height() - sh )
	return d->leftDock;
    if ( p.x() >= width() - 20 && p.x() <= width() + 5 && p.y() > mh && p.y() - height() - sh )
	return d->rightDock;
    if ( p.y() >= -5 && p.y() < mh + 20 && p.x() >= 0 && p.x() <= width() )
	return d->topDock;
    if ( p.y() >= height() - sh - 20 && p.y() <= height() + 5 && p.x() >= 0 && p.x() <= width() )
	return d->bottomDock;
    return 0;
}

/*! Returns TRUE if \a dw is a dock window known to the main window,
otherwise returns FALSE.
*/

bool QMainWindow::hasDockWindow( QDockWindow *dw )
{
    return d->dockWindows.findRef( dw ) != -1;
}

/*! Returns the \c Left dock area

  \sa rightDock() topDock() bottomDock()
*/

QDockArea *QMainWindow::leftDock() const
{
    return d->leftDock;
}

/*! Returns the \c Right dock area

  \sa leftDock() topDock() bottomDock()
*/

QDockArea *QMainWindow::rightDock() const
{
    return d->rightDock;
}

/*! Returns the \c Top dock area

  \sa bottomDock() leftDock() rightDock()
*/

QDockArea *QMainWindow::topDock() const
{
    return d->topDock;
}

/*! Returns a pointer the \c Bottom dock area

  \sa topDock() leftDock() rightDock()
*/

QDockArea *QMainWindow::bottomDock() const
{
    return d->bottomDock;
}

/*! This function is called when the user clicks the Customize menu item
    on the dock window menu.

    The customize menu item will only appear if isCustomizable() returns
    TRUE (it returns FALSE by default).

    The function is intended, for example, to provide the user a means
    of telling the application that they wish to customize the main
    window, dock windows or dock areas.

    The default implementation does nothing, but this may change in
    later Qt versions. In view of this the Customize menu item is not
    shown on the right-click menu by default. If you want the item to
    appear then reimplement isCustomizable() to return TRUE.

   \sa isCustomizable()
*/

void QMainWindow::customize()
{
}

/*!
    Returns TRUE if the dock area dock window menu includes the
    Customize menu item (which calls customize when clicked). Returns
    FALSE by default, i.e. the popup menu will not contain a Customize
    menu item.  You will need to reimplement this function and set it
    to return TRUE if you wish the user to be able to see the dock window
    menu.

  \sa customize()
*/

bool QMainWindow::isCustomizable() const
{
    return FALSE;
}

/*!
    Returns TRUE if it is appropriate to include a menu item listing
    the \a dw dock window on the dock window menu. Otherwise
    returns FALSE.

    The user is able to change the state (show or hide) a dock window
    that has a menu item by clicking the item.

    Call setAppropriate() to indicate whether or not a particular dock
    window should appear on the popup menu.

  \sa setAppropriate()
*/

bool QMainWindow::appropriate( QDockWindow *dw ) const
{
    QMap<QDockWindow*, bool>::ConstIterator it = d->appropriate.find( dw );
    if ( it == d->appropriate.end() )
	return TRUE;
    return *it;
}

/*!
    Use this function to control whether or not the \a dw dock window's
    caption should appear as a menu item on the dock window menu
    that lists the dock windows.

    If \a a is TRUE then the \a dw will appear as a menu item on the
    dock window menu.  The user is able to change the state
    (show or hide) a dock window that has a menu item by clicking the
    item; depending on the state of your application, this may or may
    not be appropriate.  If \a a is FALSE the \a dw will not appear
    on the popup menu.

    \sa showDockMenu() isCustomizable() customize()

*/

void QMainWindow::setAppropriate( QDockWindow *dw, bool a )
{
    d->appropriate.replace( dw, a );
}


#if defined(QT_ACCESSIBILITY_SUPPORT)
/*!
  \reimp
*/
QAccessibleInterface *QMainWindow::accessibleInterface()
{
    return new QAccessibleWidget( this, QAccessible::Application );
}
#endif

#ifndef QT_NO_TEXTSTREAM
static void saveDockArea( QTextStream &ts, QDockArea *a )
{
    QPtrList<QDockWindow> l = a->dockWindowList();
    for ( QDockWindow *dw = l.first(); dw; dw = l.next() ) {
	ts << QString( dw->caption() );
	ts << ",";
    }
    ts << endl;
    ts << *a;
}

/*!
    \relates QMainWindow

    Writes the layout (sizes and positions) of the dock windows in the
    dock areas of the QMainWindow \a mainWindow, including \c Minimized and \c
    TornOff dock windows, to the text stream \a ts.

    This can be used, for example, in conjunction with QSettings to save
    the user's layout.

    \sa operator>>()
*/

QTextStream &operator<<( QTextStream &ts, const QMainWindow &mainWindow )
{
    QPtrList<QDockWindow> l = mainWindow.dockWindows( Qt::Minimized );
    QDockWindow *dw = 0;
    for ( dw = l.first(); dw; dw = l.next() ) {
	ts << dw->caption();
	ts << ",";
    }
    ts << endl;

    l = mainWindow.dockWindows( Qt::TornOff );
    for ( dw = l.first(); dw; dw = l.next() ) {
	ts << dw->caption();
	ts << ",";
    }
    ts << endl;

    saveDockArea( ts, mainWindow.topDock() );
    saveDockArea( ts, mainWindow.bottomDock() );
    saveDockArea( ts, mainWindow.rightDock() );
    saveDockArea( ts, mainWindow.leftDock() );
    return ts;
}

static void loadDockArea( const QStringList &names, QDockArea *a, Qt::Dock d, QPtrList<QDockWindow> &l, QMainWindow *mw, QTextStream &ts )
{
    for ( QStringList::ConstIterator it = names.begin(); it != names.end(); ++it ) {
	for ( QDockWindow *dw = l.first(); dw; dw = l.next() ) {
	    if ( dw->caption() == *it ) {
		mw->addDockWindow( dw, d );
		break;
	    }
	}
    }
    if ( a )
	ts >> *a;
}

/*!
    \relates QMainWindow

    Reads the layout (sizes and positions) of the dock windows in the
    dock areas of the QMainWindow \a mainWindow from the text stream, \a ts,
    including \c Minimized and \c TornOff dock windows. Restores the
    dock windows and dock areas to these sizes and positions. The layout
    information must be in the format produced by operator<<().

    This can be used, for example, in conjunction with QSettings to
    restore the user's layout.

    \sa operator<<()
*/

QTextStream &operator>>( QTextStream &ts, QMainWindow &mainWindow )
{
    QPtrList<QDockWindow> l = mainWindow.dockWindows();

    QString s = ts.readLine();
    QStringList names = QStringList::split( ',', s );
    loadDockArea( names, 0, Qt::Minimized, l, &mainWindow, ts );

    s = ts.readLine();
    names = QStringList::split( ',', s );
    loadDockArea( names, 0, Qt::TornOff, l, &mainWindow, ts );

    int i = 0;
    QDockArea *areas[] = { mainWindow.topDock(), mainWindow.bottomDock(), mainWindow.rightDock(), mainWindow.leftDock() };
    for ( int d = (int)Qt::Top; d != (int)Qt::Minimized; ++d, ++i ) {
	s = ts.readLine();
	names = QStringList::split( ',', s );
	loadDockArea( names, areas[ i ], (Qt::Dock)d, l, &mainWindow, ts );
    }
    return ts;
}
#endif

#include "qmainwindow.moc"

#endif
