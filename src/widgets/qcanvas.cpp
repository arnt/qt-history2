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

class QCanvasItemPtrList : public QValueList<QCanvasItem*> {
public:
    void sort()
    {
	qHeapSort(*((QValueList<QCanvasItemPtr>*)this));
    }
    void drawUnique( QPainter& painter )
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
};

class QCanvasChunk {
public:
    QCanvasChunk() : changed(TRUE) { }
    // Other code assumes lists are not deleted.  Assignment is also
    // done on ChunkRecs.  So don't add that sort of thing here.

    void sort()
    {
	((QCanvasItemPtrList&)list).sort();
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




/*!
\class QCanvas qcanvas.h
\brief A QCanvas is a 2D graphic area upon which QCanvasItem objects are drawn.

The QCanvas and related classes (primarily QCanvasView and
QCanvasSprite, but also the other sprite abstractions)
provide for multiple concurrent views of a 2D area
containing moving graphical objects.

The QCanvas is also an indexing mechanism to the sprites it contains,
providing 2D-area-based iteration and pixelwise collision detection.

Most of the methods in QCanvas are mainly used by the existing
QCanvasItem classes, so will only be interesting if you intend
to add a new type of QCanvasItem class.  The methods of primary
interest to the typical user are, in approximate
order of decreasing usefulness:

<ul>
      <li> \link QCanvas QCanvas \endlink (int w, int h, int chunksize, int maxclusters)
      <li> void \link update update \endlink ()
      <li> int \link width width \endlink () const
      <li> int \link height height \endlink () const
      <li> QCanvasIteratorPrivate* \link topAt topAt \endlink (int x, int y)
      <li> QCanvasIteratorPrivate* \link lookIn lookIn \endlink (int x, int y, int w, int h)
      <li> QCanvasItem* \link at at \endlink (QCanvasIteratorPrivate*) const
      <li> bool \link exact exact \endlink (QCanvasIteratorPrivate*) const
      <li> void \link next next \endlink (QCanvasIteratorPrivate*&)
      <li> void \link end end \endlink (QCanvasIteratorPrivate*&)
      <li> void \link protectFromChange protectFromChange \endlink (QCanvasIteratorPrivate*)
      <li> virtual void \link drawBackground drawBackground \endlink (QPainter&, const QRect& area)
      <li> virtual void \link drawForeground drawForeground \endlink (QPainter&, const QRect& area)
</ul>

This class provides for very high redraw efficiency.  The properties
of the mechanism are:

<ul>
    <li>There is no size or number limitation on sprites (except memory).
    <li>QCanvasSprites are small in memory size.
    <li>Collision detection is efficient.
    <li>Finding sprites in an area is efficient.
    <li>Only moving sprites are redrawn.
    <li>The number of unmoving sprites on the field has little effect.
    <li>Redraws can be batched (see \link update update\endlink)
    <li>Only visible areas (those in the visible part of a QCanvasView) are redrawn.
</ul>

For example, it is quite feasible to have \e thousands of sprites used to
create background scenery, and even animate pieces of that scenery occasionally.
It is less feasible to have views where the entire scene is in continous
motion, such as a continously scrolling background.  You will be suprised
how little animation is required to make a scene look `alive'.

The various sprite abstractions are:

<ul>
    <dt><b>QCanvasSprite</b>
	<dd>A item with variable image and position.
</ul>

For the purpose of simplified description, we use the term sprite to refer
to all these and derived classes.
*/

/*!
Returns the chunk at a chunk position.
*/
QCanvasChunk& QCanvas::chunk(int i, int j) const
{ return chunks[i+chwidth*j]; }

/*!
Returns the chunk at a pixel position.
*/
QCanvasChunk& QCanvas::chunkContaining(int x, int y) const
{ return chunk(x/chunksize,y/chunksize); }

/*!
Returns the sorted list of chunks at the given chunk.  You should
not need this method - it is used internally for collision detection.
\sa QCanvas::topAt(int,int)
*/
const QCanvasItemList* QCanvas::listAtChunkTopFirst(int i, int j) const
{
    if (i>=0 && i<chwidth && j>=0 && j<chheight) {
	chunk(i,j).sort();
	return chunk(i,j).listPtr();
    } else {
	return 0;
    }
}

/*!
Returns the sorted list of all items.  You should
not need this method - it is used internally for iteration.
\sa QCanvas::all()
*/
QCanvasItemList* QCanvas::allList()
{
    QCanvasItemList* list=new QCanvasItemList;
    for (QPtrDictIterator<void> it=itemDict; it.currentKey(); ++it) {
	list->prepend((QCanvasItem*)it.currentKey());
    }
    return list;
}

/*!
\fn void QCanvas::setPositionPrecision(int downshifts)

The precision of QCanvasItem positions (and collisions - see sprite) can
be set.  A positive precision means world co-ordinates will be
bit-shifted that many places higher.  So, if the precision is 1,
world co-ordinate (50,100) is the pixel co-ordinate (25,50); for
precision 3, (50,100) is pixel (6,12).  Negative positions are
also allowed, and they imply that world-coordinates are \e lower
resolution than pixel co-ordinates. IF ANYONE FINDS A USE FOR NEGATIVES,
LET ME KNOW, OR I MIGHT REMOVE IT.

By default, pixel and world coordinates are the same.

*/

/*!
\fn int QCanvas::positionPrecision()
Returns the position precision for all coordinates in the QCanvas.
\sa setPositionPrecision(int)
*/

/*!
\fn int QCanvas::world_to_x(int i)
Convert a coordinate from world to pixel coordinates.
\sa setPositionPrecision(int)
*/

/*!
\fn int QCanvas::x_to_world(int i)
Convert a coordinate from pixel to world coordinates.
\sa setPositionPrecision(int)
*/

/*!
Constructs a QCanvas with size \c w wide and \c h high, measured
in world coordinates (by default, world coordinates equal pixel coordinates).

The values \c maxclusters and \c chunksize effect the grouping heuristics
used when redrawing the QCanvas.

   \c chunksize is the size of square chunk used to break up the
     QCanvas into area to be considered for redrawing.  It
     should be about the average size of items in the QCanvas.
     Chunks too small increase the amount of calculation required
     when drawing.  Chunks too large increase the amount of drawing
     that is needed.

   \c maxclusters is the number of rectangular groups of chunks that
     will be separately drawn.  If the QCanvas has a large number
     of small, dispersed items, this should be about that number.
     The more clusters the slower the redraw, but also the bigger
     clusters are the slower the redraw, so a balance is needed.
     Testing reveals that a large number of clusters is almost
     always best.
*/
QCanvas::QCanvas(int w, int h, int chunksze, int maxclust)
{
    init(w,h,chunksze,maxclust);
}

void QCanvas::init(int w, int h, int chunksze, int maxclust)
{
    awidth=w;
    aheight=h;
    chunksize=chunksze;
    maxclusters=maxclust;
    chwidth=(w+chunksize-1)/chunksize;
    chheight=(h+chunksize-1)/chunksize;
    chunks=new QCanvasChunk[chwidth*chheight];
    QCanvasItem::setCurrentCanvas(this);
    update_timer = 0;
}

/*!
Create a QCanvas with no size, and default chunksize/maxclusters.
You will want to call resize(int,int) at some time after creation.
*/
QCanvas::QCanvas() :
    awidth(0),aheight(0),
    chunksize(16),
    maxclusters(100),
    chwidth(0),
    chheight(0),
    chunks(new QCanvasChunk[1])
{
    QCanvasItem::setCurrentCanvas(this);
}

/*!
Destruct the field.  Does nothing.  Does \e not destroy QCanvasItem objects in
the field.
*/
QCanvas::~QCanvas()
{
}

/*!
Change the size of the QCanvas. This is a slow operation.
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

    for (QCanvasView* view=viewList.first(); view != 0; view=viewList.next()) {
	view->update(); // XXX do better
    }
}

/*!
Change the efficiency tuning parameters.  This is a slow operation.
*/
void QCanvas::retune(int chunksze, int mxclusters)
{
    QList<QCanvasItem> hidden;
    for (QPtrDictIterator<void> it=itemDict; it.currentKey(); ++it) {
	if (((QCanvasItem*)it.currentKey())->visible()) {
	    ((QCanvasItem*)it.currentKey())->hide();
	    hidden.append(((QCanvasItem*)it.currentKey()));
	}
    }

    chunksize=chunksze;
    maxclusters=mxclusters;

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
}

/*!
\fn int QCanvas::width() const
Returns the width of the sprite field, in world coordinates.
*/

/*!
\fn int QCanvas::height() const
Returns the height of the sprite field, in world coordinates.
*/

/*!
\fn int QCanvas::chunkSize() const
Returns the chunk size of the sprite field as set at construction.
\sa QCanvas::QCanvas(...)
*/

/*!
\fn bool QCanvas::sameChunk(int x1, int y1, int x2, int y2) const
Tells if the points (x1,y1) and (x2,y2) are within the same chunk.
*/

/*!
This method is called for all updates of the QCanvas.  It renders
any foreground items.  In general, it is more efficient to use additional
QCanvasItem objects on the QCanvas rather than adding rendering
at this point, as this method is <em>always</em> called, whereas
QCanvasItem objects are only redrawn if they are in the area of change.

The same warnings regarding change apply to this method
as given in drawBackground(QPainter&, const QRect&).

The default is to do nothing.

\sa forceRedraw(QRect&)
*/
void QCanvas::drawForeground(QPainter&, const QRect&)
{
}

/*!
Forces the given area to be redrawn.  This is useful when the foreground
or background are changed.
*/
void QCanvas::forceRedraw(const QRect& area)
{
    drawArea(area);
}

/*!
(internal)
This method adds an element to the list of QCanvasItem objects
in this QCanvas.  The QCanvasItem class calls this, so you
should not need it.
*/
void QCanvas::addItem(QCanvasItem* item)
{
    itemDict.insert(item,(void*)1);
}

void QCanvas::addAnimation(QCanvasItem* item)
{
    animDict.insert(item,(void*)1);
}

/*!
(internal)
This method removes an element from the list of QCanvasItem objects
in this QCanvas.  The QCanvasItem class calls this, so you
should not need it.
*/
void QCanvas::removeItem(QCanvasItem* item)
{
    itemDict.remove(item);
}

/*!
\internal
This method adds an element to the list of QCanvasView objects
viewing this QCanvas.  The QCanvasView class calls this, so you
should not need it.
*/
void QCanvas::addView(QCanvasView* view)
{
    viewList.append(view);
}

/*!
\internal
This method removes an element from the list of QCanvasView objects
viewing this QCanvas.  The QCanvasView class calls this, so you
should not need it.
*/
void QCanvas::removeView(QCanvasView* view)
{
    viewList.removeRef(view);
}

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

void QCanvas::advance()
{
    for (QPtrDictIterator<void> it=animDict; it.current(); ) {
	QCanvasItem* i = (QCanvasItem*)it.currentKey();
	++it;
	if ( i )
	    i->forward();
    }
    update();
}

/*!
This method causes all QCanvasView objects currently viewing this
QCanvas to be refreshed in all areas of the QCanvas which have
changed since the last call to this method.

In a continuously animated QCanvas, this method should be called at
a regular rate.  The best way to accomplish this is to use the QTimer
class of the Qt toolkit, or the startTimer method of QObject.  It would
not be useful to call this method excessively (eg. whenever a sprite
is moved) as the efficiency of the system is derived from a clustering
and bulk-update mechanism.

Depending on how you use QCanvas and on the hardware, you may get
improved smoothness by calling \link QApplication::syncX qApp->syncX()\endlink
after this method.
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
\internal
Redraw a given area of the QCanvas.

If only_changes then only changes to the area are redrawn.

If one_view then only one view is updated, otherwise all are.
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

    QCanvasItemPtrList allvisible;

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
This method to informs the QCanvas that the chunk containing
a given pixel is `dirty' and needs to be redrawn in the next Update.

(x,y) is a pixel location.

The sprite classes call this.  Any new derived class
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

/*
(internal)
A QCanvasIteratorPrivate is used for the actual implementation of
the QCanvasIteratorPrivate* iterator.  It stores all the information needed to
iterate over the various queries.

\sa QCanvas::topAt(int,int)
*/
class QCanvasIteratorPrivate {
private:
    QCanvasIteratorPrivate(int ax, int ay, int aw, int ah, QCanvas* canv, QImage* coll_mask) :
	x(ax),y(ay),
	w(aw),h(ah),
	collision_mask(coll_mask),
	i(x/canv->chunkSize()),
	j(y/canv->chunkSize()),
	x1(i), y1(j),
	x2((x+w-1)/canv->chunkSize()),
	y2((y+h-1)/canv->chunkSize()),
	list(canv->listAtChunkTopFirst(i,j)),
	is_copy(FALSE)
    {
	if ( list )
	    lit = list->begin();
    }

    // Iterates using items list
    QCanvasIteratorPrivate(QCanvas* canv) :
	x(0),y(0),
	w(0),h(0),
	i(1),j(1),
	x1(0),y1(0),
	x2(-1),y2(-1),
	list(canv->allList()),
	is_copy(TRUE)
    {
	if ( list )
	    lit = list->begin();
    }

public:
    static QCanvasIteratorPrivate* make(int ax, int ay, int aw, int ah, QCanvas* canv, QImage* coll_mask)
    {
	QCanvasIteratorPrivate* result=new QCanvasIteratorPrivate(ax,ay,aw,ah,
	    canv, coll_mask);

	if (result->empty()) {
	    result=result->next(canv);
	}

	return result;
    }

    static QCanvasIteratorPrivate* make(int sx, int sy, QCanvasPixmap* im, QCanvas* canv)
    {
	return make( sx-im->hotx, sy-im->hoty, im->colw, im->colh,
		canv, im->collision_mask);
    }

    static QCanvasIteratorPrivate* make(QCanvas* canv)
    {
	return new QCanvasIteratorPrivate(canv);
    }

    ~QCanvasIteratorPrivate()
    {
	if (is_copy) delete list;
    }

    QCanvasIteratorPrivate* next(const QCanvas* canv)
    {
	if (!empty()) {
	    nextElement();
	    if (!empty()) {
		return this;
	    }
	}

	while (empty()) {
	    i++;

	    if (i <= x2) {
		newList((QCanvasItemList*)canv->listAtChunkTopFirst(i,j));
	    } else {
		i=x1; j++;
		if (j <= y2) {
		    newList((QCanvasItemList*)canv->listAtChunkTopFirst(i,j));
		} else {
		    delete this;
		    return 0;
		}
	    }
	}

	return this;
    }

    QCanvasItem* element()
    {
	return *lit;
    }

    void protectFromChange()
    {
	if (!is_copy) {
	    is_copy=TRUE;
	    list=new QCanvasItemList(*list);
	    lit = list->begin();
	}
    }

    const int x, y, w, h;
    QImage* collision_mask;

private:
    void newList(QCanvasItemList* nl)
    {
	if (is_copy) {
	    delete list;
	    list=new QCanvasItemList(*nl);
	} else {
	    list=nl;
	}
	lit = list->begin();
    }

    void nextElement()
    {
	++lit;
    }

    bool empty()
    {
	return !list || lit == list->end();
    }

    int i, j;
    const int x1, y1, x2, y2;

    const QCanvasItemList* list;
    QCanvasItemList::ConstIterator lit;
    bool is_copy; // indicates that list is a copy, not canvas list.
};

/*!
QCanvasItem objects at a <em>point</em> can be traversed from top
to bottom using:

\code
    for (QCanvasIteratorPrivate* p=topAt(x,y); p; next(p)) {
    QCanvasItem* the_item = at(p);
    if (you_are_interested_in_collisions_with(the_item) && exact(p)) {
	    // Collision!
	    ...
	}
    }
\endcode

This traverses <em>at least</em> all the items at pixel position (x,y).

The QCanvasItem returned by at(QCanvasIteratorPrivate*) is very approximate, but is found
very efficiently.  It should be used as a first-cut to ignore
QCanvasItem objects you are not interested in.

exact(QCanvasIteratorPrivate*) should be used to further
examine the QCanvasItem, as depicted above.

During the traversal, the QCanvas must not be modified unless
protectFromChange(QCanvasIteratorPrivate*) is first called on the iterator.  This is for
efficiency reasons (protecting the list of hits requires extra work).

\warning Currently, the collision model is not quite correct at this
    level of abstraction, because it cannot check collision exactness
    beyond the rectangular level.  This will change in futures versions.
    For now, you should perhaps use the QCanvasItem::neighbourhood(...)
    methods instead.

\sa QCanvasItem::rtti()
*/
QCanvasIteratorPrivate* QCanvas::topAt(int x, int y)
{
    QCanvasIteratorPrivate* iterator=QCanvasIteratorPrivate::make(x,y,1,1, this, 0);
    return (QCanvasIteratorPrivate*)iterator;
}

/*!
Provides for traversal of all QCanvasItem objects in the field,
regardless of whether they are visible, on the field, or anything else.
No particular ordering it given.  You should iterate to the end of the
list, or use end(QCanvasIteratorPrivate*&).
*/
QCanvasIteratorPrivate* QCanvas::all()
{
    QCanvasIteratorPrivate* iterator=QCanvasIteratorPrivate::make(this);
    return (QCanvasIteratorPrivate*)iterator;
}

/*!
QCanvasItem objects in an <em>area</em> can be traversed using:

\code
   for (QCanvasIteratorPrivate* p=lookIn(x,y,w,h); p; next(p)) {
    QCanvasItem* the_item = at(p);
    ...
    }
\endcode

This traverses \e at \e least all the items in the given rectangle
\e at \e least once.

\sa topAt(int,int) at(QCanvasIteratorPrivate*)
*/
QCanvasIteratorPrivate* QCanvas::lookIn(int x, int y, int w, int h)
{
    QCanvasIteratorPrivate* iterator=QCanvasIteratorPrivate::make(x,y,w,h, this, 0);
    return (QCanvasIteratorPrivate*)iterator;
}

/*!
This should be called if you want to terminate a traversal without
looking at all results.  It frees memory used for the iteration.
*/
void QCanvas::end(QCanvasIteratorPrivate*& p) const
{
    if (p) {
	delete (QCanvasIteratorPrivate*)p;
	p=0;
    }
}

/*!
Returns the QCanvasItem for a given point in a traversal.

\sa topAt(int,int) lookIn(int,int,int,int)
*/
QCanvasItem* QCanvas::at(QCanvasIteratorPrivate* p) const
{
    if (p) {
	QCanvasIteratorPrivate* iterator=(QCanvasIteratorPrivate*)p;
	return iterator->element();
    } else {
	return 0;
    }
}

/*!
During the traversal, the QCanvas must not be modified.
If you wish to modify it, call this method on the
QCanvasIteratorPrivate*.  This will allow the QCanvas to be subsequently
modified without damaging the list of hits.
*/
void QCanvas::protectFromChange(QCanvasIteratorPrivate* p)
{
    if (p) {
	QCanvasIteratorPrivate* iterator=(QCanvasIteratorPrivate*)p;
	iterator->protectFromChange();
    }
}


unsigned int QCanvas::posprec=0;

/*!
\class QCanvas qcanvas.h
\brief A QCanvas with a background image.

Although it is not very useful to provide a complex background drawing
method for a QCanvas (since redraw efficiency is achieved by sprites,
not static background graphics), if your application requires only a
simple repeating background image, this class will suffice.

This class is also intended as documentation-by-example for the
QCanvas::drawBackground method.
*/

/*!
Construct an QCanvas which uses the given image file
as the background image.  Any Qt-supported image format may be used.
*/
QCanvas::QCanvas(const QString& filename, int w, int h, int chunksize, int maxclusters)
{
    init(w, h, chunksize, maxclusters);
    QPixmap p;
    if ( !p.load(filename))
	qWarning("QCanvas - failed to read %s\n",filename.latin1());
    initTiles(p, 1, 1, p.width(), p.height());

}

/*!
Construct an QCanvas which uses the given image
as the background image.
*/
QCanvas::QCanvas(QPixmap p, int w, int h, int chunksize, int maxclusters)
{
    init(w, h, chunksize, maxclusters);
    pm = p;
}


/*!
This method is called for all updates of the QCanvas.  It renders
any background graphics.  If the canvas has a background pixmap or a tiled
background, that graphics is used,
otherwise it is cleared in the
background colour to the default painter background (white).  You may
also override this method to initialize the QPainter which will be passed
to the draw method of all QCanvasItem objects.

Note that you should not put \e dynamic graphics on this background.
If the graphics for a region change, call forceRedraw(const QRect&).
Such a change should be done only very occasionally, as the redraw
is inefficient.  See the main notes for this class for a discussion
of redraw efficiency.

\sa forceRedraw(QRect&)
*/
void QCanvas::drawBackground(QPainter& painter, const QRect& area)
{
    if ( pm.isNull() ) {
	painter.eraseRect(area);
    } else if ( !grid ) {
	for (int x=area.x()/pm.width();
	    x<(area.x()+area.width()+pm.width()-1)/pm.width(); x++)
	{
	    for (int y=area.y()/pm.height();
		y<(area.y()+area.height()+pm.height()-1)/pm.height(); y++)
	    {
		painter.drawPixmap(x*pm.width(), y*pm.height(),pm);
	    }
	}
    } else {
	const int x1 = area.left()/tilew;
	int x2 = area.right()/tilew;
	const int y1 = area.top()/tileh;
	int y2 = area.bottom()/tileh;

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
  \class QCanvas qcanvas.h
  \brief A canvas with a background composed of rectangular tiles.

  The background drawn by QCanvas is a matrix of
  tiles, where a tile is an index into a single source pixmap.
*/

static int gcd(int a, int b)
{
    // XXX Could use good method, but not speed critical.

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
  Constructs a QCanvas which will be composed of
  \a h tiles horizontally and \a v tiles vertically.  Each tile
  will be an image \a tilewidth by \a tileheight pixels from
  pixmap \a p.

  The pixmap \a p is a list of tiles, arranged left to right,
  top to bottom, with tile 0 in the top-left corner, tile 1 next
  to the right, and so on.

  The QCanvas is initially sized to show exactly the given number
  of tiles horizontally and vertically.  If it is resized to be larger,
  the extra area has no tiles drawn on it.  If it is smaller, tiles to
  the right and bottom will not be visible.

  The chunksize is the
  smallest common multiple of \a tilewidth and \a tileheight (for
  example SCM(16,32)==2).
*/
QCanvas::QCanvas( QPixmap p,
        int h, int v, int tilewidth, int tileheight,
        int chunksize, int maxclusters )
{
    init(h*tilewidth, v*tileheight,
        chunksize < 0 ? scm(tilewidth,tileheight) : chunksize,
	maxclusters );
    initTiles( p, h, v, tilewidth, tileheight );
}

void QCanvas::initTiles(QPixmap p,
        int h, int v, int tilewidth, int tileheight)
{
    htiles = h;
    vtiles = v;
    if ( h && v ) {
	grid = new ushort[h*v];
	memset( grid, 0, h*v*sizeof(ushort) );
	setTiles( p, tilewidth, tileheight );
    } else {
	grid = 0;
    }
}

/*!
  \fn int tile( int x, int y ) const
  Returns the tile axt (\a x, \a y).
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
  This may be made public later.
*/
void QCanvas::setTiles( QPixmap p, int tilewidth, int tileheight )
{
    pm = p;
    tilew = tilewidth;
    tileh = tileheight;
    oneone = tilew == tileh;
    // set all dirty / repaint all views
}

/*!
  Sets the tile at (\a x, \a y) to use tile number \a tilenum,
  which is an index into the tile pixmaps.  The canvas will
  update appropriately when update() is next called.
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
	    setChangedChunk( x/(chunkSize()/tilew),
			     y/(chunkSize()/tileh) );
	}
    }
}

class QCanvasItemExtra {
    double vx,vy;
    friend QCanvasItem;
};


/*!
\class QCanvasItem qcanvas.h
\brief An abstract graphic object on a QCanvas.

This class will primarily be of interest to those wanting to
add graphic objects other than the sprite object already defined.

In most graphic rendering systems, graphic objects are considered to
have a bounding \e rectangle, and redraw optimization is based on this
simplification.  This simplistic view is not used by QCanvas.  A
QCanvas considers itself to contain numerous graphic objects, each
of which covers certain `chunks' in the QCanvas.  For graphic
objects with a rectangular bias, this has only a minor effect on
redraw efficiency (although still a major effect on collision
detection and other area indexing).  But for other shapes, such as
lines, the tighter area-bound made possible by chunks can provide
improvements.

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
coordinates place it far off the edge of the area of the QCanvas.

Derived classes should call addToChunks()
in their constructor once x(), y(), and image() are valid.
*/

/*!
Construct a QCanvasItem on the current canvas.

\sa setCurrentSpriteField(QCanvas*) setSpriteField(QCanvas*)
*/
QCanvasItem::QCanvasItem() :
    cnv(current_canvas),
    myx(0),myy(0),myz(0),
    vis(TRUE)
{
    ext = 0;
    if (cnv) cnv->addItem(this);
}


/*!
Destruct a QCanvasItem.  It is removed from its canvas.
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

void QCanvasItem::setVelocity( double vx, double vy)
{
    if ( ext || vx!=0.0 || vy!=0.0 ) {
	if ( !ext && cnv )
	    cnv->addAnimation(this);
	extra().vx = vx;
	extra().vy = vy;
    }
}

double QCanvasItem::xVelocity() const
{
    return ext ? ext->vx : 0;
}

double QCanvasItem::yVelocity() const
{
    return ext ? ext->vy : 0;
}


void QCanvasItem::forward()
{
    if ( ext ) moveBy(ext->vx,ext->vy);
}

/*!
\fn int QCanvasItem::z() const

This abstract method should return the z of the item,
which is used for visual order:  higher z sprites obscure
lower-z ones.
*/

/*!
\fn void QCanvasItem::draw(QPainter&)

This abstract method should draw the the item on the given QPainter.
*/

/*!
QCanvasItems may be created on a `current' QCanvas, which must be
set prior to any item creation.  QCanvas objects set this,
so the most recently created QCanvas will get new QCanvasItems.

This notion of `currency' makes the most common case easy to use - that
of a single instance of QCanvas in the application - rather than
having to pass around a pointer to a QCanvas.
*/
void QCanvasItem::setCurrentCanvas(QCanvas* pf)
{
    current_canvas=pf;
}

/*!
Set the QCanvas upon which the QCanvasItem is to be drawn.
Initially this will be the current canvas.

\sa setCurrentSpriteField(QCanvas*)
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
Alias for visible(TRUE).
*/
void QCanvasItem::show()
{
    setVisible(TRUE);
}

/*!
Alias for visible(FALSE).
*/
void QCanvasItem::hide()
{
    setVisible(FALSE);
}

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
*/

/*!
\fn bool QCanvasItem::act() const
Returns TRUE if the QCanvasItem is selected.
*/

/*!
*/
void QCanvasItem::setSelected(bool yes)
{
    if (sel!=yes) {
	if (yes) {
	    sel=yes;
	    addToChunks();
	} else {
	    removeFromChunks();
	    sel=yes;
	}
    }
}

/*!
\fn bool QCanvasItem::enabled() const
Returns TRUE if the QCanvasItem is enabled.
*/

/*!
*/
void QCanvasItem::setEnabled(bool yes)
{
    if (ena!=yes) {
	if (yes) {
	    ena=yes;
	    addToChunks();
	} else {
	    removeFromChunks();
	    ena=yes;
	}
    }
}

/*!
\fn bool QCanvasItem::active() const
Returns TRUE if the QCanvasItem is active.
*/

/*!
*/
void QCanvasItem::setActive(bool yes)
{
    if (act!=yes) {
	if (yes) {
	    act=yes;
	    addToChunks();
	} else {
	    removeFromChunks();
	    act=yes;
	}
    }
}


QCanvas* QCanvasItem::current_canvas=0;

/*!
\fn bool QCanvasItem::at(int x, int y) const
Should return TRUE if the item includes the given pixel position.
*/
bool QCanvasItem::at(int, int) const
{
    return FALSE;
}

/*!
\fn bool QCanvasItem::at(const QRect& rect) const
TRUE if the item intersects with the given area.
*/
bool QCanvasItem::at(const QRect&) const
{
    return FALSE;
}

bool QCanvasItem::at(const QImage*, const QRect&) const
{
    return FALSE;
}

/*!
\fn int QCanvasItem::world_to_x(int i)

Same as QCanvas::world_to_x(int i)
*/
/*!
\fn int QCanvasItem::x_to_world(int i)

Same as QCanvas::x_to_world(int i)
*/

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
	    mask.convertFromImage(*collision_mask);
	    setMask(mask);
	} else {
	    collision_mask=0;
	}
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
When testing sprite collision detection with QCanvas::exact(QCanvasIteratorPrivate*),
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

Tests if the given pixel touches the sprite.  This uses pixel-accurate
detection, using the collision mask of the sprites current image (which
is by default the image mask).  This test is useful for example to test
when the user clicks a point with they mouse.  Note however, that
QCanvas::topAt(int x, int y) and associated methods give a more
efficient and flexible technique for this purpose.
*/
bool QCanvasSprite::at(int px, int py) const
{
    px=px-absX();
    py=py-absY();

    if (px<0 || px>=width() || py<0 || py>=height())
	return FALSE;

    QImage* img=image()->collision_mask;

    if (img) {
	if (img->bitOrder() == QImage::LittleEndian) {
	    return *(img->scanLine(py) + (px >> 3)) & (1 << (px & 7));
	} else {
	    return *(img->scanLine(py) + (px >> 3)) & (1 << (7 -(px & 7)));
	}
    } else {
	return TRUE;
    }
}

/*!
\sa neighbourhood(int,int,QCanvasPixmap*) const
*/
bool QCanvas::exact(QCanvasIteratorPrivate* p) const
{
    QCanvasIteratorPrivate* iterator=(QCanvasIteratorPrivate*)p;
    QRect area(iterator->x,iterator->y,iterator->w,iterator->h);
    return iterator->element()->at(area)
	&& (!iterator->collision_mask
	    || iterator->element()->at(iterator->collision_mask, area));
}

/*!
Used by the methods associated with
QCanvas::topAt(int x, int y) to test for a close hit.
\sa neighbourhood(int,int,QCanvasPixmap*)
*/
bool QCanvasSprite::at(const QRect& rect) const
{
    QRect crect(rect.x(),rect.y(), rect.width(),rect.height());
    QRect myarea(absX(),absY(),width(),height());
    return crect.intersects(myarea);
}

/*!
Used by the methods associated with
QCanvas::topAt(int x, int y) to test for an exact hit.
\sa neighbourhood(int,int,QCanvasPixmap*)
*/
bool QCanvasSprite::at(const QImage* yourimage, const QRect& yourarea) const
{
    QRect cyourarea(yourarea.x(),yourarea.y(),
	    yourarea.width(),yourarea.height());

    QImage* myimage=image()->collision_mask;

    QRect ourarea(absX(),absY(),width(),height()); // myarea

    ourarea=ourarea.intersect(cyourarea);

    int yx=ourarea.x()-cyourarea.x();
    int yy=ourarea.y()-cyourarea.y();
    int mx=ourarea.x()-absX();
    int my=ourarea.y()-absY();
    int w=ourarea.width();
    int h=ourarea.height();

    if ( !yourimage ) {
	if ( !myimage )
	    return w>0 && h>0;
	// swap everything around
	int t;
	t=mx; mx=yx; yx=t;
	t=my; mx=yy; yy=t;
	yourimage = myimage;
	myimage = 0;
    }

    // yourimage != 0

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
    // ASSERT(myimage->bitOrder()==yourimage->bitOrder());

    if (myimage) {
	if (myimage->bitOrder() == QImage::LittleEndian) {
	    for (int j=0; j<h; j++) {
		uchar* ml = myimage->scanLine(my+j);
		uchar* yl = yourimage->scanLine(yy+j);
		for (int i=0; i<w; i++) {
		    if (*(yl + ((yx+i) >> 3)) & (1 << ((yx+i) & 7))
		    && *(ml + ((mx+i) >> 3)) & (1 << ((mx+i) & 7)))
		    {
			return TRUE;
		    }
		}
	    }
	} else {
	    for (int j=0; j<h; j++) {
		uchar* ml = myimage->scanLine(my+j);
		uchar* yl = yourimage->scanLine(yy+j);
		for (int i=0; i<w; i++) {
		    if (*(yl + ((yx+i) >> 3)) & (1 << (7-((yx+i) & 7)))
		    && *(ml + ((mx+i) >> 3)) & (1 << (7-((mx+i) & 7))))
		    {
			return TRUE;
		    }
		}
	    }
	}
    } else {
	if (yourimage->bitOrder() == QImage::LittleEndian) {
	    for (int j=0; j<h; j++) {
		uchar* yl = yourimage->scanLine(yy+j);
		for (int i=0; i<w; i++) {
		    if (*(yl + ((yx+i) >> 3)) & (1 << ((yx+i) & 7)))
		    {
			return TRUE;
		    }
		}
	    }
	} else {
	    for (int j=0; j<h; j++) {
		uchar* yl = yourimage->scanLine(yy+j);
		for (int i=0; i<w; i++) {
		    if (*(yl + ((yx+i) >> 3)) & (1 << (7-((yx+i) & 7))))
		    {
			return TRUE;
		    }
		}
	    }
	}
    }

    return FALSE;
}

/*
QCanvasIteratorPrivate* QCanvasItem::neighbourhood() const
{
    return neighbourhood(x(),y());
}

QCanvasIteratorPrivate* QCanvasItem::neighbourhood(int nx, int ny) const
{
    return neighbourhood(nx,ny,0);
}
*/

/*!
Creates an iterator which can traverse the area which the QCanvasItem
would cover if it had the given position and image.  This `would cover'
concept is useful as it allows you to check for a collision <em>before</em>
moving the sprite.

*/
/*
QCanvasIteratorPrivate* QCanvasItem::neighbourhood(int nx, int ny, QCanvasPixmap* img) const
{
    if (!cnv) return 0;

    QCanvasIteratorPrivate* iterator=
	QCanvasIteratorPrivate::make(nx,ny,img,cnv);

    while (iterator && iterator->element()==(QCanvasItem*)this)
	iterator=iterator->next(cnv);

    return (QCanvasIteratorPrivate*)iterator;
}
*/

/*!
Creates an iterator which traverses the QCanvasItem objects which
collide with this sprite at its current position.

\sa neighbourhood(int,int,QCanvasPixmap*)
*/
/*
QCanvasIteratorPrivate* QCanvasSprite::neighbourhood() const
{ return neighbourhood(x(),y(),image()); }
*/

/*!
Creates an iterator which traverses the QCanvasItem objects which
would collide with this sprite if it was moved to the given position (yet
kept its current image).

\sa neighbourhood(int,int,QCanvasPixmap*)
*/
/*
QCanvasIteratorPrivate* QCanvasSprite::neighbourhood(int nx, int ny) const
{ return neighbourhood(nx,ny,image()); }
QCanvasIteratorPrivate* QCanvasSprite::neighbourhood(int nx, int ny, QCanvasPixmap* img) const
{ return QCanvasItem::neighbourhood(nx,ny,img); }
*/

/*!
Traverse to the next QCanvasItem in a collision list.

\sa neighbourhood(int,int,QCanvasPixmap*)
*/
void QCanvas::next(QCanvasIteratorPrivate*& p) const
{
    if (p) {
	QCanvasIteratorPrivate* iterator=(QCanvasIteratorPrivate*)p;

	iterator=iterator->next(this);

	while (iterator && iterator->element()==(QCanvasItem*)this)
	    iterator=iterator->next(this);

	p=(QCanvasIteratorPrivate*)iterator;
    }
}

/*!
Test if this sprite is touching the given QCanvasItem.
This is a convenient packaging of neighbourhood() and related methods.
*/
bool QCanvasSprite::hitting(QCanvasItem& other) const
{
    return QCanvasItem::wouldHit(other,x(),y(),image());
}

/*!
Test if this sprite would (exactly) touch the given QCanvasItem.
This is a convenient packaging of neighbourhood() and related methods.
*/
bool QCanvasItem::wouldHit(QCanvasItem& other, int x, int y, QCanvasPixmap* img) const
{
/*
    // Cast off const.  Safe, because we don't call non-const methods
    // of the QCanvasItem returnsed by at().
    //
    QCanvasItem* nconst_this=(QCanvasItem*)this;

    for (QCanvasIteratorPrivate* p=nconst_this->neighbourhood(x,y,img); p; nconst_this->next(p)) {
	if (nconst_this->at(p)==&other) {
	    if (nconst_this->exact(p)) {
		nconst_this->end(p);
		return TRUE;
	    }
	}
    }
*/
    return FALSE;
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
qDebug(repaint_from_moving?"Moving":"Updating");
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
    viewport()->setBackgroundColor(white); // XXX same as canvas bg
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

/*!
\fn QPointArray QCanvasPolygonalItem::areaPoints(QPoint& offset) const;
Sets \a offset to the offset of the area, and
returns the points of the polygonal area covered
by the item.

Call hide() before this changes, show() afterwards.
*/

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

bool QCanvasPolygonalItem::at(int x, int y) const
{
    return scan(QRect(x,y,1,1));
}

bool QCanvasPolygonalItem::at(const QRect &r) const
{
    return scan(r);
}

void QCanvasPolygonalItem::chunkify(int type)
{
    QPointArray pa = areaPoints();

    if ( !pa.size() )
	return;

    // XXX Just a simple implementation for now - find the bounding
    //     rectangle and add to every chunk thus touched.
    // XXX A better algorithm would be some kine of scanline polygon
    //     render (hence the API asks for some points).

    QRect brect = pa.boundingRect();
    int chunksize=canvas()->chunkSize();
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
	}
    }
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

void QCanvasPolygon::drawShape(QPainter & p)
{
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
    pa[0] = QPoint(x(),y());
    pa[1] = pa[0] + QPoint(w-1,0);
    pa[2] = pa[0] + QPoint(w-1,h-1);
    pa[3] = pa[0] + QPoint(0,h-1);
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
    // XXX need something better to improve hit test now, and
    //     paint efficiency later.  For now, just a rectangle.
    QPointArray pa(4);
    QPoint o = QPoint(x(),y());
    pa[0] = o + QPoint(   -w/2,    -h/2);
    pa[1] = o + QPoint((w+1)/2,    -h/2);
    pa[2] = o + QPoint((w+1)/2, (w+1)/2);
    pa[3] = o + QPoint(   -w/2, (w+1)/2);
    return pa;
}

void QCanvasEllipse::drawShape(class QPainter & p)
{
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
  \fn const QRect& QCanvasText::boundingRect()
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
  For collision detection.
*/
bool QCanvasText::at(int x, int y) const
{
    return brect.contains(QPoint(x,y));
}

/*!
  For collision detection.
*/
bool QCanvasText::at(const class QRect & r) const
{
    return r.intersects(brect);
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


/*!
\fn QCanvasIteratorPrivate* QCanvasSprite::neighbourhood(int nx, int ny) const
Same as QCanvasItem::neighbourhood(int x, int y).
*/

/*!
\fn QCanvasIteratorPrivate* QCanvasSprite::neighbourhood(int nframe) const
Similar to QCanvasItem::neighbourhood(QCanvasPixmap*), but
the image is specified by index rather than actual value.
*/
/*
QCanvasIteratorPrivate* QCanvasSprite::neighbourhood(int nframe) const
{ return neighbourhood(x(),y(),nframe); }
*/

/*!
\fn QCanvasIteratorPrivate* QCanvasSprite::neighbourhood(double nx, double ny, int nframe) const
Similar to QCanvasItem::neighbourhood(int x, int y, QCanvasPixmap*), but
the image is specified by index rather than actual value.
*/
/*
QCanvasIteratorPrivate* QCanvasSprite::neighbourhood(double nx, double ny, int nframe) const
{ return QCanvasItem::neighbourhood((int)nx,(int)ny,image(nframe)); }
*/

/*!
\fn bool QCanvasSprite::wouldHit(QCanvasItem& other, double x, double y, int frame) const
Similar to QCanvasItem::wouldHit(QCanvasItem&, int x, int y, QCanvasPixmap*),
but the image is specified by index rather than actual value.
*/
bool QCanvasSprite::wouldHit(QCanvasItem& other, double x, double y, int frame) const
{
    return QCanvasItem::wouldHit(other,(int)x,(int)y,image(frame));
}
