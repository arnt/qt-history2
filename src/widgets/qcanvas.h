// QCanvas and associated classes, using Qt C++ class library.
//
// Author: Warwick Allison (warwick@troll.no)
//   Date: 19/10/97
// Copyright (C) 1995-99 by Warwick Allison.
//

#ifndef QCanvas_H
#define QCanvas_H

#include <qbitmap.h>
#include <qwidget.h>
#include <qscrollview.h>
#include <qlist.h>
#include <qptrdict.h>

class QCanvasSprite;
class QCanvasChunk;
class QCanvas;
class QCanvasItem;
class QCanvasView;
class QCanvasIteratorPrivate;
class QCanvasPixmap;

class QCanvasIterator {
    friend QCanvas;
    QCanvas *cnv;
    QCanvasIteratorPrivate* d;
    QCanvasIterator(QCanvas* c, QCanvasIteratorPrivate* p) :
	cnv(c),d(p)
    { }
public:
    ~QCanvasIterator();
    QCanvasItem* operator*();
    bool exact();
    QCanvasIterator& operator ++();
    operator bool() { return d!=0; }
};



class QCanvasItemExtra;

class Q_EXPORT QCanvasItem : public Qt
{
public:
    QCanvasItem();
    virtual ~QCanvasItem();

    double x() const { return myx; }
    double y() const { return myy; }
    double z() const { return myz; } // (depth)

    virtual void moveBy(double dx, double dy);
    void move(double x, double y);
    void setX(double a) { move(a,y()); }
    void setY(double a) { move(x(),a); }
    void setZ(double a) { myz=a; changeChunks(); }

    virtual void setVelocity( double vx, double vy);
    void setXVelocity( double vx ) { setVelocity(vx,yVelocity()); }
    void setYVelocity( double vy ) { setVelocity(xVelocity(),vy); }
    double xVelocity() const;
    double yVelocity() const;
    virtual void forward();

    // TRUE iff the item includes the given pixel position.
    virtual bool at(int x, int y) const;
    // TRUE iff the item intersects with the given area.
    virtual bool at(const QRect& rect) const;
    // TRUE iff the item intersects with the given bitmap.
    //   rect gives the offset of the bitmap and relevant area.
    // The default is to just call at(const QRect& rect) above.
    virtual bool at(const QImage* image, const QRect& rect) const;

    // Traverse intersecting items.
    //
    // See QCanvas::at() for more details.
    //
    //QCanvasIterator neighbourhood() const;
    //QCanvasIterator neighbourhood(int nx, int ny) const;
    //QCanvasIterator neighbourhood(int nx, int ny, QCanvasPixmap*) const;

    bool hitting(QCanvasItem&) const;
    bool wouldHit(QCanvasItem&, int x, int y, QCanvasPixmap*) const;

    static void setCurrentCanvas(QCanvas*);
    void setCanvas(QCanvas*);

    virtual void draw(QPainter&)=0;

    void show();
    void hide();
    virtual void setVisible(bool yes);
    bool visible() const { return (bool)vis; }
    virtual void setSelected(bool yes);
    bool selected() const { return (bool)sel; }
    virtual void setEnabled(bool yes);
    bool enabled() const { return (bool)ena; }
    virtual void setActive(bool yes);
    bool active() const { return (bool)act; }

    virtual int rtti() const;

protected:
    QCanvas* canvas() const { return cnv; }

    virtual void addToChunks()=0;
    virtual void removeFromChunks()=0;
    virtual void changeChunks()=0;

private:
    QCanvas* cnv;
    static QCanvas* current_canvas;
    double myx,myy,myz;
    QCanvasItemExtra *ext;
    QCanvasItemExtra& extra();
    uint vis:1;
    uint sel:1;
    uint ena:1;
    uint act:1;
};

#if defined(Q_TEMPLATEDLL)
// MOC_SKIP_BEGIN
template class Q_EXPORT QList< QCanvasItem >;
template class Q_EXPORT QList< QCanvasView >;
template class Q_EXPORT QValueList< QCanvasItem* >;
// MOC_SKIP_END
#endif                                                                          
typedef QValueList< QCanvasItem* > QCanvasItemList;


class Q_EXPORT QCanvas : public QObject
{
    Q_OBJECT
public:
    QCanvas();
    QCanvas(int w, int h, int chunksize=16, int maxclusters=100);
    QCanvas(const QString& imagefile, int w, int h, int chunksize=16, int maxclusters=100);
    QCanvas(QPixmap p, int w, int h, int chunksize=16, int maxclusters=100);
    QCanvas(QPixmap tiles, int htiles, int vtiles,
			int tilewidth, int tileheight,
		        int chunksize=-1, int maxclusters=100	);

    void setTile( int x, int y, int tilenum );
    int tile( int x, int y ) const { return grid[x+y*htiles]; }

    int tilesHorizontally() const { return htiles; }
    int tilesVertically() const { return vtiles; }

    int tileWidth() const { return tilew; }
    int tileHeight() const { return tileh; }

    virtual ~QCanvas();

    virtual void resize(int width, int height);
    int width() const { return awidth; }
    int height() const { return aheight; }
    QSize size() const { return QSize(awidth,aheight); }

    int chunkSize() const { return chunksize; }
    void retune(int chunksize, int maxclusters);

    QCanvasIterator allItems();
    QCanvasIterator at(int x, int y);
    QCanvasIterator at(QPoint p) { return at(p.x(),p.y()); }
    QCanvasIterator at(int x, int y, int w, int h);

    bool sameChunk(int x1, int y1, int x2, int y2) const
	{ return x1/chunksize==x2/chunksize && y1/chunksize==y2/chunksize; }
    void setChangedChunk(int i, int j);
    void setChangedChunkContaining(int x, int y);

    // These call setChangedChunk
    void addItemToChunk(QCanvasItem*, int i, int j);
    void removeItemFromChunk(QCanvasItem*, int i, int j);
    void addItemToChunkContaining(QCanvasItem*, int x, int y);
    void removeItemFromChunkContaining(QCanvasItem*, int x, int y);

    // This is internal.
    const QCanvasItemList* listAtChunkTopFirst(int i, int j) const;
    QCanvasItemList* allList();

    // These are for QCanvasView to call
    virtual void addView(QCanvasView*);
    virtual void removeView(QCanvasView*);
    void drawArea(const QRect&, QPainter* p=0, bool double_buffer=TRUE);

    // These are for QCanvasItem to call
    virtual void addItem(QCanvasItem*);
    virtual void addAnimation(QCanvasItem*);
    virtual void removeItem(QCanvasItem*);

    void setAdvancePeriod(int ms);

public slots:
    virtual void advance();
    virtual void update();

protected:
    virtual void drawBackground(QPainter&, const QRect& area);
    virtual void drawForeground(QPainter&, const QRect& area);

    void forceRedraw(const QRect&);

private:
    void init(int w, int h, int chunksze, int maxclust);

    QCanvasChunk& chunk(int i, int j) const;
    QCanvasChunk& chunkContaining(int x, int y) const;

    void drawChanges(const QRect& inarea);

	QPixmap offscr;
    int awidth,aheight;
    int chunksize;
    int maxclusters;
    int chwidth,chheight;
    QCanvasChunk* chunks;

    QList<QCanvasView> viewList;
    QPtrDict<void> itemDict;
    QPtrDict<void> animDict;

    static unsigned int posprec;
    static bool double_buffer;

    void initTiles(QPixmap p, int h, int v, int tilewidth, int tileheight);
    void setTiles( QPixmap tiles, int tilewidth, int tileheight );
    ushort *grid;
    ushort htiles;
    ushort vtiles;
    ushort tilew;
    ushort tileh;
    bool oneone;
    QPixmap pm;
    QTimer* update_timer;

    friend QCanvasIterator;
    QCanvasIteratorPrivate* all();
    QCanvasIteratorPrivate* topAt(int x, int y);
    QCanvasIteratorPrivate* topAt(QPoint p) { return topAt(p.x(),p.y()); }
    QCanvasIteratorPrivate* lookIn(int x, int y, int w, int h);
    void next(QCanvasIteratorPrivate*&) const;
    void end(QCanvasIteratorPrivate*& p) const; // need not be called for p==0
    QCanvasItem* at(QCanvasIteratorPrivate* p) const;
    bool exact(QCanvasIteratorPrivate* p) const; // Pre: (p && At(p))
    void protectFromChange(QCanvasIteratorPrivate* p);
};

class Q_EXPORT QCanvasView : public QScrollView
{
    Q_OBJECT
public:
    QCanvasView(QCanvas* viewing=0, QWidget* parent=0, const char* name=0, WFlags f=0);
    ~QCanvasView();

    virtual QRect viewArea() const;

    QCanvas* canvas() { return viewing; }
    void setCanvas(QCanvas* v);

protected:
    void drawContents( QPainter*, int cx, int cy, int cw, int ch );

private:
    QCanvas* viewing;

private slots:
    void cMoving(int,int);
};


class Q_EXPORT QCanvasPixmap : public QPixmap
{
public:
    QCanvasPixmap(const char* datafilename, const char* maskfilename=0);
    QCanvasPixmap(const QPixmap&, QPoint hotspot);
    ~QCanvasPixmap();

    int hotX() const { return hotx; }
    int hotY() const { return hoty; }
    void setHotSpot(int x, int y) { hotx = x; hoty = y; }

private:
    friend class QCanvasSprite;
    friend class QCanvasPixmapSequence;
    friend class QCanvasIteratorPrivate;

    int hotx,hoty;

    QImage* collision_mask;
    int colw,colh;
    int colhotx,colhoty;

    QBitmap mask;
};


class Q_EXPORT QCanvasPixmapSequence
{
public:
    QCanvasPixmapSequence();
    QCanvasPixmapSequence(const char* datafilenamepattern,
	const char* maskfilenamepattern=0, int framecount=1);
    QCanvasPixmapSequence(QList<QPixmap>, QList<QPoint> hotspots);
    ~QCanvasPixmapSequence();

    bool readPixmaps(const char* datafilenamepattern,
	const char* maskfilenamepattern, int framecount=1);
    bool readCollisionMasks(const char* filenamepattern);

    int operator!(); // Failure check.

    QCanvasPixmap* image(int i) const { return img[i]; }
    void setImage(int i, QCanvasPixmap* p) { delete img[i]; img[i]=p; }
    int frameCount() const { return framecount; }

private:
    int framecount;
    QCanvasPixmap** img;
};


class Q_EXPORT QCanvasSprite : public QCanvasItem
{
public:
    QCanvasSprite(QCanvasPixmapSequence*);

    QCanvasSprite();
    void setSequence(QCanvasPixmapSequence* seq);

    virtual ~QCanvasSprite();

    void move(double x, double y);
    virtual void move(double x, double y, int frame);
    void frame(int); 
    int frame() const { return frm; }
    int frameCount() const { return images->frameCount(); }

    virtual int rtti() const;

    //QCanvasIterator* neighbourhood() const;
    //QCanvasIterator* neighbourhood(int nx, int ny) const;
    //QCanvasIterator* neighbourhood(int frame) const; // Neighbourhood if Frame(frame).
    //QCanvasIterator* neighbourhood(double nx, double ny, int frame) const; // Both of above.
    //QCanvasIterator* neighbourhood(int nx, int ny, QCanvasPixmap*) const;
    bool wouldHit(QCanvasItem&, double x, double y, int frame) const;
    bool hitting(QCanvasItem& other) const;

    bool at(int x, int y) const;
    bool at(const QRect& rect) const;
    bool at(const QImage* yourimage, const QRect& yourarea) const;

protected:
    void draw(QPainter& painter);

    void addToChunks();
    void removeFromChunks();
    void changeChunks();

    int width() const;
    int height() const;

    int absX() const;
    int absY() const;
    int absX2() const;
    int absY2() const;

    int absX(int nx) const;
    int absY(int ny) const;
    int absX2(int nx) const;
    int absY2(int ny) const;

private:
    int frm;

    QCanvasPixmap* image() const { return images->image(frm); }
    QCanvasPixmap* image(int f) const { return images->image(f); }

    QCanvasPixmapSequence* images;
};


class Q_EXPORT QCanvasPolygonalItem : public QCanvasItem
{
public:
    QCanvasPolygonalItem();
    virtual ~QCanvasPolygonalItem();

    bool at(int, int) const;
    bool at(const class QRect &) const;

    int rtti() const;

    void setPen(QPen p);
    void setBrush(QBrush b);

    virtual void moveBy(double dx, double dy);

protected:
    virtual void movingBy(int dx, int dy);
    virtual QPointArray areaPoints() const=0;

    void addToChunks();
    void removeFromChunks();
    void changeChunks();

    void drawRects(QPainter & p);

    void draw(class QPainter &);
    virtual void drawShape(class QPainter &) = 0;

private:
    void chunkify(int);
    bool scan(const QRect&) const;
    QBrush brush;
    QPen pen;
};

class Q_EXPORT QCanvasRectangle : public QCanvasPolygonalItem
{
    int w, h;

public:
    QCanvasRectangle();
    QCanvasRectangle(int x, int y, int width, int height);
    ~QCanvasRectangle();

    int width() const;
    int height() const;
    void setSize(int w, int h);

    int rtti() const;

protected:
    void drawShape(class QPainter &);
    QPointArray areaPoints() const;
};


class Q_EXPORT QCanvasPolygon : public QCanvasPolygonalItem
{
    QPointArray poly;

public:
    QCanvasPolygon();
    ~QCanvasPolygon();
    void setPoints(QPointArray);
    int rtti() const;

protected:
    void movingBy(int dx, int dy);
    QPointArray areaPoints() const;
    void drawShape(class QPainter &);
};

class Q_EXPORT QCanvasEllipse : public QCanvasPolygonalItem
{
    int w, h;
    int a1, a2;

public:
    QCanvasEllipse();
    QCanvasEllipse(int width, int height,
	int startangle=0, int angle=360*16);
    ~QCanvasEllipse();

    int width() const;
    int height() const;
    void setSize(int w, int h);
    void setAngles(int start, int length);
    int rtti() const;

protected:
    QPointArray areaPoints() const;
    void drawShape(class QPainter &);
};


class Q_EXPORT QCanvasText : public QCanvasItem
{
public:
    QCanvasText();
    QCanvasText(const char*);
    QCanvasText(const char*, QFont);

    virtual ~QCanvasText();

    void setText( const char* );
    void setFont( const QFont& );
    void setColor( const QColor& );

    void moveBy(double dx, double dy);

    int textFlags() const { return flags; }
    void setTextFlags(int);

    const QRect& boundingRect() { return brect; }

    bool at(int, int) const;
    bool at(const class QRect &) const;

    virtual int rtti() const;

protected:
    virtual void draw(QPainter&);

    // Call these either side of X(), Y(), or Image() value changes.
    void addToChunks();
    void removeFromChunks();

private:
    void changeChunks();
    void setRect();
    QRect brect;
    QString text;
	int flags;
    QFont font;
    QColor col;
};

inline QCanvasIterator::~QCanvasIterator() { cnv->end(d); }
inline QCanvasItem* QCanvasIterator::operator*() { return cnv->at(d); }
inline bool QCanvasIterator::exact() { return cnv->exact(d); }
inline QCanvasIterator& QCanvasIterator::operator ++() { cnv->next(d); return *this; }
inline QCanvasIterator QCanvas::allItems() { return QCanvasIterator(this,all()); }
inline QCanvasIterator QCanvas::at(int x, int y) { return QCanvasIterator(this,topAt(x,y)); }
inline QCanvasIterator QCanvas::at(int x, int y, int w, int h) { return QCanvasIterator(this,lookIn(x,y,w,h)); }


#endif
