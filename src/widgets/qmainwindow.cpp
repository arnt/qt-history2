/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qmainwindow.cpp#2 $
**
** Implementation of something useful.
**
** Created : 979899
**
** Copyright (C) 1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qmainwindow.h"

#include "qlist.h"

#include "qtimer.h"
#include "qlayout.h"

#include "qmenubar.h"
#include "qtoolbar.h"
#include "qstatusbar.h"

#include "qtooltip.h"

RCSTAG("$Id: //depot/qt/main/src/widgets/qmainwindow.cpp#2 $");


class QMainWindowPrivate {
public:
    struct ToolBar {
	ToolBar() : t(0), nl(FALSE) {}
	ToolBar( QToolBar * tb, bool n=FALSE ): t(tb), nl(n) {}
	QToolBar * t;
	bool nl;
    };
	
    typedef QList<ToolBar> ToolBarDock;

    QMainWindowPrivate()
	: top(0), left(0), right(0), bottom(0),
	  mb(0), sb(0), ttg(0), mc(0), timer(0), tll(0),
	  ubp(FALSE)
    {
	// nothing
    }

    ~QMainWindowPrivate()
    {
	if ( top ) {
	    top->setAutoDelete( TRUE );
	    delete top;
	}
	if ( left ) {
	    left->setAutoDelete( TRUE );
	    delete left;
	}
	if ( right ) {
	    right->setAutoDelete( TRUE );
	    delete right;
	}
	if ( bottom ) {
	    bottom->setAutoDelete( TRUE );
	    delete bottom;
	}
    }

    ToolBarDock * top, * left, * right, * bottom;

    QMenuBar * mb;
    QStatusBar * sb;
    QToolTipGroup * ttg;

    QWidget * mc;

    QTimer * timer;

    QBoxLayout * tll;

    bool ubp;
};


/*!  Constructs an empty main window. */

QMainWindow::QMainWindow( QWidget * parent, const char * name )
    : QWidget( parent, name )
{
    d = new QMainWindowPrivate;
    d->timer = new QTimer( this );
    connect( d->timer, SIGNAL(timeout()),
	     this, SLOT(setUpLayout()) );
}


/*! Destroys the object and frees any allocated resources.

*/

QMainWindow::~QMainWindow()
{
    delete d;
}


/*!  Sets this main window to use the menu bar \a newMenuBar.

  The old menu bar, if there was any, is deleted along with its
  contents.

  \sa setStatusBar() menuBar()
*/

void QMainWindow::setMenuBar( QMenuBar * newMenuBar )
{
    if ( !newMenuBar )
	return;
    if ( d->mb )
	delete d->mb;
    d->mb = newMenuBar;
    d->timer->start( 0, TRUE );
}


/*!  Returns the menu bar for this window.  If there isn't any,
  menuBar() creates an empty menu bar on the fly.

  \sa setMenuBar() statusBar()
*/

QMenuBar * QMainWindow::menuBar() const
{
    if ( d->mb )
	return d->mb;

    QMenuBar * b = new QMenuBar( (QMainWindow *)this, "automatic menu bar" );
    ((QMainWindowPrivate*)d)->mb = b;
    d->timer->start( 0, TRUE );
    return b;
}


/*!  Sets this main window to use the status bar \a newStatusBar.

  The old status bar, if there was any, is deleted along with its
  contents.

  \sa setMenuBar() statusBar()
*/

void QMainWindow::setStatusBar( QStatusBar * newStatusBar )
{
    if ( !newStatusBar || newStatusBar == d->sb )
	return;
    if ( d->sb )
	delete d->sb;
    d->sb = newStatusBar;
    connect( toolTipGroup(), SIGNAL(showTip(const char *)),
	     d->sb, SLOT(message(const char *)) );
    connect( toolTipGroup(), SIGNAL(removeTip()),
	     d->sb, SLOT(clear()) );
    d->timer->start( 0, TRUE );
}


/*!  Returns the status bar for this window.  If there isn't any,
  menuBar() creates an empty status bar on the fly, and if necessary a
  tool tip group too.

  \sa setMenuBar() statusBar() toolTipGroup()
*/

QStatusBar * QMainWindow::statusBar() const
{
    if ( d->sb )
	return d->sb;

    QStatusBar * s = new QStatusBar( (QMainWindow *)this,
				     "automatic status bar" );
    s->setAutoMinimumSize( TRUE );
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

    connect( toolTipGroup(), SIGNAL(showTip(const char *)),
	     statusBar(), SLOT(message(const char *)) );
    connect( toolTipGroup(), SIGNAL(removeTip()),
	     statusBar(), SLOT(clear()) );
    d->timer->start( 0, TRUE );
}


/*!  Returns the tool tip group for this window.  If there isn't any,
  menuBar() creates an empty tool tip group on the fly.

  \sa setMenuBar() setStatusBar()
*/

QToolTipGroup * QMainWindow::toolTipGroup() const
{
    if ( d->ttg )
	return d->ttg;

    QToolTipGroup * t = new QToolTipGroup( (QMainWindow*)this,
					   "automatic tool tip group" );
    ((QMainWindowPrivate*)d)->ttg = t;
    d->timer->start( 0, TRUE );
    return t;
}


/*!  Sets \a dock to be available if \a enable is TRUE, and not
  available if \a enable is FALSE.

  The user can drag a toolbar to any enabled dock.

*/

void QMainWindow::setDockEnabled( ToolBarDock dock, bool enable )
{
    if ( enable ) {
	switch ( dock ) {
	case Top:
	    if ( !d->top )
		d->top = new QMainWindowPrivate::ToolBarDock();
	    break;
	case Left:
	    if ( !d->left )
		d->left = new QMainWindowPrivate::ToolBarDock();
	    break;
	case Right:
	    if ( !d->right )
		d->right = new QMainWindowPrivate::ToolBarDock();
	    break;
	case Bottom:
	    if ( !d->bottom )
		d->bottom = new QMainWindowPrivate::ToolBarDock();
	    break;
	}
    } else {
	warning( "ooop! unimplemented, untested, and not quite thought out." );
    }
}


/*!

*/

bool QMainWindow::isDockEnabled( ToolBarDock dock ) const
{
    switch ( dock ) {
    case Top:
	return d->top != 0;
    case Left:
	return d->left != 0;
    case Right:
	return d->right != 0;
    case Bottom:
	return d->bottom != 0;
    }
}


/*!  Adds \a toolbar to this the \a edge window of this window.

*/

void QMainWindow::addToolBar( QToolBar * toolBar, ToolBarDock edge, bool nl )
{
    if ( !toolBar )
	return;

    setDockEnabled( edge, TRUE );
    
    QMainWindowPrivate::ToolBarDock * dl = 0;
    if ( edge == Top ) {
	dl = d->top;
	toolBar->setOrientation( QToolBar::Horizontal );
    } else if ( edge == Left ) {
	dl = d->left;
	toolBar->setOrientation( QToolBar::Vertical );
    } else if ( edge == Bottom ) {
	dl = d->bottom;
	toolBar->setOrientation( QToolBar::Horizontal );
    } else if ( edge == Right ) {
	dl = d->right;
	toolBar->setOrientation( QToolBar::Vertical );
    }

    if ( !dl )
	return;

    dl->append( new QMainWindowPrivate::ToolBar( toolBar, nl ) );
    d->timer->start( 0, TRUE );

    connect( this, SIGNAL(internalUseBigPixmaps(bool)),
	     toolBar, SIGNAL(useBigPixmaps(bool)) );
}



static bool removeToolBarFromDock( QToolBar * t,
				   QMainWindowPrivate::ToolBarDock *l )
{
    if ( !l )
	return FALSE;
    QMainWindowPrivate::ToolBar * ct = l->first();
    do {
	if ( ct->t == t ) {
	    l->take();
	    delete ct;
	    return TRUE;
	}
    } while( (ct=l->next()) != 0 );
    return FALSE;
}


/*!

*/

void QMainWindow::removeToolBar( QToolBar * toolBar )
{
    if ( !toolBar )
	return;
    if ( !removeToolBarFromDock( toolBar, d->top ) &&
	 !removeToolBarFromDock( toolBar, d->left ) &&
	 !removeToolBarFromDock( toolBar, d->right ) &&
	 !removeToolBarFromDock( toolBar, d->bottom ) ) {
	debug( "QMainWindow::removeToolBar() (%s) not managing %p (%s/%s)",
	       name(), toolBar, toolBar->name(), toolBar->className() );
    } else {
	d->timer->start( 0, TRUE );
    }
}


static void addToolBarToLayout( QMainWindowPrivate::ToolBarDock * dock,
				QBoxLayout * tl,
				QBoxLayout::Direction direction,
				QBoxLayout::Direction dockDirection,
				bool mayNeedDockLayout )
{
    bool moreThanOneRow = FALSE;
    if ( !dock || dock->isEmpty() )
	return;
    dock->first();
    while ( dock->next() ) {
	if ( dock->current()->nl ) {
	    moreThanOneRow = TRUE;
	    break;
	}
    }

    QBoxLayout * dockLayout;
    if ( mayNeedDockLayout && moreThanOneRow ) {
	dockLayout = new QBoxLayout( dockDirection );
	tl->addLayout( dockLayout );
    } else {
	dockLayout = tl;
    }

    QBoxLayout * toolBarRowLayout = 0;
    QMainWindowPrivate::ToolBar * t = dock->first();
    do {
	if ( !toolBarRowLayout || t->nl ) {
	    if ( toolBarRowLayout )
		toolBarRowLayout->addStretch( 1 );
	    toolBarRowLayout = new QBoxLayout( direction );
	    dockLayout->addLayout( toolBarRowLayout, 0 );
	}
	toolBarRowLayout->addWidget( t->t, 0 );
    } while ( (t=dock->next()) != 0 );
    toolBarRowLayout->addStretch( 1 );
}


/*!  Sets up the \link QBoxLayout geometry management \endlink of this
  window.  Called automatically when needed, so you should never need
  to call this.
*/

void QMainWindow::setUpLayout()
{
    d->timer->stop();
    delete d->tll;
    d->tll = new QBoxLayout( this, QBoxLayout::Down );
    d->tll->setMenuBar( menuBar() );
    addToolBarToLayout( d->top, d->tll,
			QBoxLayout::LeftToRight, QBoxLayout::Down, FALSE );
    QBoxLayout * mwl = new QBoxLayout( QBoxLayout::LeftToRight );
    d->tll->addLayout( mwl, 1 );
    addToolBarToLayout( d->left, mwl,
			QBoxLayout::Down, QBoxLayout::LeftToRight, FALSE );
    if ( centralWidget() )
	mwl->addWidget( centralWidget(), 1 );
    else
	mwl->addStretch( 1 );
    addToolBarToLayout( d->right, mwl,
			QBoxLayout::Down, QBoxLayout::LeftToRight, TRUE );
    addToolBarToLayout( d->bottom, d->tll,
			QBoxLayout::LeftToRight, QBoxLayout::Up, TRUE );
    d->tll->addWidget( statusBar(), 0 );
    d->tll->activate();
}


/*!  \reimp */

void QMainWindow::show()
{
    setUpLayout();
    emit internalUseBigPixmaps( d->ubp );
    QWidget::show();
}


/*!

*/

void QMainWindow::setCentralWidget( QWidget * w )
{
    d->mc = w;
    d->timer->start( 0, TRUE );
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


/*!  Sets the tool bars to use big (32 by 32 pixels) if \a enable is
  TRUE, and to use small (16 by 16 pixels) if \a enable is FALSE.
*/

void QMainWindow::setUsesBigPixmaps( bool enable )
{
    d->ubp = enable;
    emit internalUseBigPixmaps( enable );
}


/*!  Returns TRUE if the tool bars managed by this window use big
  pixmaps, else FALSE.

  \sa setUsesBigPixmaps()
*/

bool QMainWindow::usesBigPixmaps() const
{
    return d->ubp;
}
