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
#include <private/qpaintengine_raster_p.h>
#include <private/qprintengine_mac_p.h>
#include <qprinter.h>
#include <qstack.h>
#include <qtextcodec.h>
#include <qwidget.h>
#include <qvarlengtharray.h>

#include <private/qfont_p.h>
#include <private/qfontengine_p.h>
#include <private/qpainter_p.h>
#include <private/qpainterpath_p.h>
#include <private/qpixmap_p.h>
#include <private/qt_mac_p.h>
#include <private/qtextengine_p.h>
#include <private/qwidget_p.h>

#include <string.h>

/*****************************************************************************
  Internal variables and functions
 *****************************************************************************/
//#define QMAC_NATIVE_GRADIENTS

/*****************************************************************************
  External functions
 *****************************************************************************/
extern CGImageRef qt_mac_create_imagemask(const QPixmap &px); //qpixmap_mac.cpp
extern QPoint qt_mac_posInWindow(const QWidget *w); //qwidget_mac.cpp
extern WindowPtr qt_mac_window_for(const QWidget *); //qwidget_mac.cpp
extern GrafPtr qt_mac_qd_context(const QPaintDevice *); //qpaintdevice_mac.cpp
extern CGContextRef qt_mac_cg_context(const QPaintDevice *); //qpaintdevice_mac.cpp
extern void qt_mac_dispose_rgn(RgnHandle r); //qregion_mac.cpp
extern const uchar *qt_patternForBrush(int, bool); //qbrush.cpp
extern QPixmap qt_pixmapForBrush(int, bool); //qbrush.cpp

//Implemented for qt_mac_p.h
QMacCGContext::QMacCGContext(QPainter *p)
{
    QPaintEngine *pe = p->paintEngine();
    if(pe->type() == QPaintEngine::MacPrinter)
        pe = static_cast<QMacPrintEngine*>(pe)->paintEngine();
    pe->syncState();
    context = 0;
    if(pe->type() == QPaintEngine::CoreGraphics)
        context = static_cast<QCoreGraphicsPaintEngine*>(pe)->handle();
    else if(pe->type() == QPaintEngine::Raster)
        context = static_cast<QRasterPaintEngine*>(pe)->macCGContext();
    CGContextRetain(context);
}


/*****************************************************************************
  QCoreGraphicsPaintEngine utility functions
 *****************************************************************************/

//colour conversion
inline static float qt_mac_convert_color_to_cg(int c) { return ((float)c * 1000 / 255) / 1000; }


#ifdef QMAC_NATIVE_GRADIENTS
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
#endif

//clipping handling
static void qt_mac_clip_cg_reset(CGContextRef hd)
{
    //setup xforms
    CGAffineTransform old_xform = CGContextGetCTM(hd);
    CGContextConcatCTM(hd, CGAffineTransformInvert(old_xform));

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
                if(i > 0
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
    if(!p.isEmpty()
            && p.elementAt(p.elementCount() - 1).x == startPt.x()
            && p.elementAt(p.elementCount() - 1).y == startPt.y())
        CGPathCloseSubpath(ret);
    return ret;
}

void qt_mac_clip_cg(CGContextRef hd, const QRegion &rgn, const QPoint *pt, CGAffineTransform *orig_xform)
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

//pattern handling (tiling)
struct QMacPattern {
    QMacPattern() : opaque(true), as_mask(false), image(0) { data.bytes = 0; }
    ~QMacPattern() { CGImageRelease(image); }
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
    if(!pat->image) { //lazy cache
        if(pat->as_mask) {
            w = h = 8;
            CGDataProviderRef provider = CGDataProviderCreateWithData(0, pat->data.bytes, 64, 0);
            pat->image = CGImageMaskCreate(w, h, 1, 1, 1, provider, 0, false);
            CGDataProviderRelease(provider);
        } else {
            w = pat->data.pixmap.width();
            h = pat->data.pixmap.height();
            if(pat->data.pixmap.depth() == 1) {
                pat->image = qt_mac_create_imagemask(pat->data.pixmap);
            } else {
                pat->image = (CGImageRef)pat->data.pixmap.macCGHandle();
                CGImageRetain(pat->image);
            }
        }
    } else {
        w = CGImageGetWidth(pat->image);
        h = CGImageGetHeight(pat->image);
    }

    //draw
    CGRect rect = CGRectMake(0, 0, w, h);
    CGContextSaveGState(c);
    if(CGImageIsMask(pat->image)) {
        if(pat->opaque) {
            CGContextSetRGBFillColor(c, qt_mac_convert_color_to_cg(pat->background.red()),
                    qt_mac_convert_color_to_cg(pat->background.green()),
                    qt_mac_convert_color_to_cg(pat->background.blue()),
                    qt_mac_convert_color_to_cg(pat->background.alpha()));
            CGContextFillRect(c, rect);
        }
        CGContextSetRGBFillColor(c, qt_mac_convert_color_to_cg(pat->foreground.red()),
                qt_mac_convert_color_to_cg(pat->foreground.green()),
                qt_mac_convert_color_to_cg(pat->foreground.blue()),
                qt_mac_convert_color_to_cg(pat->foreground.alpha()));
    }
    HIViewDrawCGImage(c, &rect, pat->image);
    CGContextRestoreGState(c);
}
static void qt_mac_dispose_pattern(void *info)
{
    QMacPattern *pat = (QMacPattern*)info;
    delete pat;
}

/*****************************************************************************
  QCoreGraphicsPaintEngine member functions
 *****************************************************************************/

inline static QPaintEngine::PaintEngineFeatures qt_mac_cg_features()
{
    // Supports all except gradients...
    return QPaintEngine::PaintEngineFeatures(QPaintEngine::AllFeatures
            & ~QPaintEngine::PaintOutsidePaintEvent
            & (~(QPaintEngine::ConicalGradientFill | QPaintEngine::BrushStroke))
#ifndef QMAC_NATIVE_GRADIENTS
            & (~(QPaintEngine::LinearGradientFill|QPaintEngine::RadialGradientFill))
#endif
            );
}

    QCoreGraphicsPaintEngine::QCoreGraphicsPaintEngine()
: QPaintEngine(*(new QCoreGraphicsPaintEnginePrivate), qt_mac_cg_features())
{
}

    QCoreGraphicsPaintEngine::QCoreGraphicsPaintEngine(QPaintEnginePrivate &dptr)
: QPaintEngine(dptr, qt_mac_cg_features())
{
}

QCoreGraphicsPaintEngine::~QCoreGraphicsPaintEngine()
{
}

    bool
QCoreGraphicsPaintEngine::begin(QPaintDevice *pdev)
{
    Q_D(QCoreGraphicsPaintEngine);
    if(isActive()) {                         // already active painting
        qWarning("QCoreGraphicsPaintEngine::begin: Painter already active");
        return false;
    }

    d->complexXForm = false;

    //initialization
    d->current.clipEnabled = false;
    d->pdev = pdev;
    d->hd = qt_mac_cg_context(pdev);
    if(d->hd) {
        d->orig_xform = CGContextGetCTM(d->hd);
        if(d->shading) {
            CGShadingRelease(d->shading);
            d->shading = 0;
        }
        d->setClip(0);  //clear the context's clipping
    }

    setActive(true);

    if(d->pdev->devType() == QInternal::Widget) {                    // device is a widget
        QWidget *w = (QWidget*)d->pdev;
        bool unclipped = w->testAttribute(Qt::WA_PaintUnclipped);

        if((w->windowType() == Qt::Desktop)) {
            if(!unclipped)
                qWarning("QCoreGraphicsPaintEngine::begin: Does not support clipped desktop on Mac OS X");
            ShowWindow(qt_mac_window_for(w));
        } else if(unclipped) {
            qWarning("QCoreGraphicsPaintEngine::begin: Does not support unclipped painting");
        }
    } else if(d->pdev->devType() == QInternal::Pixmap) {             // device is a pixmap
        QPixmap *pm = (QPixmap*)d->pdev;
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
    Q_D(QCoreGraphicsPaintEngine);
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

void QCoreGraphicsPaintEngine::updateState(const QPaintEngineState &state)
{
    QPaintEngine::DirtyFlags flags = state.state();
    if(flags & DirtyTransform)
        updateMatrix(state.matrix());
    if(flags & (DirtyBackground|DirtyBackgroundMode))
        updateBackground(state.backgroundMode(), state.backgroundBrush());
    if(flags & DirtyPen)
        updatePen(state.pen());
    if(flags & (DirtyBrush|DirtyBrushOrigin))
        updateBrush(state.brush(), state.brushOrigin());
    if(flags & DirtyFont)
        updateFont(state.font());
    if (flags & DirtyOpacity)
        updateOpacity(state.opacity());
    if (state.state() & DirtyClipEnabled) {
        if (state.isClipEnabled())
            updateClipPath(painter()->clipPath(), Qt::ReplaceClip);
        else
            updateClipPath(QPainterPath(), Qt::NoClip);
    }
    if(flags & DirtyClipPath)
        updateClipPath(state.clipPath(), state.clipOperation());
    if(flags & DirtyClipRegion)
        updateClipRegion(state.clipRegion(), state.clipOperation());
    if(flags & DirtyHints)
        updateRenderHints(state.renderHints());
}

void
QCoreGraphicsPaintEngine::updatePen(const QPen &pen)
{
    Q_D(QCoreGraphicsPaintEngine);
    Q_ASSERT(isActive());
    d->current.pen = pen;
    d->setStrokePen(pen);
    d->cosmeticPen = pen.widthF() == 0;
}

void
QCoreGraphicsPaintEngine::updateBrush(const QBrush &brush, const QPointF &brushOrigin)
{
    Q_D(QCoreGraphicsPaintEngine);
    Q_ASSERT(isActive());
    d->current.brush = brush;
    d->current.bg.origin = brushOrigin;

    if(d->shading) {
        CGShadingRelease(d->shading);
        d->shading = 0;
    }
    d->setFillBrush(brush, brushOrigin);
}

void
QCoreGraphicsPaintEngine::updateOpacity(qreal opacity)
{
    Q_D(QCoreGraphicsPaintEngine);
    CGContextSetAlpha(d->hd, opacity);
}

void
QCoreGraphicsPaintEngine::updateFont(const QFont &)
{
    Q_D(QCoreGraphicsPaintEngine);
    Q_ASSERT(isActive());
    updatePen(d->current.pen);
}

void
QCoreGraphicsPaintEngine::updateBackground(Qt::BGMode mode, const QBrush &brush)
{
    Q_D(QCoreGraphicsPaintEngine);
    Q_ASSERT(isActive());
    d->current.bg.mode = mode;
    d->current.bg.brush = brush;
}

void
QCoreGraphicsPaintEngine::updateMatrix(const QMatrix &matrix)
{
    Q_D(QCoreGraphicsPaintEngine);
    Q_ASSERT(isActive());
    d->setTransform(matrix.isIdentity() ? 0 : &matrix);

    d->complexXForm = (matrix.m11() != 1 || matrix.m22() != 1
            || matrix.m12() != 0 || matrix.m21() != 0);
}

void
QCoreGraphicsPaintEngine::updateClipPath(const QPainterPath &p, Qt::ClipOperation op)
{
    Q_D(QCoreGraphicsPaintEngine);
    Q_ASSERT(isActive());
    if(op == Qt::NoClip) {
        if(d->current.clipEnabled) {
            d->current.clipEnabled = false;
            d->current.clip = QRegion();
            d->setClip(0);
        }
    } else {
        if(!d->current.clipEnabled)
            op = Qt::ReplaceClip;
        d->current.clipEnabled = true;
        QRegion clipRegion(p.toFillPolygon().toPolygon(), p.fillRule());
        if(op == Qt::ReplaceClip) {
            d->current.clip = clipRegion;
            d->setClip(0);
            if(p.isEmpty()) {
                CGRect rect = CGRectMake(0, 0, 0, 0);
                CGContextClipToRect(d->hd, rect);
            } else {
                CGMutablePathRef path = qt_mac_compose_path(p);
                CGContextBeginPath(d->hd);
                CGContextAddPath(d->hd, path);
                CGContextClip(d->hd);
                CGPathRelease(path);
            }
        } else if(op == Qt::IntersectClip) {
            d->current.clip = d->current.clip.intersect(clipRegion);
            d->setClip(&d->current.clip);
        } else if(op == Qt::UniteClip) {
            d->current.clip = d->current.clip.unite(clipRegion);
            d->setClip(&d->current.clip);
        }
    }
}

void
QCoreGraphicsPaintEngine::updateClipRegion(const QRegion &clipRegion, Qt::ClipOperation op)
{
    Q_D(QCoreGraphicsPaintEngine);
    Q_ASSERT(isActive());
    if(op == Qt::NoClip) {
        d->current.clipEnabled = false;
        d->current.clip = QRegion();
        d->setClip(0);
    } else {
        if(!d->current.clipEnabled)
            op = Qt::ReplaceClip;
        d->current.clipEnabled = true;
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
QCoreGraphicsPaintEngine::drawPath(const QPainterPath &p)
{
    Q_D(QCoreGraphicsPaintEngine);
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
QCoreGraphicsPaintEngine::drawRects(const QRectF *rects, int rectCount)
{
    Q_D(QCoreGraphicsPaintEngine);
    Q_ASSERT(isActive());

    for (int i=0; i<rectCount; ++i) {
        QRectF r = rects[i];

        CGMutablePathRef path = CGPathCreateMutable();
        CGPathAddRect(path, 0, qt_mac_compose_rect(r, d->penOffset()));
        d->drawPath(QCoreGraphicsPaintEnginePrivate::CGFill|QCoreGraphicsPaintEnginePrivate::CGStroke,
                path);
        CGPathRelease(path);
    }
}

void
QCoreGraphicsPaintEngine::drawPoints(const QPointF *points, int pointCount)
{
    Q_D(QCoreGraphicsPaintEngine);
    Q_ASSERT(isActive());

    CGMutablePathRef path = CGPathCreateMutable();
    for(int i=0; i < pointCount; i++) {
        float x = points[i].x(), y = points[i].y();
        CGPathMoveToPoint(path, 0, x, y + 1);
        CGPathAddLineToPoint(path, 0, x + 0.01, y+1);
    }
    d->drawPath(QCoreGraphicsPaintEnginePrivate::CGStroke, path);
    CGPathRelease(path);
}

void
QCoreGraphicsPaintEngine::drawEllipse(const QRectF &r)
{
    Q_D(QCoreGraphicsPaintEngine);
    Q_ASSERT(isActive());

    CGMutablePathRef path = CGPathCreateMutable();
    CGAffineTransform transform = CGAffineTransformMakeScale(r.width() / r.height(), 1);
    CGPathAddArc(path, &transform,((r.x()+d->penOffset()) + (r.width() / 2)) / (r.width() / r.height()),
            (r.y()+d->penOffset()) + (r.height() / 2), r.height() / 2, 0, (2 * M_PI), false);
    d->drawPath(QCoreGraphicsPaintEnginePrivate::CGFill | QCoreGraphicsPaintEnginePrivate::CGStroke,
            path);
    CGPathRelease(path);
}

void
QCoreGraphicsPaintEngine::drawPolygon(const QPointF *points, int pointCount, PolygonDrawMode mode)
{
    Q_D(QCoreGraphicsPaintEngine);
    Q_ASSERT(isActive());

    CGMutablePathRef path = CGPathCreateMutable();
    CGPathMoveToPoint(path, 0, points[0].x(), points[0].y()+1);
    for(int x = 1; x < pointCount; ++x)
        CGPathAddLineToPoint(path, 0, points[x].x(), points[x].y()+1);
    if(mode != PolylineMode && points[0] != points[pointCount-1])
        CGPathAddLineToPoint(path, 0, points[0].x(), points[0].y()+1);
    uint op = QCoreGraphicsPaintEnginePrivate::CGStroke;
    if (mode != PolylineMode)
        op |= mode == OddEvenMode ? QCoreGraphicsPaintEnginePrivate::CGEOFill
            : QCoreGraphicsPaintEnginePrivate::CGFill;
    d->drawPath(op, path);
    CGPathRelease(path);
}

void
QCoreGraphicsPaintEngine::drawLines(const QLineF *lines, int lineCount)
{
    Q_D(QCoreGraphicsPaintEngine);
    Q_ASSERT(isActive());

    CGMutablePathRef path = CGPathCreateMutable();
    for(int i = 0; i < lineCount; i++) {
        const QPointF start = lines[i].p1(), end = lines[i].p2();
        CGPathMoveToPoint(path, 0, start.x(), start.y()+1);
        CGPathAddLineToPoint(path, 0, end.x(), end.y()+1);
    }
    d->drawPath(QCoreGraphicsPaintEnginePrivate::CGStroke, path);
    CGPathRelease(path);
}

void
QCoreGraphicsPaintEngine::drawPixmap(const QRectF &r, const QPixmap &pm, const QRectF &sr)
{
    Q_D(QCoreGraphicsPaintEngine);
    Q_ASSERT(isActive());
    if(pm.isNull())
        return;

    //save
    CGContextSaveGState(d->hd);

    //set clip
    QRegion rgn(r.toRect());
    qt_mac_clip_cg(d->hd, rgn, 0, 0);

    //draw
    const float sx = ((float)r.width())/sr.width(), sy = ((float)r.height())/sr.height();
    CGRect rect = CGRectMake(r.x()-(sr.x()*sx), r.y()-(sr.y()*sy), pm.width()*sx, pm.height()*sy);
    if(pm.depth() == 1) {
        if(d->current.bg.mode == Qt::OpaqueMode) {
            d->setFillBrush(d->current.bg.brush);
            CGContextFillRect(d->hd, qt_mac_compose_rect(r, 0));
        }
        const QColor &col = d->current.pen.color();
        CGContextSetRGBFillColor(d->hd, qt_mac_convert_color_to_cg(col.red()),
                qt_mac_convert_color_to_cg(col.green()),
                qt_mac_convert_color_to_cg(col.blue()),
                qt_mac_convert_color_to_cg(col.alpha()));

        CGImageRef image = qt_mac_create_imagemask(pm);
        HIViewDrawCGImage(d->hd, &rect, image);
        CGImageRelease(image);
    } else {
        HIViewDrawCGImage(d->hd, &rect, (CGImageRef)pm.macCGHandle());
    }

    //restore
    CGContextRestoreGState(d->hd);
}


static void drawImageReleaseData (void *info, const void *, size_t)
{
    delete static_cast<QImage *>(info);
}


void QCoreGraphicsPaintEngine::drawImage(const QRectF &r, const QImage &img, const QRectF &sr,
                                         Qt::ImageConversionFlags)
{
    Q_D(QCoreGraphicsPaintEngine);
    Q_ASSERT(isActive());
    if (img.isNull())
        return;

    const QImage *image = &img;
    QImage *newImage = 0;
    if (img.depth() != 32) {
        newImage = new QImage(img.convertToFormat(QImage::Format_ARGB32_Premultiplied));
        image = newImage;
    }
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4)
    uint cgflags = 0;
# ifdef kCGBitmapByteOrder32Host //only needed because CGImage.h added symbols in the minor version
    if(QSysInfo::MacintoshVersion >= QSysInfo::MV_10_4)
        cgflags |= kCGBitmapByteOrder32Host;
# endif
#else
    CGImageAlphaInfo cgflags = kCGImageAlphaPremultipliedFirst;
#endif
    switch (image->format()) {
    default:
    case QImage::Format_ARGB32_Premultiplied:
        cgflags |= kCGImageAlphaPremultipliedFirst;
        break;
    case QImage::Format_ARGB32:
        cgflags |= kCGImageAlphaFirst;
        break;
    case QImage::Format_RGB32:
        cgflags |= kCGImageAlphaNone;
        break;
    }
    QCFType<CGDataProviderRef> dataProvider = CGDataProviderCreateWithData(newImage,
                                                          image->bits(),
                                                          image->numBytes(),
                                                          drawImageReleaseData);
   QCFType<CGImageRef> cgimage = CGImageCreate(image->width(), image->height(), 8, 32,
                                        image->bytesPerLine(),
                                QCFType<CGColorSpaceRef>(CGColorSpaceCreateDeviceRGB()),
                                cgflags, dataProvider, 0, false, kCGRenderingIntentDefault);

    const float sx = ((float)r.width())/sr.width(), sy = ((float)r.height())/sr.height();
    CGRect rect = CGRectMake(r.x()-(sr.x()*sx), r.y()-(sr.y()*sy),
                             image->width()*sx, image->height()*sy);
    HIViewDrawCGImage(d->hd, &rect, cgimage);
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
    return d_func()->hd;
}

void
QCoreGraphicsPaintEngine::drawTiledPixmap(const QRectF &r, const QPixmap &pixmap,
        const QPointF &p)
{
    Q_D(QCoreGraphicsPaintEngine);
    Q_ASSERT(isActive());

    //save the old state
    CGContextSaveGState(d->hd);

    //setup the pattern
    QMacPattern *qpattern = new QMacPattern;
    qpattern->data.pixmap = pixmap;
    qpattern->opaque = (d->current.bg.mode == Qt::OpaqueMode);
    qpattern->foreground = d->current.pen.color();
    qpattern->background = d->current.bg.brush.color();
    CGPatternCallbacks callbks;
    callbks.version = 0;
    callbks.drawPattern = qt_mac_draw_pattern;
    callbks.releaseInfo = qt_mac_dispose_pattern;
    const int width = pixmap.width(), height = pixmap.height();
    CGAffineTransform trans = CGContextGetCTM(d->hd);
    CGPatternRef pat = CGPatternCreate(qpattern, CGRectMake(0, 0, width, height),
            trans, width, height,
            kCGPatternTilingNoDistortion, true, &callbks);
    CGColorSpaceRef cs = CGColorSpaceCreatePattern(0);
    CGContextSetFillColorSpace(d->hd, cs);
    float component = 1.0; //just one
    CGContextSetFillPattern(d->hd, pat, &component);
    CGSize phase = CGSizeApplyAffineTransform(CGSizeMake(-(p.x()-r.x()), -(p.y()-r.y())), trans);
    CGContextSetPatternPhase(d->hd, phase);

    //fill the rectangle
    CGRect mac_rect = CGRectMake(r.x(), r.y(), r.width(), r.height());
    CGContextFillRect(d->hd, mac_rect);

    //restore the state
    CGContextRestoreGState(d->hd);
    //cleanup
    CGColorSpaceRelease(cs);
    CGPatternRelease(pat);
}

void QCoreGraphicsPaintEngine::drawTextItem(const QPointF &pos, const QTextItem &item)
{
    Q_D(QCoreGraphicsPaintEngine);

    const QTextItemInt &ti = static_cast<const QTextItemInt &>(item);

    QPen oldPen = painter()->pen();
    QBrush oldBrush = painter()->brush();
    QPointF oldBrushOrigin = painter()->brushOrigin();
    updatePen(Qt::NoPen);
    updateBrush(oldPen.brush(), QPointF(0, 0));

    Q_ASSERT(type() == QPaintEngine::CoreGraphics);

    const bool textAA = state->renderHints() & QPainter::TextAntialiasing;
    const bool lineAA = state->renderHints() & QPainter::Antialiasing;
    if(textAA != lineAA)
        CGContextSetShouldAntialias(d->hd, textAA);

    QFontEngineMac *fe = static_cast<QFontEngineMac *>(ti.fontEngine);
    if (ti.num_glyphs)
        fe->draw(d->hd, pos.x(), pos.y(), ti, paintDevice()->height());

    if(textAA != lineAA)
        CGContextSetShouldAntialias(d->hd, !textAA);

    updatePen(oldPen);
    updateBrush(oldBrush, oldBrushOrigin);
}

QPainter::RenderHints
QCoreGraphicsPaintEngine::supportedRenderHints() const
{
    return QPainter::RenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);
}

void
QCoreGraphicsPaintEngine::updateRenderHints(QPainter::RenderHints hints)
{
    Q_D(QCoreGraphicsPaintEngine);
    CGContextSetShouldAntialias(d->hd, hints & QPainter::Antialiasing);
    CGContextSetInterpolationQuality(d->hd, (hints & QPainter::SmoothPixmapTransform) ?
                                     kCGInterpolationDefault : kCGInterpolationNone);
    CGContextSetShouldSmoothFonts(d->hd, hints & QPainter::TextAntialiasing);
}

    float
QCoreGraphicsPaintEnginePrivate::penOffset()
{
    // ### This function does not deserve to exist, remove!
    if(complexXForm)
        return 0;
    float ret = 0;
    if(current.pen.style() != Qt::NoPen) {
        if(current.pen.widthF() < 1.0)
            ret = 0.5;
        else
            ret = 0.0f;
    }
    return ret;
}

void
QCoreGraphicsPaintEnginePrivate::setStrokePen(const QPen &pen)
{
    //pencap
    CGLineCap cglinecap = kCGLineCapButt;
    if(pen.capStyle() == Qt::SquareCap)
        cglinecap = kCGLineCapSquare;
    else if(pen.capStyle() == Qt::RoundCap)
        cglinecap = kCGLineCapRound;
    CGContextSetLineCap(hd, cglinecap);

    //penwidth
    const float cglinewidth = pen.widthF() <= 0.0f ? 1.0f : float(pen.widthF());
    CGContextSetLineWidth(hd, cglinewidth);

    //join
    CGLineJoin cglinejoin = kCGLineJoinMiter;
    if(pen.joinStyle() == Qt::BevelJoin)
        cglinejoin = kCGLineJoinBevel;
    else if(pen.joinStyle() == Qt::RoundJoin)
        cglinejoin = kCGLineJoinRound;
    CGContextSetLineJoin(hd, cglinejoin);
//    CGContextSetMiterLimit(hd, pen.miterLimit());

    //pen style
    QVector<float> linedashes;
    if(pen.style() == Qt::CustomDashLine) {
        QVector<qreal> customs = pen.dashPattern();
        for(int i = 0; i < customs.size(); ++i)
            linedashes.append(customs.at(i));
    } else if(pen.style() == Qt::DashLine) {
        linedashes.append(3);
        linedashes.append(1);
    } else if(pen.style() == Qt::DotLine) {
        linedashes.append(1);
        linedashes.append(1);
    } else if(pen.style() == Qt::DashDotLine) {
        linedashes.append(3);
        linedashes.append(1);
        linedashes.append(1);
        linedashes.append(1);
    } else if(pen.style() == Qt::DashDotDotLine) {
        linedashes.append(3);
        linedashes.append(1);
        linedashes.append(1);
        linedashes.append(1);
        linedashes.append(1);
        linedashes.append(1);
    }
    for(int i = 0; i < linedashes.size(); ++i) {
        linedashes[i] *= cglinewidth;
        if(cglinecap == kCGLineCapSquare || cglinecap == kCGLineCapRound) {
            if((i%2))
                linedashes[i] += cglinewidth/2;
            else
                linedashes[i] -= cglinewidth/2;
        }
    }
    CGContextSetLineDash(hd, 0, linedashes.data(), linedashes.size());

    //color
    const QColor &col = pen.color();
    CGContextSetRGBStrokeColor(hd, qt_mac_convert_color_to_cg(col.red()),
            qt_mac_convert_color_to_cg(col.green()),
            qt_mac_convert_color_to_cg(col.blue()),
            qt_mac_convert_color_to_cg(col.alpha()));
}

void
QCoreGraphicsPaintEnginePrivate::setFillBrush(const QBrush &brush, const QPointF &offset)
{
    //pattern
    Qt::BrushStyle bs = brush.style();
    if(bs == Qt::LinearGradientPattern) {
#ifdef QMAC_NATIVE_GRADIENTS
        CGFunctionCallbacks callbacks = { 0, qt_mac_color_gradient_function, 0 };
        CGFunctionRef fill_func = CGFunctionCreate(const_cast<void *>(reinterpret_cast<const void *>(&brush)),
                1, 0, 4, 0, &callbacks);
        CGColorSpaceRef grad_colorspace = CGColorSpaceCreateDeviceRGB();
        const QLinearGradient *linGrad = static_cast<const QLinearGradient*>(brush.gradient());
        const QPointF start = linGrad->start(), stop = linGrad->finalStop();
        d->shading = CGShadingCreateAxial(grad_colorspace, CGPointMake(start.x(), start.y()),
                CGPointMake(stop.x(), stop.y()), fill_func, true, true);
        CGFunctionRelease(fill_func);
        CGColorSpaceRelease(grad_colorspace);
#endif
    } else if(bs == Qt::RadialGradientPattern || bs == Qt::ConicalGradientPattern) {
#ifdef QMAC_NATIVE_GRADIENTS
        qWarning("QCoreGraphicsPaintEngine: Unhandled gradient %d", (int)bs);
#endif
    } else if(bs != Qt::SolidPattern && bs != Qt::NoBrush) {
        int width = 0, height = 0;
        QMacPattern *qpattern = new QMacPattern;
        float components[4] = { 1.0, 1.0, 1.0, 1.0 };
        CGColorSpaceRef base_colorspace = 0;
        if(bs == Qt::TexturePattern) {
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

            Qt::BrushStyle bsForPattern;
            switch (bs) {
            // Since the matrix is flipped, we need to filp the diagonal
            default:
                bsForPattern = bs;
                break;
            case Qt::BDiagPattern:
                bsForPattern = Qt::FDiagPattern;
                break;
            case Qt::FDiagPattern:
                bsForPattern = Qt::BDiagPattern;
                break;
            }
            qpattern->data.bytes = qt_patternForBrush(bsForPattern, false);
            width = height = 8;
            const QColor &col = brush.color();
            components[0] = qt_mac_convert_color_to_cg(col.red());
            components[1] = qt_mac_convert_color_to_cg(col.green());
            components[2] = qt_mac_convert_color_to_cg(col.blue());
            base_colorspace = CGColorSpaceCreateDeviceRGB();
        }
        //qpattern->offset = offset.toPoint();
        qpattern->opaque = (current.bg.mode == Qt::OpaqueMode);
        qpattern->foreground = brush.color();
        qpattern->background = current.bg.brush.color();

        CGColorSpaceRef fill_colorspace = CGColorSpaceCreatePattern(base_colorspace);
        CGContextSetFillColorSpace(hd, fill_colorspace);

        CGAffineTransform xform = CGContextGetCTM(hd);
        xform = CGAffineTransformTranslate(xform, offset.x(), offset.y());

        CGPatternCallbacks callbks;
        callbks.version = 0;
        callbks.drawPattern = qt_mac_draw_pattern;
        callbks.releaseInfo = qt_mac_dispose_pattern;
        CGPatternRef fill_pattern = CGPatternCreate(qpattern, CGRectMake(0, 0, width, height),
                xform, width, height, kCGPatternTilingNoDistortion,
                !base_colorspace, &callbks);
        CGContextSetFillPattern(hd, fill_pattern, components);

        CGPatternRelease(fill_pattern);
        CGColorSpaceRelease(fill_colorspace);
        if(base_colorspace)
            CGColorSpaceRelease(base_colorspace);
    } else if(bs != Qt::NoBrush) {
        const QColor &col = brush.color();
        CGContextSetRGBFillColor(hd, qt_mac_convert_color_to_cg(col.red()),
                qt_mac_convert_color_to_cg(col.green()),
                qt_mac_convert_color_to_cg(col.blue()),
                qt_mac_convert_color_to_cg(col.alpha()));
    }
}

void
QCoreGraphicsPaintEnginePrivate::setClip(const QRegion *rgn)
{
    Q_Q(QCoreGraphicsPaintEngine);
    if(hd) {
        qt_mac_clip_cg_reset(hd);
        QPoint mp(0, 0);
        if(pdev->devType() == QInternal::Widget) {
            QWidget *w = static_cast<QWidget*>(pdev);
            mp = qt_mac_posInWindow(w);
            qt_mac_clip_cg(hd, w->d_func()->clippedRegion(), &mp, &orig_xform);
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
    if((ops & (CGFill | CGEOFill))) {
        if(current.brush.style() == Qt::LinearGradientPattern) {
            Q_ASSERT(path);
            CGContextBeginPath(hd);
            CGContextAddPath(hd, path);
            CGContextSaveGState(hd);
            if(ops & CGFill)
                CGContextClip(hd);
            else if(ops & CGEOFill)
                CGContextEOClip(hd);
            CGContextDrawShading(hd, shading);
            CGContextRestoreGState(hd);
            ops &= ~CGFill;
            ops &= ~CGEOFill;
        } else if(current.brush.style() == Qt::NoBrush) {
            ops &= ~CGFill;
            ops &= ~CGEOFill;
        }
    }
    if((ops & CGStroke) && current.pen.style() == Qt::NoPen)
        ops &= ~CGStroke;

    if (ops & (CGEOFill | CGFill)) {
        CGContextBeginPath(hd);
        CGContextAddPath(hd, path);
        if (ops & CGEOFill)
            CGContextEOFillPath(hd);
        else
            CGContextFillPath(hd);
    }

    if (ops & CGStroke) {
        if (cosmeticPen) {
            CGContextSaveGState(hd);
            CGContextBeginPath(hd);
            CGContextAddPath(hd, path);
            CGAffineTransform newTransform = CGAffineTransformInvert(CGContextGetCTM(hd));
            newTransform = CGAffineTransformScale(newTransform, 1, -1);
            newTransform = CGAffineTransformTranslate(newTransform, 0, -pdev->height());
            CGContextConcatCTM(hd, newTransform);
            CGContextStrokePath(hd);
            CGContextRestoreGState(hd);
        } else {
            CGContextBeginPath(hd);
            CGContextAddPath(hd, path);
            CGContextStrokePath(hd);
        }
    }
}
