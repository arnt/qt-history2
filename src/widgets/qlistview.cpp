/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qlistview.cpp#18 $
**
** Implementation of something useful
**
** Created : 979899
**
** Copyright (C) 1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qlistview.h"
#include "qtimer.h"
#include "qheader.h"
#include "qpainter.h"
#include "qstack.h"
#include "qlist.h"
#include "qstrlist.h"
#include "qpixmap.h"

#include <stdarg.h>

RCSTAG("$Id: //depot/qt/main/src/widgets/qlistview.cpp#18 $");


struct QListViewPrivate
{
    // classes that are here to avoid polluting the global name space

    // the magical hidden mother of all items
    struct Root: public QListViewItem {
	Root( QListView * parent );

	void setHeight( int );
	void invalidateHeight();
	QListView *listView() const;

	QListView * lv;
    };

    // for the stack used in drawContentsOffset()
    struct Pending {
	Pending( int level, int ypos, QListViewItem * item)
	    : l(level), y(ypos), i(item) {};

	int l; // top pixel in this item, in list view coordinates
	int y; // level of this item in the tree
	QListViewItem * i; // the item itself
    };

    // to remember what's on screen
    struct DrawableItem {
	DrawableItem( Pending * pi ) { y=pi->y; l=pi->l; i=pi->i; };
	int y;
	int l;
	QListViewItem * i;
    };

    // private variables used in QListView
    QHeader * h;
    Root * r;

    QListViewItem * currentSelected;

    QTimer * timer;
    int levelWidth;

    QList<DrawableItem> * drawables;

    bool multi;
};



/*!
  \class QListViewItem qlistview.h
  \brief The QListViewItem class implements a listview item.

  This class is not finished.
 */



/*!  Create a new list view item in the QListView \a parent */

QListViewItem::QListViewItem( QListView * parent )
{
    init();
    parent->insertItem( this );
}


/*!  Create a new list view item which is a child of \a parent */

QListViewItem::QListViewItem( QListViewItem * parent )
{
    init();
    parent->insertItem( this );
}



/*!  Create a new list view item in the QListView \a parent,
  with the contents asdf asdf asdf ### sex vold ### 

*/

QListViewItem::QListViewItem( QListView * parent,
			      const char * firstLabel, ... )
{
    init();
    parent->insertItem( this );

    columnTexts = new QStrList();
    columnTexts->append( firstLabel );
    va_list ap;
    const char * nextLabel;
    va_start( ap, firstLabel );
    while ( (nextLabel = va_arg(ap, const char *)) != 0 )
	columnTexts->append( nextLabel );
    va_end( ap );

    styleChange(); //####### ugle hack hack and away
}


/*!  Create a new list view item which is a child of \a parent,
  with the contents asdf asdf asdf ### sex vold ### 

*/

QListViewItem::QListViewItem( QListViewItem * parent,
			      const char * firstLabel, ... )
{
    init();
    parent->insertItem( this );

    columnTexts = new QStrList();
    columnTexts->append( firstLabel );
    va_list ap;
    const char * nextLabel;
    va_start( ap, firstLabel );
    while ( (nextLabel = va_arg(ap, const char *)) != 0 )
	columnTexts->append( nextLabel );
    va_end( ap );

    styleChange(); //####### ugle hack hack and away
}

/*!  Perform the initializations that's common to the constructors. */

void QListViewItem::init()
{
    ownHeight = 0;
    maybeTotalHeight = -1;
    open = TRUE;

    childCount = 0;
    parentItem = 0;
    siblingItem = childItem = 0;

    columnTexts = 0;
}


/*!  Deletes this item and all children of it, freeing up all
  allocated resources.
*/

QListViewItem::~QListViewItem()
{
    if ( parentItem )
	parentItem->removeItem( this );
    QListViewItem * nextChild = childItem;
    while ( childItem ) {
	nextChild = childItem->siblingItem;
	delete childItem;
	childItem = nextChild;
    }
    
}


/*!  Inserts \a newChild into its list of children.  Called by the
  constructor of \a newChild.
*/

void QListViewItem::insertItem( QListViewItem * newChild )
{
    invalidateHeight();
    // sorting?
    newChild->siblingItem = childItem;
    childItem = newChild;
    childCount++;
    newChild->parentItem = this;
}


/*!  Removes \a tbg from this object's list of children.
*/

void QListViewItem::removeItem( QListViewItem * tbg )
{
    invalidateHeight();

    childCount--;

    QListViewItem ** nextChild = &childItem;
    while( nextChild && *nextChild && tbg != *nextChild )
	nextChild = &((*nextChild)->siblingItem);

    if ( nextChild && tbg == *nextChild )
	*nextChild = (*nextChild)->siblingItem;
    tbg->parentItem = 0;
}


/* !

*/

int QListViewItem::compare( int , const QListViewItem *  ) const
{
    return 0;
}


#if 0
static int col; // ### not thread safe, see below

static int cmp( const void *n1, const void *n2 )
{
    if ( n1 && n2 )
	return ((QListViewItem *)n1)->compare( 1, 0 );
    else
	return 0;
}
#endif


/*     Sort this function and its younger siblingItems according to what
  key( \a column ) returns.

  This function is \e not \e reentrant.

  If you reimplement it, you do \e not need to ask your children to
  sort themselves.

  \sa key()
*/

#if 0
QListViewItem * QListViewItem::sortSiblingItems( int column )
{
    if ( nextSiblingItem == 0 )
	return this;

    int count = 0;

    QListViewItem * siblingItem = this;
    while( siblingItem ) {
	count++;
	siblingItem = siblingItem->nextSiblingItem;
    }
    QListViewItem * siblingItems = new (*QListViewItem)[count];

    siblingItem = this;
    int i;
    for( i=0; i<count; i++ ) {
	siblingItems[i] = siblingItem;
	siblingItem = siblingItem->nextSiblingItem;
    }

    col = column;
    qsort( siblingItems, count, sizeof( QListViewItem * ), cmp );

    for( i=0; i < count-1; i++ )
	siblingItems[i]->nextSiblingItem = siblingItems[i+1];
    siblingItems[count-1]->nextSiblingItem = 0;

    siblingItem = siblingItems[0];
    delete[] siblingItems;

    return siblingItem;
}
#endif


/*!  Sets this item's own height to \a height pixels.  This implictly
  changes totalHeight() too.

  \sa ownHeight() totalHeight() isOpen();
*/

void QListViewItem::setHeight( int height )
{
    if ( ownHeight != height ) {
	ownHeight = height;
	invalidateHeight();
    }
}


/*!  Invalidates the cached total height of this item including
  all open children.

  \sa setHeight() ownHeight() totalHeight()
*/

void QListViewItem::invalidateHeight()
{
    if ( maybeTotalHeight < 0 )
	return;
    maybeTotalHeight = -1;
    if ( parentItem && parentItem->isOpen() )
	parentItem->invalidateHeight();
}


/*!  Sets this item to be open (its children are visible) if \a o is
  TRUE, and to be closed (its children are not visible) if \a o is
  FALSE.

  \sa ownHeight() totalHeight()
*/

void QListViewItem::setOpen( bool o )
{
    if ( o != open ) {
	open = o && children();
	if ( children() )
	    invalidateHeight();
    }
}


/*!  Sets this item to be selected \a s is TRUE, and to not be
  selected if \a o is FALSE.  Doesn't repaint anything in either case.

  \sa ownHeight() totalHeight() */

void QListViewItem::setSelected( bool s )
{
    selected = s ? 1 : 0;
}


/*!  Returns the total height of this object, including any visible
  children.  This height is recomputed lazily and cached for as long
  as possible.

  setOwnHeight() can be used to set the item's own height, setOpen()
  to show or hide its children, and invalidateHeight() to invalidate
  the cached height.
*/

int QListViewItem::totalHeight() const
{
    if ( maybeTotalHeight >= 0 )
	return maybeTotalHeight;
    else if ( !isOpen() || !children() )
	return ownHeight;

    QListViewItem * that = (QListViewItem *)this;

    that->maybeTotalHeight = ownHeight;
    QListViewItem * child = childItem;
    while ( child != 0 ) {
	that->maybeTotalHeight += child->totalHeight();
	child = child->siblingItem;
    }
    return that->maybeTotalHeight;
}


/*!  Returns the text in column \a column, or else 0. */

const char * QListViewItem::text( int column ) const
{
    if ( columnTexts && (int)columnTexts->count() > column )
	return columnTexts->at( column );
    else
	return 0;
}


/*!  Paint the contents of one cell.

  ### undoc
*/

void QListViewItem::paintCell( QPainter * p, const QColorGroup & cg,
			       int column, int width ) const
{
    if ( !p )
	return;

    p->fillRect( 0, 0, width, height(), cg.base() );

    int r = 2;

    QPixmap * icon = 0; // ### temporary! to be replaced with an array

    if ( icon && !column ) {
	p->drawPixmap( 0, (height()-icon->height())/2, *icon );
	r += icon->width();
    }

    const char * t = text( column );
    if ( t ) {
	p->setPen( cg.text() );
	QListView *lv = listView();
	if ( lv )
	    p->setFont( lv->font() );

	// should do the ellipsis thing here
	p->drawText( r, 0, width-2-r, height(), AlignLeft + AlignVCenter, t );
    }
}


/*!  Paints a set of branches from this item to (some of) its children.

  \a p is set up with clipping and translation so that you can draw
  only in the rectangle you need to; \a cg is the color group to use,
  0,top is the top left corner of the update rectangle, w-1,top is the
  top right corner, 0,bottom-1 is the bottom left corner and the
  bottom right corner is left as an excercise for the reader.

  The update rectangle is in an undefined state when this function is
  called; this function must draw on \e all of the pixels.
*/

void QListViewItem::paintBranches( QPainter * p, const QColorGroup & cg,
				   int w, int y, int h, GUIStyle s ) const
{
    p->fillRect( 0, 0, w, h, cg.base() );

    const QListViewItem * child = firstChild();
    int linetop = 0, linebot = 0;

    int dotoffset = y & 1;

    // each branch needs at most two lines, ie. four end points
    QPointArray dotlines( children() * 4 );
    int c = 0;

    // skip the stuff above the exposed rectangle
    while ( child && y + child->height() <= 0 ) {
	y += child->totalHeight();
	child = child->nextSibling();
    }

    int bx = w / 2;

    // paint stuff in the magical area
    while ( child && y < h ) {
	linebot = y + child->height()/2;
	if ( child->children() ) {
	    // needs a box
	    p->setPen( cg.dark() );
	    p->drawRect( bx-4, linebot-4, 9, 9 );
	    // plus or minus
	    p->setPen( cg.foreground() ); // ### windows uses black
	    p->drawLine( bx - 2, linebot, bx + 2, linebot );
	    if ( !child->isOpen() )
		p->drawLine( bx, linebot - 2, bx, linebot + 2 );
	    // dotlinery
	    dotlines[c++] = QPoint( bx, linetop );
	    dotlines[c++] = QPoint( bx, linebot - 5 );
	    dotlines[c++] = QPoint( bx + 6, linebot );
	    dotlines[c++] = QPoint( w, linebot );
	    linebot += 6;
	    linetop = linebot;
	} else {
	    // just dotlinery
	    dotlines[c++] = QPoint( bx + 2, linebot ); // ### +2? +1?
	    dotlines[c++] = QPoint( w, linebot );
	}

	y += child->totalHeight();
	child = child->nextSibling();
    }

    if ( child ) // there's a child, so move linebot to edge of rectangle
	linebot = h;

    if ( linetop < linebot ) {
	dotlines[c++] = QPoint( bx, linetop );
	dotlines[c++] = QPoint( bx, linebot );
    }

    if ( s == MotifStyle ) {
	p->setPen( cg.foreground() );
	p->drawLineSegments( dotlines, 0, c/2 );
    } else {
	// this could be done much faster on X11, but not on Windows.
	// oh well.  do it the hard way.

	// thought: keep around a 64*1 and a 1*64 bitmap such that
	// drawPixmap'ing them is equivalent to drawing a horizontal
	// or vertical line with the appropriate pen.
	
	QPointArray dots( (h+4)/2 + (children()*w+3)/4 );
	// at most one dot for every second y coordinate, plus the
	// spillover at the top.  at most dot for every second x
	// coordinate for half of the width for every child.  both
	// divisions must be rounded up to the nearest integer.
	int i = 0; // index into dots
	int line; // index into dotlines
	int point; // relevant coordinate of current point
	int end; // same coordinate of the end of the current line
	int other; // the other coordinate of the current point/line
	for( line = 0; line < c; line += 2 ) {
	    // assumptions here: lines are horizontal or vertical.
	    // lines always start with the numerically lowest
	    // coordinate.
	    if ( dotlines[line].y() == dotlines[line+1].y() ) {
		end = dotlines[line+1].x();
		point = dotlines[line].x();
		other = dotlines[line].y();
		while( point < end ) {
		    dots[i++] = QPoint( point, other );
		    point += 2;
		}
	    } else {
		end = dotlines[line+1].y();
		point = dotlines[line].y();
		if ( (point & 1) != dotoffset )
		    point++;
		other = dotlines[line].x();
		while( point < end ) {
		    dots[i++] = QPoint( other, point );
		    point += 2;
		}
	    }
	}
	p->setPen( cg.dark() );
	p->drawPoints( dots, 0, i );
    }
}



QListViewPrivate::Root::Root( QListView * parent )
    : QListViewItem( parent )
{
    lv = parent;
    setHeight( 0 );
}


void QListViewPrivate::Root::setHeight( int )
{
    QListViewItem::setHeight( 0 );
}


void QListViewPrivate::Root::invalidateHeight()
{
    QListViewItem::invalidateHeight();
    lv->triggerUpdate();
}


QListView * QListViewPrivate::Root::listView() const
{
    return lv;
}


/*!
  \class QListView qlistview.h
  \brief The QListView class implements a tree/list view.

  Lots of things don't work yet.  The tree view functionality hasn't
  been tested since the last extensive changes, focus stuff doesn't
  work, input is ignored and so on.
*/

/*!  Creates a new empty list view, with \a parent as a parent and \a
  name as object name. */

QListView::QListView( QWidget * parent, const char * name )
    : QScrollView( parent, name )
{
    d = new QListViewPrivate;
    d->timer = new QTimer( this );
    d->levelWidth = 0;
    d->r = 0;
    d->h = new QHeader( this, "list view header" );
    d->currentSelected = 0;
    d->drawables = 0;
    d->multi = 0;

    connect( d->timer, SIGNAL(timeout()),
	     this, SLOT(updateContents()) );
    connect( d->h, SIGNAL(sizeChange( int, int )),
	     this, SLOT(triggerUpdate()) );
    connect( d->h, SIGNAL(sizeChange( int, int )),
	     this, SIGNAL(sizeChanged()) );
    connect( d->h, SIGNAL(moved( int, int )),
	     this, SLOT(triggerUpdate()) );
    connect( horizontalScrollBar(), SIGNAL(sliderMoved(int)),
	     d->h, SLOT(setOffset(int)) );
    connect( horizontalScrollBar(), SIGNAL(valueChanged(int)),
	     d->h, SLOT(setOffset(int)) );

    // will access d->r
    QListViewPrivate::Root * r = new QListViewPrivate::Root( this );
    d->r = r;

    setFocusProxy( viewport() );
}


/*!  Deletes the list view and all items in it, and frees all
  allocated resources.  */

QListView::~QListView()
{
    // nothing
}


/*!  Calls QListViewItem::paintCell() and/or
  QListViewItem::paintBranches() for all list view items that
  require repainting.  See the documentation for those functions for
  details.
*/

void QListView::drawContentsOffset( QPainter * p, int ox, int oy,
				    int cx, int cy, int cw, int ch )
{
    if ( !d->drawables || d->drawables->isEmpty() )
	buildDrawableList();

    QListIterator<QListViewPrivate::DrawableItem> it( *(d->drawables) );

    QRect r;
    int l;
    int fx, x;
    l = 0;
    fx = -1;
    int tx = -1;
    struct QListViewPrivate::DrawableItem * current;

    while ( (current = it.current()) != 0 ) {
	++it;

	int ih = current->i->height();
	int ith = current->i->totalHeight();
	int fc, lc, c;
	int cs;

	// need to paint current?
	if ( ih > 0 && current->y < cy+ch && current->y+ih >= cy ) {
	    if ( fx < 0 ) {
		// find first interesting column, once
		x = 0;
		c = 0;
		cs = d->h->cellSize( 0 );
		while ( x + cs <= cx && c < d->h->count() ) {
		    x += cs;
		    c++;
		    if ( c < d->h->count() )
			cs = d->h->cellSize( c );
		}
		fx = x;
		fc = c;
		while( x < cx + cw && c < d->h->count() ) {
		    x += cs;
		    c++;
		    if ( c < d->h->count() )
			cs = d->h->cellSize( c );
		}
		lc = c;
	    }

	    x = fx;
	    c = fc;

            // draw to last interesting column
            while( c < lc ) {
		int i = d->h->mapToLogical( c );
                cs = d->h->cellSize( c );
                r.setRect( x + ox, current->y + oy, cs, ih );
                if ( c + 1 == lc && x + cs < cx + cw )
                    r.setRight( cx + cw + ox - 1 );
		if ( i==0 && current->l > 0 )
		    r.setLeft( r.left() + (current->l-1) * treeStepSize() );
		
		p->save();
                p->setClipRegion( p->clipRegion().intersect(QRegion(r)) );
                p->translate( r.left(), r.top() );
		current->i->paintCell( p, colorGroup(),
				       d->h->mapToLogical( c ), r.width() );
		p->restore();
		x += cs;
		c++;
	    }
	}

	if ( tx < 0 )
	    tx = d->h->cellPos( d->h->mapToActual( 0 ) );

	// do any children of current need to be painted?
	if ( current->i->isOpen() &&
	     current->y + ith > cy &&
	     current->y + ih < cy + ch && 
	     tx < cx + cw &&
	     tx + current->l * treeStepSize() > cx ) {
	    // compute the clip rectangle the safe way

	    int rtop = current->y + ih;
	    int rbottom = current->y + ith;
	    int rleft = tx + (current->l-1)*treeStepSize();
	    int rright = rleft + treeStepSize();

	    int crtop = QMAX( rtop, cy );
	    int crbottom = QMIN( rbottom, cy+ch );
	    int crleft = QMAX( rleft, cx );
	    int crright = QMIN( rright, cx+cw );

	    r.setRect( crleft+ox, crtop+oy, 
		       crright-crleft, crbottom-crtop );

	    if ( r.isValid() ) {
		p->save();
		p->setClipRect( r );
		p->translate( rleft+ox, crtop+oy );
		current->i->paintBranches( p, colorGroup(), treeStepSize(),
					   rtop - crtop, r.height(), style() );
		p->restore();
	    }
	}
    }

    if ( d->r->totalHeight() < cy + ch ) {
	// really should call some virtual method, or at least use
	// something more configurable than colorGroup().base()
	p->fillRect( cx + ox, d->r->totalHeight() + oy,
		     cw, cy + ch - d->r->totalHeight(),
		     colorGroup().base() );
    }
}


/*! Rebuild the lis of drawable QListViewItems.  This function is
  const so that const functions can call it without requiring
  d->drawables to be mutable */

void QListView::buildDrawableList() const
{
    QStack<QListViewPrivate::Pending> stack;
    stack.push( new QListViewPrivate::Pending( 0, 0, d->r ) );

    // could mess with cy and ch in order to speed up vertical
    // scrolling
    int cy = viewY();
    int ch = viewHeight();

    struct QListViewPrivate::Pending * cur;

    // used to work around lack of support for mutable
    QList<QListViewPrivate::DrawableItem> * dl;

    if ( d->drawables ) {
	dl = ((QListView *)this)->d->drawables;
	dl->clear();
    } else {
	dl = new QList<QListViewPrivate::DrawableItem>;
	dl->setAutoDelete( TRUE );
	((QListView *)this)->d->drawables = dl;
    }

    while ( !stack.isEmpty() ) {
	cur = stack.pop();

	int ih = cur->i->height();
	int ith = cur->i->totalHeight();

	// is this item, or its branch symbol, inside the viewport?
	if ( cur->y + ith >= cy && cur->y < cy + ch )
	    dl->append( new QListViewPrivate::DrawableItem(cur));

	// push younger sibling of cur on the stack?
	if ( cur->y + ith < cy+ch && cur->i->siblingItem )
	    stack.push( new QListViewPrivate::Pending(cur->l,
						      cur->y + ith,
						      cur->i->siblingItem));

	// do any children of cur need to be painted?
	if ( cur->i->isOpen() &&
	     cur->y + ith > cy &&
	     cur->y + ih < cy + ch ) {

	    QListViewItem * c = cur->i->childItem;
	    int y = cur->y + ih;

	    // skip past some of the children quickly... not strictly
	    // necessary but it probably helps
	    while ( c && y + c->totalHeight() <= cy ) {
		y += c->totalHeight();
		c = c->siblingItem;
	    }

	    // push one child on the stack, if there is at least one
	    // needing to be painted
	    if ( c && y < cy+ch )
		stack.push( new QListViewPrivate::Pending( cur->l + 1,
							       y, c ) );
	}

	delete cur;
    }
}




/*!  Returns the number of pixels a child is offset from its parent.
  This number has meaning only for tree views.

  \sa setTreeStepSize()
*/

int QListView::treeStepSize() const
{
    return d->levelWidth;
}


/*!  Set teh the number of pixels a child is offset from its parent,
  in a tree view.

  \sa treeStepSize()
*/

 void QListView::setTreeStepSize( int l )
{
    if ( l != d->levelWidth ) {
	d->levelWidth = l;
	// update
    }
}


/*!  Inserts a top-level QListViewItem into this list view.  You
  should not need to call this; the QListViewItem constructor does it
  for you.
*/

void QListView::insertItem( QListViewItem * i )
{
    if ( d->r ) // not for d->r itself
	d->r->insertItem( i );
}


/*!  Remove all the list view items from the list, and trigger an
  update. \sa triggerUpdate() */

void QListView::clear()
{
    QListViewItem *c = (QListViewItem *)d->r->firstChild();
    QListViewItem *n;
    while( c ) {
	n = (QListViewItem *)c->nextSibling();
	delete c;
	c = n;
    }
}


/*!  Sets the header for column \a column to be labelled \a label and
  be \a size pixels wide.  If \a column is negative (as it is by
  default) setColumn() adds a new column at the right end. */

void QListView::setColumn( const char * label, int size, int column )
{
    if ( column < 0 )
	d->h->setCellSize( d->h->addLabel( label ), size );
    else
	d->h->setLabel( column, label, size );
}


/*!  Reimplemented to set the correct background mode and viewed area
  size. */

void QListView::show()
{
    QWidget * v = viewport();
    if ( v )
	v->setBackgroundMode( NoBackground );

    viewResize( 250, d->r->totalHeight() ); // ### 250
    QScrollView::show();
}


/*!  Updates the sizes of the viewport, header, scrollbars and so on.
  Don't call this directly; call triggerUpdates() instead.
*/

void QListView::updateContents()
{
    int w = 0;
    for( int i=0; i<d->h->count(); i++ )
	w += d->h->cellSize( i );

    int h = d->h->sizeHint().height();
    d->h->setGeometry( frameWidth(), frameWidth(),
		       frameRect().width(), h );
    setMargins( 0, h, 0, 0 );

    viewResize( w, d->r->totalHeight() );  // repaints
    viewport()->repaint();
}


/*!  Trigger a size-and-stuff update during the next iteration of the
  event loop.  Cleverly makes sure that there'll be just one.

*/

void QListView::triggerUpdate()
{
    d->timer->start( 0, TRUE );
}


/*!  Does nothing at present.  Intended to deliver relevant input
   events to the appropriate QListViewItem. */

bool QListView::eventFilter( QObject * o, QEvent * e )
{
    if ( o != viewport() || !e )
	return QScrollView::eventFilter( o, e );

    QMouseEvent * me = (QMouseEvent *)e;
    QFocusEvent * fe = (QFocusEvent *)e;

    switch( e->type() ) {
    case Event_MouseButtonPress:
	mousePressEvent( me );
	break;
    case Event_MouseMove:
	mouseMoveEvent( me );
	break;
    case Event_MouseButtonRelease:
	mouseReleaseEvent( me );
	break;
    case Event_FocusIn:
	focusInEvent( fe );
	break;
    case Event_FocusOut:
	focusOutEvent( fe );
	break;
    default:
	// nothing
	break;
    }
    return QScrollView::eventFilter( o, e );
}

/*!
  Returns the listview containing this item. 
*/

QListView * QListViewItem::listView() const
{
    if ( parentItem )
	return parentItem->listView();
    else 
	return 0;
}


/*! \fn bool QListViewItem::isOpen () const

  Returns TRUE if it wants to.
*/

/*! \fn const QListViewItem* QListViewItem::firstChild () const

  Returns a pointer to the first (top) child of this item. \sa
  nextSibling()
*/

/*! \fn const QListViewItem* QListViewItem::nextSibling () const

  Returns a pointer to the next sibling (below this one) of this
  item. \sa nextSibling()
*/

/*! \fn int QListViewItem::children () const

  Returns the number of children of this item.
*/

/*! \fn int QListViewItem::height () const

  Returns the height of this item in pixels.  This does not include
  the height of any children; totalHeight() returns that.
*/

/*! \fn virtual int QListViewItem::compare (int column, const QListViewItem * with) const

  Defined to return less than 0, 0 or greater than 0 depending on
  whether this item is lexicograpically before, the same as, or after
  \a with when sorted by column \a column.
*/

/*! \fn void QListView::sizeChanged () 

  This signal is emitted when the list view changes width (or height?
  not at present).
*/


/*!
  Recalculates all cached parameters when the visual appearance of the
  list view changes. The default implementation sets the height of the
  item to QFontMetrics::lineSpacing().
*/
//#### should test for columnTexts
void QListViewItem::styleChange()
{
    QListView *lv = listView();
    if ( lv )
	 setHeight( QFontMetrics( lv->font() ).lineSpacing() );
}


/*!
  Sets the font of this widget to \a f
*/

void QListView::setFont( const QFont &f )
{
    d->h->setFont( f );
    QScrollView::setFont( f );
    doStyleChange( d->r );
}


/*!
  Changes the style of \a item and all list view items 
  under it.
*/

void QListView::doStyleChange( QListViewItem *item )
{
    item->styleChange();
    QListViewItem *it = (QListViewItem*)item->firstChild();
    while ( it ) {
	doStyleChange( it );
	it = (QListViewItem*)it->nextSibling();
    }
}


/*!  Processes mouse move events on behalf of the viewed widget;
  eventFilter() calls this function.  Note that the coordinates in \a
  e is in the coordinate system of viewport(). */

void QListView::mousePressEvent( QMouseEvent * e )
{
    if ( !e )
	return;

    QListViewItem * i = itemAt( e->pos() );
    if ( !i )
	return;

    i->setSelected( isMultiSelection() ? !i->isSelected() : TRUE );
    setHighlightedItem( i );

    return;
}


/*!  Processes mouse move events on behalf of the viewed widget;
  eventFilter() calls this function.  Note that the coordinates in \a
  e is in the coordinate system of viewport(). */

void QListView::mouseReleaseEvent( QMouseEvent * e )
{
    if ( !e )
	return;

    QListViewItem * i = itemAt( e->pos() );
    if ( !i )
	return;

    i->setSelected( hightlightedItem() 
		    ? hightlightedItem()->isSelected()
		    : TRUE );
    setHighlightedItem( i );
    return;
}


/*!  Processes mouse move events on behalf of the viewed widget;
  eventFilter() calls this function.  Note that the coordinates in \a
  e is in the coordinate system of viewport(). */

void QListView::mouseMoveEvent( QMouseEvent * e )
{
    if ( !e )
	return;

    QListViewItem * i = itemAt( e->pos() );
    if ( !i )
	return;

    i->setSelected( hightlightedItem() 
		    ? hightlightedItem()->isSelected() 
		    : TRUE );
    setHighlightedItem( i );
    return;
}


/*!

*/

void QListView::focusInEvent( QFocusEvent * )
{
    return;
}


/*!

*/

void QListView::focusOutEvent( QFocusEvent * )
{
    return;
}


/*!

*/

void QListView::keyPressEvent( QKeyEvent * )
{
    return;
}


/*!  Returns a pointer to the QListViewItem at \a screenPos.  Note
  that \a screenPos is in the coordinate system of viewport(), not in
  the listview's own, much larger, coordinate system.

  itemAt() returns 0 if there is no such item.
*/

QListViewItem * QListView::itemAt( QPoint screenPos ) const
{
    if ( !d->drawables || d->drawables->isEmpty() )
	buildDrawableList();

    QListViewPrivate::DrawableItem * c = d->drawables->first();
    int p = -viewY();
    int g = screenPos.y();

    while( c && c->i && p + c->i->height() < g ) {
	p += c->i->height();
	c = d->drawables->next();
    }
    return c ? c->i : 0;
}


/*!  Sets the list view to multi-selection mode if \a enable is TRUE,
  and to single-selection mode if \a enable is FALSE.

  \sa isMultiSelection()
*/

void QListView::setMultiSelection( bool enable )
{
    d->multi = enable ? TRUE : FALSE;
}


/*!  Returns TRUE if this list view is in multi-selection mode and
  FALSE if it is in single-selection mode.

  \sa setMultiSelection()
*/

bool QListView::isMultiSelection() const
{
    return d->multi;
}


/*!  Sets \a item to be selected if \a selected is TRUE, and to be not
  selected if \a selected is FALSE.

  If the list view is in single-selection mode and \a selected is
  TRUE, the present selected item is unselected.  Unlike
  QListViewItem::setSelected(), this function updates the list view as
  necessary.

  \sa isSelected() setMultiSelection() isMultiSelection()
*/

void QListView::setSelected( QListViewItem * item, bool selected )
{
    if ( !item || item->isSelected() == selected )
	return;

    if ( selected && !isMultiSelection() && d->currentSelected ) {
	d->currentSelected->setSelected( FALSE );
	
    }

    if ( item->isSelected() != selected ) {
	item->setSelected( selected );

    }
}


/*!  Returns i->isSelected().

  Provided only because QListView provides setSelected() and I like
  completeness.
*/

bool QListView::isSelected( QListViewItem * i ) const
{
    return i ? i->isSelected() : FALSE;
}


/*!  Sets \a i to be the current highlighted item.  This highlighted
  item is used for keyboard navigation and focus indication; it
  doesn't mean anything else.

  \sa highlightedItem()
*/

void QListView::setHighlightedItem( QListViewItem * i )
{
    if ( d->currentSelected && d->currentSelected != i )
	repaint( itemRect( d->currentSelected ) );
    d->currentSelected = i;
    if ( i )
	repaint( itemRect( i ) );
}


/*!

*/

QListViewItem * QListView::hightlightedItem() const
{
    return d ? d->currentSelected : 0;
}


/*!  Returns the rectangle on the screen \a i occupies in
  viewport()'s coordinates, or an invalid rectangle if \a i is not
  currently visible.

  The rectangle returned does not include any children of the
  rectangle (ie. it uses QListViewItem::height() rather than
  QListViewItem::totalHeight()).  If you want the rectangle including
  children, you can use something like this code:

  \code
    QRect r( listView->itemRect( item ) );
    r.setHeight( (QCOORD)(QMIN( item->totalHeight(),
				listView->viewHeight() - r.y() ) ) )
  \endcode

  Note the way it avoids too-high rectangles.  totalHeight() can be
  much larger than the window system's coordinate system allows.

  itemRect() is comparatively slow.  It's best to call it only for
  items that are probably on-screen.
*/

QRect QListView::itemRect( QListViewItem * i ) const
{
    if ( !d->drawables || d->drawables->isEmpty() )
	buildDrawableList();

    QListViewPrivate::DrawableItem * c = d->drawables->first();

    while( c && c->i && c->i != i )
	c = d->drawables->next();

    if ( c && c->i ) {
	QRect r( 0, c->y - viewY(), d->h->width(), i->height() );
	if ( r.intersects( QRect( 0, 0, viewWidth(), viewHeight() ) ) )
	    return r;
    }

    return QRect( 0, 0, -1, -1 );
}
