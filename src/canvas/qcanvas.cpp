/**********************************************************************
** $Id: //depot/qt/main/src/widgets/qcanvas.cpp#1 $
**
** Implementation of QCanvas and associated classes
**
** Created : 991211
**
** Copyright (C) 1999-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the canvas module of the Qt GUI Toolkit.
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
** Licensees holding valid Qt Enterprise Edition licenses may use this
** file in accordance with the Qt Commercial License Agreement provided
** with the Software.
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


#include "qcanvas.h"
#ifndef QT_NO_CANVAS
#include "qpainter.h"
#include "qimage.h"
#include "qbitmap.h"
#include "qwidget.h"
#include "qtl.h"
#include "qtimer.h"
#include "qpolygonscanner.h"

#include <stdlib.h>

class QCanvas::Data {
public:
    QList<QCanvasView> viewList;
    QPtrDict<void> itemDict;
    QPtrDict<void> animDict;
};

class QCanvasView::Data {
public:
    QWMatrix xform;
    QWMatrix ixform;
};

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
    const int maxcl;
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
    maxcl(maxclusters)
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
    cursor = 0;
    while( cursor<count ) {
	if (cluster[cursor].intersects(biggerrect)) {
	    QRect larger=cluster[cursor];
	    include(larger,rect);
	    int cost = larger.width()*larger.height() -
		       cluster[cursor].width()*cluster[cursor].height();

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
	cursor++;
    }

    if (cheapest>=0) {
	include(cluster[cheapest],rect);
	return;
    }

    if (count < maxcl) {
	cluster[count++]=rect;
	return;
    }

    // Do cheapest of:
    //     add to closest cluster
    //     do cheapest cluster merge, add to new cluster

    lowestcost=9999999;
    cheapest=-1;
    cursor=0;
    while( cursor<count ) {
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
	cursor++;
    }

    // ###
    // could make an heuristic guess as to whether we need to bother
    // looking for a cheap merge.

    int cheapestmerge1 = -1;
    int cheapestmerge2 = -1;

    int merge1 = 0;
    while( merge1 < count ) {
	int merge2=0;
	while( merge2 < count ) {
	    if( merge1!=merge2) {
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
	    merge2++;
	}
	merge1++;
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

    // ###
    //
    // add explicit x/y ordering to that comment, move it to the top
    // and rephrase it as pre-/post-conditions.
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
    bool operator==(const QCanvasItemPtr& that) const
    {
	    return that.ptr == ptr;
    }
    operator QCanvasItem*() const { return ptr; }

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
    int r;
    while ( (r = a%b) ) {
	a=b;
	b=r;
    }
    return b;
}

static int scm(int a, int b)
{
    int g = gcd(a,b);
    return a/g*b;
}



/*!
\class QCanvas qcanvas.h
\brief The QCanvas class provides a 2D graphic area containing QCanvasItem objects.

\ingroup abstractwidgets

The QCanvas class manages these items and is displayed on the screen
through QCanvasView widgets.

It is optimized for a large number of items, and Qt provides a rich
set of item classes, including QCanvasText, QCanvasPixmap,
QCanvasSprite, QCanvasEllipse, QCanvasLine and quite a few more.

A canvas may in principle look similar to a widget with child widgets,
but there are several notable differences:

<ul>

<li> Items can be much faster, particularly when there are \e many
items or nonrectangular items. Sometimes they can also be much more
memory efficient.

<li> It's easy to detect item overlapping (collision detection).

<li> The canvas can be larger than a widget. A million-by-million
canvas is very reasonable. Whereas a widget might be very inefficient
at this size and some window systems might not support it at all,
QCanvas scales. Even with a billion pixels and a million items, it's
still fast to find a single canvas item, detect collisions, etc.

<li> Two or more QCanvasView objects can view the same canvas.

<li> Widgets offer a lot more functionality, such as input (QKeyEvent,
QMouseEvent etc.) and layout management (QGridLayout etc.).

</ul>

The canvas is made up from a background, a number of items organized
by x, y and z coordinates, and a foreground. (The z coordinate may be
viewed as a layer number - higher in front.) We'll cover each in turn.

The background is white by default, but can be set to a different
color using setBackgroundColor(), a repeated pixmap using
setBackgroundPixmap() or to a mosaic of smaller pixmaps using
setTiles(). As usual, there are corresponding getters like
backgroundColor().

Note that setBackgroundPixmap() is \e not the same as
QWidget::setBackgroundPixmap(). \l QCanvasView is a widget, but
QCanvas is not. There are also a few other functions like those in
QWidget, namely resize(), size(), width() and height().

The items are the movable objects that inherit QCanvasItem.  Each item
has a position on the canvas and a height, both of which are given as
floating-point numbers. It's possible for an item to be outside the
canvas (for example QCanvasItem::x() is greater than width()).  When
an item is off the canvas, onCanvas() returns FALSE and the canvas
disregards the item.  (Items off the canvas do not slow down any
common opererations on the canvas.)

Some items can be animated or moved; advance() moves them and
setAdvancePeriod() makes QCanvas move them by itself.

QCanvas organizes these items into \e chunks - areas on the canvas
that are used for speeding up most operations.  Many operations start
by eliminating most chunks, and then process only the items that are
in the few interesting chunks.

The chunk size is thus the key factor to QCanvas' speed: If there are
too many chunks, QCanvas needs to process too many chunks. If they're
too large, it takes too long to process each chunk.  The QCanvas
constructor picks a hopefully suitable size, but you can call retune()
to change it at any time.  chunkSize() returns the current chunk size.

The items always make sure they're in the right chunks; all you need to
ake sure is that the canvas has about the right chunk size.  A good rule
of thumb is that the size should be a bit smaller than the average item
size. If you have moving objects, it should be a bit smaller than the
average size of the moving items.

The foreground is normally nothing, but if you reimplement
drawForeground(), you can draw things in front of all canvas items.

\sa QCanvasView QCanvasItem
*/

// ### need to work in the setUpdatePeriod() stuff, update(), etc.

void QCanvas::init(int w, int h, int chunksze, int mxclusters)
{
    d = new Data;
    awidth=w;
    aheight=h;
    chunksize=chunksze;
    maxclusters=mxclusters;
    chwidth=(w+chunksize-1)/chunksize;
    chheight=(h+chunksize-1)/chunksize;
    chunks=new QCanvasChunk[chwidth*chheight];
    update_timer = 0;
    bgcolor = white;
    grid = 0;
    dblbuf = TRUE;
    debug_redraw_areas = FALSE;
}

/*! Create a QCanvas with no size. \a parent and \a name have the
  usual QObject meaning.

  You need to call resize() at some time after creation to be able to
  use the canvas.
*/
QCanvas::QCanvas( QObject* parent, const char* name )
    : QObject( parent, name )
{
    init(0,0);
}

/*!
Constructs a QCanvas that is \a w pixels wide and \a h pixels high.
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

void qt_unview(QCanvas* c)
{
    for (QCanvasView* view=c->d->viewList.first(); view != 0; view=c->d->viewList.next()) {
	view->viewing = 0;
    }
}

/*!
  Destructs the canvas.  Does \e not destroy items on the canvas.
*/
QCanvas::~QCanvas()
{
    qt_unview(this);
    QCanvasItemList all = allItems();
    for (QCanvasItemList::Iterator it=all.begin(); it!=all.end(); ++it)
	delete *it;
    delete [] chunks;
    delete [] grid;
    delete d;
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
    for (QPtrDictIterator<void> it=d->itemDict; it.currentKey(); ++it) {
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
    for (QPtrDictIterator<void> it=d->itemDict; it.currentKey(); ++it) {
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

    emit resized();
}

/*!
  \fn void QCanvas::resized()

  This signal is emitted whenever the canvas is resized.  Each QCanvasView
  connects to this signal to keep the scrollview size correct.
*/

/*!  Change the efficiency tuning parameters to \a mxclusters clusters,
each of size \a chunksze (square).  This is a slow operation if you
have many objects on the canvas.

Internally, a canvas uses a low-resolution "chunk matrix" to keep
track of all the items in the canvas. In Qt 2.2, the default for a
1024x1024 pixel canvas is to have a 64x64 chunk matrix, where each of
those chunks collects items in a 16x16 pixel square.

This default is also affected by setTiles(). You can tune this default
by using retune(), for example if you have a very large canvas and
want to trade off speed for memory then you might set the chunk size
to 32 or 64.

\a chunksize is the size of square chunk used to break up the QCanvas
into area to be considered for redrawing.  It should be about the
average size of items in the QCanvas.  Chunks too small increase the
amount of calculation required when drawing.  Chunks too large
increase the amount of drawing that is needed.

\a mxclusters is the number of rectangular groups of chunks that will
be separately drawn.  If the QCanvas has a large number of small,
dispersed items, this should be about that number.  The more clusters
the slower the redraw, but also the bigger clusters are the slower the
redraw, so a balance is needed.  Testing indicates that a large number
of clusters is almost always best.

*/
void QCanvas::retune(int chunksze, int mxclusters)
{
    maxclusters=mxclusters;

    if ( chunksize!=chunksze ) {
	QList<QCanvasItem> hidden;
	for (QPtrDictIterator<void> it=d->itemDict; it.currentKey(); ++it) {
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
  \fn bool QCanvas::onCanvas( int x, int y ) const
  Returns whether the pixel position (\a x, \a y) is on the canvas.
*/

/*!
  \fn bool QCanvas::onCanvas( const QPoint& p ) const
  Returns whether the pixel position \a p is on the canvas.
*/

/*!
  \fn bool QCanvas::validChunk( int x, int y ) const
  Returns whether the chunk position (\a x, \a y) is on the canvas.
*/

/*!
  \fn bool QCanvas::validChunk( const QPoint& p ) const
  Returns whether the chunk position \a p is on the canvas.
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
    d->itemDict.insert(item,(void*)1);
}

/*!
\internal
This method adds an element to the list of QCanvasItem objects
to be animated. The QCanvasItem class calls this.
*/
void QCanvas::addAnimation(QCanvasItem* item)
{
    d->animDict.insert(item,(void*)1);
}

/*!
\internal
This method adds an element to the list of QCanvasItem objects
which are no longer to be animated. The QCanvasItem class calls this.
*/
void QCanvas::removeAnimation(QCanvasItem* item)
{
    d->animDict.remove(item);
}

/*!
\internal
This method removes an element from the list of QCanvasItem objects
in this QCanvas.  The QCanvasItem class calls this.
*/
void QCanvas::removeItem(QCanvasItem* item)
{
    d->itemDict.remove(item);
}

/*!
\internal
This method adds an element to the list of QCanvasView objects
viewing this QCanvas.  The QCanvasView class calls this.
*/
void QCanvas::addView(QCanvasView* view)
{
    d->viewList.append(view);
    if ( htiles>1 || vtiles>1 || pm.isNull() ) {
	view->viewport()->setBackgroundColor(backgroundColor());
    } else {
	view->viewport()->setBackgroundPixmap(backgroundPixmap());
    }
}

/*!
\internal
This method removes an element from the list of QCanvasView objects
viewing this QCanvas.  The QCanvasView class calls this.
*/
void QCanvas::removeView(QCanvasView* view)
{
    d->viewList.removeRef(view);
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

/*!  Advances the animation of items on the canvas and refreshes all
  changes to all views of the canvas.

  The advance is done in two phases.  In phase 0, the
  QCanvasItem:advance() function of each animated item is called with
  paramater 0. Then all items are called again, with parameter 1. In
  phase 0, the items should not change position, merely examine other
  items on the canvas for which special processing is required, such
  as collisions between items. In phase 1, all items should change
  positions, ignoring any other items on the canvas.  This two-phase
  approach allows for considerations of "fairness", though no
  QCanvasItem subclasses supplied with Qt do anything interesting in
  phase 0.

  The canvas can be configured to call this function periodically with
  setAdvancePeriod().

  \sa update()
*/
void QCanvas::advance()
{
    QPtrDictIterator<void> it=d->animDict;
    while ( it.current() ) {
	QCanvasItem* i = (QCanvasItem*)it.currentKey();
	++it;
	if ( i )
	    i->advance(0);
    }
    // we expect the dict contains the exact same items as in the
    // first pass.
    it.toFirst();
    while ( it.current() ) {
	QCanvasItem* i = (QCanvasItem*)it.currentKey();
	++it;
	if ( i )
	    i->advance(1);
    }
    update();
}

// Don't call this unless you know what you're doing.
void QCanvas::drawViewArea( QCanvasView* view, QPainter* p, const QRect& vr, bool dbuf )
{
    QWMatrix wm = view->worldMatrix();
    QPoint tl = view->contentsToViewport(QPoint(0,0));
    QWMatrix iwm = wm.invert();

    // ivr = covers all chunks in vr
    QRect ivr = iwm.map(vr);

    QWMatrix twm;
    twm.translate(tl.x(),tl.y());

    QRect all(0,0,width(),height());

    if ( !all.contains(ivr) ) {
	// need to clip with edge of canvas
	QPointArray a( all );
	QWMatrix ttwm;
	ttwm.translate(tl.x(),tl.y());
	a = (wm*ttwm).map(a);
	p->setClipRegion(a);
    }

    if ( dbuf ) {
	ensureOffScrSize( vr.width(), vr.height() );
	QPainter dbp(&offscr);
	twm.translate(-vr.x(),-vr.y());
	twm.translate(-tl.x(),-tl.y());
	dbp.setWorldMatrix( wm*twm, TRUE );
	dbp.setClipRect(0,0,vr.width(), vr.height());
	drawCanvasArea(ivr,&dbp,FALSE);
	p->drawPixmap(vr.x()+tl.x(), vr.y()+tl.y(), offscr, 0, 0,
	    vr.width(), vr.height());
    } else {
	p->setWorldMatrix( wm*twm );

	QRect r = vr; r.moveBy(tl.x(),tl.y());
	if ( !all.contains(ivr) ) {
	    p->setClipRegion(p->clipRegion() & r);
	} else {
	    p->setClipRect(r);
	}

	drawCanvasArea(ivr,p,FALSE);
    }
}

/*!  Refreshes all changes to all views of the canvas.

  \sa advance()
*/
void QCanvas::update()
{
    QCanvasClusterizer clusterizer(d->viewList.count());
    QList<QRect> doneareas;
    doneareas.setAutoDelete(TRUE);

    QCanvasView* view;
    for ( view=d->viewList.first(); view != 0; view=d->viewList.next() ) {
	QWMatrix wm = view->worldMatrix();
	QRect area(view->contentsX(),view->contentsY(),
		   view->visibleWidth(),view->visibleHeight());
	if (area.width()>0 && area.height()>0) {
	    if ( wm.isIdentity() ) {
		clusterizer.add(area);
	    } else {
		// r = Visible area of the canvas where there are changes
		QRect r = changeBounds(view->inverseWorldMatrix().map(area));
		if ( !r.isEmpty() ) {
		    QPainter p(view->viewport());
		    drawViewArea( view, &p, wm.map(r), dblbuf );
		    doneareas.append(new QRect(r));
		}
	    }
	}
    }

    for (int i=0; i<clusterizer.clusters(); i++)
	drawChanges(clusterizer[i]);

    for ( QRect* r=doneareas.first(); r != 0; r=doneareas.next() )
	setUnchanged(*r);
}


// ### warwick - setAllChanged() is not a set function. please rename
// it. ditto setChanged(). markChanged(), perhaps?

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
    QRect thearea = area.intersect(QRect(0,0,width(),height()));

    int mx = (thearea.x()+thearea.width()+chunksize)/chunksize;
    int my = (thearea.y()+thearea.height()+chunksize)/chunksize;
    if (mx>chwidth)
	mx=chwidth;
    if (my>chheight)
	my=chheight;

    int x=thearea.x()/chunksize;
    while( x<mx) {
	int y = thearea.y()/chunksize;
	while( y<my ) {
	    chunk(x,y).change();
	    y++;
	}
	x++;
    }
}

/*!
  Sets all views of \a area to \e not be redrawn when
  update() is next called.
*/
void QCanvas::setUnchanged(const QRect& area)
{
    QRect thearea = area.intersect(QRect(0,0,width(),height()));

    int mx = (thearea.x()+thearea.width()+chunksize)/chunksize;
    int my = (thearea.y()+thearea.height()+chunksize)/chunksize;
    if (mx>chwidth)
	mx=chwidth;
    if (my>chheight)
	my=chheight;

    int x=thearea.x()/chunksize;
    while( x<mx) {
	int y = thearea.y()/chunksize;
	while( y<my ) {
	    chunk(x,y).takeChange();
	    y++;
	}
	x++;
    }
}

QRect QCanvas::changeBounds(const QRect& inarea)
{
    QRect area=inarea.intersect(QRect(0,0,width(),height()));

    int mx = (area.x()+area.width()+chunksize)/chunksize;
    int my = (area.y()+area.height()+chunksize)/chunksize;
    if (mx > chwidth)
	mx=chwidth;
    if (my > chheight)
	my=chheight;

    QRect result;

    int x=area.x()/chunksize;
    while( x<mx ) {
	int y=area.y()/chunksize;
	while( y<my ) {
	    QCanvasChunk& ch=chunk(x,y);
	    if ( ch.hasChanged() )
		result |= QRect(x,y,1,1);
	    y++;
	}
	x++;
    }

    if ( !result.isEmpty() ) {
	result.rLeft() *= chunksize;
	result.rTop() *= chunksize;
	result.rRight() *= chunksize;
	result.rBottom() *= chunksize;
	result.rRight() += chunksize;
	result.rBottom() += chunksize;
    }

    return result;
}

/*!
\internal
Redraw a given area of the QCanvas.
*/
void QCanvas::drawChanges(const QRect& inarea)
{
    QRect area=inarea.intersect(QRect(0,0,width(),height()));

    QCanvasClusterizer clusters(maxclusters);

    int mx = (area.x()+area.width()+chunksize)/chunksize;
    int my = (area.y()+area.height()+chunksize)/chunksize;
    if (mx > chwidth)
	mx=chwidth;
    if (my > chheight)
	my=chheight;

    int x=area.x()/chunksize;
    while( x<mx ) {
	int y=area.y()/chunksize;
	while( y<my ) {
	    QCanvasChunk& ch=chunk(x,y);
	    if ( ch.hasChanged() )
		clusters.add(x,y);
	    y++;
	}
	x++;
    }

    for (int i=0; i<clusters.clusters(); i++) {
	QRect elarea=clusters[i];
	elarea.setRect(
	    elarea.left()*chunksize,
	    elarea.top()*chunksize,
	    elarea.width()*chunksize,
	    elarea.height()*chunksize
	);
	drawCanvasArea(elarea);
    }
}

void QCanvas::ensureOffScrSize( int osw, int osh )
{
    if ( osw > offscr.width() || osh > offscr.height() )
	offscr.resize(QMAX(osw,offscr.width()),
		      QMAX(osh,offscr.height()));
    else if ( offscr.width() == 0 || offscr.height() == 0 )
	offscr.resize( QMAX( offscr.width(), 1),
		       QMAX( offscr.height(), 1 ) );
}

/*!
  Paints all items that are in the area \a clip to \a painter,
  using double-buffer if \a dbuf is TRUE.

  eg. to print the canvas to a printer:

  \code
  QPrinter pr;
  if ( pr.setup() ) {
    QPainter p(&pr);
    canvas.drawArea( canvas.rect(), &p );
  }
  \endcode
*/
void QCanvas::drawArea(const QRect& clip, QPainter* painter, bool dbuf)
{
    if ( painter ) drawCanvasArea( clip, painter, dbuf );
}

void QCanvas::drawCanvasArea(const QRect& inarea, QPainter* p, bool double_buffer)
{
    QRect area=inarea.intersect(QRect(0,0,width(),height()));

    if ( !dblbuf )
	double_buffer = FALSE;

    if (!d->viewList.first() && !p) return; // Nothing to do.

    int lx=area.x()/chunksize;
    int ly=area.y()/chunksize;
    int mx=area.right()/chunksize;
    int my=area.bottom()/chunksize;
    if (mx>=chwidth)
	mx=chwidth-1;
    if (my>=chheight)
	my=chheight-1;

    QCanvasItemList allvisible;

    // Stores the region within area that need to be drawn. It is relative
    // to area.topLeft()  (so as to keep within bounds of 16-bit XRegions)
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
		    // ### should at least make bands
		    rgn |= QRegion(x*chunksize-area.x(),y*chunksize-area.y(),
				    chunksize,chunksize);
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
	ensureOffScrSize( area.width(), area.height() );
	painter.begin(&offscr);
	painter.translate(-area.x(),-area.y());
	if ( p ) {
	    painter.setClipRect(QRect(0,0,area.width(),area.height()));
	} else {
	    painter.setClipRegion(rgn);
	}
	drawBackground(painter,area);
	allvisible.drawUnique(painter);
	drawForeground(painter,area);
	painter.end();
	if ( p ) {
	    p->drawPixmap( area.x(), area.y(), offscr,
		0, 0, area.width(), area.height() );
	    return;
	}
    } else if ( p ) {
	drawBackground(*p,area);
	allvisible.drawUnique(*p);
	drawForeground(*p,area);
	return;
    }

    QPoint trtr; // keeps track of total translation of rgn

    trtr -= area.topLeft();

    for (QCanvasView* view=d->viewList.first(); view; view=d->viewList.next()) {
	if ( !view->worldMatrix().isIdentity() )
	    continue; // Cannot paint those here (see callers).
	QPainter painter(view->viewport());
	QPoint tr = view->contentsToViewport(area.topLeft());
	QPoint nrtr = view->contentsToViewport(QPoint(0,0)); // new translation
	QPoint rtr = nrtr - trtr; // extra translation of rgn
	trtr += rtr; // add to total
	if (double_buffer) {
	    rgn.translate(rtr.x(),rtr.y());
	    painter.setClipRegion(rgn);
	    painter.drawPixmap(tr,offscr, QRect(QPoint(0,0),area.size()));
	} else {
	    painter.translate(nrtr.x(),nrtr.y());
	    rgn.translate(rtr.x(),rtr.y());
	    painter.setClipRegion(rgn);
	    drawBackground(painter,area);
	    allvisible.drawUnique(painter);
	    drawForeground(painter,area);
	    painter.translate(-nrtr.x(),-nrtr.y());
	}
    }
}

/*!
\internal
This method to informs the QCanvas that a given chunk is
`dirty' and needs to be redrawn in the next Update.

(x,y) is a chunk location.

The sprite classes call this.  Any new derived class of QCanvasItem
must do so too.  SetChangedChunkContaining can be used instead.
*/
void QCanvas::setChangedChunk(int x, int y)
{
    if (validChunk(x,y)) {
	QCanvasChunk& ch=chunk(x,y);
	ch.change();
    }
}

/*!
\internal
This method to informs the QCanvas that the chunk containing a given
pixel is `dirty' and needs to be redrawn in the next Update.

(x,y) is a pixel location.

The item classes call this.  Any new derived class of QCanvasItem must
do so too. SetChangedChunk can be used instead.
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
This method adds a QCanvasItem to the list of those which need to be
drawn if the given chunk is redrawn.  Like SetChangedChunk and
SetChangedChunkContaining, this method marks the chunk as `dirty'.
*/
void QCanvas::addItemToChunk(QCanvasItem* g, int x, int y)
{
    if (validChunk(x,y)) {
	chunk(x,y).add(g);
    }
}

/*!
\internal
This method removes a QCanvasItem from the list of those which need to
be drawn if the given chunk is redrawn.  Like SetChangedChunk and
SetChangedChunkContaining, this method marks the chunk as `dirty'.
*/
void QCanvas::removeItemFromChunk(QCanvasItem* g, int x, int y)
{
    if (validChunk(x,y)) {
	chunk(x,y).remove(g);
    }
}


/*!
\internal
This method adds a QCanvasItem to the list of those which need to be
drawn if the chunk containing the given pixel is redrawn.  Like
SetChangedChunk and SetChangedChunkContaining, this method marks the
chunk as `dirty'.
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

/*! Returns the color set by setBackgroundColor().
  By default, this is white.

  Note that this function is not a reimplementation
  of QWidget::backgroundColor() (QCanvas is not a subclass
  of QWidget), but all QCanvasViews that are viewing the
  canvas will set their backgrounds to this

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
    if ( bgcolor != c ) {
	bgcolor = c;
	QCanvasView* view=d->viewList.first();
	while (view != 0 ) {
	    view->viewport()->setBackgroundColor(backgroundColor());
	    view=d->viewList.next();
	}
	setAllChanged();
    }
}

/*!  Returns the pixmap set by setBackgroundPixmap().  By default,
  this is a null pixmap.

  \sa setBackgroundPixmap(), backgroundColor()
*/
QPixmap QCanvas::backgroundPixmap() const
{
    return pm;
}

/*!  Sets the solid background to be \a p, repeated as necessary to
  cover the entire canvas.

  \sa backgroundPixmap(), setBackgroundColor(), setTiles()
*/
void QCanvas::setBackgroundPixmap( const QPixmap& p )
{
    setTiles(p, 1, 1, p.width(), p.height());
    QCanvasView* view=d->viewList.first();
    while (view != 0 ) {
	view->viewport()->setBackgroundPixmap(backgroundPixmap());
	view=d->viewList.next();
    }
}

/*!  This virtual function is called for all updates of the QCanvas.
  It renders any background graphics.  If the canvas has a background
  pixmap or a tiled background, that graphic is used, otherwise the
  canvas is cleared in the background color.

  If the graphics for an area change, you must explicitly call
  setChanged(const QRect&) for the result to be visible when update()
  is next called.

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

/*! This virtual function is called for all updates of the QCanvas.
  It renders any foreground graphics.

  The same warnings regarding change apply to this method as for
  drawBackground().

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

/*! Turns double-buffering on if \a y is TRUE, or off if it is
  FALSE. The default is to use double-buffering.

  Turning off double-buffering casuses the redrawn areas to flicker a
  bit.  This can help understand the the optimizations made by QCanvas
  and also gives a (usually small) performance improvement.
*/
void QCanvas::setDoubleBuffering(bool y)
{
    dblbuf = y;
}


/*!  Sets the QCanvas to be composed of \a h tiles horizontally and \a
  v tiles vertically.  Each tile will be an image \a tilewidth by \a
  tileheight pixels from pixmap \a p.

  The pixmap \a p contains the tiles arranged left to right, top to
  bottom, with tile 0 in the top-left corner, tile 1 to the right of
  tile 0, and so on.

  If the QCanvas is larger than the matrix of tiles, the entire matrix
  is repeated as necessary to cover the area.  If it is smaller, tiles
  to the right and bottom are not visible.

  The width and height of \a p must be multipless of \a tilewidth and
  \a tileheight. If they are not, the action of this function is
  unspecified.
*/
void QCanvas::setTiles( QPixmap p,
			int h, int v, int tilewidth, int tileheight )
{
    htiles = h;
    vtiles = v;
    delete[] grid;
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
    setAllChanged();
}

/*!
  \fn int QCanvas::tile( int x, int y ) const

  Returns the tile at (\a x, \a y). Initially, all tiles are 0.

  \warning The parameters must be within range.

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


/*!  Sets the tile at (\a x, \a y) to use tile number \a tilenum,
  which is an index into the tile pixmaps.  The canvas will update
  appropriately when update() is next called.

  The images are taken from the pixmap set by setTiles() and are
  arranged in the pixmap left to right, top to bottom, with tile 0 in
  the top-left corner, tile 1 next to the right, and so on.

  \sa tile() setTiles()
*/
void QCanvas::setTile( int x, int y, int tilenum )
{
    ushort& t = grid[x+y*htiles];
    if ( t != tilenum ) {
	t = tilenum;
	if ( tilew == tileh && tilew == chunksize )
	    setChangedChunk( x, y );	    // common case
	else
	    setChanged( QRect(x*tilew,y*tileh,tilew,tileh) );
    }
}


// lesser-used data in canvas item, plus room for extension.
// Be careful adding to this - check all usages.
class QCanvasItemExtra {
    QCanvasItemExtra() : vx(0.0), vy(0.0) { }
    double vx,vy;
    friend class QCanvasItem;
};


/*!
\class QCanvasItem qcanvas.h
\brief The QCanvasItem class provides an abstract graphic object on a QCanvas.
\module canvas

A variety of subclasses provide immediately usable behaviour; this
class is a pure abstract superclass providing the behaviour that is
shared among all the concrete item classes.

A QCanvasItem object can be moved in the x(), y() and z() dimensions
using functions such as move(), moveBy(), setX(), x() and many
others. It has a size given by boundingRect().  The item can move or
change appearance automatically, using setAnimated() and
setVelocity(), and you can get information about whether it collides
using collidesWith() and collisions().

It has the visibility functions like those in QWidget, including
show() and isVisible(), as well as some that are provided without
meaning, for subclasses to use as they please: setEnabled(),
setActive() and setSelected().

Finally, the rtti() function is used for identifying subclasses of
QCanvasItem, and the canvas() returns a pointer to the canvas on which
the item lives.

An item, by default, has no speed, no size, is not animated and has no
velocity. The subclasses provided in Qt do not change these defaults
except where expressly noted.

Note that you cannot easily subclass QCanvasItem yourself - the API is
too low-level.  It's usually much easier to subclass one of
QCanvasPolygonalItem, QCanvasRectangle, QCanvasSprite, QCanvasEllipse
or QCanvasText.
*/

/*!
Constructs a QCanvasItem on \a canvas.

\sa setCanvas()
*/
QCanvasItem::QCanvasItem(QCanvas* canvas) :
    cnv(canvas),
    myx(0),myy(0),myz(0)
{
    ani=0;
    vis=0;
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
    if (cnv) {
	cnv->removeItem(this);
	cnv->removeAnimation(this);
    }
    delete ext;
}

QCanvasItemExtra& QCanvasItem::extra()
{
    if ( !ext )
	ext = new QCanvasItemExtra;
    return *ext;
}

/*! \fn double QCanvasItem::x() const

  Returns the horizontal position of the item.  Note that subclasses
  often have an origin other than the top-left corner.
*/

/*! \fn double QCanvasItem::y() const

  Returns the vertical position of the item.  Note that subclasses
  often have an origin other than the top-left corner.
*/

/*! \fn double QCanvasItem::z() const

  Returns the z height of the item, which is used for visual order:
  higher-z items obscure lower-z ones.
*/

/*! \fn void QCanvasItem::setX(double x)

  Moves the item so that its X-position is \a x.
  \sa x(), move()
*/

/*! \fn void QCanvasItem::setY(double y)

  Moves the item so that its Y-position is \a y.
  \sa y(), move()
*/

/*! \fn void QCanvasItem::setZ(double z)

  Sets the height of the item to \a z.  Higher-z items obscure lower-z
  ones.

  \sa z(), move()
*/


/*! This virtual function moves the item from its current position by
  (\a dx, \a dy).
*/
void QCanvasItem::moveBy( double dx, double dy )
{
    if ( dx || dy ) {
	removeFromChunks();
	myx += dx;
	myy += dy;
	addToChunks();
    }
}


/*!  Moves the item to (\a x, \a y) by calling the moveBy() virtual
  function.
*/
void QCanvasItem::move( double x, double y )
{
    moveBy( x-myx, y-myy );
}


/*! Returns TRUE is the item is animated.

  \sa setVelocity(), setAnimated()
*/
bool QCanvasItem::animated() const
{
    return (bool)ani;
}

/*!  Sets the item to be animated if \a y is TRUE, or not if \a y is
  FALSE.

  \sa advance(), QCanvas::advance()
*/
void QCanvasItem::setAnimated(bool y)
{
    if ( y != (bool)ani ) {
	ani = (uint)y;
	if ( y ) {
	    cnv->addAnimation(this);
	} else {
	    cnv->removeAnimation(this);
	}
    }
}

/*! \fn void QCanvasItem::setXVelocity( double vx )

  Sets the horizontal component of the item's velocity to \a vx.
*/

/*! \fn void QCanvasItem::setYVelocity( double vy )

  Sets the vertical component of the item's velocity to \a vy.
*/

/*! Sets the item to be animated and moving by \a vx and \a vy pixels
  in the horizontal and vertical directions respectively.

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

/*!  Advances the animation of the item.  The default implementation
  moves the item by the preset velocity if \a phase is 1, and does
  nothing if \a phase is 0.

  Note that if you reimplement this funciton, you may not change the
  canvas in any way, add other items, or remove items.

  \sa QCanvas::advance()
*/
void QCanvasItem::advance(int phase)
{
    if ( ext && phase==1 )
	moveBy(ext->vx,ext->vy);
}

/*!
\fn void QCanvasItem::draw(QPainter& painter)

This abstract virtual function draws the item using \a painter.
*/

/*! Sets the QCanvas upon which the QCanvasItem is to be drawn to \a c.

\sa canvas()
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

/*! Shorthand for setVisible(TRUE). */
void QCanvasItem::show()
{
    setVisible(TRUE);
}

/*! Shorthand for setVisible(FALSE). */
void QCanvasItem::hide()
{
    setVisible(FALSE);
}

/*!  Makes the items visible if \a yes is TRUE, or invisible if \a yes
  is FALSE.  The change takes effect when QCanvas::update() is next
  called.
*/
void QCanvasItem::setVisible(bool yes)
{
    if ((bool)vis!=yes) {
	if (yes) {
	    vis=(uint)yes;
	    addToChunks();
	} else {
	    removeFromChunks();
	    vis=(uint)yes;
	}
    }
}
/*! \fn bool QCanvasItem::visible() const

  Returns TRUE if the QCanvasItem is visible.  This does \e not mean
  the QCanvasItem is currently in a view, merely that if a view is
  showing the area where the QCanvasItem is, and the item is not
  obscured by items at a higher z, and the view is not obscured by
  overlying windows, it would be visible.

  \sa setVisible(), z()
*/

/*! \fn bool QCanvasItem::selected() const

  Returns TRUE if the QCanvasItem is selected.
*/

/*! Sets the selected flag of the item to \a yes and causes it to be
  redrawn when QCanvas::update() is next called.

  The behavior of QCanvas, QCanvasItem or the built-in QCanvasItem
  subclasses is not affected by this value.  setSelected() is supplied
  because many applications need it, but it is up to the application
  to define its exact meaning.
*/
void QCanvasItem::setSelected(bool yes)
{
    if ((bool)sel!=yes) {
	sel=(uint)yes;
	changeChunks();
    }
}

/*!
  \fn bool QCanvasItem::enabled() const
  Returns TRUE if the QCanvasItem is enabled.
*/

/*!  Sets the enabled flag of the item to \a yes and causes it to be
  redrawn when QCanvas::update() is next called.

  The behavior of QCanvas, QCanvasItem or the built-in QCanvasItem
  subclasses is not affected by this value.  setEnabled() is supplied
  because many applications need it, but it is up to the application
  to define its exact meaning.
*/
void QCanvasItem::setEnabled(bool yes)
{
    if (ena!=(uint)yes) {
	ena=(uint)yes;
	changeChunks();
    }
}

/*!
  \fn bool QCanvasItem::active() const
  Returns TRUE if the QCanvasItem is active.
*/

/*!  Sets the active flag of the item to \a yes and causes it to be
  redrawn when QCanvas::update() is next called.

  The behavior of QCanvas, QCanvasItem or the built-in QCanvasItem
  subclasses is not affected by this value.  setActive() is supplied
  because many applications need it, but it is up to the application
  to define its exact meaning.
*/
void QCanvasItem::setActive(bool yes)
{
    if (act!=(uint)yes) {
	act=(uint)yes;
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

    // A non-linear search may be more efficient.
    // Perhaps spiralling out from the center, or a simpler
    // vertical expansion from the centreline.

    // We assume that sprite masks don't have
    // different bit orders.
    //
    // Q_ASSERT(s1image->bitOrder()==s2image->bitOrder());

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

static bool collision_double_dispatch( const QCanvasSprite* s1,
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
    } else if ( (r1 || t1 || s1) && (r2 || t2 || s2) ) {
	// b
	QRect rc1 = i1->boundingRectAdvanced();
	QRect rc2 = i2->boundingRectAdvanced();
	return rc1.intersects(rc2);
    } else if ( e1 && e2
		&& e1->angleLength()>=360*16 && e2->angleLength()>=360*16
		&& e1->width()==e1->height()
		&& e2->width()==e2->height() ) {
	// c
	double xd = (e1->x()+e1->xVelocity())-(e2->x()+e1->xVelocity());
	double yd = (e1->y()+e1->yVelocity())-(e2->y()+e1->yVelocity());
	double rd = (e1->width()+e2->width())/2;
	return xd*xd+yd*yd <= rd*rd;
    } else if ( p1 && (p2 || s2 || t2) ) {
	// d
	QPointArray pa1 = p1->areaPointsAdvanced();
	QPointArray pa2 = p2 ? p2->areaPointsAdvanced()
			  : QPointArray(i2->boundingRectAdvanced());
	bool col= !(QRegion(pa1) & QRegion(pa2,TRUE)).isEmpty();

	return col;
    } else {
	return collision_double_dispatch(s2,p2,r2,e2,t2,
					 s1,p1,r1,e1,t1);
    }
}

/*!
  \fn bool QCanvasItem::collidesWith( const QCanvasItem* other ) const

  Returns TRUE if the item will collide with the \a other item \e after they
  have moved by their current velocities.

  \sa collisions()
*/


/*!
  \class QCanvasSprite qcanvas.h
  \brief The QCanvasSprite class provides an animated moving pixmap on a QCanvas.
  \module canvas

  A "sprite" is an image object that moves around independently of
  foreground and background.  On a QCanvas, everything moves around
  independently of the foreground and background, so a QCanvasSprite
  is just a image whose API makes it simpler to use animation and a
  hot spot.

  QCanvasSprite draws very fast, at the cost of some memory.
*/


/*!
  \reimp
*/
bool QCanvasSprite::collidesWith( const QCanvasItem* i ) const
{
    return i->collidesWith(this,0,0,0,0);
}

/*!
  Returns TRUE if the item collides with any of the given items. The
  parameters are all the same object, this is just a type resolution
  trick.
*/


bool QCanvasSprite::collidesWith( const QCanvasSprite* s,
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

  In inexact mode, the items returned are only \e near the item and
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
QCanvasItemList QCanvas::collisions(const QPoint& p) const
{
    return collisions(QRect(p,QSize(1,1)));
}

/*!
  Returns a list of items which intersect with the rectangle \a r,
  sorted from shallowest to deepest.
*/
QCanvasItemList QCanvas::collisions(const QRect& r) const
{
    QCanvasRectangle i(r,(QCanvas*)this);
    i.setPen(NoPen);
    i.show(); // doesn't actually show, since we destroy it
    QCanvasItemList l = i.collisions(TRUE);
    l.sort();
    return l;
}

/*!
  Returns a list of items which intersect with the chunks listed
  in \a chunklist, excluding \a item.  If \a exact is TRUE, only
  only those which actually QCanvasItem::collidesWith() \a item
  are returned, otherwise items are included just for being in the
  chunks.

  This is a utility function mainly used to implement the simpler
  QCanvasItem::collisions() function.
*/
QCanvasItemList QCanvas::collisions(const QPointArray& chunklist,
	    const QCanvasItem* item, bool exact) const
{
    QPtrDict<void> seen;
    QCanvasItemList result;
    for (int i=0; i<(int)chunklist.count(); i++) {
	int x = chunklist[i].x();
	int y = chunklist[i].y();
	if ( validChunk(x,y) ) {
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
  Returns the bounding rectangle of pixels that the item \e will cover
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

// warwick - some parts of the text seems to imply that the offset is
// used for positioning; move( 42, 69 ) moves the _offset_ there, not
// the top-left corner.

/*!
  \class QCanvasPixmap qcanvas.h
  \brief The QCanvasPixmap class provides a pixmap in a canvas.
  \module canvas

  The pixmap a QPixmap, and can have a mask, just like QPixmap.  The
  only way to set it is in the constructor. There are three, one
  taking a QPixmap, one a QImage and one a file name that refers to a
  file in any supported file format (see QImageIO).

  Since QCanvasSprite needs (and other uses of pixmaps often need) a
  hot spot, QCanvasPixmap provides one. When you create the
  QCanvasPixmap from a PNG file or from a QImage that has a
  QImage::offset(), offset() is initialized appropriately, otherwise
  the constructor leaves it at (0,0). You can set it later using
  setOffset().

  Note that for QCanvasPixmap objects created by a QCanvasSprite, the
  position of each QCanvasPixmap object is set so that the hot spot
  stays in the same position.

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
QCanvasPixmap::QCanvasPixmap(const QPixmap& pm, const QPoint& offset)
{
    init(pm,offset.x(),offset.y());
}

void QCanvasPixmap::init(const QImage& image)
{
    convertFromImage(image);
    hotx = image.offset().x();
    hoty = image.offset().y();
    if( image.hasAlphaBuffer() ) {
	QImage i = image.createAlphaMask();
	collision_mask = new QImage(i);
    } else
	collision_mask = 0;
}

void QCanvasPixmap::init(const QPixmap& pixmap, int hx, int hy)
{
    (QPixmap&)*this = pixmap;
    hotx = hx;
    hoty = hy;
    if( pixmap.mask() )  {
	QImage i = mask()->convertToImage();
	collision_mask = new QImage(i);
    } else
	collision_mask = 0;
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
*/

/*!
  \class QCanvasPixmapArray qcanvas.h

  \brief The QCanvasPixmapArray class provides an array of
  QCanvasPixmap, e.g. for easy animation.

  \module canvas

  QCanvasSprite uses this class to hold its frames, see QCanvasSprite
  for details.
*/

/*!
  Constructs a null array.  You should call readPixmaps() before using it
  further.
*/
QCanvasPixmapArray::QCanvasPixmapArray()
{
    img = 0;
}

/*!
  Constructs a QCanvasPixmapArray from files.

  The \a fc parameter sets the number of frames to be loaded for this
  image, and is 0 by default.

  If \a fc is not 0, \a datafilenamepattern should contain "%1",
  e.g. "foo%1.png".  The actual filenames are formed by replacing the
  %1 with four-digit integers from 0 to fc-1, e.g. foo0000.png,
  foo0001.png, foo0002.png, etc.

  If \a fc is 0, \a datafilenamepattern is asssumed to be a real
  filename.

  If \a datafilenamepattern does not exist, is not readable, isn't an
  image, or some other error occurs, the array ends up empty.
*/

QCanvasPixmapArray::QCanvasPixmapArray( const QString& datafilenamepattern,
					int fc )
{
    img = 0;
    readPixmaps(datafilenamepattern,fc);
}

/*! Constructs a QCanvasPixmapArray from QPixmaps. */

QCanvasPixmapArray::QCanvasPixmapArray(QList<QPixmap> list, QList<QPoint> hotspots) :
    framecount(list.count()),
    img(new QCanvasPixmap*[list.count()])
{
    if (list.count() != hotspots.count()) {
	qWarning("QCanvasPixmapArray: lists have different lengths");
	reset();
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
    img = 0;
}

// ### the error behaviour here is probably a bug.

/*!
  If \a fc is not 0, \a datafilenamepattern should contain "%1",
  e.g. "foo%1.png".  The actual filenames are formed by replacing the
  %1 with four-digit integers from 0 to fc-1, e.g. foo0000.png,
  foo0001.png, foo0002.png, etc.

  If \a fc is 0, \a datafilenamepattern is asssumed to be a real
  filename.

  If \a datafilenamepattern does not exist, is not readable, isn't an
  image, or some other error occurs, the array ends up in an
  unspecified and probably useless state.
*/
bool QCanvasPixmapArray::readPixmaps( const QString& datafilenamepattern,
				      int fc)
{
    return readPixmaps(datafilenamepattern,fc,FALSE);
}

/*! Reads new collision masks for the array from \a fname, which must
  name an existing readable file containing frameCount() 1-bit images,
  or a series of files

  By default, QCanvasSprite uses the image mask of a sprite to detect
  collisions. This function lets you override that.

  If the file isn't readable, contains the wrong number of images, or
  there is some other error, the array is left in an an unspecified
  and probably useless state.
*/
bool QCanvasPixmapArray::readCollisionMasks(const QString& fname)
{
    return readPixmaps(fname,framecount,TRUE);
}


bool QCanvasPixmapArray::readPixmaps( const QString& datafilenamepattern,
				      int fc, bool maskonly)
{
    if ( !maskonly ) {
	delete [] img;
	framecount = fc;
	img = new QCanvasPixmap*[fc];
    }
    bool ok = TRUE;
    bool arg = framecount > 1;
    if ( !arg )
	framecount=1;
    for (int i=0; i<framecount; i++) {
	QString r;
	r.sprintf("%04d",i);
	if ( maskonly ) {
	    img[i]->collision_mask->load(
		arg ? datafilenamepattern.arg(r) : datafilenamepattern);
	    ok = ok
	       && !img[i]->collision_mask->isNull()
	       && img[i]->collision_mask->depth()==1;
	} else {
	    img[i]=new QCanvasPixmap(
		arg ? datafilenamepattern.arg(r) : datafilenamepattern);
	    ok = ok && !img[i]->isNull();
	}
    }
    if ( !ok ) {
	reset();
    }
    return ok;
}


// ### Warwick: I do not think such advanced C++ is good Qt style. We
// never use operator!() for similar purposes elsehwere, so this
// probably should be renamed to isValid().

/*!
  This returns FALSE if the array is valid, and TRUE if it is not.
*/
int QCanvasPixmapArray::operator!()
{
    return img==0;
}

// warwick - I guess this is right? Your text said indexing was
// 1-based, this text is 0-based.

/*!
\fn QCanvasPixmap* QCanvasPixmapArray::image(int i) const

Returns pixmap \a i in the array, if \a i is nonnegative and smaller
than count(), and returns an unspecified value otherwise.
*/

/*!
  Replaces the pixmap at index \a i by \a p.

  The array is extended to at least \a i+1 elements.

  Note that \a p becomes owned by this array, which will delete \a p
  when it is itself deleted.
*/
void QCanvasPixmapArray::setImage(int i, QCanvasPixmap* p)
{
    if ( i >= framecount ) {
	QCanvasPixmap** newimg = new QCanvasPixmap*[i+1];
	memcpy(newimg, img, sizeof(newimg[0])*framecount);
	framecount = i+1;
	delete [] img;
	img = newimg;
    }
    delete img[i]; img[i]=p;
}

/*!
\fn uint QCanvasPixmapArray::count() const

Returns the number of pixmaps in the array.
*/

/*! Returns the x coordinate of the current left edge of the
  sprite. (This may change as the sprite animates.)

  \sa offset() pos()
*/
int QCanvasSprite::leftEdge() const
{
    return int(x()) - image()->hotx;
}

/*!  Returns what the x coordinate of the left edge of the sprite
  would be if it (its hot spot) were moved to x position \a nx.

  \sa offset() pos()
*/
int QCanvasSprite::leftEdge(int nx) const
{
    return nx-image()->hotx;
}

/*!  Returns the y coordinate of the top edge of the sprite. (This may
  change as the sprite animates.)

  \sa offset() pos()
*/
int QCanvasSprite::topEdge() const
{
    return int(y())-image()->hoty;
}

/*!  Returns what the y coordinate of the top edge of the sprite
  would be if it (its hot spot) were moved to y position \a ny.

  \sa offset() pos()
*/
int QCanvasSprite::topEdge(int ny) const
{
    return ny-image()->hoty;
}

/*! Returns the x coordinate of the current right edge of the
  sprite. (This may change as the sprite animates.)

  \sa offset() pos()
*/
int QCanvasSprite::rightEdge() const
{
    return leftEdge()+image()->width()-1;
}

/*!  Returns what the x coordinate of the right edge of the sprite
  would be if it (its hot spot) were moved to x position \a nx.

  \sa offset() pos()
*/
int QCanvasSprite::rightEdge(int nx) const
{
    return leftEdge(nx)+image()->width()-1;
}

/*! Returns the y coordinate of the current bottom edge of the
  sprite. (This may change as the sprite animates.)

  \sa offset() pos()
*/
int QCanvasSprite::bottomEdge() const
{
    return topEdge()+image()->height()-1;
}

/*!  Returns what the y coordinate of the top edge of the sprite
  would be if it (its hot spot) were moved to y position \a ny.

  \sa offset() pos()
*/
int QCanvasSprite::bottomEdge(int ny) const
{
    return topEdge(ny)+image()->height()-1;
}

/*!
  \fn QCanvasPixmap* QCanvasSprite::image() const
  Returns the current image frame.
  \sa frame(), setFrame()
*/

/*!
  \fn QCanvasPixmap* QCanvasSprite::image(int f) const
  Returns image frame \a f.
*/

/*!
  The image the sprite \e will have after advance(1) is called.
  Be default this is the same as image().
*/
QCanvasPixmap* QCanvasSprite::imageAdvanced() const
{
    return image();
}

/*!
  Returns the bounding rectangle of pixels covered by the sprite.
  This assumes that the images are tightly cropped (ie. do not have
  transparent pixels all along a side).
*/
QRect QCanvasSprite::boundingRect() const
{
    return QRect(leftEdge(), topEdge(), width(), height());
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
	if ( br.isValid() ) {
	    r.resize((br.width()/chunksize+2)*(br.height()/chunksize+2));
	    for (int j=br.top()/chunksize; j<=br.bottom()/chunksize; j++) {
		for (int i=br.left()/chunksize; i<=br.right()/chunksize; i++) {
		    r[n++] = QPoint(i,j);
		}
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
	for (int j=topEdge()/chunksize; j<=bottomEdge()/chunksize; j++) {
	    for (int i=leftEdge()/chunksize; i<=rightEdge()/chunksize; i++) {
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
	for (int j=topEdge()/chunksize; j<=bottomEdge()/chunksize; j++) {
	    for (int i=leftEdge()/chunksize; i<=rightEdge()/chunksize; i++) {
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
    painter.drawPixmap(leftEdge(),topEdge(),*image());
}

/*!
  \class QCanvasView qcanvas.h
  \brief The QCanvasView class provides an on-screen view of a QCanvas.
  \module canvas

  Displays a view of a QCanvas, with scrollbars available if
  desired. There can be more than one view of a canvas.

  The view of a canvas is the object which the user can see and
  interact with, hence any interactivity will be based on events from
  a view. For example, by subclassing QCanvasView and overriding
  QScrollView::contentsMousePressEvent(), an application can provide a
  canvas where the user can interact with items on the canvas.

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
  Constructs a QCanvasView.  You will probably want to call setCanvas() before
  showing the widget.

  The usual QWidget parameters may also be supplied.
*/
QCanvasView::QCanvasView(QWidget* parent, const char* name, WFlags f) :
    QScrollView(parent,name,f)
{
    d = new Data;
    viewing = 0;
    setCanvas(0);
    connect(this,SIGNAL(contentsMoving(int,int)),this,SLOT(cMoving(int,int)));
}

/*!
  Constructs a QCanvasView which views \a canvas.  The
  usual QWidget parameters may also be supplied.
*/
QCanvasView::QCanvasView(QCanvas* canvas, QWidget* parent, const char* name, WFlags f) :
    QScrollView(parent,name,f)
{
    d = new Data;
    viewing = 0;
    setCanvas(canvas);

    connect(this,SIGNAL(contentsMoving(int,int)),this,SLOT(cMoving(int,int)));
}

/*!
  Destructs the view. The associated canvas is \e not deleted.
*/
QCanvasView::~QCanvasView()
{
    delete d;
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
	disconnect(viewing);
	viewing->removeView(this);
    }
    viewing=canvas;
    if (viewing) {
	connect(viewing,SIGNAL(resized()), this, SLOT(updateContentsSize()));
	viewing->addView(this);
    }
    updateContentsSize();
}

/*!
  Returns the current transformation matrix.

  \sa setWorldMatrix()
*/
const QWMatrix &QCanvasView::worldMatrix() const
{
    return d->xform;
}

/*!
  Returns the inverse of the current transformation matrix.

  \sa setWorldMatrix()
*/
const QWMatrix &QCanvasView::inverseWorldMatrix() const
{
    return d->ixform;
}

/*!
  Sets the transform of the view to \a wm. Note that performance
  drops considerably when this is used.

  The matrix must be invertible.

  \sa worldMatrix()
*/
void QCanvasView::setWorldMatrix( const QWMatrix & wm )
{
    d->xform = wm;
    d->ixform = wm.invert();
    updateContentsSize();
    viewport()->update();
}

void QCanvasView::updateContentsSize()
{
    if ( viewing ) {
	QRect br = d->xform.map(QRect(0,0,viewing->width(),viewing->height()));
	resizeContents(br.width(),br.height());
    } else {
	resizeContents(1,1);
    }
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
	if ( !d->xform.isIdentity() ) {
	    viewing->drawViewArea(this,p,r,FALSE);
	} else {
	    viewing->drawCanvasArea(r,p,!repaint_from_moving);
	}
	repaint_from_moving = FALSE;
    } else {
	p->eraseRect(r);
    }
}

/*!
  Suggests a size sufficient to view the entire canvas.
  \internal
  Why not like this in QScrollView?
*/
QSize QCanvasView::sizeHint() const
{
    if ( !canvas() )
	return QScrollView::sizeHint();
    return canvas()->size()+QSize(frameWidth(),frameWidth())*2;
}


/*!
  \class QCanvasPolygonalItem qcanvas.h
  \brief The QCanvasPolygonalItem class provides a polygonal object in
  a QCanvas.
  \module canvas

  The mostly rectangular classes QCanvasSprite and QCanvasText use the
  object's bounding rectangle for movement, repainting and collision
  calculation. However, for most other items, the bounding rectangle
  can be far too large - a diagonal line is the worst case, but there
  are many others that are very bad.

  QCanvasPolygonalItem provides polygon-based handling of bounding
  rectangles, etc, much speeding up such cases.

  Derived classes should try to define as small as possible an area to
  maximize efficiency, but must \e definitely be contained completely
  within the polygonal area.  Calculating the exact requirements is
  usually difficult, but if you allow a small overestimate it can be
  easy and quick, while still getting almost all of
  QCanvasPolygonalItem's speed.

  Note that all subclasses must call hide() in their destructors while
  the functions numAreaPoints() and getAreaPoints() still are valid.

  Normally, QCanvasPolygonalItem uses the odd-even algorithm for
  determining whether an object intersects this object. You can change
  that using setWinding().

  By default, QCanvasPolygonalItem objects have no pen and a black brush.
  You can change this with setPen() and setBrush(), but note that most
  QCanvasPolygonalItem subclasses only use the brush, ignoring the pen
  setting.
*/


/*
  Since most polygonal items don't have a pen, the default is
  NoPen and a black brush.
*/
static const QPen& defaultPolygonPen()
{
    static QPen* dp=0;
    if ( !dp )
	dp = new QPen(Qt::NoPen);
    return *dp;
}
static const QBrush& defaultPolygonBrush()
{
    static QBrush* db=0;
    if ( !db )
	db = new QBrush(Qt::black);
    return *db;
}

/*!
  Constructs a QCanvasPolygonalItem on \a canvas.
*/
QCanvasPolygonalItem::QCanvasPolygonalItem(QCanvas* canvas) :
    QCanvasItem(canvas),
    br(defaultPolygonBrush()),
    pn(defaultPolygonPen())
{
    wind=0;
}

/*! Destruct the QCanvasPolygonalItem.  Derived classes \e must call
  hide() in their destructor, as this destructor cannot call
  virtual methods such as that.
*/
QCanvasPolygonalItem::~QCanvasPolygonalItem()
{
}

/*! Returns TRUE if the polygonal item uses the \e winding algorithm
  for determine the "inside" of the polygon, of FALSE if it uses the
  odd-even algorithm.

  The default is to use the odd-even algorithm.

  \sa setWinding()
*/
bool QCanvasPolygonalItem::winding() const
{
    return wind;
}

/*! Sets whether the polygonal item to use \e winding algorithm for
  determine the "inside" of the polygon, rather than the odd-even
  algorithm.

  The default is to use the odd-even algorithm.

  \sa winding()
*/
void QCanvasPolygonalItem::setWinding(bool enable)
{
    wind = enable;
}


/*!
  Returns the points advanced by the current xVelocity() and yVelocity().
*/
QPointArray QCanvasPolygonalItem::areaPointsAdvanced() const
{
    int dx = int(x()+xVelocity())-int(x());
    int dy = int(y()+yVelocity())-int(y());
    QPointArray r = areaPoints();
    if ( dx || dy )
	r.translate(dx,dy);
    return r;
}

//#define QCANVAS_POLYGONS_DEBUG
#ifdef QCANVAS_POLYGONS_DEBUG
static QWidget* dbg_wid=0;
static QPainter* dbg_ptr=0;
#endif

class QPolygonalProcessor {
public:
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
#ifdef QCANVAS_POLYGONS_DEBUG
	dbg_start();
#endif
    }

    inline void add(int x, int y)
    {
	if ( pnt >= (int)result.size() ) {
	    result.resize(pnt*2+10);
	}
	result[pnt++] = QPoint(x+bounds.x(),y+bounds.y());
#ifdef QCANVAS_POLYGONS_DEBUG
	if ( dbg_ptr ) {
	    int cs = canvas->chunkSize();
	    QRect r(x*cs+bounds.x()*cs,y*cs+bounds.y()*cs,cs-1,cs-1);
	    dbg_ptr->setPen(Qt::blue);
	    dbg_ptr->drawRect(r);
	}
#endif
    }

    inline void addBits(int x1, int x2, uchar newbits, int xo, int yo)
    {
	for (int i=x1; i<=x2; i++)
	    if ( newbits & (1<<i) )
		add(xo+i,yo);
    }

#ifdef QCANVAS_POLYGONS_DEBUG
    void dbg_start()
    {
	if ( !dbg_wid ) {
	    dbg_wid = new QWidget;
	    dbg_wid->resize(800,600);
	    dbg_wid->show();
	    dbg_ptr = new QPainter(dbg_wid);
	    dbg_ptr->setBrush(Qt::NoBrush);
	}
	dbg_ptr->fillRect(dbg_wid->rect(),Qt::white);
    }
#endif

    void doSpans(int n, QPoint* pt, int* w)
    {
	int cs = canvas->chunkSize();
	for (int j=0; j<n; j++) {
	    int y = pt[j].y()/cs-bounds.y();
	    uchar* l = bitmap.scanLine(y);
	    int x = pt[j].x();
	    int x1 = x/cs-bounds.x();
	    int x2 = (x+w[j])/cs-bounds.x();
	    int x1q = x1/8;
	    int x1r = x1%8;
	    int x2q = x2/8;
	    int x2r = x2%8;
#ifdef QCANVAS_POLYGONS_DEBUG
	    if ( dbg_ptr ) dbg_ptr->setPen(Qt::yellow);
#endif
	    if ( x1q == x2q ) {
		uchar newbits = (~l[x1q]) & (((2<<(x2r-x1r))-1)<<x1r);
		if ( newbits ) {
#ifdef QCANVAS_POLYGONS_DEBUG
		    if ( dbg_ptr ) dbg_ptr->setPen(Qt::darkGreen);
#endif
		    addBits(x1r,x2r,newbits,x1q*8,y);
		    l[x1q] |= newbits;
		}
	    } else {
#ifdef QCANVAS_POLYGONS_DEBUG
		if ( dbg_ptr ) dbg_ptr->setPen(Qt::blue);
#endif
		uchar newbits1 = (~l[x1q]) & (0xff<<x1r);
		if ( newbits1 ) {
#ifdef QCANVAS_POLYGONS_DEBUG
		    if ( dbg_ptr ) dbg_ptr->setPen(Qt::green);
#endif
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
#ifdef QCANVAS_POLYGONS_DEBUG
		    if ( dbg_ptr ) dbg_ptr->setPen(Qt::red);
#endif
		    addBits(0,x2r,newbits2,x2q*8,y);
		    l[x2q] |= newbits2;
		}
	    }
#ifdef QCANVAS_POLYGONS_DEBUG
	    if ( dbg_ptr ) {
		dbg_ptr->drawLine(pt[j],pt[j]+QPoint(w[j],0));
	    }
#endif
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

    scanPolygon(pa, wind, processor);

    return processor.result;
}

QPointArray QCanvasRectangle::chunks() const
{
    // No need to do a polygon scan!
    return QCanvasItem::chunks();
}

/*!
  Returns the bounding rectangle of the polygonal item,
  based on areaPoints().
*/
QRect QCanvasPolygonalItem::boundingRect() const
{
    return areaPoints().boundingRect();
}

/*!
  Reimplemented from QCanvasItem, this draws the item by setting the
  pen and brush on \a p and calling drawShape().
*/
void QCanvasPolygonalItem::draw(QPainter & p)
{
    p.setPen(pn);
    p.setBrush(br);
    drawShape(p);
}

/*!
  \fn void QCanvasPolygonalItem::drawShape(QPainter & p)

  Subclasses must reimplement this function to draw their shape. The
  pen and brush of \a p are already set to pen() and brush() prior to
  calling this function.

  \sa draw()
*/

/*!
  \fn QPen QCanvasPolygonalItem::pen() const

  Returns the QPen used to draw the outline of the item, if any.

  \sa setPen()
*/

/*!
  \fn QBrush QCanvasPolygonalItem::brush() const

  Returns the QBrush used to fill the item, if filled.

  \sa setBrush()
*/

/*!
  Sets the QPen used when drawing the item.
  Note that many QCanvasPolygonalItem do not use the pen value.

  \sa setBrush(), pen(), drawShape()
*/
void QCanvasPolygonalItem::setPen(QPen p)
{
    if ( pn != p ) {
	pn = p;
	changeChunks();
    }
}

/*!
  Sets the QBrush used when drawing item.

  \sa setPen(), brush(), drawShape()
*/
void QCanvasPolygonalItem::setBrush(QBrush b)
{
    if ( br != b) {
	br = b;
	changeChunks();
    }
}


/*!
  \class QCanvasPolygon qcanvas.h
  \brief The QCanvasPolygon class provides a movable polygon in a canvas.
  \module canvas

  Paints a polygon in a QBrush.
*/

/*!
  Constructs a pointless polygon on \a canvas.
  You should call setPoints() before
  using it further.
*/
QCanvasPolygon::QCanvasPolygon(QCanvas* canvas) :
    QCanvasPolygonalItem(canvas)
{
}

/*!
  Destructs the polygon.
*/
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

/*!
  Sets the points of the polygon to be \a pa, which will be translated
  by x(), y() as the polygon is moved.
*/
void QCanvasPolygon::setPoints(QPointArray pa)
{
    removeFromChunks();
    poly = pa;
    poly.translate((int)x(),(int)y());
    addToChunks();
}

/*!
  \reimp
*/
void QCanvasPolygon::moveBy(double dx, double dy)
{
    // Note: does NOT call QCanvasPolygonalItem::moveBy(), since that
    // only does half this work.
    //
    int idx = int(x()+dx)-int(x());
    int idy = int(y()+dy)-int(y());
    if ( idx || idy ) {
	removeFromChunks();
	poly.translate(idx,idy);
    }
    myx+=dx;
    myy+=dy;
    if ( idx || idy ) {
	addToChunks();
    }
}

/*!
  \class QCanvasSpline qcanvas.h
  \brief The QCanvasSpline class provides a multi-bezier spline.
  \module canvas

  A QCanvasSpline is a sequence of 4-point bezier curves joined
  together to make a curved shape.

  You set the control points of the spline with setControlPoints().

  If the bezier is closed(), then the first control point will be
  re-used as the last control point. Therefore, a closed bezier must
  have a multiple of 3 control points and an open bezier must have
  one extra.

  The beziers are not necessarily joined "smoothly". To ensure this,
  set control points appropriately (general references about beziers
  will explain this in detail).
*/

/*!
  Create a spline with no control points.

  \sa setControlPoints()
*/
QCanvasSpline::QCanvasSpline(QCanvas* canvas) :
    QCanvasPolygon(canvas),
    cl(TRUE)
{
}

/*!
  Destruct the spline.
*/
QCanvasSpline::~QCanvasSpline()
{
}

/*!
  Set the spline control points to \a ctrl.

  If \a close is TRUE,
  then the first point in \a ctrl will be re-used as the last point,
  and there must be a multiple of 3 control points (6, 9, 12, ...).

  If \a close is FALSE,
  one additional control point is required (4, 7, 11, ...).
*/
void QCanvasSpline::setControlPoints(QPointArray ctrl, bool close)
{
    ASSERT( (int)ctrl.count() % 3 == ( close ? 0 : 1 ) );
    cl = close;
    bez = ctrl;
    recalcPoly();
}

/*!
  Returns the current set control points.

  \sa setControlPoints(), closed()
*/
QPointArray QCanvasSpline::controlPoints() const
{
    return bez;
}

/*!
  Returns whether the control points are considered a closed set.
*/
bool QCanvasSpline::closed() const
{
    return cl;
}

void QCanvasSpline::recalcPoly()
{
    QList<QPointArray> segs;
    segs.setAutoDelete(TRUE);
    int n=0;
    for (int i=0; i<(int)bez.count()-1; i+=3) {
	QPointArray ctrl(4);
	ctrl[0] = bez[i+0];
	ctrl[1] = bez[i+1];
	ctrl[2] = bez[i+2];
	if ( cl )
	    ctrl[3] = bez[(i+3)%(int)bez.count()];
	else
	    ctrl[3] = bez[i+3];
	QPointArray *seg = new QPointArray(ctrl.cubicBezier());
	n += seg->count()-1;
	segs.append(seg);
    }
    QPointArray p(n+1);
    n=0;
    for (QPointArray* seg = segs.first(); seg; seg = segs.next()) {
	for (int i=0; i<(int)seg->count()-1; i++)
	    p[n++] = seg->point(i);
	if ( n == (int)p.count()-1 )
	    p[n] = seg->point(seg->count()-1);
    }
    QCanvasPolygon::setPoints(p);
}


/*!
  \fn QPointArray QCanvasPolygonalItem::areaPoints() const

  Must return the points bounding the shape.  Note that the returned
  points are \e outside the object, not touching it.
*/

/*!
  \fn QPointArray QCanvasPolygon::points() const

  Returns the vertices of the polygon, not translated by the position.

  \sa setPoints(), areaPoints()
*/
QPointArray QCanvasPolygon::points() const
{
    QPointArray pa = areaPoints();
    pa.translate(int(-x()),int(-y()));
    return pa;
}

/*!
  Returns the vertices of the polygon translated by the x(), y() position.

  \sa setPoints(), points()
*/
QPointArray QCanvasPolygon::areaPoints() const
{
    return poly;
}

/*!
  \class QCanvasLine qcanvas.h
  \brief The QCanvasLine class provides a simple line on a canvas.
  \module canvas

  The setPen() function

  Currently, only solid lines with width 1 are supported.
*/

/*!
  Constructs a line from (0,0) to (0,0) on \a canvas.

  \sa setPoints().
*/
QCanvasLine::QCanvasLine(QCanvas* canvas) :
    QCanvasPolygonalItem(canvas)
{
    x1 = y1 = x2 = y2 = 0;
}

/*!
  Destroys the line, removing it from its canvas.
*/
QCanvasLine::~QCanvasLine()
{
    hide();
}

/*!
  \reimp
*/
void QCanvasLine::setPen(QPen p)
{
    if ( pen() != p ) {
	removeFromChunks();
	QCanvasPolygonalItem::setPen(p);
	addToChunks();
    }
}

/*!
  \fn QPoint QCanvasLine::startPoint () const

  Returns the start of the line.

  \sa setPoints(), endPoint()
*/

/*!
  \fn QPoint QCanvasLine::endPoint () const

  Returns the end of the line.

  \sa setPoints(), startPoint()
*/

/*!
  Sets the ends of the line to (\a xa, \a ya) and (\a xb, \a yb).
*/
void QCanvasLine::setPoints(int xa, int ya, int xb, int yb)
{
    if ( x1 != xa || x2 != xb || y1 != ya || y2 != yb ) {
	removeFromChunks();
	x1 = xa;
	y1 = ya;
	x2 = xb;
	y2 = yb;
	addToChunks();
    }
}

/*!
  \reimp
*/
void QCanvasLine::drawShape(QPainter &p)
{
    p.drawLine((int)(x()+x1), (int)(y()+y1), (int)(x()+x2), (int)(y()+y2));
}

/*!
  \reimp

  Note that the area defined by the line is somewhat thicker than the
  line that is actually drawn.
*/
QPointArray QCanvasLine::areaPoints() const
{
    QPointArray p(4);
    int xi = int(x());
    int yi = int(y());
    int pw = pen().width();
    int dx = QABS(x1-x2);
    int dy = QABS(y1-y2);
    if ( pw <= 1 ) {
	// thin
	if ( dx > dy ) {
	    p[0] = QPoint(x1+xi,y1+yi-1);
	    p[1] = QPoint(x2+xi,y2+yi-1);
	    p[2] = QPoint(x2+xi,y2+yi+1);
	    p[3] = QPoint(x1+xi,y1+yi+1);
	} else {
	    p[0] = QPoint(x1+xi-1,y1+yi);
	    p[1] = QPoint(x2+xi-1,y2+yi);
	    p[2] = QPoint(x2+xi+1,y2+yi);
	    p[3] = QPoint(x1+xi+1,y1+yi);
	}
    } else {
	pw = pw*4/3+2; // approx pw*sqrt(2)
	int px = x1<x2 ? -pw : pw ;
	int py = y1<y2 ? -pw : pw ;
	if ( dx && dy && (dx > dy ? (dx*2/dy <= 2) : (dy*2/dx <= 2)) ) {
	    // steep
	    if ( px == py ) {
		p[0] = QPoint(x1+xi   ,y1+yi+py);
		p[1] = QPoint(x2+xi-px,y2+yi   );
		p[2] = QPoint(x2+xi   ,y2+yi-py);
		p[3] = QPoint(x1+xi+px,y1+yi   );
	    } else {
		p[0] = QPoint(x1+xi+px,y1+yi   );
		p[1] = QPoint(x2+xi   ,y2+yi-py);
		p[2] = QPoint(x2+xi-px,y2+yi   );
		p[3] = QPoint(x1+xi   ,y1+yi+py);
	    }
	} else if ( dx > dy ) {
	    // horizontal
	    p[0] = QPoint(x1+xi+px,y1+yi+py);
	    p[1] = QPoint(x2+xi-px,y2+yi+py);
	    p[2] = QPoint(x2+xi-px,y2+yi-py);
	    p[3] = QPoint(x1+xi+px,y1+yi-py);
	} else {
	    // vertical
	    p[0] = QPoint(x1+xi+px,y1+yi+py);
	    p[1] = QPoint(x2+xi+px,y2+yi-py);
	    p[2] = QPoint(x2+xi-px,y2+yi-py);
	    p[3] = QPoint(x1+xi-px,y1+yi+py);
	}
    }
    return p;
}

/*!
  \class QCanvasRectangle qcanvas.h
  \brief The QCanvasRectangle provides a movable rectangle in a canvas
  \module canvas

  This item paints a single rectangle which may have any pen() and
  brush(), but may not be tilted/rotated.

  For rotated rectangles, use QCanvasPolygon.
*/

/*!
  Constructs a rectangle (0,0,32,32) on \a canvas.
*/
QCanvasRectangle::QCanvasRectangle(QCanvas* canvas) :
    QCanvasPolygonalItem(canvas),
    w(32), h(32)
{
}

/*!
  Constructs a rectangle positioned and sized by \a r on \a canvas.
*/
QCanvasRectangle::QCanvasRectangle(const QRect& r, QCanvas* canvas) :
    QCanvasPolygonalItem(canvas),
    w(r.width()), h(r.height())
{
    move(r.x(),r.y());
}

/*!
  Constructs a rectangle with position \a x, \a y
  and size \a width by \a height, on \a canvas.
*/
QCanvasRectangle::QCanvasRectangle(int x, int y, int width, int height,
	QCanvas* canvas) :
    QCanvasPolygonalItem(canvas),
    w(width), h(height)
{
    move(x,y);
}

/*!
  Destructs the rectangle.
*/
QCanvasRectangle::~QCanvasRectangle()
{
    hide();
}


/*!
  Returns the width of the rectangle.
*/
int QCanvasRectangle::width() const
{
    return w;
}

/*!
  Returns the height of the rectangle.
*/
int QCanvasRectangle::height() const
{
    return h;
}

/*!
  Sets the \a width and \a height of the rectangle.
*/
void QCanvasRectangle::setSize(int width, int height)
{
    if ( w != width || h != height ) {
	removeFromChunks();
	w = width;
	h = height;
	addToChunks();
    }
}

/*!
  \fn QSize QCanvasRectangle::size() const

  Returns the width() and height() of the rectangle.

  \sa rect(), setSize()
*/

/*!
  \fn QRect QCanvasRectangle::rect() const
  Returns the integer-converted x(), y() position and size() of the rectangle
  as a QRect.
*/

/*!
  \reimp
*/
QPointArray QCanvasRectangle::areaPoints() const
{
    QPointArray pa(4);
    int pw = (pen().width()+1)/2;
    if ( pw < 1 ) pw = 1;
    if ( pen() == NoPen ) pw = 0;
    pa[0] = QPoint((int)x()-pw,(int)y()-pw);
    pa[1] = pa[0] + QPoint(w+pw*2,0);
    pa[2] = pa[1] + QPoint(0,h+pw*2);
    pa[3] = pa[0] + QPoint(0,h+pw*2);
    return pa;
}

/*!
  Draws the rectangle on \a p.
*/
void QCanvasRectangle::drawShape(QPainter & p)
{
    p.drawRect((int)x(), (int)y(), w, h);
}


/*!
  \class QCanvasEllipse qcanvas.h
  \brief An ellipse with a movable center point.
  \module canvas

  Paints an ellipse or "pie segment" with a QBrush.
*/

/*!
  Constructs a 32x32 ellipse, centered at (0,0) on \a canvas.
*/
QCanvasEllipse::QCanvasEllipse(QCanvas* canvas) :
    QCanvasPolygonalItem(canvas),
    w(32), h(32),
    a1(0), a2(360*16)
{
}

/*!
  Constructs a \a width by \a height pixel ellipse, centered at (0,0)
  on \a canvas.
*/
QCanvasEllipse::QCanvasEllipse(int width, int height, QCanvas* canvas) :
    QCanvasPolygonalItem(canvas),
    w(width),h(height),
    a1(0),a2(360*16)
{
}

/*!
  Constructs a \a width by \a height pixel ellipse, centered at (0,0)
  on \a canvas, starting at angle \a startangle, extending for angle
  \a angle.  \a startangle and \a angle are in 1/16 degrees.
*/
QCanvasEllipse::QCanvasEllipse(int width, int height,
    int startangle, int angle, QCanvas* canvas) :
    QCanvasPolygonalItem(canvas),
    w(width),h(height),
    a1(startangle),a2(angle)
{
}

/*!
  Destructs the ellipse.
*/
QCanvasEllipse::~QCanvasEllipse()
{
    hide();
}

/*!
  Returns the width of the ellipse.
*/
int QCanvasEllipse::width() const
{
    return w;
}

/*!
  Returns the height of the ellipse.
*/
int QCanvasEllipse::height() const
{
    return h;
}

/*!
  Sets the \a width and \a height of the ellipse.
*/
void QCanvasEllipse::setSize(int width, int height)
{
    if ( w != width || h != height ) {
	removeFromChunks();
	w = width;
	h = height;
	addToChunks();
    }
}

/*!
  \fn int QCanvasEllipse::angleStart() const

  Returns the start angle in 1/16 degrees.  Initially this will be 0.

  \sa setAngles(), angleLength()
*/

/*!
  \fn int QCanvasEllipse::angleLength() const

  Returns the length angle in 1/16 degrees.  Initially this will be
  360*16 - ie. a solid ellipse.

  \sa setAngles(), angleStart()
*/

/*!
  Sets the angles for the ellipse to start at \a start and continue
  for \a length units.  Each unit is 1/16 of a degree. By default
  the ellipse will start at 0 and have length 360*16 - ie. a solid
  ellipse.

  \sa angleStart(), angleLength()
*/
void QCanvasEllipse::setAngles(int start, int length)
{
    if ( a1 != start || a2 != length ) {
	removeFromChunks();
	a1 = start;
	a2 = length;
	addToChunks();
    }
}

/*!
  \reimp
*/
QPointArray QCanvasEllipse::areaPoints() const
{
    QPointArray r;
    r.makeArc(int(x()-w/2.0-1),int(y()-h/2.0-1),w+2,h+3,a1,a2);
    r.resize(r.size()+1);
    r.setPoint(r.size()-1,x(),y());
    return r;
}

/*!
  Draws the ellipse, centered at x(), y().

  Note that QCanvasPolygon does not support an outline (pen is
  always NoPen).
*/
void QCanvasEllipse::drawShape(QPainter & p)
{
    p.setPen(NoPen); // since QRegion(QPointArray) excludes outline :-(  )-:
    if ( !a1 && a2 == 360*16 ) {
	p.drawEllipse(int(x()-w/2.0+0.5), int(y()-h/2.0+0.5), w, h);
    } else {
	p.drawPie(int(x()-w/2.0+0.5), int(y()-h/2.0+0.5), w, h, a1, a2);
    }
}


/*!
\class QCanvasText qcanvas.h
\brief A text object on a QCanvas.
\module canvas

  A QCanvasText has text, a font, color, and position.
*/

/*!
  Constructs a QCanvasText with the text "<text>", on \a canvas.
*/
QCanvasText::QCanvasText(QCanvas* canvas) :
    QCanvasItem(canvas),
    txt("<text>"), flags(0)
{
    setRect();
}

/*!
  Constructs a QCanvasText with the text \a t, on \a canvas.

  The text should not contain newlines.
*/
QCanvasText::QCanvasText(const QString& t, QCanvas* canvas) :
    QCanvasItem(canvas),
    txt(t), flags(0)
{
    setRect();
}

/*!
  Constructs a QCanvasText with the text \a t and font \a f, on \a canvas.

  The text should not contain newlines.
*/
QCanvasText::QCanvasText(const QString& t, QFont f, QCanvas* canvas) :
    QCanvasItem(canvas),
    txt(t), flags(0),
    fnt(f)
{
    setRect();
}

/*!
  Destruct the sprite.
*/
QCanvasText::~QCanvasText()
{
    removeFromChunks();
}

/*!
  Returns the bounding rectangle of the text.
*/
QRect QCanvasText::boundingRect() const { return brect; }

void QCanvasText::setRect()
{
    static QWidget *w=0;
    if (!w) w = new QWidget;
    QPainter p(w);
    p.setFont(fnt);
    brect = p.boundingRect(QRect(int(x()),int(y()),0,0), flags, txt);
}

/*!
  \fn int QCanvasText::textFlags() const
  Returns the currently set alignment flags.
  \sa setTextFlags(), Qt::AlignmentFlags
*/


/*!
  Sets the alignment flags.  These are a bitwise OR or \e some of the
  flags available to QPainter::drawText() - see Qt::AlignmentFlags.

  The DontClip and WordBreak flags are not supported.
*/
void QCanvasText::setTextFlags(int f)
{
    if ( flags != f ) {
	removeFromChunks();
	flags = f;
	setRect();
	addToChunks();
    }
}

/*!
  Returns the text to be displayed.

  \sa setText()
*/
QString QCanvasText::text() const
{
    return txt;
}


/*!
  Sets the text to be displayed.  The text may contain newlines.

  \sa text(), setFont(), setColor()
*/
void QCanvasText::setText( const QString& t )
{
    if ( txt != t ) {
	removeFromChunks();
	txt = t;
	setRect();
	addToChunks();
    }
}

/*!
  Returns the font in which the text is drawn.
  \sa setFont()
*/
QFont QCanvasText::font() const
{
    return fnt;
}

/*!
  Sets the font in which the text is drawn.
  \sa font()
*/
void QCanvasText::setFont( const QFont& f )
{
    if ( f != fnt ) {
	removeFromChunks();
	fnt = f;
	setRect();
	addToChunks();
    }
}

/*!
  Returns the color of the text.
  \sa setColor()
*/
QColor QCanvasText::color() const
{
    return col;
}

/*!
  Sets the color of the text.
  \sa color(), setFont()
*/
void QCanvasText::setColor(const QColor& c)
{
    col=c;
    changeChunks();
}


/*!
  \reimp
*/
void QCanvasText::moveBy(double dx, double dy)
{
    int idx = int(x()+dx)-int(x());
    int idy = int(y()+dy)-int(y());
    if ( idx || idy ) {
	removeFromChunks();
	brect.moveBy(idx, idy);
    }
    myx+=dx;
    myy+=dy;
    if ( idx || idy ) {
	addToChunks();
    }
}

/*!
  Draws the text.
*/
void QCanvasText::draw(QPainter& painter)
{
    painter.setFont(fnt);
    painter.setPen(col);
    painter.drawText(brect, flags, txt);
}

/*!
  \reimp
*/
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
use values greater than 1000 preferably a large random number,
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

/*!
Returns 1.
*/
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
Returns 7.

\sa QCanvasItem::rtti()
*/
int QCanvasLine::rtti() const { return 7; }


/*!
Returns 8.

\sa QCanvasItem::rtti()
*/
int QCanvasSpline::rtti() const { return 8; }


/*!
Constructs a QCanvasSprite which uses images from the given array.

The sprite in initially at (0,0) on \a canvas, using frame 0.
*/
QCanvasSprite::QCanvasSprite(QCanvasPixmapArray* a, QCanvas* canvas) :
    QCanvasItem(canvas),
    frm(0),
    images(a)
{
}


/*!
\fn void QCanvasSprite::setSequence(QCanvasPixmapArray* a)

Set the array of images used for displaying the sprite.  Note that
the array should have enough images for the sprites current frame()
to be valid.
*/
void QCanvasSprite::setSequence(QCanvasPixmapArray* a)
{
    bool isvisible = visible();
    if ( isvisible && images )
	hide();
    images = a;
    if ( isvisible )
	show();
}

/*!
\internal

Marks any chunks the sprite touches as changed.
*/
void QCanvasSprite::changeChunks()
{
    if (visible() && canvas()) {
	int chunksize=canvas()->chunkSize();
	for (int j=topEdge()/chunksize; j<=bottomEdge()/chunksize; j++) {
	    for (int i=leftEdge()/chunksize; i<=rightEdge()/chunksize; i++) {
		canvas()->setChangedChunk(i,j);
	    }
	}
    }
}

/*!
Destruct the sprite.
It is removed from its QCanvas in this process.
*/
QCanvasSprite::~QCanvasSprite()
{
    removeFromChunks();
}

/*!
Sets the animation frame used for displaying the sprite to
\a f, an index into the QCanvasSprite's QCanvasPixmapArray.

\sa frame(), move(double,double,int)
*/
void QCanvasSprite::setFrame(int f)
{
    move(x(),y(),f);
}

/*!
\fn int QCanvasSprite::frame() const
Returns the index into the QCanvasSprite's QCanvasPixmapArray
of the current animation frame.

\sa setFrame(), move(double,double,int)
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

class QCanvasPolygonScanner : public QPolygonScanner {
    QPolygonalProcessor& processor;
public:
    QCanvasPolygonScanner(QPolygonalProcessor& p) :
	processor(p)
    {
    }
    void processSpans( int n, QPoint* point, int* width )
    {
	processor.doSpans(n,point,width);
    }
};

void QCanvasPolygonalItem::scanPolygon(const QPointArray& pa, int winding, QPolygonalProcessor& process) const
{
    QCanvasPolygonScanner scanner(process);
    scanner.scan(pa,winding);
}


#endif // QT_NO_CANVAS
