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

// ####################
/*
   -- Not working yet: --
   - dockwidgets movable settings
   - opaque moving
   - linup dockwidgets

   -- Implemented but not tested --
   - setDockEnabled, isDockEnabled
*/
// ####################



#include "qmainwindow.h"
#ifndef QT_NO_COMPLEXWIDGETS

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

    QList<QDockWidget> dockWidgets;
    QMap<Qt::Dock, bool> docks;
    QStringList disabledDocks;
    bool dockMenu;

    QPopupMenu *rmbMenu;

};


// NOT REVISED
/*! \class QMainWindow qmainwindow.h

  \brief The QMainWindow class provides a typical application window,
  with a menu bar, some tool bars and a status bar.

  \ingroup application

  In addition, you need the large central widget, which you supply and
  tell QMainWindow about using setCentralWidget(), and perhaps a few
  tool bars, which you can add using addToolBar().

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

  The QMainWindow allows by default toolbars in all docking areas.
  You can use setDockEnabled() to enable and disable docking areas
  for toolbars. Currently, only \c Top, \c Left, \c Right, \c Bottom
  and \c Minimized are meaningful.

  Several functions let you change the appearance of a QMainWindow
  globally: <ul>
  <li> setRightJustification() determines whether QMainWindow
  should ensure that the toolbars fill the available space
  (see also QToolBar::setHorizontalStretchable() and QToolBar::setVerticalStretchable()),
  <li>  setUsesBigPixmaps() determines whether QToolButton (and other
  classes) should draw small or large pixmaps (see QIconSet for more
  about that),
  <li> setUsesTextLabel() determines whether the toolbar buttons (and
  other classes), should display a textlabel in addition to pixmaps (see
  QToolButton for more about that).
  </ul>

  Toolbars can be dragged by the user into each enabled docking area
  and inside each docking area to change the order of the toolbars
  there. This feature can be enabled and disabled using setToolBarsMovable().
  By default this feature is enabled. If the \c Minimized dock is enabled the user
  can hide(minimize)/show(restore) a toolbar with a click on the toolbar handle. The handles of
  all minimized toolbars are drawn below the menu bar in one row, and if the user
  moves the mouse cursor onto such a handle, the label of the toolbar
  is displayed in a tool tip (see QToolBar::label()). So if you enable the Minimized dock,
  you should specify a meaningful label for each toolbar.

  Normally toolbars are moved transparently (this means while the user
  drags one, a rectangle is drawn on the screen). With setOpaqueMoving()
  it's possible to switch between opaque and transparent moving
  of toolbars.

  The main window's menubar is static (on the top) by default. If you want a movable
  menubar, create a QMenuBar as stretchable widget inside its
  own movable toolbar and restrict this toolbar to only live within the
  Top or Bottom dock:
  \code
  QToolBar *tb = new QToolBar( this );
  addToolBar( tb, tr( "Menubar" ), Top, FALSE );
  QMenuBar *mb = new QMenuBar( tb );
  mb->setFrameStyle( QFrame::NoFrame );
  tb->setStretchableWidget( mb );
  setDockEnabled( tb, Left, FALSE );
  setDockEnabled( tb, Right, FALSE );
  \endcode

  An application with multiple toolbars can choose to save the current
  toolbar layout in order to restore it in the next session. To do so,
  use getLocation() on each toolbar, store the data and restore the
  layout using moveToolBar() on each toolbar again. When restoring,
  ensure to move the toolbars in exactly the same order in which you
  got the information.

  For multi-document interfaces (MDI), use a QWorkspace as central
  widget.

  <img src=qmainwindow-m.png> <img src=qmainwindow-w.png>

  \sa QToolBar QStatusBar QMenuBar QToolTipGroup QDialog
*/

/*!
  \enum QMainWindow::Dock

  Each toolbar can be in one of the following positions:
  <ul>
    <li>\c Top - above the central widget, below the menubar.
    <li>\c Bottom - below the central widget, above the status bar.
    <li>\c Left - to the left of the central widget.
    <li>\c Right - to the left of the central widget.
    <li>\c Minimized - the toolbar is not shown - all handles of minimized toolbars
    are drawn in one row below the menu bar.
  </ul>

  Other values are also defined for future expansion.
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
}


/*! Destructs the object and frees any allocated resources.

*/

QMainWindow::~QMainWindow()
{
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

  The user can drag a toolbar to any enabled dock.
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

/*!
  Sets \a dock to be available for the toolbar \a tb if \a enable is TRUE, and not
  available if \a enable is FALSE.

  The user can drag the toolbar to any enabled dock.
*/


void QMainWindow::setDockEnabled( QDockWidget *tb, Dock dock, bool enable )
{
    QString s;
    s.sprintf( "%p_%d", tb, (int)dock );
    if ( enable )
	d->disabledDocks.remove( s );
    else if ( d->disabledDocks.find( s ) == d->disabledDocks.end() )
	d->disabledDocks << s;
}

/*!
  Returns TRUE if \a dock is enabled for the toolbar \a tb , or FALSE if it is not.

  \sa setDockEnabled()
*/

bool QMainWindow::isDockEnabled( QDockWidget *tb, Dock dock ) const
{
    QString s;
    s.sprintf( "%p_%d", tb, (int)dock );
    return d->disabledDocks.find( s ) == d->disabledDocks.end();
}



/*!  Adds \a toolBar to this the end of \a edge and makes it start a
new line of tool bars if \a nl is TRUE.

If \a toolBar is already managed by some main window, it is first
removed from that window.
*/

void QMainWindow::addDockWidget( QDockWidget * toolBar,
			      Dock edge, bool newLine )
{
    moveDockWidget( toolBar, edge );
    toolBar->setNewLine( newLine );
    if ( d->dockWidgets.find( toolBar ) == -1 )
	d->dockWidgets.append( toolBar );
    connect( toolBar, SIGNAL( positionChanged() ),
	     this, SLOT( slotPositionChanged() ) );
    toolBar->installEventFilter( this );
}


/*!  Adds \a toolBar to this the end of \a edge, labelling it \a label
and makes it start a new line of tool bars if \a newLine is TRUE.

If \a toolBar is already managed by some main window, it is first
removed from that window.
*/

void QMainWindow::addDockWidget( QDockWidget * toolBar, const QString &label,
			      Dock edge, bool newLine )
{
    addDockWidget( toolBar, edge, newLine );
    if ( toolBar->inherits( "QToolBar" ) )
	( (QToolBar*)toolBar )->setLabel( label );
}

/*!
  Moves \a toolBar to this the end of \a edge.

  If \a toolBar is already managed by some main window, it is moved from
  that window to this.
*/

void QMainWindow::moveDockWidget( QDockWidget * toolBar, Dock edge )
{
    toolBar->removeFromDock();
    switch ( edge ) {
    case Top:
	d->topDock->addDockWidget( toolBar );
	break;
    case Bottom:
	d->bottomDock->addDockWidget( toolBar );
	break;
    case Right:
	d->rightDock->addDockWidget( toolBar );
	break;
    case Left:
	d->leftDock->addDockWidget( toolBar );
	break;
    case TornOff:
	toolBar->undock();
	break;
    case Minimized:
	qDebug( "todo Minimize" );
	break;
    case Unmanaged:
	break;
    }
    if ( toolBar->inherits( "QToolBar" ) )
	( (QToolBar*)toolBar )->setOrientation( toolBar->orientation() );
}

/*!
  Moves \a toolBar to the position \a index of \a edge.

  If \a toolBar is already managed by some main window, it is moved from
  that window to this.
*/

void QMainWindow::moveDockWidget( QDockWidget * toolBar, Dock edge, bool nl, int index, int extraOffset )
{
    toolBar->setNewLine( nl );
    toolBar->setOffset( extraOffset );
    toolBar->removeFromDock();
    switch ( edge ) {
    case Top:
	d->topDock->addDockWidget( toolBar, index );
	break;
    case Bottom:
	d->bottomDock->addDockWidget( toolBar, index );
	break;
    case Right:
	d->rightDock->addDockWidget( toolBar, index );
	break;
    case Left:
	d->leftDock->addDockWidget( toolBar, index );
	break;
    case TornOff:
	toolBar->undock();
	break;
    case Minimized:
	qDebug( "todo Minimize" );
	break;
    case Unmanaged:
	break;
    }
    if ( toolBar->inherits( "QToolBar" ) )
	( (QToolBar*)toolBar )->setOrientation( toolBar->orientation() );
}

/*!
  Removes \a toolBar from this main window's docking area, if \a toolBar is
  non-null and known by this main window.
*/

void QMainWindow::removeDockWidget( QDockWidget * toolBar )
{
    toolBar->hide();
    d->dockWidgets.removeRef( toolBar );
    disconnect( toolBar, SIGNAL( positionChanged() ),
		this, SLOT( slotPositionChanged() ) );
    toolBar->removeEventFilter( this );
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

    if ( !d->topDock->isEmpty() && style() == WindowsStyle )
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

    if ( e->type() == QEvent::MouseButtonPress &&
	 o->inherits( "QDockWidget" ) && d->dockMenu ) {
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
	    removeDockWidget( (QDockWidget *)(e->child()) );
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
  \fn void QMainWindow::toolBarPositionChanged( QDockWidget *toolbar )

  This signal is emitted when the \a toolbar has changed its position.
  This means it has been moved to another dock or inside the dock.

  \sa getLocation()
*/

/*!
  Sets this main window to right-justifies its toolbars if \a enable
  is TRUE. If enable is FALSE, only stretchable toolbars are expanded,
  while non-stretchable toolbars get just the space they need. Given
  that most toolbars are not stretchable, this usually results in a
  ragged right edge.

  The default is FALSE.

  \sa rightJustification(), QDockWidget::setVerticalStretchable(), QDockWidget::setHorizontalStretchable()
*/

void QMainWindow::setRightJustification( bool enable )
{
    if ( enable == d->justify )
	return;
    d->justify = enable;
    triggerLayout( TRUE );
}


/*!  Returns TRUE if this main windows right-justifies its toolbars, and
  FALSE if it uses a ragged right edge.

  The default is to use a ragged right edge.

  ("Right edge" sometimes means "bottom edge".)

  \sa setRightJustification()
*/

bool QMainWindow::rightJustification() const
{
    return d->justify;
}


void QMainWindow::triggerLayout( bool deleteLayout )
{
    if ( !deleteLayout && d->tll ) {
    } else {
	delete d->tll;
	d->tll = 0;
	setUpLayout();
    }
    QApplication::postEvent( this, new QEvent( QEvent::LayoutHint ) );
    d->topDock->updateLayout();
    d->bottomDock->updateLayout();
    d->rightDock->updateLayout();
    d->leftDock->updateLayout();
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

/*!
  Finds and gives back the \a dock and the \a index there of the toolbar \a tb. \a dock is
  set to the dock of the mainwindow in which \a tb is and \a index is set to the
  position of the toolbar in this dock. If the toolbar has a new line, \a nl is set to TRUE,
  else to FALSE.

  This method returns TRUE if the information could be found out, otherwise FALSE
  (e.g. because the toolbar \a tb was not found in this mainwindow)
*/

bool QMainWindow::getLocation( QDockWidget *tb, Dock &dock, int &index, bool &nl, int &extraOffset ) const
{
    dock = TornOff;
    if ( d->topDock->hasDockWidget( tb, &index ) )
	dock = Top;
    else if ( d->bottomDock->hasDockWidget( tb, &index ) )
	dock = Bottom;
    else if ( d->leftDock->hasDockWidget( tb, &index ) )
	dock = Left;
    else if ( d->rightDock->hasDockWidget( tb, &index ) )
	dock = Right;
    else
	index = 0;
    nl = tb->newLine();
    extraOffset = tb->offset();
    return TRUE;
}

QList<QToolBar> QMainWindow::toolBars( Dock dock ) const
{
    QList<QDockWidget> lst = dockWidgets( dock );
    QList<QToolBar> tbl;
    for ( QDockWidget *w = lst.first(); w; w = lst.next() ) {
	if ( w->inherits( "QToolBar" ) )
	    tbl.append( (QToolBar*)w );
    }
    return tbl;
}

/*!
  \fn QList<QDockWidget> QMainWindow::toolBars( Dock dock ) const

  Returns a list of all toolbars which are placed in \a dock.
*/

QList<QDockWidget> QMainWindow::dockWidgets( Dock dock ) const
{
    QList<QDockWidget> lst;
    switch ( dock ) {
    case Top:
	return d->topDock->dockWidgetList();
    case Bottom:
	return d->bottomDock->dockWidgetList();
    case Left:
	return d->leftDock->dockWidgetList();
    case Right:
	return d->rightDock->dockWidgetList();
    case TornOff:
	{
	    for ( QDockWidget *w = d->dockWidgets.first(); w; w = d->dockWidgets.last() ) {
		if ( !w->area() && w->place() == QDockWidget::OutsideDock )
		    lst.append( w );
	    }
	}
	return lst;
    case Minimized:
	qWarning( "todo minimized" );
	break;
    default:
	break;
    }
    return lst;
}

/*!
  Sets the toolbars to be movable if \a enable is TRUE, or static
  otherwise.

  Movable toolbars can be dragged around between and within the
  different toolbar docks by the user. By default toolbars are moved
  transparent, but this setting can be changed by setOpaqueMoving().

  The default is TRUE.

  \sa setDockEnabled(), toolBarsMovable(), setOpaqueMoving()
*/

void QMainWindow::setDockWidgetsMovable( bool enable )
{
    d->movable = enable;
    QObjectList *l = queryList( "QDockWidget" );
    if ( l ) {
	for ( QObject *o = l->first(); o; o = l->next() )
	    ( (QDockWidget*)o )->update();
    }
    delete l;
    triggerLayout( TRUE );
}

/*!
  Returns whether or not the toolbars of this main window are movable.

  \sa setDockWidgetsMovable()
*/

bool QMainWindow::dockWidgetsMovable() const
{
    return d->movable;
}

/*!
  If you set \a b to TRUE, the use can move the
  toolbars opaque, otherwise this is done transparent. This
  setting makes only sense, if toolBarsMovable() is set to TRUE.

  \sa setToolbarsMovable()
*/

void QMainWindow::setOpaqueMoving( bool b )
{
    d->opaque = b;
}

/*!
  Returns whether the toolbars of the mainwindow can
  be moved opaque or transparent.

  \sa setOpaqueMoving()
*/

bool QMainWindow::opaqueMoving() const
{
    return d->opaque;
}

/*!
  As toolbars can be freely moved inside docks, it's possible to line them
  up nicely with this method to get rid of all the unused space. If \a keepNewLines
  is TRUE, all toolbars stay in the line in which they are, else they are packed
  together as compact as possible.

  The method only works if movable() returns TRUE.
*/

void QMainWindow::lineUpDockWidgets( bool keepNewLines )
{
    d->topDock->lineUp( keepNewLines );
    d->leftDock->lineUp( keepNewLines );
    d->rightDock->lineUp( keepNewLines );
    d->bottomDock->lineUp( keepNewLines );
}

/*!
  Returns TRUE, if rightclicking on an empty space on a toolbar dock
  or rightclicking on a toolbar handle opens a popup menu which allows
  lining up toolbars and hiding/showing toolbars.

  \sa setDockEnabled(), lineUpDockWidgets()
*/

bool QMainWindow::isDockMenuEnabled() const
{
    return d->dockMenu;
}

/*!
  When passing TRUE for \a b here, rightclicking on an empty space on a toolbar dock
  or rightclicking on a toolbar handle opens a popup menu which allows lining up toolbars
  and hiding/showing toolbars.

  \sa lineUpDockWidgets(), isDockMenuEnabled()
*/

void QMainWindow::setDockMenuEnabled( bool b )
{
    d->dockMenu = b;
}

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
    QObjectList *l = queryList( "QDockWidget" );
    QIntDict<QDockWidget> id2Widget;
    if ( l && !l->isEmpty() ) {
	for ( QObject *o = l->first(); o; o = l->next() ) {
	    QDockWidget *dw = (QDockWidget*)o;
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
    if ( !id2Widget.isEmpty() )
	d->rmbMenu->insertSeparator();
    int config = d->rmbMenu->insertItem( tr( "Customize..." ) );
    int result = d->rmbMenu->exec( globalPos );
    if ( result == config ) {
	qDebug( "todo: Action/Toolbar editing" );
	return TRUE;
    }

    QDockWidget *dw = 0;
    if ( !( dw = id2Widget.find( result ) ) )
	return TRUE;
    if ( dw->isVisible() )
	dw->hide();
    else
	dw->show();
    return TRUE;
}

void QMainWindow::slotPositionChanged()
{
    if ( sender()->inherits( "QDockWidget" ) )
	emit dockWidgetPositionChanged( (QDockWidget*)sender() );
    if ( sender()->inherits( "QToolBar" ) )
	emit toolBarPositionChanged( (QToolBar*)sender() );
}

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

#include "qmainwindow.moc"

#endif
