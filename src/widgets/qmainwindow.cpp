/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qmainwindow.cpp#87 $
**
** Implementation of QMainWindow class
**
** Created : 980312
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qmainwindow.h"

#include "qlist.h"
#include "qtimer.h"
#include "qlayout.h"
#include "qobjectlist.h"
#include "qobjectdict.h"
#include "qapplication.h"
#include "qmap.h"

#include "qpainter.h"

#include "qmenubar.h"
#include "qtoolbar.h"
#include "qstatusbar.h"
#include "qscrollview.h"

#include "qtooltip.h"
#include "qwhatsthis.h"

#include "qlayoutengine.h"


// NOT REVISED
/*! \class QMainWindow qmainwindow.h

  \brief The QMainWindow class provides a typical application window,
  with a menu bar, some tool bars and a status bar.

  \ingroup realwidgets
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
  for toolbar. Currently, only \c Top, \c Left, \c Right and \c Bottom
  are meaningful.

  Several functions let you change the appearance of a QMainWindow
  globally: setRightJustification() determines whether QMainWindow
  should ensure that the toolbars fill the available space,
  setUsesBigPixmaps() determines whether QToolButton (and other
  classes) should draw small or large pixmaps (see QIconSet for more
  about that).

  Toolbars can be dragged by the user into each enabled docking area
  and inside each docking area to change the order of the toolbars
  there.

  An application with multiple toolbars should always save the layout
  of the toolbars (docking area and position there) and restore that
  when loading the application again. To get the information about
  the docking area and position for each toolbar, use the
  findDockAndIndexOfToolbar() and toolBarsOnDock() methods. To move
  the toolbar again to a position of a docking area when restoring, use one of the
  moveToolBar() methods.
  When saving and restoring this layout, you have to be careful to save
  and restore it in the same order to make it working correctly. So, it's
  suggested to get first the toolbars of each docking area with
  toolBarsOnDock(). Then get all information of each of these toolbars with
  findDockAndIndexOfToolbar() for each toolbar in each list and save
  it in the order of the list you got from toolBarsOnDock().
  When restoring the layout, create the toolbars exactly in the order in
  which the infos are loaded.

  For multidocument interfaces (MDI), use a QWorkspace as central
  widget.

  <img src=qmainwindow-m.png> <img src=qmainwindow-w.png>

  \sa QToolBar QStatusBar QMenuBar QToolTipGroup QDialog
*/

/*!
  \enum QMainWindow::ToolBarDock

  Each toolbar can be in one of the following positions:
  <ul>
    <li>\c Top - above the central widget, below the menubar.
    <li>\c Bottom - below the central widget, above the status bar.
    <li>\c Left - to the left of the central widget.
    <li>\c Right - to the left of the central widget.
  </ul>

  Other values are also defined for future expansion.
*/

class QMainWindowPrivate {
public:
    struct ToolBar {
	ToolBar() : t(0), nl(FALSE) {}
	ToolBar( QToolBar * tb, bool n=FALSE )
	    : t(tb), nl(n) {}
	QToolBar * t;
	bool nl;
	QValueList<int> disabledDocks;
    };
	
    typedef QList<ToolBar> ToolBarDock;
    enum InsertPos { Before, After, TotalAfter };

    QMainWindowPrivate()
	: top(0), left(0), right(0), bottom(0), tornOff(0), unmanaged(0),
	  mb(0), sb(0), ttg(0), mc(0), timer(0), tll(0), ubp( FALSE ),
	  justify( FALSE )
    {
	// nothing
    }

    ToolBar *findToolbar( QToolBar *t, QMainWindowPrivate::ToolBarDock *&dock );
    ToolBar *takeToolBarFromDock( QToolBar * t );

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

    QPoint pos;
    QPoint offset;

    QPainter *rectPainter;
    QRect oldPosRect;
    QRect origPosRect;
    bool oldPosRectValid, movedEnough;
    QMainWindow::ToolBarDock origDock, oldDock;
    int oldiPos;
    QToolBar *oldCovering;

    QMap< int, bool > dockable;
};



class QToolLayout : public QLayout
{
public:
    QToolLayout( QWidget *parent, int border=0, int space=-1,
		const char *name=0 )
	: QLayout( parent, border, space, name ) { init(); }
    QToolLayout( QLayout* parent, int space=-1, const char *name=0 )
	: QLayout( parent, space, name )  { init(); }
    QToolLayout( int space=-1, const char *name=0 )
	: QLayout( space, name )  { init(); }
    ~QToolLayout();

    void addItem( QLayoutItem *item);
    bool hasHeightForWidth() const { return TRUE; }
    int heightForWidth( int ) const;
    QSize sizeHint() const { return minimumSize(); }
    QSize minimumSize() const;
    QLayoutIterator iterator();
    QSizePolicy::ExpandData expanding() const { return QSizePolicy::NoDirection; }
    void setDirection( QBoxLayout::Direction d ) { dir = d; }
    QBoxLayout::Direction direction() const { return dir; }
    void newLine();
    void setRightJustified( bool on ) { fill = on; }
    bool rightJustified() const { return fill; }
    void invalidate();
protected:
    void setGeometry( const QRect& );
private:
    void init();
    int layoutItems( const QRect&, bool testonly = FALSE );
    QList<QLayoutItem> list;
    QArray<QLayoutStruct> *array;
    QBoxLayout::Direction dir;
    bool fill;
    int cached_width;
    int cached_hfw;
};


void QToolLayout::init()
{
    list.setAutoDelete( TRUE );
    array = 0;
    dir = QBoxLayout::Down;
    fill = FALSE;
    cached_width = 0;
}

QToolLayout::~QToolLayout()
{
    delete array;
}

QSize QToolLayout::minimumSize() const
{
    QSize s;
    QListIterator<QLayoutItem> it(list);
    QLayoutItem *o;
    while ( (o=it.current()) != 0 ) {
	++it;
	if ( o->isEmpty() )
	    continue;
	s = s.expandedTo( o->minimumSize() );
    }
    return s;
}

void QToolLayout::invalidate()
{
    cached_width = 0;
    delete array;
    array=0;
}

int QToolLayout::layoutItems( const QRect &r, bool testonly )
{
    int n = list.count();
    if ( !testonly ) {
	if ( !array ) {
	    array = new QArray<QLayoutStruct>(n);
	    for (int i = 0; i < n; i++ ) {
		QLayoutStruct &a = (*array)[i];
		QLayoutItem *o = list.at(i);
		a.init();
		a.empty = FALSE;
		a.sizeHint = o->sizeHint().width();
		a.expansive = o->expanding() & QSizePolicy::Horizontal;
	    }
	}
	ASSERT( array->size() == list.count() );
    }
    bool up = dir == QBoxLayout::Up;
    int y = r.y();
    int lineh = 0;		//height of this line so far.
    int linew = 0;

    int start = 0;
    int idx = 0;

    QLayoutItem *next = list.first();
    QSize nsh = next->sizeHint();
    while ( idx < n ) {
	QSize sh = nsh;
	idx++;
	if ( idx < n ) {
	    next = list.at(idx);
	    nsh =  next->sizeHint();
	} else {
	    next = 0;
	    nsh = QSize(0,0);
	}
	linew = linew + sh.width() + spacing();
	lineh = QMAX( lineh, sh.height() );
	if ( !next || idx > start && linew + nsh.width() > r.width() ) {
	    //linebreak
	    if ( !testonly ) {
		int width = fill ? r.width() : linew - spacing();
		qGeomCalc( *array, start, idx-start, r.left(), width,
			   spacing() );
		for ( int i = start; i < idx; i++ ) {
		    QRect g( (*array)[i].pos,
			     up ? r.y() + r.bottom() - y - lineh : y,
			     (*array)[i].size, lineh );
		    list.at(i)->setGeometry( g );
		}
	    }
	    y = y + lineh + spacing();
	    linew = 0;
	    lineh = 0;
	    start = idx;
	}
		
    }
    return y - r.y() - spacing();
}


int QToolLayout::heightForWidth( int w ) const
{
    if ( cached_width != w ) {
	//Not all C++ compilers support "mutable" yet:
	QToolLayout * mthis = (QToolLayout*)this;
	int h = mthis->layoutItems( QRect(0,0,w,0), TRUE );
	mthis->cached_hfw = h;
	return h;
    }
    return cached_hfw;
}

void QToolLayout::addItem( QLayoutItem *item )
{
    list.append( item );
}


class QToolLayoutIterator :public QGLayoutIterator
{
public:
    QToolLayoutIterator( QList<QLayoutItem> *l ) :idx(0), list(l)  {}
    uint count() const { return list->count(); }
    QLayoutItem *current() { return idx < (int)count() ? list->at(idx) : 0;  }
    QLayoutItem *next() { idx++; return current(); }
    QLayoutItem *takeCurrent() { return list->take( idx ); }
private:
    int idx;
    QList<QLayoutItem> *list;
};

QLayoutIterator QToolLayout::iterator()
{
    return QLayoutIterator( new QToolLayoutIterator( &list ) );
}

void QToolLayout::setGeometry( const QRect &r )
{
    QLayout::setGeometry( r );
    layoutItems( r );
}

/*!  Constructs an empty main window. */

QMainWindow::QMainWindow( QWidget * parent, const char * name, WFlags f )
    : QWidget( parent, name, f )
{
    d = new QMainWindowPrivate;
    d->timer = new QTimer( this );
    d->rectPainter = 0;
    connect( d->timer, SIGNAL(timeout()),
	     this, SLOT(setUpLayout()) );

    d->dockable[ (int)Left ] = TRUE;
    d->dockable[ (int)Right ] = TRUE;
    d->dockable[ (int)Top ] = TRUE;
    d->dockable[ (int)Bottom ] = TRUE;
    d->dockable[ (int)Unmanaged ] = TRUE;
    d->dockable[ (int)TornOff ] = TRUE;
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
    if ( l && l->count() )
	b = (QMenuBar *)l->first();
    else
	b = new QMenuBar( (QMainWindow *)this, "automatic menu bar" );
    delete l;
    d->mb = b;
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

    connect( toolTipGroup(), SIGNAL(showTip(const QString&)),
	     statusBar(), SLOT(message(const QString&)) );
    connect( toolTipGroup(), SIGNAL(removeTip()),
	     statusBar(), SLOT(clear()) );
    triggerLayout();
}


/*!  Returns the tool tip group for this window.  If there isn't any,
  menuBar() creates an empty tool tip group on the fly.

  \sa menuBar() statusBar()
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
	d->dockable[ (int)dock ] = TRUE;
    } else {
	d->dockable[ (int)dock ] = FALSE;
    }
}


/*!  Returns TRUE if \a dock is enabled, or FALSE if it is not.

  \sa setDockEnabled()
*/

bool QMainWindow::isDockEnabled( ToolBarDock dock ) const
{
    switch ( dock ) {
    case Top:
	return d->dockable[ (int)dock ];
    case Left:
	return d->dockable[ (int)dock ];
    case Right:
	return d->dockable[ (int)dock ];
    case Bottom:
	return  d->dockable[ (int)dock ];
    case TornOff:
	return d->dockable[ (int)dock ];
    case Unmanaged:
	return d->dockable[ (int)dock ];
    }
    return FALSE; // for illegal values of dock
}

/*!
  Sets \a dock to be available for the toolbar \a tb if \a enable is TRUE, and not
  available if \a enable is FALSE.

  The user can drag the toolbar to any enabled dock.
*/


void QMainWindow::setDockEnabled( QToolBar *tb, ToolBarDock dock, bool enable )
{
    QMainWindowPrivate::ToolBarDock *dummy;
    QMainWindowPrivate::ToolBar *t = d->findToolbar( tb, dummy );
    if ( !t )
	return;

    if ( enable ) {
	if ( t->disabledDocks.contains( (int)dock ) )
	    t->disabledDocks.remove( (int)dock );
    } else {
	if ( !t->disabledDocks.contains( (int)dock ) )
	    t->disabledDocks.append( (int)dock );
    }
}

/*!
  Returns TRUE if \a dock is enabled for the toolbar \a tb , or FALSE if it is not.

  \sa setDockEnabled()
*/

bool QMainWindow::isDockEnabled( QToolBar *tb, ToolBarDock dock ) const
{
    QMainWindowPrivate::ToolBarDock *dummy;
    QMainWindowPrivate::ToolBar *t = d->findToolbar( tb, dummy );
    if ( !t )
	return FALSE;

    return !(bool)t->disabledDocks.contains( (int)dock );
}



/*!  Adds \a toolBar to this the end of \a edge and makes it start a
new line of tool bars if \a nl is TRUE.

If \a toolBar is already managed by some main window, it is first
removed from that window.
*/

void QMainWindow::addToolBar( QToolBar * toolBar,
			      ToolBarDock edge, bool newLine )
{
    if ( !toolBar )
	return;

    if ( toolBar->mw ) {
	toolBar->mw->removeToolBar( toolBar );
	toolBar->mw = this;
    }

    setDockEnabled( edge, TRUE );
    setDockEnabled( toolBar, edge, TRUE );

    QMainWindowPrivate::ToolBarDock * dl = 0;
    if ( edge == Top ) {
	dl = d->top;
	toolBar->setOrientation( QToolBar::Horizontal );
	toolBar->installEventFilter( this );
    } else if ( edge == Left ) {
	dl = d->left;
	toolBar->setOrientation( QToolBar::Vertical );
	toolBar->installEventFilter( this );
    } else if ( edge == Bottom ) {
	dl = d->bottom;
	toolBar->setOrientation( QToolBar::Horizontal );
	toolBar->installEventFilter( this );
    } else if ( edge == Right ) {
	dl = d->right;
	toolBar->setOrientation( QToolBar::Vertical );
	toolBar->installEventFilter( this );
    } else if ( edge == TornOff ) {
	dl = d->tornOff;
    } else if ( edge == Unmanaged ) {
	dl = d->unmanaged;
    }

    if ( !dl )
	return;

    dl->append( new QMainWindowPrivate::ToolBar( toolBar, newLine ) );
    triggerLayout();
}


/*!  Adds \a toolBar to this the end of \a edge, labelling it \a label
and makes it start a new line of tool bars if \a newLine is TRUE.

If \a toolBar is already managed by some main window, it is first
removed from that window.
*/

void QMainWindow::addToolBar( QToolBar * toolBar, const QString &label,
			      ToolBarDock edge, bool newLine )
{
    if ( toolBar ) {
	toolBar->setLabel( label );
	addToolBar( toolBar, edge, newLine );
    }
}



QMainWindowPrivate::ToolBar * QMainWindowPrivate::findToolbar( QToolBar * t,
							       QMainWindowPrivate::ToolBarDock *&dock )
{
    QMainWindowPrivate::ToolBarDock* docks[] = {
	left, right, top, bottom, unmanaged, tornOff
    };


    QMainWindowPrivate::ToolBarDock *l = 0;

    for ( unsigned int i = 0; i < 6; ++i ) {
	l = docks[ i ];
	if ( !l )
	    continue;
	QMainWindowPrivate::ToolBar * ct = l->first();
	do {
	    if ( ct && ct->t == t ) {
		dock = l;
		return ct;
	    }
	} while( ( ct = l->next() ) != 0 );
    }

    dock = 0;
    return 0;
}

QMainWindowPrivate::ToolBar * QMainWindowPrivate::takeToolBarFromDock( QToolBar * t )
{
    QMainWindowPrivate::ToolBarDock *l;
    QMainWindowPrivate::ToolBar *tb = findToolbar( t, l );
    if ( tb && l )
	return l->take( l->findRef( tb ) );
    return 0;
}

/*!
  Moves \a toolBar before the tollbar \a relative if \a after is FALSE, or after
  \a relative if \a after is TRUE.

  If \a toolBar is already managed by some main window, it is moved from
  that window to this.
*/

void QMainWindow::moveToolBar( QToolBar *toolBar, ToolBarDock edge, QToolBar *relative, bool after  )
{
    if ( !toolBar )
	return;

    if ( relative == toolBar )
	return;

    bool nl = FALSE;
    QValueList<int> dd;
    if ( toolBar->mw && toolBar->mw != this ) {
	QMainWindowPrivate::ToolBarDock *dummy;
	QMainWindowPrivate::ToolBar *tb = d->findToolbar( toolBar, dummy );
	if ( tb ) {
	    nl = tb->nl;
	    dd = tb->disabledDocks;
	}
	toolBar->mw->removeToolBar( toolBar );
    }

    QMainWindowPrivate::ToolBar * ct;
    ct = d->takeToolBarFromDock( toolBar );

    if ( ct ) {
	setDockEnabled( edge, TRUE );
	setDockEnabled( toolBar, edge, TRUE );

	QMainWindowPrivate::ToolBarDock * dl = 0;
	if ( edge == Top ) {
	    dl = d->top;
	    toolBar->setOrientation( QToolBar::Horizontal );
	    toolBar->installEventFilter( this );
	} else if ( edge == Left ) {
	    dl = d->left;
	    toolBar->setOrientation( QToolBar::Vertical );
	    toolBar->installEventFilter( this );
	} else if ( edge == Bottom ) {
	    dl = d->bottom;
	    toolBar->setOrientation( QToolBar::Horizontal );
	    toolBar->installEventFilter( this );
	} else if ( edge == Right ) {
	    dl = d->right;
	    toolBar->setOrientation( QToolBar::Vertical );
	    toolBar->installEventFilter( this );
	} else if ( edge == TornOff ) {
	    dl = d->tornOff;
	} else if ( edge == Unmanaged ) {
	    dl = d->unmanaged;
	}

	if ( !dl ) {
	    delete ct;
	    return;
	}

	if ( !relative )
	    dl->append( ct );
	else {
	    QMainWindowPrivate::ToolBar *t = dl->first();
	    int i = 0;
	    for ( ; t; t = dl->next(), ++i ) {
		if ( t->t == relative )
		    break;
	    }
	    if ( after )
		++i;
	    if ( i > (int)dl->count() )
		i = dl->count();
	    dl->insert( i, ct );
	    if ( !after && (int)dl->count() > i + 1 && dl->at( i + 1 )->nl ) {
		dl->at( i + 1 )->nl = FALSE;
		dl->at( i )->nl = TRUE;
	    }
	    if ( after && ct->nl )
		ct->nl = FALSE;
	}
    } else {
	addToolBar( toolBar, edge, nl );
	QMainWindowPrivate::ToolBarDock *dummy;
	QMainWindowPrivate::ToolBar *tb = d->findToolbar( toolBar, dummy );
	if ( tb )
	    tb->disabledDocks = dd;
    }

    triggerLayout();
}

/*!
  Moves \a toolBar to this the end of \a edge.

  If \a toolBar is already managed by some main window, it is moved from
  that window to this.
*/

void QMainWindow::moveToolBar( QToolBar * toolBar, ToolBarDock edge )
{
    moveToolBar( toolBar, edge, 0, TRUE );
}

/*!
  Moves \a toolBar to the position \a index of \a edge.

  If \a toolBar is already managed by some main window, it is moved from
  that window to this.
*/

void QMainWindow::moveToolBar( QToolBar * toolBar, ToolBarDock edge, bool nl, int index )
{
    QMainWindowPrivate::ToolBarDock * dl = 0;
    switch ( edge ) {
    case Left:
	dl = d->left;
	break;
    case Right:
	dl = d->right;
	break;
    case Top:
	dl = d->top;
	break;
    case Bottom:
	dl = d->bottom;
	break;
    case Unmanaged:
	dl = d->unmanaged;
	break;
    case TornOff:
	dl = d->tornOff;
	break;
    }
	
    QMainWindowPrivate::ToolBarDock *dummy;
    QMainWindowPrivate::ToolBar *tt = d->findToolbar( toolBar, dummy );
    if ( nl && tt )
	tt->nl = nl;
    if ( !dl ) {
	moveToolBar( toolBar, edge, 0, TRUE );
    } else {
	QMainWindowPrivate::ToolBar *tb = 0;
	if ( index >= (int)dl->count() )
	    tb = 0;
	else
	    tb = dl->at( index );
	if ( !tb )
	    moveToolBar( toolBar, edge, 0, TRUE );
	else
	    moveToolBar( toolBar, edge, tb->t, FALSE );
    }
}

/*!
  Removes \a toolBar from this main window, if \a toolBar is
  non-null and known by this main window.
*/

void QMainWindow::removeToolBar( QToolBar * toolBar )
{
    if ( !toolBar )
	return;
    QMainWindowPrivate::ToolBar * ct;
    ct = d->takeToolBarFromDock( toolBar );
    if ( ct ) {
	toolBar->mw = 0;
	delete ct;
	triggerLayout();
    }
}



class QToolBoxLayout : public QBoxLayout
{
public:
    QToolBoxLayout( Direction d ) :QBoxLayout(d) {}
    QSize minimumSize() const;
};

QSize QToolBoxLayout::minimumSize() const
{
    QSize s(0,0);
    QLayoutIterator it = ((QToolBoxLayout*)this)->iterator();
    QLayoutItem *o;
    while ( (o = it.current()) ) {
	++it;
	s = s.expandedTo( o->minimumSize() );
    }
    s.setHeight(1);
    return s;
}

static QMainWindowPrivate::ToolBar *findCoveringToolbar( QMainWindowPrivate::ToolBarDock *dock,
							const QPoint &pos,
							 QMainWindowPrivate::InsertPos &ipos )
{
    if ( !dock )
	return 0;
    QMainWindowPrivate::ToolBar * t = dock->first(), *tmp = 0;
    if ( !t )
	return 0;

    if ( t->t->orientation() == Qt::Horizontal ) {
	while ( t ) {
	    if ( pos.y() >= t->t->y() && pos.y() <= t->t->y() + t->t->height() ) {
		if ( pos.x() >= t->t->x() && pos.x() <= t->t->x() + t->t->width() ) {
		    if ( pos.x() < t->t->x() + t->t->width() / 2 )
			ipos = QMainWindowPrivate::Before;
		    else {
			tmp = dock->next();
			if ( !tmp || tmp->nl )
			    ipos = QMainWindowPrivate::TotalAfter;
			else
			    ipos = QMainWindowPrivate::After;
		    }
		    return t;
		}
		tmp = t;
		t = dock->next();
		if ( !t || t->nl || t->t->y() > tmp->t->y() ) {
		    ipos = QMainWindowPrivate::TotalAfter;
		    return tmp;
		}
	    } else {
		t = dock->next();
	    }
	}
    } else {
	while ( t ) {
	    if ( pos.x() >= t->t->x() && pos.x() <= t->t->x() + t->t->width() ) {
		if ( pos.y() >= t->t->y() && pos.y() <= t->t->y() + t->t->height() ) {
		    if ( pos.y() < t->t->y() + t->t->height() / 2 )
			ipos = QMainWindowPrivate::Before;
		    else {
			tmp = dock->next();
			if ( !tmp || tmp->nl )
			    ipos = QMainWindowPrivate::TotalAfter;
			else
			    ipos = QMainWindowPrivate::After;
		    }
		    return t;
		}
		tmp = t;
		t = dock->next();
		if ( !t || t->nl || t->t->y() > tmp->t->x() ) {
		    ipos = QMainWindowPrivate::TotalAfter;
		    return tmp;
		}
	    } else {
		t = dock->next();
	    }
	}
    }

    return 0;
}


static void addToolBarToLayout( QMainWindowPrivate::ToolBarDock * dock,
				QBoxLayout * tl,
				QBoxLayout::Direction direction,
				QBoxLayout::Direction dockDirection,
				bool /*mayNeedDockLayout*/,
				bool justify,
				Qt::GUIStyle style )
{
    if ( !dock || dock->isEmpty() )
	return;

    QBoxLayout * dockLayout = tl;

    if ( direction == QBoxLayout::RightToLeft || direction == QBoxLayout::LeftToRight ){
	QToolLayout *layout = 0;
	QMainWindowPrivate::ToolBar * t = dock->first();
	while ( t ) {
	    if ( !layout || t->nl ) {
		layout = new QToolLayout( tl, -1, "tool layout" );
		layout->setDirection( dockDirection );
		layout->setRightJustified( justify );
	    }
	    layout->add(t->t);
	    t = dock->next();
	}
    } else {
	QBoxLayout * toolBarRowLayout = 0;
	QMainWindowPrivate::ToolBar * t = dock->first();
	bool anyToolBars;
	do {
	    bool nl = t->nl;
	    if ( !toolBarRowLayout || nl ) {
		if ( toolBarRowLayout && !justify )
		    toolBarRowLayout->addStretch( 1 );
		toolBarRowLayout = new QToolBoxLayout( direction );
		dockLayout->addLayout( toolBarRowLayout, 0 );
	    }
	    toolBarRowLayout->addWidget( t->t, 0 );
	    anyToolBars = TRUE;//####
	} while ( (t=dock->next()) != 0 );

	if ( anyToolBars && style == Qt::MotifStyle )
	    dockLayout->addSpacing( 2 );

	if ( toolBarRowLayout && (!justify || !anyToolBars) )
	    toolBarRowLayout->addStretch( 1 );
    }
}


/*!  Sets up the geometry management of this window.  Called
  automatically when needed, so you should never need to call this.
*/

void QMainWindow::setUpLayout()
{
    if ( !d->mb ) {
	// slightly evil hack here.  reconsider this after 2.0
	QObjectList * l
	    = ((QObject*)this)->queryList( "QMenuBar", 0, FALSE, FALSE );
	if ( l && l->count() )
	    d->mb = menuBar();
	delete l;
    }
    if ( !d->sb ) {
	// as above.
	QObjectList * l
	    = ((QObject*)this)->queryList( "QStatusBar", 0, FALSE, FALSE );
	if ( l && l->count() )
	    d->sb = statusBar();
	delete l;
    }

    d->timer->stop();
    delete d->tll;
    d->tll = new QBoxLayout( this, QBoxLayout::Down );
    //    d->tll->setResizeMode( QLayout::FreeResize );//############# we need floating layout with height for width here!!!!

    if ( d->mb && !d->mb->testWState(Qt::WState_ForceHide) )
	d->tll->setMenuBar( d->mb );
    if ( style() == WindowsStyle )
	d->tll->addSpacing( 1 );

    addToolBarToLayout( d->top, d->tll,
			QBoxLayout::LeftToRight, QBoxLayout::Down, FALSE,
			d->justify, style() );
    QBoxLayout * mwl = new QBoxLayout( QBoxLayout::LeftToRight );
    d->tll->addLayout( mwl, 100 );
    addToolBarToLayout( d->left, mwl,
			QBoxLayout::Down, QBoxLayout::LeftToRight, FALSE,
			d->justify, style() );
    if ( centralWidget() &&
	 !centralWidget()->testWState(Qt::WState_ForceHide) )
	mwl->addWidget( centralWidget(), 100 );
    else
	mwl->addStretch( 100 );
    addToolBarToLayout( d->right, mwl,
			QBoxLayout::Down, QBoxLayout::LeftToRight, FALSE,
			d->justify, style() );
    addToolBarToLayout( d->bottom, d->tll,
			QBoxLayout::LeftToRight, QBoxLayout::Up, TRUE,
			d->justify, style() );

    if ( d->sb && !d->sb->testWState(Qt::WState_ForceHide) ) {
	d->tll->addWidget( d->sb, 0 );
	// make the sb stay on top of tool bars if there isn't enough space
	d->sb->raise();
    }
    //debug( "act %d, %d", x(), y() );
    d->tll->activate();

}


/*!  \reimp */
void QMainWindow::show()
{
    setUpLayout();
    QWidget::show();
}


/*!  \reimp */
QSize QMainWindow::sizeHint() const
{
    if ( !d->tll ) {
	QMainWindow* that = (QMainWindow*) this;
	that->setUpLayout();
    }
    return d->tll->totalSizeHint();
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
#if 0
    // this code should only be used if there's a menu bar, in Windows
    // Style, and there's a tool bar immediately below it.  or
    // something like that.  I'll have to figure it out again.
    if ( style() == WindowsStyle && d->mb ) {
	QPainter p( this );
	int y = d->mb->height();
	p.setPen( colorGroup().dark() );
	p.drawLine( 0, y, width()-1, y );
    }
#endif
}


/*!
  \reimp
*/

bool QMainWindow::eventFilter( QObject* o, QEvent *e )
{
    if ( ( e->type() == QEvent::MouseButtonPress ||
		  e->type() == QEvent::MouseMove ||
		  e->type() == QEvent::MouseButtonRelease ) &&
		o && o->inherits( "QToolBar" )  ) {
	moveToolBar( (QToolBar *)o, (QMouseEvent *)e );
	return TRUE;
    }
    return QWidget::eventFilter( o, e );
}


/*!
  Monitors events to ensure layout is updated.
*/
void QMainWindow::resizeEvent( QResizeEvent* )
{
    //    setUpLayout(); // #####
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
	    removeToolBar( (QToolBar *)(e->child()) );
	    triggerLayout();
	}
    }
}

/*!
  Monitors events to ensure layout is updated.
*/

bool QMainWindow::event( QEvent * e )
{
    if ( e->type() == QEvent::LayoutHint )
	triggerLayout();
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
    delete d->tll;
    d->tll = 0;
    d->timer->start( 0, TRUE );
}

static QRect findRectInDockingArea( QMainWindowPrivate *d, QMainWindow::ToolBarDock dock, const QPoint &pos,
				    const QRect &areaRect, QToolBar *tb, int &ipos, QToolBar *&covering  )
{
    // find the container where the needed toolbar is
    QMainWindowPrivate::ToolBarDock *tdock = 0;
    switch ( dock ) {
    case QMainWindow::Left:
	tdock = d->left;
	break;
    case QMainWindow::Right:
	tdock = d->right;
	break;
    case QMainWindow::Top:
	tdock = d->top;
	break;
    case QMainWindow::Bottom:
	tdock = d->bottom;
	break;
    case QMainWindow::Unmanaged:
	tdock = d->unmanaged;
	break;
    case QMainWindow::TornOff:
	tdock = d->tornOff;
	break;
    }

    // find the toolbar which will be the relative toolbar for the moved one
    QMainWindowPrivate::ToolBar *t = findCoveringToolbar( tdock, pos,
							  (QMainWindowPrivate::InsertPos&)ipos );
    covering = 0;

    Qt::Orientation o;
    if ( dock == QMainWindow::Top || dock == QMainWindow::Bottom )
	o = Qt::Horizontal;
    else
	o = Qt::Vertical;

    // calc width and height of the tb depending on the orientation it _would_ have
    int w = o == tb->orientation() ? tb->width() : tb->height();
    int h = o != tb->orientation() ? tb->width() : tb->height();

    // if a relative toolbar for the moving one was found
    if ( t ) {
	covering = t->t;
	QRect r;
	bool hasRect = FALSE;
	
	// if the relative toolbar is the same as the moving one, nothing would change
	if ( t->t == tb ) {
	    hasRect = TRUE;
	    r = QRect( tb->pos(), tb->size() );
	}
	
	// calc the rect which the moving toolbar would cover
	if ( !hasRect ) {
	    switch( ipos ) {
	    case QMainWindowPrivate::Before: {
		hasRect = TRUE;
		if ( o == Qt::Horizontal )
		    r = QRect( t->t->x() - w / 2, t->t->y(), w, h );
		else
		    r = QRect( t->t->x(), t->t->y() - h / 2, w, h );
	    } break;
	    case QMainWindowPrivate::After: {
		hasRect = TRUE;
		if ( o == Qt::Horizontal )
		    r = QRect( t->t->x() + t->t->width() - w / 2, t->t->y(), w, h );
		else
		    r = QRect( t->t->x(), t->t->y() + t->t->height() - h / 2, w, h );
	    } break;
	    case QMainWindowPrivate::TotalAfter: {
		hasRect = TRUE;
		if ( o == Qt::Horizontal )
		    r = QRect( t->t->x() + t->t->width(), t->t->y(), w, h );
		else
		    r = QRect( t->t->x(), t->t->y() + t->t->height(), w, h );
	    } break;
	    }
	}
	
	if ( hasRect )
	    return r;
    }

    // no relative toolbar was found means no toolbar yet in the dock,
    // so the moving one would be the first one .. so calc the rect for that
    switch ( dock ) {
    case QMainWindow::Top:
	return QRect( areaRect.x(), areaRect.y(), w, h );
    case QMainWindow::Left:
	return QRect( areaRect.x(), areaRect.y(), w, h );
    case QMainWindow::Right:
	return QRect( areaRect.x() + areaRect.width() - w, areaRect.y(), w, h );
    case QMainWindow::Bottom:
	return QRect( areaRect.x(), areaRect.y() + areaRect.height() - h, w, h );
    case QMainWindow::Unmanaged: case QMainWindow::TornOff:
	return areaRect;
    }

    return areaRect;
}


/*!
  \internal
  Finds the docking area for the toolbar \a tb. \a pos has to be the position of the mouse
  inside the QMainWindow. \a rect is set to the docking area into which the toolbar should
  be moved if the position of the mouse is \a pos. This method returns the identifier of
  the docking area, which is described by \a rect.

  If the mouse is not in any docking area, Unmanaged is returned as docking area.
*/

QMainWindow::ToolBarDock QMainWindow::findDockArea( const QPoint &pos, QRect &rect, QToolBar *tb,
						    int &ipos, QToolBar *&covering )
{
    // calculate some values for docking areas
    int left, right, top, bottom;
    left = right = top = bottom = 30;
    int h1 = d->mb ? d->mb->height() : 0;
    int h2 = d->sb ? d->sb->height() : 0;
    bool hasTop, hasBottom, hasLeft, hasRight;
    hasTop = hasBottom = hasLeft= hasRight = FALSE;
    if ( d->mc ) {
	if ( d->mc->x() > 0 ) {
	    hasLeft = TRUE;
	    left = d->mc->x();
	}
	if ( d->mc->x() + d->mc->width() < width() ) {
	    hasRight = TRUE;
	    right = width() - ( d->mc->x() + d->mc->width() );
	}
	if ( d->mc->y() > h1 ) {
	    hasTop = TRUE;
	    top = d->mc->y() - h1;
	}
	if ( d->mc->y() + d->mc->height() < height() - h2  ) {
	    hasBottom = TRUE;
	    bottom = height() - ( d->mc->y() + d->mc->height() ) - h2;
	}
    }

    // some checks...
    if ( left < 20 ) {
	hasLeft = FALSE;
	left = 30;
    }
    if ( right < 20 ) {
	hasRight = FALSE;
	right = 30;
    }
    if ( top < 20 ) {
	hasTop = FALSE;
	top = 30;
    }
    if ( bottom < 20 ) {
	hasBottom = FALSE;
	bottom = 30;
    }

    // calculate the docking areas
    QRect leftArea( 0, h1, left, height() - h1 - h2 );
    QRect topArea( 0, h1, width(), top );
    QRect rightArea( width() - right, h1, right, height() - h1 - h2 );
    QRect bottomArea( 0, height() - bottom - h2, width(), bottom );
    QRect leftTop = leftArea.intersect( topArea );
    QRect leftBottom = leftArea.intersect( bottomArea );
    QRect rightTop = rightArea.intersect( topArea );
    QRect rightBottom = rightArea.intersect( bottomArea );

    // now polish the docking areas a bit... (subtract interstections)
    if ( hasTop )
	leftArea = QRegion( leftArea ).subtract( leftTop ).boundingRect();
    if ( hasBottom )
	leftArea = QRegion( leftArea ).subtract( leftBottom ).boundingRect();
    if ( hasTop )
	rightArea = QRegion( rightArea ).subtract( rightTop ).boundingRect();
    if ( hasBottom )
	rightArea = QRegion( rightArea ).subtract( rightBottom ).boundingRect();

    // find the docking area into which the toolbar should/could be moved

    // if the mouse is in a rect which belongs to two docking areas (intersection),
    // find the one with higher priority (that's depending on the original position)
    if ( leftTop.contains( pos ) ) {
	if ( tb->orientation() == Vertical ) {
	    rect = findRectInDockingArea( d, Left, pos, leftArea, tb, ipos, covering );
	    return Left;
	}
	rect = findRectInDockingArea( d, Top, pos, topArea, tb, ipos, covering );
	return Top;
    }
    if ( leftBottom.contains( pos ) ) {
	if ( tb->orientation() == Vertical ) {
	    rect = findRectInDockingArea( d, Left, pos, leftArea, tb, ipos, covering );
	    return Left;
	}
	rect = findRectInDockingArea( d, Bottom, pos, bottomArea, tb, ipos, covering );
	return Bottom;
    }
    if ( rightTop.contains( pos ) ) {
	if ( tb->orientation() == Vertical ) {
	    rect = findRectInDockingArea( d, Right, pos, rightArea, tb, ipos, covering );
	    return Right;
	}
	rect = findRectInDockingArea( d, Top, pos, topArea, tb, ipos, covering );
	return Top;
    }
    if ( rightBottom.contains( pos ) ) {
	if ( tb->orientation() == Vertical ) {
	    rect = findRectInDockingArea( d, Right, pos, rightArea, tb, ipos, covering );
	    return Right;
	}
	rect = findRectInDockingArea( d, Bottom, pos, bottomArea, tb, ipos, covering );
	return Bottom;
    }

    // if the mouse is not in an intersection, it's easy....
    if ( leftArea.contains( pos ) ) {
	rect = findRectInDockingArea( d, Left, pos, leftArea, tb, ipos, covering );
	return Left;
    }
    if ( topArea.contains( pos ) ) {
	rect = findRectInDockingArea( d, Top, pos, topArea, tb, ipos, covering );
	return Top;
    }
    if ( rightArea.contains( pos ) ) {
	rect = findRectInDockingArea( d, Right, pos, rightArea, tb, ipos, covering );
	return Right;
    }
    if ( bottomArea.contains( pos ) ) {
	rect = findRectInDockingArea( d, Bottom, pos, bottomArea, tb, ipos, covering );
	return Bottom;
    }

    // mouse pos outside of any docking area
    return Unmanaged;
}

/*!
  Handles mouse event \e e on behalf of tool bar \a t and does all
  the funky docking.
*/

void QMainWindow::moveToolBar( QToolBar* t , QMouseEvent * e )
{
    if ( e->type() == QEvent::MouseButtonPress ) {
	// don't allow repaints of the central widget as this may be a problem for our rects
	if ( d->mc ) {
	    if ( d->mc->inherits( "QScrollView" ) )
		( (QScrollView*)d->mc )->viewport()->setUpdatesEnabled( FALSE );
	    else
		d->mc->setUpdatesEnabled( FALSE );
	}
	
	// create the painter for our rects
	QApplication::setOverrideCursor( Qt::pointingHandCursor );
	uint flags = getWFlags();
	setWFlags( WPaintUnclipped );
	d->rectPainter = new QPainter;
	setWFlags( flags );
	d->rectPainter->begin( this );
	d->rectPainter->setPen( QPen( color0, 2 ) );
	d->rectPainter->setRasterOp( NotROP );

	// init some stuff
	QPoint pos = mapFromGlobal( e->globalPos() );
	QRect r;
	int ipos;
	QToolBar *covering;
	d->origDock = findDockArea( pos, r, t, ipos, covering );
	r = QRect( t->pos(), t->size() );
	d->rectPainter->drawRect( r );
	d->oldPosRect = r;
	d->origPosRect = r;
	d->oldPosRectValid = TRUE;
	d->pos = mapFromGlobal( e->globalPos() );
	d->movedEnough = FALSE;
	d->oldDock = d->origDock;
	d->oldCovering = covering;
	d->oldiPos = ipos;
	return;
    } else if ( e->type() == QEvent::MouseButtonRelease ) {

	// delete the rect painter
	QApplication::restoreOverrideCursor();
	if ( d->rectPainter ) {
	    if ( d->oldPosRectValid )
		d->rectPainter->drawRect( d->oldPosRect );
	    d->rectPainter->end();
	}
	delete d->rectPainter;
	d->rectPainter = 0;
	
	// allow repaints in central widget again
	if ( d->mc ) {
	    if ( d->mc->inherits( "QScrollView" ) )
		( (QScrollView*)d->mc )->viewport()->setUpdatesEnabled( TRUE );
	    else
		d->mc->setUpdatesEnabled( TRUE );
	}

	// finally really move the toolbar
	int ipos = d->oldiPos;
	QToolBar *covering = d->oldCovering;
	ToolBarDock dock = d->oldDock;
	if ( dock != Unmanaged /*&& dock != d->origDock*/ && isDockEnabled( dock ) &&
	     isDockEnabled( t, dock ) )
	    moveToolBar( t, dock, covering, ipos != QMainWindowPrivate::Before );
	return;
    }

    QPoint p( e->globalPos() );
    QPoint pos = mapFromGlobal( p );
    if ( !d->movedEnough &&
	 ( pos - d->pos ).manhattanLength() > 8 )
	d->movedEnough = TRUE;

    // find the dock, rect, etc. for the current mouse pos
    d->pos = pos;
    QRect r;
    int ipos;
    QToolBar *covering;
    ToolBarDock dock = findDockArea( pos, r, t, ipos, covering );

    // if we will not be able to move the toolbar into the dock/rect
    // we got, it will not be moved at all - show this to the user
    if ( dock == Unmanaged || !isDockEnabled( dock ) ||
	 !isDockEnabled( t, dock ) )
	r = d->origPosRect;

    // draw the new rect where the toolbar would be moved
    if ( d->rectPainter ) {
	if ( d->oldPosRectValid && d->oldPosRect != r )
	    d->rectPainter->drawRect( d->oldPosRect );
	if ( d->oldPosRect != r )
	    d->rectPainter->drawRect( r );
    }
    d->oldPosRect = r;
    d->oldPosRectValid = TRUE;
    d->oldDock = dock;
    d->oldCovering = covering;
    d->oldiPos = ipos;
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
    setUpLayout();
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

bool QMainWindow::findDockAndIndexOfToolbar( QToolBar *tb, ToolBarDock &dock, int &index, bool &nl ) const
{
    if ( !tb )
	return FALSE;

    QMainWindowPrivate::ToolBarDock *td;
    QMainWindowPrivate::ToolBar *t = d->findToolbar( tb, td );

    if ( !td || !t )
	return FALSE;

    if ( td == d->left )
	dock = Left;
    else if ( td == d->right )
	dock = Right;
    else if ( td == d->top )
	dock = Top;
    else if ( td == d->bottom )
	dock = Bottom;
    else if ( td == d->unmanaged )
	dock = Unmanaged;
    else if ( td == d->tornOff )
	dock = TornOff;

    index = td->findRef( t );
    nl = t->nl;

    return TRUE;
}

/*!
  Returns a list of all toolbars which are placed in \a dock.
*/

QList<QToolBar> QMainWindow::toolBarsOnDock( ToolBarDock dock ) const
{
    QList<QToolBar> lst;
    QMainWindowPrivate::ToolBarDock *tdock = 0;
    switch ( dock ) {
    case QMainWindow::Left:
	tdock = d->left;
	break;
    case QMainWindow::Right:
	tdock = d->right;
	break;
    case QMainWindow::Top:
	tdock = d->top;
	break;
    case QMainWindow::Bottom:
	tdock = d->bottom;
	break;
    case QMainWindow::Unmanaged:
	tdock = d->unmanaged;
	break;
    case QMainWindow::TornOff:
	tdock = d->tornOff;
	break;
    }

    if ( tdock ) {
	QMainWindowPrivate::ToolBar *t = tdock->first();
	for ( ; t; t = tdock->next() )
	    lst.append( t->t );
    }

    return lst;
}

