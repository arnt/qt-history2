/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qtabbar.cpp#56 $
**
** Implementation of QTabBar class
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qtabbar.h"
#include "qaccel.h"
#include "qbitmap.h"

#include <ctype.h>

/*!
  \class QTab qtabbar.h
  \brief The structures in a QTabBar.

  For custom QTabBar tab headings.

  \sa QTabBar
*/

QTab::~QTab()
{
    // gcc says: just give me a vtbl and I'll be satisfied
}

// this struct can be expanded without breaking binary compatibility
struct QTabPrivate {
    int id;
    int focus;
    QTab * pressed;
    QAccel * a;
    QTabBar::Shape s;
};


/*!
  \class QTabBar qtabbar.h

  \brief The QTabBar class provides a tab bar, for use in e.g. tabbed
  dialogs.

  \ingroup realwidgets

  The class is quite simple; it draws the tabs in one of four shapes
  and emits a signal when one is selected.  It can be subclassed to
  tailor the look and feel.

  The four possible shapes are <ul> <li> \c RoundedAbove - (the
  default) rounded tabs to be located above the pages.  <li> \c
  RoundedBelow - rounded tabs to be located below the pages. <li> \c
  TriangularAbove - triangular tabs to be located above the pages
  (very unusual, included for completeness). <li> \c TriangularBelow
  - triangular tabs to be located below the pages. </ul>

  The choice of tab shape is still a matter of taste, to a large
  degree.  Tab dialogs (preferences and the like) invariable use \c
  RoundedAbove and nobody uses \c TriangularAbove.  Tab controls in
  windows other than dialogs almost always either \c RoundedBelow or
  \c TriangularBelow.  Many spreadsheets and other tab controls where
  all the pages are essentially similar to use \c TriangularBelow,
  while \c RoundedBelow is used mostly when the pages are different
  (e.g. a multi-page tool palette).  There is no strong tradition yet,
  however, so use your taste and create the tradition.

  The most important part of QTabBar's API is the signal selected().
  It's emitted whenever the selected page changes (even at startup,
  when the selected page changes from 'none').  There are also a slot,
  setCurrentTab(), which can be used to select a page
  programmatically.

  QTabBar creates automatic accelerator keys in the manner of QButton;
  e.g. if a tab's label is "\&Graphics" Alt-G becomes an accelerator
  key for switching to that tab.

  The following virtual functions may need to be reimplemented: <ul>
  <li> paint() paints a single tab.  paintEvent() calls paint() for
  each tab in such a way that any overlap will look right.  <li>
  addTab() creates a new tab and adds it to the bar. <li> selectTab()
  decides which, if any, tab the user selects with the mouse. </ul>

  <img src=qtabbar-m.gif> <img src=qtabbar-w.gif>
*/

/*!
  \fn void QTabBar::selected( int id )

  QTabBar emits this signal whenever any tab is selected, whether by
  the program or the user.  The argument \a id is the ID if the tab
  as returned by addTab().

  show() is guaranteed to emit this signal, so that you can display
  your page in a slot connected to this signal.
*/

/*!
  Creates a new, empty tab bar.
*/

QTabBar::QTabBar( QWidget * parent, const char *name )
    : QWidget( parent, name )
{
    d = new QTabPrivate;
    d->id = 0;
    d->focus = 0;
    d->a = new QAccel( this, "tab accelerators" );
    d->s = RoundedAbove;
    l = new QList<QTab>;
    l->setAutoDelete( TRUE );
    setFocusPolicy( TabFocus );

    connect( d->a, SIGNAL(activated(int)), this, SLOT(setCurrentTab(int)) );
}


/*!
  Deletes the tab control and free the memory it used.
*/

QTabBar::~QTabBar()
{
    delete d;
    delete l;
}


/*!
  Adds \a newTab to the tab control.

  Allocates a new id, sets t's id, locates it just to the right of the
  existing tabs, inserts an accelerator if the tab's label contains the
  string "&p" for some value of p, adds it to the bar, and returns the
  newly allocated id.
*/

int QTabBar::addTab( QTab * newTab )
{
    QFontMetrics fm = fontMetrics();
    QTab  *t = l->first();
    int lw = fm.width( newTab->label );
    int h = fm.height();
    if ( d->s == RoundedAbove || d->s == RoundedBelow )
	h += 10;
    if ( t ) {
	QRect r( t->r );
	while ( (t = l->next()) != 0 )
	    r = r.unite( t->r );
	newTab->r.setRect( r.right()-3, 0, lw + 24,
			   QMAX( r.height(), h ) );
    } else {
	newTab->r.setRect( 0, 0, lw + 24, h );
    }

    newTab->id = d->id++;
    l->append( newTab );

    int p = QAccel::shortcutKey( newTab->label );
    if ( p )
	d->a->insertItem( p, newTab->id );

    return newTab->id;
}


/*!
  Enable tab \a id if \a enable is TRUE, or disable it if \a enable is
  FALSE.  If \a id is currently selected, setTabEnabled() makes
  another tab selected.

  setTabEnabled() calls repaint() if this causes a change in \a id's
  status.

  \sa update(), isTabEnabled()
*/

void QTabBar::setTabEnabled( int id, bool enabled )
{
    QTab * t;
    for( t = l->first(); t; t = l->next() ) {
	if ( t && t->id == id ) {
	    if ( t->enabled != enabled ) {
		t->enabled = enabled;
		d->a->setItemEnabled( t->id, enabled );
		QRect r( t->r );
		if ( !enabled && id == currentTab() ) {
		    QPoint p1( t->r.center() ), p2;
		    int m = 2147483647;
		    int distance;
		    // look for the closest enabled tab - measure the
		    // distance between the centers of the two tabs
		    for( QTab * n = l->first(); n; n = l->next() ) {
			if ( n->enabled ) {
			    p2 = n->r.center();
			    distance = (p2.x() - p1.x())*(p2.x() - p1.x()) +
				       (p2.y() - p1.y())*(p2.y() - p1.y());
			    if ( distance < m ) {
				t = n;
				distance = m;
			    }
			}
		    }
		    if ( t->enabled ) {
			r = r.unite( t->r );
			l->append( l->take( l->findRef( t ) ) );
			emit selected( t->id );
		    }
		}
		updateMask();
		repaint( r );
	    }
	    return;
	}
    }
}


/*!
  Returns TRUE if the tab with id \a id is enabled, or FALSE if it
  is disabled or there is no such tab.

  \sa setTabEnabled()
*/

bool QTabBar::isTabEnabled( int id ) const
{
    QTab * t;
    for( t = l->first(); t; t = l->next() ) {
	if ( t && t->id == id )
	    return t->enabled;
    }
    return FALSE;
}



/*!
  Returns a suitable size for the tab control.
*/

QSize QTabBar::sizeHint() const
{
    QTab * t = l->first();
    if ( t ) {
	QRect r( t->r );
	while ( (t = l->next()) != 0 )
	    r = r.unite( t->r );
	return r.size();
    } else {
	return QSize( 0, 0 );
    }
}

/*!
  Specifies that this widget can use more, but is able to survive on
  less, horizontal space; and has a fixed height.
*/

QSizePolicy QTabBar::sizePolicy() const
{
    return QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed );
}


/*!  Paint the single tab \a t using \a p.  If and only if \a selected
  is TRUE, \a t is currently selected.

  This virtual function may be reimplemented to change the look of
  QTabBar.  If you decide to reimplement it, you may also need to
  reimplement sizeHint().
*/

void QTabBar::paint( QPainter * p, QTab * t, bool selected ) const
{
    QRect r( t->r );
    int o = style().defaultFrameWidth() > 1 ? 1 : 0;
    if ( d->s == RoundedAbove ) {
	if ( selected ) {
	    p->setPen( colorGroup().background() );
	    p->drawLine( r.left()+1+o, r.bottom(), r.right()-2, r.bottom() );
	    p->drawLine( r.left()+1+o, r.bottom(), r.left()+1+o, r.top()+2 );
	    if ( style() == MotifStyle )
		p->setPen( colorGroup().light() ); // uglify
	    else
		p->setPen( colorGroup().midlight() );
	    if ( o )
		p->drawLine( r.left(), r.top()+3,
			     r.left(), r.bottom());
	    p->setPen( colorGroup().light() );
	} else {
	    p->setPen( colorGroup().light() );
	    p->drawLine( r.left()+o, r.bottom(), r.right(), r.bottom() );
	    r.setRect( r.left() + 2, r.top() + 2,
		       r.width() - 4, r.height() - 2 );
	}

	p->drawLine( r.left()+o, r.bottom(), r.left()+o, r.top() + 2 );
	p->drawPoint( r.left()+1+o, r.top() + 1 );
	p->drawLine( r.left()+2+o, r.top(),
		     r.right() - 2, r.top() );

	p->setPen( colorGroup().dark() );
	p->drawLine( r.right() - 1, r.top() + 2,
		     r.right() - 1, r.bottom() - 1 );
	p->setPen( colorGroup().shadow() );
	if ( o ) {
	    p->drawPoint( r.right() - 1, r.top() + 1 );
	    p->drawLine( r.right(), r.top() + 2, r.right(), r.bottom() - 1 );
	}
    } else if ( d->s == RoundedBelow ) {
	if ( selected ) {
	    p->setPen( colorGroup().background() );
	    p->drawLine( r.left()+1+o, r.top(), r.right()-2, r.top() );
	    p->drawLine( r.left()+1+o, r.top(), r.left()+1+o, r.bottom()-2 );
	    if ( o ) {
		if ( style() == MotifStyle )
		    p->setPen( colorGroup().light() ); // uglify
		else
		    p->setPen( colorGroup().midlight() );
		p->drawLine( r.left(), r.top(),
			     r.left(), r.bottom() - 3 );
	    }
	    p->setPen( colorGroup().dark() );
	} else {
	    p->setPen( colorGroup().dark() );
	    p->drawLine( r.left()+o, r.top(), r.right(), r.top() );
	    r.setRect( r.left() + 1+o, r.top(),
		       r.width() - 4, r.height() - 2 );
	}

	p->drawLine( r.right() - 1, r.top(),
		     r.right() - 1, r.bottom() - 2 );
	p->drawPoint( r.right() - 2, r.bottom() - 2 );
	p->drawLine( r.right() - 2, r.bottom() - 1,
		     r.left() + o, r.bottom() - 1 );
	p->drawPoint( r.left() + o, r.bottom() - 2 );
	
	p->setPen( colorGroup().shadow() );
	if ( o ) {
	    p->drawLine( r.right(), r.top(),
			 r.right(), r.bottom() - 1 );
	    p->drawPoint( r.right() - 1, r.bottom() - 1 );
	    p->drawLine( r.right() - 1, r.bottom(),
			 r.left() + 2+o, r.bottom() );
	}

	if ( o || selected) {
	    p->setPen( colorGroup().light() );
	    p->drawLine( r.left()+o, r.top(),
			 r.left()+o, r.bottom() - 2 );
	}
    } else {
	
	// triangular, above or below
	int y;
	int x;
	QPointArray a( 10 );
	a.setPoint( 0, 0, -1 );
	a.setPoint( 1, 0, 0 );
	y = t->r.height()-2;
	x = y/3;
	a.setPoint( 2, x++, y-1 );
	a.setPoint( 3, x++, y );
	a.setPoint( 3, x++, y++ );
	a.setPoint( 4, x, y );
	
	int i;
	int right = t->r.width() - 1;
	for ( i = 0; i < 5; i++ )
	    a.setPoint( 9-i, right - a.point( i ).x(), a.point( i ).y() );

	if ( d->s == TriangularAbove )
	    for ( i = 0; i < 10; i++ )
		a.setPoint( i, a.point(i).x(),
			    t->r.height() - 1 - a.point( i ).y() );

	a.translate( t->r.left(), t->r.top() );

	if ( selected )
	    p->setBrush( colorGroup().base() );
	else
	    p->setBrush( colorGroup().background() );
	p->setPen( colorGroup().foreground() );
	p->drawPolygon( a );
	p->setBrush( NoBrush );
    }

    p->setFont( font() );

    int w = p->fontMetrics().width( t->label ) + 4;
    int h = p->fontMetrics().height() + 4;
    paintLabel( p, QRect( r.left() + (r.width()-w)/2 - 3,
			  r.top() + (r.height()-h)/2,
			  w, h ), t, t->id == keyboardFocusTab() );
}

/*!
  Paints the label of tab \a t centered in rectangle \a br using
  painter \a p and draws a focus indication if \a has_focus is TRUE.
*/

void QTabBar::paintLabel( QPainter* p, const QRect& br,
                          QTab* t, bool has_focus ) const
{
    if ( t->enabled ) {
	p->setPen( palette().normal().text() );
	p->drawText( br, AlignCenter | ShowPrefix, t->label );
    } else if ( style() == MotifStyle ) {
	p->setPen( palette().disabled().text() );
	p->drawText( br, AlignCenter | ShowPrefix, t->label );
    } else { // Windows style, disabled
	p->setPen( colorGroup().light() );
	QRect wr = br;
	wr.moveBy( 1, 1 );
	p->drawText( wr, AlignCenter | ShowPrefix, t->label );
	p->setPen( palette().disabled().text() );
	p->drawText( br, AlignCenter | ShowPrefix, t->label );
    }

    if ( !has_focus )
	return;

    if ( style() == WindowsStyle )
	p->drawWinFocusRect( br, backgroundColor() );
    else // shouldn't this be black, irrespective of everything?
	p->drawRect( br );
}


/*!
  Draws the mask for this tab bar.

  \internal
  This is not totally right - a few corner pixels missing.
*/

void  QTabBar::updateMask()
{
    if ( !autoMask() )
	return;
    QBitmap bm( size() );
    bm.fill( color0 );

    QPainter p;
    p.begin( &bm, this );
    p.setBrush(color1);
    p.setPen(color1);

    QTab *t = l->first();
    while ( t ) {
	p.drawRect( t->r );
	t  = l->next();
    }

    p.end();
    setMask( bm );
}

/*!
  Repaints the tab row.  All the painting is done by paint();
  paintEvent() only decides which tabs need painting and in what
  order.

  \sa paint()
*/

void QTabBar::paintEvent( QPaintEvent * e )
{
    QPainter p( this );
    p.setClipRegion( e->region() );

    QTab * t;
    t = l->first();
    do {
	QTab * n = l->next();
	if ( t && t->r.intersects( e->rect() ) )
	    paint( &p, t, n == 0 );
	t = n;
    } while ( t != 0 );

}


/*!
  This virtual functions is called by the mouse event handlers to
  determine which tab is pressed.  The default implementation returns
  a pointer to the tab whose bounding rectangle contains \a p, if
  exactly one tab's bounding rectangle contains \a p.  It returns 0
  else.

  \sa mousePressEvent() mouseReleaseEvent()
*/

QTab * QTabBar::selectTab( const QPoint & p ) const
{
    QTab * selected = 0;
    bool moreThanOne = FALSE;

    QListIterator<QTab> i( *l );
    while( i.current() ) {
	QTab * t = i.current();
	++i;

	if ( t && t->r.contains( p ) ) {
	    if ( selected )
		moreThanOne = TRUE;
	    else
		selected = t;
	}
    }

    return moreThanOne ? 0 : selected;
}


/*!
  Handles mouse press events; records what tab the mouse points to.
*/

void QTabBar::mousePressEvent( QMouseEvent * e )
{
    d->pressed = selectTab( e->pos() );
}


/*!
  Handles mouse release events for the tab control.  Checks that the
  mouse is released over the tab where it was pressed, and if it was,
  selects that tab.
*/

void QTabBar::mouseReleaseEvent( QMouseEvent * e )
{
    QTab * t = d->pressed;
    d->pressed = 0;
    if ( t != 0 && t == selectTab( e->pos() ) && t->enabled ) {
	QRect r = l->last()->r;
	if ( l->findRef( t ) >= 0 )
	    l->append( l->take() );

	d->focus = t->id;
	updateMask();
	if ( t->r.intersects( r ) ) {
	    repaint( r.unite( t->r ) );
	} else {
	    repaint( r );
	    repaint( t->r );
	}
	emit selected( t->id );
    }
}


/*!  Shows the widget, and ensures that one tab is selected.
*/

void QTabBar::show()
{
    QTab * t = l->last();
    if ( !isVisible() && (t = l->first()) != 0 )
	l->append( l->take() );
    QWidget::show();
    if ( t )
	emit selected( t->id );
}


/*!  If a page is currently visible, returns its ID.  If no page is
  currently visible, returns either -1 or the ID of one of the pages.

  Even if the return value is not -1, you cannot assume either that
  the user can see the relevant page, or that the tab \link
  isTabEnabled() is enabled.\endlink

  When you need to display something, the return value from this
  function represents the best page to display.  That's all.

  \sa selected()
*/

int QTabBar::currentTab() const
{
    QTab * t = l->last();

    return t ? t->id : -1;
}


/*! Raises the tab with ID \a id and emits the selected() signal.

  \sa currentTab() selected() tab()
*/

void QTabBar::setCurrentTab( int id )
{
    setCurrentTab( tab( id ) );
}


/*! Raises \a tab and emits the selected() signal unless the tab was
  already current.

  \sa currentTab() selected()
*/

void QTabBar::setCurrentTab( QTab * tab )
{
    if ( tab && l ) {
	if ( l->last() == tab )
	    return;

	QRect r = l->last()->r;
	if ( l->findRef( tab ) >= 0 )
	    l->append( l->take() );

	d->focus = tab->id;
	updateMask();
	if ( tab->r.intersects( r ) ) {
	    repaint( r.unite( tab->r ) );
	} else {
	    repaint( r );
	    repaint( tab->r );
	}
	emit selected( tab->id );
    }
}

/*!  If this tab control has keyboard focus, returns the ID of the
  tab Space will select.  Otherwise, returns -1.
*/

int QTabBar::keyboardFocusTab() const
{
    return hasFocus() ? d->focus : -1;
}


/*!  Handles the tab bar's keyboard interface (if enabled).

  The right and left arrow keys move a selector, the spacebar makes
  the tab with the selector active.  All other keys are ignored.
*/

void QTabBar::keyPressEvent( QKeyEvent * e )
{
    int old = d->focus;

    if ( e->key() == Key_Left ) {
	// left - skip past any disabled ones
	if ( d->focus > 0 ) {
	    QTab * t;
	    do {
		d->focus--;
		t = l->first();
		while ( t && t->id != d->focus )
		    t = l->next();
	    } while ( d->focus >= 0 && t && !t->enabled );
	}
	if ( d->focus < 0 )
	    d->focus = old;
	e->accept();
    } else if ( e->key() == Key_Right ) {
	// right - ditto
	if ( d->focus < d->id-1 ) {
	    QTab * t;
	    do {
		d->focus++;
		t = l->first();
		while ( t && t->id != d->focus )
		    t = l->next();
	    } while ( d->focus < d->id && t && !t->enabled );
	}
	if ( d->focus >= d->id )
	    d->focus = old;
	e->accept();
    } else {
	// other keys - ignore
	e->ignore();
    }

    // if the focus moved, repaint and signal
    if ( old != d->focus ) {
	QTab * t = l->last();
	QRect r( 0,0, -1, -1 );
	if ( t )
	    r = t->r;
	while ( t && t->id != d->focus )
	    t = l->prev();
	if ( t ) {
	    l->append( l->take() );
	    r = r.unite( t->r );
	    e->accept();
	    updateMask();
	    if ( r.isValid() )
		repaint( r );
	    emit selected( t->id );
	}
    }
}


/*!  Returns a pointer to the tab with ID \a id, or 0 if there is no
  such tab.
*/

QTab * QTabBar::tab( int id )
{
    QTab * t;
    for( t = l->first(); t; t = l->next() )
	if ( t && t->id == id )
	    return t;
    return 0;
}

/*!
  The list of QTab objects added.
*/
QList<QTab> * QTabBar::tabList()
{
    return l;
}


/*!  Returns the shape of this tab bar. \sa setShape() */

QTabBar::Shape QTabBar::shape() const
{
    return d ? d->s : RoundedAbove;
}


/*!  Sets the shape of this tab bar to \a s and refreshes the bar.
*/

void QTabBar::setShape( Shape s )
{
    if ( !d || d->s == s )
	return;
    //######### must recalculate heights
    d->s = s;
    updateMask();
    repaint();
}

