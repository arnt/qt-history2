/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qlistview.cpp#16 $
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
#include "qstrlist.h"
#include "qpixmap.h"

#include <stdarg.h>

RCSTAG("$Id: //depot/qt/main/src/widgets/qlistview.cpp#16 $");


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

    // a drawable item, for the stack used in drawContentsOffset()
    struct PendingItem {
	PendingItem( int level, int ypos, const QListViewItem * item)
	    : l(level), y(ypos), i(item) {};

	int l;
	int y;
	const QListViewItem * i;
    };

    // private variables used in QListView
    QHeader * h;
    Root * r;

    QTimer * timer;
    int levelWidth;
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


/*!  Create a new list view item which is a child of \a parent,
  with the contents asdf asdf asdf ### sex vold ### */

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
}


/*!  Create a new list view item which is a child of \a parent,
  with the contents asdf asdf asdf ### sex vold ### */

QListViewItem::QListViewItem( QListViewItem * parent,
			      const QPixmap pm,
			      const char * firstLabel, ... )
{
    init();
    parent->insertItem( this );
    icon = new QPixmap( pm );

    columnTexts = new QStrList();
    columnTexts->append( firstLabel );
    va_list ap;
    const char * nextLabel;
    va_start( ap, firstLabel );
    while ( (nextLabel = va_arg(ap, const char *)) != 0 )
	columnTexts->append( nextLabel );
    va_end( ap );
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
    icon = 0;
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
    return columnTexts ? columnTexts->at( column ) : 0;
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

    if ( icon && !column ) {
	p->drawPixmap( 0, (height()-icon->height())/2, *icon );
	r += icon->width();
    }

    const char * t = text( column );
    if ( t ) {
	p->setPen( cg.text() );
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

void QListViewItem::paintTreeBranches( QPainter * p, const QColorGroup & cg,
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
  QListViewItem::paintTreeBranches() for all list view items that
  require repainting.  See the documentation for those functions for
  details.
*/

void QListView::drawContentsOffset( QPainter * p, int ox, int oy,
				    int cx, int cy, int cw, int ch )
{
    QStack<QListViewPrivate::PendingItem> stack;

    stack.push( new QListViewPrivate::PendingItem( 0, 0, d->r ) );

    QRect r;
    int l;
    int fx, x;
    l = 0;
    fx = -1;
    int tx = -1;
    struct QListViewPrivate::PendingItem * current;

    while ( !stack.isEmpty() ) {
	current = stack.pop();

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

	// push younger sibling of current on the stack?
	if ( current->y + ith < cy+ch && current->i->nextSibling() )
	    stack.push( new QListViewPrivate::PendingItem( current->l,
							   current->y + ith,
							   current->i->nextSibling() ) );

	// do any children of current need to be painted?
	if ( current->i->isOpen() &&
	     current->y + ith > cy &&
	     current->y + ih < cy + ch ) {
	    // perhaps even a branch?
	    if ( tx < 0 )
		tx = d->h->cellPos( d->h->mapToActual( 0 ) );
		
	    if ( tx < cx + cw &&
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
		    current->i->paintTreeBranches( p, colorGroup(),
						      treeStepSize(), 
						      rtop - crtop,
						      r.height(), style() );
		    p->restore();
		}
	    }

	    const QListViewItem * c = current->i->firstChild();
	    int y = current->y + ih;

	    // skip past some of the children quickly... not strictly
	    // necessary but it probably helps
	    while ( c && y + c->totalHeight() <= cy ) {
		y += c->totalHeight();
		c = c->nextSibling();
	    }

	    // push one child on the stack, if there is at least one
	    // needing to be painted
	    if ( c && y < cy+ch )
		stack.push( new QListViewPrivate::PendingItem( current->l + 1,
							       y, c ) );
	}

	delete current;
    }

    if ( d->r->totalHeight() < cy + ch ) {
	// really should call some virtual method, or at least use
	// something more configurable than colorGroup().base()
	p->fillRect( cx + ox, d->r->totalHeight() + oy,
		     cw, cy + ch - d->r->totalHeight(),
		     colorGroup().base() );
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
    if ( o == viewport() && e ) {
	switch( e->type() ) {
	case Event_MouseButtonPress:
	case Event_MouseMove:
	case Event_MouseButtonRelease:
	    
	case Event_FocusIn:
	    // fall through
	case Event_FocusOut:
	    
	    break;
	default:
	    // nothing
	    break;
	}
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
