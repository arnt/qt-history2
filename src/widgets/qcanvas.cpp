/****************************************************************************
**
** Implementation of QCanvas and associated classes
**
** Author  : Warwick Allison (warwick@troll.no)
** Created : 19960330
**
** Copyright (C) 1996-1997 by Warwick Allison.
**
*****************************************************************************/


#include "qcanvas.h"
#include <qpainter.h>
#include <qimage.h>
#include <qwidget.h>
#include <qfile.h>
#include <qtl.h>
#include <qtimer.h>

#include <stdlib.h>

// clusterizer

class QCanvasClusterizer {
public:
    QCanvasClusterizer(int maxclusters);
    ~QCanvasClusterizer();

    void add(int x, int y); // 1x1 rectangle (point)
    void add(int x, int y, int w, int h);
    void add(const QRect& rect);

    void clear();
    int clusters() { return count; }
    const QRect& operator[](int i);

private:
    QRect* cluster;
    int count;
    const int max;
};

static
void include(QRect& r, const QRect& rect)
{
    if (rect.left()<r.left()) {
            r.setLeft(rect.left());
    }
    if (rect.right()>r.right()) {
            r.setRight(rect.right());
    }
    if (rect.top()<r.top()) {
            r.setTop(rect.top());
    }
    if (rect.bottom()>r.bottom()) {
            r.setBottom(rect.bottom());
    }
}

/*
A QCanvasClusterizer groups rectangles (QRects) into non-overlapping rectangles
by a merging heuristic.
*/
QCanvasClusterizer::QCanvasClusterizer(int maxclusters) :
    cluster(new QRect[maxclusters]),
    count(0),
    max(maxclusters)
{ }

QCanvasClusterizer::~QCanvasClusterizer()
{
    delete [] cluster;
}

void QCanvasClusterizer::clear()
{
    count=0;
}

void QCanvasClusterizer::add(int x, int y)
{
    add(QRect(x,y,1,1));
}

void QCanvasClusterizer::add(int x, int y, int w, int h)
{
    add(QRect(x,y,w,h));
}

void QCanvasClusterizer::add(const QRect& rect)
{
    QRect biggerrect(rect.x()-1,rect.y()-1,rect.width()+2,rect.height()+2);

    //assert(rect.width()>0 && rect.height()>0);

    int cursor;

    for (cursor=0; cursor<count; cursor++) {
        if (cluster[cursor].contains(rect)) {
            // Wholly contained already.
            return;
        }
    }

    int lowestcost=9999999;
    int cheapest=-1;
    for (cursor=0; cursor<count; cursor++) {
        if (cluster[cursor].intersects(biggerrect)) {
            QRect larger=cluster[cursor];
            include(larger,rect);
            int cost=larger.width()*larger.height()
                    - cluster[cursor].width()*cluster[cursor].height();

            if (cost < lowestcost) {
                bool bad=FALSE;
                for (int c=0; c<count && !bad; c++) {
                    bad=cluster[c].intersects(larger) && c!=cursor;
                }
                if (!bad) {
                    cheapest=cursor;
                    lowestcost=cost;
                }
            }
        }
    }
    if (cheapest>=0) {
        include(cluster[cheapest],rect);
        return;
    }

    if (count < max) {
        cluster[count++]=rect;
        return;
    }

    // Do cheapest of:
    //     add to closest cluster
    //     do cheapest cluster merge, add to new cluster

    lowestcost=9999999;
    cheapest=-1;
    for (cursor=0; cursor<count; cursor++) {
        QRect larger=cluster[cursor];
        include(larger,rect);
        int cost=larger.width()*larger.height()
                - cluster[cursor].width()*cluster[cursor].height();
        if (cost < lowestcost) {
            bool bad=FALSE;
            for (int c=0; c<count && !bad; c++) {
                bad=cluster[c].intersects(larger) && c!=cursor;
            }
            if (!bad) {
                cheapest=cursor;
                lowestcost=cost;
            }
        }
    }

    // XXX could make an heuristic guess as to whether we
    // XXX need to bother looking for a cheap merge.

    int cheapestmerge1=-1;
    int cheapestmerge2=-1;

    for (int merge1=0; merge1<count; merge1++) {
        for (int merge2=0; merge2<count; merge2++) {
            if (merge1!=merge2) {
                QRect larger=cluster[merge1];
                include(larger,cluster[merge2]);
                int cost=larger.width()*larger.height()
                    - cluster[merge1].width()*cluster[merge1].height()
                    - cluster[merge2].width()*cluster[merge2].height();
                if (cost < lowestcost) {
                    bool bad=FALSE;
                    for (int c=0; c<count && !bad; c++) {
                        bad=cluster[c].intersects(larger) && c!=cursor;
                    }
                    if (!bad) {
                        cheapestmerge1=merge1;
                        cheapestmerge2=merge2;
                        lowestcost=cost;
                    }
                }
            }
        }
    }

    if (cheapestmerge1>=0) {
        include(cluster[cheapestmerge1],cluster[cheapestmerge2]);
        cluster[cheapestmerge2]=cluster[count--];
    } else {
        // if (!cheapest) debugRectangles(rect);
        include(cluster[cheapest],rect);
    }

    // NB: clusters do not intersect (or intersection will
    //     overwrite).  This is a result of the above algorithm,
    //     given the assumption that (x,y) are ordered topleft
    //     to bottomright.
}

const QRect& QCanvasClusterizer::operator[](int i)
{
    return cluster[i];
}

// end of clusterizer



class QCanvasItemPtr {
public:
    QCanvasItemPtr() : ptr(0) { }
    QCanvasItemPtr( QCanvasItem* p ) : ptr(p) { }

    int operator<=(const QCanvasItemPtr& that) const
    {
	// Order same-z objects by identity.
	if (that.ptr->z()==ptr->z())
	    return (long)that.ptr <= (long)ptr;
	return that.ptr->z() <= ptr->z();
    }
    int operator<(const QCanvasItemPtr& that) const
    {
	// Order same-z objects by identity.
	if (that.ptr->z()==ptr->z())
	    return (long)that.ptr < (long)ptr;
	return that.ptr->z() < ptr->z();
    }
    int operator>(const QCanvasItemPtr& that) const
    {
	// Order same-z objects by identity.
	if (that.ptr->z()==ptr->z())
	    return (long)that.ptr > (long)ptr;
	return that.ptr->z() > ptr->z();
    }
    operator QCanvasItem*() { return ptr; }

private:
    QCanvasItem* ptr;
};

void QCanvasItemList::sort()
{
    qHeapSort(*((QValueList<QCanvasItemPtr>*)this));
}

void QCanvasItemList::drawUnique( QPainter& painter )
{
    QCanvasItem* prev=0;
    for (Iterator it=fromLast(); it!=end(); --it) {
	QCanvasItem *g=*it;
	if (g!=prev) {
	    g->draw(painter);
	    prev=g;
	}
    }
}

class QCanvasChunk {
public:
    QCanvasChunk() : changed(TRUE) { }
    // Other code assumes lists are not deleted.  Assignment is also
    // done on ChunkRecs.  So don't add that sort of thing here.

    void sort()
    {
	list.sort();
    }

    const QCanvasItemList* listPtr() const
    {
	return &list;
    }

    void add(QCanvasItem* item)
    {
	list.prepend(item);
	changed = TRUE;
    }

    void remove(QCanvasItem* item)
    {
	list.remove(item);
	changed = TRUE;
    }

    void change()
    {
	changed = TRUE;
    }

    bool takeChange()
    {
	bool y = changed;
	changed = FALSE;
       	return y;
    }

private:
    QCanvasItemList list;
    bool changed;
};


static int gcd(int a, int b)
{
    // XXX Should use good method, but not speed critical.

    int r = QMIN(a,b);
    while ( a%r || b%r )
	r--;
    return r;
}
static int scm(int a, int b)
{
    int g = gcd(a,b);
    return a/g*b;
}



/*!
\class QCanvas qcanvas.h
\brief A QCanvas is a 2D graphic area upon which QCanvasItem objects exist.

A QCanvas contains any number of QCanvasItem subclassed objects and has
any number of QCanvasView widgets observing some part of the canvas.

A canvas containing
many items is different to a widgets containing many subwidgets in the
following ways:

<ul>
    <li>Items are drawn much faster than widgets, especially when non-rectangular.
    <li>Items use less memory than widgets.
    <li>You can do efficient item-to-item hit tests ("collision detection")
	    with items in a canvas.
    <li>Finding items in an area is efficient.
    <li>You can have multiple views of a canvas.
</ul>

Widgets of course offer richer functionality, such as hierarchies,
events, layout, etc.

A canvas has a solid background and a foreground. By default, the canvas will
have a white background, which can be changed with setBackgroundColor().
If you want an image, use setBackgroundPixmap(). A third option is to use
<i>tiles</i>, where the canvas background is
a matrix of small images all the same size,
each chosen from a defined larger pixmap. See setTiles().

On top of the background are objects of QCanvasItems subclasses. Each item
has a Z-height (see QCanvasItem::z()), with the lower-Z items on the background
and higher-Z items on top of them.

Above everything in the canvas is the foreground, as defined by the
drawForeground() function. By default this function draws nothing.

Changes to the items on the canvas are refreshed to the views whenever
update() is called, including creation of new items (which
are created visible, unlike widgets), movement of item, change of shape,
change of visibility, and destruction.

QCanvas has some built-in animation features. If you call QCanvasItem::setVelocity() 
on an item, it will move forward whenever advance() is call.  The advance() function
also calls update(), so you only need to call one or the other. If no items
have a velocity, then advance() is the same as update().

You can have advance() or update() called automatically with setAdvancePeriod()
or setUpdatePeriod() respectively.
*/

void QCanvas::init(int w, int h, int chunksze, int mxclusters)
{
    awidth=w;
    aheight=h;
    chunksize=chunksze;
    maxclusters=mxclusters;
    chwidth=(w+chunksize-1)/chunksize;
    chheight=(h+chunksize-1)/chunksize;
    chunks=new QCanvasChunk[chwidth*chheight];
    QCanvasItem::setCurrentCanvas(this);
    update_timer = 0;
    bgcolor = white;
    grid = 0;
}

/*!
Create a QCanvas with no size.
You will want to call resize(int,int) at some time after creation.
*/
QCanvas::QCanvas()
{
    init(0,0);
}

/*!
Constructs a QCanvas with that is \c w pixels wide and \c h pixels high.
*/
QCanvas::QCanvas(int w, int h)
{
    init(w,h);
}

/*!
  Constructs a QCanvas which will be composed of
  \a h tiles horizontally and \a v tiles vertically.  Each tile
  will be an image \a tilewidth by \a tileheight pixels from
  pixmap \a p.

  The pixmap \a p is a list of tiles, arranged left to right,
  top to bottom, with tile 0 in the top-left corner, tile 1 next
  to the right, and so on.

  The QCanvas is initially sized to show exactly the given number
  of tiles horizontally and vertically.  If it is resized to be larger,
  the entire matrix of tiles will be repeated as much as necessary to
  cover the area.  If it is smaller, tiles to
  the right and bottom will not be visible.
*/
QCanvas::QCanvas( QPixmap p,
        int h, int v, int tilewidth, int tileheight )
{
    init(h*tilewidth, v*tileheight, scm(tilewidth,tileheight) );
    setTiles( p, h, v, tilewidth, tileheight );
}

/*!
  Destructs the canvas.  Does \e not destroy items on the canvas.
*/
QCanvas::~QCanvas()
{
}

/*!
\internal
Returns the chunk at a chunk position.
*/
QCanvasChunk& QCanvas::chunk(int i, int j) const
{ return chunks[i+chwidth*j]; }

/*!
\internal
Returns the chunk at a pixel position.
*/
QCanvasChunk& QCanvas::chunkContaining(int x, int y) const
{ return chunk(x/chunksize,y/chunksize); }

/*!
Returns a list of all items in the canvas.
*/
QCanvasItemList QCanvas::allItems()
{
    QCanvasItemList list;
    for (QPtrDictIterator<void> it=itemDict; it.currentKey(); ++it) {
	list.prepend((QCanvasItem*)it.currentKey());
    }
    return list;
}


/*!
Changes the size of the QCanvas. This is a slow operation.
*/
void QCanvas::resize(int w, int h)
{
    if (awidth==w && aheight==h)
	return;

    QCanvasItem* item;
    QList<QCanvasItem> hidden;
    for (QPtrDictIterator<void> it=itemDict; it.currentKey(); ++it) {
	if (((QCanvasItem*)it.currentKey())->visible()) {
	    ((QCanvasItem*)it.currentKey())->hide();
	    hidden.append(((QCanvasItem*)it.currentKey()));
	}
    }

    int nchwidth=(w+chunksize-1)/chunksize;
    int nchheight=(h+chunksize-1)/chunksize;

    QCanvasChunk* newchunks = new QCanvasChunk[nchwidth*nchheight];

    // Commit the new values.
    //
    awidth=w;
    aheight=h;
    chwidth=nchwidth;
    chheight=nchheight;
    delete [] chunks;
    chunks=newchunks;

    for (item=hidden.first(); item != 0; item=hidden.next()) {
	item->show();
    }

    setAllChanged();
}

/*!
Change the efficiency tuning parameters.  This is a slow operation.

Internally, a canvas uses a low-resolution "chunk matrix" to keep track of
all the items in the canvas. By default, a 1024x1024 pixel canvas will have
a 64x64 chunk matrix, each chunk collecting items in a 16x16 pixel square.
This default is also affected by setTiles().
You can tune this default by using retune(), for example if you have a very
large canvas and want to trade off speed for memory then you might set the
chunk size to 32 or 64.

\a chunksze is the size of square chunk used to break up the
 QCanvas into area to be considered for redrawing.  It
 should be about the average size of items in the QCanvas.
 Chunks too small increase the amount of calculation required
 when drawing.  Chunks too large increase the amount of drawing
 that is needed.

\a mxclusters is the number of rectangular groups of chunks that
 will be separately drawn.  If the QCanvas has a large number
 of small, dispersed items, this should be about that number.
 The more clusters the slower the redraw, but also the bigger
 clusters are the slower the redraw, so a balance is needed.
 Testing reveals that a large number of clusters is almost
 always best.
*/
void QCanvas::retune(int chunksze, int mxclusters)
{
    maxclusters=mxclusters;

    if ( chunksize!=chunksze ) {
	QList<QCanvasItem> hidden;
	for (QPtrDictIterator<void> it=itemDict; it.currentKey(); ++it) {
	    if (((QCanvasItem*)it.currentKey())->visible()) {
		((QCanvasItem*)it.currentKey())->hide();
		hidden.append(((QCanvasItem*)it.currentKey()));
	    }
	}

	chunksize=chunksze;

	int nchwidth=(awidth+chunksize-1)/chunksize;
	int nchheight=(aheight+chunksize-1)/chunksize;

	QCanvasChunk* newchunks = new QCanvasChunk[nchwidth*nchheight];

	// Commit the new values.
	//
	chwidth=nchwidth;
	chheight=nchheight;
	delete [] chunks;
	chunks=newchunks;

	for (QCanvasItem* item=hidden.first(); item != 0; item=hidden.next()) {
	    item->show();
	}

	oneone = tilew == tileh && tilew == chunksize;
    }
}

/*!
\fn int QCanvas::width() const
Returns the width of the canvas, in pixels.
*/

/*!
\fn int QCanvas::height() const
Returns the height of the canvas, in pixels.
*/

/*!
\fn int QCanvas::chunkSize() const
Returns the chunk size of the sprite field as set at construction.
\sa retune()
*/

/*!
\fn bool QCanvas::sameChunk(int x1, int y1, int x2, int y2) const
\internal
Tells if the points (x1,y1) and (x2,y2) are within the same chunk.
*/

/*!
\internal
This method adds an element to the list of QCanvasItem objects
in this QCanvas.  The QCanvasItem class calls this.
*/
void QCanvas::addItem(QCanvasItem* item)
{
    itemDict.insert(item,(void*)1);
}

/*!
\internal
This method adds an element to the list of QCanvasItem objects
to be animated. The QCanvasItem class calls this.
*/
void QCanvas::addAnimation(QCanvasItem* item)
{
    animDict.insert(item,(void*)1);
}

/*!
\internal
This method adds an element to the list of QCanvasItem objects
which are no longer to be animated. The QCanvasItem class calls this.
*/
void QCanvas::removeAnimation(QCanvasItem* item)
{
    animDict.remove(item);
}

/*!
\internal
This method removes an element from the list of QCanvasItem objects
in this QCanvas.  The QCanvasItem class calls this.
*/
void QCanvas::removeItem(QCanvasItem* item)
{
    itemDict.remove(item);
}

/*!
\internal
This method adds an element to the list of QCanvasView objects
viewing this QCanvas.  The QCanvasView class calls this.
*/
void QCanvas::addView(QCanvasView* view)
{
    viewList.append(view);
}

/*!
\internal
This method removes an element from the list of QCanvasView objects
viewing this QCanvas.  The QCanvasView class calls this.
*/
void QCanvas::removeView(QCanvasView* view)
{
    viewList.removeRef(view);
}

/*!
  Sets the canvas to call advance() every \a ms milliseconds.
  Any previous setting by setAdvancePeriod() or setUpdatePeriod() is cancelled.
*/
void QCanvas::setAdvancePeriod(int ms)
{
    if ( ms<0 ) {
	if ( update_timer )
	    update_timer->stop();
    } else {
	if ( !update_timer ) {
	    update_timer = new QTimer(this);
	    connect(update_timer,SIGNAL(timeout()),this,SLOT(advance()));
	}
	update_timer->start(ms);
    }
}

/*!
  Sets the canvas to call update() every \a ms milliseconds.
  Any previous setting by setAdvancePeriod() or setUpdatePeriod() is cancelled.
*/
void QCanvas::setUpdatePeriod(int ms)
{
    if ( ms<0 ) {
	if ( update_timer )
	    update_timer->stop();
    } else {
	if ( !update_timer ) {
	    update_timer = new QTimer(this);
	    connect(update_timer,SIGNAL(timeout()),this,SLOT(update()));
	}
	update_timer->start(ms);
    }
}

/*!
  Advances the animation of items on the canvas and
  refreshes all changes to all views of the canvas.

  The advance is done in two phases.
  In phase 0, the QCanvasItem:advance() function of each animated item
  is called with paramater 0. Then all items are called again, with
  parameter 1. In phase 0, the items should not change position, merely
  examine other items on the canvas for which special processing is
  required, such as collisions between items. In phase 1, all items
  should change positions, ignoring any other items on the canvas.
  This two-phase approach allows for considerations of "fairness",
  though no QCanvasItem subclasses supplied with Qt do anything
  interesting in phase 0.

  The canvas can be configured to call this function periodically
  with setAdvancePeriod().

  \sa update()
*/
void QCanvas::advance()
{
    for (QPtrDictIterator<void> it=animDict; it.current(); ) {
	QCanvasItem* i = (QCanvasItem*)it.currentKey();
	++it;
	if ( i )
	    i->advance(0);
    }
    for (QPtrDictIterator<void> it2=animDict; it2.current(); ) {
	QCanvasItem* i = (QCanvasItem*)it2.currentKey();
	++it2;
	if ( i )
	    i->advance(1);
    }
    update();
}

/*!
  Refreshes all changes to all views of the canvas.

  \sa advance()
*/
void QCanvas::update()
{
    QCanvasClusterizer clusterizer(viewList.count());

    for (QCanvasView* view=viewList.first(); view != 0; view=viewList.next()) {
	QRect area=view->viewArea();
	if (area.width()>0 && area.height()>0) {
	    clusterizer.add(area);
	}
    }

    for (int i=0; i<clusterizer.clusters(); i++) {
	drawChanges(clusterizer[i]);
    }
}

/*!
  Sets all views of the canvas to be entirely redrawn when
  update() is next called.
*/
void QCanvas::setAllChanged()
{
    setChanged(QRect(0,0,width(),height()));
}

/*!
  Sets all views of \a area to be entirely redrawn when
  update() is next called.
*/
void QCanvas::setChanged(const QRect& area)
{
    QRect thearea=area.intersect(QRect(0,0,width(),height()));

    int mx=(thearea.x()+thearea.width()+chunksize)/chunksize;
    int my=(thearea.y()+thearea.height()+chunksize)/chunksize;
    if (mx>chwidth) mx=chwidth;
    if (my>chheight) my=chheight;

    for (int x=thearea.x()/chunksize; x<mx; x++) {
	for (int y=thearea.y()/chunksize; y<my; y++) {
	    chunk(x,y).change();
	}
    }
}

/*!
\internal
Redraw a given area of the QCanvas.

If only_changes then only changes to the area are redrawn.
*/
void QCanvas::drawChanges(const QRect& inarea)
{
    QRect area=inarea.intersect(QRect(0,0,width(),height()));

    QCanvasClusterizer clusters(maxclusters);

    int mx=(area.x()+area.width()+chunksize)/chunksize;
    int my=(area.y()+area.height()+chunksize)/chunksize;
    if (mx>chwidth) mx=chwidth;
    if (my>chheight) my=chheight;

    for (int x=area.x()/chunksize; x<mx; x++) {
	for (int y=area.y()/chunksize; y<my; y++) {
	    QCanvasChunk& ch=chunk(x,y);
	    if (ch.takeChange()) {
		clusters.add(x,y);
	    }
	}
    }

    for (int i=0; i<clusters.clusters(); i++) {
	QRect elarea=clusters[i];
	elarea.setRect(
	    elarea.left()*chunksize,
	    elarea.top()*chunksize,
	    elarea.width()*chunksize,
	    elarea.height()*chunksize
	);
	drawArea(elarea);
    }
}

/*!
\internal
Redraw a given area of the QCanvas.

If only_changes then only changes to the area are redrawn.

If one_view then only one view is updated, otherwise all are.
*/
void QCanvas::drawArea(const QRect& inarea, QPainter* p, bool double_buffer)
{
    QRect area=inarea.intersect(QRect(0,0,width(),height()));

    if (!viewList.first() && !p) return; // Nothing to do.

    int lx=area.x()/chunksize;
    int ly=area.y()/chunksize;
    int mx=(area.x()+area.width()+chunksize)/chunksize;
    int my=(area.y()+area.height()+chunksize)/chunksize;
    if (mx>chwidth) mx=chwidth;
    if (my>chheight) my=chheight;

    QCanvasItemList allvisible;

    for (int x=lx; x<mx; x++) {
	for (int y=ly; y<my; y++) {
	    // Only reset change if all views updating, and
	    // wholy within area. (conservative:  ignore entire boundary)
	    //
	    // Disable this to help debugging.
	    //
	    if (!p) {
		if (x>lx && x<mx-1 && y>ly && y<my-1)
		    chunk(x,y).takeChange();
	    }
	    allvisible += *chunk(x,y).listPtr();
	}
    }
    allvisible.sort();

    if ( double_buffer ) {
	QPainter painter;
	int osw = area.width();
	int osh = area.height();
	if ( osw > offscr.width() || osh > offscr.height() )
	    offscr.resize(QMAX(osw,offscr.width()),QMAX(osh,offscr.height()));
	painter.begin(&offscr);
	painter.translate(-area.x(),-area.y());
	drawBackground(painter,area);
	allvisible.drawUnique(painter);
	drawForeground(painter,area);
	painter.end();
	if ( p ) {
	    p->drawPixmap( area.x(), area.y(), offscr,
		0, 0, area.width(), area.height() );
	    return;
	}
    }

    for (QCanvasView* view=viewList.first(); view; view=viewList.next()) {
	QPainter painter(view->viewport());
	QPoint tr = view->contentsToViewport(area.topLeft());
	if (double_buffer) {
	    painter.drawPixmap(tr,offscr, QRect(QPoint(0,0),area.size()));
	} else {
	    QPoint tl = view->viewportToContents(QPoint(0,0));
	    painter.translate(-tl.x(),-tl.y());
	    painter.setClipRect(painter.xForm(area));
	    drawBackground(painter,area);
	    allvisible.drawUnique(painter);
	    drawForeground(painter,area);
	}
    }
}

/*!
\internal
This method to informs the QCanvas that a given chunk is
`dirty' and needs to be redrawn in the next Update.

(x,y) is a chunk location.

The sprite classes call this.  Any new derived class
of QCanvasItem must do so too.  SetChangedChunkContaining can be used
instead.
*/
void QCanvas::setChangedChunk(int x, int y)
{
    if (x>=0 && x<chwidth && y>=0 && y<chheight) {
	QCanvasChunk& ch=chunk(x,y);
	ch.change();
    }
}

/*!
\internal
This method to informs the QCanvas that the chunk containing
a given pixel is `dirty' and needs to be redrawn in the next Update.

(x,y) is a pixel location.

The item classes call this.  Any new derived class
of QCanvasItem must do so too. SetChangedChunk can be used instead.
*/
void QCanvas::setChangedChunkContaining(int x, int y)
{
    if (x>=0 && x<width() && y>=0 && y<height()) {
	QCanvasChunk& chunk=chunkContaining(x,y);
	chunk.change();
    }
}

/*!
\internal
This method adds a QCanvasItem to the list of those which need to
be drawn if the given chunk is redrawn.  Like SetChangedChunk
and SetChangedChunkContaining, this method marks the chunk as `dirty'.
*/
void QCanvas::addItemToChunk(QCanvasItem* g, int x, int y)
{
    if (x>=0 && x<chwidth && y>=0 && y<chheight) {
	chunk(x,y).add(g);
    }
}

/*!
\internal
This method removes a QCanvasItem from the list of those which need to
be drawn if the given chunk is redrawn.  Like SetChangedChunk
and SetChangedChunkContaining, this method marks the chunk as `dirty'.
*/
void QCanvas::removeItemFromChunk(QCanvasItem* g, int x, int y)
{
    if (x>=0 && x<chwidth && y>=0 && y<chheight) {
	chunk(x,y).remove(g);
    }
}


/*!
\internal
This method adds a QCanvasItem to the list of those which need to
be drawn if the chunk containing the given pixel is redrawn.
Like SetChangedChunk and SetChangedChunkContaining, this method
marks the chunk as `dirty'.
*/
void QCanvas::addItemToChunkContaining(QCanvasItem* g, int x, int y)
{
    if (x>=0 && x<width() && y>=0 && y<height()) {
	chunkContaining(x,y).add(g);
    }
}

/*!
\internal
This method removes a QCanvasItem from the list of those which need to
be drawn if the chunk containing the given pixel is redrawn.
Like SetChangedChunk and SetChangedChunkContaining, this method
marks the chunk as `dirty'.
*/
void QCanvas::removeItemFromChunkContaining(QCanvasItem* g, int x, int y)
{
    if (x>=0 && x<width() && y>=0 && y<height()) {
	chunkContaining(x,y).remove(g);
    }
}

/*!
  Sets the solid background to be the color \a c.
  \sa setBackgroundPixmap(), setTiles()
*/
void QCanvas::setBackgroundColor( const QColor& c )
{
    bgcolor = c;
    setAllChanged();
}

/*!
  Sets the solid background to be \a p, repeated as necessary to cover
  the entire canvas.

  \sa setBackgroundColor(), setTiles()
*/
void QCanvas::setBackgroundPixmap( const QPixmap& p )
{
    setTiles(p, 1, 1, p.width(), p.height());
}

/*!
  This method is called for all updates of the QCanvas.  It renders
  any background graphics.  If the canvas has a background pixmap or a tiled
  background, that graphics is used,
  otherwise it is cleared in the
  background color to the default background color (white).

  If the graphics for an area change, you must explicitly
  call setChanged(const QRect&) for the result to be visible
  when update() is next called.

  \sa setBackgroundColor(), setBackgroundPixmap(), setTiles()
*/
void QCanvas::drawBackground(QPainter& painter, const QRect& clip)
{
    if ( pm.isNull() ) {
	painter.fillRect(clip,bgcolor);
    } else if ( !grid ) {
	for (int x=clip.x()/pm.width();
	    x<(clip.x()+clip.width()+pm.width()-1)/pm.width(); x++)
	{
	    for (int y=clip.y()/pm.height();
		y<(clip.y()+clip.height()+pm.height()-1)/pm.height(); y++)
	    {
		painter.drawPixmap(x*pm.width(), y*pm.height(),pm);
	    }
	}
    } else {
	const int x1 = clip.left()/tilew;
	int x2 = clip.right()/tilew;
	const int y1 = clip.top()/tileh;
	int y2 = clip.bottom()/tileh;

	const int roww = pm.width()/tilew;

	for (int j=y1; j<=y2; j++) {
	    int jj = j%tilesVertically();
	    for (int i=x1; i<=x2; i++) {
		int t = tile(i%tilesHorizontally(), jj);
		int tx = t % roww;
		int ty = t / roww;
		painter.drawPixmap( i*tilew, j*tileh, pm,
				tx*tilew, ty*tileh, tilew, tileh );
	    }
	}
    }
}

/*!
  \fn void QCanvas::drawForeground(QPainter& painter, const QRect& clip)

  This method is called for all updates of the QCanvas.  It renders
  any foreground graphics.

  The same warnings regarding change apply to this method
  as for drawBackground().

  The default is to draw nothing.
*/
void QCanvas::drawForeground(QPainter&, const QRect&)
{
}


/*!
  Sets the QCanvas to be composed of
  \a h tiles horizontally and \a v tiles vertically.  Each tile
  will be an image \a tilewidth by \a tileheight pixels from
  pixmap \a p.

  The pixmap \a p is a list of tiles, arranged left to right,
  top to bottom, with tile 0 in the top-left corner, tile 1 next
  to the right, and so on.

  If the QCanvas is 
  larger than the matrix of tiles,
  the entire matrix will be repeated as much as necessary to
  cover the area.  If it is smaller, tiles to
  the right and bottom will not be visible.

  There are optimizations built-in for the case where the tiles
  are square and the canvas is not retuned.
*/
void QCanvas::setTiles(QPixmap p,
        int h, int v, int tilewidth, int tileheight)
{
    htiles = h;
    vtiles = v;
    delete grid;
    if ( h && v ) {
	grid = new ushort[h*v];
	memset( grid, 0, h*v*sizeof(ushort) );
	pm = p;
	tilew = tilewidth;
	tileh = tileheight;
    } else {
	grid = 0;
    }
    if ( h + v > 10 ) {
	int s = scm(tilewidth,tileheight);
	retune( s < 128 ? s : QMAX(tilewidth,tileheight) );
    }
    oneone = tilew == tileh && tilew == chunksize;
    setAllChanged();
}

/*!
  \fn int tile( int x, int y ) const
  Returns the tile at (\a x, \a y).
  \warning the parameters must be within range.
*/

/*!
  \fn int tilesHorizontally() const
  Returns the number of tiles horizontally.
*/

/*!
  \fn int tilesVertically() const
  Returns the number of tiles vertically.
*/

/*!
  \fn int tileWidth() const
  Returns the width of each tile.
*/
/*!
  \fn int tileHeight() const
  Returns the height of each tile.
*/


/*!
  Sets the tile at (\a x, \a y) to use tile number \a tilenum,
  which is an index into the tile pixmaps.  The canvas will
  update appropriately when update() is next called.

  The images are taken from the pixmap set by setTiles() and
  are arranged in the pixmap left to right, top to bottom, with tile 0
  in the top-left corner, tile 1 next to the right, and so on.
*/
void QCanvas::setTile( int x, int y, int tilenum )
{
    ushort& t = grid[x+y*htiles];
    if ( t != tilenum ) {
	t = tilenum;
	if ( oneone ) {
	    // common case
	    setChangedChunk( x, y );
	} else {
	    setChanged( QRect(x*tilew,y*tileh,tilew,tileh) );
	}
    }
}


// lesser-used data in canvas item, plus room for extension.
// Be careful adding to this - check all usages.
class QCanvasItemExtra {
    double vx,vy;
    friend QCanvasItem;
};


/*!
\class QCanvasItem qcanvas.h
\brief An abstract graphic object on a QCanvas.

QCanvasItems can be moved, hidden, and tested for collision with
other items. They have selected, enabled, and active state flags
which subclasses may use to adjust appearance or behavior.

In most graphic rendering systems, graphic objects are considered to
have a bounding \e rectangle, and redraw optimization is based on this
simplification.  This system is not used by QCanvas.  A
QCanvas considers itself to contain numerous graphic objects, each
of which covers certain `chunks' in the QCanvas.  For graphic
objects with a rectangular bias, this has only a minor effect on
redraw efficiency (although still a major effect on collision
detection and other area-based indexing).  For other shapes, such as
lines, the tighter area-bound made possible by chunks can provide
vast improvements.

Whenever a QCanvasItem moves, it must add and remove itself
from the chunks of the QCanvas upon which it moves.  If the
QCanvasItem is much smaller than the chunk size of the
QCanvas, it will usually be in one, sometimes 2, and occasionally
3 or 4 chunks.  If the QCanvasItem is larger than the
chunk size of the QCanvas, it will span a number of chunks.
Clearly there is a trade-off between tight bounds and excessive
numbers of chunks a QCanvasItem will have to add and remove
itself from.

Note that a QCanvasItem may be `on' a QCanvas even if it's
coordinates place it far off the edge of the area of the QCanvas,
however, collision detection only works for parts of an item
that are within the area of the canvas.
*/

/*!
Constructs a QCanvasItem on the current canvas.

\sa setCurrentSpriteField(QCanvas*) setSpriteField(QCanvas*)
*/
QCanvasItem::QCanvasItem() :
    cnv(current_canvas),
    myx(0),myy(0),myz(0)
{
    ani=0;
    vis=1;
    sel=0;
    ena=0;
    act=0;

    ext = 0;
    if (cnv) cnv->addItem(this);
}


/*!
Destructs the QCanvasItem.  It is removed from its canvas.
*/
QCanvasItem::~QCanvasItem()
{
    if (cnv) cnv->removeItem(this);
    delete ext;
}

QCanvasItemExtra& QCanvasItem::extra()
{
    if ( !ext )
	ext = new QCanvasItemExtra;
    return *ext;
}

/*!
  Returns TRUE is the item is animated.
  \sa setVelocity(), setAnimated()
*/
bool QCanvasItem::animated() const
{
    return ani;
}

/*!
  Sets the item to be animated (or not if \a y is FALSE).

  \sa advance(), QCanvas::advance()
*/
void QCanvasItem::setAnimated(bool y)
{
    if ( y != ani ) {
	ani = y;
	if ( y ) {
	    cnv->addAnimation(this);
	} else {
	    cnv->removeAnimation(this);
	}
    }
}

/*!
  Sets the item to be animated and moving by
  \a dx and \a dy pixels in the horizontal and
  vertical directions respectively.

  \sa advance().
*/
void QCanvasItem::setVelocity( double vx, double vy)
{
    if ( ext || vx!=0.0 || vy!=0.0 ) {
	if ( !ani )
	    setAnimated(TRUE);
	extra().vx = vx;
	extra().vy = vy;
    }
}

/*!
  Returns the horizontal component of the velocity of the item.
*/
double QCanvasItem::xVelocity() const
{
    return ext ? ext->vx : 0;
}

/*!
  Returns the vertical component of the velocity of the item.
*/
double QCanvasItem::yVelocity() const
{
    return ext ? ext->vy : 0;
}

/*!
  Advances the animation of the item.  The default is
  to move the item by the preset velocity (see setVelocity())
  if \a stage is 1.

  \sa QCanvas::advance()
*/
void QCanvasItem::advance(int phase)
{
    if ( ext && phase==1 ) moveBy(ext->vx,ext->vy);
}

/*!
\fn int QCanvasItem::z() const

Returns the z depth of the item,
which is used for visual order:  higher-z items obscure
lower-z ones.
*/

/*!
\fn void QCanvasItem::draw(QPainter& painter)

This abstract method should draw the item using \a painter.
*/

/*!
By default, items are created on the most recently created
QCanvas, because QCanvas objects call this static function.

You can change the canvas used for items with setCanvas().
*/
void QCanvasItem::setCurrentCanvas(QCanvas* c)
{
    current_canvas=c;
}

/*!
Sets the QCanvas upon which the QCanvasItem is to be drawn to \a c.
Initially this will be the current canvas.

\sa setCurrentCanvas(QCanvas*)
*/
void QCanvasItem::setCanvas(QCanvas* c)
{
    bool v=visible();
    setVisible(FALSE);
    if (cnv) {
	cnv->removeItem(this);
    }
    cnv=c;
    if (cnv) {
	cnv->addItem(this);
	if ( ext )
	    cnv->addAnimation(this);
    }
    setVisible(v);
}

/*!
Shorthand for setVisible(TRUE).
*/
void QCanvasItem::show()
{
    setVisible(TRUE);
}

/*!
Shorthand for setVisible(FALSE).
*/
void QCanvasItem::hide()
{
    setVisible(FALSE);
}

/*!
  Makes the items visible (or invisible if \a yes is FALSE)
  when QCanvas::update() is next called.
*/
void QCanvasItem::setVisible(bool yes)
{
    if (vis!=yes) {
	if (yes) {
	    vis=yes;
	    addToChunks();
	} else {
	    removeFromChunks();
	    vis=yes;
	}
    }
}
/*!
  \fn bool QCanvasItem::visible() const
  Returns TRUE if the QCanvasItem is visible.  This does <em>not</em>
  mean the QCanvasItem is currently in a view, merely that if a view
  was showing the area where the QCanvasItem is, and the item
  was not obscured by items at a higher z, it would be visible.

  \sa setVisible(), z()
*/

/*!
  \fn bool QCanvasItem::selected() const
  Returns TRUE if the QCanvasItem is selected.
*/

/*!
  Sets the selected flag of the item to \a yes and
  causes it to be redrawn.

  The behavior of items is not influenced by this value -
  it is for users of the QCanvas/QCanvasItem/QCanvasView classes
  to use it if needed.

  Note that subclasses may not look any different
  if there draw() functions ignore the value
  of selected().
*/
void QCanvasItem::setSelected(bool yes)
{
    if (sel!=yes) {
	sel=yes;
	changeChunks();
    }
}

/*!
  \fn bool QCanvasItem::enabled() const
  Returns TRUE if the QCanvasItem is enabled.
*/

/*!
  Sets the enabled flag of the item to \a yes and
  causes it to be redrawn when QCanvas::update() is
  next called.

  The behavior of items is not influenced by this value -
  it is for users of the QCanvas/QCanvasItem/QCanvasView classes
  to use it if needed.

  Note that subclasses may not look any different
  if there draw() functions ignore the value
  of enabled().
*/
void QCanvasItem::setEnabled(bool yes)
{
    if (ena!=yes) {
	ena=yes;
	changeChunks();
    }
}

/*!
  \fn bool QCanvasItem::active() const
  Returns TRUE if the QCanvasItem is active.
*/

/*!
  Sets the active flag of the item to \a yes and
  causes it to be redrawn when QCanvas::update() is
  next called.

  The behavior of items is not influenced by this value -
  it is for users of the QCanvas/QCanvasItem/QCanvasView classes
  to use it if needed.

  Note that subclasses may not look any different
  if there draw() functions ignore the value
  of active().
*/
void QCanvasItem::setActive(bool yes)
{
    if (act!=yes) {
	act=yes;
	changeChunks();
    }
}

bool qt_testCollision(const QCanvasSprite* s1, const QCanvasSprite* s2)
{
    const QImage* s2image = s2->imageAdvanced()->collision_mask;
    QRect s2area = s2->boundingRectAdvanced();

    QRect cyourarea(s2area.x(),s2area.y(),
	    s2area.width(),s2area.height());

    QImage* s1image=s1->imageAdvanced()->collision_mask;

    QRect s1area = s1->boundingRectAdvanced();

    QRect ourarea = s1area.intersect(cyourarea);

    if ( ourarea.isEmpty() )
	return FALSE;

    int x2=ourarea.x()-cyourarea.x();
    int y2=ourarea.y()-cyourarea.y();
    int x1=ourarea.x()-s1area.x();
    int y1=ourarea.y()-s1area.y();
    int w=ourarea.width();
    int h=ourarea.height();

    if ( !s2image ) {
	if ( !s1image )
	    return w>0 && h>0;
	// swap everything around
	int t;
	t=x1; x1=x2; x2=t;
	t=y1; x1=y2; y2=t;
	s2image = s1image;
	s1image = 0;
    }

    // s2image != 0

    // XXX
    // XXX A non-linear search would typically be more
    // XXX efficient.  Optimal would be spiralling out
    // XXX from the center, but a simple vertical expansion
    // XXX from the centreline would suffice.
    // XXX
    // XXX My sister just had a baby 40 minutes ago, so
    // XXX I'm too brain-spun to implement it correctly!
    // XXX
    //

    // Let's make an assumption.  That sprite masks don't have
    // bit orders for different machines!
    //
    // ASSERT(s1image->bitOrder()==s2image->bitOrder());

    if (s1image) {
	if (s1image->bitOrder() == QImage::LittleEndian) {
	    for (int j=0; j<h; j++) {
		uchar* ml = s1image->scanLine(y1+j);
		uchar* yl = s2image->scanLine(y2+j);
		for (int i=0; i<w; i++) {
		    if (*(yl + ((x2+i) >> 3)) & (1 << ((x2+i) & 7))
		    && *(ml + ((x1+i) >> 3)) & (1 << ((x1+i) & 7)))
		    {
			return TRUE;
		    }
		}
	    }
	} else {
	    for (int j=0; j<h; j++) {
		uchar* ml = s1image->scanLine(y1+j);
		uchar* yl = s2image->scanLine(y2+j);
		for (int i=0; i<w; i++) {
		    if (*(yl + ((x2+i) >> 3)) & (1 << (7-((x2+i) & 7)))
		    && *(ml + ((x1+i) >> 3)) & (1 << (7-((x1+i) & 7))))
		    {
			return TRUE;
		    }
		}
	    }
	}
    } else {
	if (s2image->bitOrder() == QImage::LittleEndian) {
	    for (int j=0; j<h; j++) {
		uchar* yl = s2image->scanLine(y2+j);
		for (int i=0; i<w; i++) {
		    if (*(yl + ((x2+i) >> 3)) & (1 << ((x2+i) & 7)))
		    {
			return TRUE;
		    }
		}
	    }
	} else {
	    for (int j=0; j<h; j++) {
		uchar* yl = s2image->scanLine(y2+j);
		for (int i=0; i<w; i++) {
		    if (*(yl + ((x2+i) >> 3)) & (1 << (7-((x2+i) & 7))))
		    {
			return TRUE;
		    }
		}
	    }
	}
    }

    return FALSE;
}

QCanvas* QCanvasItem::current_canvas=0;

static bool collision_double_dispatch(
			      	const QCanvasSprite* s1,
				const QCanvasPolygonalItem* p1,
				const QCanvasRectangle* r1,
				const QCanvasEllipse* e1,
				const QCanvasText* t1,
				const QCanvasSprite* s2,
				const QCanvasPolygonalItem* p2,
				const QCanvasRectangle* r2,
				const QCanvasEllipse* e2,
				const QCanvasText* t2 )
{
    const QCanvasItem* i1 = s1 ?
		(const QCanvasItem*)s1 : p1 ?
		(const QCanvasItem*)p1 : r1 ?
		(const QCanvasItem*)r1 : e1 ?
		(const QCanvasItem*)e1 : (const QCanvasItem*)t1;
    const QCanvasItem* i2 = s2 ?
		(const QCanvasItem*)s2 : p2 ?
		(const QCanvasItem*)p2 : r2 ?
		(const QCanvasItem*)r2 : e2 ?
		(const QCanvasItem*)e2 : (const QCanvasItem*)t2;

    if ( s1 && s2 ) {
	// a
	return qt_testCollision(s1,s2);
    } else if ( (r1 || t2 || s1) && (r2 || t2 || s2) ) {
	// b
	QRect rc1 = i1->boundingRectAdvanced();
	QRect rc2 = i1->boundingRectAdvanced();
	return rc1.intersects(rc2);
    } else if ( e1 && e2
	    && e1->angleLength()>=360*16 && e2->angleLength()>=360*16
	    && e1->width()==e1->height()
	    && e2->width()==e2->height() )
    {
	// c
	double xd = (e1->x()+e1->xVelocity())-(e2->x()+e1->xVelocity());
	double yd = (e1->y()+e1->yVelocity())-(e2->y()+e1->yVelocity());
	double rd = (e1->width()+e2->width())/2;
	return xd*xd+yd*yd <= rd*rd;
    } else if ( p1 && (p2 || s2 || t2) ) {
	// d
	QPointArray pa1 = p1->areaPointsAdvanced();
	QPointArray pa2 = p2 ? p2->areaPointsAdvanced()
		       : i2->boundingRectAdvanced();
	bool col= !(QRegion(pa1) & QRegion(pa2,TRUE)).isEmpty();

	return col;
    } else {
	return collision_double_dispatch(s2,p2,r2,e2,t2,
					 s1,p1,r1,e1,t1);
    }
}

// XXX XXX XXX documentation effort up to here (48%)

bool QCanvasSprite::collidesWith( const QCanvasItem* i ) const
{
    return i->collidesWith(this,0,0,0,0);
}

bool QCanvasSprite::collidesWith(  const QCanvasSprite* s,
				 const QCanvasPolygonalItem* p,
				 const QCanvasRectangle* r,
				 const QCanvasEllipse* e,
				 const QCanvasText* t ) const
{
    return collision_double_dispatch(s,p,r,e,t,this,0,0,0,0);
}

bool QCanvasPolygonalItem::collidesWith( const QCanvasItem* i ) const
{
    return i->collidesWith(0,this,0,0,0);
}

bool QCanvasPolygonalItem::collidesWith(  const QCanvasSprite* s,
				 const QCanvasPolygonalItem* p,
				 const QCanvasRectangle* r,
				 const QCanvasEllipse* e,
				 const QCanvasText* t ) const
{
    return collision_double_dispatch(s,p,r,e,t,0,this,0,0,0);
}

bool QCanvasRectangle::collidesWith( const QCanvasItem* i ) const
{
    return i->collidesWith(0,this,this,0,0);
}

bool QCanvasRectangle::collidesWith(  const QCanvasSprite* s,
				 const QCanvasPolygonalItem* p,
				 const QCanvasRectangle* r,
				 const QCanvasEllipse* e,
				 const QCanvasText* t ) const
{
    return collision_double_dispatch(s,p,r,e,t,0,this,this,0,0);
}


bool QCanvasEllipse::collidesWith( const QCanvasItem* i ) const
{
    return i->collidesWith(0,this,0,this,0);
}

bool QCanvasEllipse::collidesWith(  const QCanvasSprite* s,
				 const QCanvasPolygonalItem* p,
				 const QCanvasRectangle* r,
				 const QCanvasEllipse* e,
				 const QCanvasText* t ) const
{
    return collision_double_dispatch(s,p,r,e,t,0,this,0,this,0);
}

bool QCanvasText::collidesWith( const QCanvasItem* i ) const
{
    return i->collidesWith(0,0,0,0,this);
}

bool QCanvasText::collidesWith(  const QCanvasSprite* s,
				 const QCanvasPolygonalItem* p,
				 const QCanvasRectangle* r,
				 const QCanvasEllipse* e,
				 const QCanvasText* t ) const
{
    return collision_double_dispatch(s,p,r,e,t,0,0,0,0,this);
}

QCanvasItemList QCanvasItem::collisions(bool exact) const
{
    return canvas()->collisions(chunks(),this,exact);
}

/*!
  Returns a list of items which intersect with the point \a p,
  sorted from shallowest to deepest.
*/
QCanvasItemList QCanvas::collisions(QPoint p) const
{
    return collisions(QRect(p,QSize(1,1)));
}

/*!
  Returns a list of items which intersect with the rectangle \a r,
  sorted from shallowest to deepest.
*/
QCanvasItemList QCanvas::collisions(QRect r) const
{
    QCanvasRectangle i(r);
    i.setCanvas((QCanvas*)this);
    QCanvasItemList l = i.collisions(TRUE);
    l.sort();
    return l;
}

/*!
  Returns a list of items which intersect with the chunks listed
  in \a chunks, excluding \a item.  If \a exact is TRUE, only
  only those which actually QCanvasItem::collidesWith() \a item
  are returned, otherwise items are included just for being in the
  chunks.

  This is a utility function mainly used to implement the simpler
  QCanvasItem::collisions() function.
*/
QCanvasItemList QCanvas::collisions(QPointArray chunks,
	    const QCanvasItem* item, bool exact) const
{
    QPtrDict<void> seen;
    QCanvasItemList result;
    for (int i=0; i<(int)chunks.count(); i++) {
	int x = chunks[i].x();
	int y = chunks[i].y();
	const QCanvasItemList* l = chunk(x,y).listPtr();
	for (QCanvasItemList::ConstIterator it=l->begin(); it!=l->end(); ++it) {
	    QCanvasItem *g=*it;
	    if ( g != item ) {
		if ( !seen.find(g) && (!exact || item->collidesWith(g)) ) {
		    seen.replace(g,(void*)1);
		    result.append(g);
		}
	    }
	}
    }
    return result;
}

void QCanvasItem::addToChunks()
{
    if (visible() && canvas()) {
	QPointArray pa = chunks();
	for (int i=0; i<(int)pa.count(); i++)
	    canvas()->addItemToChunk(this,pa[i].x(),pa[i].y());
    }
}

void QCanvasItem::removeFromChunks()
{
    if (visible() && canvas()) {
	QPointArray pa = chunks();
	for (int i=0; i<(int)pa.count(); i++)
	    canvas()->removeItemFromChunk(this,pa[i].x(),pa[i].y());
    }
}

void QCanvasItem::changeChunks()
{
    if (visible() && canvas()) {
	QPointArray pa = chunks();
	for (int i=0; i<(int)pa.count(); i++)
	    canvas()->setChangedChunk(pa[i].x(),pa[i].y());
    }
}



/*!
\class QCanvasPixmap qcanvas.h
\brief A QCanvasPixmap is a sprite frame image

Note that QCanvasPixmap should be considered an internal class at
the current time.  This will change (as will the interface) once
alpha and description support for QImage is clarified.

\sa QCanvasPixmapSequence QCanvasItem QCanvasSprite
*/

/*!
Construct a QCanvasPixmap from two image files.

The QCanvasPixmap is a masked QPixmap used internally by
sprite classes.

The dataname file must be a PPM file of the form:

\code
P6
# HOTSPOT x y
...
\endcode

That is, it must have an additional comment which gives the (x,y)
coordinate of the `hotspot' of the image.

The `hotspot' position defines the origin pixel in the image
For example, if the hotspot is (10,5), it will be displayed
drawn 10 pixels to the left of and 5 pixels above the actual
(x,y) coordinate of the sprite.

The maskname can be any monochrome image format, such as PBM.
No special comments in the file are needed or recognized.

The maskname may also be 0, in which case the sprite has no mask (it is
a solid rectangle).  This will also be the case if the file maskname
does not exist.
*/
QCanvasPixmap::QCanvasPixmap(const char* dataname, const char* maskname) :
    hotx(0),hoty(0),
    collision_mask(0),
    colhotx(0),colhoty(0)
{
    {
	QFile file(dataname);
	if (file.open(IO_ReadOnly)) {
	    char line[128];
	    file.readLine(line,128); // Skip "P6"/"P3" line
	    file.readLine(line,128);

	    while (line[0]=='#') {
		// Comment line - see if it has additional parameters
		if (0==strncmp(line,"# HOTSPOT ",10)) {
		    sscanf(line+10,"%d %d",&hotx,&hoty);
		    colhotx=hotx;
		    colhoty=hoty;
		}
		file.readLine(line,127);
	    }
	}
    }

    if (!load(dataname)) {
	fprintf(stderr,"QCanvasPixmap::QCanvasPixmap - Failed to read %s\n",dataname);
	exit(1);
    }

    if (maskname) {
	QImageIO iio;
	iio.setFileName(maskname);
	if (iio.read()) {
	    collision_mask=new QImage(iio.image());
	    QBitmap m;
	    m.convertFromImage(*collision_mask);
	    setMask(m);
	} else {
	    collision_mask=0;
	}
    } else if ( mask() ) {
	collision_mask = new QImage(mask()->convertToImage());
qDebug("%dx%dx%d mask",collision_mask->width(),collision_mask->height(),collision_mask->depth());
    } else {
	collision_mask=0;
    }

    colw=width();
    colh=height();
}

/*!
Construct a QCanvasPixmap from a QPixmap and a
\link QCanvasPixmap::setHotSpot() hotspot.\endlink

The QCanvasPixmap is a masked QPixmap used internally by
sprite classes.
*/
QCanvasPixmap::QCanvasPixmap(const QPixmap& pm, QPoint hotspot) :
    QPixmap(pm),
    hotx(hotspot.x()),hoty(hotspot.y()),
    collision_mask(0),
    colhotx(hotspot.x()),colhoty(hotspot.y())
{
    const QBitmap *mask = pm.mask();
    if (mask) {
	collision_mask = new QImage(mask->convertToImage());
    } else {
	collision_mask=0;
    }

    colw=width();
    colh=height();
}


/*!
Deletes any collision mask.
*/
QCanvasPixmap::~QCanvasPixmap()
{
    delete collision_mask;
}

/*!
  \fn int QCanvasPixmap::hotX() const
  Returns the X-position \link QCanvasPixmap::setHotSpot() hotspot\endlink set
  for the pixmap.
*/
/*!
  \fn int QCanvasPixmap::hotY() const
  Returns the Y-position \link QCanvasPixmap::setHotSpot() hotspot\endlink set
  for the pixmap.
*/
/*!
  \fn void QCanvasPixmap::setHotSpot(int x, int y)
  Sets the hotspot to (\a x, \a y).

  The hotspot position defines the origin pixel in the image
  For example, if the hotspot is (10,5), it will be displayed
  drawn 10 pixels to the left of and 5 pixels above the actual
  (x,y) coordinate of the sprite.
*/

/*!
\class QCanvasPixmapSequence qcanvas.h
\brief A sequence of QCanvasPixmap
to have multiple frames for animation.

This allows sprite objects
to have multiple frames for animation.

QCanvasPixmaps for sprite objects are collected into
QCanvasPixmapSequences, which are
the set of images a sprite will use, indexed by the sprite's
frame.  This allows sprites to simply have animated forms.
*/

/*!
Construct a QCanvasPixmapSequence from files.

The framecount parameter sets the number of frames to be
loaded for this image.  The filenames should be printf-style
strings such as "foo%03d.ppm" or "animdir/%4d.pbm".  The actual
filenames are formed by sprintf-ing a string with each integer
from 0 to framecount-1, eg. foo000.ppm, foo001.ppm, foo002.ppm, etc.
*/
QCanvasPixmapSequence::QCanvasPixmapSequence(const char* datafilenamepattern,
	const char* maskfilenamepattern, int fc)
{
    img = 0;
    readPixmaps(datafilenamepattern,maskfilenamepattern,fc);
}

bool QCanvasPixmapSequence::readPixmaps(const char* datafilenamepattern,
        const char* maskfilenamepattern, int fc)
{
    delete [] img;
    framecount = fc;
    img  = new QCanvasPixmap*[fc];
    for (int i=0; i<framecount; i++) {
	char data[1024],mask[1024];
	sprintf(data,datafilenamepattern,i);
	if (maskfilenamepattern) {
	    sprintf(mask,maskfilenamepattern,i);
	    img[i]=new QCanvasPixmap(data,mask);
	} else {
	    img[i]=new QCanvasPixmap(data,0);
	}
    }
    return TRUE;
}

/*!
Construct a QCanvasPixmapSequence from QPixmaps.
*/
QCanvasPixmapSequence::QCanvasPixmapSequence(QList<QPixmap> list, QList<QPoint> hotspots) :
    framecount(list.count()),
    img(new QCanvasPixmap*[list.count()])
{
    if (list.count() != hotspots.count())
	warning("QCanvasPixmapSequence: lists have different lengths");
    list.first();
    hotspots.first();
    for (int i=0; i<framecount; i++) {
	img[i]=new QCanvasPixmap(*list.current(), *hotspots.current());
	list.next();
	hotspots.next();
    }
}

/*!
  Destructs the pixmap sequence.
*/
QCanvasPixmapSequence::~QCanvasPixmapSequence()
{
    for (int i=0; i<framecount; i++)
	delete img[i];
    delete img;
}

/*!
Constructor failure check.  Currently not implemented.
Exceptions would be a better solution.
*/
int QCanvasPixmapSequence::operator!()
{
    return 0;
}

/*!
\fn QCanvasPixmap* QCanvasPixmapSequence::image(int i) const
Returns the \a i-th pixmap in the sequence.
*/

/*!
\fn void QCanvasPixmapSequence::setImage(int i, QCanvasPixmap* p)
Deletes the \a i-th pixmap and replaces it by \a p.  Note that \a p
becomes owned by the sequence - it will be deleted later.
*/

/*!
\fn int QCanvasPixmapSequence::frameCount() const
Returns the length of the sequence.
*/

/*!
When testing sprite collision detection
the default is to use the image mask of the sprite.  By using
readCollisionMasks(const char*), an alternate mask can be used.
Also, by using QCanvasItem::setPixelCollisionPrecision(int),
the resolution of the collision mask can be different to the
resolution of the pixel coordinates of the sprites.

The filename is a printf-style string, as per the constructor
for QCanvasPixmapSequence.

The image must be a PBM file, with a HOTSPOT comment as described
in QCanvasPixmap::QCanvasPixmap(const char*, const char*).

\sa QCanvasItem::setPixelCollisionPrecision(int).
*/
bool QCanvasPixmapSequence::readCollisionMasks(const char* fname)
{
    for (int i=0; i<framecount; i++) {
	char filename[1024];
	sprintf(filename,fname,i);

	{
	    QFile file(filename);
	    if (file.open(IO_ReadOnly)) {
		char line[128];
		file.readLine(line,128); // Skip "P1"/"P4" line
		file.readLine(line,128);

		while (line[0]=='#') {
		    // Comment line - see if it has additional parameters
		    if (0==strncmp(line,"# HOTSPOT ",10)) {
			sscanf(line+10,"%d %d",&img[i]->colhotx,&img[i]->colhoty);
		    }
		    file.readLine(line,128);
		}
	    }
	}
	delete img[i]->collision_mask;
	QImageIO iio;
	iio.setFileName(filename);
	if (!iio.read()) {
	    qWarning("QCanvasPixmapSequence::readCollisionMasks - Failed to read %s\n",filename);
	    return FALSE;
	}
	img[i]->collision_mask=new QImage(iio.image());
    }
    return TRUE;
}

/*!
\class QCanvasItem qcanvas.h
\brief A QCanvasItem which renders as a masked image.

\sa QCanvasSprite
*/


/*!
\fn QCanvasPixmap* QCanvasItem::image() const
This abstract method should return the pixmap to be drawn
for the sprite.
*/

/*!
\fn int QCanvasItem::x() const
This abstract method should return the horizontal location, measured
from the left edge of the QCanvas, at
which to draw the pixmap of the sprite.  Note that the hotspot of the
pixmap will be taken into account.
*/
/*!
\fn int QCanvasItem::y() const
This abstract method should return the vertical location, measured
from the top edge of the QCanvas, at
which to draw the pixmap of the sprite.  Note that the hotspot of the
pixmap will be taken into account.
*/

/*!
The absolute horizontal position of the QCanvasItem.  This is
the pixel position of the left edge of the image, as it takes into
account the hotspot.
*/
int QCanvasSprite::absX() const
{
    return int(x())-image()->hotx;
}

/*!
The absolute horizontal position of the QCanvasItem, if it was moved to
\a nx.
*/
int QCanvasSprite::absX(int nx) const
{
    return nx-image()->hotx;
}

/*!
The absolute vertical position of the QCanvasItem.  This is
the pixel position of the top edge of the image, as it takes into
account the hotspot.
*/
int QCanvasSprite::absY() const
{
    return int(y())-image()->hoty;
}

/*!
The absolute vertical position of the QCanvasItem, if it was
moved to \a ny.
*/
int QCanvasSprite::absY(int ny) const
{
    return ny-image()->hoty;
}

/*!
The right edge of the sprite image.

\sa absX()
*/
int QCanvasSprite::absX2() const
{
    return absX()+image()->width()-1;
}

/*!
The right edge of the sprite image, if it was moved to \a nx.

\sa absX()
*/
int QCanvasSprite::absX2(int nx) const
{
    return absX(nx)+image()->width()-1;
}

/*!
The bottom edge of the sprite image.

\sa absY()
*/
int QCanvasSprite::absY2() const
{
    return absY()+image()->height()-1;
}

/*!
The bottom edge of the sprite image, \a if it was moved to \a ny.

\sa absY()
*/
int QCanvasSprite::absY2(int ny) const
{
    return absY(ny)+image()->height()-1;
}

QCanvasPixmap* QCanvasSprite::imageAdvanced() const
{
    return image();
}

QRect QCanvasItem::boundingRectAdvanced() const
{
    int dx = int(x()+xVelocity())-int(x());
    int dy = int(y()+yVelocity())-int(y());
    QRect r = boundingRect();
    r.moveBy(dx,dy);
    return r;
}

QRect QCanvasSprite::boundingRect() const
{
    return QRect(absX(), absY(), width(), height());
}

/*!
Add the sprite to the chunks in its QCanvas which it overlaps.

This must be called as the values of x(), y(), and image() change
such that the QCanvasItem is removed from chunks it is in,
the values of x(), y(), and image() change, then it is added
back into the then covered chunks in the QCanvas.
*/
QPointArray QCanvasItem::chunks() const
{
    QPointArray r;
    int n=0;
    QRect br = boundingRect();
    if (visible() && canvas()) {
	int chunksize=canvas()->chunkSize();
	br &= QRect(0,0,canvas()->width(),canvas()->height());
	r.resize((br.width()/chunksize+2)*(br.height()/chunksize+2));
	for (int j=br.top()/chunksize; j<=br.bottom()/chunksize; j++) {
	    for (int i=br.left()/chunksize; i<=br.right()/chunksize; i++) {
		r[n++] = QPoint(i,j);
	    }
	}
    }
    r.resize(n);
    return r;
}


/*!
Add the sprite to the chunks in its QCanvas which it overlaps.

This must be called as the values of x(), y(), and image() change
such that the QCanvasItem is removed from chunks it is in,
the values of x(), y(), and image() change, then it is added
back into the then covered chunks in the QCanvas.
*/
void QCanvasSprite::addToChunks()
{
    if (visible() && canvas()) {
	int chunksize=canvas()->chunkSize();
	for (int j=absY()/chunksize; j<=absY2()/chunksize; j++) {
	    for (int i=absX()/chunksize; i<=absX2()/chunksize; i++) {
		canvas()->addItemToChunk(this,i,j);
	    }
	}
    }
}

/*!
Remove the sprite from the chunks in its QCanvas which it overlaps.

\sa addToChunks()
*/
void QCanvasSprite::removeFromChunks()
{
    if (visible() && canvas()) {
	int chunksize=canvas()->chunkSize();
	for (int j=absY()/chunksize; j<=absY2()/chunksize; j++) {
	    for (int i=absX()/chunksize; i<=absX2()/chunksize; i++) {
		canvas()->removeItemFromChunk(this,i,j);
	    }
	}
    }
}

/*!
The width of the sprite, in its current image.
*/
int QCanvasSprite::width() const
{
    return image()->width();
}

/*!
The height of the sprite, in its current image.
*/
int QCanvasSprite::height() const
{
    return image()->height();
}


/*!
(override)

Draws the current image of the sprite at its current position,
as given by image() and x(), y().
*/
void QCanvasSprite::draw(QPainter& painter)
{
    painter.drawPixmap(absX(),absY(),*image());
}

/*!
\class QCanvasView qcanvas.h
\brief An abstraction which views a QCanvas

Use one of the derived classes.
*/

/*!
Destructs the view.
*/
QCanvasView::~QCanvasView()
{
    setCanvas(0);
}

/*!
Change the QCanvas which the QCanvasView is viewing.
*/
void QCanvasView::setCanvas(QCanvas* c)
{
    if (viewing) {
	viewing->removeView(this);
    }
    viewing=c;
    if (viewing) {
	viewing->addView(this);
    }
    resizeContents(viewing->width(),viewing->height());
}

/*!
\fn QRect QCanvasView::viewArea() const

viewArea returns the area of `viewing' which this QCanvasView is
viewing.
The tighter this rectangle can
be defined, the more efficient QCanvas::update will be.

Return zero-width area if nothing viewed.
*/

/*!
\fn void  QCanvasView::beginPainter(QPainter & )
Called to initialize the QPainter - normally calls QPainter::begin().
*/
/*!
\fn void  QCanvasView::flush (const QRect & area)
Called just before the painter is ended.
*/

/*!
\fn bool QCanvasView::preferDoubleBuffering () const

Return TRUE if double-buffering is best for this class of widget.
If any view viewing an area prefers double-buffering, that area will
be double buffered for all views.
*/

/*!
\class QCanvasView qcanvas.h
\brief A QWidget which views a QCanvas

This is where QCanvas meets a QWidget - this class displays
a QCanvas.
*/

static bool repaint_from_moving = FALSE;

void QCanvasView::cMoving(int x, int y)
{
    int dx = x - contentsX();
    int dy = y - contentsY();
    repaint_from_moving = QABS(dx) < width()/8 && QABS(dy) < height()/8;
}

/*!
(override)

Repaint the appropriate area of the QCanvas which this
QCanvasView is viewing.
*/
void QCanvasView::drawContents(QPainter *p, int cx, int cy, int cw, int ch)
{
    QRect r(cx,cy,cw,ch);
    if (viewing) {
	viewing->drawArea(r,p,!repaint_from_moving);
	repaint_from_moving = FALSE;
    } else {
        p->eraseRect(r);
    }
}

/*!
\class QCanvasView qcanvas.h
\brief A QWidget which views a QCanvas, and has scrollbars.
*/

/*!
Construct a QCanvasView which views the given QCanvas.  The
usual QWidget parameters may also be passed.
*/
QCanvasView::QCanvasView(QCanvas* v, QWidget* parent, const char* name, WFlags f) :
    QScrollView(parent,name,f)
{
    viewing = 0;
    setCanvas(v);
    viewport()->setBackgroundColor(v->backgroundColor());
    connect(this,SIGNAL(contentsMoving(int,int)),this,SLOT(cMoving(int,int)));
}

/*!
Returns the area of
the viewport.
*/
QRect QCanvasView::viewArea() const
{
    return QRect(contentsX(),contentsY(),contentsWidth(),contentsHeight());
}

/*!
\class QCanvasPolygonalItem qcanvas.h
\brief A QCanvasItem which renders itself in a polygonal area.

QCanvasPolygonalItem is an abstract class that is useful for all items
which cover a polygonal area of chunks on the field.
Sprites, the other branch of QCanvasItem derivatives usually
cover
a simple rectangular area and are dealt with specially, but typical
geometric shapes such as lines and circles would be quite inefficiently
bounded by rectangular areas - a diagonal line from one corner of the
field area to the other bound be bounded by a rectangle covering
the entire area! QCanvasPolygonalItem objects allow the area to be
defined by a polygon - a sequence of points indicating the chunks
bounding the area covered by the item.

Derived classes should try to define as small as possible an area
to maximize efficiency, but must \e definately be contained completely
within the polygonal area.  Calculating the exact requirements will
generally be difficult, and hence a certain amount of over-estimation
could be expected.
*/

/*!
Construct a QCanvasPolygonalItem.
Derived classes should call addToChunks()
in their constructor once numAreaPoints() and getAreaPoints() are valid.
*/
QCanvasPolygonalItem::QCanvasPolygonalItem()
{
}

/*!
Destruct the QCanvasPolygonalItem.  Derived classes \e must remove the area
from any chunks, as this destructor cannot call the virtual methods
required to do so.  That is, they must call hide() in their
destructor.
*/
QCanvasPolygonalItem::~QCanvasPolygonalItem()
{
}

/*
 * concave: scan convert nvert-sided concave non-simple polygon with vertices at
 * (point[i].x, point[i].y) for i in [0..nvert-1] within the window win by
 * calling spanproc for each visible span of pixels.
 * Polygon can be clockwise or counterclockwise.
 * Algorithm does uniform point sampling at pixel centers.
 * Inside-outside test done by Jordan's rule: a point is considered inside if
 * an emanating ray intersects the polygon an odd number of times.
 *
 *  Paul Heckbert    30 June 81, 18 Dec 89
 */

typedef struct {	/* a polygon edge */
    double x;    /* x coordinate of edge's intersection with current scanline */
    double dx;    /* change in x with respect to y */
    int i;    /* edge number: edge i goes from g_pt[i] to g_pt[i+1] */
} Edge;

static QPointArray g_pt;	/* vertices */

static void
delete_edge(Edge* active, int& nact, int i)	/* remove edge i from active list */
{
    int j;

    for (j=0; j<nact && active[j].i!=i; j++);
    if (j>=nact) return;    /* edge not in active list; happens at win->y0*/
    nact--;
    memcpy(&active[j], &active[j+1], (nact-j)*sizeof(active[0]));
}

static void
insert_edge(Edge* active, int& nact, int i, int y)	/* append edge i to end of active list */
{
    int j;
    double dx;
    QPoint p, q;

    j = i<(int)g_pt.size()-1 ? i+1 : 0;
    if (g_pt[i].y() < g_pt[j].y()) {p = g_pt[i]; q = g_pt[j];}
    else	   {p = g_pt[j]; q = g_pt[i];}
    /* initialize x position at intersection of edge with scanline y */
    active[nact].dx = dx = double(q.x()-p.x())/(q.y()-p.y());
    active[nact].x = dx*(y+.5-p.y())+p.x();
    active[nact].i = i;
    nact++;
}

/* comparison routines for qsort */
static
int compare_ind(const void *u, const void *v)
{
    return g_pt[*(int*)u].y() <= g_pt[*(int*)v].y() ? -1 : 1;
}
static
int compare_active(const void *u, const void *v)
{
    return ((Edge*)u)->x <= ((Edge*)v)->x ? -1 : 1;
}


bool QCanvasPolygonalItem::scan(const QRect& win) const
{
    int k, y0, y1, y, i, j, xl, xr;
    int *ind;		/* list of vertex indices, sorted by g_pt[ind[j]].y */

    int nact;			/* number of active edges */
    Edge *active;		/* active edge list:edges crossing scanline y */

    g_pt = areaPoints();
    int n = g_pt.size();	/* number of vertices */
    if (n<=0) return FALSE;
    ind = new int[n];
    active = new Edge[n];

    /* create y-sorted array of indices ind[k] into vertex list */
    for (k=0; k<n; k++)
	ind[k] = k;
    qsort(ind, n, sizeof(ind[0]), compare_ind);	/* sort ind by g_pt[ind[k]].y */

    nact = 0;				/* start with empty active list */
    k = 0;				/* ind[k] is next vertex to process */
    y0 = QMAX(win.top(), g_pt[ind[0]].y());		/* ymin of polygon */
    y1 = QMIN(win.bottom(), g_pt[ind[n-1]].y()-1);	/* ymax of polygon */

    for (y=y0; y<=y1; y++) {		/* step through scanlines */
	/* scanline y is at y+.5 in continuous coordinates */

	/* check vertices between previous scanline and current one, if any */
	for (; k<n && g_pt[ind[k]].y()<=y; k++) {
	    /* to simplify, if pt.y=y+.5, pretend it's above */
	    /* invariant: y-.5 < g_pt[i].y <= y+.5 */
	    i = ind[k];
	    /*
	     * insert or delete edges before and after vertex i (i-1 to i,
	     * and i to i+1) from active list if they cross scanline y
	     */
	    j = i>0 ? i-1 : n-1;	/* vertex previous to i */
	    if (g_pt[j].y() < y)	/* old edge, remove from active list */
		delete_edge(active,nact,j);
	    else if (g_pt[j].y() > y)	/* new edge, add to active list */
		insert_edge(active,nact,j, y);
	    j = i<n-1 ? i+1 : 0;	/* vertex next after i */
	    if (g_pt[j].y() < y)	/* old edge, remove from active list */
		delete_edge(active,nact,i);
	    else if (g_pt[j].y() > y)	/* new edge, add to active list */
		insert_edge(active,nact,i, y);
	}

	/* sort active edge list by active[j].x */
	qsort(active, nact, sizeof(active[0]), compare_active);

	/* draw horizontal segments for scanline y */
	for (j=0; j<nact; j+=2) {	/* draw horizontal segments */
	    /* span 'tween j & j+1 is inside, span tween j+1 & j+2 is outside */
	    xl = int(active[j].x+.5);		/* left end of span */
	    if (xl<win.left()) xl = win.left();
	    xr = int(active[j+1].x-.5);	/* right end of span */
	    if (xr>win.right()) xr = win.right();
	    if (xl<=xr)
		return TRUE;
	    active[j].x += active[j].dx;	/* increment edge coords */
	    active[j+1].x += active[j+1].dx;
	}
    }

    return FALSE;
}

QPointArray QCanvasPolygonalItem::chunkify(int type)
{
    QPointArray pa = areaPoints();

    if ( !pa.size() )
	return pa;

    // XXX Just a simple implementation for now - find the bounding
    //     rectangle and add to every chunk thus touched.
    // XXX A better algorithm would be some kine of scanline polygon
    //     render (hence the API asks for some points).

    QPointArray result;
    QRect brect = pa.boundingRect();
    int chunksize=canvas()->chunkSize();
    int n=0;
    if ( type == 3 )
	result.resize((brect.width()/chunksize+2)*(brect.height()/chunksize+2));
    for (int j=brect.top()/chunksize; j<=brect.bottom()/chunksize; j++) {
	int i;
	switch ( type ) {
	  case 0:
	    for (i=brect.left()/chunksize; i<=brect.right()/chunksize; i++)
		canvas()->removeItemFromChunk(this,i,j);
	    break;
	  case 1:
	    for (i=brect.left()/chunksize; i<=brect.right()/chunksize; i++)
		canvas()->addItemToChunk(this,i,j);
	    break;
	  case 2:
	    for (i=brect.left()/chunksize; i<=brect.right()/chunksize; i++)
		canvas()->setChangedChunk(i,j);
	    break;
	  case 3:
	    for (i=brect.left()/chunksize; i<=brect.right()/chunksize; i++)
		result[n++] = QPoint(i,j);
	    break;
	}
    }
    if ( type == 3 )
	result.resize(n);
    return result;
}

QRect QCanvasPolygonalItem::boundingRect() const
{
    return areaPoints().boundingRect();
}

/*!
  Adds the sprite to the appropriate chunks.
*/
void QCanvasPolygonalItem::addToChunks()
{
    if (visible() && canvas())
	chunkify(1);
}

/*!
  Removes the sprite from the appropriate chunks.
*/
void QCanvasPolygonalItem::removeFromChunks()
{
    if (visible() && canvas())
	chunkify(0);
}

/*!
  Marks the appropriate chunks as changed.
*/
void QCanvasPolygonalItem::changeChunks()
{
    if (visible() && canvas())
	chunkify(2);
}

void QCanvasPolygonalItem::draw(QPainter & p)
{
    p.setPen(pen);
    p.setBrush(brush);
    drawShape(p);
}

void QCanvasPolygonalItem::moveBy(double dx, double dy)
{
    if ( dx || dy ) {
	removeFromChunks();
	movingBy(dx,dy);
	QCanvasItem::moveBy(dx,dy);
    }
}

void QCanvasPolygonalItem::movingBy(int, int)
{
}

void QCanvasPolygonalItem::setPen(QPen p)
{
    pen = p;
    changeChunks();
}

void QCanvasPolygonalItem::setBrush(QBrush b)
{
    // XXX if transparent, needn't add to inner chunks
    brush = b;
    changeChunks();
}



/*!
  \class QPolygon qcanvas.h
  \brief A polygon with a movable reference point.

  Paints a polygon in a QBrush and QPen.
*/
QCanvasPolygon::QCanvasPolygon()
{
}

QCanvasPolygon::~QCanvasPolygon()
{
    hide();
}

/*!
  Note that QCanvasPolygon does not support an outline (pen is
  always NoPen).
*/
void QCanvasPolygon::drawShape(QPainter & p)
{
    p.setPen(NoPen); // since QRegion(QPointArray) excludes outline :-(  )-:
    p.drawPolygon(poly);
}

void QCanvasPolygon::setPoints(QPointArray pa)
{
    removeFromChunks();
    poly = pa;
    poly.translate(x(),y());
    addToChunks();
}

void QCanvasPolygon::movingBy(int dx, int dy)
{
    poly.translate(dx,dy);
}

QPointArray QCanvasPolygonalItem::areaPointsAdvanced() const
{
    int dx = int(x()+xVelocity())-int(x());
    int dy = int(y()+yVelocity())-int(y());
    QPointArray r = areaPoints();
    if ( dx || dy )
	r.translate(dx,dy);
    return r;
}


/*!
  \fn QPointArray QCanvasPolygonalItem::areaPoints() const

  Must return the points bounding the shape.  Note that the returned
  points are \e outside the object, not touching it.
*/

QPointArray QCanvasPolygon::areaPoints() const
{
    return poly;
}

/*!
  \class QCanvasRectangle qcanvas.h
  \brief A rectangle with a movable top-left point.

  Paints a rectangle in a QBrush and QPen.
*/
QCanvasRectangle::QCanvasRectangle() :
    w(32), h(32)
{
}

QCanvasRectangle::QCanvasRectangle(const QRect& r) :
    w(r.width()), h(r.height())
{
    move(r.x(),r.y());
    addToChunks();
}

QCanvasRectangle::QCanvasRectangle(int x, int y, int width, int height) :
    w(width), h(height)
{
    move(x,y);
    addToChunks();
}

QCanvasRectangle::~QCanvasRectangle()
{
    hide();
}


int QCanvasRectangle::width() const
{
    return w;
}

int QCanvasRectangle::height() const
{
    return h;
}

void QCanvasRectangle::setSize(int width, int height)
{
    removeFromChunks();
    w = width;
    h = height;
    addToChunks();
}

QPointArray QCanvasRectangle::areaPoints() const
{
    QPointArray pa(4);
    pa[0] = QPoint(x()-1,y()-1);
    pa[1] = pa[0] + QPoint(w+1,0);
    pa[2] = pa[0] + QPoint(w+1,h+1);
    pa[3] = pa[0] + QPoint(0,h+1);
    return pa;
}

void QCanvasRectangle::drawShape(class QPainter & p)
{
    p.drawRect(x(), y(), w, h);
}


/*!
  \class QCanvasEllipse qcanvas.h
  \brief An ellipse with a movable center point.

  Paints an ellipse in a QBrush and QPen.
*/


QCanvasEllipse::QCanvasEllipse() :
    w(32), h(32),
    a1(0), a2(360*16)
{
    addToChunks();
}

QCanvasEllipse::QCanvasEllipse(int width, int height,
    int startangle, int angle) :
    w(width),h(height),
    a1(startangle),a2(angle)
{
}

QCanvasEllipse::~QCanvasEllipse()
{
    hide();
}

int QCanvasEllipse::width() const
{
    return w;
}

int QCanvasEllipse::height() const
{
    return h;
}

void QCanvasEllipse::setSize(int width, int height)
{
    removeFromChunks();
    w = width;
    h = height;
    addToChunks();
}

void QCanvasEllipse::setAngles(int start, int length)
{
    removeFromChunks();
    a1 = start;
    a2 = length;
    addToChunks();
}

QPointArray QCanvasEllipse::areaPoints() const
{
    QPointArray r;
    r.makeArc(x()-w/2,y()-h/2,w,h,a1,a2);
    return r;
}

/*!
  Note that QCanvasPolygon does not support an outline (pen is
  always NoPen).
*/
void QCanvasEllipse::drawShape(class QPainter & p)
{
    p.setPen(NoPen); // since QRegion(QPointArray) excludes outline :-(  )-:
    if ( !a1 && a2 == 360*16 ) {
	p.drawEllipse(QRect(QPoint(x()-w/2,y()-h/2),
			QPoint(x()+(w+1)/2, y()+(w+1)/2)));
    } else {
	p.drawPie(QRect(QPoint(x()-w/2,y()-h/2),
			QPoint(x()+(w+1)/2, y()+(w+1)/2)), a1, a2);
    }
}


/*!
\class QCanvasText qcanvas.h
\brief A text object on a QCanvas.

  A QCanvasText has text, a font, color, and position.
*/

/*!
  Construct a QCanvasText with the text "<text>".
*/
QCanvasText::QCanvasText() :
    text("<text>"), flags(0)
{
    setRect();
}

/*!
  Construct a QCanvasText with the text \a t.

  The text should not contain newlines.
*/
QCanvasText::QCanvasText(const char* t) :
    text(t), flags(0)
{
    setRect();
}

/*!
  Construct a QCanvasText with the text \a t and font \a f.

  The text should not contain newlines.
*/
QCanvasText::QCanvasText(const char* t, QFont f) :
    text(t), flags(0),
    font(f)
{
    setRect();
}

/*!
  Destruct the sprite.
*/
QCanvasText::~QCanvasText()
{
}

QRect QCanvasText::boundingRect() const { return brect; }

void QCanvasText::setRect()
{
    static QWidget *w=0;
    if (!w) w = new QWidget;
    QPainter p(w);
    p.setFont(font);
    brect = p.boundingRect(QRect(int(x()),int(y()),0,0), flags, text);
}

/*!
  \fn int QCanvasText::textFlags() const
  Returns the currently set alignment flags.
  \sa setTextFlags()
*/


/*!
  Sets the alignment flags.  These are a bitwise OR or \e some of the
  flags available to QPainter::drawText().

  <ul>
       <li> AlignLeft aligns to the left border.
       <li> AlignRight aligns to the right border.
       <li> AlignHCenter aligns horizontally centered.
       <li> AlignTop aligns to the top border.
       <li> AlignBottom aligns to the bottom border.
       <li> AlignVCenter aligns vertically centered
       <li> AlignCenter (= AlignHCenter | AlignVCenter)
       <li> SingleLine ignores newline characters in the text.
       <li> ExpandTabs expands tabulators.
       <li> ShowPrefix displays "&x" as "x" underlined.
       <li> GrayText grays out the text.
  </ul>

  The DontClip and WordBreak flags are not supported.
*/
void QCanvasText::setTextFlags(int f)
{
    removeFromChunks();
    flags = f;
    setRect();
    addToChunks();
}

/*!
  \fn int QCanvasText::x() const
  Returns the X-position of the left edge of the text.
*/

/*!
  \fn int QCanvasText::y() const
  Returns the Y-position of the top edge of the text.
*/

/*!
  \fn const QRect& QCanvasText::boundingRect() const
  Returns the bounding rectangle of the text.
*/

/*!
  Sets the text to be displayed.  The text may contain newlines.
*/
void QCanvasText::setText( const char* t )
{
    removeFromChunks();
    text = t;
    setRect();
    addToChunks();
}

/*!
  Sets the font in which the text is drawn.
*/
void QCanvasText::setFont( const QFont& f )
{
    removeFromChunks();
    font = f;
    setRect();
    addToChunks();
}

/*!
  Sets the color of the text.
*/
void QCanvasText::setColor(const QColor& c)
{
    col=c;
    changeChunks();
}


void QCanvasText::moveBy(double dx, double dy)
{
    removeFromChunks();
    brect.moveBy(dx, dy);
    QCanvasItem::moveBy(dx,dy);
}

/*!
  Draws the text.
*/
void QCanvasText::draw(QPainter& painter)
{
    painter.setFont(font);
    painter.setPen(col);
    painter.drawText(brect, flags, text);
}

void QCanvasText::changeChunks()
{
    if (visible() && canvas()) {
	int chunksize=canvas()->chunkSize();
	for (int j=brect.top()/chunksize; j<=brect.bottom()/chunksize; j++) {
	    for (int i=brect.left()/chunksize; i<=brect.right()/chunksize; i++) {
		canvas()->setChangedChunk(i,j);
	    }
	}
    }
}

/*!
  Adds the sprite to the appropriate chunks.
*/
void QCanvasText::addToChunks()
{
    if (visible() && canvas()) {
	int chunksize=canvas()->chunkSize();
	for (int j=brect.top()/chunksize; j<=brect.bottom()/chunksize; j++) {
	    for (int i=brect.left()/chunksize; i<=brect.right()/chunksize; i++) {
		canvas()->addItemToChunk(this,i,j);
	    }
	}
    }
}

/*!
  Removes the sprite to the appropriate chunks.
*/
void QCanvasText::removeFromChunks()
{
    if (visible() && canvas()) {
	int chunksize=canvas()->chunkSize();
	for (int j=brect.top()/chunksize; j<=brect.bottom()/chunksize; j++) {
	    for (int i=brect.left()/chunksize; i<=brect.right()/chunksize; i++) {
		canvas()->removeItemFromChunk(this,i,j);
	    }
	}
    }
}



/*!
Returns 0.

Although often frowned upon by purists, Run Time
Type Identification is very useful in this case, as it allows
a QCanvas to be an efficient indexed storage mechanism.

Make your derived classes return their own values for rtti(), and you
can distinguish between objects returned by QCanvas::at().  You should
use values greater than 1000 preferrably a large random number,
to allow for extensions to this class.

However, it is important not to overuse this facility, as
it damages extensibility.  For example, once you have identified
a base class of a QCanvasItem found by QCanvas::at(), cast it
to that type and call meaningful methods rather than acting
upon the object based on its rtti value.

For example:

\code
    if (field.at(p)->rtti() == MySprite::rtti()) {
    MySprite* s = (MySprite*)field.at(p);
    if (s->isDamagable()) s->loseHitPoints(1000);
    if (s->isHot()) myself->loseHitPoints(1000);
    ...
    }
\endcode
*/
int QCanvasItem::rtti() const { return 0; }

int QCanvasSprite::rtti() const { return 1; }

/*!
Returns 2.

\sa QCanvasItem::rtti()
*/
int QCanvasPolygonalItem::rtti() const { return 2; }

/*!
Returns 3.

\sa QCanvasItem::rtti()
*/
int QCanvasText::rtti() const { return 3; }

/*!
Returns 4.

\sa QCanvasItem::rtti()
*/
int QCanvasPolygon::rtti() const { return 4; }

/*!
Returns 5.

\sa QCanvasItem::rtti()
*/
int QCanvasRectangle::rtti() const { return 5; }

/*!
Returns 6.

\sa QCanvasItem::rtti()
*/
int QCanvasEllipse::rtti() const { return 6; }


/*!
\fn QCanvasSprite::QCanvasPositionedSprite(QCanvasPixmapSequence* seq)

Create a QCanvasSprite which uses images from the given sequence.

The sprite in initially at (0,0) on the current canvas
(see constructor for QCanvas), using the 0th sequence frame.
*/
QCanvasSprite::QCanvasSprite(QCanvasPixmapSequence* seq) :
    frm(0),
    images(seq)
{
    show();
    addToChunks();
}

/*!
\fn QCanvasSprite::QCanvasPositionedSprite()

Create a QCanvasSprite without defining its image sequence.

The sprite in initially at (0,0) on the current canvas
(see constructor for QCanvas), using the 0th sequence frame.

Note that you must call setSequence(QCanvasPixmapSequence*) before
doing anything else with the sprite.
*/
QCanvasSprite::QCanvasSprite() :
    frm(0),
    images(0)
{
}

/*!
\fn void QCanvasSprite::setSequence(QCanvasPixmapSequence* seq)

Set the sequence of images used for displaying the sprite.  Note that
the sequence should have enough images for the sprites current frame()
to be valid.
*/
void QCanvasSprite::setSequence(QCanvasPixmapSequence* seq)
{
    bool vis=visible();
    if (vis && images) hide();
    images=seq;
    if (vis) show();
}

/*!
\fn QCanvasSprite::changeChunks()

\internal

Marks any chunks the sprite touches as changed.
*/
void QCanvasSprite::changeChunks()
{
    if (visible() && canvas()) {
        int chunksize=canvas()->chunkSize();
        for (int j=absY()/chunksize; j<=absY2()/chunksize; j++) {
            for (int i=absX()/chunksize; i<=absX2()/chunksize; i++) {
                canvas()->setChangedChunk(i,j);
            }
        }
    }
}

/*!
\fn QCanvasSprite::~QCanvasPositionedSprite()

Destruct the sprite.
It is removed from its QCanvas in this process.
*/
QCanvasSprite::~QCanvasSprite()
{
    removeFromChunks();
}

/*!
\fn int QCanvasSprite::x() const

Returns the stored horizontal position of the sprite.
*/

/*!
\fn int QCanvasSprite::y() const

Returns the stored vertical position of the sprite.
*/

/*!
\fn int QCanvasSprite::z() const

Returns the stored z of the sprite.
*/

/*!
\fn void QCanvasSprite::z(int a)

Sets the stored z of the sprite.
*/

/*!
\fn void QCanvasSprite::frame(int f)

Set the animation frame used for displaying the sprite to
the given index into the QCanvasSprite's QCanvasPixmapSequence.

\sa move(double,double,int)
*/
void QCanvasSprite::frame(int f)
{
    move(x(),y(),f);
}

/*!
\fn int QCanvasSprite::frame() const
Returns the index into the QCanvasSprite's QCanvasPixmapSequence
of the current animation frame.

\sa move(double,double,int)
*/

/*!
\fn int QCanvasSprite::frameCount() const
Returns the number of frames in the QCanvasSprite's QCanvasPixmapSequence.
*/

/*!
\fn void QCanvasSprite::moveBy(double dx, double dy)
Move the sprite from its current position by the given amounts.
*/
void QCanvasItem::moveBy(double dx, double dy)
{
    removeFromChunks();
    myx+=dx;
    myy+=dy;
    addToChunks();
}

void QCanvasItem::move(double x, double y)
{
    moveBy(x-myx,y-myy);
}
void QCanvasSprite::move(double x, double y) { QCanvasItem::move(x,y); }

/*!
\fn void QCanvasSprite::move(double nx, double ny, int nf)

Set both the position and the frame of the sprite.
*/
void QCanvasSprite::move(double nx, double ny, int nf)
{
    if (visible() && canvas()) {
	hide();
	QCanvasItem::move(nx,ny);
	frm=nf;
	show();
    } else {
	QCanvasItem::move(nx,ny);
	frm=nf;
    }
}

