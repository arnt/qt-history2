#include <qptrlist.h>
#include <qwidget.h>
#include <qpainter.h>
#include <qbitmap.h>
#include <qwindowsystem_qws.h>

/*
#include "qgfxmatrox_qws.cpp"
#include "qgfxvoodoo_qws.cpp"
#include "qgfxmach64_qws.cpp"
*/

class QGfxRec {

public:

    int xoffs;
    int yoffs;
    int w;
    int h;

    QGfx * gfx;
    QScreen * screen;

};

class QCursorRec {

public:

    int xoffs;
    int yoffs;
    int w;
    int h;
    QScreenCursor * cursor;
    bool visible;

};

class QRepeaterGfx : public QGfx {

public:

    QRepeaterGfx();
    ~QRepeaterGfx();
    void addScreen(QScreen *,QScreenCursor *,int,int,bool);

    virtual void setPen(const QPen &);
    virtual void setFont(const QFont &);
    virtual void setBrush(const QBrush &);
    virtual void setBrushPixmap(const QPixmap *);
    virtual void setBrushOffset (int, int);
    virtual void setClipRect (int, int, int, int);
    virtual void setClipRegion (const QRegion &);
    virtual void setClipping (bool);
    virtual void setOffset (int, int);
    virtual void setWidgetRect (int, int, int, int);
    virtual void setWidgetRegion (const QRegion &);
    virtual void setSourceWidgetOffset (int, int);
    virtual void setGlobalRegionIndex (int);
    virtual void setDashedLines (bool);
    virtual void setDashes (char *, int);
    virtual void setOpaqueBackground (bool);
    virtual void setBackgroundColor (QColor);
    virtual void drawPoint (int, int);
    virtual void drawPoints (const QPointArray &, int, int);
    virtual void moveTo (int, int);
    virtual void lineTo (int, int);
    virtual void drawLine (int, int, int, int);
    virtual void drawPolyline (const QPointArray &, int, int);
    virtual QPoint pos () const;
    virtual void fillRect (int, int, int, int);
    virtual void drawPolygon (const QPointArray &, bool, int, int);
    virtual void setLineStep (int);
    virtual void blt (int, int, int, int, int, int);
    virtual void scroll (int, int, int, int, int, int);
    virtual void stretchBlt (int, int, int, int, int, int);
    virtual void tiledBlt (int, int, int, int);
    virtual void setSource (const QPaintDevice *);
    virtual void setSource (const QImage *);
    virtual void setSource(unsigned char *,int,int,int,int,QRgb *,int);
    virtual void setSourcePen ();
    virtual void setAlphaType (AlphaType);
    virtual void setAlphaSource (unsigned char *, int);
    virtual void setAlphaSource (int, int = -1, int = -1, int = -1);
    virtual void drawText (int, int, const QString &);
    virtual void setClut (QRgb *, int);
    virtual void save ();
    virtual void restore ();
    virtual void setRop (Qt::RasterOp);

private:

    QPtrList<QGfxRec> gfxen;
    QRegion widgetclip;
    bool desktopsource;
    int xoffs;
    int yoffs;

};

QRepeaterGfx::QRepeaterGfx()
{
    desktopsource=false;
    xoffs=0;
    yoffs=0;
}

QRepeaterGfx::~QRepeaterGfx()
{
    QGfxRec * walker;
    for(walker=gfxen.first();walker;walker=gfxen.next()) {
	delete walker->gfx;
    }
    // FIXME: why is this needed?
    while(qt_fbdpy->grabbed()) {
	qt_fbdpy->ungrab();
    }
}

QString dumpRegion(QRegion r)
{
    QMemArray<QRect> myrects=r.rects();
    QString ret="(";
    for(int loopc=0;loopc<myrects.size();loopc++) {
	QRect rect=myrects[loopc];
	ret+="[";
	ret+=QString::number(rect.left());
	ret+=".";
	ret+=QString::number(rect.top());
	ret+=".";
	ret+=QString::number(rect.width());
	ret+=".";
	ret+=QString::number(rect.height());
	ret+="]";
    }
    ret+=")";
    return ret;
}

void QRepeaterGfx::addScreen(QScreen * s,QScreenCursor *c,int x,int y,bool b)
{
    QGfxRec * tmp=new QGfxRec;
    tmp->xoffs=x;
    tmp->yoffs=y;
    tmp->w=s->width();
    tmp->h=s->height();
    tmp->gfx=s->screenGfx();
    tmp->gfx->setOffset(-(tmp->xoffs),-(tmp->yoffs));
    tmp->gfx->setScreen(s,c,b,s->opType(),s->lastOp());
    tmp->screen=s;
    gfxen.append(tmp);
}

void QRepeaterGfx::setPen(const QPen & p)
{
    for(QGfxRec * walker=gfxen.first();walker;walker=gfxen.next()) {
	walker->gfx->setPen(p);
    }
}

void QRepeaterGfx::setFont(const QFont & f)
{
    for(QGfxRec * walker=gfxen.first();walker;walker=gfxen.next()) {
	walker->gfx->setFont(f);
    }
}

void QRepeaterGfx::setBrush(const QBrush & b)
{
    for(QGfxRec * walker=gfxen.first();walker;walker=gfxen.next()) {
	walker->gfx->setBrush(b);
    }
}

void QRepeaterGfx::setBrushPixmap(const QPixmap * p)
{
    for(QGfxRec * walker=gfxen.first();walker;walker=gfxen.next()) {
	walker->gfx->setBrushPixmap(p);
    }
}

void QRepeaterGfx::setBrushOffset (int x, int y)
{
    for(QGfxRec * walker=gfxen.first();walker;walker=gfxen.next()) {
	walker->gfx->setBrushOffset(x,y);
    }
}

void QRepeaterGfx::setClipRect (int x, int y, int w, int h)
{
    for(QGfxRec * walker=gfxen.first();walker;walker=gfxen.next()) {
	QRect r2(walker->xoffs,walker->yoffs,walker->w,walker->h);
	QRect r1(x,y,w,h);
	r1.moveBy(xoffs,yoffs);
	r1=r1.intersect(r2);
	r1.moveBy(-xoffs,-yoffs);
	walker->gfx->setClipRect(r1.left(),r1.top(),r1.width(),r1.height());
    }
}

void QRepeaterGfx::setClipRegion (const QRegion & r)
{
    for(QGfxRec * walker=gfxen.first();walker;walker=gfxen.next()) {
	QRegion r1(QRect(walker->xoffs,walker->yoffs,walker->w,walker->h));
	QRegion r2=r;
	r2.translate(xoffs,yoffs);
	r2=r1.intersect(r2);
	r2.translate(-xoffs,-yoffs);
	walker->gfx->setClipRegion(r2);
    }
}

void QRepeaterGfx::setClipping (bool b)
{
    for(QGfxRec * walker=gfxen.first();walker;walker=gfxen.next()) {
	walker->gfx->setClipping(b);
    }
}

void QRepeaterGfx::setOffset (int x, int y)
{
    for(QGfxRec * walker=gfxen.first();walker;walker=gfxen.next()) {
	walker->gfx->setOffset(x-walker->xoffs,y-walker->yoffs);
    }
    xoffs=x;
    yoffs=y;
}

void QRepeaterGfx::setWidgetRect (int x, int y, int w, int h)
{
    for(QGfxRec * walker=gfxen.first();walker;walker=gfxen.next()) {
	QRect r(x-walker->xoffs,y-walker->yoffs,w,h);
	QRect r2(0,0,walker->w,walker->h);
	QRect r3=r2.intersect(r);
	walker->gfx->setWidgetRect(r3.left(),r3.top(),
				   r3.width(),r3.height());
    }
    widgetclip=QRect(x,y,w,h);
}

void QRepeaterGfx::setWidgetRegion (const QRegion & r)
{
    for(QGfxRec * walker=gfxen.first();walker;walker=gfxen.next()) {
	QRegion r1=r;
	r1.translate(-(walker->xoffs),-(walker->yoffs));
	QRegion r2(0,0,walker->w,walker->h);
	r1=r1.intersect(r2);
	walker->gfx->setWidgetRegion(r1);
    }
    widgetclip=r;
}

void QRepeaterGfx::setSourceWidgetOffset (int x, int y)
{
    for(QGfxRec * walker=gfxen.first();walker;walker=gfxen.next()) {
	walker->gfx->setSourceWidgetOffset(x-walker->xoffs,y-walker->yoffs);
    }
}

void QRepeaterGfx::setGlobalRegionIndex (int i)
{
    for(QGfxRec * walker=gfxen.first();walker;walker=gfxen.next()) {
	walker->gfx->setGlobalRegionIndex(i);
    }
}

void QRepeaterGfx::setDashedLines (bool b)
{
    for(QGfxRec * walker=gfxen.first();walker;walker=gfxen.next()) {
	walker->gfx->setDashedLines(b);
    }
}

void QRepeaterGfx::setDashes (char * c, int i)
{
    for(QGfxRec * walker=gfxen.first();walker;walker=gfxen.next()) {
	walker->gfx->setDashes(c,i);
    }
}

void QRepeaterGfx::setOpaqueBackground (bool b)
{
    for(QGfxRec * walker=gfxen.first();walker;walker=gfxen.next()) {
	walker->gfx->setOpaqueBackground(b);
    }
}

void QRepeaterGfx::setBackgroundColor (QColor c)
{
    for(QGfxRec * walker=gfxen.first();walker;walker=gfxen.next()) {
	walker->gfx->setBackgroundColor(c);
    }
}

void QRepeaterGfx::drawPoint (int x, int y)
{
    for(QGfxRec * walker=gfxen.first();walker;walker=gfxen.next()) {
	walker->gfx->drawPoint(x,y);
    }
}

void QRepeaterGfx::drawPoints (const QPointArray & a, int b, int c)
{
    for(QGfxRec * walker=gfxen.first();walker;walker=gfxen.next()) {
	QPointArray d=a;
	//d.translate(-(walker->xoffs),-(walker->yoffs));
	walker->gfx->drawPoints(d,b,c);
    }
}

void QRepeaterGfx::moveTo (int x, int y)
{
    for(QGfxRec * walker=gfxen.first();walker;walker=gfxen.next()) {
	walker->gfx->moveTo(x,y);
    }
}

void QRepeaterGfx::lineTo (int x, int y)
{
    for(QGfxRec * walker=gfxen.first();walker;walker=gfxen.next()) {
	walker->gfx->lineTo(x,y);
    }
}

void QRepeaterGfx::drawLine (int x1, int y1, int x2, int y2)
{
    for(QGfxRec * walker=gfxen.first();walker;walker=gfxen.next()) {
	walker->gfx->drawLine(x1,y1,x2,y2);
    }
}

void QRepeaterGfx::drawPolyline (const QPointArray & a, int b, int c)
{
    for(QGfxRec * walker=gfxen.first();walker;walker=gfxen.next()) {
	QPointArray d=a;
	//d.translate(-(walker->xoffs),-(walker->yoffs));
	walker->gfx->drawPolyline(d,b,c);
    }
}

QPoint QRepeaterGfx::pos () const
{
    return QPoint(0,0);  // FIXME
}

void QRepeaterGfx::fillRect (int x, int y, int w, int h)
{
    for(QGfxRec * walker=gfxen.first();walker;walker=gfxen.next()) {
	walker->gfx->fillRect(x,y,w,h);
    }
}

void QRepeaterGfx::drawPolygon (const QPointArray & a, bool b, int c, int d)
{
    for(QGfxRec * walker=gfxen.first();walker;walker=gfxen.next()) {
	QPointArray e=a;
	//e.translate(-(walker->xoffs),-(walker->yoffs));
	walker->gfx->drawPolygon(e,b,c,d);
    }
}

void QRepeaterGfx::setLineStep (int l)
{
    for(QGfxRec * walker=gfxen.first();walker;walker=gfxen.next()) {
	walker->gfx->setLineStep(l);
    }
}

void QRepeaterGfx::blt (int x, int y, int w, int h, int sx, int sy)
{
    for(QGfxRec * walker=gfxen.first();walker;walker=gfxen.next()) {
	walker->gfx->blt(x,y,w,h,sx,sy);
    }
}

void QRepeaterGfx::stretchBlt (int x, int y, int w, int h, int sw, int sh)
{
    for(QGfxRec * walker=gfxen.first();walker;walker=gfxen.next()) {
	walker->gfx->stretchBlt(x,y,w,h,
				sw,sh);
    }
}

void QRepeaterGfx::tiledBlt (int x, int y, int w, int h)
{
    for(QGfxRec * walker=gfxen.first();walker;walker=gfxen.next()) {
	walker->gfx->tiledBlt(x,y,w,h);
    }
}

void QRepeaterGfx::setSource (const QPaintDevice * p)
{
    if(p->scanLine(0)==(uchar *)0xdeadbeef) {
	desktopsource=true;
    } else {
	desktopsource=false;
    }
    for(QGfxRec * walker=gfxen.first();walker;walker=gfxen.next()) {
	walker->gfx->setSource(p);
    }
}

void QRepeaterGfx::setSource (const QImage * i)
{
    desktopsource=false;
    for(QGfxRec * walker=gfxen.first();walker;walker=gfxen.next()) {
	walker->gfx->setSource(i);
    }
}

void QRepeaterGfx::setSource (unsigned char * c,int w,int h,int l,
			      int d,QRgb * r,int n)
{
    desktopsource=false;
    for(QGfxRec * walker=gfxen.first();walker;walker=gfxen.next()) {
	walker->gfx->setSource(c,w,h,l,d,r,n);
    }
}

void QRepeaterGfx::setSourcePen ()
{
    desktopsource=false;
    for(QGfxRec * walker=gfxen.first();walker;walker=gfxen.next()) {
	walker->gfx->setSourcePen();
    }
}

void QRepeaterGfx::setAlphaType (QGfx::AlphaType a)
{
    for(QGfxRec * walker=gfxen.first();walker;walker=gfxen.next()) {
	walker->gfx->setAlphaType(a);
    }
}

void QRepeaterGfx::setAlphaSource (unsigned char * c, int i)
{
    for(QGfxRec * walker=gfxen.first();walker;walker=gfxen.next()) {
	walker->gfx->setAlphaSource(c,i);
    }
}

void QRepeaterGfx::setAlphaSource (int a, int b = -1, int c= -1, int d = -1)
{
    for(QGfxRec * walker=gfxen.first();walker;walker=gfxen.next()) {
	walker->gfx->setAlphaSource(a,b,c,d);
    }
}

void QRepeaterGfx::drawText (int x, int y, const QString & s)
{
    for(QGfxRec * walker=gfxen.first();walker;walker=gfxen.next()) {
	walker->gfx->drawText(x,y,s);
    }
}

void QRepeaterGfx::setClut (QRgb * r, int i)
{
    for(QGfxRec * walker=gfxen.first();walker;walker=gfxen.next()) {
	walker->gfx->setClut(r,i);
    }
}

void QRepeaterGfx::save ()
{
    for(QGfxRec * walker=gfxen.first();walker;walker=gfxen.next()) {
	walker->gfx->save();
    }
}

void QRepeaterGfx::restore ()
{
    for(QGfxRec * walker=gfxen.first();walker;walker=gfxen.next()) {
	walker->gfx->restore();
    }
}

void QRepeaterGfx::setRop (Qt::RasterOp r)
{
    for(QGfxRec * walker=gfxen.first();walker;walker=gfxen.next()) {
	walker->gfx->setRop(r);
    }
}

class QRepeaterCursor : public QScreenCursor
{

public:

    QRepeaterCursor();
    ~QRepeaterCursor();
    void addScreen(QScreen *,int,int);

    virtual void init(SWCursorData *,bool);
    virtual void set(const QImage &image,int hotx,int hoty);
    virtual void move(int,int);
    virtual void show();
    virtual void hide();

    virtual bool restoreUnder( const QRect &, QGfxRasterBase * = 0 )
        { return false; }
    virtual void saveUnder() {}
    virtual void drawCursor() {}

    virtual bool supportsAlphaCursor() { return false; }

    static bool enabled() { return false; }

    int xpos;
    int ypos;
    bool visible;

private:

    QPtrList<QCursorRec> cursors;

};

class QScreenRec {

public:

    QScreenRec(QScreen * s,char * sl,QString f,bool b) { screen=s; xoffs=-1;
                                       slot=strdup(sl); swcursor=b;
                                       fb=f; yoffs=0; }

    QScreen * screen;
    int xoffs;
    int yoffs;
    QScreenCursor * cursor;
    char * slot;
    bool swcursor;
    QString fb;

};

class QRepeaterScreen : public QScreen {

public:

    QRepeaterScreen(int);
    virtual ~QRepeaterScreen();

    virtual bool connect(const QString &);
    virtual QGfx * createGfx(unsigned char *,int,int,int,int);
    virtual bool initDevice();
    virtual void disconnect() {}
    virtual void setMode(int,int,int) {}
    virtual int initCursor(void *,bool=FALSE);
    QImage * readScreen(int,int,int,int,QRegion &);
    QRegion getRequiredUpdate(int,int,int,int,int,int);

private:

    bool sw_cursor_exists;

    QPtrList<QScreenRec> screens;

};

// r==the region of the virtual screen not covered by any of the framebuffers,
//    intersected with the rectangle we're grabbing

QImage * QRepeaterScreen::readScreen(int x,int y,int w,int h,QRegion & r)
{
    QImage * ret=new QImage(w,h,32);
    QScreenRec * walker;
    QRect r1(x,y,w,h);

    bool did_hide=false;

    QGfx * gfx=ret->graphicsContext();
    r=QRegion(QRect(x,y,w,h));
    QRepeaterCursor * qrc=(QRepeaterCursor *)qt_screencursor;
    if((qrc->visible) && sw_cursor_exists && QRect(x,y,w,h).intersects
       (QRect(qrc->xpos,qrc->ypos,32,32))) {
	qt_screencursor->hide();
	did_hide=true;
    }

    for(walker=screens.first();walker;walker=screens.next()) {
	QRect r2(walker->xoffs,walker->yoffs,walker->screen->width(),
		 walker->screen->height());
	if(r1.intersects(r2)) {
	    QScreen * s=walker->screen;
	    gfx->setSource(s->base(),s->width(),s->height(),s->linestep(),
			   s->depth(),s->clut(),s->numCols());
	    r2=r1.intersect(r2);
	    r-=QRegion(r2);
	    QRect r3=r2;
	    r3.moveBy(-(walker->xoffs),-(walker->yoffs));
	    gfx->blt(r2.left()-x,r2.top()-y,r3.width(),r3.height(),
		     r3.left(),r3.top());
	}
    }
    delete gfx;

    if(did_hide) {
	qt_screencursor->show();
    }

    return ret;
}

extern char * qt_qws_hardcoded_slot;

QRepeaterScreen::QRepeaterScreen(int)
    : QScreen(0)
{
    screens.append(new QScreenRec(new QLinuxFbScreen(0),
				  "/proc/bus/pci/01/00.0","/dev/fb0",false));
    screens.append(new QScreenRec(new QLinuxFbScreen(0),
				  "/proc/bus/pci/00/0a.0","/dev/fb1",false));
    data=(uchar *)0xdeadbeef;
    sw_cursor_exists=false;
}

int QRepeaterScreen::initCursor(void * v,bool b)
{
    QRepeaterCursor * qrc=new QRepeaterCursor();
    unsigned char * c=(unsigned char *)v;
    QScreenRec * walker;
    int count=0;
    QScreen * tmp=qt_screen;
    for(walker=screens.first();walker;walker=screens.next()) {
	qt_screen=walker->screen;
	int am=walker->screen->initCursor((void *)c,b);
	c-=am;
	count+=am;
	qrc->addScreen(walker->screen,walker->xoffs,walker->yoffs);
    }
    qt_screencursor=qrc;
    qt_screen=tmp;
    return count;
}

QRepeaterScreen::~QRepeaterScreen()
{
    QScreenRec * walker;
    for(walker=screens.first();walker;walker=screens.next()) {
	delete walker->screen;
    }
}

bool QRepeaterScreen::initDevice()
{
    QScreenRec * walker;
    for(walker=screens.first();walker;walker=screens.next()) {
	walker->screen->initDevice();
    }
    return true;
}

bool QRepeaterScreen::connect(const QString &)
{
    QScreenRec * walker;
    int count=0;
    d=0;
    w=0;
    h=0;

    int wcount=0;

    for(walker=screens.first();walker;walker=screens.next()) {
	qt_qws_hardcoded_slot=walker->slot;
	qDebug("Connecting to "+walker->fb);
	walker->screen->connect(walker->fb);
	qDebug("Done");
	if(walker->xoffs==-1) {
	    walker->xoffs=wcount;
	    walker->yoffs=0;
	    wcount+=walker->screen->width();
	}
	if((walker->xoffs+walker->screen->width()) > w) {
	    w=walker->xoffs+walker->screen->width();
	}
      	if((walker->yoffs+walker->screen->height()) > h) {
	    h=walker->yoffs+walker->screen->height();
	}
	if(walker->screen->depth() > d)
	    d=walker->screen->depth();
    }

    lstep=0xdeadbeef;
    pixeltype=0;
    hotx=0;
    hoty=0;
    size=0x6666;
    displayId=0;
    dw=w;
    dh=h;
    initted=true;

    return true;
}

QGfx * QRepeaterScreen::createGfx(uchar * buffer,int w,int h,int d,
				  int linestep)
{
    if(buffer==(uchar *)0xdeadbeef) {
	// screen
	QRepeaterGfx * qrg=new QRepeaterGfx();
        QScreenRec * walker;
	for(walker=screens.first();walker;walker=screens.next()) {
	    qrg->addScreen(walker->screen,walker->cursor,walker->xoffs,
			   walker->yoffs,walker->swcursor);
	}
	qrg->setWidgetRect(0,0,w,h);
	return qrg;
    } else {
	return QScreen::createGfx(buffer,w,h,d,linestep);
    }
}

QRepeaterCursor::QRepeaterCursor()
{
    visible=false;
}

QRepeaterCursor::~QRepeaterCursor()
{
    QCursorRec * walker;
    for(walker=cursors.first();walker;walker=cursors.next()) {
	delete walker->cursor;
    }
}

void QRepeaterCursor::addScreen(QScreen * s,int x,int y)
{
    QCursorRec * tmp=new QCursorRec;
    tmp->xoffs=x;
    tmp->yoffs=y;
    tmp->w=s->width();
    tmp->h=s->height();
    tmp->visible=false;
    tmp->cursor=qt_screencursor;
    cursors.append(tmp);
}

void QRepeaterCursor::init(SWCursorData *,bool)
{
    // Handled by screen's cursor construction method
}

void QRepeaterCursor::set(const QImage &image,int hotx,int hoty)
{
    QCursorRec * walker;
    for(walker=cursors.first();walker;walker=cursors.next()) {
	walker->cursor->set(image,hotx,hoty);
    }
}

void QRepeaterCursor::move(int x,int y)
{
    QCursorRec * walker;
    for(walker=cursors.first();walker;walker=cursors.next()) {
	int xx=x-(walker->xoffs);
	int yy=y-(walker->yoffs);
	if(xx<0 || yy<0 || xx>walker->w || yy>walker->h) {
	    walker->cursor->hide();
	    walker->cursor->move(walker->w,walker->h);
	    walker->visible=false;
	} else {
	    walker->visible=true;
	    if(visible)
		walker->cursor->show();
	    walker->cursor->move(xx,yy);
	}
    }
    xpos=x;
    ypos=y;
}

void QRepeaterCursor::show()
{
    QCursorRec * walker;
    for(walker=cursors.first();walker;walker=cursors.next()) {
	if(walker->visible)
	    walker->cursor->show();
   }
    visible=true;
}

void QRepeaterCursor::hide()
{
    QCursorRec * walker;
    for(walker=cursors.first();walker;walker=cursors.next()) {
	if(walker->visible)
	    walker->cursor->hide();
    }
    visible=false;
}

void QRepeaterGfx::scroll (int x, int y, int w, int h, int sx, int sy)
{
    QRect r1(x+xoffs,y+yoffs,w,h);
    QRect r2(sx+xoffs,sy+yoffs,w,h);

    QRegion destregion(r1);
    QRegion srcregion(r2);

    r2=r2.unite(r1);

    bool crossover=false;
    QGfxRec * walker;
    for(walker=gfxen.first();walker;walker=gfxen.next()) {
	if(r2.right() < walker->xoffs || r2.bottom() < walker->yoffs ||
	   r2.left() > (walker->xoffs + walker->w) ||
	   r2.top() > (walker->yoffs + walker->h)) {

	} else if(r2.left() >= walker->xoffs && r2.top() >= walker->yoffs &&
		  r2.right() <= (walker->xoffs+walker->w) &&
		  r2.bottom() <= (walker->yoffs+walker->h)) {

	} else {
	    crossover=true;
	}
    }

    if(!crossover) {
	bool did_hide=false;
	QRepeaterCursor * qrc=(QRepeaterCursor *)qt_screencursor;

	for(walker=gfxen.first();walker;walker=gfxen.next()) {
	    if(r2.left() >= walker->xoffs && r2.top() >= walker->yoffs &&
	       r2.right() <= (walker->xoffs+walker->w) &&
	       r2.bottom() <= (walker->yoffs+walker->h)) {
		QScreen * tmp=qt_screen;
		qt_screen=walker->screen;
		walker->gfx->setOffset(xoffs-walker->xoffs,
				       yoffs-walker->yoffs);
		walker->gfx->scroll(x,y,w,h,sx,sy);
		qt_screen=tmp;
	    }
	}

	return;

    }; /* else {
	QRegion r;

	QImage * i=((QRepeaterScreen *)qt_screen)->readScreen(
						sx+xoffs,sy+yoffs,w,h,r);
	QGfx * tmp=qt_screen->screenGfx();
	tmp->setSource(i);
	tmp->setWidgetRegion(widgetclip);
	tmp->blt(x+xoffs,y+yoffs,w,h,0,0);
	delete tmp;
	delete i;

	r.translate(x-sx,y-sy);

	if(!r.isEmpty())
	    qt_fbdpy->repaintRegion(r);
    }
    */

    QRegion toupdate(r2);

    for(walker=gfxen.first();walker;walker=gfxen.next()) {
	walker->gfx->scroll(x,y,w,h,sx,sy);
	QRegion sregion(QRect(walker->xoffs,walker->yoffs,
			      walker->w,walker->h));
	QRegion ssrc=sregion.intersect(srcregion);
	QRegion sdest=ssrc;
	sdest.translate(x-sx,y-sy);
	sdest=sregion.intersect(sdest);
	toupdate=toupdate.subtract(sdest);
    }

    if(!toupdate.isEmpty()) {
	qt_fbdpy->repaintRegion(toupdate);
    }
}

/*
void QRepeaterCursor::draw()
{
    QCursorRec * walker;
    for(walker=cursors.first();walker;walker=cursors.next()) {
	walker->cursor->draw();
    }
}
*/

QScreen * qt_get_screen_repeater(int)
{
    return new QRepeaterScreen(0);
}
