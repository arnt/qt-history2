/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qlistview.cpp#2 $
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
#include "qscrbar.h"
#include <stdlib.h>

RCSTAG("$Id: //depot/qt/main/src/widgets/qlistview.cpp#2 $");

/*!  Create a new list view item in the QListView \a parent
  and is named \a name.
*/

QListViewItem::QListViewItem( QListView * parent, const char * name )
    : QObject( parent, name )
{
    ownHeight = 0;
    maybeTotalHeight = -1;
    open = TRUE;

    childCount = 0;
    parentItem = 0;
    siblingItem = childItem = 0;

    parent->insertItem( this );
}


/*!  Create a new list view item which is a child of \a parent
  and is named \a name.
*/

QListViewItem::QListViewItem( QListViewItem * parent, const char * name )
    : QObject( parent, name )
{
    ownHeight = 0;
    maybeTotalHeight = -1;
    open = TRUE;

    childCount = 0;
    parentItem = parent;
    siblingItem = childItem = 0;

    parent->insertItem( this );
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


/*!

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

#ifdef 0
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


/*!  Paint the contents of one cell.

  The default draws a rectangle just inside the cell rectangle using
  the supplied pen and brush.  This is useless for production work but
  useful for debugging.
*/

void QListViewItem::paintCell( QPainter * p, const QColorGroup & cg,
			       int column, int width ) const
{
    NOT_USED( column );

    if ( !p )
	return;

    p->fillRect( 0, 0, width, height(), cg.background() );
    p->setPen( cg.foreground() );
    p->drawRect( 1, 1, width-2, height()-2 );
}


/*!  
*/

void QListViewItem::paintTreeBranches( QPainter * p, const QColorGroup & cg,
				       int w, int top, int bottom ) const
{
    const QListViewItem * child = firstChild();
    int y = 0;
    int linetop = (top > 64) ? ((top-30) & ~31) : 2;
    // each branch needs at most two lines, ie. four end points
    QPointArray dotlines( children() * 4 );
    int c = 0;

    // skip the stuff above the exposed rectangle
    while ( child && y + child->totalHeight() <= top ) {
	y += child->totalHeight();
	child = child->nextSibling();
    }

    int bx = w / 2;
    int by;

    // paint stuff in the magical area
    while ( c && y < bottom ) {
	by = y + child->height()/2;
	if ( child->children() ) {
	    // needs a box
	    p->setPen( cg.dark() );
	    p->drawRect( bx-4, by-4, 9, 9 );
	    // plus or minus
	    p->setPen( cg.foreground() ); // ### windows appears to use black
	    p->drawLine( bx - 2, by, bx + 2, by );
	    if ( !child->isOpen() )
		p->drawLine( bx, by - 2, bx, by + 2 );
	    // dotlinery
	    dotlines[c++] = QPoint( bx, linetop );
	    dotlines[c++] = QPoint( bx, by - 5 );
	    dotlines[c++] = QPoint( bx + 6, by );
	    dotlines[c++] = QPoint( w, by );
	    by += 6; // oooh! nasty overloading here! see linetop != by below
	    linetop = by;
	} else {
	    // just dotlinery
	    dotlines[c++] = QPoint( bx + ((by - linetop) & 1), by );
	    dotlines[c++] = QPoint( w, by );
	}

	if ( linetop != by ) {
	    dotlines[c++] = QPoint( bx, linetop );
	    dotlines[c++] = QPoint( bx, by );
	}

	y += child->totalHeight();
	child = child->nextSibling();
    }

    p->setPen( QPen( cg.dark(), 0, DotLine ) );
    p->drawLineSegments( dotlines, 0, c/2 );
}



struct QListViewRoot: public QListViewItem {
    QListViewRoot( QListView * parent, const char * name );

    void setHeight( int );
    void invalidateHeight();

    QTimer * timer;
    int levelWidth;

    QHeader * h;
};



QListViewRoot::QListViewRoot( QListView * parent, const char * name )
    : QListViewItem( parent, name )
{
    timer = new QTimer( this );
    setHeight( 0 );
    levelWidth = 0;
    h = 0;
}


void QListViewRoot::setHeight( int )
{
    QListViewItem::setHeight( 0 );
}


void QListViewRoot::invalidateHeight()
{
    QListViewItem::invalidateHeight();
    timer->start( 0, TRUE );
}

/*!

*/

QListView::QListView( QWidget * parent, const char * name )
    : QScrollView( parent, name )
{
    root = 0;
    // will access root
    QListViewRoot * r = new QListViewRoot( this, "root" );
    root = r;
    root->h = new QHeader( this, "list view header" );
    connect( root->timer, SIGNAL(timeout()),
	     this, SLOT(updateContents()) );
    connect( root->h, SIGNAL(sizeChange( int, int )),
	     this, SLOT(triggerUpdate()) );
    connect( root->h, SIGNAL(sizeChange( int, int )),
	     this, SIGNAL(sizeChanged()) );
    connect( root->h, SIGNAL(moved( int, int )),
	     this, SLOT(triggerUpdate()) );

    setFocusProxy( viewport() );
}


/*!

*/

QListView::~QListView()
{
    // nothing
}


/*!

*/

void QListView::drawContentsOffset( QPainter * p, int ox, int oy,
				    int cx, int cy, int cw, int ch )
{
    struct PendingItem {
	PendingItem() {};

	int l;
	int y;
	const QListViewItem * item;
	struct PendingItem * next;
    };

    struct PendingItem * head = new PendingItem();
    head->l = 0;
    head->y = 0;
    head->item = root;
    head->next = 0;

    QRect r;
    int l;
    int fx, x;
    l = 0;
    fx = -1;
    int tx = -1;

    while( head ) {
	int ih = head->item->height();
	int ith = head->item->totalHeight();
	int fc, lc, c;
	int cs;

	// need to paint this item?
	if ( ih > 0 && head->y < cy+cw && head->y+ih >= cy ) {

	    if ( fx < 0 ) {
		// find first interesting column, once
		x = 0;
		c = 0;
		cs = root->h->cellSize( 0 );
		while ( x + cs <= cx && c < root->h->count() ) {
		    x += cs;
		    c++;
		    if ( c < root->h->count() )
			cs = root->h->cellSize( c );
		}
		fx = x;
		fc = c;
		while( x < cx + cw && c < root->h->count() ) {
		    if ( root->h->mapToLogical( c ) == 0 )
			tx = x;
		    x += cs;
		    c++;
		    if ( c < root->h->count() )
			cs = root->h->cellSize( c );
		}
		lc = c;
	    }

	    x = fx;
	    c = fc;

	    // draw to last interesting column
	    while( c < lc ) {
		cs = root->h->cellSize( c );
		r.setRect( x + ox, head->y + oy, cs, ih );
		if ( c + 1 == lc && x + cs < cx + cw )
		    r.setRight( cx + cw + ox - 1 );
		p->save();
		p->setClipRect( r ); // ### could intersect a bit
		p->translate( r.left(), r.top() );
		head->item->paintCell( p, colorGroup(),
				       root->h->mapToLogical( c ), r.width() );
		p->restore();
		x += cs;
		c++;
	    }
	}

	struct PendingItem * next;

	// do any children of the head need to be painted?
	if ( head->item->isOpen() &&
	     head->y + ith > cy &&
	     head->y + ih < cy + ch ) {
	    // perhaps even a branch?
	    if ( tx >= 0 ) {
		p->save();
		// compute the clip rectangle the safe way

		int rtop = head->y + ih;
		int rbottom = head->y + ith;
		int rright = tx;
		int rleft = tx + treeStepSize();
		rtop = QMAX( rtop, cy + oy );
		rbottom = QMIN( rbottom, cy+oy+ch );
		rleft = QMAX( rleft, cx+ox );
		rright = QMIN( rright, cx+ox+cw );

		r.setRect( rleft+ox, rtop+ox, rright-rleft, rbottom-rtop );

		p->setClipRect( r );
		head->item->paintTreeBranches( p, colorGroup(),
					       treeStepSize(),
					       r.top(), r.bottom() + 1 );
		p->restore();
	    }

	    // also push the children on the stack
	    const QListViewItem * c = head->item->firstChild();
	    int y = head->y + ih;

	    while ( c && y + c->totalHeight() <= cy ) {
		y += c->totalHeight();
		c = c->nextSibling();
	    }

	    while ( c && y < cy+ch ) {
		next = new PendingItem;
		next->l = head->l + 1;
		next->next = head->next;
		head->next = next;
		next->y = y;
		next->item = c;
		y += c->totalHeight();
		c = c->nextSibling();
	    }
	}

	next = head->next;
	delete head;
	head = next;
    }

    if ( root->totalHeight() < cy + ch ) {
	// really should call some virtual method, or at least use
	// something more configurable than colorGroup().base()
	p->fillRect( cx + ox, root->totalHeight() + oy,
		     cw, cy + ch - root->totalHeight(),
		     colorGroup().base() );
    }
}



/*!  Returns the number of pixels a child is offset from its parent.
  This number has meaning only for tree views.

  \sa setTreeStepSize()
*/

int QListView::treeStepSize() const
{
    return root->levelWidth;
}


/*!  Set teh the number of pixels a child is offset from its parent,
  in a tree view.

  \sa treeStepSize()
*/

 void QListView::setTreeStepSize( int l )
{
    if ( l != root->levelWidth ) {
	root->levelWidth = l;
	// update
    }
}


/*!  Inserts a top-level QListViewItem into this list view.  You
  should not need to call this; the QListViewItem constructor does it
  for you.
*/

void QListView::insertItem( QListViewItem * i )
{
    if ( root ) // not for root itself
	root->insertItem( i );
}


/*!

*/

void QListView::clear()
{
    const QListViewItem * c = root->firstChild();
    const QListViewItem * n;
    while( c ) {
	n = c->nextSibling();
	delete c; // ### deleting a const object?  should it work?
	c = n;
    }
}


/*!

*/

void QListView::setColumn( const char * label, int size, int column )
{
    if ( column < 0 )
	root->h->setCellSize( root->h->addLabel( label ), size );
    else
	root->h->setLabel( column, label, size );
}


/*!

*/

void QListView::show()
{
    QWidget * v = viewport();
    if ( v )
	v->setBackgroundMode( NoBackground );

    viewResize( 250, root->totalHeight() );
    QScrollView::show();
}


/*!

*/

void QListView::updateContents()
{
    int w = 0;
    for( int i=0; i<root->h->count(); i++ )
	w += root->h->cellSize( i );

    int h = root->h->sizeHint().height();
    root->h->setGeometry( frameWidth(), frameWidth(),
			  frameRect().width(), h );
    setMargins( 0, h, 0, 0 );

    viewResize( w, root->totalHeight() );
}


/*!

*/

void QListView::triggerUpdate()
{
    root->timer->start( 0, TRUE );
    viewport()->repaint();
}


/*!

*/

void QListView::resizeEvent( QResizeEvent * e )
{
    QScrollView::resizeEvent( e );
    updateContents();
}


/*!

*/

bool QListView::eventFilter( QObject * o, QEvent * e )
{
    if ( o == viewport() && e ) {
	switch( e->type() ) {
	case Event_MouseButtonPress:
	    break;
	case Event_MouseMove:
	    break;
	case Event_MouseButtonRelease:
	    break;
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
