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
class QCanvasPolygonalItem;
class QCanvasRectangle;
class QCanvasPolygon;
class QCanvasEllipse;
class QCanvasText;
class QCanvasChunk;
class QCanvas;
class QCanvasItem;
class QCanvasView;
class QCanvasPixmap;

#if defined(Q_TEMPLATEDLL)
// MOC_SKIP_BEGIN
template class Q_EXPORT QList< QCanvasItem >;
template class Q_EXPORT QList< QCanvasView >;
template class Q_EXPORT QValueList< QCanvasItem* >;
// MOC_SKIP_END
#endif                                                                          

class QCanvasItemList : public QValueList<QCanvasItem*> {
public:
    void sort();
    void drawUnique( QPainter& painter );
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

    bool animated() const;
    void setAnimated(bool y);
    virtual void setVelocity( double vx, double vy);
    void setXVelocity( double vx ) { setVelocity(vx,yVelocity()); }
    void setYVelocity( double vy ) { setVelocity(xVelocity(),vy); }
    double xVelocity() const;
    double yVelocity() const;
    virtual void advance(int stage);

    virtual bool collidesWith( const QCanvasItem* ) const=0;
    virtual bool collidesWith(   const QCanvasSprite*,
				 const QCanvasPolygonalItem*,
				 const QCanvasRectangle*,
				 const QCanvasEllipse*,
				 const QCanvasText* ) const=0;

    QCanvasItemList collisions(bool exact /* NO DEFAULT */ ) const;

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

    virtual QRect boundingRect() const=0;
    virtual QRect boundingRectAdvanced() const;

protected:
    QCanvas* canvas() const { return cnv; }

    virtual QPointArray chunks() const;
    virtual void addToChunks();
    virtual void removeFromChunks();
    virtual void changeChunks();

private:
    QCanvas* cnv;
    static QCanvas* current_canvas;
    double myx,myy,myz;
    QCanvasItemExtra *ext;
    QCanvasItemExtra& extra();
    uint ani:1;
    uint vis:1;
    uint sel:1;
    uint ena:1;
    uint act:1;
};


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

    bool sameChunk(int x1, int y1, int x2, int y2) const
	{ return x1/chunksize==x2/chunksize && y1/chunksize==y2/chunksize; }
    void setChangedChunk(int i, int j);
    void setChangedChunkContaining(int x, int y);

    // These call setChangedChunk
    void addItemToChunk(QCanvasItem*, int i, int j);
    void removeItemFromChunk(QCanvasItem*, int i, int j);
    void addItemToChunkContaining(QCanvasItem*, int x, int y);
    void removeItemFromChunkContaining(QCanvasItem*, int x, int y);

    QCanvasItemList allItems();
    QCanvasItemList collisions(QPoint) const;
    QCanvasItemList collisions(QRect) const;
    QCanvasItemList collisions(QPointArray pa, const QCanvasItem* item, bool exact) const;

    // These are for QCanvasView to call
    virtual void addView(QCanvasView*);
    virtual void removeView(QCanvasView*);
    void drawArea(const QRect&, QPainter* p=0, bool double_buffer=TRUE);

    // These are for QCanvasItem to call
    virtual void addItem(QCanvasItem*);
    virtual void addAnimation(QCanvasItem*);
    virtual void removeItem(QCanvasItem*);
    virtual void removeAnimation(QCanvasItem*);

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
    friend bool qt_testCollision(const QCanvasSprite* s1, const QCanvasSprite* s2);

    int hotx,hoty;

    QImage* collision_mask;
    int colw,colh;
    int colhotx,colhoty;
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

    bool collidesWith( const QCanvasItem* ) const;
    bool collidesWith(   const QCanvasSprite*,
			 const QCanvasPolygonalItem*,
			 const QCanvasRectangle*,
			 const QCanvasEllipse*,
			 const QCanvasText* ) const;

    QRect boundingRect() const;

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
    QCanvasPixmap* image() const { return images->image(frm); }
    virtual QCanvasPixmap* imageAdvanced() const;
    QCanvasPixmap* image(int f) const { return images->image(f); }

private:
    int frm;

    friend bool qt_testCollision(const QCanvasSprite* s1, const QCanvasSprite* s2);

    QCanvasPixmapSequence* images;
};


class Q_EXPORT QCanvasPolygonalItem : public QCanvasItem
{
public:
    QCanvasPolygonalItem();
    virtual ~QCanvasPolygonalItem();

    bool collidesWith( const QCanvasItem* ) const;
    bool collidesWith(   const QCanvasSprite*,
			 const QCanvasPolygonalItem*,
			 const QCanvasRectangle*,
			 const QCanvasEllipse*,
			 const QCanvasText* ) const;

    void setPen(QPen p);
    void setBrush(QBrush b);

    virtual void moveBy(double dx, double dy);

    virtual QPointArray areaPoints() const=0;
    virtual QPointArray areaPointsAdvanced() const;
    QRect boundingRect() const;

    int rtti() const;

protected:
    virtual void movingBy(int dx, int dy);

    void addToChunks();
    void removeFromChunks();
    void changeChunks();

    void drawRects(QPainter & p);

    void draw(class QPainter &);
    virtual void drawShape(class QPainter &) = 0;

private:
    QPointArray chunkify(int);
    bool scan(const QRect&) const;
    QBrush brush;
    QPen pen;
};

class Q_EXPORT QCanvasRectangle : public QCanvasPolygonalItem
{
    int w, h;

public:
    QCanvasRectangle();
    QCanvasRectangle(const QRect&);
    QCanvasRectangle(int x, int y, int width, int height);
    ~QCanvasRectangle();

    int width() const;
    int height() const;
    void setSize(int w, int h);
    QPointArray areaPoints() const;
    QRect rect() const { return QRect(int(x()),int(y()),w,h); }

    bool collidesWith( const QCanvasItem* ) const;
    bool collidesWith(   const QCanvasSprite*,
			 const QCanvasPolygonalItem*,
			 const QCanvasRectangle*,
			 const QCanvasEllipse*,
			 const QCanvasText* ) const;

    int rtti() const;

protected:
    void drawShape(class QPainter &);
};


class Q_EXPORT QCanvasPolygon : public QCanvasPolygonalItem
{
    QPointArray poly;

public:
    QCanvasPolygon();
    ~QCanvasPolygon();
    void setPoints(QPointArray);
    QPointArray areaPoints() const;

    int rtti() const;

protected:
    void movingBy(int dx, int dy);
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
    int angleStart() const { return a1; }
    int angleLength() const { return a2; }
    QPointArray areaPoints() const;

    bool collidesWith( const QCanvasItem* ) const;
    bool collidesWith(   const QCanvasSprite*,
			 const QCanvasPolygonalItem*,
			 const QCanvasRectangle*,
			 const QCanvasEllipse*,
			 const QCanvasText* ) const;

    int rtti() const;

protected:
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

    QRect boundingRect() const;

    bool collidesWith( const QCanvasItem* ) const;
    bool collidesWith(   const QCanvasSprite*,
			 const QCanvasPolygonalItem*,
			 const QCanvasRectangle*,
			 const QCanvasEllipse*,
			 const QCanvasText* ) const;

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
