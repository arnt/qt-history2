/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qtabbar.cpp#10 $
**
** Implementation of QTabBar class
**
** Copyright (C) 1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qtabbar.h"
#include "qkeycode.h"

RCSTAG("$Id: //depot/qt/main/src/widgets/qtabbar.cpp#10 $");


QTab::~QTab()
{
    // gcc says: just give me a vtbl and I'll be satisfied
}

// this struct can be expanded without breaking binary compatibility
struct QTabPrivate {
    int id;
    int focus;
    QTab * pressed;
};


/*!
  \class QTabBar qtabbar.h

  \brief The QTabBar class provides a tab bar, for use in e.g. tabbed
  dialogs.

  As implemented, the class provides a look and feel suitable for
  QTabDialog.  It can be subclassed easily, to provide tab bars with
  other appearances.

  The following virtual functions may need to be reimplemented: <ul>
  <li> paint() paints a single tab.  paintEvent() calls paint() for
  each tab in such a way that any overlap will look right.  <li>
  addTab() creates a new tab and adds it to the bar. <li> selectTab()
  decides which, if any, tab the user selects with the mouse. </ul>

*/

/*!
  Create a new, empty tab bar.
*/

QTabBar::QTabBar( QWidget * parent, const char * name )
    : QWidget( parent, name )
{
    d = new QTabPrivate;
    d->id = 0;
    d->focus = 0;
    l = new QListT<QTab>;
    l->setAutoDelete( TRUE );
    setFocusPolicy( TabFocus );
}


/*!
  Delete the tab control and free the memory it used.
*/

QTabBar::~QTabBar()
{
    delete d;
    delete l;
}


/*!
  Add \a newTab to the tab control.

  Allocate a new id, set t's id, locate it just to the right of the
  existing tabs, add it to the bar, and return the newly allocated id.
*/

int QTabBar::addTab( QTab * newTab )
{
    QFontMetrics fm = fontMetrics();
    QTab  *t = l->first();
    QRect br( fm.boundingRect( newTab->label ) );
    if ( t ) {
	QRect r( t->r );
	while ( (t = l->next()) != 0 )
	    r = r.unite( t->r );
	newTab->r.setRect( r.right(), 0, br.width() + 24,
			   QMAX( r.height(), fm.height() + 10 ) );
    } else {
	newTab->r.setRect( 0, 0,
			   br.width() + 24, fm.height() + 10 );
    }

    newTab->id = d->id++;
    l->append( newTab );

    return newTab->id;
}


/*!
  Enable tab \a id if \a enable is TRUE, or disable it if \a enable is
  FALSE.

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
		repaint( t->r );
	    }
	    return;
	}
    }
}


/*!
  Return TRUE if the tab with id \a id is enabled, or FALSE if it
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


/*!  Paint the single tab \a t using \a p.  If and only if \a selected
  is TRUE, \a t is currently selected.

  This virtual function may be reimplemented to change the look of
  QTabBar.  If you decide to reimplement it, you may also need to
  reimplement sizeHint().
*/

void QTabBar::paint( QPainter * p, QTab * t, bool selected ) const
{
    p->setPen( white );
    if ( selected ) {
	p->drawLine( t->r.left(), t->r.bottom() - 1, t->r.left(), 2 );
	p->drawPoint( t->r.left()+1, 1 );
	p->drawLine( t->r.left()+2, t->r.top(),
		     t->r.right() - 5, t->r.top() );
	p->setPen( colorGroup().dark() );
	p->drawPoint( t->r.right() - 4, t->r.top()+1 );
	p->drawLine( t->r.right() - 3, t->r.top() + 2,
		     t->r.right() - 3, t->r.bottom() - 2);
	p->setPen( black );
	p->drawPoint( t->r.right() - 3, t->r.top()+1 );
	p->drawLine( t->r.right() - 2, t->r.top() + 2,
		     t->r.right() - 2, t->r.bottom() - 2);
	p->setPen( white );
	p->drawLine( t->r.right()-3, t->r.bottom() - 1,
		     t->r.right(), t->r.bottom() - 1 );
	QFont bold( font() );
	bold.setWeight( QFont::Bold );
	p->setFont( bold );
    } else {
	p->drawLine( t->r.left()+1, t->r.bottom() - 2, t->r.left()+1, 4 );
	p->drawPoint( t->r.left()+2, 3 );
	p->drawLine( t->r.left()+3, t->r.top() + 2,
		     t->r.right() - 4, t->r.top() + 2 );
	p->setPen( colorGroup().dark() );
	p->drawPoint( t->r.right() - 3, t->r.top() + 3 );
	p->drawLine( t->r.right() - 2, t->r.top() + 4,
		     t->r.right() - 2, t->r.bottom() - 2);
	p->setPen( black );
	p->drawPoint( t->r.right() - 2, t->r.top() + 3 );
	p->drawLine( t->r.right() - 1, t->r.top() + 4,
		     t->r.right() - 1, t->r.bottom() - 2);
	p->setPen( white );
	p->drawLine( t->r.left(), t->r.bottom() - 1,
		     t->r.right(), t->r.bottom() - 1 );
	p->setFont( font() );
    }

    if ( t->enabled )
	p->setPen( palette().normal().foreground() );
    else
	p->setPen( palette().disabled().foreground() );

    QRect br = p->fontMetrics().boundingRect( t->label );
    br.setHeight( p->fontMetrics().height() );
    br.setRect( t->r.left() + (t->r.width()-br.width())/2 - 3,
		t->r.top() + (t->r.height()-br.height())/2,
		br.width() + 5,
		br.height() + 2 );

    p->drawText( br, AlignCenter, t->label );

    if ( t->id != keyboardFocusTab() )
	return;
    if ( style() == WindowsStyle )
	p->drawWinFocusRect( br );
    else
	p->drawRect( br );
}


/*!
  Repaints the tab row.  All the painting is done by paint();
  paintEvent() only decides which tabs need painting and in what
  order.

  \sa paint()
*/

void QTabBar::paintEvent( QPaintEvent * e )
{
    QPainter p;

    p.begin( this );
    p.setClipping( TRUE );
    p.setClipRect( e->rect() );

    QTab * t;
    t = l->first();
    do {
	QTab * n = l->next();
	if ( t && t->r.intersects( e->rect() ) )
	    paint( &p, t, n == 0 );
	t = n;
    } while ( t != 0 );

    p.end();
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

  When when you need to display something, the return value from this
  function represents the best page to display.  That's all.

  \sa selected()
*/

int QTabBar::currentTab() const
{
    QTab * t = l->last();

    return t ? t->id : -1;
}



/*!  If this tab control has keyboard focus, returns the ID of the
  tab Space will select.  Otherwise, returns -1.
*/

int QTabBar::keyboardFocusTab() const
{
    return hasFocus() ? d->focus : -1;
}


/*!  Handles the tab bar's keyboard interface (if enabled).

  The right and left arrow keys move a selector, the space bar makes
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
    } else if ( e->key() == Key_Space ) {
	// space - select
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
	    if ( r.isValid() )
		repaint( r );
	    emit selected( t->id );
	}
    } else {
	// other keys - ignore
	e->ignore();
    }

    // if the focus moved, repaint a bit
    if ( old != d->focus ) {
	QRect r( 0,0, -1,-1 );
	QTab * t = l->first();
	while ( t ) {
	    if ( t->id == d->focus || t->id == old )
		r = r.unite( t->r );
	    t = l->next();
	}
	if ( r.isValid() )
	    repaint( r );
    }
}
