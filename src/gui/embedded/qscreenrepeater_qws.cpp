/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qgfxrepeater_qws.h"

#include "qgfxdriverfactory_qws.h"
#include "qgfxraster_qws.h"
#include "qgfxlinuxfb_qws.h"
#include <qlist.h>
#include <qwidget.h>
#include <qpainter.h>
#include <qbitmap.h>
#include <qwindowsystem_qws.h>
#include <qlibraryinfo.h>
/*
#include "qgfxmatrox_qws.cpp"
#include "qgfxvoodoo_qws.cpp"
#include "qgfxmach64_qws.cpp"
*/

extern QString qws_topdir();

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
    QScreen *screen;
    bool visible;

};

class QRepeaterGfx : public QGfx {

public:

    QRepeaterGfx();
    ~QRepeaterGfx();
    void addScreen(QScreen *,QScreenCursor *,int,int,bool);

    virtual void setClipDeviceRegion(const QRegion &);
    virtual void setWidgetDeviceRegion(const QRegion &);

    virtual void setPen(const QPen &);
    virtual void setBrush(const QBrush &);
    virtual void setBrushOrigin (int, int);
    virtual void setClipRegion (const QRegion &, Qt::ClipOperation);
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
    virtual void drawPoints (const QPolygon &, int, int);
    virtual void moveTo (int, int);
    virtual void lineTo (int, int);
    virtual void drawLine (int, int, int, int);
    virtual void drawPolyline (const QPolygon &, int, int);
    virtual QPoint pos () const;
    virtual void fillRect (int, int, int, int);
    virtual void drawPolygon (const QPolygon &, bool, int, int);
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
    virtual void setClut (QRgb *, int);
    virtual void save ();
    virtual void restore ();

private:

    QList<QGfxRec*> gfxen;
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
    for (int i = 0; i < gfxen.count(); ++i) {
        QGfxRec * walker = gfxen.at(i);
        delete walker->gfx;
    }
}

QString dumpRegion(QRegion r)
{
    QVector<QRect> myrects=r.rects();
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
    QScreen * tmp2=qt_screen;
    qt_screen=s;
    tmp->xoffs=x;
    tmp->yoffs=y;
    tmp->w=s->width();
    tmp->h=s->height();
    tmp->gfx=s->screenGfx();
    tmp->gfx->setOffset(-(tmp->xoffs),(tmp->yoffs));
    tmp->gfx->setScreen(s,c,b,s->opType(),s->lastOp());
    tmp->screen=s;
    gfxen.append(tmp);
    qt_screen=tmp2;
}

void QRepeaterGfx::setPen(const QPen & p)
{
    for (int i = 0; i < gfxen.count(); ++i) {
        QGfxRec * walker = gfxen.at(i);
        walker->gfx->setPen(p);
    }
}

void QRepeaterGfx::setBrush(const QBrush & b)
{
    for (int i = 0; i < gfxen.count(); ++i) {
        QGfxRec * walker = gfxen.at(i);
        walker->gfx->setBrush(b);
    }
}

void QRepeaterGfx::setBrushOrigin (int x, int y)
{
    for (int i = 0; i < gfxen.count(); ++i) {
        QGfxRec * walker = gfxen.at(i);
        walker->gfx->setBrushOrigin(x,y);
    }
}

void QRepeaterGfx::setClipDeviceRegion (const QRegion & r)
{
    for (int i = 0; i < gfxen.count(); ++i) {
        QGfxRec * walker = gfxen.at(i);
        QRegion r1(QRect(walker->xoffs,walker->yoffs,walker->w,walker->h));
        QRegion r2=r;
        r2.translate(xoffs,yoffs);
        r2=r1.intersect(r2);
        r2.translate(-xoffs,-yoffs);
        walker->gfx->setClipDeviceRegion(r2);
    }
}

void QRepeaterGfx::setWidgetDeviceRegion (const QRegion & r)
{
    for (int i = 0; i < gfxen.count(); ++i) {
        QGfxRec * walker = gfxen.at(i);
        QRegion r1=r;
        r1.translate(-(walker->xoffs),-(walker->yoffs));
        QRegion r2(0,0,walker->w,walker->h);
        r1=r1.intersect(r2);
        walker->gfx->setWidgetDeviceRegion(r1);
    }
}

void QRepeaterGfx::setClipRegion (const QRegion & r, Qt::ClipOperation op)
{
    for (int i = 0; i < gfxen.count(); ++i) {
        QGfxRec * walker = gfxen.at(i);
        QRegion r1(QRect(walker->xoffs,walker->yoffs,walker->w,walker->h));
        QRegion r2=r;
        r2.translate(xoffs,yoffs);
        r2=r1.intersect(r2);
        r2.translate(-xoffs,-yoffs);
        walker->gfx->setClipRegion(r2, op);
    }
}

void QRepeaterGfx::setClipping (bool b)
{
    for (int i = 0; i < gfxen.count(); ++i) {
        QGfxRec * walker = gfxen.at(i);
        walker->gfx->setClipping(b);
    }
}

void QRepeaterGfx::setOffset (int x, int y)
{
    for (int i = 0; i < gfxen.count(); ++i) {
        QGfxRec * walker = gfxen.at(i);
        walker->gfx->setOffset(x-walker->xoffs,y-walker->yoffs);
    }
    xoffs=x;
    yoffs=y;
}

void QRepeaterGfx::setWidgetRect (int x, int y, int w, int h)
{
    for (int i = 0; i < gfxen.count(); ++i) {
        QGfxRec * walker = gfxen.at(i);
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
    for (int i = 0; i < gfxen.count(); ++i) {
        QGfxRec * walker = gfxen.at(i);
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
    for (int i = 0; i < gfxen.count(); ++i) {
        QGfxRec * walker = gfxen.at(i);
        walker->gfx->setSourceWidgetOffset(x-walker->xoffs,y-walker->yoffs);
    }
}

void QRepeaterGfx::setGlobalRegionIndex (int idx)
{
    for (int i = 0; i < gfxen.count(); ++i) {
        QGfxRec * walker = gfxen.at(i);
        walker->gfx->setGlobalRegionIndex(idx);
    }
}

void QRepeaterGfx::setDashedLines (bool b)
{
    for (int i = 0; i < gfxen.count(); ++i) {
        QGfxRec * walker = gfxen.at(i);
        walker->gfx->setDashedLines(b);
    }
}

void QRepeaterGfx::setDashes (char * c, int n)
{
    for (int i = 0; i < gfxen.count(); ++i) {
        QGfxRec * walker = gfxen.at(i);
        walker->gfx->setDashes(c,n);
    }
}

void QRepeaterGfx::setOpaqueBackground (bool b)
{
    for (int i = 0; i < gfxen.count(); ++i) {
        QGfxRec * walker = gfxen.at(i);
        walker->gfx->setOpaqueBackground(b);
    }
}

void QRepeaterGfx::setBackgroundColor (QColor c)
{
    for (int i = 0; i < gfxen.count(); ++i) {
        QGfxRec * walker = gfxen.at(i);
        walker->gfx->setBackgroundColor(c);
    }
}

void QRepeaterGfx::drawPoint (int x, int y)
{
    for (int i = 0; i < gfxen.count(); ++i) {
        QGfxRec * walker = gfxen.at(i);
        walker->gfx->drawPoint(x,y);
    }
}

void QRepeaterGfx::drawPoints (const QPolygon & a, int b, int c)
{
    for (int i = 0; i < gfxen.count(); ++i) {
        QGfxRec * walker = gfxen.at(i);
        QPolygon d=a;
        //d.translate(-(walker->xoffs),-(walker->yoffs));
        walker->gfx->drawPoints(d,b,c);
    }
}

void QRepeaterGfx::moveTo (int x, int y)
{
    for (int i = 0; i < gfxen.count(); ++i) {
        QGfxRec * walker = gfxen.at(i);
        walker->gfx->moveTo(x,y);
    }
}

void QRepeaterGfx::lineTo (int x, int y)
{
    for (int i = 0; i < gfxen.count(); ++i) {
        QGfxRec * walker = gfxen.at(i);
        walker->gfx->lineTo(x,y);
    }
}

void QRepeaterGfx::drawLine (int x1, int y1, int x2, int y2)
{
    for (int i = 0; i < gfxen.count(); ++i) {
        QGfxRec * walker = gfxen.at(i);
        walker->gfx->drawLine(x1,y1,x2,y2);
    }
}

void QRepeaterGfx::drawPolyline (const QPolygon & a, int b, int c)
{
    for (int i = 0; i < gfxen.count(); ++i) {
        QGfxRec * walker = gfxen.at(i);
        QPolygon d=a;
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
    for (int i = 0; i < gfxen.count(); ++i) {
        QGfxRec * walker = gfxen.at(i);
        walker->gfx->fillRect(x,y,w,h);
    }
}

void QRepeaterGfx::drawPolygon (const QPolygon & a, bool b, int c, int d)
{
    for (int i = 0; i < gfxen.count(); ++i) {
        QGfxRec * walker = gfxen.at(i);
        QPolygon e=a;
        //e.translate(-(walker->xoffs),-(walker->yoffs));
        walker->gfx->drawPolygon(e,b,c,d);
    }
}

void QRepeaterGfx::setLineStep (int l)
{
    for (int i = 0; i < gfxen.count(); ++i) {
        QGfxRec * walker = gfxen.at(i);
        walker->gfx->setLineStep(l);
    }
}

void QRepeaterGfx::blt (int x, int y, int w, int h, int sx, int sy)
{
    for (int i = 0; i < gfxen.count(); ++i) {
        QGfxRec * walker = gfxen.at(i);
        walker->gfx->blt(x,y,w,h,sx,sy);
    }
}

void QRepeaterGfx::stretchBlt (int x, int y, int w, int h, int sw, int sh)
{
    for (int i = 0; i < gfxen.count(); ++i) {
        QGfxRec * walker = gfxen.at(i);
        walker->gfx->stretchBlt(x,y,w,h,
                                sw,sh);
    }
}

void QRepeaterGfx::tiledBlt (int x, int y, int w, int h)
{
    for (int i = 0; i < gfxen.count(); ++i) {
        QGfxRec * walker = gfxen.at(i);
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
    for (int i = 0; i < gfxen.count(); ++i) {
        QGfxRec * walker = gfxen.at(i);
        walker->gfx->setSource(p);
    }
}

void QRepeaterGfx::setSource (const QImage * img)
{
    desktopsource=false;
    for (int i = 0; i < gfxen.count(); ++i) {
        QGfxRec * walker = gfxen.at(i);
        walker->gfx->setSource(img);
    }
}

void QRepeaterGfx::setSource (unsigned char * c,int w,int h,int l,
                              int d,QRgb * r,int n)
{
    desktopsource=false;
    for (int i = 0; i < gfxen.count(); ++i) {
        QGfxRec * walker = gfxen.at(i);
        walker->gfx->setSource(c,w,h,l,d,r,n);
    }
}

void QRepeaterGfx::setSourcePen ()
{
    desktopsource=false;
    for (int i = 0; i < gfxen.count(); ++i) {
        QGfxRec * walker = gfxen.at(i);
        walker->gfx->setSourcePen();
    }
}

void QRepeaterGfx::setAlphaType (QGfx::AlphaType a)
{
    for (int i = 0; i < gfxen.count(); ++i) {
        QGfxRec * walker = gfxen.at(i);
        walker->gfx->setAlphaType(a);
    }
}

void QRepeaterGfx::setAlphaSource (unsigned char * c, int linestep)
{
    for (int i = 0; i < gfxen.count(); ++i) {
        QGfxRec * walker = gfxen.at(i);
        walker->gfx->setAlphaSource(c,linestep);
    }
}

void QRepeaterGfx::setAlphaSource (int a, int b, int c, int d)
{
    for (int i = 0; i < gfxen.count(); ++i) {
        QGfxRec * walker = gfxen.at(i);
        walker->gfx->setAlphaSource(a,b,c,d);
    }
}

void QRepeaterGfx::setClut (QRgb * r, int n)
{
    for (int i = 0; i < gfxen.count(); ++i) {
        QGfxRec * walker = gfxen.at(i);
        walker->gfx->setClut(r,n);
    }
}

void QRepeaterGfx::save ()
{
    for (int i = 0; i < gfxen.count(); ++i) {
        QGfxRec * walker = gfxen.at(i);
        walker->gfx->save();
    }
}

void QRepeaterGfx::restore ()
{
    for (int i = 0; i < gfxen.count(); ++i) {
        QGfxRec * walker = gfxen.at(i);
        walker->gfx->restore();
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

    virtual bool restoreUnder(const QRect &)
        { return false; }
    virtual void saveUnder() {}
    virtual void drawCursor() {}

    virtual bool supportsAlphaCursor() { return false; }

    static bool enabled() { return false; }

    int xpos;
    int ypos;
    bool visible;

private:

    QList<QCursorRec*> cursors;

};

class QScreenRec {

public:

    QScreenRec(QScreen * s,char * sl,QString f,bool b) { screen=s; xoffs=-1;
                                       slot=qstrdup(sl); swcursor=b;
                                       fb=f; yoffs=0; }

    QScreen * screen;
    int xoffs;
    int yoffs;
    QScreenCursor * cursor;
    char * slot;
    bool swcursor;
    QString fb;

};

int QRepeaterScreen::sharedRamSize(void * end)
{
    int count=0;
    void * tmp=end;
    for (int i = 0; i < screens.count(); ++i) {
        QScreenRec * it = screens.at(i);;
        int ret=it->screen->sharedRamSize(tmp);
        ((char *)tmp)-=ret;
        count+=ret;
    }
    return count;
}

void QRepeaterScreen::setDirty(const QRect & r)
{
    for (int i = 0; i < screens.count(); ++i) {
        QScreenRec * walker = screens.at(i);;
         QRect r2=r;
         r2.translate(-(walker->xoffs),-(walker->yoffs));
         QRect screenrect(walker->xoffs,walker->yoffs,
                          walker->screen->width(),
                          walker->screen->height());
         if(r2.intersects(screenrect)) {
             r2=r2.intersect(screenrect);
             walker->screen->setDirty(r2);
         }
     }
}

extern char * qt_qws_hardcoded_slot;

static QScreen *qt_lookup_screen(int display_id, QString driver)
{
    QStringList driverList = QGfxDriverFactory::keys();
    QStringList::Iterator it;
    for (it = driverList.begin(); it != driverList.end(); ++it) {
        if (driver.isEmpty() || QString(*it) == driver) {
            QScreen *ret = QGfxDriverFactory::create(*it, display_id);
            if (ret)
                return ret;
        }
    }
    return 0;
}

QRepeaterScreen::QRepeaterScreen(int)
    : QScreen(0)
{
    data=(uchar *)0xdeadbeef;
    sw_cursor_exists=false;

    // Use config file - not sure if this is the right place to put it
    QString fn = QLibraryInfo::location(QLibraryInfo::PrefixPath) + "/lib/fonts/screens";
    FILE * screendef=fopen(fn.toLocal8Bit().constData(),"r");
    if(!screendef) {
        sw_cursor_exists=false;
        screens.append(new QScreenRec(new QLinuxFbScreen(0),
                                      "/proc/bus/pci/01/00.0",":0",true));
        return;
    }

    char buf[200]="";
    char name[200]="";
    char spec[200]="";
    char pci[200]="";

    fgets(buf,200,screendef);
    while(!feof(screendef)) {
        if(buf[0]!='#') {
            int num;
            int swcursor;
            int x;
            int y;
            sscanf(buf,"%s %d %s %d %s %d %d",name,&num,spec,&swcursor,
                   pci,&x,&y);
            QScreen * tmp=qt_lookup_screen(num,name);
            if(!tmp) {
                qDebug("Failure to find repeater screen %s",buf);
            } else {
                QScreenRec * tmp2=new QScreenRec(tmp,pci,spec,swcursor==0);
                tmp2->xoffs=x;
                tmp2->yoffs=y;
                screens.append(tmp2);
                if(swcursor!=0) {
                    sw_cursor_exists=true;
                }
            }
        }
        fgets(buf,200,screendef);
    }
    fclose(screendef);
    qt_screen=this;
}

int QRepeaterScreen::initCursor(void * v,bool b)
{
    QRepeaterCursor * qrc=new QRepeaterCursor();
    unsigned char * c=(unsigned char *)v;
    int count=0;
    QScreen * tmp=qt_screen;
    for (int i = 0; i < screens.count(); ++i) {
        QScreenRec * walker = screens.at(i);;
        qt_screen=walker->screen;
        int am=walker->screen->initCursor((void *)c,b);
        walker->cursor=qt_screencursor;
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
    for (int i = 0; i < screens.count(); ++i) {
        QScreenRec * walker = screens.at(i);;
        delete walker->screen;
    }
}

bool QRepeaterScreen::initDevice()
{
    for (int i = 0; i < screens.count(); ++i) {
        QScreenRec * walker = screens.at(i);;
        walker->screen->initDevice();
    }
    return true;
}

bool QRepeaterScreen::connect(const QString &)
{
    d=0;
    w=0;
    h=0;

    int wcount=0;

    for (int i = 0; i < screens.count(); ++i) {
        QScreenRec * walker = screens.at(i);;
        qt_qws_hardcoded_slot=walker->slot;
        walker->screen->connect(walker->fb);
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

    if(d==0)
        d=32;

    lstep=0xdeadbeef;
    pixeltype=NormalPixel;
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
    for (int i = 0; i < screens.count(); ++i) {
        QScreenRec * walker = screens.at(i);;
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
    for (int i = 0; i < cursors.count(); ++i) {
        QCursorRec * walker = cursors.at(i);;
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
    tmp->screen = s;
    cursors.append(tmp);
}

void QRepeaterCursor::init(SWCursorData *,bool)
{
    // Handled by screen's cursor construction method
}

void QRepeaterCursor::set(const QImage &image,int hotx,int hoty)
{
    QScreen * tmp=qt_screen;
    for (int i = 0; i < cursors.count(); ++i) {
        QCursorRec * walker = cursors.at(i);;
        if (walker->cursor) {
            qt_screen = walker->screen;
            walker->cursor->set(image,hotx,hoty);
        }
    }
    qt_screen = tmp;
}

void QRepeaterCursor::move(int x,int y)
{
    QScreen *tmp = qt_screen;
    for (int i = 0; i < cursors.count(); ++i) {
        QCursorRec * walker = cursors.at(i);;
        int xx=x-(walker->xoffs);
        int yy=y-(walker->yoffs);
        if (walker->cursor) {
            qt_screen = walker->screen;
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
    }
    xpos=x;
    ypos=y;
    qt_screen = tmp;
}

void QRepeaterCursor::show()
{
    QScreen *tmp = qt_screen;
    for (int i = 0; i < cursors.count(); ++i) {
        QCursorRec * walker = cursors.at(i);;
        if(walker->visible && walker->cursor) {
            qt_screen = walker->screen;
            walker->cursor->show();
        }
   }
    qt_screen = tmp;
    visible=true;
}

void QRepeaterCursor::hide()
{
    QScreen * tmp=qt_screen;
    for (int i = 0; i < cursors.count(); ++i) {
        QCursorRec * walker = cursors.at(i);;
        if(walker->visible&&walker->cursor) {
            qt_screen = walker->screen;
            walker->cursor->hide();
        }
    }
    visible=false;
    qt_screen=tmp;
}

void QRepeaterGfx::scroll (int x, int y, int w, int h, int sx, int sy)
{
    QRect r1(x+xoffs,y+yoffs,w,h);
    QRect r2(sx+xoffs,sy+yoffs,w,h);

    QRegion destregion(r1);
    QRegion srcregion(r2);

    QRegion toupdate;

    for (int i = 0; i < gfxen.count(); ++i) {
        QGfxRec * walker = gfxen.at(i);
        QScreen * tmp=qt_screen;
        qt_screen=walker->screen;
        walker->gfx->setOffset(xoffs-walker->xoffs,
                               yoffs-walker->yoffs);
        walker->gfx->setClipping(false);
        walker->gfx->scroll(x,y,w,h,sx,sy);
        walker->gfx->setClipping(true);
        qt_screen=tmp;
        QRegion screen(QRect(walker->xoffs,walker->yoffs,
                             walker->w,walker->h));
        QRegion tmp1=destregion;
        QRegion tmp2=srcregion;
        QRegion tmp3=destregion.unite(srcregion);
        tmp3=tmp3.intersect(screen);

        QRegion tmp4=srcregion.intersect(screen);
        tmp4.translate(x-sx,y-sy);
        tmp4=srcregion.intersect(tmp4);

        tmp3=tmp3.subtract(tmp4);

        toupdate=toupdate.unite(tmp3);
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
