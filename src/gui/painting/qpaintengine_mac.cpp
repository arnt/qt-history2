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

#include <qbitmap.h>
#include <qpaintdevice.h>
#include <private/qpaintengine_mac_p.h>
#include <qpainterpath.h>
#include <qpixmapcache.h>
#include <private/qprintengine_mac_p.h>
#include <qprinter.h>
#include <qstack.h>
#include <qtextcodec.h>
#include <qtextcodec.h>
#include <qwidget.h>
#include <qvarlengtharray.h>

#include <private/qfont_p.h>
#include <private/qfontengine_p.h>
#include <private/qpaintengine_mac_p.h>
#include <private/qpainter_p.h>
#include <private/qpainterpath_p.h>
#include <private/qpixmap_p.h>
#include <private/qt_mac_p.h>
#include <private/qwidget_p.h>

#include <string.h>

#define d d_func()
#define q q_func()

/*****************************************************************************
  Internal variables and functions
 *****************************************************************************/

/*****************************************************************************
  External functions
 *****************************************************************************/
extern QPoint posInWindow(const QWidget *w); //qwidget_mac.cpp
extern WindowPtr qt_mac_window_for(const QWidget *); //qwidget_mac.cpp
extern GrafPtr qt_macQDHandle(const QPaintDevice *); //qpaintdevice_mac.cpp
extern CGContextRef qt_macCreateCGHandle(const QPaintDevice *); //qpaintdevice_mac.cpp
extern CGImageRef qt_mac_create_cgimage(const QPixmap &, Qt::PixmapDrawingMode, bool); //qpixmap_mac.cpp
extern void qt_mac_dispose_rgn(RgnHandle r); //qregion_mac.cpp
extern const uchar *qt_patternForBrush(int, bool); //qbrush.cpp
extern QPixmap qt_pixmapForBrush(int, bool); //qbrush.cpp

//Implemented for qt_mac_p.h
QMacCGContext::QMacCGContext(QPainter *p)
{
    QPaintEngine *pe = p->paintEngine();
    if(pe->type() == QPaintEngine::MacPrinter)
        pe = static_cast<QMacPrintEngine*>(pe)->paintEngine();
    Q_ASSERT(pe->type() == QPaintEngine::CoreGraphics);
    pe->syncState();
    context = static_cast<QCoreGraphicsPaintEngine*>(pe)->handle();
    CGContextRetain(context);
}

/*****************************************************************************
  QQuickDrawPaintEngine member functions
 *****************************************************************************/

inline static QPaintEngine::PaintEngineFeatures qt_mac_qd_features()
{
    return QPaintEngine::PaintEngineFeatures(
        QPaintEngine::UsesFontEngine|QPaintEngine::PixmapScale
        |QPaintEngine::AlphaPixmap|QPaintEngine::PenWidthTransform
        );
}

QQuickDrawPaintEngine::QQuickDrawPaintEngine()
    : QPaintEngine(*(new QQuickDrawPaintEnginePrivate), qt_mac_qd_features())
{
}

QQuickDrawPaintEngine::QQuickDrawPaintEngine(QPaintEnginePrivate &dptr, PaintEngineFeatures devcaps)
    : QPaintEngine(dptr, (devcaps ? devcaps : qt_mac_qd_features()))
{
}

QQuickDrawPaintEngine::~QQuickDrawPaintEngine()
{
}

bool
QQuickDrawPaintEngine::begin(QPaintDevice *pdev)
{
    if(isActive()) {                         // already active painting
        qWarning("QQuickDrawPaintEngine::begin: Painter is already active."
                  "\n\tYou must end() the painter before a second begin()");
        return false;
    }

    // Set up the polygon clipper.
    const int BUFFERZONE = 100;
    d->polygonClipper.setBoundingRect(QRect(-BUFFERZONE,
                                            -BUFFERZONE,
                                            pdev->width() + 2*BUFFERZONE,
                                            pdev->height() + 2 * BUFFERZONE));

    d->saved = new QMacSavedPortInfo;     //save the gworld now, we'll reset it in end()
    d->pdev = pdev;
    setActive(true);
    assignf(IsActive | DirtyFont);

    d->clip.serial = 0;
    d->clip.dirty = false;
    d->offx = d->offy = 0;
    bool unclipped = false;
    if(d->pdev->devType() == QInternal::Widget) {
        QWidget *w = static_cast<QWidget*>(d->pdev);
        { //offset painting in widget relative the tld
            QPoint wp = posInWindow(w);
            d->offx = wp.x();
            d->offy = wp.y();
        }
        bool unclipped = w->testAttribute(Qt::WA_PaintUnclipped);

        if(!d->locked) {
            LockPortBits(GetWindowPort(qt_mac_window_for(w)));
            d->locked = true;
        }

        if((w->windowType() == Qt::Desktop)) {
            if(!unclipped)
                qWarning("QQuickDrawPaintEngine::begin: Does not support clipped desktop on Mac OS X");
            ShowWindow(qt_mac_window_for(w));
        } else if(unclipped) {
            qWarning("QQuickDrawPaintEngine::begin: Does not support unclipped painting");
        }
    } else if(d->pdev->devType() == QInternal::Pixmap) {             // device is a pixmap
        QPixmap *pm = static_cast<QPixmap*>(d->pdev);
        if(pm->isNull()) {
            qWarning("QQuickDrawPaintEngine::begin: Cannot paint null pixmap");
            end();
            return false;
        }
    }
    d->unclipped = unclipped;

    setDirty(QPaintEngine::DirtyPen);
    setDirty(QPaintEngine::DirtyBrush);
    setDirty(QPaintEngine::DirtyBackground);

    if(type() != CoreGraphics)
        setupQDPort(true); //force setting paint device, this does unclipped fu
    return true;
}

bool
QQuickDrawPaintEngine::end()
{
    setActive(false);

    if(d->locked) {
        if(d->pdev->devType() == QInternal::Widget)
            UnlockPortBits(GetWindowPort(qt_mac_window_for(static_cast<QWidget*>(d->pdev))));
        d->locked = false;
    }

    delete d->saved;
    d->saved = 0;
    if(d->pdev->devType() == QInternal::Widget && static_cast<QWidget*>(d->pdev)->windowType() == Qt::Desktop)
        HideWindow(qt_mac_window_for(static_cast<QWidget*>(d->pdev)));

    d->pdev = 0;
    return true;
}

void
QQuickDrawPaintEngine::updatePen(const QPen &pen)
{
    d->current.pen = pen;
}


void
QQuickDrawPaintEngine::updateBrush(const QBrush &brush, const QPointF &origin)
{
    d->current.brush = brush;
    d->current.bg.origin = origin;
}

void
QQuickDrawPaintEngine::updateFont(const QFont &)
{
    clearf(DirtyFont);
    updatePen(d->current.pen);
}

void
QQuickDrawPaintEngine::updateBackground(Qt::BGMode mode, const QBrush &bgBrush)
{
    Q_ASSERT(isActive());
    d->current.bg.mode = mode;
    d->current.bg.brush = bgBrush;
}

void
QQuickDrawPaintEngine::updateMatrix(const QMatrix &)
{
}

void
QQuickDrawPaintEngine::setClippedRegionInternal(QRegion *rgn)
{
    if(rgn) {
        d->current.clip = *rgn;
        setf(ClipOn);
    } else {
        d->current.clip = QRegion();
        clearf(ClipOn);
    }
    d->clip.dirty = 1;
}

void
QQuickDrawPaintEngine::updateClipRegion(const QRegion &region, Qt::ClipOperation op)
{
    Q_ASSERT(isActive());
    if(op == Qt::NoClip) {
        setClippedRegionInternal(0);
    } else {
        QRegion clip = region;
        if(testf(ClipOn)) {
            if(op == Qt::IntersectClip)
                clip = d->current.clip.intersect(clip);
            else if(op == Qt::UniteClip)
                clip = d->current.clip.unite(clip);
        }
        setClippedRegionInternal(&clip);
    }
}

void
QQuickDrawPaintEngine::drawLine(const QLineF &line)
{
    Q_ASSERT(isActive());
    setupQDPort();
    if(d->clip.paintable.isEmpty())
        return;
    setupQDPen();
    MoveTo(qRound(line.x1())+d->offx,qRound(line.y1())+d->offy);
    LineTo(qRound(line.x2())+d->offx,qRound(line.y2())+d->offy);
}

void
QQuickDrawPaintEngine::drawRect(const QRectF &in_r)
{
    Q_ASSERT(isActive());
    setupQDPort();
    if(d->clip.paintable.isEmpty())
        return;

    QRect r = in_r.toRect().intersect(d->polygonClipper.boundingRect()).normalize();

    Rect rect;
    SetRect(&rect, qRound(r.x())+d->offx, qRound(r.y())+d->offy,
            qRound(r.x() + r.width())+d->offx, qRound(r.y() + r.height())+d->offy);
    if(d->current.brush.style() != Qt::NoBrush) {
        setupQDBrush();
        if(d->current.brush.style() == Qt::SolidPattern) {
            PaintRect(&rect);
        } else {
            QPixmap pm;
            if(d->brush_style_pix) {
                pm = *d->brush_style_pix;
                if(d->current.bg.mode == Qt::OpaqueMode) {
                    ::RGBColor f;
                    f.red = d->current.bg.brush.color().red()*256;
                    f.green = d->current.bg.brush.color().green()*256;
                    f.blue = d->current.bg.brush.color().blue()*256;
                    RGBForeColor(&f);
                    PaintRect(&rect);
                }
            } else {
                pm = d->current.brush.texture();
            }
            if(!pm.isNull()) {
                //save the clip
                bool clipon = testf(ClipOn);
                QRegion clip = d->current.clip;

                //create the region
                QRegion newclip(r);
                if(clipon)
                    newclip &= clip;
                setClippedRegionInternal(&newclip);

                //draw the brush
                drawTiledPixmap(r, pm, QPoint(r.x(), r.y()) - d->current.bg.origin, Qt::ComposePixmap);

                //restore the clip
                setClippedRegionInternal(clipon ? &clip : 0);
            }
        }
    }
    if(d->current.pen.style() != Qt::NoPen) {
        setupQDPen();
        FrameRect(&rect);
    }
}

void
QQuickDrawPaintEngine::drawPoint(const QPointF &pt)
{
    Q_ASSERT(isActive());
    if(d->current.pen.style() != Qt::NoPen) {
        setupQDPort();
        if(d->clip.paintable.isEmpty())
            return;
        setupQDPen();
        MoveTo(qRound(pt.x()) + d->offx, qRound(pt.y()) + d->offy);
        Line(0, 0);
    }
}

void
QQuickDrawPaintEngine::drawPoints(const QPointF *points, int pointCount)
{
    Q_ASSERT(isActive());

    if(d->current.pen.style() != Qt::NoPen) {
        setupQDPort();
        if(d->clip.paintable.isEmpty())
            return;
        setupQDPen();
        for(int i=0; i < pointCount; i++) {
            MoveTo(qRound(points[i].x())+d->offx, qRound(points[i].y())+d->offy);
            Line(0, 0);
        }
    }
}

void
QQuickDrawPaintEngine::drawEllipse(const QRectF &in_r)
{
    Q_ASSERT(isActive());

    QRect r = in_r.toRect().intersect(d->polygonClipper.boundingRect()).normalize();

    setupQDPort();
    if(d->clip.paintable.isEmpty())
        return;

    Rect mac_r;
    SetRect(&mac_r, r.x() + d->offx, r.y() + d->offy,
            r.x() + r.width() + d->offx, r.y() + r.height() + d->offy);
    if(d->current.brush.style() != Qt::NoBrush) {
        setupQDBrush();
        if(d->current.brush.style() == Qt::SolidPattern) {
            PaintOval(&mac_r);
        } else {
            QPixmap pm = 0;
            if(d->brush_style_pix) {
                pm = *d->brush_style_pix;
                if(d->current.bg.mode == Qt::OpaqueMode) {
                    ::RGBColor f;
                    f.red = d->current.bg.brush.color().red()*256;
                    f.green = d->current.bg.brush.color().green()*256;
                    f.blue = d->current.bg.brush.color().blue()*256;
                    RGBForeColor(&f);
                    PaintOval(&mac_r);
                }
            } else {
                pm = d->current.brush.texture();
            }
            if(!pm.isNull()) {
                //save the clip
                bool clipon = testf(ClipOn);
                QRegion clip = d->current.clip;

                //create the region
                QRegion newclip(r, QRegion::Ellipse);
                if(clipon)
                    newclip &= clip;
                setClippedRegionInternal(&newclip);

                //draw the brush
                drawTiledPixmap(r, pm, QPointF(r.x(), r.y()) - d->current.bg.origin, Qt::ComposePixmap);

                //restore the clip
                setClippedRegionInternal(clipon ? &clip : 0);
            }
        }
    }

    if(d->current.pen.style() != Qt::NoPen) {
        setupQDPen();
        FrameOval(&mac_r);
    }
}

void
QQuickDrawPaintEngine::drawLines(const QLineF *lines, int lineCount)
{
    Q_ASSERT(isActive());

    setupQDPort();
    if(d->clip.paintable.isEmpty())
        return;

    setupQDPen();
    for(int i = 0; i < lineCount; i++) {
        const QPointF start = lines[i].p1(), end = lines[i].p2();
        MoveTo(qRound(start.x()) + d->offx, qRound(start.y()) + d->offy);
        LineTo(qRound(end.x()) + d->offx, qRound(end.y()) + d->offy);
    }
}

void
QQuickDrawPaintEngine::drawPolygon(const QPointF *points, int pointCount, PolygonDrawMode mode)
{
    Q_ASSERT(isActive());

    // clip and round
    int cCount;
    qt_float_point *cPoints;
    d->polygonClipper.clipPolygon((qt_float_point *)points, pointCount, &cPoints, &cCount);
    QVarLengthArray<QPoint> fixedPoints(cCount);
    for (int i = 0; i < cCount; ++i) {
        fixedPoints[i].rx() = qRound(cPoints[i].x);
        fixedPoints[i].ry() = qRound(cPoints[i].y);
    }

    //do the drawing
    if (mode == PolylineMode) {
        if(pointCount)
            return;
        setupQDPort();
        if(d->clip.paintable.isEmpty())
            return;

        setupQDPen();
        QPoint penPoint(0, 0);
        if(d->current.pen.width() > 0) {
            int dot = d->current.pen.width() / 2;
            penPoint = QPoint(-dot, -dot);
        }

        // We draw 5000 chunks at a time because of limitations in QD
        for(int chunk = 0; chunk < fixedPoints.count();) {
            //make a region of it
            PolyHandle poly = OpenPoly();
            MoveTo(fixedPoints[chunk].x()+d->offx+penPoint.x(), fixedPoints[chunk].y()+d->offy+penPoint.y());
            for(int last_chunk=chunk+5000; chunk < last_chunk; chunk++) {
                if(chunk == pointCount)
                    break;
                LineTo(fixedPoints[chunk].x()+d->offx+penPoint.x(), fixedPoints[chunk].y()+d->offy+penPoint.y());
            }
            ClosePoly();
            //now draw it
            FramePoly(poly);
            KillPoly(poly);
        }
    } else {
        setupQDPort();
        if(d->clip.paintable.isEmpty())
            return;

        PolyHandle polyHandle = OpenPoly();
        MoveTo(fixedPoints[0].x()+d->offx, fixedPoints[0].y()+d->offy);
        for(int x = 1; x < fixedPoints.size(); x++)
            LineTo(fixedPoints[x].x()+d->offx, fixedPoints[x].y()+d->offy);
        LineTo(fixedPoints[0].x()+d->offx, fixedPoints[0].y()+d->offy);
        ClosePoly();

        if(d->current.brush.style() != Qt::NoBrush) {
            setupQDBrush();
            if(d->current.brush.style() == Qt::SolidPattern) {
                PaintPoly(polyHandle);
            } else {
                QPixmap pm = 0;
                if(d->brush_style_pix) {
                    pm = *d->brush_style_pix;
                    if(d->current.bg.mode == Qt::OpaqueMode) {
                        ::RGBColor f;
                        f.red = d->current.bg.brush.color().red()*256;
                        f.green = d->current.bg.brush.color().green()*256;
                        f.blue = d->current.bg.brush.color().blue()*256;
                        RGBForeColor(&f);
                        PaintPoly(polyHandle);
                    }
                } else {
                    pm = d->current.brush.texture();
                }

                if(!pm.isNull()) {
                    //save the clip
                    bool clipon = testf(ClipOn);
                    QRegion clip = d->current.clip;

                    QPolygon pa;
                    pa.reserve(pointCount);
                    for (int i=0; i<pointCount; ++i)
                        pa.append(points[i].toPoint());

                    //create the region
                    QRegion newclip(pa);
                    if(clipon)
                        newclip &= clip;
                    setClippedRegionInternal(&newclip);

                    //draw the brush
                    QRect r(pa.boundingRect());
                    drawTiledPixmap(r, pm, r.topLeft() - d->current.bg.origin, Qt::ComposePixmap);

                    //restore the clip
                    setClippedRegionInternal(clipon ? &clip : 0);
                }
            }
        }
        if(d->current.pen.style() != Qt::NoPen) {
            setupQDPen();
            FramePoly(polyHandle);
        }
        KillPoly(polyHandle);
    }
}

void
QQuickDrawPaintEngine::drawTiledPixmap(const QRectF &r, const QPixmap &pixmap, const QPointF &p,
				       Qt::PixmapDrawingMode mode)
{
    int yPos=qRound(r.y()), xPos, drawH, drawW, yOff=qRound(p.y()), xOff;
    int rBottom = qRound(r.y() + r.height());
    int rRight = qRound(r.x() + r.width());
    while(yPos < rBottom) {
        drawH = pixmap.height() - yOff;    // Cropping first row
        if(yPos + drawH > rBottom)        // Cropping last row
            drawH = rBottom - yPos;
        xPos = qRound(r.x());
        xOff = qRound(p.x());
        while(xPos < rRight) {
            drawW = pixmap.width() - xOff; // Cropping first column
            if(xPos + drawW > rRight)    // Cropping last column
                drawW = rRight - xPos;
            drawPixmap(QRect(xPos, yPos, drawW, drawH), pixmap, QRect(xOff, yOff, drawW, drawH),
                       mode);
            xPos += drawW;
            xOff = 0;
        }
        yPos += drawH;
        yOff = 0;
    }
}

void
QQuickDrawPaintEngine::drawPixmap(const QRectF &r, const QPixmap &pm, const QRectF &sr,
                                  Qt::PixmapDrawingMode mode)
{
    Q_ASSERT(isActive());
    if(pm.isNull())
        return;

    setupQDPort();
    if(d->clip.paintable.isEmpty())
        return;

    //setup port
    ::RGBColor f;
    if(pm.depth() == 1) {
        f.red = d->current.pen.color().red()*256;
        f.green = d->current.pen.color().green()*256;
        f.blue = d->current.pen.color().blue()*256;
    } else {
        f.red = f.green = f.blue = 0;
    }
    RGBForeColor(&f);
    f.red = f.green = f.blue = ~0;
    RGBBackColor(&f);

    //get pixmap bits
    const BitMap *srcbitmap = GetPortBitMapForCopyBits(qt_macQDHandle(&pm));
    const QPixmap *srcmask=0;
    if(mode == Qt::ComposePixmap) {
        if(pm.data->alphapm)
            srcmask = pm.data->alphapm;
        else
            srcmask = pm.mask();
    } else if(mode == Qt::CopyPixmap) {
        if(d->pdev->devType() == QInternal::Pixmap) {
            QPixmap *dst = static_cast<QPixmap*>(d->pdev);
            if(pm.mask() && !pm.isQBitmap()) {
                QBitmap bm(dst->size(), true);
                QPainter p(&bm);
                if(dst->mask() && r != QRectF(0, 0, dst->width(), dst->height()))
                    p.drawPixmap(0, 0, *dst->mask(), Qt::CopyPixmap);
                p.drawPixmap(r, *pm.mask(), sr, Qt::CopyPixmap);
                dst->setMask(bm);
            }
            if(pm.data->alphapm) {
                if(!dst->data->alphapm) {
                    dst->data->alphapm = new QPixmap(dst->size());
                    dst->data->alphapm->fill(qRgba(255, 255, 255, 0));
                }
                QPainter p(dst->data->alphapm);
                p.drawPixmap(r, *pm.data->alphapm, sr, Qt::CopyPixmap);
            }
        }
    }

    //get pdev bits
    const BitMap *dstbitmap=0;
    switch(d->pdev->devType()) {
    case QInternal::Widget: {
        QWidget *w = static_cast<QWidget*>(d->pdev);
        dstbitmap = GetPortBitMapForCopyBits(GetWindowPort(qt_mac_window_for(w)));
        break; }
    case QInternal::Printer:
    case QInternal::Pixmap: {
        dstbitmap = GetPortBitMapForCopyBits(qt_macQDHandle(d->pdev));
        break; }
    }

    //get copy mode
    short copymode = srcCopy;
    if(srcmask && srcmask->depth() > 1)
        copymode = ditherCopy;

    //do the blt
    Rect srcr;
    SetRect(&srcr, qRound(sr.x()), qRound(sr.y()),
            qRound(sr.x() + sr.width()), qRound(sr.y()+sr.height()));
    Rect dstr;
    SetRect(&dstr, d->offx + qRound(r.x()), d->offy + qRound(r.y()),
            d->offx + qRound(r.x() + r.width()),
            d->offy + qRound(r.y() + r.height()));
    if(srcmask) {
        const BitMap *maskbits = GetPortBitMapForCopyBits(qt_macQDHandle(srcmask));
        if(d->pdev->devType() == QInternal::Printer) { //can't use CopyDeepMask on a printer
            QPixmap tmppix(qRound(r.width()), qRound(r.height()), pm.depth());
            Rect pixr;
            SetRect(&pixr, 0, 0, qRound(r.width()), qRound(r.height()));
            const BitMap *pixbits = GetPortBitMapForCopyBits((GWorldPtr)tmppix.handle());
            {
                QMacSavedPortInfo pi(&tmppix);
                EraseRect(&pixr);
                CopyDeepMask(srcbitmap, maskbits, pixbits, &srcr, &srcr, &pixr, copymode, 0);
            }
            setupQDPort(true);
            CopyBits(pixbits, dstbitmap, &pixr, &dstr, srcOr, 0); //use srcOr transfer, to "emulate" the mask
        } else {
            CopyDeepMask(srcbitmap, maskbits, dstbitmap, &srcr, &srcr, &dstr, copymode, 0);
        }
    } else {
        CopyBits(srcbitmap, dstbitmap, &srcr, &dstr, copymode, 0);
    }
}

void
QQuickDrawPaintEngine::initialize()
{
}

void
QQuickDrawPaintEngine::cleanup()
{
}

/*!
    \internal
*/
void
QQuickDrawPaintEngine::setupQDPen()
{
    //pen size
    int dot = d->current.pen.width();
    if(dot < 1)
        dot = 1;
    PenSize(dot, dot);

    //forecolor
    ::RGBColor f;
    f.red = d->current.pen.color().red()*256;
    f.green = d->current.pen.color().green()*256;
    f.blue = d->current.pen.color().blue()*256;
    Pattern pat;
    GetQDGlobalsBlack(&pat);
    PenPat(&pat);
    RGBForeColor(&f);

    //backcolor
    ::RGBColor b;
    b.red = d->current.bg.brush.color().red()*256;
    b.green = d->current.bg.brush.color().green()*256;
    b.blue = d->current.bg.brush.color().blue()*256;
    RGBBackColor(&b);

    //penmodes
    PenMode(patCopy);
}

/*!
    \internal
*/
void
QQuickDrawPaintEngine::setupQDBrush()
{
    //pattern
    delete d->brush_style_pix;
    d->brush_style_pix = 0;
    int bs = d->current.brush.style();
    if(bs >= Qt::Dense1Pattern && bs <= Qt::DiagCrossPattern) {
        d->brush_style_pix = new QPixmap(8, 8);
        d->brush_style_pix->setMask(qt_pixmapForBrush(bs, true));
        d->brush_style_pix->fill(d->current.brush.color());
    } else if(bs == Qt::TexturePattern) {
        QPixmap texture = d->current.brush.texture();
        if(texture.isQBitmap()) {
            d->brush_style_pix = new QPixmap(texture.width(), texture.height());
            d->brush_style_pix->setMask(*((QBitmap*)&texture));
            d->brush_style_pix->fill(d->current.brush.color());
        }
    }

    //forecolor
    ::RGBColor f;
    f.red = d->current.brush.color().red()*256;
    f.green = d->current.brush.color().green()*256;
    f.blue = d->current.brush.color().blue()*256;
    Pattern pat;
    GetQDGlobalsBlack(&pat);
    PenPat(&pat);
    RGBForeColor(&f);

    //backcolor
    ::RGBColor b;
    b.red = d->current.bg.brush.color().red()*256;
    b.green = d->current.bg.brush.color().green()*256;
    b.blue = d->current.bg.brush.color().blue()*256;
    RGBBackColor(&b);

    //penmodes
    PenMode(patCopy);
}

/*!
    \internal
*/
void
QQuickDrawPaintEngine::setupQDFont()
{
    setupQDPen();
}

/*!
    \internal
*/
void QQuickDrawPaintEngine::setupQDPort(bool force, QPoint *off, QRegion *rgn)
{
    bool remade_clip = false;
    if(d->pdev->devType() == QInternal::Printer) {
        if(force) {
            remade_clip = true;
            d->clip.pdev = QRegion(0, 0, d->pdev->width(), d->pdev->height());
        }
    } else if(d->pdev->devType() == QInternal::Widget) {                    // device is a widget
        QWidget *w = (QWidget*)d->pdev;
        if(!(remade_clip = force)) {
            if(!w->isVisible())
                remade_clip = d->clip.serial;
            else
                remade_clip = (d->clip.serial != w->d_func()->clippedSerial(!d->unclipped));
        }
        if(remade_clip) {
            //offset painting in widget relative the tld
            QPoint wp = posInWindow(w);
            d->offx = wp.x();
            d->offy = wp.y();

            if(!w->isVisible()) {
                d->clip.pdev = QRegion(0, 0, 0, 0); //make the clipped reg empty if not visible!!!
                d->clip.serial = 0;
            } else {
                d->clip.pdev = w->d_func()->clippedRegion(!d->unclipped);
                d->clip.serial = w->d_func()->clippedSerial(!d->unclipped);
            }
            QRegion sysClip = systemClip();
            if(!sysClip.isEmpty()) {
                sysClip.translate(wp.x(), wp.y());
                d->clip.pdev &= sysClip;
            }
        }
    } else if(d->pdev->devType() == QInternal::Pixmap) {             // device is a pixmap
        QPixmap *pm = (QPixmap*)d->pdev;
        if(force) {//clip out my bounding rect
            remade_clip = true;
            d->clip.pdev = QRegion(0, 0, pm->width(), pm->height());
        }
    }
    if(remade_clip || d->clip.dirty) {         //update clipped region
        remade_clip = true;
        if(!d->clip.pdev.isEmpty() && testf(ClipOn)) {
            d->clip.paintable = d->current.clip;
            d->clip.paintable.translate(d->offx, d->offy);
            d->clip.paintable &= d->clip.pdev;
        } else {
            d->clip.paintable = d->clip.pdev;
        }

        CGrafPtr ptr = qt_macQDHandle(d->pdev);
        if(RgnHandle rgn = d->clip.paintable.handle()) {
            QDAddRegionToDirtyRegion(ptr, rgn);
        } else {
            QRect qr = d->clip.paintable.boundingRect();
            Rect mr; SetRect(&mr, qr.x(), qr.y(), qr.right(), qr.bottom());
            QDAddRectToDirtyRegion(ptr, &mr);
        }
        d->clip.dirty = false;
    }
    { //setup the port
        QMacSavedPortInfo::setPaintDevice(d->pdev);
        if(type() != CoreGraphics)
            QMacSavedPortInfo::setClipRegion(d->clip.paintable);
    }
    if(off)
        *off = QPoint(d->offx, d->offy);
    if(rgn)
        *rgn = d->clip.paintable;
}

/*****************************************************************************
  QCoreGraphicsPaintEngine utility functions
 *****************************************************************************/

//colour conversion
inline static float qt_mac_convert_color_to_cg(int c) { return ((float)c * 1000 / 255) / 1000; }

//pattern handling (tiling)
struct QMacPattern {
    QMacPattern() : opaque(true), as_mask(false), image(0) { data.bytes = 0; }
    //input
    QColor background, foreground;
    bool opaque;
    bool as_mask;
    struct {
        QPixmap pixmap;
        const uchar *bytes;
    } data;
    //output
    CGImageRef image;
};
static void qt_mac_draw_pattern(void *info, CGContextRef c)
{
    QMacPattern *pat = (QMacPattern*)info;
    int w = 0, h = 0;
    if (!pat->image) {
        CGImageRef image = 0;
        if (pat->as_mask) {
            w = h = 8;
            CGDataProviderRef provider = CGDataProviderCreateWithData(0, pat->data.bytes, 64, 0);
            image = CGImageMaskCreate(w, h, 1, 1, 1, provider, 0, false);
            CGDataProviderRelease(provider);
        } else {
            w = pat->data.pixmap.width();
            h = pat->data.pixmap.height();
            image = qt_mac_create_cgimage(pat->data.pixmap, Qt::ComposePixmap,
                                          pat->data.pixmap.isQBitmap());
        }
        if(pat->opaque && CGImageIsMask(image)) {
            QPixmap tmp(w, h);
            CGRect rect = CGRectMake(0, 0, w, h);
            CGContextRef ctx = qt_macCreateCGHandle(&tmp);
            CGContextSetRGBFillColor(ctx, qt_mac_convert_color_to_cg(pat->background.red()),
                                     qt_mac_convert_color_to_cg(pat->background.green()),
                                     qt_mac_convert_color_to_cg(pat->background.blue()),
                                     qt_mac_convert_color_to_cg(pat->background.alpha()));
            CGContextFillRect(ctx, rect);
            CGContextSetRGBFillColor(ctx, qt_mac_convert_color_to_cg(pat->foreground.red()),
                                     qt_mac_convert_color_to_cg(pat->foreground.green()),
                                     qt_mac_convert_color_to_cg(pat->foreground.blue()),
                                     qt_mac_convert_color_to_cg(pat->foreground.alpha()));
            HIViewDrawCGImage(ctx, &rect, image);
            pat->image = qt_mac_create_cgimage(tmp, Qt::CopyPixmap, false);
            CGImageRelease(image);
        } else {
            pat->image = image;
        }
    } else {
        w = CGImageGetWidth(pat->image);
        h = CGImageGetHeight(pat->image);
    }
    CGRect rect = CGRectMake(0, 0, w, h);
    HIViewDrawCGImage(c, &rect, pat->image); //top left
}
static void qt_mac_dispose_pattern(void *info)
{
    QMacPattern *pat = (QMacPattern*)info;
    if(pat->image)
        CGImageRelease(pat->image);
    delete pat;
}

//gradiant callback
static void qt_mac_color_gradient_function(void *info, const float *in, float *out)
{
    QBrush *brush = static_cast<QBrush *>(info);
    QGradientStops stops = brush->gradient()->stops();
    QColor c1 = stops.first().second;
    QColor c2 = stops.last().second;
    const float red = qt_mac_convert_color_to_cg(c1.red());
    out[0] = red + in[0] * (qt_mac_convert_color_to_cg(c2.red())-red);
    const float green = qt_mac_convert_color_to_cg(c1.green());
    out[1] = green + in[0] * (qt_mac_convert_color_to_cg(c2.green())-green);
    const float blue = qt_mac_convert_color_to_cg(c1.blue());
    out[2] = blue + in[0] * (qt_mac_convert_color_to_cg(c2.blue())-blue);
    const float alpha = qt_mac_convert_color_to_cg(c1.alpha());
    out[3] = alpha + in[0] * (qt_mac_convert_color_to_cg(c2.alpha()) - alpha);
}

//clipping handling
static void qt_mac_clip_cg_reset(CGContextRef hd)
{
    //setup xforms
    CGAffineTransform old_xform = CGContextGetCTM(hd);
    CGContextConcatCTM(hd, CGAffineTransformInvert(old_xform));
    CGContextConcatCTM(hd, CGAffineTransformIdentity);

    //do the clip reset
    QRect qrect = QRect(0, 0, 99999, 999999);
    Rect qdr; SetRect(&qdr, qrect.left(), qrect.top(), qrect.right(),
                      qrect.bottom());
    ClipCGContextToRegion(hd, &qdr, QRegion(qrect).handle(true));

    //reset xforms
    CGContextConcatCTM(hd, CGAffineTransformInvert(CGContextGetCTM(hd)));
    CGContextConcatCTM(hd, old_xform);
}

static CGRect qt_mac_compose_rect(const QRectF &r, float off=0)
{
    return CGRectMake(r.x()+off, r.y()+off, r.width(), r.height());
}

static CGMutablePathRef qt_mac_compose_path(const QPainterPath &p, float off=0)
{
    CGMutablePathRef ret = CGPathCreateMutable();
    QPointF startPt;
    for (int i=0; i<p.elementCount(); ++i) {
        const QPainterPath::Element &elm = p.elementAt(i);
        switch (elm.type) {
        case QPainterPath::MoveToElement:
            if (i > 0
                && p.elementAt(i - 1).x == startPt.x()
                && p.elementAt(i - 1).y == startPt.y())
                CGPathCloseSubpath(ret);
            startPt = QPointF(elm.x, elm.y);
            CGPathMoveToPoint(ret, 0, elm.x+off, elm.y+off);
            break;
        case QPainterPath::LineToElement:
            CGPathAddLineToPoint(ret, 0, elm.x+off, elm.y+off);
            break;
        case QPainterPath::CurveToElement:
            Q_ASSERT(p.elementAt(i+1).type == QPainterPath::CurveToDataElement);
            Q_ASSERT(p.elementAt(i+2).type == QPainterPath::CurveToDataElement);
            CGPathAddCurveToPoint(ret, 0,
                                  elm.x+off, elm.y+off,
                                  p.elementAt(i+1).x+off, p.elementAt(i+1).y+off,
                                  p.elementAt(i+2).x+off, p.elementAt(i+2).y+off);
            i+=2;
            break;
        default:
            qFatal("QCoreGraphicsPaintEngine::drawPath(), unhandled type: %d", elm.type);
            break;
        }
    }
    if (!p.isEmpty()
        && p.elementAt(p.elementCount() - 1).x == startPt.x()
        && p.elementAt(p.elementCount() - 1).y == startPt.y())
        CGPathCloseSubpath(ret);
    return ret;
}

static void qt_mac_clip_cg(CGContextRef hd, const QRegion &rgn, const QPoint *pt, CGAffineTransform *orig_xform)
{
    CGAffineTransform old_xform = CGAffineTransformIdentity;
    if(orig_xform) { //setup xforms
        old_xform = CGContextGetCTM(hd);
        CGContextConcatCTM(hd, CGAffineTransformInvert(old_xform));
        CGContextConcatCTM(hd, *orig_xform);
    }

    //do the clipping
    CGContextBeginPath(hd);
    if(rgn.isEmpty()) {
        CGContextAddRect(hd, CGRectMake(0, 0, 0, 0));
    } else {
        QVector<QRect> rects = rgn.rects();
        const int count = rects.size();
        for(int i = 0; i < count; i++) {
            const QRect &r = rects[i];
            CGRect mac_r = CGRectMake(r.x(), r.y(), r.width(), r.height());
            if(pt) {
                mac_r.origin.x -= pt->x();
                mac_r.origin.y -= pt->y();
            }
            CGContextAddRect(hd, mac_r);
        }
    }
    CGContextClip(hd);

    if(orig_xform) {//reset xforms
        CGContextConcatCTM(hd, CGAffineTransformInvert(CGContextGetCTM(hd)));
        CGContextConcatCTM(hd, old_xform);
    }
}

/*****************************************************************************
  QCoreGraphicsPaintEngine member functions
 *****************************************************************************/

inline static QPaintEngine::PaintEngineFeatures qt_mac_cg_features()
{
    return QPaintEngine::PaintEngineFeatures(
        QPaintEngine::CoordTransform|QPaintEngine::PixmapTransform|
        QPaintEngine::PatternTransform|QPaintEngine::PenWidthTransform|
        QPaintEngine::PainterPaths|QPaintEngine::PixmapScale|
        QPaintEngine::UsesFontEngine|QPaintEngine::LinearGradientFill|
        QPaintEngine::ClipTransform|QPaintEngine::AlphaStroke|
        QPaintEngine::AlphaFill|QPaintEngine::AlphaPixmap|
        QPaintEngine::FillAntialiasing|QPaintEngine::LineAntialiasing
        );
}

QCoreGraphicsPaintEngine::QCoreGraphicsPaintEngine()
    : QQuickDrawPaintEngine(*(new QCoreGraphicsPaintEnginePrivate), qt_mac_cg_features())
{
}

QCoreGraphicsPaintEngine::QCoreGraphicsPaintEngine(QPaintEnginePrivate &dptr)
    : QQuickDrawPaintEngine(dptr, qt_mac_cg_features())
{
}

QCoreGraphicsPaintEngine::~QCoreGraphicsPaintEngine()
{
}

bool
QCoreGraphicsPaintEngine::begin(QPaintDevice *pdev)
{
    if(isActive()) {                         // already active painting
        qWarning("QCoreGraphicsPaintEngine::begin: Painter is already active."
                  "\n\tYou must end() the painter before a second begin()");
        return false;
    }

    //initialization
    d->offx = d->offy = 0; // (quickdraw compat!!)
    d->pdev = pdev;
    d->hd = qt_macCreateCGHandle(pdev);
    if(d->hd) {
        d->orig_xform = CGContextGetCTM(d->hd);
        if(d->shading) {
            CGShadingRelease(d->shading);
            d->shading = 0;
        }
        d->setClip(0);  //clear the context's clipping
    }

    setActive(true);
    assignf(IsActive | DirtyFont);

    if(d->pdev->devType() == QInternal::Widget) {                    // device is a widget
        QWidget *w = (QWidget*)d->pdev;
        { //offset painting in widget relative the tld (quickdraw compat!!!)
            QPoint wp = posInWindow(w);
            d->offx = wp.x();
            d->offy = wp.y();
        }
	bool unclipped = w->testAttribute(Qt::WA_PaintUnclipped);

        if((w->windowType() == Qt::Desktop)) {
            if(!unclipped)
                qWarning("QCoreGraphicsPaintEngine::begin: Does not support clipped desktop on Mac OSX");
            ShowWindow(qt_mac_window_for(w));
        } else if(unclipped) {
            qWarning("QCoreGraphicsPaintEngine::begin: Does not support unclipped painting");
        }
    } else if(d->pdev->devType() == QInternal::Pixmap) {             // device is a pixmap
        QPixmap *pm = (QPixmap*)d->pdev;
        if(pm->depth() == 1) {
            setRenderHint(QPainter::Antialiasing, false);
            setRenderHint(QPainter::TextAntialiasing, false);
        }

        if(pm->isNull()) {
            qWarning("QCoreGraphicsPaintEngine::begin: Cannot paint null pixmap");
            end();
            return false;
        }
    }

    setDirty(QPaintEngine::DirtyPen);
    setDirty(QPaintEngine::DirtyBrush);
    setDirty(QPaintEngine::DirtyBackground);
    setDirty(QPaintEngine::DirtyHints);
    return true;
}

bool
QCoreGraphicsPaintEngine::end()
{
    setActive(false);
    if(d->pdev->devType() == QInternal::Widget && static_cast<QWidget*>(d->pdev)->windowType() == Qt::Desktop)
        HideWindow(qt_mac_window_for(static_cast<QWidget*>(d->pdev)));
    if(d->shading) {
        CGShadingRelease(d->shading);
        d->shading = 0;
    }
    d->pdev = 0;
    if(d->hd) {
        CGContextSynchronize(d->hd);
        CGContextRelease(d->hd);
        d->hd = 0;
    }
    return true;
}

void
QCoreGraphicsPaintEngine::updatePen(const QPen &pen)
{
    Q_ASSERT(isActive());
    d->current.pen = pen;

    //pen style
    float *lengths = 0;
    int count = 0;
    if(pen.style() == Qt::DashLine) {
        static float inner_lengths[] = { 3, 1 };
        lengths = inner_lengths;
        count = 2;
    } else if(pen.style() == Qt::DotLine) {
        static float inner_lengths[] = { 1, 1 };
        lengths = inner_lengths;
        count = 2;
    } else if(pen.style() == Qt::DashDotLine) {
        static float inner_lengths[] = { 3, 1, 1, 1 };
        lengths = inner_lengths;
        count = 4;
    } else if(pen.style() == Qt::DashDotDotLine) {
        static float inner_lengths[] = { 3, 1, 1, 1, 1, 1 };
        lengths = inner_lengths;
        count = 6;
    }
    CGContextSetLineDash(d->hd, 0, lengths, count);

    //pencap
    CGLineCap cglinecap = kCGLineCapButt;
    if(pen.capStyle() == Qt::SquareCap)
        cglinecap = kCGLineCapSquare;
    else if(pen.capStyle() == Qt::RoundCap)
        cglinecap = kCGLineCapRound;
    CGContextSetLineCap(d->hd, cglinecap);

    //penwidth
    CGContextSetLineWidth(d->hd, pen.width() <= 0 ? 1 : pen.width());

    //join
    CGLineJoin cglinejoin = kCGLineJoinMiter;
    if(pen.joinStyle() == Qt::BevelJoin)
        cglinejoin = kCGLineJoinBevel;
    else if(pen.joinStyle() == Qt::RoundJoin)
        cglinejoin = kCGLineJoinRound;
    CGContextSetLineJoin(d->hd, cglinejoin);

    //color
    const QColor &col = pen.color();
    CGContextSetRGBStrokeColor(d->hd, qt_mac_convert_color_to_cg(col.red()),
                               qt_mac_convert_color_to_cg(col.green()),
                               qt_mac_convert_color_to_cg(col.blue()),
                               qt_mac_convert_color_to_cg(col.alpha()));
}

void
QCoreGraphicsPaintEngine::updateBrush(const QBrush &brush, const QPointF &brushOrigin)
{
    Q_ASSERT(isActive());
    d->current.brush = brush;
    d->current.bg.origin = brushOrigin;

    if(d->shading) {
        CGShadingRelease(d->shading);
        d->shading = 0;
    }

    //pattern
    Qt::BrushStyle bs = brush.style();
    if(bs == Qt::LinearGradientPattern) {
        CGFunctionCallbacks callbacks = { 0, qt_mac_color_gradient_function, 0 };
        CGFunctionRef fill_func = CGFunctionCreate(const_cast<void *>(reinterpret_cast<const void *>(&brush)), 1, 0, 4, 0, &callbacks);
        CGColorSpaceRef grad_colorspace = CGColorSpaceCreateDeviceRGB();
        const QLinearGradient *linGrad = static_cast<const QLinearGradient*>(brush.gradient());
        const QPointF start = linGrad->start(), stop = linGrad->finalStop();
        d->shading = CGShadingCreateAxial(grad_colorspace, CGPointMake(start.x(), start.y()),
                                          CGPointMake(stop.x(), stop.y()), fill_func, true, true);
        CGFunctionRelease(fill_func);
        CGColorSpaceRelease(grad_colorspace);
    } else if(bs != Qt::SolidPattern && bs != Qt::NoBrush) {
        int width = 0, height = 0;
        QMacPattern *qpattern = new QMacPattern;
        float components[4] = { 1.0, 1.0, 1.0, 1.0 };
        CGColorSpaceRef base_colorspace = 0;
        if (bs == Qt::TexturePattern) {
            qpattern->data.pixmap = brush.texture();
            if(qpattern->data.pixmap.isQBitmap()) {
                const QColor &col = brush.color();
                components[0] = qt_mac_convert_color_to_cg(col.red());
                components[1] = qt_mac_convert_color_to_cg(col.green());
                components[2] = qt_mac_convert_color_to_cg(col.blue());
                base_colorspace = CGColorSpaceCreateDeviceRGB();
            }
            width = qpattern->data.pixmap.width();
            height = qpattern->data.pixmap.height();
        } else {
            qpattern->as_mask = true;
            qpattern->data.bytes = qt_patternForBrush(bs, false);
            width = height = 8;
            const QColor &col = brush.color();
            components[0] = qt_mac_convert_color_to_cg(col.red());
            components[1] = qt_mac_convert_color_to_cg(col.green());
            components[2] = qt_mac_convert_color_to_cg(col.blue());
            base_colorspace = CGColorSpaceCreateDeviceRGB();
        }
        qpattern->opaque = (d->current.bg.mode == Qt::OpaqueMode);
        qpattern->foreground = brush.color();
        qpattern->background = d->current.bg.brush.color();

        CGColorSpaceRef fill_colorspace = CGColorSpaceCreatePattern(base_colorspace);
        CGContextSetFillColorSpace(d->hd, fill_colorspace);

        CGPatternCallbacks callbks;
        callbks.version = 0;
        callbks.drawPattern = qt_mac_draw_pattern;
        callbks.releaseInfo = qt_mac_dispose_pattern;
        CGPatternRef fill_pattern = CGPatternCreate(qpattern, CGRectMake(0, 0, width, height),
                                                    CGContextGetCTM(d->hd), width, height,
                                                    kCGPatternTilingNoDistortion, !base_colorspace,
                                                    &callbks);
        CGContextSetFillPattern(d->hd, fill_pattern, components);

        CGPatternRelease(fill_pattern);
        CGColorSpaceRelease(fill_colorspace);
        if(base_colorspace)
            CGColorSpaceRelease(base_colorspace);
    } else if(bs != Qt::NoBrush) {
        const QColor &col = brush.color();
        CGContextSetRGBFillColor(d->hd, qt_mac_convert_color_to_cg(col.red()),
                                 qt_mac_convert_color_to_cg(col.green()),
                                 qt_mac_convert_color_to_cg(col.blue()),
                                 qt_mac_convert_color_to_cg(col.alpha()));
    }
}

void
QCoreGraphicsPaintEngine::updateFont(const QFont &)
{
    Q_ASSERT(isActive());
    clearf(DirtyFont);
    updatePen(d->current.pen);
}

void
QCoreGraphicsPaintEngine::updateBackground(Qt::BGMode mode, const QBrush &brush)
{
    Q_ASSERT(isActive());
    d->current.bg.mode = mode;
    d->current.bg.brush = brush;
}

void
QCoreGraphicsPaintEngine::updateMatrix(const QMatrix &matrix)
{
    Q_ASSERT(isActive());
    d->setTransform(matrix.isIdentity() ? 0 : &matrix);
}

void
QCoreGraphicsPaintEngine::updateClipPath(const QPainterPath &p, Qt::ClipOperation op)
{
    Q_ASSERT(isActive());
    if(op == Qt::NoClip) {
        clearf(ClipOn);
        d->current.clip = QRegion();
        d->setClip(0);
    } else {
        if(!testf(ClipOn))
            op = Qt::ReplaceClip;
        setf(ClipOn);
        QRegion clipRegion(p.toFillPolygon().toPolygon(), p.fillRule());
        if(op == Qt::ReplaceClip) {
            d->current.clip = clipRegion;
            d->setClip(0);
        } else if(op == Qt::IntersectClip) {
            d->current.clip = d->current.clip.intersect(clipRegion);
        }
        if(op == Qt::UniteClip) {
            d->current.clip = d->current.clip.unite(clipRegion);
            d->setClip(&d->current.clip);
        } else {
            CGMutablePathRef path = qt_mac_compose_path(p);
            CGContextBeginPath(d->hd);
            CGContextAddPath(d->hd, path);
            CGContextClip(d->hd);
            CGPathRelease(path);
        }
    }
}

void
QCoreGraphicsPaintEngine::updateClipRegion(const QRegion &clipRegion, Qt::ClipOperation op)
{
    Q_ASSERT(isActive());
    if(op == Qt::NoClip) {
        clearf(ClipOn);
        d->current.clip = QRegion();
        d->setClip(0);
    } else {
        if(!testf(ClipOn))
            op = Qt::ReplaceClip;
        setf(ClipOn);
        if(op == Qt::IntersectClip)
            d->current.clip = d->current.clip.intersect(clipRegion);
        else if(op == Qt::ReplaceClip)
            d->current.clip = clipRegion;
        else if(op == Qt::UniteClip)
            d->current.clip = d->current.clip.unite(clipRegion);
        d->setClip(&d->current.clip);
    }
}

void
QCoreGraphicsPaintEngine::drawLine(const QLineF &line)
{
    Q_ASSERT(isActive());

    CGContextBeginPath(d->hd);
    CGContextMoveToPoint(d->hd, line.x1(), line.y1()+1);
    CGContextAddLineToPoint(d->hd, line.x2(), line.y2()+1);
    d->drawPath(QCoreGraphicsPaintEnginePrivate::CGStroke);
}

void
QCoreGraphicsPaintEngine::drawPath(const QPainterPath &p)
{
    CGMutablePathRef path = qt_mac_compose_path(p, d->penOffset() ? .5 : 0);
    uchar ops = QCoreGraphicsPaintEnginePrivate::CGStroke;
    if(p.fillRule() == Qt::WindingFill)
        ops |= QCoreGraphicsPaintEnginePrivate::CGFill;
    else
        ops |= QCoreGraphicsPaintEnginePrivate::CGEOFill;
    CGContextBeginPath(d->hd);
    d->drawPath(ops, path);
    CGPathRelease(path);
}

void
QCoreGraphicsPaintEngine::drawRect(const QRectF &r)
{
    Q_ASSERT(isActive());

    CGMutablePathRef path = 0;
    if(d->current.brush.style() == Qt::LinearGradientPattern) {
        path = CGPathCreateMutable();
        CGPathAddRect(path, 0, qt_mac_compose_rect(r, d->penOffset()));
    } else {
        CGContextBeginPath(d->hd);
        CGContextAddRect(d->hd, qt_mac_compose_rect(r, d->penOffset()));
    }
    d->drawPath(QCoreGraphicsPaintEnginePrivate::CGFill|QCoreGraphicsPaintEnginePrivate::CGStroke,
                path);
    if(path)
        CGPathRelease(path);
}

void
QCoreGraphicsPaintEngine::drawPoint(const QPointF &p)
{
    Q_ASSERT(isActive());

    CGContextBeginPath(d->hd);
    CGContextMoveToPoint(d->hd, p.x(), p.y()+1);
    CGContextAddLineToPoint(d->hd, p.x(), p.y()+1);
    d->drawPath(QCoreGraphicsPaintEnginePrivate::CGStroke);
}

void
QCoreGraphicsPaintEngine::drawPoints(const QPointF *points, int pointCount)
{
    Q_ASSERT(isActive());

    CGContextBeginPath(d->hd);
    for(int i=0; i < pointCount; i++) {
        float x = points[i].x(), y = points[i].y();
        CGContextMoveToPoint(d->hd, x, y+1);
        CGContextAddLineToPoint(d->hd, x, y+1);
    }
    d->drawPath(QCoreGraphicsPaintEnginePrivate::CGStroke);
}

void
QCoreGraphicsPaintEngine::drawEllipse(const QRectF &r)
{
    Q_ASSERT(isActive());

    //setup a clip
    CGContextSaveGState(d->hd);
    CGContextBeginPath(d->hd);
    CGContextAddRect(d->hd, qt_mac_compose_rect(QRectF(r.x(), r.y(), r.width()+1, r.height()+1)));
    CGContextClip(d->hd);

    CGMutablePathRef path = CGPathCreateMutable();
    CGAffineTransform transform = CGAffineTransformMakeScale(r.width() / r.height(), 1);
    CGPathAddArc(path, &transform,((r.x()+d->penOffset()) + (r.width() / 2)) / (r.width() / r.height()),
                 (r.y()+d->penOffset()) + (r.height() / 2), r.height() / 2, 0, (2 * M_PI), false);
    d->drawPath(QCoreGraphicsPaintEnginePrivate::CGFill | QCoreGraphicsPaintEnginePrivate::CGStroke,
                path);
    CGPathRelease(path);

    //restore
    CGContextRestoreGState(d->hd);
}

void
QCoreGraphicsPaintEngine::drawPolygon(const QPointF *points, int pointCount, PolygonDrawMode mode)
{
    Q_ASSERT(isActive());

    if (mode == PolylineMode) {
        CGContextMoveToPoint(d->hd, points[0].x(), points[0].y()+1);
        for(int x = 1; x < pointCount; ++x)
            CGContextAddLineToPoint(d->hd, points[x].x(), points[x].y()+1);
        d->drawPath(QCoreGraphicsPaintEnginePrivate::CGStroke);
    } else {
        CGMutablePathRef path = CGPathCreateMutable();
        CGPathMoveToPoint(path, 0, points[0].x(), points[0].y()+1);
        for(int x = 1; x < pointCount; ++x)
            CGPathAddLineToPoint(path, 0, points[x].x(), points[x].y()+1);
        if (points[0] != points[pointCount-1])
            CGPathAddLineToPoint(path, 0, points[0].x(), points[0].y()+1);
        CGContextBeginPath(d->hd);
        d->drawPath(QCoreGraphicsPaintEnginePrivate::CGFill
                    | QCoreGraphicsPaintEnginePrivate::CGStroke, path);
    }
}

void
QCoreGraphicsPaintEngine::drawLines(const QLineF *lines, int lineCount)
{
    Q_ASSERT(isActive());

    CGContextBeginPath(d->hd);
    for(int i = 0; i < lineCount; i++) {
        const QPointF start = lines[i].p1(), end = lines[i].p2();
        CGContextMoveToPoint(d->hd, start.x(), start.y()+1);
        CGContextAddLineToPoint(d->hd, end.x(), end.y()+1);
    }
    d->drawPath(QCoreGraphicsPaintEnginePrivate::CGStroke);
}

void
QCoreGraphicsPaintEngine::drawPixmap(const QRectF &r, const QPixmap &pm, const QRectF &sr,
                                     Qt::PixmapDrawingMode mode)
{
    Q_ASSERT(isActive());
    if(pm.isNull())
        return;

    //save
    CGContextSaveGState(d->hd);

    //setup
    bool asMask = pm.isQBitmap() || pm.depth() == 1;
    if(asMask) {     //set colour
        if(d->pdev->devType() == QInternal::Pixmap && static_cast<QPixmap*>(d->pdev)->isQBitmap()) {
            asMask = false; //if the destination is a bitmap no need for the colour tricks --Sam
        } else {
            const QColor &col = d->current.pen.color();
            CGContextSetRGBFillColor(d->hd, qt_mac_convert_color_to_cg(col.red()),
                                     qt_mac_convert_color_to_cg(col.green()),
                                     qt_mac_convert_color_to_cg(col.blue()),
                                     qt_mac_convert_color_to_cg(col.alpha()));
        }
    }
    //set clip
    QRegion rgn(r.toRect());
    qt_mac_clip_cg(d->hd, rgn, 0, 0);

    //draw
    if(mode == Qt::CopyPixmap) {
        if(d->pdev->devType() == QInternal::Pixmap) {
            QPixmap *dst = static_cast<QPixmap*>(d->pdev);
            if(pm.mask() && !pm.isQBitmap()) {
                QBitmap bm(dst->size(), true);
                QPainter p(&bm);
                if(dst->mask() && r != QRectF(0, 0, dst->width(), dst->height()))
                    p.drawPixmap(0, 0, *dst->mask(), Qt::CopyPixmap);
                p.drawPixmap(r, *pm.mask(), sr, Qt::CopyPixmap);
                dst->setMask(bm);
            }
            if(pm.data->alphapm) {
                if(!dst->data->alphapm) {
                    dst->data->alphapm = new QPixmap(dst->size());
                    dst->data->alphapm->fill(qRgba(255, 255, 255, 0));
                }
                QPainter p(dst->data->alphapm);
                p.drawPixmap(r, *pm.data->alphapm, sr, Qt::CopyPixmap);
            }
        }
    }
    const float sx = ((float)r.width())/sr.width(), sy = ((float)r.height())/sr.height();
    CGRect rect = CGRectMake(r.x()-(sr.x()*sx), r.y()-(sr.y()*sy), pm.width()*sx, pm.height()*sy);
    CGImageRef image = qt_mac_create_cgimage(pm, mode, asMask);
    HIViewDrawCGImage(d->hd, &rect, image); //top left
    CGImageRelease(image);

    //restore
    CGContextRestoreGState(d->hd);
}

void
QCoreGraphicsPaintEngine::initialize()
{
}

void
QCoreGraphicsPaintEngine::cleanup()
{
}

CGContextRef
QCoreGraphicsPaintEngine::handle() const
{
    return d->hd;
}

void
QCoreGraphicsPaintEngine::drawTiledPixmap(const QRectF &r, const QPixmap &pixmap,
                                          const QPointF &p, Qt::PixmapDrawingMode)
{
    Q_ASSERT(isActive());

    //save the old state
    CGContextSaveGState(d->hd);
    //setup the pattern
    QMacPattern *qpattern = new QMacPattern;
    qpattern->data.pixmap = pixmap;
    qpattern->opaque = false;
    CGPatternCallbacks callbks;
    callbks.version = 0;
    callbks.drawPattern = qt_mac_draw_pattern;
    callbks.releaseInfo = qt_mac_dispose_pattern;
    const int width = pixmap.width(), height = pixmap.height();
    CGPatternRef pat = CGPatternCreate(qpattern, CGRectMake(0, 0, width, height),
                                       CGContextGetCTM(d->hd), width, height,
                                       kCGPatternTilingNoDistortion, true, &callbks);
    CGColorSpaceRef cs = CGColorSpaceCreatePattern(0);
    CGContextSetFillColorSpace(d->hd, cs);
    float component = 1.0; //just one
    CGContextSetFillPattern(d->hd, pat, &component);
    CGContextSetPatternPhase(d->hd, CGSizeMake(p.x()-r.x(), p.y()-r.y()));
    //fill the rectangle
    CGRect mac_rect = CGRectMake(r.x(), r.y(), r.width(), r.height());
    CGContextFillRect(d->hd, mac_rect);
    //restore the state
    CGContextRestoreGState(d->hd);
    //cleanup
    CGColorSpaceRelease(cs);
    CGPatternRelease(pat);
}

QPainter::RenderHints
QCoreGraphicsPaintEngine::supportedRenderHints() const
{
    return QPainter::RenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
}

void
QCoreGraphicsPaintEngine::updateRenderHints(QPainter::RenderHints hints)
{
    CGContextSetShouldAntialias(d->hd, hints & QPainter::Antialiasing);
    CGContextSetShouldSmoothFonts(d->hd, hints & QPainter::TextAntialiasing);
}

float
QCoreGraphicsPaintEnginePrivate::penOffset()
{
    float ret = 0;
    if(current.pen.style() != Qt::NoPen) {
        if(current.pen.width() <= 0)
            ret = 0.5;
        else
            ret = float(current.pen.width()) / 2;
    }
    return ret;
}

void
QCoreGraphicsPaintEnginePrivate::setClip(const QRegion *rgn)
{
    if(hd) {
        qt_mac_clip_cg_reset(hd);
        QPoint mp(0, 0);
        if(d->pdev->devType() == QInternal::Widget) {
            QWidget *w = static_cast<QWidget*>(pdev);
            mp = posInWindow(w);
            qt_mac_clip_cg(hd, w->d->clippedRegion(), &mp, &orig_xform);
        }
        QRegion sysClip = q->systemClip();
        if(!sysClip.isEmpty())
            qt_mac_clip_cg(hd, sysClip, 0, &orig_xform);
        if(rgn)
            qt_mac_clip_cg(hd, *rgn, 0, 0);
    }
}

void QCoreGraphicsPaintEnginePrivate::drawPath(uchar ops, CGMutablePathRef path)
{
    Q_ASSERT((ops & (CGFill | CGEOFill)) != (CGFill | CGEOFill)); //can't really happen
    if ((ops & (CGFill | CGEOFill))) {
        if (current.brush.style() == Qt::LinearGradientPattern) {
            Q_ASSERT(path);
            CGContextBeginPath(hd);
            CGContextAddPath(hd, path);
            CGContextSaveGState(hd);
            if (ops & CGFill)
                CGContextClip(hd);
            else if (ops & CGEOFill)
                CGContextEOClip(hd);
            CGContextDrawShading(hd, shading);
            CGContextRestoreGState(hd);
            ops &= ~CGFill;
            ops &= ~CGEOFill;
        } else if (current.brush.style() == Qt::NoBrush) {
            ops &= ~CGFill;
            ops &= ~CGEOFill;
        }
    }
    if ((ops & CGStroke) && current.pen.style() == Qt::NoPen)
        ops &= ~CGStroke;

    CGPathDrawingMode mode;
    if ((ops & (CGStroke | CGFill)) == (CGStroke | CGFill))
        mode = kCGPathFillStroke;
    else if ((ops & (CGStroke | CGEOFill)) == (CGStroke | CGEOFill))
        mode = kCGPathEOFillStroke;
    else if (ops & CGStroke)
        mode = kCGPathStroke;
    else if (ops & CGEOFill)
        mode = kCGPathEOFill;
    else if (ops & CGFill)
        mode = kCGPathFill;
    else //nothing to do..
        return;
    if(path) {
        CGContextBeginPath(hd);
        CGContextAddPath(hd, path);
    }
    CGContextDrawPath(hd, mode);
}
