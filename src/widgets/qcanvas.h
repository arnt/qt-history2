// QCanvas and associated classes, using Qt C++ class library.
//
// Author: Warwick Allison (warwick@troll.no)
//   Date: 19/10/97
// Copyright (C) 1995-97 by Warwick Allison.
//

#ifndef QCanvas_H
#define QCanvas_H

#include <qbitmap.h>
#include <qwidget.h>
#include <qscrollbar.h>
#include <qlist.h>

class QCanvasSprite;
class QCanvasChunk;
class QCanvas;
class QCanvasView;
class QCanvasIterator;
class QCanvasPixmap;


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


    // TRUE iff the graphic includes the given pixel position.
    virtual bool at(int x, int y) const;
    // TRUE iff the graphic intersects with the given area.
    virtual bool at(const QRect& rect) const;
    // TRUE iff the graphic intersects with the given bitmap.
    //   rect gives the offset of the bitmap and relevant area.
    // The default is to just call at(const QRect& rect) above.
    virtual bool at(const QImage* image, const QRect& rect) const;

    // Traverse intersecting items.
    //
    // See QCanvas::topAt() for more details.
    //
    virtual QCanvasIterator* neighbourhood() const;
    virtual QCanvasIterator* neighbourhood(int nx, int ny) const;
    virtual QCanvasIterator* neighbourhood(int nx, int ny, QCanvasPixmap*) const;
    //
    void next(QCanvasIterator*&) const;
    void end(QCanvasIterator*& p) const; // need not be called for p==0
    QCanvasItem* at(QCanvasIterator* p) const;
    bool exact(QCanvasIterator* p) const; // Pre: (p && at(p))
    bool hitting(QCanvasItem&) const;
    bool wouldHit(QCanvasItem&, int x, int y, QCanvasPixmap*) const;

    static void setCurrentCanvas(QCanvas*);
    void setCanvas(QCanvas*);

    virtual void draw(QPainter&)=0;

    void show();
    void hide();
    void setVisible(bool yes);
    bool visible() const; // initially TRUE [unlike QWidget]

    virtual int rtti() const;

protected:
    QCanvas* canvas;

    virtual void addToChunks()=0;
    virtual void removeFromChunks()=0;
    virtual void changeChunks()=0;

private:
    static QCanvas* current_canvas;
    double myx,myy,myz;
    bool vis;
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

    int chunkSize() const { return chunksize; }
    void retune(int chunksize, int maxclusters);

    // (x,y) is *world* position, (i,j) is chunk coordinate.

    QCanvasIterator* all();
    QCanvasIterator* topAt(int x, int y);
    QCanvasIterator* topAt(QPoint p) { return topAt(p.x(),p.y()); }
    QCanvasIterator* lookIn(int x, int y, int w, int h);
    void next(QCanvasIterator*&) const;
    void end(QCanvasIterator*& p) const; // need not be called for p==0
    QCanvasItem* at(QCanvasIterator* p) const;
    bool exact(QCanvasIterator* p) const; // Pre: (p && At(p))
    void protectFromChange(QCanvasIterator* p);

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

    static void setPositionPrecision(int downshifts) { posprec=downshifts; }
    static int positionPrecision() { return posprec; }
    static int world_to_x(int i) { return i>>posprec; }
    static int x_to_world(int i) { return i<<posprec; }

    // These are for QCanvasView to call
    void addView(QCanvasView*);
    void removeView(QCanvasView*);
    void updateInView(QCanvasView*, const QRect&); // pre: view has been added

    // These are for QCanvasItem to call
    void addItem(QCanvasItem*);
    void removeItem(QCanvasItem*);

    void setUpdatePeriod(int ms);

public slots:
    // Call this after some amount of QCanvasItem motion.
    void update();

protected:
    virtual void drawBackground(QPainter&, const QRect& area);
    virtual void drawForeground(QPainter&, const QRect& area);

    void forceRedraw(const QRect&);

private:
    void init(int w, int h, int chunksze, int maxclust);

    QCanvasChunk& chunk(int i, int j) const;
    QCanvasChunk& chunkContaining(int x, int y) const;

    void drawArea(const QRect&, bool only_changes, QCanvasView* one_view);

	QPixmap offscr;
    int awidth,aheight;
    int chunksize;
    int maxclusters;
    int chwidth,chheight;
    QCanvasChunk* chunks;

    QList<QCanvasView> viewList;
    QList<QCanvasItem> graphicList;

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
};

class Q_EXPORT QCanvasView : public QWidget
{
    Q_OBJECT
public:
    QCanvasView(QCanvas* viewing=0, QWidget* parent=0, const char* name=0, WFlags f=0);
    ~QCanvasView();

    virtual QRect viewArea() const;
    virtual bool preferDoubleBuffering() const;
    virtual void beginPainter(QPainter&);
    virtual void flush(const QRect& area);
    virtual void updateGeometries();

    QCanvas* canvas() { return viewing; }
    void setCanvas(QCanvas* v);

protected:
    // Cause Update for this widget
    virtual void paintEvent(QPaintEvent *);
    virtual void resizeEvent(QResizeEvent *);
    virtual int hSteps() const;
    virtual int vSteps() const;

protected slots:
    void vScroll(int);
    void hScroll(int);

private:
    QCanvas* viewing;
    QPixmap offscr;
    QScrollBar *hscroll;
    QScrollBar *vscroll;
    int vscrpv;
    int hscrpv;
};


class Q_EXPORT QCanvasPixmap : public QPixmap
{
public:
    QCanvasPixmap(const char* datafilename, const char* maskfilename);
    QCanvasPixmap(const QPixmap&, QPoint hotspot);
    ~QCanvasPixmap();

    int hotX() const { return hotx; }
    int hotY() const { return hoty; }
    void setHotSpot(int x, int y) { hotx = x; hoty = y; }

private:
    friend class QCanvasSprite;
    friend class QCanvasPixmapSequence;
    friend class QCanvasIterator;

    int hotx,hoty;

    QImage* collision_mask;
    int colw,colh;
    int colhotx,colhoty;

    QBitmap mask;
};


class Q_EXPORT QCanvasPixmapSequence
{
public:
    QCanvasPixmapSequence(const char* datafilenamepattern,
	const char* maskfilenamepattern, int framecount=1);
    QCanvasPixmapSequence(QList<QPixmap>, QList<QPoint> hotspots);
    ~QCanvasPixmapSequence();

    void readCollisionMasks(const char* filenamepattern);

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

    QCanvasIterator* neighbourhood() const;
    QCanvasIterator* neighbourhood(int nx, int ny) const;
    QCanvasIterator* neighbourhood(int frame) const; // Neighbourhood if Frame(frame).
    QCanvasIterator* neighbourhood(double nx, double ny, int frame) const; // Both of above.
    QCanvasIterator* neighbourhood(int nx, int ny, QCanvasPixmap*) const;
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
    virtual void movingBy(double dx, double dy);
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
    QPoint pos;
    QBrush brush;
    QPen pen;
};

class Q_EXPORT QCanvasRectangle : public QCanvasPolygonalItem
{
    int w, h;

public:
    QCanvasRectangle();
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

#endif
