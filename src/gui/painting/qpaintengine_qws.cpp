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

#include <private/qpaintengine_qws_p.h>
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

#include <qmemorymanager_qws.h>

#define QT_NO_NATIVE_XFORM
#define QT_NO_NATIVE_PATH
#define QT_NO_NATIVE_GRADIENT
// #define QT_NO_NATIVE_ALPHA

// This function decides which native functions the painter should support.
// You may change the supported feature set by (un)commenting the defines above.
static QPaintEngine::PaintEngineFeatures qt_decide_paintengine_features()
{
    QPaintEngine::PaintEngineFeatures commonFeatures =
        QPaintEngine::UsesFontEngine
#ifndef QT_NO_NATIVE_XFORM
        | QPaintEngine::CoordTransform
        | QPaintEngine::PenWidthTransform
        | QPaintEngine::PixmapTransform
        | QPaintEngine::PixmapScale
        | QPaintEngine::ClipTransform
#endif
#ifndef QT_NO_NATIVE_PATH
        | QPaintEngine::PainterPaths
#endif
#ifndef QT_NO_NATIVE_GRADIENT
        | QPaintEngine::LinearGradientFill
#endif
#ifndef QT_NO_NATIVE_ALPHA
        | QPaintEngine::AlphaPixmap
        | QPaintEngine::AlphaFill
        | QPaintEngine::AlphaStroke
#endif
        | QPaintEngine::QwsPaintEngine
        ;
    return commonFeatures;
}

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

void qt_draw_background(QPaintEngine * /*pe*/, int/* x*/, int /*y*/, int /*w*/,  int /*h*/)
{
//     QWSPaintEngine *p = static_cast<QWSPaintEngine *>(pe);
// // //     XSetForeground(p->d->dpy, p->d->gc, p->d->bg_brush.color().pixel(p->d->scrn));
// // //     qt_draw_transformed_rect(pp, x, y, w, h, true);
// // //     XSetForeground(p->d->dpy, p->d->gc, p->d->cpen.color().pixel(p->d->scrn));
}
// ########


#define d d_func()
#define q q_func()

QWSPaintEngine::QWSPaintEngine(QPaintEnginePrivate &dptr)
    : QPaintEngine(dptr, qt_decide_paintengine_features())
{
}

QWSPaintEngine::QWSPaintEngine()
    : QPaintEngine(*(new QWSPaintEnginePrivate), qt_decide_paintengine_features())
{
}

QWSPaintEngine::~QWSPaintEngine()
{
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
        if (w->isVisible() && w->window()->isVisible()) {
            int rgnIdx = w->window()->data->alloc_region_index;
            if (rgnIdx >= 0) {
                r = w->testAttribute(Qt::WA_PaintUnclipped) ? w->d->allocatedRegion() : w->d->paintableRegion();
                QRegion req;
                bool changed = false;
                QWSDisplay::grab();
                const int *rgnRev = QPaintDevice::qwsDisplay()->regionManager()->revision(rgnIdx);
                if (w->window()->data->alloc_region_revision != *rgnRev) {
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
            d->gfx->setClipRegion(w->rect(), Qt::ReplaceClip);
         else
#endif
             d->gfx->setClipRegion(QRegion(), Qt::NoClip);
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
            d->gfx->setClipRegion(QRect(0,0,data->w,data->h), Qt::ReplaceClip); //### default for gfx?????
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

    // make sure that paintEventClipRegion is set on the gfx
    updateClipRegion(QRegion(), Qt::NoClip); // checks for isActive(), so must be after setActive.

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

void QWSPaintEngine::updateBrush(const QBrush &brush, const QPointF &bgOrigin)
{
    if (!d->gfx)
        return;
    d->gfx->setBrush(brush);
    d->gfx->setBrushOrigin(qRound(bgOrigin.x()), qRound(bgOrigin.y()));
}

void QWSPaintEngine::updateFont(const QFont & /*font*/)
{
//    qDebug("QWSPaintEngine::updateFont");
}
void QWSPaintEngine::updateBackground(Qt::BGMode /*mode*/, const QBrush & /*bgBrush*/)
{
//    qDebug("QWSPaintEngine::updateBackground");
}
void QWSPaintEngine::updateMatrix(const QMatrix &)
{
//    qDebug("QWSPaintEngine::updateMatrix");
}
void QWSPaintEngine::updateClipRegion(const QRegion &clipRegion, Qt::ClipOperation op)
{
    bool clipEnabled = op != Qt::NoClip;
    Q_ASSERT(isActive());
    QRegion sysClip = systemClip();
    bool eventClip = !sysClip.isEmpty();

/*
  if (enable == testf(ClipOn)
  && (paintEventDevice != device() || !enable
  || !paintEventSaveRegion || paintEventSaveRegion->isNull()))
  return;
*/
    if (clipEnabled || eventClip) {
        QRegion crgn;
        if (clipEnabled) {
            crgn = clipRegion;
            if (eventClip)
                crgn = crgn.intersect(sysClip);
        } else {
            crgn = sysClip;
            op = Qt::ReplaceClip;
        }
        //note that gfx is already translated by redirection_offset
        d->gfx->setClipRegion(crgn, op);
    } else {
        d->gfx->setClipping(false);
    }

    if (clipEnabled)
        setf(ClipOn);
    else
        clearf(ClipOn);
}

void QWSPaintEngine::drawLines(const QLineF *lines, int lineCount)
{
    if (state->pen.style() == Qt::NoPen)
        return;
    for (int i=0; i<lineCount; ++i) {
        d->gfx->drawLine(qRound(lines->x1()), qRound(lines->y1()), qRound(lines->x2()), qRound(lines->y2()));
        ++lines;
    }
}

void QWSPaintEngine::drawRects(const QRectF *rects, int rectCount)
{
    for (int i=0; i<rectCount; ++i) {
        QRectF r = rects[i];

        int w = qRound(r.width());
        int h = qRound(r.height());
        if (!w && !h)
            return;
        int x1 = qRound(r.x());
        int y1 = qRound(r.y());

        if (state->pen.style() != Qt::NoPen) {
            if (state->pen.width() > 1) {
                QPolygon a(QRect(x1,y1,w,h), true);
                drawPolyInternal(a);
                return;
            }
            int x2 = x1 + w;
            int y2 = y1 + h;
            bool paintBottom = y1 < y2;
            bool paintLeft   = y1 < y2 - 1;
            bool paintRight  = x1 < x2 && paintLeft;

            d->gfx->drawLine(x1, y1, x2, y1); // Top
            if (paintBottom)
                d->gfx->drawLine(x1, y2, x2, y2); // Bottom
            if (paintLeft)
                d->gfx->drawLine(x1, y1+1, x1, y2-1); // Left
            if (paintRight)
                d->gfx->drawLine(x2, y1+1, x2, y2-1); // Right

            x1 += 1;
            y1 += 1;
            w -= 1;
            h -= 1;
        }

        if (w > 0 && h > 0)
            d->gfx->fillRect(x1, y1, w, h);
    }
}

void QWSPaintEngine::drawPoints(const QPointF *points, int pointCount)
{
    for (int i=0; i<pointCount; ++i) {
        d->gfx->drawPoint(qRound(points->x()), qRound(points->y()));
        ++points;
    }
}

void QWSPaintEngine::drawPolyInternal(const QPolygon &a, bool close)
{
    if (a.size() < 2 || !d->gfx)
        return;

    int x1, y1, x2, y2;                                  // connect last to first point
    a.point(a.size()-1, &x1, &y1);
    a.point(0, &x2, &y2);
    bool do_close = close && !(x1 == x2 && y1 == y2);

    if (close && state->brush.style() != Qt::NoBrush) { // draw filled polygon
        d->gfx->drawPolygon(a,false,0,a.size());
        if (state->pen.style() == Qt::NoPen) {          // draw fake outline
            d->gfx->drawPolyline(a,0,a.size());
            if (do_close)
                d->gfx->drawLine(x1,y1,x2,y2);
        }
    }
    if (state->pen.style() != Qt::NoPen) {              // draw outline
        d->gfx->drawPolyline(a,0,a.size());
        if (do_close)
            d->gfx->drawLine(x1,y1,x2,y2);
    }
}

void QWSPaintEngine::drawEllipse(const QRectF &r)
{
    QPainterPath path;
#ifndef QT_QWS_NO_STUPID_HACKS
    if (state->pen.style() == Qt::NoPen) {
        QPen savePen = state->pen;
        updatePen(QPen(state->brush.color()));
        path.addEllipse(r.x(), r.y(), r.width()-1, r.height()-1);
        drawPolyInternal(path.toSubpathPolygons().at(0).toPolygon());
        updatePen(savePen);
    } else
#endif
    {
    path.addEllipse(r.x(), r.y(), r.width(), r.height());
    drawPolyInternal(path.toSubpathPolygons().at(0).toPolygon());
    }
}

void QWSPaintEngine::drawPolygon(const QPointF *points, int pointCount, PolygonDrawMode mode)
{
    QPolygon pa;
    pa.reserve(pointCount);
    for (int i=0; i<pointCount; ++i)
        pa << points[i].toPoint();

    if (mode == PolylineMode) {
        if (state->pen.style() != Qt::NoPen)
            d->gfx->drawPolyline(pa, 0, pa.size());
    } else {
        d->gfx->drawPolygon(pa, (mode == WindingMode), 0, pa.size());
    }

}

void QWSPaintEngine::drawPixmap(const QRectF &r, const QPixmap &pixmap, const QRectF &sr,
                                Qt::PixmapDrawingMode mode)
{
    if (pixmap.isNull())
        return;
    int x = qRound(r.x());
    int y = qRound(r.y());
    int w = qRound(r.width());
    int h = qRound(r.height());

    int sx = qRound(sr.x());
    int sy = qRound(sr.y());
    int sw = qRound(sr.width());
    int sh = qRound(sr.height());

    bool hasAlpha = pixmap.data->hasAlpha;
    bool hasMask = !pixmap.mask().isNull();

    if ((w != sw || h != sh) && (sx != 0 || sy != 0))
        qDebug("QWSPaintEngine::drawPixmap offset stretch not implemented");

    QPixmap no_mask;
    if (hasAlpha && hasMask) {
        // convert the mask to alpha 0, since gfx doesn't yet support both mask and alpha
        QImage img = pixmap.toImage();
        no_mask = QPixmap::fromImage(img);
        d->gfx->setSource(&no_mask);
        hasMask = false;
    } else {
        d->gfx->setSource(&pixmap);
    }

    if(mode == Qt::ComposePixmap && hasMask) {
        QBitmap mymask = pixmap.mask();
        const unsigned char * thebits = mymask.qwsScanLine(0);
        int ls = mymask.qwsBytesPerLine();
        d->gfx->setAlphaType(QGfx::LittleEndianMask);
        d->gfx->setAlphaSource(const_cast<uchar*>(thebits),ls);
    } else {
        QGfx::AlphaType alphaType = QGfx::IgnoreAlpha;
        if (mode != Qt::CopyPixmapNoMask && mode != Qt::CopyPixmap) {
            if (hasAlpha)
                alphaType = QGfx::InlineAlpha;
#if 0 //source-over-destination blending not supported for 4.0
            if (d->pdev->devType() == QInternal::Pixmap) {
                QPixmap *p = static_cast<QPixmap*>(d->pdev);
                if (p->data->hasAlpha)
                    alphaType = QGfx::DestinationAlpha;
            }
#endif
        }
        d->gfx->setAlphaType(alphaType);
    }
    if (sw == w && sh == h)
        d->gfx->blt(x,y,sw,sh,sx,sy);
    else
        d->gfx->stretchBlt(x,y,w,h,sw,sh);
}


Qt::HANDLE QWSPaintEngine::handle() const{
    qDebug("QWSPaintEngine::handle");
    return 0;
}


void QWSPaintEngine::initialize(){
//        qDebug("QWSPaintEngine::initialize");
}
void QWSPaintEngine::cleanup(){
//        qDebug("QWSPaintEngine::cleanup");
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
    d->gfx->setBrushOrigin(rx-sx, ry-sy);
    d->gfx->tiledBlt(rx, ry, w, h);
    if (state)
        d->gfx->setBrushOrigin(qRound(state->bgOrigin.x()), qRound(state->bgOrigin.y()));
    else
        d->gfx->setBrushOrigin(0, 0);
}

//### This doesn't really belong here; we need qscreen_qws.cpp

/*!
    \internal
*/
QWSPaintEngine *QScreen::createPaintEngine()
{
    return new QWSPaintEngine;
}
