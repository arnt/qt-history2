/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qtabbar.cpp#22 $
**
** Implementation of QTabBar class
**
** Copyright (C) 1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qtabbar.h"
#include "qkeycode.h"
#include "qaccel.h"

#include <ctype.h>

RCSTAG("$Id: //depot/qt/main/src/widgets/qtabbar.cpp#22 $");


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

  <img src=qtabbar-m.gif> <img src=qtabbar-w.gif>
*/

/*! \fn void QTabBar::selected( int id )

  QTabBar emits this signal whenever any tab is selected, whether by
  the program or the user.  The argument \a id is the ID if the tab
  as returned by addTab().

  show() is guaranteed to emit this signal, so that you can display
  your page in a slot connected to this signal.
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
    d->a = new QAccel( this, "tab accelerators" );
    l = new QListT<QTab>;
    l->setAutoDelete( TRUE );
    setFocusPolicy( TabFocus );

    connect( d->a, SIGNAL(activated(int)), this, SLOT(setCurrentTab(int)) );
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
  existing tabs, insert an accelerator if the tab's label contains the
  string "&p" for some value of p, add it to the bar, and return the
  newly allocated id.
*/

int QTabBar::addTab( QTab * newTab )
{
    QFontMetrics fm = fontMetrics();
    QTab  *t = l->first();
    int lw = fm.width( newTab->label );
    if ( t ) {
	QRect r( t->r );
	while ( (t = l->next()) != 0 )
	    r = r.unite( t->r );
	newTab->r.setRect( r.right()-3, 0, lw + 24,
			   QMAX( r.height(), fm.height() + 10 ) );
    } else {
	newTab->r.setRect( 0, 0, lw + 24, fm.height() + 10 );
    }

    newTab->id = d->id++;
    l->append( newTab );

    const char * p = strchr( newTab->label, '&' );
    while( p && *p && p[1] == '&' )
	p = strchr( p+2, '&' );
    if ( p && *p && isalpha(p[1]) )
	d->a->insertItem( ALT + toupper(p[1]), newTab->id );

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
		    int d;
		    // look for the closest enabled tab - measure the
		    // distance between the centers of the two tabs
		    for( QTab * n = l->first(); n; n = l->next() ) {
			if ( n->enabled ) {
			    p2 = n->r.center();
			    d = (p2.x() - p1.x())*(p2.x() - p1.x()) +
				(p2.y() - p1.y())*(p2.y() - p1.y());
			    if ( d < m ) {
				t = n;
				d = m;
			    }
			}
		    }
		    if ( t->enabled ) {
			r = r.unite( t->r );
			l->append( l->take( l->findRef( t ) ) );
			emit selected( t->id );
		    }
		}
		repaint( r );
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

    QRect r( t->r );
    if ( selected ) {
	p->setPen( colorGroup().background() );
	p->drawLine( r.left()+1, r.bottom(), r.right()-2, r.bottom() );
	p->drawLine( r.left()+1, r.bottom(), r.left()+1, r.top()+2 );
	p->setPen( colorGroup().light() );
	QFont bold( font() );
	bold.setWeight( QFont::Bold );
	p->setFont( bold );
    } else {
	p->setPen( colorGroup().light() );
	p->drawLine( r.left(), r.bottom(), r.right(), r.bottom() );
	r.setRect( r.left() + 2, r.top() + 2, r.width() - 4, r.height() - 2 );
	p->setFont( font() );
    }
    
    p->drawLine( r.left(), r.bottom(), r.left(), r.top() + 2 );
    p->drawPoint( r.left()+1, r.top() + 1 );
    p->drawLine( r.left()+2, r.top(),
		 r.right() - 2, r.top() );

    p->setPen( colorGroup().dark() );
    p->drawLine( r.right() - 1, r.top() + 2, r.right() - 1, r.bottom() - 1 ); 
    p->setPen( black );
    p->drawPoint( r.right() - 1, r.top() + 1 );
    p->drawLine( r.right(), r.top() + 2, r.right(), r.bottom() - 1 ); 

    QRect br = p->fontMetrics().boundingRect( t->label );
    br.setHeight( p->fontMetrics().height() );
    br.setRect( r.left() + (r.width()-br.width())/2 - 3,
		r.top() + (r.height()-br.height())/2,
		br.width() + 5,
		br.height() + 2 );

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


/*!  "Raises" the tab with ID \a id and emits the selected() signal.

  \sa currentTab() selected() tab()
*/

void QTabBar::setCurrentTab( int id )
{
    setCurrentTab( tab( id ) );
}


/*! "Raises" \a tab and emits the selected() signal unless the tab was
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
