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
  for toolbars. Currently, only \c Top, \c Left, \c Right, \c Bottom
  and \c Hidden are meaningful.

  Several functions let you change the appearance of a QMainWindow
  globally: <ul>
  <li> setRightJustification() determines whether QMainWindow
  should ensure that the toolbars fill the available space
  (see also QToolBar::setStretchable()),
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
  By default this feature is enabled. If the \c Hidden dock is enabled the user
  can hide/show a toolbar with a click on the toolbar handle. The handles of
  all hidden toolbars are drawn below the menu bar in one row, and if the user
  moves the mouse cursor onto such a handle, the label of the toolbar
  is displayed in a tool tip (see QToolBar::label()).

  An application with multiple toolbars can choose to save the current
  toolbar layout in order to restore it in the next session. To do so,
  use getLocation() on each toolbar, store the data and restore the
  layout using moveToolBar() on each toolbar again. When restoring,
  ensure to move the toolbars in exactly the same order in which you
  got the information.

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
    <li>\c Hidden - the toolbar is not shown - all handles of hidden toolbars
    are drawn in one row below the menu bar.
  </ul>

  Other values are also defined for future expansion.
*/

class HideDock;

class QMainWindowPrivate {
public:
    struct ToolBar {
	ToolBar() : t(0), nl(FALSE) {}
	ToolBar( QToolBar * tb, bool n=FALSE )
	    : t(tb), nl(n), oldDock( QMainWindow::Top ), oldIndex( 0 )  {}
	QToolBar * t;
	bool stretchable() const
	    { return t->orientation() == Qt::Horizontal && t->stretchable(); }
	bool fullwidth() const
	    { return t->orientation() == Qt::Horizontal && t->fullwidth(); }
	bool nl;
	QValueList<int> disabledDocks;
	QMainWindow::ToolBarDock oldDock;
	int oldIndex;
    };
	
    typedef QList<ToolBar> ToolBarDock;
    enum InsertPos { Before, After, TotalAfter };

    QMainWindowPrivate()
	:  tornOff(0), unmanaged(0), hidden( 0 ),
	  mb(0), sb(0), ttg(0), mc(0), timer(0), tll(0), ubp( FALSE ), utl( FALSE ),
	  justify( FALSE )
    {
	top = new ToolBarDock;
	left = new ToolBarDock;
	right = new ToolBarDock;
	bottom = new ToolBarDock;
	rectPainter = 0;
	dockable[ (int)QMainWindow::Left ] = TRUE;
	dockable[ (int)QMainWindow::Right ] = TRUE;
	dockable[ (int)QMainWindow::Top ] = TRUE;
	dockable[ (int)QMainWindow::Bottom ] = TRUE;
	dockable[ (int)QMainWindow::Unmanaged ] = TRUE;
	dockable[ (int)QMainWindow::Hidden ] = TRUE;
	dockable[ (int)QMainWindow::TornOff ] = TRUE;

	movable = TRUE;
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
	if ( hidden ) {
	    hidden->setAutoDelete( TRUE );
	    delete hidden;
	}
    }

    ToolBarDock * top, * left, * right, * bottom, * tornOff, * unmanaged, *hidden;

    QMenuBar * mb;
    QStatusBar * sb;
    QToolTipGroup * ttg;

    QWidget * mc;

    QTimer * timer;

    QBoxLayout * tll;

    bool ubp;
    bool utl;
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
    HideDock *hideDock;

    bool movable;

    QMap< int, bool > dockable;
};

class HideToolTip : public QToolTip
{
public:
    HideToolTip( QWidget *parent ) : QToolTip( parent ) {}

    void maybeTip( const QPoint &pos );
};


class HideDock : public QWidget
{
public:
    HideDock( QMainWindow *parent, QMainWindowPrivate *p ) : QWidget( parent ) {
	hide();
	setFixedHeight( style().toolBarHandleExtend() );
	d = p;
	pressedHandle = -1;
	pressed = FALSE;
	setMouseTracking( TRUE );
	win = parent;
	tip = new HideToolTip( this );
    }
    ~HideDock() { delete tip; }

protected:
    void paintEvent( QPaintEvent * ) {
	if ( !d->hidden || d->hidden->isEmpty() )
	    return;
	QPainter p( this );
	p.fillRect( rect(), colorGroup().brush( QColorGroup::Background ) );
	QMainWindowPrivate::ToolBar *tb;
	int x = 0;
	int i = 0;
	for ( tb = d->hidden->first(); tb; tb = d->hidden->next(), ++i ) {
	    style().drawToolBarHandle( &p, QRect( x, 0, 30, 10 ), Qt::Vertical,
				       i == pressedHandle, colorGroup(), TRUE );
	    x += 30;
	}
    }

    void mousePressEvent( QMouseEvent *e ) {
	pressed = TRUE;
	if ( !d->hidden || d->hidden->isEmpty() )
	    return;
	mouseMoveEvent( e );
	
	if ( e->button() == RightButton && pressedHandle != -1 ) {
	    QMainWindowPrivate::ToolBar *tb = d->hidden->at( pressedHandle );
	    QPopupMenu menu( this );
	    int left = menu.insertItem( QMainWindow::tr( "&Left" ) );
	    menu.setItemEnabled( left, win->isDockEnabled( QMainWindow::Left )
				 && win->isDockEnabled( tb->t, QMainWindow::Left ) );
	    int right = menu.insertItem( QMainWindow::tr( "&Right" ) );
	    menu.setItemEnabled( left, win->isDockEnabled( QMainWindow::Right )
				 && win->isDockEnabled( tb->t, QMainWindow::Right ) );
	    int top = menu.insertItem( QMainWindow::tr( "&Top" ) );
	    menu.setItemEnabled( left, win->isDockEnabled( QMainWindow::Top )
				 && win->isDockEnabled( tb->t, QMainWindow::Top ) );
	    int bottom = menu.insertItem( QMainWindow::tr( "&Bottom" ) );
	    menu.setItemEnabled( left, win->isDockEnabled( QMainWindow::Bottom )
				 && win->isDockEnabled( tb->t, QMainWindow::Bottom ) );
	    menu.insertSeparator();
	    int hide = menu.insertItem( QMainWindow::tr( "R&estore" ) );
	    QMainWindow::ToolBarDock dock = tb->oldDock;
	    menu.setItemEnabled( left, win->isDockEnabled( dock )
				 && win->isDockEnabled( tb->t, dock ) );
	    int res = menu.exec( e->globalPos() );
	    pressed = FALSE;
	    pressedHandle = -1;
	    repaint( TRUE );
	    if ( res == left )
		win->moveToolBar( tb->t, QMainWindow::Left );
	    else if ( res == right )
		win->moveToolBar( tb->t, QMainWindow::Right );
	    else if ( res == top )
		win->moveToolBar( tb->t, QMainWindow::Top );
	    else if ( res == bottom )
		win->moveToolBar( tb->t, QMainWindow::Bottom );
	    else if ( res == hide )
		win->moveToolBar( tb->t,  dock );
	    else
		return;
	    tb->t->show();
	}
    }

    void mouseMoveEvent( QMouseEvent *e ) {
	if ( !d->hidden || d->hidden->isEmpty() )
	    return;
	if ( !pressed )
	    return;
	QMainWindowPrivate::ToolBar *tb;
	int x = 0;
	int i = 0;
	if ( e->y() >= 0 && e->y() <= height() ) {
	    for ( tb = d->hidden->first(); tb; tb = d->hidden->next(), ++i ) {
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
	if ( !d->hidden || d->hidden->isEmpty() )
	    return;
	if ( e->button() == LeftButton ) {
	    if ( e->y() >= 0 && e->y() <= height() ) {
		QMainWindowPrivate::ToolBar *tb = d->hidden->at( pressedHandle );
		tb->t->show();
		win->moveToolBar( tb->t, tb->oldDock, tb->nl, tb->oldIndex );
	    }
	}
	pressedHandle = -1;
	repaint( TRUE );
    }

private:
    QMainWindowPrivate *d;
    QMainWindow *win;
    int pressedHandle;
    bool pressed;
    HideToolTip *tip;

    friend class HideToolTip;

};

void HideToolTip::maybeTip( const QPoint &pos )
{
    if ( !parentWidget() )
	return;
    HideDock *dock = (HideDock*)parentWidget();
	
    if ( !dock->d->hidden || dock->d->hidden->isEmpty() )
	return;
    QMainWindowPrivate::ToolBar *tb;
    int x = 0;
    int i = 0;
    for ( tb = dock->d->hidden->first(); tb; tb = dock->d->hidden->next(), ++i ) {
	if ( pos.x() >= x && pos.x() <= x + 30 ) {
	    if ( !tb->t->label().isEmpty() )
		tip( QRect( x, 0, 30, dock->height() ), tb->t->label() );
	    return;
	}
	x += 30;
    }
}


/*
  This layout class does not work very well for vertical toolbars yet
 */

class QToolLayout : public QLayout
{
public:
    QToolLayout( QLayout* parent, QMainWindowPrivate::ToolBarDock *d,
		 QBoxLayout::Direction dd, bool justify,
		 int space=-1, const char *name=0 )
	: QLayout( parent, space, name ), dock(d), dir(dd), fill(justify)
	{ init(); }

    ~QToolLayout();

    void addItem( QLayoutItem *item);
    bool hasHeightForWidth() const;
    int heightForWidth( int ) const;
    QSize sizeHint() const;
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
    QMainWindowPrivate::ToolBarDock *dock;
    QArray<QLayoutStruct> *array;
    QBoxLayout::Direction dir;
    bool fill;
    int cached_width;
    int cached_hfw;
};


QSize QToolLayout::sizeHint() const
{
    if ( hasHeightForWidth() )
	return minimumSize();
    // Only vertical scrollbars below this line
    int w = 0;
    int h = 0;
    QListIterator<QMainWindowPrivate::ToolBar> it(*dock);
    QMainWindowPrivate::ToolBar *tb;
    while ( (tb=it.current()) != 0 ) {
	++it;
	QSize ms = tb->t->sizeHint().expandedTo(tb->t->minimumSize());
	h += ms.height();
	w = QMAX( w, ms.width() );
    }
    return QSize(w,h);
}

bool QToolLayout::hasHeightForWidth() const
{
    //direction is the dock's direction, which is perpendicular to the layout
    return dir == QBoxLayout::Up || dir == QBoxLayout::Down;
}

void QToolLayout::init()
{
    array = 0;
    cached_width = 0;
}

QToolLayout::~QToolLayout()
{
    delete array;
}

QSize QToolLayout::minimumSize() const
{
    if ( !dock )
	return QSize(0,0);
    QSize s;

    QListIterator<QMainWindowPrivate::ToolBar> it(*dock);
    QMainWindowPrivate::ToolBar *tb;
    while ( (tb=it.current()) != 0 ) {
	++it;
	s = s.expandedTo( tb->t->minimumSizeHint() )
	    .expandedTo(tb->t->minimumSize());
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
    int n = dock->count();
    if ( n == 0 )
	return 0;
    //#####  We have to do the full calculation also in test mode, ??????

//     if ( !testonly ) {
    if ( !array || array->size() != dock->count() ) {
	delete array;
	array = new QArray<QLayoutStruct>(n);
	for (int i = 0; i < n; i++ ) {
	    QLayoutStruct &a = (*array)[i];
	    QMainWindowPrivate::ToolBar *tb = dock->at(i);
	    a.init();
	    a.empty = FALSE;
	    a.sizeHint = tb->t->sizeHint().width();
	    a.expansive = tb->stretchable() || fill;
		
	}
    }
//     }
    bool up = dir == QBoxLayout::Up;
    int y = r.y();
    int lineh = 0;		//height of this line so far.
    int linew = 0;

    int start = 0;
    int idx = 0;

    QMainWindowPrivate::ToolBar *next = dock->first();
    QSize nsh = next->t->sizeHint();
    bool fillLine = fill;
    while ( next ) {
	QSize sh = nsh;
	fillLine = fillLine || next->stretchable();
	bool fullwidth = next->fullwidth();
	idx++;
	if ( idx < n ) {
	    next = dock->at(idx);
	    nsh =  next->t->sizeHint();
	} else {
	    next = 0;
	    nsh = QSize(0,0);
	}
	linew = linew + sh.width() + spacing();
	lineh = QMAX( lineh, sh.height() );
	if ( !next || next->nl ||fullwidth||next->fullwidth()
	     || idx > start && linew + nsh.width() > r.width() ) {
	    //linebreak
	    int width = fillLine ? r.width() : linew - spacing();
	    qGeomCalc( *array, start, idx-start, r.left(), width,
		       spacing() );
	    if ( !testonly ) {
		for ( int i = start; i < idx; i++ ) {
		    QRect g( (*array)[i].pos,
			     up ? r.y() + r.bottom() - y - lineh : y,
			     (*array)[i].size, lineh );
		    dock->at(i)->t->setGeometry( g );
		}
	    }
	    y = y + lineh + spacing();
	    linew = 0;
	    lineh = 0;
	    start = idx;
	    fillLine = fill;
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

void QToolLayout::addItem( QLayoutItem * /*item*/ )
{
    //evil
    //    list.append( item );
}


class QToolLayoutIterator :public QGLayoutIterator
{
public:
    QToolLayoutIterator( QList<QLayoutItem> *l ) :idx(0), list(l)  {}
    uint count() const { return list ? list->count() : 0; }
    QLayoutItem *current() { return list && idx < (int)count() ? list->at(idx) : 0;  }
    QLayoutItem *next() { idx++; return current(); }
    QLayoutItem *takeCurrent() { return list ? list->take( idx ) : 0; }
private:
    int idx;
    QList<QLayoutItem> *list;
};

QLayoutIterator QToolLayout::iterator()
{
    //This is evil. Pretend you didn't see this.
    return QLayoutIterator( new QToolLayoutIterator( 0/*&list*/ ) );
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
    d->hideDock = new HideDock( this, d );
}


/*! Destructs the object and frees any allocated resources.

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
    //###    triggerLayout();
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
    //###    ((QMainWindow *)this)->triggerLayout();
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
	case Hidden:
	    if ( !d->hidden )
		d->hidden = new QMainWindowPrivate::ToolBarDock();
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
    case Hidden:
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
    } else if ( edge == Hidden ) {
	dl = d->hidden;
    }

    if ( !dl )
	return;

    QMainWindowPrivate::ToolBar *tb = new QMainWindowPrivate::ToolBar( toolBar, newLine );
    dl->append( tb );
    if ( tb && edge != Hidden ) {
	tb->oldDock = edge;
	tb->oldIndex = dl->findRef( tb );
    }
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
	left, right, top, bottom, unmanaged, tornOff, hidden
    };


    QMainWindowPrivate::ToolBarDock *l = 0;

    for ( unsigned int i = 0; i < 7; ++i ) {
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

    bool recalc = FALSE;
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
	} else if ( edge == Hidden ) {
	    dl = d->hidden;
	}

	if ( !dl ) {
	    if ( edge != Hidden ) {
		ct->oldDock = edge;
		ct->oldIndex = dl->findRef( ct );
	    } else
		recalc = TRUE;
	    delete ct;
	    return;
	}

	if ( !relative ) {
	    dl->append( ct );
	} else {
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
	if ( edge != Hidden ) {
	    ct->oldDock = edge;
	    ct->oldIndex = dl->findRef( ct );
	} else
	    recalc = TRUE;
    } else {
	addToolBar( toolBar, edge, nl );
	QMainWindowPrivate::ToolBarDock *dummy;
	QMainWindowPrivate::ToolBar *tb = d->findToolbar( toolBar, dummy );
	if ( tb )
	    tb->disabledDocks = dd;
	if ( tb && edge != Hidden ) {
	    tb->oldDock = edge;
	    tb->oldIndex = 0;
	}
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
    moveToolBar( toolBar, edge, (QToolBar*)0, TRUE );
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
    case Hidden:
	dl = d->hidden;
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
	moveToolBar( toolBar, edge, (QToolBar*)0, TRUE );
    } else {
	QMainWindowPrivate::ToolBar *tb = 0;
	if ( index >= (int)dl->count() )
	    tb = 0;
	else
	    tb = dl->at( index );
	if ( !tb )
	    moveToolBar( toolBar, edge, (QToolBar*)0, TRUE );
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


static QMainWindowPrivate::ToolBar *findCoveringToolbar( QMainWindowPrivate::ToolBarDock *dock,
							const QPoint &pos,
							 QMainWindowPrivate::InsertPos &ipos )
{
    if ( !dock )
	return 0;
    QMainWindowPrivate::ToolBar * t = dock->first(), *tmp = 0, *maybe = 0;
    if ( !t )
	return 0;

    if ( t->t->orientation() == Qt::Horizontal ) {
	while ( t ) {
	    if ( pos.x() >= t->t->x() && pos.x() <= t->t->x() + t->t->width() )
		maybe = t;
	    if ( pos.y() >= t->t->y() && pos.y() <= t->t->y() + t->t->height() ) {
		if ( pos.x() >= t->t->x() && pos.x() <= t->t->x() + t->t->width() ) {
		    if ( pos.x() < t->t->x() + t->t->width() / 2 )
			ipos = QMainWindowPrivate::Before;
		    else {
			tmp = dock->next();
			if ( ( !tmp || tmp->nl || tmp->t->stretchMode() == QToolBar::FullWidth ) &&
			     t->t->stretchMode() != QToolBar::FullWidth )
			    ipos = QMainWindowPrivate::TotalAfter;
			else
			    ipos = QMainWindowPrivate::After;
		    }
		    return t;
		}
		tmp = t;
		t = dock->next();
		if ( !t || t->nl || t->t->y() > tmp->t->y() || t->t->stretchMode() == QToolBar::FullWidth ) {
		    ipos = QMainWindowPrivate::TotalAfter;
		    return tmp;
		}
	    } else {
		t = dock->next();
	    }
	}

	if ( maybe ) {
	    t = maybe;
	    dock->findRef( maybe );
	    tmp = dock->next();
	    if ( ( !tmp || tmp->nl || tmp->t->stretchMode() == QToolBar::FullWidth ) &&
		 t->t->stretchMode() != QToolBar::FullWidth )
		ipos = QMainWindowPrivate::TotalAfter;
	    else
		ipos = QMainWindowPrivate::After;
	    return maybe;
	}
    } else {
	while ( t ) {
	    if ( pos.y() >= t->t->y() && pos.y() <= t->t->y() + t->t->height() )
		maybe = t;
	    if ( QRect( t->t->pos(), t->t->size() ).contains( pos ) ) {
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
		if ( !t || t->nl || t->t->x() > tmp->t->x() || t->t->stretchable() ) {
		    ipos = QMainWindowPrivate::TotalAfter;
		    return tmp;
		}
	    } else {
		t = dock->next();
	    }
	}
	
	if ( maybe ) {
	    t = maybe;
	    dock->findRef( maybe );
	    tmp = dock->next();
	    if ( !tmp || tmp->nl )
		ipos = QMainWindowPrivate::TotalAfter;
	    else
		ipos = QMainWindowPrivate::After;
	    return maybe;
	}
    }

    ipos = QMainWindowPrivate::TotalAfter;
    return dock->last();
}

/*!  Sets up the geometry management of this window.  Called
  automatically when needed, so you should never need to call this.
*/

void QMainWindow::setUpLayout()
{
    //### Must rewrite!
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

    delete d->tll;
    d->tll = new QBoxLayout( this, QBoxLayout::Down );

    if ( d->mb && !d->mb->testWState(Qt::WState_ForceHide) )
	d->tll->setMenuBar( d->mb );
    if ( style() == WindowsStyle )
	d->tll->addSpacing( 1 );

    d->tll->addWidget( d->hideDock );
    d->hideDock->setFixedHeight( style().toolBarHandleExtend() );
    d->tll->addSpacing( 1 );
    if ( d->hidden && !d->hidden->isEmpty() ) {
	d->hideDock->show();
	QMainWindowPrivate::ToolBar *tb;
	for ( tb = d->hidden->first(); tb; tb = d->hidden->next() )
	    tb->t->hide();
	d->hideDock->repaint( TRUE );
    } else {
	d->hideDock->hide();
    }

    (void) new QToolLayout( d->tll, d->top, QBoxLayout::Down, d->justify );

    QBoxLayout * mwl = new QBoxLayout( QBoxLayout::LeftToRight );

    d->tll->addLayout( mwl, 100 );
    (void) new QToolLayout( mwl, d->left, QBoxLayout::LeftToRight, d->justify );

    if ( centralWidget() &&
	 !centralWidget()->testWState(Qt::WState_ForceHide) )
	mwl->addWidget( centralWidget(), 100 );
    else
	mwl->addStretch( 100 );
    (void) new QToolLayout( mwl, d->right, QBoxLayout::LeftToRight, d->justify );

    (void) new QToolLayout( d->tll, d->bottom, QBoxLayout::Down, d->justify );

    if ( d->sb && !d->sb->testWState(Qt::WState_ForceHide) ) {
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
	   e->type() == QEvent::MouseButtonRelease )
	 && o && o->inherits( "QToolBar" )  ) {
	if ( d->movable ) {
	    moveToolBar( (QToolBar *)o, (QMouseEvent *)e );
	    return TRUE;
	}
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

/*!\reimp
*/

bool QMainWindow::event( QEvent * e ) //### remove 3.0
{
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
  \fn void QMainWindow::startMovingToolbar( QToolBar *toolbar )

  This signal is emitted when the \a toolbar starts moving because
  the user started dragging it.
*/

/*!
  \fn void QMainWindow::endMovingToolbar( QToolBar *toolbar )

  This signal is emitted if the \a toolbar has been moved by
  the user and he/she released the mouse button now, so he/she
  stopped the moving.
*/

/*!
  Sets this main window to right-justifies its toolbars if \a enable
  is TRUE. If enable is FALSE, only stretchable toolbars are expanded,
  while non-stretchable toolbars get just the space they need. Given
  that most toolbars are not stretchable, this usually results in a
  ragged right edge.

  The default is FALSE.

  \sa rightJustification(), QToolBar::setStretchable()
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
    if ( !deleteLayout ) {
	if ( d->hidden && !d->hidden->isEmpty() ) {
	    d->hideDock->show();
	    QMainWindowPrivate::ToolBar *tb;
	    for ( tb = d->hidden->first(); tb; tb = d->hidden->next() )
		tb->t->hide();
	    d->hideDock->repaint( TRUE );
	} else {
	    d->hideDock->hide();
	}
    } else {
	delete d->tll;
	d->tll = 0;
	if ( isVisible() )
	    setUpLayout();
    }
    QApplication::postEvent( this, new QEvent( QEvent::LayoutHint) );
}

static QRect findRectInDockingArea( QMainWindowPrivate *d, QMainWindow *mw,
				    QMainWindow::ToolBarDock dock, const QPoint &pos,
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
    case QMainWindow::Hidden:
	tdock = d->hidden;
	break;
    case QMainWindow::TornOff:
	tdock = d->tornOff;
	break;
    }

    Qt::Orientation o;
    if ( dock == QMainWindow::Top || dock == QMainWindow::Bottom )
	o = Qt::Horizontal;
    else
	o = Qt::Vertical;

    // calc the minimum toolbar size of the dock
    int ms = 0xFFFFFF;
    if ( tdock ) {
	QMainWindowPrivate::ToolBar *tt = tdock->first();
	for ( ; tt; tt = tdock->next() )
	    ms = QMIN( ms, o == Qt::Horizontal ? tt->t->width() : tt->t->height() );
    }

    // find the toolbar which will be the relative toolbar for the moved one
    QMainWindowPrivate::ToolBar *t = findCoveringToolbar( tdock, pos,
							  (QMainWindowPrivate::InsertPos&)ipos );
    covering = 0;

    // calc width and height of the tb depending on the orientation it _would_ have
    if ( o == Qt::Horizontal ) {
	if ( ipos == QMainWindowPrivate::TotalAfter &&
	     t->t->x() + t->t->width() + 10 > mw->width() )
	    ipos = QMainWindowPrivate::After;
    } else {
	if ( ipos == QMainWindowPrivate::TotalAfter &&
	     t->t->y() + t->t->height() + 10 > mw->height() )
	    ipos = QMainWindowPrivate::After;
    }
    int w = o == tb->orientation() ? tb->width() : tb->height();
    int h = o != tb->orientation() ? tb->width() : tb->height();
    if ( o == Qt::Horizontal ) {
	if ( w > ms && ipos != QMainWindowPrivate::TotalAfter )
	    w = ms; // #### should be calced somehow
	if ( t && t->t )
	    QMIN( w, h = t->t->height() );
	else if ( o != tb->orientation() )
	    h = QMIN( h, 30 );
    } else {
	if ( h > ms && ipos != QMainWindowPrivate::TotalAfter )
	    h = ms; // #### should be calced somehow
	if ( t && t->t )
	    w = QMIN( w, t->t->width() );
	else if ( o != tb->orientation() )
	    w = QMIN( w, 30 );
    }

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
    case QMainWindow::Unmanaged: case QMainWindow::TornOff: case QMainWindow::Hidden:
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
    if ( d->mb && style() == WindowsStyle )
	h1++;
    h1 += d->hideDock->isVisible() ? d->hideDock->height() + 1 : 0;
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
	if ( tb->orientation() == Vertical && !hasTop ) {
	    rect = findRectInDockingArea( d, this, Left, pos, leftArea, tb, ipos, covering );
	    return Left;
	}
	rect = findRectInDockingArea( d, this, Top, pos, topArea, tb, ipos, covering );
	return Top;
    }
    if ( leftBottom.contains( pos ) ) {
	if ( tb->orientation() == Vertical && !hasBottom ) {
	    rect = findRectInDockingArea( d, this, Left, pos, leftArea, tb, ipos, covering );
	    return Left;
	}
	rect = findRectInDockingArea( d, this, Bottom, pos, bottomArea, tb, ipos, covering );
	return Bottom;
    }
    if ( rightTop.contains( pos ) ) {
	if ( tb->orientation() == Vertical && !hasTop ) {
	    rect = findRectInDockingArea( d, this, Right, pos, rightArea, tb, ipos, covering );
	    return Right;
	}
	rect = findRectInDockingArea( d, this, Top, pos, topArea, tb, ipos, covering );
	return Top;
    }
    if ( rightBottom.contains( pos ) ) {
	if ( tb->orientation() == Vertical && !hasBottom ) {
	    rect = findRectInDockingArea( d, this, Right, pos, rightArea, tb, ipos, covering );
	    return Right;
	}
	rect = findRectInDockingArea( d, this, Bottom, pos, bottomArea, tb, ipos, covering );
	return Bottom;
    }

    // if the mouse is not in an intersection, it's easy....
    if ( leftArea.contains( pos ) ) {
	rect = findRectInDockingArea( d, this, Left, pos, leftArea, tb, ipos, covering );
	return Left;
    }
    if ( topArea.contains( pos ) ) {
	rect = findRectInDockingArea( d, this, Top, pos, topArea, tb, ipos, covering );
	return Top;
    }
    if ( rightArea.contains( pos ) ) {
	rect = findRectInDockingArea( d, this, Right, pos, rightArea, tb, ipos, covering );
	return Right;
    }
    if ( bottomArea.contains( pos ) ) {
	rect = findRectInDockingArea( d, this, Bottom, pos, bottomArea, tb, ipos, covering );
	return Bottom;
    }

    // mouse pos outside of any docking area
    return Unmanaged;
}

static QRect fixRect( const QRect &r )
{
    if ( r.width() > r.height() )
	return QRect( r.x() + 1, r.y(), r.width() - 2, r.height() );
    return QRect( r.x(), r.y() + 1, r.width(), r.height() - 2 );
}

/*!
  Handles mouse event \a e on behalf of tool bar \a t and does all
  the funky docking.
*/

void QMainWindow::moveToolBar( QToolBar* t , QMouseEvent * e )
{
    if ( e->type() == QEvent::MouseButtonPress ) {
	if ( ( e->button() & RightButton ) ) {
	    emit startMovingToolbar( t );
	    QPopupMenu menu( this );
	    int left = menu.insertItem( tr( "&Left" ) );
	    menu.setItemEnabled( left, isDockEnabled( Left ) && isDockEnabled( t, Left ) );
	    int right = menu.insertItem( tr( "&Right" ) );
	    menu.setItemEnabled( left, isDockEnabled( Right ) && isDockEnabled( t, Right ) );
	    int top = menu.insertItem( tr( "&Top" ) );
	    menu.setItemEnabled( left, isDockEnabled( Top ) && isDockEnabled( t, Top ) );
	    int bottom = menu.insertItem( tr( "&Bottom" ) );
	    menu.setItemEnabled( left, isDockEnabled( Bottom ) && isDockEnabled( t, Bottom ) );
	    menu.insertSeparator();
	    int hide = menu.insertItem( tr( "&Hide" ) );
	    menu.setItemEnabled( left, isDockEnabled( Hidden ) && isDockEnabled( t, Hidden ) );
	    int res = menu.exec( e->globalPos() );
	    if ( res == left )
		moveToolBar( t, Left );
	    else if ( res == right )
		moveToolBar( t, Right );
	    else if ( res == top )
		moveToolBar( t, Top );
	    else if ( res == bottom )
		moveToolBar( t, Bottom );
	    else if ( res == hide )
		moveToolBar( t,  Hidden );
	    emit endMovingToolbar( t );
	    return;
	}
	if ( ( e->button() & MidButton ) ) {
	    return;
	}
	emit startMovingToolbar( t );

	// don't allow repaints of the central widget as this may be a problem for our rects
	if ( d->mc ) {
	    if ( d->mc->inherits( "QScrollView" ) )
		( (QScrollView*)d->mc )->viewport()->setUpdatesEnabled( FALSE );
	    else
		d->mc->setUpdatesEnabled( FALSE );
	}
	
	// create the painter for our rects
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
	d->oldPosRect = r;
	d->origPosRect = r;
	d->oldPosRectValid = FALSE;
	d->pos = mapFromGlobal( e->globalPos() );
	d->movedEnough = FALSE;
	d->oldDock = d->origDock;
	d->oldCovering = covering;
	d->oldiPos = ipos;
	
	return;
    } else if ( e->type() == QEvent::MouseButtonRelease ) {
	if ( ( e->button() & RightButton ) ) {
	    return;
	}
	if ( ( e->button() & MidButton ) ) {
	    return;
	}
	// delete the rect painter
	if ( d->rectPainter ) {
	    if ( d->oldPosRectValid )
		d->rectPainter->drawRect( fixRect( d->oldPosRect ) );
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

	// finally really move the toolbar, if the mouse was moved...
	if ( d->movedEnough ) {
// 	    QApplication::restoreOverrideCursor();
	    int ipos = d->oldiPos;
	    QToolBar *covering = d->oldCovering;
	    ToolBarDock dock = d->oldDock;
	    if ( dock != Unmanaged && isDockEnabled( dock ) &&
		 isDockEnabled( t, dock ) )
		moveToolBar( t, dock, covering, ipos != QMainWindowPrivate::Before );
	} else { // ... or hide it if it was only a click
	    if ( isDockEnabled( Hidden ) && isDockEnabled( t, Hidden ) )
		moveToolBar( t, Hidden );
	}
	
	emit endMovingToolbar( t );
	
	return;
    } else if ( e->type() == QMouseEvent::MouseMove ) {
	if ( ( e->state() & LeftButton ) != LeftButton )
	    return;
    }

    // find out if the mouse had been moved yet...
    QPoint p( e->globalPos() );
    QPoint pos = mapFromGlobal( p );
    if ( !d->movedEnough && pos != d->pos ) {
    	d->movedEnough = TRUE;
// 	QApplication::setOverrideCursor( pointingHandCursor );
    }

    // if no mouse movement yet, don't do anything
    if ( !d->movedEnough )
	return;

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
	    d->rectPainter->drawRect( fixRect( d->oldPosRect ) );
	if ( !d->oldPosRectValid || d->oldPosRect != r )
	    d->rectPainter->drawRect( fixRect( r ) );
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

bool QMainWindow::getLocation( QToolBar *tb, ToolBarDock &dock, int &index, bool &nl ) const
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
    else if ( td == d->hidden )
	dock = Hidden;

    index = td->findRef( t );
    nl = t->nl;

    return TRUE;
}

/*!
  Returns a list of all toolbars which are placed in \a dock.
*/

QList<QToolBar> QMainWindow::toolBars( ToolBarDock dock ) const
{
    QList<QToolBar> lst;
    QMainWindowPrivate::ToolBarDock *tdock = 0;
    switch ( dock ) {
    case Left:
	tdock = d->left;
	break;
    case Right:
	tdock = d->right;
	break;
    case Top:
	tdock = d->top;
	break;
    case Bottom:
	tdock = d->bottom;
	break;
    case Unmanaged:
	tdock = d->unmanaged;
	break;
    case Hidden:
	tdock = d->hidden;
	break;
    case TornOff:
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

/*!
  Sets the toolbars to be movable if \a enable is TRUE, or static
  otherwise.

  Movable toolbars can be dragged around between and within the
  different toolbar docks by the user.

  The default is TRUE.

  \sa setDockEnabled(), toolbarsMovable()
*/

void QMainWindow::setToolBarsMovable( bool enable )
{
    d->movable = enable;
}

/*!
  Returns whether or not the toolbars of this main window are movable.

  \sa setToolbarsMovable()
*/

bool QMainWindow::toolBarsMovable() const
{
    return d->movable;
}
