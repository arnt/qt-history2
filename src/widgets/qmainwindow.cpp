/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qmainwindow.cpp#87 $
**
** Implementation of QMainWindow class
**
** Created : 980312
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
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

#include "qtimer.h"
#include "qlayout.h"
#include "qobjectlist.h"
#include "qobjectdict.h"
#include "qapplication.h"
#include "qlist.h"
#include "qmap.h"

#include "qpainter.h"

#include "qmenubar.h"
#include "qtoolbar.h"
#include "qstatusbar.h"
#include "qscrollview.h"
#include "qtooltip.h"
#include "qdatetime.h"

#include "qtooltip.h"
#include "qwhatsthis.h"

//#define QMAINWINDOW_DEBUG

//***********************************************************************************
// -------------------------- static convenience functions -------------------
//***********************************************************************************

bool operator<( const QRect &r1, const QRect &r2 )
{
    return ( r1.x() < r2.x() ||
	     r1.y() < r2.y() );
}

static Qt::Orientation swap_orientation( Qt::Orientation o )
{
    if ( o == Qt::Horizontal )
	return Qt::Vertical;
    else
	return Qt::Horizontal;
}

static int tb_pos( QToolBar *t, Qt::Orientation o, bool swap = FALSE )
{
    if ( swap )
	o = swap_orientation( o );
    if ( o == Qt::Horizontal )
	return t->x();
    else
	return t->y();
}

static int tb_extend( QToolBar *t, Qt::Orientation o, bool swap = FALSE )
{
    if ( swap )
	o = swap_orientation( o );
    if ( o == Qt::Horizontal )
	return t->width();
    else
	return t->height();
}

static int rect_pos( const QRect &r, Qt::Orientation o, bool swap = FALSE )
{
    if ( swap )
	o = swap_orientation( o );
    if ( o == Qt::Horizontal )
	return r.x();
    else
	return r.y();
}

static int rect_extend( const QRect &r, Qt::Orientation o, bool swap = FALSE )
{
    if ( swap )
	o = swap_orientation( o );
    if ( o == Qt::Horizontal )
	return r.width();
    else
	return r.height();
}

static int size_extend( const QSize &s, Qt::Orientation o, bool swap = FALSE )
{
    if ( swap )
	o = swap_orientation( o );
    if ( o == Qt::Horizontal )
	return s.width();
    else
	return s.height();
}

static QSize size_hint( QToolBar *tb )
{
    if ( !tb || !tb->isVisible() )
	return QSize( 0, 0 );
    return tb->sizeHint();
}

//************************************************************************************************
// ------------------------ QMainWindowPrivate  -----------------------
//************************************************************************************************
class QHideDock;
class QToolLayout;
class QMainWindowPrivate {
public:
    struct ToolBar {
	ToolBar() : t(0), nl(FALSE) {}
	ToolBar( QToolBar * tb, bool n=FALSE )
	    : t(tb), nl(n), oldDock( QMainWindow::Top ), oldIndex( 0 ), extraOffset( -1 )  {}
	QToolBar * t;
	bool isStretchable( Qt::Orientation o ) const
	    { return o == Qt::Horizontal ? t->isHorizontalStretchable() : t->isVerticalStretchable(); }
	bool nl;
	QValueList<int> disabledDocks;
	QMainWindow::ToolBarDock oldDock;
	int oldIndex;
	int extraOffset;
    };

    typedef QList<ToolBar> ToolBarDock;
    enum InsertPos { Before, After, Above, Below, SameIndex };

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
	dockable[ (int)QMainWindow::Minimized ] = TRUE;
	dockable[ (int)QMainWindow::TornOff ] = TRUE;
	lLeft = lRight = lTop = lBottom = 0;
	lastTopHeight = -1;
	movable = TRUE;
	inMovement = FALSE;
	dockMenu = TRUE;
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
    QToolLayout *lLeft, *lRight, *lTop, *lBottom;

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
    QMainWindow::ToolBarDock oldDock, origDock;
    QHideDock *hideDock;
    QPoint cursorOffset;
    int lastTopHeight;

    bool movable;
    bool opaque;
    bool inMovement;
    bool dockMenu;

    QMap< int, bool > dockable;
};

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

//************************************************************************************************
// --------------------------------- QToolLayout ---------------------------------
//************************************************************************************************

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
    int widthForHeight( int ) const;
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
    QBoxLayout::Direction dir;
    bool fill;
    int cached_width;
    int cached_hfw;

    friend class QMainWindowLayout;
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
	QSize ms = size_hint( tb->t ).expandedTo( tb->t->minimumSize() );
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
    cached_width = 0;
}

QToolLayout::~QToolLayout()
{
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
}

int QToolLayout::layoutItems( const QRect &r, bool testonly )
{
    int n = dock->count();
    if ( n == 0 )
	return 0;

    Qt::Orientation o = dock->first()->t->orientation();

    QMainWindowPrivate::ToolBar *t = dock->first(), *t2 = 0;

    QList<QMainWindowPrivate::ToolBar> row;
    row.setAutoDelete( FALSE );
    int stretchs = 0;
    int e = 0;
    int pos = rect_pos( r, o, TRUE );
    int lineExtend = 0;
    while ( TRUE ) {
	QSize sh = t ? size_hint( t->t ) : QSize();
	int nx = e;
	if ( t && t->extraOffset != -1 && t->extraOffset > e )
	    nx = t->extraOffset;
	if ( nx + size_extend( sh, o ) > rect_extend( r, o ) )
	    nx = QMAX( e, rect_extend( r, o ) - size_extend( sh, o ) );
	if ( !t || t->nl || nx + size_extend( sh, o ) > rect_extend( r, o ) ) {
	    QValueList<QRect> rects;
	    int s = stretchs > 0 ? ( rect_extend( r, o ) - e + rect_pos( r, o ) ) / stretchs : 0;
	    int p = rect_pos( r, o );
	    QMainWindowPrivate::ToolBar *tmp = 0;
	    for ( t2 = row.first(); t2; t2= row.next() ) {
		QRect g;
		int oldPos = p;
		int space = 0;
		if ( t2->extraOffset > p ) {
		    space = t2->extraOffset - p;
		    p = t2->extraOffset;
		}
		if ( o == Qt::Horizontal ) {
		    int ext = size_hint( t2->t ).width();
		    if ( p + ext > r.width() && p == t2->extraOffset )
			p = QMAX( p - space, r.width() - ext );
		    if ( p > oldPos && tmp && ( tmp->t->isHorizontalStretchable() || fill ) ) {
			QRect r = rects.last();
			int d = p - ( r.x() + r.width() );
			rects.last().setWidth( r.width() + d );
		    }
		    if ( t2->t->isHorizontalStretchable() || fill )
			ext += QMAX( 0, ( t2->t->isHorizontalStretchable() || fill ? s : 0 ) );
		    if ( p + ext > r.width() && ( t2->t->isHorizontalStretchable() || fill ) )
			ext = r.width() - p;
		    g = QRect( p, pos, ext , lineExtend );
		} else {
		    int ext = size_hint( t2->t ).height();
		    if ( p + ext > r.y() + r.height() && p == t2->extraOffset )
			p = QMAX( p - space, ( r.y() + r.height() ) - ext );
		    if ( p > oldPos && tmp && ( tmp->t->isVerticalStretchable() || fill ) ) {
			QRect r = rects.last();
			int d = p - ( r.y() + r.height() );
			rects.last().setHeight( r.height() + d );
		    }
		    if ( t2->t->isVerticalStretchable() || fill )
			ext += QMAX( 0, ( t2->t->isVerticalStretchable() || fill ? s : 0 ) );
		    if ( p + ext > r.y() + r.height() && ( t2->t->isVerticalStretchable() || fill ) )
			ext = ( r.y() + r.height() ) - p;
		    g = QRect( pos, p, lineExtend, ext );
		}
		rects.append( g );
		p = rect_pos( g, o ) + rect_extend( g, o );
		tmp = t2;
	    }
	    if ( !testonly ) {
		QValueList<QRect>::Iterator it = rects.begin();
		for ( t2 = row.first(); t2; t2= row.next(), ++it ) {
		    t2->t->setGeometry( *it );
		}
	    }
	    stretchs = 0;
	    e = 0;
	    pos += lineExtend;
	    lineExtend = 0;
	    row.clear();
	    nx = 0;
	    if ( t && t->extraOffset != -1 && t->extraOffset > e )
		nx = t->extraOffset;
	}

	if ( !t )
	    break;

	e = nx + size_extend( sh, o );
	lineExtend = QMAX( lineExtend, size_extend( sh, o, TRUE ) );
	row.append( t );
	if ( ( o == Qt::Horizontal && t->t->isHorizontalStretchable() ||
	       o == Qt::Vertical && t->t->isVerticalStretchable() ) || fill )
	    ++stretchs;
	t = dock->next();
    }

    return pos - rect_pos( r, o, TRUE ) - spacing();
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

int QToolLayout::widthForHeight( int h ) const
{
//     if ( cached_width != w ) {
	//Not all C++ compilers support "mutable" yet:
	QToolLayout * mthis = (QToolLayout*)this;
	int w = mthis->layoutItems( QRect( 0, 0, 0, h ), TRUE );
	return w;
//     }
//     return cached_hfw;
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

//************************************************************************************************
// --------------------------------- QMainWindowLayout -----------------------
//************************************************************************************************

class QMainWindowLayout : public QLayout
{
public:
    QMainWindowLayout( QLayout* parent = 0 );
    ~QMainWindowLayout() {}

    void addItem( QLayoutItem *item);
    void setLeftDock( QToolLayout *l );
    void setRightDock( QToolLayout *r );
    void setCentralWidget( QWidget *w );
    bool hasHeightForWidth() const { return FALSE; }
    QSize sizeHint() const;
    QSize minimumSize() const;
    QLayoutIterator iterator();
    QSizePolicy::ExpandData expanding() const { return QSizePolicy::NoDirection; }
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
    QToolLayout *left, *right;
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
	w = QMAX( w, left->minimumSize().width() );
	h = QMAX( h, left->minimumSize().height() );
    }
    if ( right ) {
	w = QMAX( w, right->minimumSize().width() );
	h = QMAX( h, right->minimumSize().height() );
    }
    if ( central ) {
	w = QMAX( w, central->minimumSize().width() );
	h = QMAX( h, central->minimumSize().height() );
    }

    return QSize( w, h );
}

QMainWindowLayout::QMainWindowLayout( QLayout* parent )
    : QLayout( parent ), left( 0 ), right( 0 ), central( 0 )
{
    cached_height = -1; cached_wfh = -1;
}

void QMainWindowLayout::setLeftDock( QToolLayout *l )
{
    left = l;
}

void QMainWindowLayout::setRightDock( QToolLayout *r )
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
	wl = left->widthForHeight( r.height() );
    if ( right )
	wr = right->widthForHeight( r.height() );
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


class QMainWindowLayoutIterator : public QGLayoutIterator
{
public:
    QMainWindowLayoutIterator( QList<QLayoutItem> *l ) :idx(0), list(l)  {}
    uint count() const { return list ? list->count() : 0; }
    QLayoutItem *current() { return list && idx < (int)count() ? list->at(idx) : 0;  }
    QLayoutItem *next() { idx++; return current(); }
    QLayoutItem *takeCurrent() { return list ? list->take( idx ) : 0; }
private:
    int idx;
    QList<QLayoutItem> *list;
};

QLayoutIterator QMainWindowLayout::iterator()
{
    //This is evil. Pretend you didn't see this.
    return QLayoutIterator( new QMainWindowLayoutIterator( 0/*&list*/ ) );
}

//************************************************************************************************
// --------------------------------- Minimized Dock -----------------------
//************************************************************************************************


class QHideToolTip : public QToolTip
{
public:
    QHideToolTip( QWidget *parent ) : QToolTip( parent ) {}

    void maybeTip( const QPoint &pos );
};


class QHideDock : public QWidget
{
public:
    QHideDock( QMainWindow *parent, QMainWindowPrivate *p ) : QWidget( parent ) {
	hide();
	setFixedHeight( style().toolBarHandleExtend() );
	d = p;
	pressedHandle = -1;
	pressed = FALSE;
	setMouseTracking( TRUE );
	win = parent;
	tip = new QHideToolTip( this );
    }
    ~QHideDock() { delete tip; }

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
	    if ( !tb->t->isVisible() )
		continue;
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

	if ( e->button() == RightButton && win->isDockMenuEnabled() ) {
	    if ( pressedHandle != -1 ) {
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
		    win->moveToolBar( tb->t, tb->oldDock, tb->nl, tb->oldIndex, tb->extraOffset );
		else
		    return;
		tb->t->show();
	    } else {
		win->rightMouseButtonMenu( e->globalPos() );
	    }
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
		if ( !tb->t->isVisible() )
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
       	if ( !d->hidden || d->hidden->isEmpty() )
	    return;
	if ( e->button() == LeftButton ) {
	    if ( e->y() >= 0 && e->y() <= height() ) {
		QMainWindowPrivate::ToolBar *tb = d->hidden->at( pressedHandle );
		tb->t->show();
		win->moveToolBar( tb->t, tb->oldDock, tb->nl, tb->oldIndex, tb->extraOffset );
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
    QHideToolTip *tip;

    friend class QHideToolTip;

};

void QHideToolTip::maybeTip( const QPoint &pos )
{
    if ( !parentWidget() )
	return;
    QHideDock *dock = (QHideDock*)parentWidget();

    if ( !dock->d->hidden || dock->d->hidden->isEmpty() )
	return;
    QMainWindowPrivate::ToolBar *tb;
    int x = 0;
    int i = 0;
    for ( tb = dock->d->hidden->first(); tb; tb = dock->d->hidden->next(), ++i ) {
	if ( !tb->t->isVisible() )
	    continue;
	if ( pos.x() >= x && pos.x() <= x + 30 ) {
	    if ( !tb->t->label().isEmpty() )
		tip( QRect( x, 0, 30, dock->height() ), tb->t->label() );
	    return;
	}
	x += 30;
    }
}


//************************************************************************************************
// ------------------------ static internal functions  -----------------------
//************************************************************************************************

static QRect findRectInDockingArea( QMainWindowPrivate *d, QMainWindow::ToolBarDock dock,
				    const QPoint &pos, QToolBar *tb )
{
    Qt::Orientation o = dock == QMainWindow::Top || dock == QMainWindow::Bottom ?
			Qt::Horizontal : Qt::Vertical;
    bool swap = o != tb->orientation();
    QPoint offset( 0, 0 );
    if ( swap ) {
	if ( o == Qt::Horizontal )
	    offset = QPoint( tb->height() / 2, 0 );
	else
	    offset = QPoint( 0, tb->width() / 2 );
    }
    return QRect( pos - d->cursorOffset - offset,
		  swap ? QSize( tb->height(), tb->width() ) : tb->size() );
}

static void saveToolLayout( QMainWindowPrivate *d, QMainWindow::ToolBarDock dock, QToolBar *tb )
{
    QMainWindowPrivate::ToolBarDock * dl = 0;
    switch ( dock ) {
    case QMainWindow::Left:
	dl = d->left;
	break;
    case QMainWindow::Right:
	dl = d->right;
	break;
    case QMainWindow::Top:
	dl = d->top;
	break;
    case QMainWindow::Bottom:
	dl = d->bottom;
	break;
    case QMainWindow::Unmanaged:
	dl = d->unmanaged;
	break;
    case QMainWindow::Minimized:
	dl = d->hidden;
	break;
    case QMainWindow::TornOff:
	dl = d->tornOff;
	break;
    }

    QMainWindowPrivate::ToolBarDock *dummy;
    QMainWindowPrivate::ToolBar *t = 0;
    t = d->findToolbar( tb, dummy );
    int i = dl->findRef( t );
    if ( i == -1 )
	return;
    if ( i + 1 < (int)dl->count() )
	dl->at( i + 1 )->nl = TRUE;
}

static void findNewToolbarPlace( QMainWindowPrivate *d, QToolBar *tb, QMainWindow::ToolBarDock dock,
				 const QRect &dockArea, QToolBar *&relative, int &ipos )
{
    relative = 0;
    Qt::Orientation o = dock == QMainWindow::Top || dock == QMainWindow::Bottom ?
			Qt::Horizontal : Qt::Vertical;
    QMainWindowPrivate::ToolBarDock * dl = 0;
    int dy = 0;
    switch ( dock ) {
    case QMainWindow::Left:
	dl = d->left;
	if ( d->lLeft )
	    dy = d->lLeft->geometry().y() + ( d->top && !d->top->isEmpty() ? 8 : 0 );
	if ( d->lLeft && d->lastTopHeight != -1 && !dy )
	    dy = d->lastTopHeight;
	else
	    d->lastTopHeight = dy;
	break;
    case QMainWindow::Right:
	dl = d->right;
	if ( d->lRight )
	    dy = d->lRight->geometry().y() + ( d->top && !d->top->isEmpty() ? 8 : 0 );
	if ( d->lRight && d->lastTopHeight != -1 && !dy )
	    dy = d->lastTopHeight;
	else
	    d->lastTopHeight = dy;
	break;
    case QMainWindow::Top:
	dl = d->top;
	break;
    case QMainWindow::Bottom:
	dl = d->bottom;
	break;
    case QMainWindow::Unmanaged:
	dl = d->unmanaged;
	break;
    case QMainWindow::Minimized:
	dl = d->hidden;
	break;
    case QMainWindow::TornOff:
	dl = d->tornOff;
	break;
    }

    QList<QMainWindowPrivate::ToolBar> bars;
    QMap<QRect, QList<QMainWindowPrivate::ToolBar> > rows;
    QMainWindowPrivate::ToolBar *first = 0, *last = 0;

    QMainWindowPrivate::ToolBar *t = dl->first();
    int oldPos = dl->first() ? tb_pos( dl->first()->t, o, TRUE ) : 0;
    bool makeNextNl = FALSE;
    bool hadNl = FALSE;
    while ( t ) {
	if ( !t->t->isVisible() ) {
	    t = dl->next();
	    continue;
	}
	if ( t->t == tb )
	    hadNl = t->nl;
	else
	    last = t;
	if ( !first && t->t != tb )
	    first = t;
	int pos = tb_pos( t->t, o, TRUE );

	if ( pos != oldPos ) {
	    QRect r;
	    if ( o == Qt::Horizontal ) {
 		r = QRect( dockArea.x(), QMAX( oldPos, dockArea.y() ),
 			   dockArea.width(),
			   bars.last() ? bars.last()->t->height() : pos - oldPos - dockArea.y() );
	    } else {
		r = QRect( QMAX( oldPos, dockArea.x() ), dockArea.y(),
			   bars.last() ? bars.last()->t->width() : pos - oldPos - dockArea.x(),
			   dockArea.height() );
	    }
	    rows[ r ] = bars;
	    bars.clear();
	    bars.append( t );
	    if ( t->t == tb ) {
		makeNextNl = TRUE;
		t->nl = FALSE;
	    } else {
		makeNextNl = FALSE;
		t->nl = TRUE;
	    }
	    oldPos = pos;
	} else {
	    if ( !makeNextNl ) {
		t->nl = FALSE;
	    } else {
		t->nl = TRUE;
	    }
	    makeNextNl = FALSE;
	    bars.append( t );
	}
	t = dl->next();
    }
    if ( !bars.isEmpty() ) {
	    QRect r;
	    if ( o == Qt::Horizontal ) {
		r = QRect( dockArea.x(), bars.last()->t->y(), dockArea.width(), bars.last()->t->height() );
	    } else {
		r = QRect( bars.last()->t->x(), dockArea.y(), bars.last()->t->width(), dockArea.height() );
	    }
	    rows[ r ] = bars;
	    bars.clear();
    }

    QMainWindowPrivate::ToolBarDock *dummy;
    t = d->findToolbar( tb, dummy );
    QMainWindowPrivate::ToolBar *me = t;
    t->extraOffset = rect_pos( d->oldPosRect, o ) + rect_pos( dockArea, o ) - dy;
    if ( rows.isEmpty() ) {

	relative = 0;
    } else {
	QMap<QRect, QList<QMainWindowPrivate::ToolBar> >::Iterator it = rows.begin();
#ifdef QMAINWINDOW_DEBUG
	qDebug( "compare (%d, %d, %d, %d) ...",
		d->oldPosRect.x(), d->oldPosRect.y(),
		d->oldPosRect.width(), d->oldPosRect.height() );
#endif
	for ( ; it != rows.end(); ++it ) {
#ifdef QMAINWINDOW_DEBUG
	    qDebug( "  ... width (%d, %d, %d, %d)",
		    it.key().x(), it.key().y(),
		    it.key().width(), it.key().height() );
#endif
	    if ( it.key().intersects( d->oldPosRect ) ) {
		QRect ir = it.key().intersect( d->oldPosRect );
		if ( rect_extend( ir, o, TRUE ) < 3 )
		    continue;
		int div = 4;
		int mul = 3;
		if ( d->opaque ) {
		    mul = 2;
		    div = 5;
		}
		bool contains = it.key().contains( d->oldPosRect );
		if ( !contains && rect_extend( d->oldPosRect, o, TRUE ) > rect_extend( it.key(), o, TRUE ) &&
		     rect_pos( d->oldPosRect, o, TRUE ) < rect_pos( it.key(), o, TRUE ) &&
		     rect_pos( d->oldPosRect, o, TRUE ) + rect_extend( d->oldPosRect, o, TRUE ) >
		     rect_pos( it.key(), o, TRUE ) + rect_extend( it.key(), o, TRUE ) )
		    contains = TRUE;
		if ( !contains && rect_extend( ir, o, TRUE ) < ( mul * rect_extend( it.key(), o, TRUE ) ) / div ) {
		    if ( rect_pos( ir, o, TRUE ) <= rect_pos( it.key(), o, TRUE ) ) {
#ifdef QMAINWINDOW_DEBUG
			qDebug( "above" );
#endif
			relative = ( *it ).first()->t;
			ipos = QMainWindowPrivate::Above;
			QMainWindowPrivate::ToolBar *l = ( *it ).first();
			if ( relative == tb && ( *it ).next() ) {
			    relative = ( *it ).current()->t;
			} else if ( relative == tb ) {
			    ipos = QMainWindowPrivate::SameIndex;
			    l->nl = TRUE;
			}
		    } else {
#ifdef QMAINWINDOW_DEBUG
			qDebug( "below" );
#endif
			relative = ( *it ).first()->t;
			ipos = QMainWindowPrivate::Below;
			QMainWindowPrivate::ToolBar *l = ( *it ).first();
			if ( relative == tb && ( *it ).next() ) {
			    relative = ( *it ).current()->t;
			} else if ( relative == tb ) {
			    ipos = QMainWindowPrivate::SameIndex;
			    l->nl = TRUE;
			}
		    }
		} else {
#ifdef QMAINWINDOW_DEBUG
		    qDebug( "insinde" );
#endif
		    bars = *it;
		    t = bars.first();
		    QMainWindowPrivate::ToolBar *b = 0, *last = 0;
		    bool hasMyself = FALSE;
		    while ( t ) {
			if ( t->t == tb ) {
			    hasMyself = TRUE;
			    t = bars.next();
			    continue;
			}
			if ( tb_pos( t->t, o ) + tb_extend( t->t, o ) / 2 < rect_pos( d->oldPosRect, o ) ) {
			    b = t;
			    t = bars.next();
			    continue;
			}
			last = t;
			break;
		    }

		    if ( !b && hadNl && me )
			me->nl = TRUE;

		    if ( !b && last ) {
			relative = last->t;
			ipos = QMainWindowPrivate::Before;
#ifdef QMAINWINDOW_DEBUG
			qDebug( "...before" );
#endif
		    } else if ( b ) {
			relative = b->t;
			ipos = QMainWindowPrivate::After;
#ifdef QMAINWINDOW_DEBUG
			qDebug( "...after" );
#endif
		    }
		    if ( !relative && hasMyself ) {
#ifdef QMAINWINDOW_DEBUG
			qDebug( "...no relative and have myself => self index" );
#endif
			relative = tb;
			ipos = QMainWindowPrivate::SameIndex;
		    } else if ( !relative ) {
#ifdef QMAINWINDOW_DEBUG
			qDebug( "AUTSCH!!!!!!!!!!" );
#endif
		    }
		}
		return;
	    }
	}
    }
    if ( rect_pos( d->oldPosRect, o, TRUE ) < rect_pos( dockArea, o, TRUE ) && first ) {
	relative = first->t;
	ipos = QMainWindowPrivate::Above;
    } else if ( rect_pos( d->oldPosRect, o, TRUE ) + rect_extend( d->oldPosRect, o, TRUE ) >
		rect_pos( dockArea, o, TRUE ) + rect_extend( d->oldPosRect, o, TRUE ) && last ) {
	relative = last->t;
	ipos = QMainWindowPrivate::Below;
    } else {
	relative = 0;
	ipos = QMainWindowPrivate::Before;
    }

}






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
  By default this feature is enabled. If the \c Minimized dock is enabled the user
  can hide(minimize)/show a toolbar with a click on the toolbar handle. The handles of
  all minimized toolbars are drawn below the menu bar in one row, and if the user
  moves the mouse cursor onto such a handle, the label of the toolbar
  is displayed in a tool tip (see QToolBar::label()).

  Normally toolbars are moved transparently (this means while the user
  drags one, a rectangle is drawn on the screen). With setOpaqueMoving()
  it's possible to switch betrween opaque and transparent moving
  of toolbars.

  The main window's menubar is static  by default. If you want a movable
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
    d->hideDock = new QHideDock( this, d );
    d->opaque = FALSE;
    installEventFilter( this );
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
	case Minimized:
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
    case Minimized:
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
    } else if ( edge == Minimized ) {
	dl = d->hidden;
    }

    if ( !dl )
	return;

    QMainWindowPrivate::ToolBar *tb = new QMainWindowPrivate::ToolBar( toolBar, newLine );
    dl->append( tb );
    if ( tb && edge != Minimized ) {
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

/*!
  Moves \a toolBar before the tollbar \a relative if \a after is FALSE, or after
  \a relative if \a after is TRUE.

  If \a toolBar is already managed by some main window, it is moved from
  that window to this.
*/

void QMainWindow::moveToolBar( QToolBar *toolBar, ToolBarDock edge, QToolBar *relative, int ipos )
{
    if ( !toolBar )
	return;

    if ( relative == toolBar || ipos == QMainWindowPrivate::SameIndex ) {
#ifdef QMAINWINDOW_DEBUG
	QMainWindowPrivate::ToolBarDock *dummy;
	QMainWindowPrivate::ToolBar *t = d->findToolbar( toolBar, dummy );
	qDebug( "move to same index, offset: %d", t ? t->extraOffset : -2 );
#endif
	triggerLayout();
	emit toolBarPositionChanged( toolBar );
	return;
    }

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
    Qt::Orientation o;
    if ( edge == QMainWindow::Top || edge == QMainWindow::Bottom )
	o = Qt::Horizontal;
    else
	o = Qt::Vertical;

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
	} else if ( edge == Minimized ) {
	    dl = d->hidden;
	}

	if ( !dl ) {
	    if ( edge != Minimized ) {
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
	    if ( ipos != QMainWindowPrivate::Above && ipos != QMainWindowPrivate::Below ) {
		for ( ; t; t = dl->next(), ++i ) {
		    if ( t->t == relative )
			break;
		}
		if ( ipos == QMainWindowPrivate::After )
		    ++i;
	    } else if ( ipos == QMainWindowPrivate::Below ) {
		int ry;
		bool doIt = FALSE;
		for ( ; t; t = dl->next(), ++i ) {
		    if ( t->t == relative ) {
			ry = tb_pos( t->t, o, TRUE );
			doIt = TRUE;
		    }
		    if ( doIt && tb_pos( t->t, o, TRUE ) > ry ) {
			break;
		    }
		}
	    } else if ( ipos == QMainWindowPrivate::Above ) {
		int ry;
		bool doIt = FALSE;
		t = dl->last();
		i = dl->count();
		for ( ; t; t = dl->prev(), --i ) {
		    if ( t->t == relative ) {
			ry = tb_pos( t->t, o, TRUE );
			doIt = TRUE;
		    }
		    if ( doIt && tb_pos( t->t, o, TRUE ) < ry ) {
			break;
		    }
		}
	    }

	    if ( i > (int)dl->count() )
		i = dl->count();
	    dl->insert( i, ct );
	    bool after = ipos == QMainWindowPrivate::After;
	    if ( ipos == QMainWindowPrivate::Before &&
		 (int)dl->count() > i + 1 && dl->at( i + 1 )->nl ) {
		dl->at( i + 1 )->nl = FALSE;
		dl->at( i )->nl = TRUE;
	    }
	    if ( after && ct->nl ) {
		ct->nl = FALSE;
	    }
	    if ( ipos == QMainWindowPrivate::Below ) {
		dl->at( i )->nl = TRUE;
		if ( (int)dl->count() > i + 1 ) {
		    dl->at( i + 1 )->nl = TRUE;
		}
	    }
	    if ( ipos == QMainWindowPrivate::Above ) {
		dl->at( i )->nl = TRUE;
		if ( (int)dl->count() > i + 1 ) {
		    dl->at( i + 1 )->nl = TRUE;
		}
	    }
	}
	if ( edge != Minimized ) {
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
	if ( tb && edge != Minimized ) {
	    tb->oldDock = edge;
	    tb->oldIndex = 0;
	}
    }

    triggerLayout();
    // update, so that the line below the menubar may be drawn/earsed
    update();
    emit toolBarPositionChanged( toolBar );
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

void QMainWindow::moveToolBar( QToolBar * toolBar, ToolBarDock edge, bool nl, int index, int extraOffset )
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
    case Minimized:
	dl = d->hidden;
	break;
    case TornOff:
	dl = d->tornOff;
	break;
    }

    QMainWindowPrivate::ToolBarDock *dummy;
    QMainWindowPrivate::ToolBar *tt = d->findToolbar( toolBar, dummy );
    tt->extraOffset = extraOffset;
    if ( nl && tt )
	tt->nl = nl;
    if ( !dl ) {
	moveToolBar( toolBar, edge, (QToolBar*)0, QMainWindowPrivate::After );
    } else {
	QMainWindowPrivate::ToolBar *tb = 0;
	bool after = FALSE;
	if ( index >= (int)dl->count() ) {
	    tb = 0;
	} else {
	    if ( index > 0 && !nl ) {
		after = TRUE;
		tb = dl->at( index - 1 );
	    } else {
		tb = dl->at( index );
	    }
	}

	if ( !tb )
	    moveToolBar( toolBar, edge, (QToolBar*)0, QMainWindowPrivate::After );
	else
	    moveToolBar( toolBar, edge, tb->t,
			 after ? QMainWindowPrivate::After : QMainWindowPrivate::Before );
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

    bool space = FALSE;
    if ( d->mb && !d->mb->testWState(Qt::WState_ForceHide) ) {
	d->tll->setMenuBar( d->mb );
	space = TRUE;
    }
    
    d->hideDock->setFixedHeight( style().toolBarHandleExtend() );

    if ( d->hidden && !d->hidden->isEmpty() ) {
	int visibles = 0;
	d->hideDock->show();
	QMainWindowPrivate::ToolBar *tb;
	for ( tb = d->hidden->first(); tb; tb = d->hidden->next() ) {
	    if ( tb->t->isVisible() )
		visibles++;
	    tb->t->resize( 0, 0 );
	    tb->t->move( -tb->t->width() - 2, -tb->t->height() - 2 );
	}
	if ( !visibles ) {
	    d->hideDock->hide();
	} else {
	    d->hideDock->repaint( TRUE );
	    update();
	}
    } else {
	d->hideDock->hide();
    }
    if ( d->hideDock->isVisible() && space && style() == WindowsStyle )
	d->tll->addSpacing( 2 );
    d->tll->addWidget( d->hideDock );
    d->tll->addSpacing( 1 );

    d->lTop = new QToolLayout( d->tll, d->top, QBoxLayout::Down, d->justify );

    QMainWindowLayout *mwl = new QMainWindowLayout( d->tll );

    d->tll->setStretchFactor( mwl, 100 );
    d->lLeft = new QToolLayout( mwl, d->left, QBoxLayout::LeftToRight, d->justify );
    mwl->setLeftDock( d->lLeft );

    if ( centralWidget() &&
	 !centralWidget()->testWState(Qt::WState_ForceHide) )
	mwl->setCentralWidget( centralWidget() );
    d->lRight = new QToolLayout( mwl, d->right, QBoxLayout::LeftToRight, d->justify );
    mwl->setRightDock( d->lRight );

    d->lBottom = new QToolLayout( d->tll, d->bottom, QBoxLayout::Down, d->justify );

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
    if ( style() == WindowsStyle && d->mb && d->top && !d->top->isEmpty() ) {
	QPainter p( this );
	int y = d->mb->height() + 1;
	style().drawSeparator( &p, 0, y, width() - 1, y, colorGroup() );
    }
}


/*!
  \reimp
*/

bool QMainWindow::eventFilter( QObject* o, QEvent *e )
{
    if ( e->type() == QEvent::MouseButtonPress &&
	 o == this && !d->inMovement &&
	 ( (QMouseEvent*)e )->button() == RightButton ) {
	QMouseEvent *me = (QMouseEvent*)e;
	rightMouseButtonMenu( me->globalPos() );
    } else if ( ( e->type() == QEvent::MouseButtonPress ||
	   e->type() == QEvent::MouseMove ||
	   e->type() == QEvent::MouseButtonRelease )
	 && o && o->inherits( "QToolBar" )  ) {
	QMouseEvent *me = (QMouseEvent*)e;
	if ( d->movable && ( ( me->button() & LeftButton || me->state() & LeftButton ) ||
	     ( ( me->button() & RightButton ) && d->dockMenu ) ) ) {
	    moveToolBar( (QToolBar *)o, me );
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
  \fn void QMainWindow::startMovingToolBar( QToolBar *toolbar )

  This signal is emitted when the \a toolbar starts moving because
  the user started dragging it.
*/

/*!
  \fn void QMainWindow::endMovingToolBar( QToolBar *toolbar )

  This signal is emitted if the \a toolbar has been moved by
  the user and he/she released the mouse button now, so he/she
  stopped the moving.
*/

/*!
  \fn void QMainWindow::toolBarPositionChanged( QToolBar *toolbar )

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
	    int visibles = 0;
	    d->hideDock->show();
	    QMainWindowPrivate::ToolBar *tb;
	    for ( tb = d->hidden->first(); tb; tb = d->hidden->next() ) {
		if ( tb->t->isVisible() )
		    visibles++;
		tb->t->resize( 0, 0 );
		tb->t->move( -tb->t->width() - 2, -tb->t->height() - 2 );
	    }
	    if ( !visibles ) {
		d->hideDock->hide();
	    } else {
		d->hideDock->repaint( TRUE );
		update();
	    }
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

/*!
  \internal
  Finds the docking area for the toolbar \a tb. \a pos has to be the position of the mouse
  inside the QMainWindow. \a rect is set to the docking area into which the toolbar should
  be moved if the position of the mouse is \a pos. This method returns the identifier of
  the docking area, which is described by \a rect.

  If the mouse is not in any docking area, Unmanaged is returned as docking area.
*/

QMainWindow::ToolBarDock QMainWindow::findDockArea( const QPoint &pos, QRect &rect,
						    QToolBar *tb, QRect *r2 )
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
	left = 10;
    }
    if ( right < 20 ) {
	hasRight = FALSE;
	right = 10;
    }
    if ( top < 20 ) {
	hasTop = FALSE;
	top = 10;
    }
    if ( bottom < 20 ) {
	hasBottom = FALSE;
	bottom = 10;
    }

    // calculate the docking areas
    QRect leftArea( -10, h1, left + 20, height() - h1 - h2 );
    QRect topArea( -10, h1 - 10, width() + 20, top + 20 );
    QRect rightArea( width() - right - 10, h1, right + 20, height() - h1 - h2 );
    QRect bottomArea( -10, height() - bottom - h2 - 10, width() + 20, bottom + 20 );
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
	    rect = findRectInDockingArea( d, Left, pos, tb );
	    if ( r2 )
		*r2 = leftArea;
	    return Left;
	}
	rect = findRectInDockingArea( d, Top, pos, tb );
	if ( r2 )
	    *r2 = topArea;
	return Top;
    }
    if ( leftBottom.contains( pos ) ) {
	if ( tb->orientation() == Vertical && !hasBottom ) {
	    rect = findRectInDockingArea( d, Left, pos, tb );
	    if ( r2 )
		*r2 = leftArea;
	    return Left;
	}
	rect = findRectInDockingArea( d, Bottom, pos, tb );
	if ( r2 )
	    *r2 = bottomArea;
	return Bottom;
    }
    if ( rightTop.contains( pos ) ) {
	if ( tb->orientation() == Vertical && !hasTop ) {
	    rect = findRectInDockingArea( d, Right, pos, tb );
	    if ( r2 )
		*r2 = rightArea;
	    return Right;
	}
	rect = findRectInDockingArea( d, Top, pos, tb );
	if ( r2 )
	    *r2 = topArea;
	return Top;
    }
    if ( rightBottom.contains( pos ) ) {
	if ( tb->orientation() == Vertical && !hasBottom ) {
	    rect = findRectInDockingArea( d, Right, pos, tb );
	    if ( r2 )
		*r2 = rightArea;
	    return Right;
	}
	rect = findRectInDockingArea( d, Bottom, pos, tb );
	if ( r2 )
	    *r2 = bottomArea;
	return Bottom;
    }

    // if the mouse is not in an intersection, it's easy....
    if ( leftArea.contains( pos ) ) {
	rect = findRectInDockingArea( d, Left, pos, tb );
	if ( r2 )
	    *r2 = leftArea;
	return Left;
    }
    if ( topArea.contains( pos ) ) {
	rect = findRectInDockingArea( d, Top, pos, tb );
	if ( r2 )
	    *r2 = topArea;
	return Top;
    }
    if ( rightArea.contains( pos ) ) {
	rect = findRectInDockingArea( d, Right, pos, tb );
	if ( r2 )
	    *r2 = rightArea;
	return Right;
    }
    if ( bottomArea.contains( pos ) ) {
	rect = findRectInDockingArea( d, Bottom, pos, tb );
	if ( r2 )
	    *r2 = bottomArea;
	return Bottom;
    }

    rect = QRect( pos - d->cursorOffset, tb->size() );

    // mouse pos outside of any docking area
    return Unmanaged;
}


/*!
  Handles mouse event \a e on behalf of tool bar \a t and does all
  the funky docking.
*/

void QMainWindow::moveToolBar( QToolBar* t , QMouseEvent * e )
{
    if ( e->type() == QEvent::MouseButtonPress && !d->inMovement ) {
	if ( t->orientation() == Horizontal ) {
	    if ( e->x() > 10 )
		return;
	}
	
	if ( t->orientation() == Vertical ) {
	    if ( e->y() > 10 )
		return;
	}
	
	if ( ( e->button() & RightButton ) ) {
	    if ( !isDockMenuEnabled() )
		return;
	    emit startMovingToolBar( t );
	    d->inMovement = TRUE;
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
	    menu.setItemEnabled( left, isDockEnabled( Minimized ) && isDockEnabled( t, Minimized ) );
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
		moveToolBar( t,  Minimized );
	    emit endMovingToolBar( t );
	    d->inMovement = FALSE;
	    return;
	}
	if ( ( e->button() & MidButton ) ) {
	    return;
	}
	emit startMovingToolBar( t );
	d->inMovement = TRUE;

	// don't allow repaints of the central widget as this may be a problem for our rects
	if ( d->mc ) {
	    if ( d->mc->inherits( "QScrollView" ) )
		( (QScrollView*)d->mc )->viewport()->setUpdatesEnabled( FALSE );
	    else
		d->mc->setUpdatesEnabled( FALSE );
	}

	// create the painter for our rects
	if ( !d->opaque ) {
	    uint flags = getWFlags();
	    setWFlags( WPaintUnclipped );
	    d->rectPainter = new QPainter;
	    setWFlags( flags );
	    d->rectPainter->begin( this );
	    d->rectPainter->setPen( QPen( color0, 2 ) );
	    d->rectPainter->setRasterOp( NotROP );
	}

	// init some stuff
	QPoint pos = mapFromGlobal( e->globalPos() );
	QRect r;
	QMainWindow::ToolBarDock dock = findDockArea( pos, r, t );
	r = QRect( t->pos(), t->size() );
	d->oldPosRect = r;
	d->origPosRect = r;
	d->oldPosRectValid = FALSE;
	d->pos = mapFromGlobal( e->globalPos() );
	d->movedEnough = FALSE;
	d->origDock = d->oldDock = dock;
	d->cursorOffset = t->mapFromGlobal( e->globalPos() );

	return;
    } else if ( e->type() == QEvent::MouseButtonRelease && d->inMovement ) {
	if ( ( e->button() & RightButton ) ) {
	    return;
	}
	if ( ( e->button() & MidButton ) ) {
	    return;
	}

	// finally really move the toolbar, if the mouse was moved...
	if ( d->movedEnough ) {
	    if ( !d->opaque ) {
		ToolBarDock dock = d->oldDock;
		if ( dock != Unmanaged && isDockEnabled( dock ) &&
		     isDockEnabled( t, dock ) ) {
		    int ipos;
		    QToolBar *relative;
		    QPoint pos = mapFromGlobal( e->globalPos() );
		    QRect r, r2;
		    findDockArea( pos, r, t, &r2 );
		    if ( dock != d->origDock ) {
			saveToolLayout( d, d->origDock, t );
		    }
		    findNewToolbarPlace( d, t, dock, r2, relative, ipos );
		    moveToolBar( t, dock, relative, ipos );
		} else {
		    QPoint op = t->pos();
		    QRect r = d->oldPosRect;
		    int a = r.x() - op.x();
		    int b = r.y() - op.y();
		    int ap = a / 20;
		    int bp = b / 20;
		    QTime time;
		    time.start();
		    int i = 0;
		    while ( TRUE ) {
			if ( time.elapsed() > 10 ) {
			    ++i;
			    d->rectPainter->drawRect( r );
			    r.moveBy( -ap, -bp );
			    d->rectPainter->drawRect( r );
			    qApp->processEvents();
			    if ( i == 20 )
				break;
			    time.start();
			}
		    }
		    d->oldPosRect = r;
		}
	    }
	} else { // ... or hide it if it was only a click
	    saveToolLayout( d, d->origDock, t );
	    if ( isDockEnabled( Minimized ) && isDockEnabled( t, Minimized ) )
		moveToolBar( t, Minimized );
	}

	// delete the rect painter
	if ( d->rectPainter && !d->opaque ) {
	    if ( d->oldPosRectValid )
		d->rectPainter->drawRect( d->oldPosRect );
	    d->rectPainter->end();
	}

	if ( !d->opaque ) {
	    delete d->rectPainter;
	    d->rectPainter = 0;
	}

	// allow repaints in central widget again
	if ( d->mc ) {
	    if ( d->mc->inherits( "QScrollView" ) )
		( (QScrollView*)d->mc )->viewport()->setUpdatesEnabled( TRUE );
	    else
		d->mc->setUpdatesEnabled( TRUE );
	}

	emit endMovingToolBar( t );
	d->inMovement = FALSE;

	return;
    } else if ( e->type() == QMouseEvent::MouseMove ) {
	if ( ( e->state() & LeftButton ) != LeftButton || !d->inMovement )
	    return;
    }

    // find out if the mouse had been moved yet...
    QPoint p( e->globalPos() );
    QPoint pos = mapFromGlobal( p );
    if ( !d->movedEnough && pos != d->pos ) {
    	d->movedEnough = TRUE;
    }

    // if no mouse movement yet, don't do anything
    if ( !d->movedEnough )
	return;

    // find the dock, rect, etc. for the current mouse pos
    d->pos = pos;
    QRect r;
    ToolBarDock dock = findDockArea( pos, r, t );

    // draw the new rect where the toolbar would be moved
    if ( d->rectPainter && !d->opaque ) {
	if ( d->oldPosRectValid && d->oldPosRect != r )
	    d->rectPainter->drawRect( d->oldPosRect );
	if ( dock == Unmanaged || !isDockEnabled( dock ) ||
	     !isDockEnabled( t, dock ) ) {
#if defined(_WS_WIN32_)
	    d->rectPainter->setPen( QPen( color0, 1, DashLine ) );
#else
	    d->rectPainter->setPen( QPen( color0, 2, DashLine ) );
#endif
	} else {
	    d->rectPainter->setPen( QPen( color0, 2 ) );
	}
	if ( !d->oldPosRectValid || d->oldPosRect != r )
	    d->rectPainter->drawRect( r );
    } else if ( d->opaque ) {
	if ( dock == Unmanaged ) {
	    dock = d->origDock;
	} else {
	    int ipos;
	    QToolBar *relative;
	    QRect r, r2;
	    findDockArea( pos, r, t, &r2 );
	    if ( dock != d->origDock ) {
		saveToolLayout( d, d->origDock, t );
	    }
	    findNewToolbarPlace( d, t, dock, r2, relative, ipos );
	    moveToolBar( t, dock, relative, ipos );
	}
    }
    d->oldPosRect = r;
    d->oldPosRectValid = TRUE;
    d->oldDock = dock;
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

bool QMainWindow::getLocation( QToolBar *tb, ToolBarDock &dock, int &index, bool &nl, int &extraOffset ) const
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
	dock = Minimized;

    index = td->findRef( t );
    nl = t->nl;
    extraOffset = t->extraOffset;

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
    case Minimized:
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
  different toolbar docks by the user. By default toolbars are moved
  transparent, but this setting can be changed by setOpaqueMoving().

  The default is TRUE.

  \sa setDockEnabled(), toolbarsMovable(), setOpaqueMoving()
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

void QMainWindow::lineUpToolBars( bool keepNewLines )
{
    if ( !d->movable )
	return;

    QMainWindowPrivate::ToolBarDock* docks[] = {
	d->left, d->right, d->top, d->bottom, d->unmanaged, d->tornOff, d->hidden
    };

    QMainWindowPrivate::ToolBarDock *l = 0;

    for ( unsigned int i = 0; i < 7; ++i ) {
	l = docks[ i ];
	if ( !l || l->isEmpty() )
	    continue;
	QMainWindowPrivate::ToolBar *t = 0;
	for ( t = l->first(); t; t = l->next() ) {
	    t->extraOffset = -1;
	    if ( !keepNewLines )
		t->nl = FALSE;
	}
    }

    triggerLayout();
}

/*!
  \internal
*/

void QMainWindow::rightMouseButtonMenu( const QPoint &p )
{
    if ( !d->movable || !d->dockMenu )
	return;

    QMainWindowPrivate::ToolBarDock* docks[] = {
	d->left, d->right, d->top, d->bottom, d->unmanaged, d->tornOff, d->hidden
    };

    QMainWindowPrivate::ToolBarDock *l = 0;
    QIntDict<QMainWindowPrivate::ToolBar> ids;
    bool sep;
    QPopupMenu m;
    m.setCheckable( TRUE );
    for ( unsigned int i = 0; i < 7; ++i ) {
	sep = FALSE;
	l = docks[ i ];
	if ( !l || l->isEmpty() )
	    continue;
	QMainWindowPrivate::ToolBar *t = 0;
	for ( t = l->first(); t; t = l->next() ) {
	    if ( !t->t->label().isEmpty() ) {
		int id = m.insertItem( t->t->label() );
		ids.insert( id, t );
		if ( l != d->hidden )
		    m.setItemChecked( id, TRUE );
		sep = TRUE;
	    }
	}
	if ( sep )
	    m.insertSeparator();
    }
    int lineUp1 = m.insertItem( tr( "Line Up Toolbars (compact)" ) );
    int lineUp2 = m.insertItem( tr( "Line Up Toolbars (normal)" ) );

    int id = m.exec( p );
    if ( id == lineUp1 ) {
	lineUpToolBars( FALSE );
    } else if ( id == lineUp2 ) {
	lineUpToolBars( TRUE );
    } else if ( ids.find( id ) ) {
	QMainWindowPrivate::ToolBar *t = ids[ id ];
	if ( m.isItemChecked( id ) ) {
	    if ( isDockEnabled( Minimized ) && isDockEnabled( t->t, Minimized ) )
		moveToolBar( t->t, Minimized );
	} else {
	    t->t->show();
	    moveToolBar( t->t, t->oldDock, t->nl, t->oldIndex, t->extraOffset );
	}
    }
}

/*!
  Returns TRUE, if rightclicking on an empty space on a toolbar dock
  or rightclicking on a toolbar handle opens a popup menu which allows
  lining up toolbars and hiding/showing toolbars.

  \sa setDockEnabled(), lineUpToolBars()
*/

bool QMainWindow::isDockMenuEnabled() const
{
    return d->dockMenu;
}

/*!
  When passing TRUE for \a b here, rightclicking on an empty space on a toolbar dock
  or rightclicking on a toolbar handle opens a popup menu which allows lining up toolbars
  and hiding/showing toolbars.

  \sa lineUpToolBars(), isDockMenuEnabled()
*/

void QMainWindow::setDockMenuEnabled( bool b )
{
    d->dockMenu = b;
}
