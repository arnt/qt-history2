/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qmainwindow.cpp#87 $
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
#include "qlist.h"
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
    int cached_height;
    int cached_wfh;
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
    cached_height = -1; cached_wfh = -1;
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
	    central->setGeometry( g.x() + wl, g.y(), w, r.height() );
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
	   justify( FALSE ), movable( TRUE ), opaque( FALSE ), dockMenu( TRUE ),
	   rmbMenu( 0 )
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

    QList<QDockWindow> dockWindows;
    QMap<Qt::Dock, bool> docks;
    QStringList disabledDocks;
    bool dockMenu;
    QHideDock *hideDock;

    QPopupMenu *rmbMenu;

};


// NOT REVISED
/*! \class QMainWindow qmainwindow.h

  \brief The QMainWindow class provides a typical application window,
  with a menu bar, some tool bars and a status bar.

  \ingroup application

  In addition, you need the large central widget, which you supply and
  tell QMainWindow about using setCentralWidget(), and perhaps a few
  tool bars - or dock windows in general-, which you can add using
  addDockWindow().

  The central widget is not touched by QMainWindow.  QMainWindow
  manages its geometry, and that is all.  For example, the
  application/application.cpp example (an editor) sets a QMultiLineEdit
  to be the central widget.

  QMainWindow automatically detects the creation of a menu bar or
  status bar if you specify the QMainWindow as parent, or you can use
  the provided menuBar() and statusBar() functions.  menuBar() and
  statusBar() create a suitable widget if one doesn't exist, and
  updates the window's layout to make space.

  QMainWindow also provides a QToolTipGroup connected to the status
  bar.  toolTipGroup() provides access to the QToolTipGroup, but there
  is no way to set the tool tip group.

  The QMainWindow allows by default dock window (toolbar) docking in
  all areas.  You can use setDockEnabled() to enable and disable
  docking areas for toolbars. Currently, \c Top, \c Left, \c Right, \c
  Bottom, \c Minimized and \c TornOff (floating) are meaningful.

  For further documentation note, that toolbars are just dock windows
  with some extra functionality. So everything which applies to dock
  windows also applies to toolbars. See the documentation of
  QDockWindow and QToolBar for further information.

  Several functions let you change the appearance of a QMainWindow
  globally: <ul>

  <li> setRightJustification() determines whether QMainWindow should
  ensure that the toolbars/dock windows fill the available space. This
  is nearly never usful, rather use
  QDockWindow::setHorizontalStretchable() and
  QDockWindow::setVerticalStretchable()) to make specific dock
  windows/toolbars stretchable.

  <li> setUsesBigPixmaps() determines whether QToolButton (and other
  classes) should draw small or large pixmaps (see QIconSet for more
  about that),

  <li> setUsesTextLabel() determines whether the toolbar buttons (and
  other classes), should display a textlabel in addition to pixmaps
  (see QToolButton for more about that).

  </ul>

  Dock windows can be dragged by the user into each enabled docking
  area, inside each docking area and undocked (floated).. This feature
  can be enabled and disabled using setDockWindowsMovable().  By
  default this feature is enabled. If the \c Minimized dock is enabled
  the user can hide(minimize)/show(restore) a dock window with a click
  on the toolbar handle. The handles of all minimized toolbars are
  drawn below the menu bar in one row, and if the user moves the mouse
  cursor onto such a handle, the caption of the dock window is
  displayed in a tool tip (see QDockWindow::caption() or
  QToolBar::label()). So if you enable the Minimized dock, you should
  specify a meaningful caption/label for each dock
  window/toolbar. With a double click on a dock window handle, the
  dockwindow gets undocked. With a double click on a floating dock
  window's titlebar the dock window gets docked again.

  All this management of dock windows and toolbars is infact done by
  QDockArea, but this is done transparently through the QMainWindow API.

  Normally dock windows are moved transparently (this means while the
  user drags one, a rectangle is drawn on the screen). With
  setOpaqueMoving() it's possible to switch between opaque and
  transparent moving of toolbars.

  The main window's menubar is static (on the top) by default. If you
  want a movable menubar, create a QMenuBar as stretchable widget
  inside its own movable dock window or toolbar and restrict this
  dock window to only live within the Top or Bottom dock:

  \code
  QToolBar *tb = new QToolBar( this );
  addToolBar( tb, tr( "Menubar" ), Top, FALSE );
  QMenuBar *mb = new QMenuBar( tb );
  mb->setFrameStyle( QFrame::NoFrame );
  tb->setStretchableWidget( mb );
  setDockEnabled( tb, Left, FALSE );
  setDockEnabled( tb, Right, FALSE );
  \endcode

  An application with multiple dock windows can choose to save the
  current dock window layout in order to restore it in the next
  session. You can do this by using the streaming operators for
  QMainWindow.

  To save the layout and all positions do this:

  \code
  QFile f( filename );
  if ( f.open( IO_WriteOnly ) ) {
      QTextStream ts( &f );
      ts << *mainWindow;
      f.close();
  }
  /endcode

  To restore the dock window positions and sizes (normally on the next
  start of the applications), do following:

  \code
  QFile f( filename );
  if ( f.open( IO_ReadOnly ) ) {
      QTextStream ts( &f );
      ts >> *mainWindow;
      f.close();
  }
  /endcode

  For multi-document interfaces (MDI), use a QWorkspace as central
  widget.

  <img src=qmainwindow-m.png> <img src=qmainwindow-w.png>

  \sa QToolBar QDockWindow QStatusBar QMenuBar QToolTipGroup QDialog
*/

/*!
  \enum Qt::Dock

  Each dock window can be in one of the following positions:

  \value Top  above the central widget, below the menubar.

  \value Bottom  below the central widget, above the status bar.

  \value Left  to the left of the central widget.

  \value Right to the left of the central widget.

  \value Minimized the toolbar is not shown - all handles of minimized
  toolbars are drawn in one row below the menu bar.

  \value TornOff floating dock window as own top level window which
  always stays on to of the mainwindow.

  \value Unmanaged not managed by a QMainWindow.
*/

/*!  Constructs an empty main window. */

QMainWindow::QMainWindow( QWidget * parent, const char * name, WFlags f )
    : QWidget( parent, name, f )
{
    d = new QMainWindowPrivate;
    d->opaque = FALSE;
    installEventFilter( this );
    d->topDock = new QDockArea( Horizontal, QDockArea::Normal, this, "qt_top_dock" );
    connect( d->topDock, SIGNAL( rightButtonPressed( const QPoint & ) ),
	     this, SLOT( showDockMenu( const QPoint & ) ) );
    d->bottomDock = new QDockArea( Horizontal, QDockArea::Reverse, this, "qt_bottom_dock" );
    connect( d->bottomDock, SIGNAL( rightButtonPressed( const QPoint & ) ),
	     this, SLOT( showDockMenu( const QPoint & ) ) );
    d->leftDock = new QDockArea( Vertical, QDockArea::Normal, this, "qt_left_dock" );
    connect( d->leftDock, SIGNAL( rightButtonPressed( const QPoint & ) ),
	     this, SLOT( showDockMenu( const QPoint & ) ) );
    d->rightDock = new QDockArea( Vertical, QDockArea::Reverse, this, "qt_right_dock" );
    connect( d->rightDock, SIGNAL( rightButtonPressed( const QPoint & ) ),
	     this, SLOT( showDockMenu( const QPoint & ) ) );
    d->hideDock = new QHideDock( this );
    d->hideDock->installEventFilter( this );
}


/*! Destructs the object and frees any allocated resources.

*/

QMainWindow::~QMainWindow()
{
    delete layout();
    delete d;
}

#ifndef QT_NO_MENUBAR
/*!  Sets this main window to use the menu bar \a newMenuBar.

  The old menu bar, if there was any, is deleted along with its
  contents.

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


/*!  Returns the menu bar for this window.  If there isn't any,
  menuBar() creates an empty menu bar on the fly.

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

  The old status bar, if there was any, is deleted along with its
  contents.

  Note that \a newStatusBar must be a child of this main window, and
  that it is not automatically displayed.  If you call this function
  after show(), you probably also need to call \a
  newStatusBar->show().

  Note that \a newStatusBar must be a child of this main window, and
  that it is not automatically displayed.  If you call this function
  after show(), you probably also need to call \a
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


/*!  Returns the status bar for this window.  If there isn't any,
  statusBar() creates an empty status bar on the fly, and if necessary
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

  The old tool tip group, if there was any, is deleted along with its
  contents.  All the tool tips connected to it lose the ability to
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


/*!  Returns the tool tip group for this window.  If there isn't any,
  toolTipGroup() creates an empty tool tip group on the fly.

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


/*!  Sets \a dock to be available if \a enable is TRUE, and not
  available if \a enable is FALSE.

  The user can drag a dock window to any enabled dock.
*/

void QMainWindow::setDockEnabled( Dock dock, bool enable )
{
    d->docks.replace( dock, enable );
}


/*!  Returns TRUE if \a dock is enabled, or FALSE if it is not.

  \sa setDockEnabled()
*/

bool QMainWindow::isDockEnabled( Dock dock ) const
{
    return d->docks[ dock ];
}

/*!  Returns TRUE if \a area is enabled, or FALSE if it is not.

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
  Sets \a dock to be available for the toolbar \a tb if \a enable is TRUE, and not
  available if \a enable is FALSE.

  The user can drag the toolbar to any enabled dock.
*/


void QMainWindow::setDockEnabled( QDockWindow *tb, Dock dock, bool enable )
{
    QString s;
    s.sprintf( "%p_%d", tb, (int)dock );
    if ( enable )
	d->disabledDocks.remove( s );
    else if ( d->disabledDocks.find( s ) == d->disabledDocks.end() )
	d->disabledDocks << s;
}

/*!  Returns TRUE if \a area is enabled for the dock window \a tb, or
FALSE if it is not.

  \sa setDockEnabled()
*/

bool QMainWindow::isDockEnabled( QDockWindow *tb, QDockArea *area ) const
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
    return isDockEnabled( tb, dock );
}

/*!  Returns TRUE if \a dock is enabled for the dock window \a tb , or
  FALSE if it is not.

  \sa setDockEnabled()
*/

bool QMainWindow::isDockEnabled( QDockWindow *tb, Dock dock ) const
{
    QString s;
    s.sprintf( "%p_%d", tb, (int)dock );
    return d->disabledDocks.find( s ) == d->disabledDocks.end();
}



/*!  Adds \a dockWindow to this the end of \a edge and makes it start
  a new line of dock windows if \a nl is TRUE.

  If \a dockWindow is already managed by some main window, it is first
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


/*!  Adds \a dockWindow to this the end of \a edge, labelling it \a
label and makes it start a new line of dock windows if \a newLine is
TRUE.

If \a dockWindow is already managed by some main window, it is first
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
  Moves \a dockWindow to this the end of \a edge.

  If \a dockWindow is already managed by some main window, it is moved from
  that window to this.
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

/*!
  Moves \a dockWindow to the position \a index of \a edge.

  If \a dockWindow is already managed by some main window, it is moved from
  that window to this.
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
  Removes \a dockWindow from this main window's docking area, if \a dockWindow is
  non-null and known by this main window.
*/

void QMainWindow::removeDockWindow( QDockWindow * dockWindow )
{
    dockWindow->hide();
    d->dockWindows.removeRef( dockWindow );
    disconnect( dockWindow, SIGNAL( placeChanged( QDockWindow::Place ) ),
		this, SLOT( slotPlaceChanged() ) );
    dockWindow->removeEventFilter( this );
}

/*!  Sets up the geometry management of this window.  Called
  automatically when needed, so you should never need to call this.
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

/*!  Sets the central widget for this window to \a w.  The central
  widget is the one around which the toolbars etc. are arranged.
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


/*!  Returns a pointer to the main child of this main widget.  The
  main child is the big widget around which the tool bars are
  arranged.

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

    if ( e->type() == QEvent::MouseButtonPress && d->dockMenu &&
	 ( ( o->inherits( "QDockWindow" ) && hasDockWindow( (QDockWindow*)o ) ) || o == d->hideDock ) ) {
	if ( ( (QMouseEvent*)e )->button() == RightButton ) {
	    if ( showDockMenu( ( (QMouseEvent*)e )->globalPos() ) )
		return TRUE;
	}
    }

    return QWidget::eventFilter( o, e );
}


/*!
  Monitors events to ensure layout is updated.
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


/*!  Returns the state last set by setUsesBigPixmaps().  The initial
  state is FALSE.
  \sa setUsesBigPixmaps();
*/

bool QMainWindow::usesBigPixmaps() const
{
    return d->ubp;
}


/*!  Sets tool buttons in this main windows to use big pixmaps if \a
  enable is TRUE, and small pixmaps if \a enable is FALSE.

  The default is FALSE.

  Tool buttons and other interested widgets are responsible for
  reading the correct state on startup, and for connecting to this
  widget's pixmapSizeChanged() signal.

  \sa QToolButton::setUsesBigPixmap()
*/

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

/*!  Returns the state last set by setUsesTextLabel().  The initial
  state is FALSE.
  \sa setUsesTextLabel();
*/

bool QMainWindow::usesTextLabel() const
{
    return d->utl;
}


/*!  Sets tool buttons in this main windows to use text labels if \a
  enable is TRUE, and no text labels otherwise.

  The default is FALSE.

  Tool buttons and other interested widgets are responsible for
  reading the correct state on startup, and for connecting to this
  widget's usesTextLabelChanged() signal.

  \sa QToolButton::setUsesTextLabel()
*/

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


/*! \fn void QMainWindow::pixmapSizeChanged( bool )

  This signal is called whenever the setUsesBigPixmaps() is called
  with a value which is different from the current setting.  All
  relevant widgets must connect to this signal.
*/

/*! \fn void QMainWindow::usesTextLabelChanged( bool )

  This signal is called whenever the setUsesTextLabel() is called
  with a value which is different from the current setting.  All
  relevant widgets must connect to this signal.
*/

/*!
  \fn void QMainWindow::dockWindowPositionChanged( QDockWindow *dockWindow )

  This signal is emitted when the \a dockWindow has changed its position.
  This means it has been moved to another dock or inside the dock.

  \sa getLocation()
*/

/*!  Sets this main window to right-justifies its dock windows if \a
  enable is TRUE. If enable is FALSE, only stretchable dock windows
  are expanded, while non-stretchable dock windows get just the space
  they need. Given that most dock windows are not stretchable, this
  usually results in a ragged right edge.

  The default is FALSE.

  \sa rightJustification(), QDockWindow::setVerticalStretchable(), QDockWindow::setHorizontalStretchable()
*/

void QMainWindow::setRightJustification( bool enable )
{
    if ( enable == d->justify )
	return;
    d->justify = enable;
    triggerLayout( TRUE );
}


/*!  Returns TRUE if this main windows right-justifies its dock
  windows, and FALSE if it uses a ragged right edge.

  The default is to use a ragged right edge.

  ("Right edge" sometimes means "bottom edge".)

  \sa setRightJustification()
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
    Enters What's This? question mode and returns immediately.

    This is the same as QWhatsThis::enterWhatsThisMode(), but as a slot of of a
    main window object. This way it can be easily used for popup menus
    as in the code fragment:

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

/*!  Finds and gives back the \a dock and the \a index there of the
  dock window \a tb. \a dock is set to the dock of the mainwindow in
  which \a tb is and \a index is set to the position of the dock
  window in this dock. If the dock window has a new line, \a nl is set
  to TRUE, else to FALSE.

  This method returns TRUE if the information could be found out,
  otherwise FALSE (e.g. because the dock window \a tb was not found in
  this mainwindow)
*/

bool QMainWindow::getLocation( QDockWindow *tb, Dock &dock, int &index, bool &nl, int &extraOffset ) const
{
    dock = TornOff;
    if ( d->topDock->hasDockWindow( tb, &index ) )
	dock = Top;
    else if ( d->bottomDock->hasDockWindow( tb, &index ) )
	dock = Bottom;
    else if ( d->leftDock->hasDockWindow( tb, &index ) )
	dock = Left;
    else if ( d->rightDock->hasDockWindow( tb, &index ) )
	dock = Right;
    else if ( tb->parentWidget() == d->hideDock ) {
	index = 0;
	dock = Minimized;
    } else {
	index = 0;
    }
    nl = tb->newLine();
    extraOffset = tb->offset();
    return TRUE;
}

/*!  Returns a list of all toolbars which are placed in \a dock.
*/

QList<QToolBar> QMainWindow::toolBars( Dock dock ) const
{
    QList<QDockWindow> lst = dockWindows( dock );
    QList<QToolBar> tbl;
    for ( QDockWindow *w = lst.first(); w; w = lst.next() ) {
	if ( w->inherits( "QToolBar" ) )
	    tbl.append( (QToolBar*)w );
    }
    return tbl;
}

/*!  Returns a list of all dock windows which are placed in \a dock.
*/

QList<QDockWindow> QMainWindow::dockWindows( Dock dock ) const
{
    QList<QDockWindow> lst;
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

/* Returns the list of dock windows which belong to this main window
   */

QList<QDockWindow> QMainWindow::dockWindows() const
{
    return d->dockWindows;
}

/*!  Sets the dock windows to be movable if \a enable is TRUE, or
  static otherwise.

  Movable dock windows can be dragged around between and within the
  different docks or undocked/docked from/into the mainwindow by the
  user. By default dock windows are moved transparent, but this
  setting can be changed by setOpaqueMoving().

  The default is TRUE.

  \sa setDockEnabled(), dockWindowsMovable(), setOpaqueMoving()
*/

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

/*!
  Returns whether or not the toolbars of this main window are movable.

  \sa setDockWindowsMovable()
*/

bool QMainWindow::dockWindowsMovable() const
{
    return d->movable;
}

/*!  If you set \a b to TRUE, the use can move the dock windows
  opaque, otherwise this is done transparent. This setting makes only
  sense, if dockWindowsMovable() is set to TRUE.

  \sa setDockWindowsMovable()
*/

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

/*!  Returns whether the dock windows of the mainwindow can be moved
  opaque or transparent.

  \sa setOpaqueMoving()
*/

bool QMainWindow::opaqueMoving() const
{
    return d->opaque;
}

/*!  As dock windows can be freely moved inside docks, it's possible
  to line them up nicely with this method to get rid of all the unused
  space. If \a keepNewLines is TRUE, all dock windows stay in the line
  in which they are, else they are packed together as compact as
  possible.

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

/*!  Returns TRUE, if rightclicking on a dock window or on a dock
  opens a popup menu which allows lining up dock windows,
  hiding/showing dock windows and customizing them.

  \sa setDockEnabled(), lineUpDockWindows()
*/

bool QMainWindow::isDockMenuEnabled() const
{
    return d->dockMenu;
}

/*!  When passing TRUE for \a b here, rightclicking on a dock window
  or a dock opens a popup menu which allows lining up dock windows,
  hiding/showing dock windows and customizing them.

  \sa lineUpDockWindows(), isDockMenuEnabled()
*/

void QMainWindow::setDockMenuEnabled( bool b )
{
    d->dockMenu = b;
}

/*! Shows the dock menu at the position \a globalPos which allows
  lining up dock windows, hiding/showing dock windows and customizing
  them.

  If you need a specialized popup menu here, you can reimplement that
  function.
*/

bool QMainWindow::showDockMenu( const QPoint &globalPos )
{
    if ( !d->dockMenu )
	return FALSE;
    if ( !d->rmbMenu ) {
	d->rmbMenu = new QPopupMenu( this );
	d->rmbMenu->setCheckable( TRUE );
    } else {
	d->rmbMenu->clear();
    }
    QObjectList *l = queryList( "QDockWindow" );
    QIntDict<QDockWindow> id2Widget;
    if ( l && !l->isEmpty() ) {
	for ( QObject *o = l->first(); o; o = l->next() ) {
	    QDockWindow *dw = (QDockWindow*)o;
	    QString label;
	    if ( dw->inherits( "QToolBar" ) )
		label = ( (QToolBar*)dw )->label();
	    if ( label.isEmpty() )
		label = dw->caption();
	    if ( !label.isEmpty() ) {
		int id = d->rmbMenu->insertItem( label );
		d->rmbMenu->setItemChecked( id, dw->isVisible() );
		id2Widget.insert( id, dw );
	    }
	}
    }
    delete l;

    if ( !id2Widget.isEmpty() )
	d->rmbMenu->insertSeparator();
    int lineup = -2;

    if ( dockWindowsMovable() )
	lineup = d->rmbMenu->insertItem( tr( "Line up" ) );
    int config = d->rmbMenu->insertItem( tr( "Customize..." ) );
    int result = d->rmbMenu->exec( globalPos );
    if ( result == config ) {
	qDebug( "todo: Action/Toolbar editing" );
	return TRUE;
    } else if ( result == lineup ) {
	lineUpDockWindows( TRUE );
	return TRUE;
    }

    QDockWindow *dw = 0;
    if ( !( dw = id2Widget.find( result ) ) )
	return TRUE;
    if ( dw->isVisible() )
	dw->hide();
    else
	dw->show();
    return TRUE;
}

void QMainWindow::slotPlaceChanged()
{
    if ( sender()->inherits( "QDockWindow" ) )
	emit dockWindowPositionChanged( (QDockWindow*)sender() );
    if ( sender()->inherits( "QToolBar" ) )
	emit toolBarPositionChanged( (QToolBar*)sender() );
}

/*! For internal use of QDockWindow only.
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

/*! Returns TRUE, if \a dw is a dockwindow known to the mainwindow,
otherwise FALSE.
*/

bool QMainWindow::hasDockWindow( QDockWindow *dw )
{
    return d->dockWindows.findRef( dw ) != -1;
}

/*! Returns the left dock area */

QDockArea *QMainWindow::leftDock() const
{
    return d->leftDock;
}

/*! Returns the right dock area */

QDockArea *QMainWindow::rightDock() const
{
    return d->rightDock;
}

/*! Returns the top dock area */

QDockArea *QMainWindow::topDock() const
{
    return d->topDock;
}

/*! Returns the bottom dock area */

QDockArea *QMainWindow::bottomDock() const
{
    return d->bottomDock;
}

#ifndef QT_NO_TEXTSTREAM
static void saveDockArea( QTextStream &ts, QDockArea *a )
{
    QList<QDockWindow> l = a->dockWindowList();
    for ( QDockWindow *dw = l.first(); dw; dw = l.next() ) {
	ts << QString( dw->caption() );
	ts << ",";
    }
    ts << endl;
    ts << *a;
}

/* Writes the layout of the dock windows in the dock areas of the \a
   mainWindow to the text stream \a ts.*/

QTextStream &operator<<( QTextStream &ts, const QMainWindow &mainWindow )
{
    QList<QDockWindow> l = mainWindow.dockWindows( Qt::Minimized );
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

static void loadDockArea( const QStringList &names, QDockArea *a, Qt::Dock d, QList<QDockWindow> &l, QMainWindow *mw, QTextStream &ts )
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

/* Reads the layout description of the dock windows in the dock areas
   of the \a mainWindow from the text stream \a ts and restores it. */

QTextStream &operator>>( QTextStream &ts, QMainWindow &mainWindow )
{
    QList<QDockWindow> l = mainWindow.dockWindows();

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
