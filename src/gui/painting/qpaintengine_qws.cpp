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

#include "qpaintengine_qws.h"
#include "qpainter_p.h"
#include <private/qfontengine_p.h>
#include "qwsdisplay_qws.h"
#include "qwidget.h"
#include "private/qwidget_p.h"
#include "qwsregionmanager_qws.h"

#include "qpixmap.h"
#include "private/qpixmap_p.h"
#include "qgfx_qws.h"
#include "qscreen_qws.h"
#include "qregion.h"
#include "qbitmap.h"
#include <private/qpaintengine_p.h>

#include <qmemorymanager_qws.h>

/* paintevent magic to provide Windows semantics on Qt/E
 */
static QRegion* paintEventClipRegion = 0;
//static QRegion* paintEventSaveRegion = 0;
static QPaintDevice* paintEventDevice = 0;

void qt_set_paintevent_clipping(QPaintDevice* dev, const QRegion& region)
{
    if (!paintEventClipRegion)
        paintEventClipRegion = new QRegion(region);
    else
        *paintEventClipRegion = region;
    paintEventDevice = dev;

#ifdef QWS_EXTRA_DEBUG
    qDebug("qt_set_paintevent_clipping");
    QMemArray<QRect> ar = region.rects();
    for (int i=0; i<int(ar.size()); i++) {
        QRect r = ar[i];
        qDebug("   r[%d]:  %d,%d %dx%d", i,
                r.x(), r.y(), r.width(), r.height());
    }
#endif
}

void qt_clear_paintevent_clipping()
{
    delete paintEventClipRegion;
//    delete paintEventSaveRegion;
    paintEventClipRegion = 0;
//    paintEventSaveRegion = 0;
    paintEventDevice = 0;
}

static QList<QPainter*> *widgetPainterList = 0;

void qwsUpdateActivePainters()
{
    /* ##############
    if (widgetPainterList) {
        for (int i = 0; i < widgetPainterList->size(); ++i) {
            QPainter *ptr = widgetPainterList->at(i);
            ptr->save();
            delete ptr->gfx;
            ptr->gfx = ptr->device()->graphicsContext();
            ptr->setf(QPainter::VolatileDC);
            ptr->restore();
        }
    }
    */
}


void qt_draw_background(QPaintEngine *pe, int/* x*/, int /*y*/, int /*w*/,  int /*h*/)
{
    QWSPaintEngine *p = static_cast<QWSPaintEngine *>(pe);
// // //     XSetForeground(p->d->dpy, p->d->gc, p->d->bg_brush.color().pixel(p->d->scrn));
// // //     qt_draw_transformed_rect(pp, x, y, w, h, true);
// // //     XSetForeground(p->d->dpy, p->d->gc, p->d->cpen.color().pixel(p->d->scrn));
}
// ########


class QWSPaintEnginePrivate : public QPaintEnginePrivate
{
    Q_DECLARE_PUBLIC(QWSPaintEngine)
public:
    QWSPaintEnginePrivate() :gfx(0), clipChildren(true) {}
    QGfx *gfx;
    bool clipChildren;
};

#define d d_func()
#define q q_func()

QWSPaintEngine::QWSPaintEngine(QPaintEnginePrivate &dptr)
    : QPaintEngine(dptr, UsesFontEngine)
{
//    d->pdev = pdev;
//        qDebug("QWSPaintEngine::QWSPaintEngine");
}

QWSPaintEngine::QWSPaintEngine()
    : QPaintEngine(*(new QWSPaintEnginePrivate), UsesFontEngine)
{
//        qDebug("QWSPaintEngine::QWSPaintEngine");
}

QWSPaintEngine::~QWSPaintEngine()
{
//        qDebug("QWSPaintEngine::~QWSPaintEngine");
}



bool QWSPaintEngine::begin(QPaintDevice *pdev)
{
    if (isActive()) {                         // already active painting
        qWarning("QWSC::begin: Painter is already active."
                  "\n\tYou must end() the painter before a second begin()");
        return true;
    }

    Q_ASSERT(d->gfx == 0);

    d->pdev = pdev;

    if (d->pdev->devType() == QInternal::Widget) {
        QWidget *w =  static_cast<QWidget *>(d->pdev) ;

        d->gfx=qt_screen->screenGfx();

        QPoint offset=w->mapToGlobal(QPoint(0,0));
        QRegion r; // empty if not visible
        if (w->isVisible() && w->topLevelWidget()->isVisible()) {
            int rgnIdx = w->topLevelWidget()->data->alloc_region_index;
            if (rgnIdx >= 0) {
                r = w->testAttribute(Qt::WA_PaintUnclipped) ? w->allocatedRegion() : w->paintableRegion();
                QRegion req;
                bool changed = false;
                QWSDisplay::grab();
                const int *rgnRev = QPaintDevice::qwsDisplay()->regionManager()->revision(rgnIdx);
                if (w->topLevelWidget()->data->alloc_region_revision != *rgnRev) {
                    // The TL region has changed, so we better make sure we're
                    // not writing to any regions we don't own anymore.
                    // We'll get a RegionModified event soon that will get our
                    // regions back in sync again.
                    req = QPaintDevice::qwsDisplay()->regionManager()->region(rgnIdx);
                    changed = true;
                }
                d->gfx->setGlobalRegionIndex(rgnIdx);
                QWSDisplay::ungrab();
                if (changed) {
                    r &= req;
                }
            }
        }
        d->gfx->setWidgetDeviceRegion(r);
        d->gfx->setOffset(offset.x(),offset.y());
        // Clip the window decoration for TL windows.
        // It is possible for these windows to draw on the wm decoration if
        // they change the clip region.  Bug or feature?
#ifndef QT_NO_QWS_MANAGER
         if (w->d->extra && w->d->extra->topextra && w->d->extra->topextra->qwsManager)
            d->gfx->setClipRegion(w->rect());
#endif
    } else if (d->pdev->devType() == QInternal::Pixmap) {
        QPixmap *p = static_cast<QPixmap*>(d->pdev);
        if(p->isNull()) {
            qDebug("Can't make QGfx for null pixmap\n");
            d->gfx = 0;
        } else {
            uchar * mydata;
            int xoffset,linestep;
            QPixmapData *data=p->data;
            int depth = p->depth();

            memorymanager->findPixmap(data->id,data->rw,p->depth(),&mydata,&xoffset,&linestep);

            d->gfx = QGfx::createGfx(p->depth(), mydata, data->w,data->h, linestep);
            if(depth <= 8) {
                if(depth==1 && !(data->clut)) {
                    data->clut=new QRgb[2];
                    data->clut[0]=qRgb(255,255,255);
                    data->clut[1]=qRgb(0,0,0);
                    data->numcols = 2;
                }
                if (data->numcols)
                    d->gfx->setClut(data->clut,data->numcols);
            }
        }

    } else {
        qFatal("QWSPaintEngine can only do widgets and pixmaps");
    }

//    qDebug("QWSPaintEngine::begin %p gfx %p", this, d->gfx);
    setActive(true);

    // ### SHould be done by QPainter..
//     updatePen(ps);
//     updateBrush(ps);
//     updateClipRegion(ps);

    return true;
}



bool QWSPaintEngine::begin(QImage *img)
{
    if (isActive()) {                         // already active painting
        qWarning("QWSPaintEngine::begin: Painter is already active."
                 "\n\tYou must end() the painter before a second begin()");
        return true;
    }

    Q_ASSERT(d->gfx == 0);
    d->pdev = 0;

    if(!img->depth()) {
        qWarning("Trying to create image for null depth");
        return 0;
    }

    //###??? QSize s = qt_screen->mapToDevice(QSize(img->width(),img->height()));
    d->gfx = QGfx::createGfx(img->depth(), img->bits(), img->width(), img->height(), img->bytesPerLine());
    if(img->depth()<=8) {
        QRgb * tmp=img->colorTable();
        int nc=img->numColors();
        if(tmp==0) {
            static QRgb table[2] = { qRgb(255,255,255), qRgb(0,0,0) };
            tmp=table;
            nc=2;
        }
        d->gfx->setClut(tmp,nc);
    }

//    qDebug("QWSPaintEngine::begin(QImage*) %p gfx %p", this, d->gfx);
    setActive(true);

    // ### SHould be done by QPainter..
//     updatePen(ps);
//     updateBrush(ps);
//     updateClipRegion(ps);

    return true;



}




bool QWSPaintEngine::begin(QScreen *screen)
{
    if (isActive()) {                         // already active painting
        qWarning("QWSPaintEngine::begin: Painter is already active."
                 "\n\tYou must end() the painter before a second begin()");
        return true;
    }

    Q_ASSERT(d->gfx == 0);
    d->pdev = 0;
    d->gfx = screen->screenGfx();

    setActive(true);

    return true;
}




bool QWSPaintEngine::end(){
    setActive(false);
//    qDebug("QWSPaintEngine::end %p (gfx %p)", this, d->gfx);
    delete d->gfx;
    d->gfx = 0;
    return true;
}

void QWSPaintEngine::updatePen(const QPen &pen)
{
    d->gfx->setPen(pen);

//    qDebug("QWSPaintEngine::updatePen");
}


QPixmap qt_pixmapForBrush(int brushStyle, bool invert); //in qbrush.cpp


void QWSPaintEngine::updateBrush(const QBrush &brush, const QPoint &)
{
    if (!d->gfx)
        return;
    int bs=brush.style();
    if (bs >= Qt::Dense1Pattern && bs <= Qt::DiagCrossPattern) {
            QPixmap *pm = new QPixmap(qt_pixmapForBrush(bs, false));
            d->gfx->setBrushPixmap(pm);
    } else {
        d->gfx->setBrushPixmap(brush.pixmap());
    }
    d->gfx->setBrush(brush);
}

void QWSPaintEngine::updateFont(const QFont &font)
{
//    qDebug("QWSPaintEngine::updateFont");
}
void QWSPaintEngine::updateBackground(Qt::BGMode mode, const QBrush &bgBrush)
{
//    qDebug("QWSPaintEngine::updateBackground");
}
void QWSPaintEngine::updateMatrix(const QMatrix &)
{
//    qDebug("QWSPaintEngine::updateMatrix");
}
void QWSPaintEngine::updateClipRegion(const QRegion &clipRegion, bool clipEnabled)
{
//    qDebug("QWSPaintEngine::updateClipRegion");

    Q_ASSERT(isActive());

    bool painterClip = clipEnabled;
    bool eventClip = paintEventDevice == d->pdev && paintEventClipRegion;
/*
  if (enable == testf(ClipOn)
  && (paintEventDevice != device() || !enable
  || !paintEventSaveRegion || paintEventSaveRegion->isNull()))
  return;
*/

    if (painterClip || eventClip) {
        QRegion crgn;
        if (painterClip) {
            crgn = clipRegion;
            if (eventClip)
                crgn = crgn.intersect(*paintEventClipRegion);
        } else {
            crgn = *paintEventClipRegion;
        }
        //note that gfx is already translated by redirection_offset
        d->gfx->setClipRegion(crgn);
    } else {
        d->gfx->setClipping(false);
    }
    if (painterClip)
        setf(ClipOn);
    else
        clearf(ClipOn);
}

void QWSPaintEngine::drawLine(const QPoint &p1, const QPoint &p2)
{
    if (state->pen.style() != Qt::NoPen)
        d->gfx->drawLine(p1.x(), p1.y(), p2.x(), p2.y());
}
void QWSPaintEngine::drawRect(const QRect &r)
{
    //############ gfx->setBrushOffset(x-bro.x(), y-bro.y());

    int x1, y1, w, h;
    r.getRect(&x1, &y1, &w, &h);

    if (state->pen.style() != Qt::NoPen) {
        if (state->pen.width() > 1) {
            QPointArray a(r, true);
            drawPolyInternal(a);
            return;
        } else        {
            int x2 = x1 + (w-1);
            int y2 = y1 + (h-1);
            d->gfx->drawLine(x1, y1, x2, y1);
            d->gfx->drawLine(x2, y1, x2, y2);
            d->gfx->drawLine(x1, y2, x2, y2);
            d->gfx->drawLine(x1, y1, x1, y2);
            x1 += 1;
            y1 += 1;
            w -= 2;
            h -= 2;
        }
    }

    d->gfx->fillRect(x1, y1, w, h);
}
void QWSPaintEngine::drawPoint(const QPoint &p)
{
    d->gfx->drawPoint(p.x(), p.y());
}

void QWSPaintEngine::drawPoints(const QPointArray &pa)
{
    if (state->pen.style() == Qt::NoPen)
        return;

    d->gfx->drawPoints(pa, 0, pa.size());
}


void QWSPaintEngine::drawPolyInternal(const QPointArray &a, bool close)
{
    if (a.size() < 2 || !d->gfx)
        return;

    int x1, y1, x2, y2;                                // connect last to first point
    a.point(a.size()-1, &x1, &y1);
    a.point(0, &x2, &y2);
    bool do_close = close && !(x1 == x2 && y1 == y2);

    if (close && state->brush.style() != Qt::NoBrush) {        // draw filled polygon
        d->gfx->drawPolygon(a,false,0,a.size());
        if (state->pen.style() == Qt::NoPen) {                // draw fake outline
            d->gfx->drawPolyline(a,0,a.size());
            if (do_close)
                d->gfx->drawLine(x1,y1,x2,y2);
        }
    }
    if (state->pen.style() != Qt::NoPen) {                // draw outline
        d->gfx->drawPolyline(a,0,a.size());
        if (do_close)
            d->gfx->drawLine(x1,y1,x2,y2);
    }
}




void QWSPaintEngine::drawEllipse(const QRect &r)
{
    int x, y, w, h;
    r.getRect(&x, &y, &w, &h);

    QPointArray a;
// #ifndef QT_NO_TRANSFORMATIONS
//     a.makeArc(x, y, w, h, 0, 360*16, xmat);
//     a.translate(-redirection_offset);
// #else
//     map(x, y, &x, &y);
    a.makeEllipse(x, y, w, h);
//#endif
/*###########
    QPen oldpen=pen();
    QPen tmppen=oldpen;
    tmppen.setJoinStyle(Qt::BevelJoin);
    setPen(tmppen);
*/
    drawPolyInternal(a);
}

void QWSPaintEngine::drawPolygon(const QPointArray &pa, PolygonDrawMode mode)
{
#if 0
#ifndef QT_NO_TRANSFORMATIONS
    bool tx = (txop != TxNone);
#else
    bool tx = xlatex || xlatey;
#endif
    if (tx) {
        pa = xForm(a, index, npoints);
        if (pa.size() != a.size()) {
            index   = 0;
            npoints = pa.size();
        }
        pa.translate(-redirection_offset);
    }

#endif
    if (mode == UnconnectedMode) {
        if (state->pen.style() != Qt::NoPen)
            d->gfx->drawPolyline(pa, 0, pa.size());
    } else {
        d->gfx->drawPolygon(pa, (mode == WindingMode), 0, pa.size());
    }

}

void QWSPaintEngine::drawPixmap(const QRect &r, const QPixmap &pixmap, const QRect &sr,
                                Qt::PixmapDrawingMode mode)
    //(int x, int y, const QPixmap &pixmap, int sx, int sy, int sw, int sh)
{
    int x,y,w,h,sx,sy,sw,sh;
    r.getRect(&x, &y, &w, &h);
    sr.getRect(&sx, &sy, &sw, &sh);

    if ((w != sw || h != sh) && (sx != 0) && (sy != 0))
        qDebug("QWSPaintEngine::drawPixmap offset stretch notimplemented");

    d->gfx->setSource(&pixmap);
    if(mode == Qt::ComposePixmap && pixmap.mask()) {
        QBitmap * mymask=((QBitmap *)pixmap.mask());
        unsigned char * thebits=mymask->scanLine(0);
        int ls=mymask->bytesPerLine();
        d->gfx->setAlphaType(QGfx::LittleEndianMask);
        d->gfx->setAlphaSource(thebits,ls);
    } else if (pixmap.data->hasAlpha){
        d->gfx->setAlphaType(QGfx::InlineAlpha);
    } else {
        d->gfx->setAlphaType(QGfx::IgnoreAlpha);
    }
    if (sw == w && sh == h)
        d->gfx->blt(x,y,sw,sh,sx,sy);
    else
        d->gfx->stretchBlt(x,y,w,h,sw,sh);
}

void QWSPaintEngine::drawTiledPixmap(const QRect &r, const QPixmap &pixmap, const QPoint &s,
				     Qt::PixmapDrawingMode mode)
{
    qDebug("QWSPaintEngine::drawTiledPixmap");
}

Qt::HANDLE QWSPaintEngine::handle() const{
    qDebug("QWSPaintEngine::handle");
    return 0;
}


void QWSPaintEngine::initialize(){
        qDebug("QWSPaintEngine::initialize");
}
void QWSPaintEngine::cleanup(){
        qDebug("QWSPaintEngine::cleanup");
}

void QWSPaintEngine::setGlobalRegionIndex(int idx)
{
    d->gfx->setGlobalRegionIndex(idx);
}

void QWSPaintEngine::setWidgetDeviceRegion(const QRegion &r)
{
    d->gfx->setWidgetDeviceRegion(r);
}

void QWSPaintEngine::setClipDeviceRegion(const QRegion &r)
{
    d->gfx->setClipDeviceRegion(r);
}


void QWSPaintEngine::scroll(int rx,int ry,int w,int h,int sx, int sy)
{
    d->gfx->scroll(rx, ry, w, h,sx, sy);
}

void QWSPaintEngine::fillRect(int rx,int ry,int w,int h)
{
    d->gfx->fillRect(rx, ry, w, h);
}



void QWSPaintEngine::blt(const QPaintDevice &src, int rx,int ry,int w,int h, int sx, int sy)
{
    d->gfx->setSource(&src);
    d->gfx->setAlphaType(QGfx::IgnoreAlpha);
    d->gfx->blt(rx,ry,w,h,sx,sy);
}

void QWSPaintEngine::blt(const QImage &src, int rx,int ry,int w,int h, int sx, int sy)
{
    d->gfx->setSource(&src);
    d->gfx->setAlphaType(QGfx::IgnoreAlpha);
    d->gfx->blt(rx,ry,w,h,sx,sy);
}

void QWSPaintEngine::stretchBlt(const QPaintDevice &src, int rx,int ry,int w,int h, int sw,int sh)
{
    d->gfx->setSource(&src);
    d->gfx->setAlphaType(QGfx::IgnoreAlpha);
    d->gfx->stretchBlt(rx,ry,w,h,sw,sh);
}

void QWSPaintEngine::alphaPenBlt(const void* src, int bpl, bool mono, int rx,int ry,int w,int h, int sx, int sy)
{
    d->gfx->setSourcePen(); //### optimization: do this outside the loop...
    d->gfx->setAlphaType(mono ? QGfx::BigEndianMask : QGfx::SeparateAlpha);
    d->gfx->setAlphaSource((uchar*)src, bpl);
    d->gfx->blt(rx,ry,w,h,sx,sy);
    d->gfx->setAlphaType(QGfx::IgnoreAlpha); //### outside the loop ?
}

void QWSPaintEngine::tiledBlt(const QImage &src, int rx,int ry,int w,int h, int sx, int sy)
{
    d->gfx->setSource(&src);
    d->gfx->setBrushOffset(sx, sy);
    d->gfx->tiledBlt(rx, ry, w, h);
}

//########### This doesn't really belong here; we need qscreen_qws.cpp


QWSPaintEngine *QScreen::createPaintEngine()
{
    return new QWSPaintEngine;
}
