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

    bool hasChanged() const
    {
	return changed;
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

<h3>Drawing</h3>

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

<h3>Animation</h3>

QCanvas has some built-in animation features. If you call QCanvasItem::setVelocity() 
on an item, it will move forward whenever advance() is call.  The advance() function
also calls update(), so you only need to call one or the other. If no items
have a velocity, then advance() is the same as update().

You can have advance() or update() called automatically with setAdvancePeriod()
or setUpdatePeriod() respectively.

<h3>Collision Detection</h3>

Items on the canvas can be tested for collisions with these functions, each of
which returns a list of items which match the hit, sorted from top to bottom
(ie. by decreasing QCanvasItem::z() value).

<ul>
    <li>collisions(QPoint) - items which will collide with a point.
    <li>collisions(QRect) - items which will collide with a rectangle.
</ul>

You can also test for item-to-item collisions with QCanvasItem::collisions().
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
    dblbuf = TRUE;
    debug_redraw_areas = FALSE;
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
  \fn QSize QCanvas::size() const
  Returns the size of the canvas, in pixels.
*/

/*!
  \fn int QCanvas::chunkSize() const
  Returns the chunk size of the canvas as set at construction.
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
	QRect area(view->contentsX(),view->contentsY(),
		view->contentsWidth(),view->contentsHeight());
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
	    if (ch.hasChanged()) {
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

    if ( !dblbuf )
	double_buffer = FALSE;

    if (!viewList.first() && !p) return; // Nothing to do.

    int lx=area.x()/chunksize;
    int ly=area.y()/chunksize;
    int mx=area.right()/chunksize;
    int my=area.bottom()/chunksize;
    if (mx>=chwidth) mx=chwidth-1;
    if (my>=chheight) my=chheight-1;

    QCanvasItemList allvisible;

    QRegion rgn;

    for (int x=lx; x<=mx; x++) {
	for (int y=ly; y<=my; y++) {
	    // Only reset change if all views updating, and
	    // wholy within area. (conservative:  ignore entire boundary)
	    //
	    // Disable this to help debugging.
	    //
	    if (!p) {
		if ( chunk(x,y).takeChange() ) {
		    // XXX should at least make bands
		    rgn |= QRegion(x*chunksize,y*chunksize,chunksize,chunksize);
		    allvisible += *chunk(x,y).listPtr();
		}
	    } else {
		allvisible += *chunk(x,y).listPtr();
	    }
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
	painter.setClipRegion(rgn);
	if (double_buffer) {
	    painter.drawPixmap(tr,offscr, QRect(QPoint(0,0),area.size()));
	} else {
	    QPoint tl = view->viewportToContents(QPoint(0,0));
	    painter.translate(-tl.x(),-tl.y());
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
  Returns the color set by setBackgroundColor().
  By default, this is white.

  \sa setBackgroundColor(), backgroundPixmap()
*/
QColor QCanvas::backgroundColor() const
{
    return bgcolor;
}

/*!
  Sets the solid background to be the color \a c.
  \sa backgroundColor(), setBackgroundPixmap(), setTiles()
*/
void QCanvas::setBackgroundColor( const QColor& c )
{
    bgcolor = c;
    setAllChanged();
}

/*!
  Returns the pixmap set by setBackgroundPixmap().
  By default, this is a \link QPixmap::isNull() null\endlink
  pixmap.

  \sa setBackgroundPixmap(), backgroundColor()
*/
QPixmap QCanvas::backgroundPixmap() const
{
    return pm;
}

/*!
  Sets the solid background to be \a p, repeated as necessary to cover
  the entire canvas.

  \sa backgroundPixmap(), setBackgroundColor(), setTiles()
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
  This method is called for all updates of the QCanvas.  It renders
  any foreground graphics.

  The same warnings regarding change apply to this method
  as for drawBackground().

  The default is to draw nothing.
*/
void QCanvas::drawForeground(QPainter& painter, const QRect& clip)
{
    if ( debug_redraw_areas ) {
	painter.setPen(red);
	painter.setBrush(NoBrush);
	painter.drawRect(clip);
    }
}

void QCanvas::setDoubleBuffering(bool y)
{
    dblbuf = y;
}

/*!
  For experimentation, calling setRedrawAreaDisplay(TRUE) will cause the
  redrawn rectangles to be shown with a red outline. This can be useful in
  understanding the optimizations made by QCanvas.

  This should not be used for end-user applications.
*/
void QCanvas::setRedrawAreaDisplay(bool s)
{
    debug_redraw_areas = s;
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
  \fn int QCanvas::tile( int x, int y ) const

  Returns the tile set at (\a x,\a y). Initially,
  all tiles are 0.

  \warning the parameters must be within range.

  \sa setTile()
*/

/*!
  \fn int QCanvas::tilesHorizontally() const
  Returns the number of tiles horizontally.
*/

/*!
  \fn int QCanvas::tilesVertically() const
  Returns the number of tiles vertically.
*/

/*!
  \fn int QCanvas::tileWidth() const
  Returns the width of each tile.
*/
/*!
  \fn int QCanvas::tileHeight() const
  Returns the height of each tile.
*/


/*!
  Sets the tile at (\a x, \a y) to use tile number \a tilenum,
  which is an index into the tile pixmaps.  The canvas will
  update appropriately when update() is next called.

  The images are taken from the pixmap set by setTiles() and
  are arranged in the pixmap left to right, top to bottom, with tile 0
  in the top-left corner, tile 1 next to the right, and so on.

  \sa tile()
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

For details of collision detection, see collisions(). Other functions
related to collision detection are collidesWith(), and the
QCanvas::collisions() functions.
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
\fn double QCanvasItem::x() const
Returns the horizontal position of the item.
Note that subclasses often have
an origin other than the top-left corner.
*/

/*!
\fn double QCanvasItem::y() const
Returns the vertical position of the item.
Note that subclasses often have
an origin other than the top-left corner.
*/

/*!
\fn double QCanvasItem::z() const

Returns the z height of the item,
which is used for visual order:  higher-z items obscure
lower-z ones.
*/

/*!
  \fn void QCanvasItem::setX(double x)

  Moves the item so that its X-position is \a x;
  \sa x(), move()
*/

/*!
  \fn void QCanvasItem::setY(double y)

  Moves the item so that its Y-position is \a y;
  \sa y(), move()
*/

/*!
  \fn void QCanvasItem::setZ(double z)

  Sets the height of the item to \a z.
  Higher-z items obscure lower-z ones.

  \sa z(), move()
*/

/*!
Moves the item from its current position by the given amounts.
*/
void QCanvasItem::moveBy(double dx, double dy)
{
    removeFromChunks();
    myx+=dx;
    myy+=dy;
    addToChunks();
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
  \fn void QCanvasItem::setXVelocity( double vx )

  Sets the horizontal component of the item's velocity to \a vx.
*/

/*!
  \fn void QCanvasItem::setYVelocity( double vy )

  Sets the vertical component of the item's velocity to \a vy.
*/

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

\sa setCurrentCanvas(QCanvas*), canvas()
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
  \fn QCanvas* QCanvasItem::canvas() const

  Returns the canvas containing the item.
*/

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

/*!
  \fn bool QCanvasItem::collidesWith( const QCanvasItem* other ) const

  Returns TRUE if the item will collide with the \a other item \i after they
  have moved by their current velocities.

  \sa collisions()
*/


/*!
  \class QCanvasSprite qcanvas.h
  \brief A masked image on a canvas.

  ...
*/


/*!
  \reimp
*/
bool QCanvasSprite::collidesWith( const QCanvasItem* i ) const
{
    return i->collidesWith(this,0,0,0,0);
}

/*!
  \fn bool QCanvasItem::collidesWith(  const QCanvasSprite* s,
				 const QCanvasPolygonalItem* p,
				 const QCanvasRectangle* r,
				 const QCanvasEllipse* e,
				 const QCanvasText* t ) const

  Returns TRUE if the item collides with any of the given items. The parameters
  are all the same object, this is just a type resolution trick.
*/


bool QCanvasSprite::collidesWith(  const QCanvasSprite* s,
				 const QCanvasPolygonalItem* p,
				 const QCanvasRectangle* r,
				 const QCanvasEllipse* e,
				 const QCanvasText* t ) const
{
    return collision_double_dispatch(s,p,r,e,t,this,0,0,0,0);
}

/*!
  \reimp
*/
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

/*!
  \reimp
*/
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


/*!
  \reimp
*/
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

/*!
  \reimp
*/
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

/*!
  Returns the list of items that this item collides with.

  A collision is generally defined as pixels of one item drawing on the
  pixels of another item, but not all subclasses are so precise. Also,
  since pixelwise collision detection can be slow, this function
  works in either exact or inexact mode, according to the \a exact
  parameter.

  In exact mode, items returned have been accurately tested to collide
  with the item.

  In inexact mode, the items returned are only \i near the item and
  should be tested using collidesWith() if they are interesting collision
  candidates. By using this, you can ignore some items for which collisions
  are not interesting.

  The returned list is just a list of QCanvasItems, but often you will need
  to cast the items to more useful types. The safe way to do that is to
  use rtti() before casting. This provides some of the functionality of
  standard C++ dynamic cast operation even on compilers where that is not
  available.

  Note that while a QCanvasItem may be `on' a QCanvas even if it's
  coordinates place it far off the edge of the area of the QCanvas,
  collision detection only works for parts of an item
  that are within the area of the canvas.
*/
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

/*!
  \internal
  Adds the item to all the chunks it covers.
*/
void QCanvasItem::addToChunks()
{
    if (visible() && canvas()) {
	QPointArray pa = chunks();
	for (int i=0; i<(int)pa.count(); i++)
	    canvas()->addItemToChunk(this,pa[i].x(),pa[i].y());
    }
}

/*!
  \internal
  Removes the item from all the chunks it covers.
*/
void QCanvasItem::removeFromChunks()
{
    if (visible() && canvas()) {
	QPointArray pa = chunks();
	for (int i=0; i<(int)pa.count(); i++)
	    canvas()->removeItemFromChunk(this,pa[i].x(),pa[i].y());
    }
}

/*!
  \internal
  Sets all the chunks covered by the item to be refreshed with QCanvas::update()
  is next called.
*/
void QCanvasItem::changeChunks()
{
    if (visible() && canvas()) {
	QPointArray pa = chunks();
	for (int i=0; i<(int)pa.count(); i++)
	    canvas()->setChangedChunk(pa[i].x(),pa[i].y());
    }
}

/*!
  \fn QRect QCanvasItem::boundingRect() const

  Returns the bounding rectangle of pixels that the item covers.

  \sa boundingRectAdvanced()
*/

/*!
  Returns the bounding rectangle of pixels that the item \i will cover
  after advance(1) is called.

  \sa boundingRect()
*/
QRect QCanvasItem::boundingRectAdvanced() const
{
    int dx = int(x()+xVelocity())-int(x());
    int dy = int(y()+yVelocity())-int(y());
    QRect r = boundingRect();
    r.moveBy(dx,dy);
    return r;
}



/*!
  \class QCanvasPixmap qcanvas.h
  \brief A QCanvasPixmap is a pixmap with an offset.

  QImage has an offset or "hot spot", but QPixmap does not.
  This class adds the notion of an offset to QPixmap as this
  is very useful for the canvas sprites where QCanvasPixmap is used.
  It also keeps a copy of the display mask for use in collision
  detection.

  Note that PNG format files already have support for an offset.

  \sa QCanvasPixmapArray QCanvasItem QCanvasSprite
*/


/*!
  Constructs a QCanvasPixmap from an image file by loading it.
*/
QCanvasPixmap::QCanvasPixmap(const QString& datafilename)
{
    QImage image(datafilename);
    init(image);
}
/*!
  Constructs a QCanvasPixmap from an image.
*/
QCanvasPixmap::QCanvasPixmap(const QImage& image)
{
    init(image);
}
/*!
  Constructs a QCanvasPixmap from a pixmap and an offset.
*/
QCanvasPixmap::QCanvasPixmap(const QPixmap& pm, QPoint offset)
{
    init(pm,offset.x(),offset.y());
}

void QCanvasPixmap::init(const QImage& image)
{
    convertFromImage(image);
    hotx = image.offset().x();
    hoty = image.offset().y();
    if ( image.hasAlphaBuffer() )
	collision_mask = new QImage(image.createAlphaMask());
}

void QCanvasPixmap::init(const QPixmap& pixmap, int hx, int hy)
{
    (QPixmap&)*this = pixmap;
    hotx = hx;
    hoty = hy;
    if ( pixmap.mask() )
	collision_mask = new QImage(mask()->convertToImage());
}

/*!
  Destructs the pixmap.
*/
QCanvasPixmap::~QCanvasPixmap()
{
    delete collision_mask;
}

/*!
  \fn int QCanvasPixmap::offsetX() const
  Returns the X-offset of the pixmap.

  \sa setOffset()
*/
/*!
  \fn int QCanvasPixmap::offsetY() const
  Returns the Y-offset of the pixmap.

  \sa setOffset()
*/
/*!
  \fn void QCanvasPixmap::setOffset(int x, int y)
  Sets the offset to (\a x, \a y).

  The offset position or "hot spot" defines the origin pixel in the image
  For example, if the offset is (10,5), it will be displayed
  drawn 10 pixels to the left of and 5 pixels above the actual
  (x,y) coordinate of the sprite.
*/

/*!
  \class QCanvasPixmapArray qcanvas.h
  \brief An array of QCanvasPixmap to have multiple frames for animation.

  This allows sprite objects
  to have multiple frames for animation.

  QCanvasPixmaps for sprite objects are collected into
  QCanvasPixmapArrays, which are
  the set of images a sprite will use, indexed by the sprite's
  frame.  This allows sprites to simply have animated forms.
*/

/*!
Construct a QCanvasPixmapArray from files.

The framecount parameter sets the number of frames to be
loaded for this image.  The filenames should contain a "%1",
strings such as "foo%1.png".  The actual
filenames are formed by replaceing the %1 with each integer
from 0 to framecount-1, with leading zeroes sufficient for 4 digits.
eg. foo0000.png, foo0001.png, foo0002.png, etc.
*/
QCanvasPixmapArray::QCanvasPixmapArray(const QString& datafilenamepattern, int fc)
{
    img = 0;
    readPixmaps(datafilenamepattern,fc);
}

/*!
Construct a QCanvasPixmapArray from QPixmaps.
*/
QCanvasPixmapArray::QCanvasPixmapArray(QList<QPixmap> list, QList<QPoint> hotspots) :
    framecount(list.count()),
    img(new QCanvasPixmap*[list.count()])
{
    if (list.count() != hotspots.count()) {
	qWarning("QCanvasPixmapArray: lists have different lengths");
	delete [] img;
	img = 0;
    } else {
	list.first();
	hotspots.first();
	for (int i=0; i<framecount; i++) {
	    img[i]=new QCanvasPixmap(*list.current(), *hotspots.current());
	    list.next();
	    hotspots.next();
	}
    }
}

/*!
  Destructs the pixmap array.
*/
QCanvasPixmapArray::~QCanvasPixmapArray()
{
    reset();
}

void QCanvasPixmapArray::reset()
{
    for (int i=0; i<framecount; i++)
	delete img[i];
    delete [] img;
}

/*!
  Read new pixmaps into the array.
*/
bool QCanvasPixmapArray::readPixmaps(const QString& datafilenamepattern, int fc)
{
    return readPixmaps(datafilenamepattern,fc,FALSE);
}
/*!
  When testing sprite collision detection
  the default is to use the image mask of the sprite.  By using
  readCollisionMasks(), an alternate mask can be used. The images
  must be 1-bit deep.
*/
bool QCanvasPixmapArray::readCollisionMasks(const QString& fname)
{
    return readPixmaps(fname,framecount,TRUE);
}


bool QCanvasPixmapArray::readPixmaps(const QString& datafilenamepattern, int fc, bool maskonly)
{
    if ( !maskonly ) {
	delete [] img;
	framecount = fc;
	img = new QCanvasPixmap*[fc];
    }
    bool ok = TRUE;
    for (int i=0; i<framecount; i++) {
	QString r;
	r.sprintf("%04d",i);
	if ( maskonly ) {
	    img[i]->collision_mask->load(datafilenamepattern.arg(r));
	    ok &= !img[i]->collision_mask->isNull() && img[i]->collision_mask->depth()==1;
	} else {
	    img[i]=new QCanvasPixmap(datafilenamepattern.arg(r));
	    ok &= !img[i]->isNull();
	}
    }
    if ( !ok ) {
	reset();
    }
    return ok;
}

/*!
  Constructor failure check.
*/
int QCanvasPixmapArray::operator!()
{
    return img==0;
}

/*!
\fn QCanvasPixmap* QCanvasPixmapArray::image(int i) const
Returns the \a i-th pixmap in the array.
*/

/*!
Replaces the \a i-th pixmap by \a p.  Note that \a p
becomes owned by the array - it will be deleted later.
*/
void QCanvasPixmapArray::setImage(int i, QCanvasPixmap* p)
{
    delete img[i]; img[i]=p;
}

/*!
\fn int QCanvasPixmapArray::count() const
Returns the length of the array.
*/


/*!
  Moves the item to (\a x,\a y) by calling the moveBy()
  virtual function.
*/
void QCanvasItem::move(double x, double y)
{
    moveBy(x-myx,y-myy);
}


/*!
The absolute horizontal position of the sprite.  This is
the pixel position of the left edge of the image, as it takes into
account the offset.
*/
int QCanvasSprite::absX() const
{
    return int(x())-image()->hotx;
}

/*!
The absolute horizontal position of the sprite, if it was moved to
\a nx.
*/
int QCanvasSprite::absX(int nx) const
{
    return nx-image()->hotx;
}

/*!
The absolute vertical position of the sprite.  This is
the pixel position of the top edge of the image, as it takes into
account the offset.
*/
int QCanvasSprite::absY() const
{
    return int(y())-image()->hoty;
}

/*!
The absolute vertical position of the sprite, if it was
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
The right edge of the sprite image, if the sprite was moved to \a nx.

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
The bottom edge of the sprite image, \a if the sprite was moved to \a ny.

\sa absY()
*/
int QCanvasSprite::absY2(int ny) const
{
    return absY(ny)+image()->height()-1;
}

/*!
  The image the sprite \i will have after advance(1) is called.
  Be default this is the same as image().
*/
QCanvasPixmap* QCanvasSprite::imageAdvanced() const
{
    return image();
}

/*
  Returns the bounding rectangle of pixels covered by the item.
*/
QRect QCanvasSprite::boundingRect() const
{
    return QRect(absX(), absY(), width(), height());
}

/*!
  \intenal
  Returns the chunks covered by the item.
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
  \internal
  Add the sprite to the chunks in its QCanvas which it overlaps.
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
  \internal
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
  \sa frame()
*/
int QCanvasSprite::width() const
{
    return image()->width();
}

/*!
  The height of the sprite, in its current image.
  \sa frame()
*/
int QCanvasSprite::height() const
{
    return image()->height();
}


/*!
  Draws the current image of the sprite at its current position,
  as given by image() and x(), y().
*/
void QCanvasSprite::draw(QPainter& painter)
{
    painter.drawPixmap(absX(),absY(),*image());
}

/*!
  \class QCanvasView qcanvas.h
  \brief A QWidget which views a QCanvas.

  Displays a view of a QCanvas, with scrollbars available as
  for all QScrollView subclasses. There can be more than one
  view of a canvas.

  The view of a canvas is the object which the user can see and
  interact with, hence any interactivity will be based on events
  from a view. For example, by subclassing QCanvasView and overriding
  QScrollView::contentsMousePressEvent(), an application can allow the
  user to interact with items on the canvas.

  \code
void MyCanvasView::contentsMousePressEvent(QMouseEvent* e)
{
    QCanvasItemList list = canvas()->collisions(e->pos());
    if ( !list.isEmpty() ) {
	QCanvasItem* item = list.first();

	// Process the top item
	...
    }
}
  \endcode

  Most of the functionality of QCanvasView is the functionality
  available for all QScrollView subclasses.
*/

/*!
  Constructs a QCanvasView which views \a canvas.  The
  usual QWidget parameters may also be supplied.
*/
QCanvasView::QCanvasView(QCanvas* canvas, QWidget* parent, const char* name, WFlags f) :
    QScrollView(parent,name,f)
{
    viewing = 0;
    setCanvas(canvas);
    viewport()->setBackgroundColor(viewing->backgroundColor());
    connect(this,SIGNAL(contentsMoving(int,int)),this,SLOT(cMoving(int,int)));
}

/*!
  Destructs the view. The associated canvas is \i not deleted.
*/
QCanvasView::~QCanvasView()
{
    setCanvas(0);
}

/*!
  \fn QCanvas* QCanvasView::canvas() const

  Returns the canvas which the view is currently viewing.
*/


/*!
  Changes the QCanvas which the QCanvasView is viewing to \a canvas.
*/
void QCanvasView::setCanvas(QCanvas* canvas)
{
    if (viewing) {
	viewing->removeView(this);
    }
    viewing=canvas;
    if (viewing) {
	viewing->addView(this);
    }
    resizeContents(viewing->width(),viewing->height());
}


static bool repaint_from_moving = FALSE;

void QCanvasView::cMoving(int x, int y)
{
    // A little kludge to smooth up repaints when scrolling
    int dx = x - contentsX();
    int dy = y - contentsY();
    repaint_from_moving = QABS(dx) < width()/8 && QABS(dy) < height()/8;
}

/*!
Repaints the appropriate area of the QCanvas which this
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

QSize QCanvasView::sizeHint() const
{
    return canvas()->size()+QSize(frameWidth(),frameWidth())*2;
}


/*!
  \class QCanvasPolygonalItem qcanvas.h
  \brief A QCanvasItem which renders itself in a polygonal area.

  QCanvasPolygonalItem is an abstract class that is useful for all items
  which cover a polygonal area of the canvas.
  QCanvasSprite and QCanvasText, the other branches of QCanvasItem derivatives
  usually cover a simple rectangular area and are dealt with specially,
  but typical geometric shapes such as lines and circles would be too
  inefficiently bounded by rectangular areas - a diagonal line from one
  corner of the canvas area to the other bound be bounded by a rectangle covering
  the entire area! QCanvasPolygonalItem objects allow the area to be
  defined by a polygon - a sequence of points
  bounding the area covered by the item.

  Derived classes should try to define as small as possible an area
  to maximize efficiency, but must \e definately be contained completely
  within the polygonal area.  Calculating the exact requirements may
  be difficult, but a small amount of over-estimation is better than
  any under-estimation, which will give drawing errors.

  All subclasses must call addToChunks()
  in their constructor once numAreaPoints() and getAreaPoints() are valid,
  and must call hide() in their
  destructor while those functions are still valid.
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

struct QPolygonalProcessor {
    QPolygonalProcessor(QCanvas* c, const QPointArray& pa) :
	canvas(c)
    {
	QRect pixelbounds = pa.boundingRect();
	int cs = canvas->chunkSize();
	bounds.setLeft(pixelbounds.left()/cs);
	bounds.setRight(pixelbounds.right()/cs);
	bounds.setTop(pixelbounds.top()/cs);
	bounds.setBottom(pixelbounds.bottom()/cs);
	bitmap = QImage(bounds.width(),bounds.height(),1,2,QImage::LittleEndian);
	pnt = 0;
	bitmap.fill(0);
    }

    inline void add(int x, int y)
    {
	if ( pnt >= (int)result.size() ) {
	    result.resize(pnt*2+10);
	}
	result[pnt++] = QPoint(x+bounds.x(),y+bounds.y());
    }

    inline void addBits(int x1, int x2, uchar newbits, int xo, int yo)
    {
	for (int i=x1; i<=x2; i++)
	    if ( newbits & (1<<i) )
		add(xo+i,yo);
    }

    void doSpans(int n, QPoint* pt, int* w)
    {
	int cs = canvas->chunkSize();
	for (int j=0; j<n; j++) {
	    int y = pt[j].y()/cs-bounds.y();
	    uchar* l = bitmap.scanLine(y);
	    int x = pt[j].x();
	    int x1 = x/cs-bounds.x();
	    int x2 = (x+w[j]-1)/cs-bounds.x();
	    int x1q = x1/8;
	    int x1r = x1%8;
	    int x2q = x2/8;
	    int x2r = x2%8;
	    if ( x1q == x2q ) {
		uchar newbits = (~l[x1q]) & (((2<<(x2r-x1r))-1)<<x1r);
		if ( newbits ) {
		    addBits(x1r,x2r,newbits,x1q*8,y);
		    l[x1q] |= newbits;
		}
	    } else {
		uchar newbits1 = (~l[x1q]) & (0xff<<x1r);
		if ( newbits1 ) {
		    addBits(x1r,7,newbits1,x1q*8,y);
		    l[x1q] |= newbits1;
		}
		for (int i=x1q+1; i<x2q; i++) {
		    if ( l[i] != 0xff ) {
			addBits(0,7,~l[i],i*8,y);
			l[i]=0xff;
		    }
		}
		uchar newbits2 = (~l[x2q]) & (0xff>>(7-x2r));
		if ( newbits2 ) {
		    addBits(0,x2r,newbits2,x2q*8,y);
		    l[x2q] |= newbits2;
		}
	    }
	}
	result.resize(pnt);
    }

    int pnt;
    QPointArray result;
    QCanvas* canvas;
    QRect bounds;
    QImage bitmap;
};


QPointArray QCanvasPolygonalItem::chunks() const
{
    QPointArray pa = areaPoints();

    if ( !pa.size() )
	return pa;

    QPolygonalProcessor processor(canvas(),pa);

    scanPolygon(pa, 0, processor);

    return processor.result;
}

QRect QCanvasPolygonalItem::boundingRect() const
{
    return areaPoints().boundingRect();
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
  \class QCanvasPolygon qcanvas.h
  \brief A polygon with a movable reference point.

  Paints a polygon in a QBrush.
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
    addToChunks();
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
QCanvasText::QCanvasText(const QString& t) :
    text(t), flags(0)
{
    setRect();
}

/*!
  Construct a QCanvasText with the text \a t and font \a f.

  The text should not contain newlines.
*/
QCanvasText::QCanvasText(const QString& t, QFont f) :
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
void QCanvasText::setText( const QString& t )
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
    QCanvasItem* item;
    // Find an item, eg. with QCanvasItem::collisions().
    ...
    if (item->rtti() == MySprite::rtti()) {
        MySprite* s = (MySprite*)item;
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
\fn QCanvasSprite::QCanvasPositionedSprite(QCanvasPixmapArray* seq)

Create a QCanvasSprite which uses images from the given array.

The sprite in initially at (0,0) on the current canvas
(see constructor for QCanvas), using the 0th array frame.
*/
QCanvasSprite::QCanvasSprite(QCanvasPixmapArray* seq) :
    frm(0),
    images(seq)
{
    show();
    addToChunks();
}

/*!
\fn QCanvasSprite::QCanvasPositionedSprite()

Create a QCanvasSprite without defining its image array.

The sprite in initially at (0,0) on the current canvas
(see constructor for QCanvas), using the 0th array frame.

Note that you must call setSequence(QCanvasPixmapArray*) before
doing anything else with the sprite.
*/
QCanvasSprite::QCanvasSprite() :
    frm(0),
    images(0)
{
}

/*!
\fn void QCanvasSprite::setSequence(QCanvasPixmapArray* seq)

Set the array of images used for displaying the sprite.  Note that
the array should have enough images for the sprites current frame()
to be valid.
*/
void QCanvasSprite::setSequence(QCanvasPixmapArray* seq)
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
\fn void QCanvasSprite::frame(int f)

Set the animation frame used for displaying the sprite to
the given index into the QCanvasSprite's QCanvasPixmapArray.

\sa move(double,double,int)
*/
void QCanvasSprite::frame(int f)
{
    move(x(),y(),f);
}

/*!
\fn int QCanvasSprite::frame() const
Returns the index into the QCanvasSprite's QCanvasPixmapArray
of the current animation frame.

\sa move(double,double,int)
*/

/*!
\fn int QCanvasSprite::frameCount() const
Returns the number of frames in the QCanvasSprite's QCanvasPixmapArray.
*/


/*!
  \reimp
  \internal
  Keep it visible.
*/
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


// Based on Xserver code miFillGeneralPoly...
/*
 *
 *     Written by Brian Kelleher;  Oct. 1985
 *
 *     Routine to fill a polygon.  Two fill rules are
 *     supported: frWINDING and frEVENODD.
 *
 *     See fillpoly.h for a complete description of the algorithm.
 */

/*
 *     These are the data structures needed to scan
 *     convert regions.  Two different scan conversion
 *     methods are available -- the even-odd method, and
 *     the winding number method.
 *     The even-odd rule states that a point is inside
 *     the polygon if a ray drawn from that point in any
 *     direction will pass through an odd number of
 *     path segments.
 *     By the winding number rule, a point is decided
 *     to be inside the polygon if a ray drawn from that
 *     point in any direction passes through a different
 *     number of clockwise and counter-clockwise path
 *     segments.
 *
 *     These data structures are adapted somewhat from
 *     the algorithm in (Foley/Van Dam) for scan converting
 *     polygons.
 *     The basic algorithm is to start at the top (smallest y)
 *     of the polygon, stepping down to the bottom of
 *     the polygon by incrementing the y coordinate.  We
 *     keep a list of edges which the current scanline crosses,
 *     sorted by x.  This list is called the Active Edge Table (AET)
 *     As we change the y-coordinate, we update each entry in
 *     in the active edge table to reflect the edges new xcoord.
 *     This list must be sorted at each scanline in case
 *     two edges intersect.
 *     We also keep a data structure known as the Edge Table (ET),
 *     which keeps track of all the edges which the current
 *     scanline has not yet reached.  The ET is basically a
 *     list of ScanLineList structures containing a list of
 *     edges which are entered at a given scanline.  There is one
 *     ScanLineList per scanline at which an edge is entered.
 *     When we enter a new edge, we move it from the ET to the AET.
 *
 *     From the AET, we can implement the even-odd rule as in
 *     (Foley/Van Dam).
 *     The winding number rule is a little trickier.  We also
 *     keep the EdgeTableEntries in the AET linked by the
 *     nextWETE (winding EdgeTableEntry) link.  This allows
 *     the edges to be linked just as before for updating
 *     purposes, but only uses the edges linked by the nextWETE
 *     link as edges representing spans of the polygon to
 *     drawn (as with the even-odd rule).
 */

/*
 * for the winding number rule
 */
#define CLOCKWISE          1
#define COUNTERCLOCKWISE  -1

/* $XConsortium: miscanfill.h,v 1.5 94/04/17 20:27:50 dpw Exp $ */
/*

Copyright (c) 1987  X Consortium

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from the X Consortium.

*/


/*
 *     scanfill.h
 *
 *     Written by Brian Kelleher; Jan 1985
 *
 *     This file contains a few macros to help track
 *     the edge of a filled object.  The object is assumed
 *     to be filled in scanline order, and thus the
 *     algorithm used is an extension of Bresenham's line
 *     drawing algorithm which assumes that y is always the
 *     major axis.
 *     Since these pieces of code are the same for any filled shape,
 *     it is more convenient to gather the library in one
 *     place, but since these pieces of code are also in
 *     the inner loops of output primitives, procedure call
 *     overhead is out of the question.
 *     See the author for a derivation if needed.
 */

/*
 *  In scan converting polygons, we want to choose those pixels
 *  which are inside the polygon.  Thus, we add .5 to the starting
 *  x coordinate for both left and right edges.  Now we choose the
 *  first pixel which is inside the pgon for the left edge and the
 *  first pixel which is outside the pgon for the right edge.
 *  Draw the left pixel, but not the right.
 *
 *  How to add .5 to the starting x coordinate:
 *      If the edge is moving to the right, then subtract dy from the
 *  error term from the general form of the algorithm.
 *      If the edge is moving to the left, then add dy to the error term.
 *
 *  The reason for the difference between edges moving to the left
 *  and edges moving to the right is simple:  If an edge is moving
 *  to the right, then we want the algorithm to flip immediately.
 *  If it is moving to the left, then we don't want it to flip until
 *  we traverse an entire pixel.
 */
#define BRESINITPGON(dy, x1, x2, xStart, d, m, m1, incr1, incr2) { \
    int dx;      /* local storage */ \
\
    /* \
     *  if the edge is horizontal, then it is ignored \
     *  and assumed not to be processed.  Otherwise, do this stuff. \
     */ \
    if ((dy) != 0) { \
        xStart = (x1); \
        dx = (x2) - xStart; \
        if (dx < 0) { \
            m = dx / (dy); \
            m1 = m - 1; \
            incr1 = -2 * dx + 2 * (dy) * m1; \
            incr2 = -2 * dx + 2 * (dy) * m; \
            d = 2 * m * (dy) - 2 * dx - 2 * (dy); \
        } else { \
            m = dx / (dy); \
            m1 = m + 1; \
            incr1 = 2 * dx - 2 * (dy) * m1; \
            incr2 = 2 * dx - 2 * (dy) * m; \
            d = -2 * m * (dy) + 2 * dx; \
        } \
    } \
}

#define BRESINCRPGON(d, minval, m, m1, incr1, incr2) { \
    if (m1 > 0) { \
        if (d > 0) { \
            minval += m1; \
            d += incr1; \
        } \
        else { \
            minval += m; \
            d += incr2; \
        } \
    } else {\
        if (d >= 0) { \
            minval += m1; \
            d += incr1; \
        } \
        else { \
            minval += m; \
            d += incr2; \
        } \
    } \
}


/*
 *     This structure contains all of the information needed
 *     to run the bresenham algorithm.
 *     The variables may be hardcoded into the declarations
 *     instead of using this structure to make use of
 *     register declarations.
 */
typedef struct {
    int minor;         /* minor axis        */
    int d;           /* decision variable */
    int m, m1;       /* slope and slope+1 */
    int incr1, incr2; /* error increments */
} BRESINFO;


#define BRESINITPGONSTRUCT(dmaj, min1, min2, bres) \
	BRESINITPGON(dmaj, min1, min2, bres.minor, bres.d, \
                     bres.m, bres.m1, bres.incr1, bres.incr2)

#define BRESINCRPGONSTRUCT(bres) \
        BRESINCRPGON(bres.d, bres.minor, bres.m, bres.m1, bres.incr1, bres.incr2)


typedef struct _EdgeTableEntry {
     int ymax;             /* ycoord at which we exit this edge. */
     BRESINFO bres;        /* Bresenham info to run the edge     */
     struct _EdgeTableEntry *next;       /* next in the list     */
     struct _EdgeTableEntry *back;       /* for insertion sort   */
     struct _EdgeTableEntry *nextWETE;   /* for winding num rule */
     int ClockWise;        /* flag for winding number rule       */
} EdgeTableEntry;


typedef struct _ScanLineList{
     int scanline;              /* the scanline represented */
     EdgeTableEntry *edgelist;  /* header node              */
     struct _ScanLineList *next;  /* next in the list       */
} ScanLineList;


typedef struct {
     int ymax;                 /* ymax for the polygon     */
     int ymin;                 /* ymin for the polygon     */
     ScanLineList scanlines;   /* header node              */
} EdgeTable;


/*
 * Here is a struct to help with storage allocation
 * so we can allocate a big chunk at a time, and then take
 * pieces from this heap when we need to.
 */
#define SLLSPERBLOCK 25

typedef struct _ScanLineListBlock {
     ScanLineList SLLs[SLLSPERBLOCK];
     struct _ScanLineListBlock *next;
} ScanLineListBlock;

/*
 * number of points to buffer before sending them off
 * to scanlines() :  Must be an even number
 */
#define NUMPTSTOBUFFER 200

/*
 *
 *     a few macros for the inner loops of the fill code where
 *     performance considerations don't allow a procedure call.
 *
 *     Evaluate the given edge at the given scanline.
 *     If the edge has expired, then we leave it and fix up
 *     the active edge table; otherwise, we increment the
 *     x value to be ready for the next scanline.
 *     The winding number rule is in effect, so we must notify
 *     the caller when the edge has been removed so he
 *     can reorder the Winding Active Edge Table.
 */
#define EVALUATEEDGEWINDING(pAET, pPrevAET, y, fixWAET) { \
   if (pAET->ymax == y) {          /* leaving this edge */ \
      pPrevAET->next = pAET->next; \
      pAET = pPrevAET->next; \
      fixWAET = 1; \
      if (pAET) \
         pAET->back = pPrevAET; \
   } \
   else { \
      BRESINCRPGONSTRUCT(pAET->bres); \
      pPrevAET = pAET; \
      pAET = pAET->next; \
   } \
}


/*
 *     Evaluate the given edge at the given scanline.
 *     If the edge has expired, then we leave it and fix up
 *     the active edge table; otherwise, we increment the
 *     x value to be ready for the next scanline.
 *     The even-odd rule is in effect.
 */
#define EVALUATEEDGEEVENODD(pAET, pPrevAET, y) { \
   if (pAET->ymax == y) {          /* leaving this edge */ \
      pPrevAET->next = pAET->next; \
      pAET = pPrevAET->next; \
      if (pAET) \
         pAET->back = pPrevAET; \
   } \
   else { \
      BRESINCRPGONSTRUCT(pAET->bres); \
      pPrevAET = pAET; \
      pAET = pAET->next; \
   } \
}

/***********************************************************

Copyright (c) 1987  X Consortium

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.


Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the name of Digital not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

#define MAXINT 0x7fffffff
#define MININT -MAXINT

/*
 *     fillUtils.c
 *
 *     Written by Brian Kelleher;  Oct. 1985
 *
 *     This module contains all of the utility functions
 *     needed to scan convert a polygon.
 *
 */
/*
 *     InsertEdgeInET
 *
 *     Insert the given edge into the edge table.
 *     First we must find the correct bucket in the
 *     Edge table, then find the right slot in the
 *     bucket.  Finally, we can insert it.
 *
 */
bool
miInsertEdgeInET(EdgeTable *ET, EdgeTableEntry *ETE,
	int scanline, ScanLineListBlock **SLLBlock, int *iSLLBlock)
{
    register EdgeTableEntry *start, *prev;
    register ScanLineList *pSLL, *pPrevSLL;
    ScanLineListBlock *tmpSLLBlock;

    /*
     * find the right bucket to put the edge into
     */
    pPrevSLL = &ET->scanlines;
    pSLL = pPrevSLL->next;
    while (pSLL && (pSLL->scanline < scanline))
    {
        pPrevSLL = pSLL;
        pSLL = pSLL->next;
    }

    /*
     * reassign pSLL (pointer to ScanLineList) if necessary
     */
    if ((!pSLL) || (pSLL->scanline > scanline))
    {
        if (*iSLLBlock > SLLSPERBLOCK-1)
        {
            tmpSLLBlock =
		  (ScanLineListBlock *)malloc(sizeof(ScanLineListBlock));
	    if (!tmpSLLBlock)
		return FALSE;
            (*SLLBlock)->next = tmpSLLBlock;
            tmpSLLBlock->next = (ScanLineListBlock *)NULL;
            *SLLBlock = tmpSLLBlock;
            *iSLLBlock = 0;
        }
        pSLL = &((*SLLBlock)->SLLs[(*iSLLBlock)++]);

        pSLL->next = pPrevSLL->next;
        pSLL->edgelist = (EdgeTableEntry *)NULL;
        pPrevSLL->next = pSLL;
    }
    pSLL->scanline = scanline;

    /*
     * now insert the edge in the right bucket
     */
    prev = (EdgeTableEntry *)NULL;
    start = pSLL->edgelist;
    while (start && (start->bres.minor < ETE->bres.minor))
    {
        prev = start;
        start = start->next;
    }
    ETE->next = start;

    if (prev)
        prev->next = ETE;
    else
        pSLL->edgelist = ETE;
    return TRUE;
}

/*
 *     CreateEdgeTable
 *
 *     This routine creates the edge table for
 *     scan converting polygons.
 *     The Edge Table (ET) looks like:
 *
 *    EdgeTable
 *     --------
 *    |  ymax  |        ScanLineLists
 *    |scanline|-->------------>-------------->...
 *     --------   |scanline|   |scanline|
 *                |edgelist|   |edgelist|
 *                ---------    ---------
 *                    |             |
 *                    |             |
 *                    V             V
 *              list of ETEs   list of ETEs
 *
 *     where ETE is an EdgeTableEntry data structure,
 *     and there is one ScanLineList per scanline at
 *     which an edge is initially entered.
 *
 */

typedef struct {
    int x, y;
} DDXPointRec, *DDXPointPtr;

/*
 *     Clean up our act.
 */
void
miFreeStorage(ScanLineListBlock   *pSLLBlock)
{
    register ScanLineListBlock   *tmpSLLBlock;

    while (pSLLBlock)
    {
        tmpSLLBlock = pSLLBlock->next;
        free(pSLLBlock);
        pSLLBlock = tmpSLLBlock;
    }
}

bool
miCreateETandAET(int count, DDXPointPtr pts, EdgeTable *ET,
	EdgeTableEntry *AET, EdgeTableEntry *pETEs, ScanLineListBlock *pSLLBlock)
{
    register DDXPointPtr top, bottom;
    register DDXPointPtr PrevPt, CurrPt;
    int iSLLBlock = 0;

    int dy;

    if (count < 2)  return TRUE;

    /*
     *  initialize the Active Edge Table
     */
    AET->next = (EdgeTableEntry *)NULL;
    AET->back = (EdgeTableEntry *)NULL;
    AET->nextWETE = (EdgeTableEntry *)NULL;
    AET->bres.minor = MININT;

    /*
     *  initialize the Edge Table.
     */
    ET->scanlines.next = (ScanLineList *)NULL;
    ET->ymax = MININT;
    ET->ymin = MAXINT;
    pSLLBlock->next = (ScanLineListBlock *)NULL;

    PrevPt = &pts[count-1];

    /*
     *  for each vertex in the array of points.
     *  In this loop we are dealing with two vertices at
     *  a time -- these make up one edge of the polygon.
     */
    while (count--)
    {
        CurrPt = pts++;

        /*
         *  find out which point is above and which is below.
         */
        if (PrevPt->y > CurrPt->y)
        {
            bottom = PrevPt, top = CurrPt;
            pETEs->ClockWise = 0;
        }
        else
        {
            bottom = CurrPt, top = PrevPt;
            pETEs->ClockWise = 1;
        }

        /*
         * don't add horizontal edges to the Edge table.
         */
        if (bottom->y != top->y)
        {
            pETEs->ymax = bottom->y-1;  /* -1 so we don't get last scanline */

            /*
             *  initialize integer edge algorithm
             */
            dy = bottom->y - top->y;
            BRESINITPGONSTRUCT(dy, top->x, bottom->x, pETEs->bres);

            if (!miInsertEdgeInET(ET, pETEs, top->y, &pSLLBlock, &iSLLBlock))
	    {
		miFreeStorage(pSLLBlock->next);
		return FALSE;
	    }

            ET->ymax = QMAX(ET->ymax, PrevPt->y);
            ET->ymin = QMIN(ET->ymin, PrevPt->y);
            pETEs++;
        }

        PrevPt = CurrPt;
    }
    return TRUE;
}

/*
 *     loadAET
 *
 *     This routine moves EdgeTableEntries from the
 *     EdgeTable into the Active Edge Table,
 *     leaving them sorted by smaller x coordinate.
 *
 */

void
miloadAET(EdgeTableEntry *AET, EdgeTableEntry *ETEs)
{
    register EdgeTableEntry *pPrevAET;
    register EdgeTableEntry *tmp;

    pPrevAET = AET;
    AET = AET->next;
    while (ETEs)
    {
        while (AET && (AET->bres.minor < ETEs->bres.minor))
        {
            pPrevAET = AET;
            AET = AET->next;
        }
        tmp = ETEs->next;
        ETEs->next = AET;
        if (AET)
            AET->back = ETEs;
        ETEs->back = pPrevAET;
        pPrevAET->next = ETEs;
        pPrevAET = ETEs;

        ETEs = tmp;
    }
}

/*
 *     computeWAET
 *
 *     This routine links the AET by the
 *     nextWETE (winding EdgeTableEntry) link for
 *     use by the winding number rule.  The final
 *     Active Edge Table (AET) might look something
 *     like:
 *
 *     AET
 *     ----------  ---------   ---------
 *     |ymax    |  |ymax    |  |ymax    |
 *     | ...    |  |...     |  |...     |
 *     |next    |->|next    |->|next    |->...
 *     |nextWETE|  |nextWETE|  |nextWETE|
 *     ---------   ---------   ^--------
 *         |                   |       |
 *         V------------------->       V---> ...
 *
 */
void
micomputeWAET(EdgeTableEntry *AET)
{
    register EdgeTableEntry *pWETE;
    register int inside = 1;
    register int isInside = 0;

    AET->nextWETE = (EdgeTableEntry *)NULL;
    pWETE = AET;
    AET = AET->next;
    while (AET)
    {
        if (AET->ClockWise)
            isInside++;
        else
            isInside--;

        if ((!inside && !isInside) ||
            ( inside &&  isInside))
        {
            pWETE->nextWETE = AET;
            pWETE = AET;
            inside = !inside;
        }
        AET = AET->next;
    }
    pWETE->nextWETE = (EdgeTableEntry *)NULL;
}

/*
 *     InsertionSort
 *
 *     Just a simple insertion sort using
 *     pointers and back pointers to sort the Active
 *     Edge Table.
 *
 */

int
miInsertionSort(EdgeTableEntry *AET)
{
    register EdgeTableEntry *pETEchase;
    register EdgeTableEntry *pETEinsert;
    register EdgeTableEntry *pETEchaseBackTMP;
    register int changed = 0;

    AET = AET->next;
    while (AET)
    {
        pETEinsert = AET;
        pETEchase = AET;
        while (pETEchase->back->bres.minor > AET->bres.minor)
            pETEchase = pETEchase->back;

        AET = AET->next;
        if (pETEchase != pETEinsert)
        {
            pETEchaseBackTMP = pETEchase->back;
            pETEinsert->back->next = AET;
            if (AET)
                AET->back = pETEinsert->back;
            pETEinsert->next = pETEchase;
            pETEchase->back->next = pETEinsert;
            pETEchase->back = pETEinsert;
            pETEinsert->back = pETEchaseBackTMP;
            changed = 1;
        }
    }
    return(changed);
}

void QCanvasPolygonalItem::scanPolygon(const QPointArray& pa, int winding, QPolygonalProcessor& process) const
{
    DDXPointPtr ptsIn = (DDXPointPtr)pa.data();
    register EdgeTableEntry *pAET;  /* the Active Edge Table   */
    register int y;                 /* the current scanline    */
    register int nPts = 0;          /* number of pts in buffer */
    register EdgeTableEntry *pWETE; /* Winding Edge Table      */
    register ScanLineList *pSLL;    /* Current ScanLineList    */
    register DDXPointPtr ptsOut;      /* ptr to output buffers   */
    int *width;
    DDXPointRec FirstPoint[NUMPTSTOBUFFER]; /* the output buffers */
    int FirstWidth[NUMPTSTOBUFFER];
    EdgeTableEntry *pPrevAET;       /* previous AET entry      */
    EdgeTable ET;                   /* Edge Table header node  */
    EdgeTableEntry AET;             /* Active ET header node   */
    EdgeTableEntry *pETEs;          /* Edge Table Entries buff */
    ScanLineListBlock SLLBlock;     /* header for ScanLineList */
    int fixWAET = 0;

    if (pa.size() < 3)
	return;

    if(!(pETEs = (EdgeTableEntry *)
        malloc(sizeof(EdgeTableEntry) * pa.size())))
	return;
    ptsOut = FirstPoint;
    width = FirstWidth;
    if (!miCreateETandAET(pa.size(), ptsIn, &ET, &AET, pETEs, &SLLBlock))
    {
	free(pETEs);
	return;
    }
    pSLL = ET.scanlines.next;

    if (winding==0)
    {
        /*
         *  for each scanline
         */
        for (y = ET.ymin; y < ET.ymax; y++)
        {
            /*
             *  Add a new edge to the active edge table when we
             *  get to the next edge.
             */
            if (pSLL && y == pSLL->scanline)
            {
                miloadAET(&AET, pSLL->edgelist);
                pSLL = pSLL->next;
            }
            pPrevAET = &AET;
            pAET = AET.next;

            /*
             *  for each active edge
             */
            while (pAET)
            {
                ptsOut->x = pAET->bres.minor;
		ptsOut++->y = y;
                *width++ = pAET->next->bres.minor - pAET->bres.minor;
                nPts++;

                /*
                 *  send out the buffer when its full
                 */
                if (nPts == NUMPTSTOBUFFER)
		{
		    process.doSpans( nPts, (QPoint*)FirstPoint, FirstWidth );
                    ptsOut = FirstPoint;
                    width = FirstWidth;
                    nPts = 0;
                }
                EVALUATEEDGEEVENODD(pAET, pPrevAET, y)
                EVALUATEEDGEEVENODD(pAET, pPrevAET, y);
            }
            miInsertionSort(&AET);
        }
    }
    else      /* default to WindingNumber */
    {
        /*
         *  for each scanline
         */
        for (y = ET.ymin; y < ET.ymax; y++)
        {
            /*
             *  Add a new edge to the active edge table when we
             *  get to the next edge.
             */
            if (pSLL && y == pSLL->scanline)
            {
                miloadAET(&AET, pSLL->edgelist);
                micomputeWAET(&AET);
                pSLL = pSLL->next;
            }
            pPrevAET = &AET;
            pAET = AET.next;
            pWETE = pAET;

            /*
             *  for each active edge
             */
            while (pAET)
            {
                /*
                 *  if the next edge in the active edge table is
                 *  also the next edge in the winding active edge
                 *  table.
                 */
                if (pWETE == pAET)
                {
                    ptsOut->x = pAET->bres.minor;
		    ptsOut++->y = y;
                    *width++ = pAET->nextWETE->bres.minor - pAET->bres.minor;
                    nPts++;

                    /*
                     *  send out the buffer
                     */
                    if (nPts == NUMPTSTOBUFFER)
                    {
			process.doSpans( nPts, (QPoint*)FirstPoint, FirstWidth );
                        ptsOut = FirstPoint;
                        width  = FirstWidth;
                        nPts = 0;
                    }

                    pWETE = pWETE->nextWETE;
                    while (pWETE != pAET)
                        EVALUATEEDGEWINDING(pAET, pPrevAET, y, fixWAET);
                    pWETE = pWETE->nextWETE;
                }
                EVALUATEEDGEWINDING(pAET, pPrevAET, y, fixWAET);
            }

            /*
             *  reevaluate the Winding active edge table if we
             *  just had to resort it or if we just exited an edge.
             */
            if (miInsertionSort(&AET) || fixWAET)
            {
                micomputeWAET(&AET);
                fixWAET = 0;
            }
        }
    }

    /*
     *     Get any spans that we missed by buffering
     */
    process.doSpans( nPts, (QPoint*)FirstPoint, FirstWidth );
    free(pETEs);
    miFreeStorage(SLLBlock.next);
}
/***** END OF X11-based CODE *****/


