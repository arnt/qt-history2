/****************************************************************************
** $Id: $
**
** Implementation of QTab and QTabBar classes
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

#include "qtabbar.h"
#ifndef QT_NO_TABBAR
#include "qaccel.h"
#include "qbitmap.h"
#include "qtoolbutton.h"
#include "qapplication.h"

#include <ctype.h>


// NOT REVISED
/*!
  \class QTab qtabbar.h
  \brief The QTab class provides the structures in a QTabBar.


  For custom QTabBar tab headings.

  \sa QTabBar
*/


/*! \fn QTab::QTab()
  Constructs an empty tab.  All fields are set to empty.
*/


/*! \fn QTab::QTab( const QString& text )
  Constructs a tab with a \a text.
*/


/*! \fn QTab::QTab( const QIconSet& icon, const QString& text )
  Constructs a tab with an \a icon and a \a text.
*/


/*! Destroys the tab and frees up all allocated resources */

QTab::~QTab()
{
    delete iconset;
}


/*!
  \class QTabBar qtabbar.h

  \brief The QTabBar class provides a tab bar for use in tabbed
  dialogs, for example.

  \ingroup advanced

  The class is quite simple: it draws the tabs in one of four shapes
  and emits a signal when one is selected.  It can be subclassed to
  tailor the look and feel.

  QTabBar itself supports four possible shapes, described in the
  QTabBar::Shape documentation.

  The choice of tab shape is still a matter of taste, to a large
  degree.  Tab dialogs (preferences and the like) invariably use \c
  RoundedAbove; nobody uses \c TriangularAbove.  Tab controls in
  windows other than dialogs almost always use either \c RoundedBelow or
  \c TriangularBelow.  Many spreadsheets and other tab controls in which
  all the pages are essentially similar use \c TriangularBelow,
  whereas \c RoundedBelow is used mostly when the pages are different
  (e.g., a multi-page tool palette).  There is no strong tradition yet,
  however, so use your taste and create the tradition!

  The most important part of QTabBar's API is the signal selected().
  It is emitted whenever the selected page changes (even at startup,
  when the selected page changes from 'none').  There are also a slot,
  setCurrentTab(), which can be used to select a page
  programmatically.

  QTabBar creates automatic accelerator keys in the manner of QButton;
  e.g., if a tab's label is "\&Graphics", Alt-G becomes an accelerator
  key for switching to that tab.

  The following virtual functions may need to be reimplemented: <ul>
  <li> paint() paints a single tab.  paintEvent() calls paint() for
  each tab so that any overlap will look right.  <li>
  addTab() creates a new tab and adds it to the bar. <li> selectTab()
  decides which, if any, tab the user selects with the mouse. </ul>

  <img src=qtabbar-m.png> <img src=qtabbar-w.png>
*/

/*! \enum QTabBar::Shape
  This enum type lists the built-in shapes supported by QTabBar:

  \value RoundedAbove  the normal rounded look above the pages

  \value RoundedBelow  the normal rounded look below the pages

  \value TriangularAbove  triangular tabs above the pages (very
  unusual; included for completeness)

  \value TriangularBelow  triangular tabs similar to those used in
  the spreadsheet Excel, for example
*/

struct QTabPrivate {
    int id;
    int focus;
    QAccel * a;
    QTabBar::Shape s;
    QToolButton* rightB;
    QToolButton* leftB;
    bool  scrolls;
};


/*!
  \fn void QTabBar::selected( int id )

  QTabBar emits this signal whenever any tab is selected, whether by
  the program or the user.  The argument \a id is the id of the tab
  as returned by addTab().

  show() is guaranteed to emit this signal; you can display
  your page in a slot connected to this signal.
*/


/*!
  Constructs a new, empty tab bar with parent \a parent called \a name.
*/

QTabBar::QTabBar( QWidget * parent, const char *name )
    : QWidget( parent, name, WRepaintNoErase | WResizeNoErase  )
{
    d = new QTabPrivate;
    d->id = 0;
    d->focus = 0;
    d->a = new QAccel( this, "tab accelerators" );
    d->s = RoundedAbove;
    d->scrolls = FALSE;
    d->leftB = new QToolButton( LeftArrow, this );
    connect( d->leftB, SIGNAL( clicked() ), this, SLOT( scrollTabs() ) );
    d->leftB->hide();
    d->rightB = new QToolButton( RightArrow, this );
    connect( d->rightB, SIGNAL( clicked() ), this, SLOT( scrollTabs() ) );
    d->rightB->hide();
    l = new QPtrList<QTab>;
    lstatic = new QPtrList<QTab>;
    lstatic->setAutoDelete( TRUE );
    setFocusPolicy( TabFocus );
    setSizePolicy( QSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed ) );

    connect( d->a, SIGNAL(activated(int)), this, SLOT(setCurrentTab(int)) );
}


/*!
  Destroys the tab control, freeing memory used.
*/

QTabBar::~QTabBar()
{
    delete d;
    d = 0;
    delete l;
    l = 0;
    delete lstatic;
    lstatic = 0;
}


/*!
  Adds \a newTab to the tab control.

  Allocates a new id, sets \a newTab's id, locates it just to the right of the
  existing tabs, inserts an accelerator if the tab's label contains the
  string "&p" for some value of p, adds it to the bar and returns the
  newly allocated id.
*/

int QTabBar::addTab( QTab * newTab )
{
    return insertTab( newTab );
}


/*!
  Inserts \a newTab to the tab control.

  If \a index is not specified, the tab is simply added. Otherwise
  it's inserted at the specified position.

  Allocates a new id, sets \a newTab's id, locates it respectively,
  inserts an accelerator if the tab's label contains the string "&p"
  for some value of p, adds it to the bar and returns the newly
  allocated id.
*/

int QTabBar::insertTab( QTab * newTab, int index )
{
    newTab->id = d->id++;
    l->insert( 0, newTab );
    if ( index < 0 || index > int(lstatic->count()) )
	lstatic->append( newTab );
    else
	lstatic->insert( index, newTab );

    layoutTabs();
    updateArrowButtons();
    makeVisible( tab( currentTab() ) );

    int p = QAccel::shortcutKey( newTab->label );
    if ( p )
	d->a->insertItem( p, newTab->id );

    return newTab->id;
}


/*!
  Removes tab \a t from the tab control.
*/
void QTabBar::removeTab( QTab * t )
{
    //#### accelerator labels??
    l->remove( t );
    lstatic->remove( t );
    layoutTabs();
    updateArrowButtons();
    makeVisible( tab( currentTab() ) );
    update();
}


/*!
  Enables tab \a id if \a enabled is TRUE or disables it if \a enabled is
  FALSE.  If \a id is currently selected, setTabEnabled() makes
  another tab selected.

  setTabEnabled() updates the display if this causes a
  change in \a id's status.

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
				m = distance;
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



/*!\reimp
*/
QSize QTabBar::sizeHint() const
{
    QTab * t = l->first();
    if ( t ) {
	QRect r( t->r );
	while ( (t = l->next()) != 0 )
	    r = r.unite( t->r );
	return r.size().expandedTo( QApplication::globalStrut() );
    } else {
	return QSize( 0, 0 ).expandedTo( QApplication::globalStrut() );
    }
}

/*! \reimp */

QSize QTabBar::minimumSizeHint() const
{
    return QSize( d->rightB->sizeHint().width() * 2 + 75, sizeHint().height() );
}

/*!  Paints the single tab \a t using \a p.  If and only if \a selected
  is TRUE, \a t is drawn currently selected.

  This virtual function may be reimplemented to change the look of
  QTabBar.  If you decide to reimplement it, you may also need to
  reimplement sizeHint().
*/

void QTabBar::paint( QPainter * p, QTab * t, bool selected ) const
{
    QStyle::CFlags flags = QStyle::CStyle_Default;

    void *data[1];
    data[0] = t;

    if ( selected )
	flags |= QStyle::CStyle_Selected;
    style().drawControl( QStyle::CE_TabBarTab, p, this, t->rect(),
			 colorGroup(), flags, data );

    QRect r( t->r );
    p->setFont( font() );

    int iw = 0;
    int ih = 0;
    if ( t->iconset != 0 ) {
	iw = t->iconset->pixmap( QIconSet::Small, QIconSet::Normal ).width() + 2;
	ih = t->iconset->pixmap( QIconSet::Small, QIconSet::Normal ).height();
    }
    int w = iw + p->fontMetrics().width( t->label ) + 4;
    int h = QMAX(p->fontMetrics().height() + 4, ih );
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
    QRect r = br;
    if ( t->iconset) {
	// the tab has an iconset, draw it in the right mode
	QIconSet::Mode mode = (t->enabled && isEnabled())
	    ? QIconSet::Normal : QIconSet::Disabled;
	if ( mode == QIconSet::Normal && has_focus )
	    mode = QIconSet::Active;
	QPixmap pixmap = t->iconset->pixmap( QIconSet::Small, mode );
	int pixw = pixmap.width();
	int pixh = pixmap.height();
	r.setLeft( r.left() + pixw + 2 );
	p->drawPixmap( br.left()+2, br.center().y()-pixh/2, pixmap );
    }

    void * data[2];
    data[0] = (void *) t;
    data[1] = (void *) &has_focus;
    style().drawControl( QStyle::CE_TabBarLabel, p, this, br, colorGroup(),
			 QStyle::CStyle_Default, data );
}


/*!
  Repaints the tab row.  All the painting is done by paint();
  paintEvent() only decides which tabs need painting and in what
  order. The event is passed in \a e.

  \sa paint()
*/

void QTabBar::paintEvent( QPaintEvent * e )
{
    QPainter p( this );

    if ( backgroundMode() == X11ParentRelative ) {
	erase();
    } else {
	p.setBrushOrigin( rect().bottomLeft() );
	p.fillRect( 0, 0, width(), height(),
		    QBrush( colorGroup().brush( QColorGroup::Background ) ));
    }

    QTab * t;
    t = l->first();
    do {
	QTab * n = l->next();
	if ( t && t->r.intersects( e->rect() ) )
	    paint( &p, t, n == 0 );
	t = n;
    } while ( t != 0 );

    if ( d->scrolls && lstatic->first()->r.left() < 0 ) {
	QPointArray a;
	int h = height();
	if ( d->s == RoundedAbove ) {
	    p.fillRect( 0, 3, 4, h-5,
			QBrush( colorGroup().brush( QColorGroup::Background ) ));
	    a.setPoints( 5,  0,2,  3,h/4, 0,h/2, 3,3*h/4, 0,h );
	} else if ( d->s == RoundedBelow ) {
	    p.fillRect( 0, 2, 4, h-5,
			QBrush( colorGroup().brush( QColorGroup::Background ) ));
	    a.setPoints( 5,  0,0,  3,h/4, 0,h/2, 3,3*h/4, 0,h-3 );
	}

	if ( !a.isEmpty() ) {
	    p.setPen( colorGroup().light() );
	    p.drawPolyline( a );
	    a.translate( 1, 0 );
	    p.setPen( colorGroup().midlight() );
	    p.drawPolyline( a );
	}
    }
}


/*!
  This virtual function is called by the mouse event handlers to
  determine which tab is pressed.  The default implementation returns
  a pointer to the tab whose bounding rectangle contains \a p, if
  exactly one tab's bounding rectangle contains \a p.  Otherwise it returns 0.

  \sa mousePressEvent() mouseReleaseEvent()
*/

QTab * QTabBar::selectTab( const QPoint & p ) const
{
    QTab * selected = 0;
    bool moreThanOne = FALSE;

    QPtrListIterator<QTab> i( *l );
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


/*!\reimp
*/
void QTabBar::mousePressEvent( QMouseEvent * e )
{
    if ( e->button() != LeftButton ) {
	e->ignore();
	return;
    }
    QTab * t = selectTab( e->pos() );
    if ( t != 0 && t == selectTab( e->pos() ) && t->enabled ) {
	setCurrentTab( t );
    }
}


/*!\reimp
*/

void QTabBar::mouseMoveEvent ( QMouseEvent *e )
{
    if ( e->button() != LeftButton )
	e->ignore();
}

/*!\reimp
*/

void QTabBar::mouseReleaseEvent( QMouseEvent *e )
{
    if ( e->button() != LeftButton )
	e->ignore();
}


/*!  \reimp
*/
void QTabBar::show()
{
    //  ensures that one tab is selected.
    QTab * t = l->last();
    QWidget::show();

    if ( t )
	emit selected( t->id );
}

/*! \property QTabBar::currentTab
    \brief the id of the currently visible tab in the tab bar

  If no tab page is currently visible, -1 will be the current value
  for this property.
  Even if the property value is not -1, you cannot assume either that
  the user can see the relevant page, or that the tab is enabled.
  When you need to display something the value of this property
  represents the best page to display.

  When this property is set to \e id, it will raise the tab with the
  id \e id and emit the selected() signal.

  \sa selected() isTabEnabled()
*/

int QTabBar::currentTab() const
{
    const QTab * t = l->getLast();

    return t ? t->id : -1;
}

void QTabBar::setCurrentTab( int id )
{
    setCurrentTab( tab( id ) );
}


/*! 
    \overload
    Raises \a tab and emits the selected() signal unless the tab was
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

	setMicroFocusHint( tab->rect().x(), tab->rect().y(), tab->rect().width(), tab->rect().height(), FALSE );

	if ( tab->r.intersects( r ) ) {
	    repaint( r.unite( tab->r ) );
	} else {
	    repaint( r );
	    repaint( tab->r );
	}
	makeVisible( tab );
	emit selected( tab->id );
    }
}

/*! \property QTabBar::keyboardFocusTab
    \brief the id of the tab that currently has the keyboard focus

  This property contains the id of the tab that currently has the
  keyboard focus. If this tab bar does not have keyboard focus, the
  value of this property will be -1.

*/

int QTabBar::keyboardFocusTab() const
{
    return hasFocus() ? d->focus : -1;
}


/*!\reimp
*/
void QTabBar::keyPressEvent( QKeyEvent * e )
{
    //   The right and left arrow keys move a selector, the spacebar
    //   makes the tab with the selector active.  All other keys are
    //   ignored.

    int old = d->focus;

    bool reverse = QApplication::reverseLayout();
    if ( ( !reverse && e->key() == Key_Left ) || ( reverse && e->key() == Key_Right ) ) {
	// left - skip past any disabled ones
	if ( d->focus > 0 ) {
	    QTab * t = lstatic->last();
	    while ( t && t->id != d->focus )
		t = lstatic->prev();
	    do {
		t = lstatic->prev();
	    } while ( t && !t->enabled);
	    if (t)
		d->focus = t->id;
	}
	if ( d->focus < 0 )
	    d->focus = old;
    } else if ( ( !reverse && e->key() == Key_Right ) || ( reverse && e->key() == Key_Left ) ) {
	QTab * t = lstatic->first();
	while ( t && t->id != d->focus )
	    t = lstatic->next();
	do {
	    t = lstatic->next();
	} while ( t && !t->enabled);

	if (t)
	    d->focus = t->id;
	if ( d->focus >= d->id )
	    d->focus = old;
    } else {
	// other keys - ignore
	e->ignore();
	return;
    }

    // if the focus moved, repaint and signal
    if ( old != d->focus ) {
	setCurrentTab( d->focus );
    }
}


/*!  Returns a pointer to the tab with id \a id or 0 if there is no
  such tab.

  \sa count()
*/

QTab * QTabBar::tab( int id ) const
{
    QTab * t;
    for( t = l->first(); t; t = l->next() )
	if ( t && t->id == id )
	    return t;
    return 0;
}


/*! Returns a pointer to the tab at the position \a index.

  \sa indexOf
*/

QTab * QTabBar::tabAt( int index ) const
{
    QTab * t;
    t = lstatic->at( index );
    return t;
}


/*!
  Returns the position index of the tab with id \a id.

  \sa indexOf, tabAt
 */
int QTabBar::indexOf( int id ) const
{
    QTab * t;
    int idx = 0;
    for( t = lstatic->first(); t; t = lstatic->next() ) {
	if ( t && t->id == id )
	    return idx;
	idx++;
    }
    return -1;
}


/*! \property QTabBar::count
    \brief the number of tabs in the tab bar

  \sa tab()
*/
int QTabBar::count() const
{
    return l->count();
}


/*!
  The list of QTab objects added.
*/
QPtrList<QTab> * QTabBar::tabList()
{
    return l;
}


/*! \property QTabBar::shape
    \brief the shape of this tab bar

    The value of this property can be one of the following:
    \c RoundedAbove (default), \c RoundedBelow, \c TriangularAbove or \c
    TriangularBelow.
*/
QTabBar::Shape QTabBar::shape() const
{
    return d ? d->s : RoundedAbove;
}

void QTabBar::setShape( Shape s )
{
    if ( !d || d->s == s )
	return;
    //######### must recalculate heights
    d->s = s;
    update();
}

/*!
  Lays out all existing tabs (i.e., sets their \c r attribute) according
  to their label and their iconset.
 */
void QTabBar::layoutTabs()
{
    if ( lstatic->isEmpty() )
	return;

    int hframe, vframe, overlap;
    hframe  = style().pixelMetric( QStyle::PM_TabBarHorizontalFrame, this );
    vframe  = style().pixelMetric( QStyle::PM_TabBarVerticalFrame, this );
    overlap = style().pixelMetric( QStyle::PM_TabBarOverlap, this );

    QFontMetrics fm = fontMetrics();
    int x = 0;
    QRect r;
    QTab *t;
    bool reverse = QApplication::reverseLayout();
    if ( reverse )
	t = lstatic->last();
    else
	t = lstatic->first();
    while ( t ) {
	int lw = fm.width( t->label );
	int iw = 0;
	int ih = 0;
	if ( t->iconset != 0 ) {
	    iw = t->iconset->pixmap( QIconSet::Small, QIconSet::Normal ).width() + 2;
	    ih = t->iconset->pixmap( QIconSet::Small, QIconSet::Normal ).height();
	}
	int h = QMAX( fm.height(), ih );
	h = QMAX( h, QApplication::globalStrut().height() );

	h += vframe;
	t->r.setRect( x, 0, QMAX( lw + hframe + iw,
				  QApplication::globalStrut().width() ), h );
	x += t->r.width() - overlap;
	r = r.unite( t->r );
	if ( reverse )
	    t = lstatic->prev();
	else
	    t = lstatic->next();
    }
    for ( t = lstatic->first(); t; t = lstatic->next() )
	t->r.setHeight( r.height() );
}

/*!
  \reimp
*/

void QTabBar::styleChange( QStyle& old )
{
	layoutTabs();
	QWidget::styleChange( old );
}

/*!
  \reimp
*/
void QTabBar::focusInEvent( QFocusEvent * )
{
    QTab *t = l->first();
    for ( ; t; t = l->next() ) {
	if ( t->id == d->focus ) {
	    QPainter p;
	    p.begin( this );
	    QRect r = t->r;
	    p.setFont( font() );

	    int iw = 0;
	    int ih = 0;
	    if ( t->iconset != 0 ) {
		iw = t->iconset->pixmap( QIconSet::Small, QIconSet::Normal ).width() + 2;
		ih = t->iconset->pixmap( QIconSet::Small, QIconSet::Normal ).height();
	    }
	    int w = iw + p.fontMetrics().width( t->label ) + 4;
	    int h = QMAX(p.fontMetrics().height() + 4, ih );
	    paintLabel( &p, QRect( r.left() + ( r.width() -w ) /2 - 3,
				   r.top() + ( r.height()-h ) / 2,
				   w, h ), t, TRUE );
	    p.end();
	}
    }
}

/*!
  \reimp
*/
void QTabBar::focusOutEvent( QFocusEvent * )
{
    QTab *t = l->first();
    for ( ; t; t = l->next() ) {
	if ( t->id == d->focus ) {
	    QPainter p;
	    p.begin( this );
	    p.setBrushOrigin( rect().bottomLeft() );
	    QRect r = t->r;
	    p.setFont( font() );

	    int iw = 0;
	    int ih = 0;
	    if ( t->iconset != 0 ) {
		iw = t->iconset->pixmap( QIconSet::Small, QIconSet::Normal ).width() + 2;
		ih = t->iconset->pixmap( QIconSet::Small, QIconSet::Normal ).height();
	    }
	    int w = iw + p.fontMetrics().width( t->label ) + 4;
	    int h = QMAX(p.fontMetrics().height() + 4, ih );
	    p.fillRect( QRect( r.left() + ( r.width() -w ) / 2 - 4,
				   r.top() + ( r.height()-h ) / 2 - 1,
			       w + 2, h + 2 ), colorGroup().brush(QColorGroup::Background ) );

	    QStyle::CFlags flags = QStyle::CStyle_Default;
	    flags |= QStyle::CStyle_Selected;
	    void *data[1];
	    data[0] = t;
	    style().drawControl( QStyle::CE_TabBarTab, &p, this, t->rect(),
				 colorGroup(), flags, data );

	    paintLabel( &p, QRect( r.left() + ( r.width() -w ) /2 - 3,
				   r.top() + ( r.height()-h ) / 2,
				   w, h ), t, FALSE );
	    p.end();
	}
    }
}

/*!
  \reimp
*/
void QTabBar::resizeEvent( QResizeEvent * )
{
    const int arrowWidth = 16;
    d->rightB->setGeometry( width() - arrowWidth, 0, arrowWidth, height() );
    d->leftB->setGeometry( width() - 2*arrowWidth, 0, arrowWidth, height() );
    layoutTabs();
    updateArrowButtons();
    makeVisible( tab( currentTab() ));
}

void QTabBar::scrollTabs()
{
    QTab* left = 0;
    QTab* right = 0;
    for ( QTab* t = lstatic->first(); t; t = lstatic->next() ) {
	if ( t->r.left() < 0 && t->r.right() > 0 )
	    left = t;
	if ( t->r.left() < d->leftB->x()+2 )
	    right = t;
    }

    if ( sender() == d->leftB )
	makeVisible( left );
    else  if ( sender() == d->rightB )
	makeVisible( right );
}

void QTabBar::makeVisible( QTab* tab  )
{
    bool tooFarLeft = ( tab && tab->r.left() < 0 );
    bool tooFarRight = ( tab && tab->r.right() >= d->leftB->x() );

    if ( !d->scrolls || ( !tooFarLeft && ! tooFarRight ) )
	return;

    layoutTabs();

    int offset = 0;

    if ( tooFarLeft )
	offset = tab == lstatic->first() ? 0 : tab->r.left() - 8;
    else if ( tooFarRight ) {
	offset = tab->r.right() - d->leftB->x() + 1;
    }

    for ( QTab* t = lstatic->first(); t; t = lstatic->next() )
	t->r.moveBy( -offset, 0 );

    d->leftB->setEnabled( offset != 0 );
    d->rightB->setEnabled( lstatic->last()->r.right() >= d->leftB->x() );


    update();
}

void QTabBar::updateArrowButtons()
{
    bool b = lstatic->last() &&	( lstatic->last()->r.right() > width() );
    d->scrolls = b;
    if ( d->scrolls ) {
	d->leftB->setEnabled( FALSE );
	d->rightB->setEnabled( TRUE );
	d->leftB->show();
	d->rightB->show();
    } else {
	d->leftB->hide();
	d->rightB->hide();
    }
}
#endif
