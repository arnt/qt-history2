/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qmainwindow.cpp#18 $
**
** Implementation of QMainWindow class
**
** Created : 980312
**
** Copyright (C) 1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qmainwindow.h"

#include "qlist.h"

#include "qtimer.h"
#include "qlayout.h"
#include "qobjcoll.h"

#include "qpainter.h"

#include "qmenubar.h"
#include "qtoolbar.h"
#include "qstatusbar.h"

#include "qtooltip.h"

RCSTAG("$Id: //depot/qt/main/src/widgets/qmainwindow.cpp#18 $");

/*notready
  \class QMainWindow
  \brief ...
  \ingroup realwidgets
  \ingroup application

  ...
*/

class QMainWindowPrivate {
public:
    struct ToolBar {
	ToolBar() : t(0), l(0), nl(FALSE) {}
	ToolBar( QToolBar * tb, const char * label, bool n=FALSE )
	    : t(tb), l(label), nl(n) {}
	QToolBar * t;
	QString l;
	bool nl;
    };
	
    typedef QList<ToolBar> ToolBarDock;

    QMainWindowPrivate()
	: top(0), left(0), right(0), bottom(0), tornOff(0), unmanaged(0),
	  mb(0), sb(0), ttg(0), mc(0), timer(0), tll(0), ubp( FALSE ),
	  justify( FALSE )
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
	if ( tornOff ) {
	    tornOff->setAutoDelete( TRUE );
	    delete tornOff;
	}
	if ( unmanaged ) {
	    unmanaged->setAutoDelete( TRUE );
	    delete unmanaged;
	}
    }

    ToolBarDock * top, * left, * right, * bottom, * tornOff, * unmanaged;

    QMenuBar * mb;
    QStatusBar * sb;
    QToolTipGroup * ttg;

    QWidget * mc;

    QTimer * timer;

    QBoxLayout * tll;

    bool ubp;
    bool justify;
};


/*!  Constructs an empty main window. */

QMainWindow::QMainWindow( QWidget * parent, const char * name, WFlags f )
    : QWidget( parent, name, f )
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
    d->mb->installEventFilter( this );
    triggerLayout();
}


/*!  Returns the menu bar for this window.  If there isn't any,
  menuBar() creates an empty menu bar on the fly.

  \sa setMenuBar() statusBar()
*/

QMenuBar * QMainWindow::menuBar() const
{
    if ( d->mb )
	return d->mb;

    QObjectList * l
	= ((QObject*)this)->queryList( "QMenuBar", 0, FALSE, FALSE );
    QMenuBar * b;
    if ( l && l->count() )
	b = (QMenuBar *)l->first();
    else
	b = new QMenuBar( (QMainWindow *)this, "automatic menu bar" );
    delete l;
    ((QMainWindowPrivate*)d)->mb = b;
    d->mb->installEventFilter( this );
    ((QMainWindow *)this)->triggerLayout();
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
    d->sb->installEventFilter( this );
    triggerLayout();
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

    QObjectList * l
	= ((QObject*)this)->queryList( "QStatusBar", 0, FALSE, FALSE );
    QStatusBar * s;
    if ( l && l->count() )
	s = (QStatusBar *)l->first();
    else
	s = new QStatusBar( (QMainWindow *)this, "automatic status bar" );
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

    connect( toolTipGroup(), SIGNAL(showTip(const char *)),
	     statusBar(), SLOT(message(const char *)) );
    connect( toolTipGroup(), SIGNAL(removeTip()),
	     statusBar(), SLOT(clear()) );
    triggerLayout();
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
    ((QMainWindow *)this)->triggerLayout();
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
	case TornOff:
	    if ( !d->tornOff )
		d->tornOff = new QMainWindowPrivate::ToolBarDock();
	    break;
	case Unmanaged:
	    if ( !d->unmanaged )
		d->unmanaged = new QMainWindowPrivate::ToolBarDock();
	    break;
	}
    } else {
	warning( "oops! unimplemented, untested, and not quite thought out." );
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
    case TornOff:
	return d->tornOff != 0;
    case Unmanaged:
	return d->unmanaged != 0;
    }
    return FALSE; // for illegal values of dock
}


/*!  Adds \a toolbar to this the \a edge window of this window.

*/

void QMainWindow::addToolBar( QToolBar * toolBar, const char * label,
			      ToolBarDock edge, bool nl )
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
    } else if ( edge == TornOff ) {
	dl = d->tornOff;
    } else if ( edge == Unmanaged ) {
	dl = d->unmanaged;
    }

    if ( !dl )
	return;

    dl->append( new QMainWindowPrivate::ToolBar( toolBar, label, nl ) );
    triggerLayout();
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


/*!  Removes \a toolBar from this main window, if \a toolBar is
  non-null and known by this main window.
*/

void QMainWindow::removeToolBar( QToolBar * toolBar )
{
    if ( !toolBar )
	return;
    if ( removeToolBarFromDock( toolBar, d->top ) ||
	 removeToolBarFromDock( toolBar, d->left ) ||
	 removeToolBarFromDock( toolBar, d->right ) ||
	 removeToolBarFromDock( toolBar, d->bottom ) ||
	 removeToolBarFromDock( toolBar, d->tornOff ) ||
	 removeToolBarFromDock( toolBar, d->unmanaged ) )
	triggerLayout();
}


static void addToolBarToLayout( QMainWindowPrivate::ToolBarDock * dock,
				QBoxLayout * tl,
				QBoxLayout::Direction direction,
				QBoxLayout::Direction dockDirection,
				bool mayNeedDockLayout,
				bool justify,
				GUIStyle style )
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
	    if ( toolBarRowLayout ) {
		if ( !justify )
		    toolBarRowLayout->addStretch( 1 );
	    }
	    toolBarRowLayout = new QBoxLayout( direction );
	    dockLayout->addLayout( toolBarRowLayout, 0 );
	}
	toolBarRowLayout->addWidget( t->t, 0 );
    } while ( (t=dock->next()) != 0 );

    if ( !toolBarRowLayout )
	return;

    if ( style == MotifStyle )
	dockLayout->addSpacing( 2 );

    if ( !justify )
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
    if ( d->mb )
	d->tll->setMenuBar( d->mb );
    if ( style() == WindowsStyle )
	d->tll->addSpacing( 1 );
    addToolBarToLayout( d->top, d->tll,
			QBoxLayout::LeftToRight, QBoxLayout::Down, FALSE,
			d->justify, style() );
    QBoxLayout * mwl = new QBoxLayout( QBoxLayout::LeftToRight );
    d->tll->addLayout( mwl, 1 );
    addToolBarToLayout( d->left, mwl,
			QBoxLayout::Down, QBoxLayout::LeftToRight, FALSE,
			d->justify, style() );
    if ( centralWidget() )
	mwl->addWidget( centralWidget(), 1 );
    else
	mwl->addStretch( 1 );
    addToolBarToLayout( d->right, mwl,
			QBoxLayout::Down, QBoxLayout::LeftToRight, TRUE,
			d->justify, style() );
    addToolBarToLayout( d->bottom, d->tll,
			QBoxLayout::LeftToRight, QBoxLayout::Up, TRUE,
			d->justify, style() );
    if ( d->sb )
	d->tll->addWidget( d->sb, 0 );
    d->tll->activate();
}


/*!  \reimp */

void QMainWindow::show()
{
    setUpLayout();
    QWidget::show();
}


/*!  Sets the central widget for this window to \a w.  The centail
  widget is the one around which the toolbars etc. are arranged.
*/

void QMainWindow::setCentralWidget( QWidget * w )
{
    d->mc = w;
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
    if ( style() == WindowsStyle ) {
	QPainter p( this );
	int y = menuBar()->height();
	p.setPen( colorGroup().dark() );
	p.drawLine( 0, y, width()-1, y );
    }
}


/*!
  Monitors events to ensure layout is updated.  
*/

bool QMainWindow::eventFilter( QObject* o, QEvent *e )
{
    if ( e->type() == Event_Show
      || e->type() == Event_Hide )
	triggerLayout();
    return QWidget::eventFilter(o,e);
}

/*!
  Monitors events to ensure layout is updated.  
*/

bool QMainWindow::event( QEvent * e )
{
    if ( e->type() == Event_ChildRemoved ) {
	QChildEvent * c = (QChildEvent *) e;
	if ( c->child() == 0 ||
	     c->child()->testWFlags( WType_TopLevel ) ) {
	    // nothing
	} else if ( c->child() == d->sb ) {
	    d->sb = 0;
	    triggerLayout();
	} else if ( c->child() == d->mb ) {
	    d->mb = 0;
	    triggerLayout();
	} else if ( c->child() == d->mc ) {
	    d->mc = 0;
	    triggerLayout();
	} else {
	    removeToolBar( (QToolBar *)(c->child()) );
	    triggerLayout();
	}
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
}


/*! \fn void QMainWindow::pixmapSizeChanged( bool )

  This signal is called whenever the setUsesBigPixmaps() is called
  with a value which is different from the current setting.  All
  relevant widgets must connect to this signal.
*/


/*!  Sets this main window to expand its toolbars to fill all
  available space if \a enable is TRUE, and to give the toolbars just
  the space they need if \a enable is FALSE.

  The default is FALSE.

  \sa rightJustification();
*/

void QMainWindow::setRightJustification( bool enable )
{
    if ( enable == d->justify )
	return;
    d->justify = enable;
    triggerLayout();
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


void QMainWindow::triggerLayout()
{
    if ( isVisibleToTLW() ) {
	setUpLayout();
	d->timer->stop();
    } else {
	d->timer->start( 0, TRUE );
    }
}
