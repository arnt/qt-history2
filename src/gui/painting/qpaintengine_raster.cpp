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

#define QT_FT_BEGIN_HEADER
#define QT_FT_END_HEADER
#include <private/qrasterdefs_p.h>
#include <private/qgrayraster_p.h>
#include <private/qblackraster_p.h>

#include <qpainterpath.h>
#include <qdebug.h>
#include <qhash.h>
#include <qlabel.h>
#include <qbitmap.h>

#include <private/qmath_p.h>

#include <private/qdatabuffer_p.h>
#include <private/qpainter_p.h>
#include <private/qtextengine_p.h>
#include <private/qpixmap_p.h>
#include <private/qfontengine_p.h>

#include "qpaintengine_raster_p.h"

#if defined(Q_WS_X11)
#  include <qwidget.h>
#  include <qx11info_x11.h>
#  include <X11/Xlib.h>
#  include <X11/Xutil.h>
#elif defined(Q_WS_WIN)
#  include <qt_windows.h>
#  include <qvarlengtharray.h>
#  include <private/qfontengine_p.h>
#elif defined(Q_WS_MAC)
#  include <private/qt_mac_p.h>
#  if Q_BYTE_ORDER == Q_BIG_ENDIAN
#    define BITMAPS_ARE_MSB
#  endif
#endif

#if defined(Q_WS_WIN64)
#  include <malloc.h>
#endif



/*
    Used to prevent division by zero in LinearGradientData::init.
    This number does not have to be the smallest possible positive qreal.
    The size of the result of QPaintDevice::width() is more interesting, since the error
    is accumulated for each pixel in the interpolation. Thus, interpolating over a large number of pixels
    will make the error larger.
*/
#define GRADIENT_EPSILON  0.000000001

#define qreal_to_fixed_26_6(f) (int(f * 64))
#define qt_swap_int(x, y) { int tmp = (x); (x) = (y); (y) = tmp; }
#define qt_swap_qreal(x, y) { qreal tmp = (x); (x) = (y); (y) = tmp; }

static QT_FT_Raster qt_gray_raster;
static QT_FT_Raster qt_black_raster;

static void qt_initialize_ft()
{
//    printf("qt_initialize_ft()\n");
    int error;

    // The antialiasing raster.
    error = qt_ft_grays_raster.raster_new(0, &qt_gray_raster);
    if (error) {
        qWarning("failed to initlize qt_ft_grays_raster");
        return;
    }

    unsigned long poolSize = 128 * 128;

#if defined(Q_WS_WIN64)
    unsigned char *poolBase = (unsigned char *) _aligned_malloc(poolSize, __alignof(void*));
#else
    unsigned char *poolBase = (unsigned char *) malloc(poolSize);
#endif

    qt_ft_grays_raster.raster_reset(qt_gray_raster, poolBase, poolSize);

    // Initialize the standard raster.
    error = qt_ft_standard_raster.raster_new(0, &qt_black_raster);
    if (error) {
        qWarning("Failed to initialize standard raster: code=%d\n", error);
        return;
    }
    qt_ft_standard_raster.raster_reset(qt_black_raster, poolBase, poolSize);
}

#ifdef Q_WS_WIN
static void qt_draw_text_item(const QPointF &point, const QTextItemInt &ti, HDC hdc,
                       QRasterPaintEnginePrivate *d);
#endif

// #define QT_DEBUG_DRAW
// #define QT_DEBUG_CONVERT

/********************************************************************************
 * Span functions
 */
static void qt_span_fill_clipped(int count, const QSpan *spans, void *userData);
static void qt_span_clip(int count, const QSpan *spans, void *userData);

struct ClipData
{
    QClipData *oldClip;
    QClipData *newClip;
    Qt::ClipOperation operation;
};

static void qt_scanconvert(QT_FT_Outline *outline, ProcessSpans callback, void *userData, QRasterPaintEnginePrivate *d);


enum LineDrawMode {
    LineDrawClipped,
    LineDrawNormal,
    LineDrawIncludeLastPixel
};

static void drawLine_midpoint_i(const QLine &line, ProcessSpans span_func, void *data,
                                LineDrawMode style, const QRect &devRect);
// static void drawLine_midpoint_f(const QLineF &line, qt_span_func span_func, void *data,
//                                 LineDrawMode style, const QRect &devRect);


/********************************************************************************
 * class QFTOutlineMapper
 *
 * Used to map between QPainterPath and the QT_FT_Outline structure used by the
 * freetype scanconvertor.
 *
 * The outline mapper uses a path iterator to get points from the path,
 * so that it is possible to transform the points as they are converted. The
 * callback can be a noop, translate or full-fledged xform. (Tests indicated
 * that using a C callback was low cost).
 */
class QFTOutlineMapper
{
public:

    /*!
      Sets up the matrix to be used for conversion. This also
      sets up the qt_path_iterator function that is used as a callback
      to get points.
    */
    void setMatrix(const QMatrix &m, uint /* txop */)
    {
        m_m11 = m.m11();
        m_m12 = m.m12();
        m_m21 = m.m21();
        m_m22 = m.m22();
        m_dx = m.dx();
        m_dy = m.dy();
    }

    inline qreal map_x(qreal x, qreal y) { return m_m11*x + m_m21*y + m_dx; }

    inline qreal map_y(qreal x, qreal y) { return m_m12*x + m_m22*y + m_dy; }

    void beginOutline(Qt::FillRule fillRule)
    {
#ifdef QT_DEBUG_CONVERT
        printf("QFTOutlineMapper::beginOutline rule=%d\n", fillRule);
#endif

        m_points.reset();
        m_tags.reset();
        m_contours.reset();
        m_outline.flags = fillRule == Qt::WindingFill
                          ? QT_FT_OUTLINE_NONE
                          : QT_FT_OUTLINE_EVEN_ODD_FILL;
        m_subpath_start.x = m_subpath_start.y = 0;
    }

    void endOutline()
    {
        closeSubpath();

        m_outline.n_contours = m_contours.size();
        m_outline.n_points = m_points.size();

        m_outline.points = m_points.data();
        m_outline.tags = m_tags.data();
        m_outline.contours = m_contours.data();


#ifdef QT_DEBUG_CONVERT
        printf("QFTOutlineMapper::endOutline\n");

        printf(" - contours: %d\n", m_outline.n_contours);
        for (int i=0; i<m_outline.n_contours; ++i) {
            printf("   - %d\n", m_outline.contours[i]);
        }

        printf(" - points: %d\n", m_outline.n_points);
        for (int i=0; i<m_outline.n_points; ++i) {
            printf("   - %d -- %.2f, %.2f, (%d, %d)\n", i,
                   m_outline.points[i].x / 64.0,
                   m_outline.points[i].y / 64.0,
                   m_outline.points[i], m_outline.points[i]);
        }
#endif
    }

    inline void closeSubpath()
    {
#ifdef QT_DEBUG_CONVERT
        printf("QFTOutlineMapper::closeSubpath()\n");
#endif
        int pointCount = m_points.size();
        if (pointCount>0) {
#ifdef QT_DEBUG_CONVERT
            printf(" - implicitly closed\n");
#endif

            QT_FT_Vector last = m_points.at(pointCount-1);

            // implicitly close last if not already closed.
            if (m_subpath_start.x != last.x || m_subpath_start.y != last.y) {
                m_points.add(m_subpath_start);
                m_tags.add(QT_FT_CURVE_TAG_ON);
            }

            // for close only (not first point)
            m_contours.add(m_points.size() - 1);
        }
    }


    void moveTo(const QPointF &pt)
    {
        QT_FT_Vector pt_fixed = { qreal_to_fixed_26_6(map_x(pt.x(), pt.y())),
                                  qreal_to_fixed_26_6(map_y(pt.x(), pt.y())) };
#ifdef QT_DEBUG_CONVERT
        printf("QFTOutlineMapper::moveTo(%.2f, %.2f): subpath started=(%.2f,%.2f)\n",
               pt_fixed.x / 64.0, pt_fixed.y / 64.0,
               m_subpath_start.x / 64.0, m_subpath_start.y / 64.0);
#endif
        closeSubpath();
        m_points.add(pt_fixed);
        m_tags.add(QT_FT_CURVE_TAG_ON);
        m_subpath_start = pt_fixed;
    }


    void lineTo(const QPointF &pt)
    {
        QT_FT_Vector pt_fixed = { qreal_to_fixed_26_6(map_x(pt.x(), pt.y())),
                                  qreal_to_fixed_26_6(map_y(pt.x(), pt.y())) };
#ifdef QT_DEBUG_CONVERT
        printf("QFTOutlineMapper::lineTo(%.2f, %.2f)\n",
               pt_fixed.x / 64.0, pt_fixed.y / 64.0);
#endif
        m_points.add(pt_fixed);
        m_tags.add(QT_FT_CURVE_TAG_ON);
    }


    void curveTo(const QPointF &cp1, const QPointF &cp2, const QPointF &ep)
    {
        QT_FT_Vector cp1_fixed = { qreal_to_fixed_26_6(map_x(cp1.x(), cp1.y())),
                                   qreal_to_fixed_26_6(map_y(cp1.x(), cp1.y())) };
        QT_FT_Vector cp2_fixed = { qreal_to_fixed_26_6(map_x(cp2.x(), cp2.y())),
                                   qreal_to_fixed_26_6(map_y(cp2.x(), cp2.y())) };
        QT_FT_Vector ep_fixed = { qreal_to_fixed_26_6(map_x(ep.x(), ep.y())),
                                  qreal_to_fixed_26_6(map_y(ep.x(), ep.y())) };

        m_points.add(cp1_fixed);
        m_points.add(cp2_fixed);
        m_points.add(ep_fixed);

#ifdef QT_DEBUG_CONVERT
        printf("QFTOutlineMapper::curveTo((%.2f,%.2f) ,(%.2f,%.2f), (%.2f,%.2f))\n",
               cp1_fixed.x / 64.0, cp1_fixed.y / 64.0,
               cp2_fixed.x / 64.0, cp2_fixed.y / 64.0,
               ep_fixed.x / 64.0, ep_fixed.y / 64.0);
#endif

        m_tags.add(QT_FT_CURVE_TAG_CUBIC);         // Control point 1
        m_tags.add(QT_FT_CURVE_TAG_CUBIC);         // Control point 2
        m_tags.add(QT_FT_CURVE_TAG_ON);            // End point
    }



    QT_FT_Outline *convertPath(const QPainterPath &path)
    {
        Q_ASSERT(!path.isEmpty());

        int elmCount = path.elementCount();


#ifdef QT_DEBUG_CONVERT
        printf("QFTOutlineMapper::convertPath(), size=%d\n", elmCount);
#endif


        beginOutline(path.fillRule());

        for (int index=0; index<elmCount; ++index) {
            const QPainterPath::Element &elm = path.elementAt(index);

            switch (elm.type) {

            case QPainterPath::MoveToElement:
                if (index == elmCount - 1)
                    continue;
                moveTo(elm);
                break;

            case QPainterPath::LineToElement:
                lineTo(elm);
                break;

            case QPainterPath::CurveToElement:
                curveTo(elm, path.elementAt(index + 1), path.elementAt(index + 2));
                index += 2;
                break;

            default:
                break; // This will never hit..
            }
        }

        endOutline();

        return &m_outline;
    }
public:
    QDataBuffer<QT_FT_Vector> m_points;
    QDataBuffer<char> m_tags;
    QDataBuffer<short> m_contours;
    QT_FT_Outline m_outline;

    QT_FT_Vector m_subpath_start;

    // Matrix
    qreal m_m11;
    qreal m_m12;
    qreal m_m21;
    qreal m_m22;
    qreal m_dx;
    qreal m_dy;
};

static void qt_ft_outline_move_to(qfixed x, qfixed y, void *data)
{
    ((QFTOutlineMapper *) data)->moveTo(QPointF(qt_fixed_to_real(x), qt_fixed_to_real(y)));
}

static void qt_ft_outline_line_to(qfixed x, qfixed y, void *data)
{
    ((QFTOutlineMapper *) data)->lineTo(QPointF(qt_fixed_to_real(x), qt_fixed_to_real(y)));
}

static void qt_ft_outline_cubic_to(qfixed c1x, qfixed c1y,
                             qfixed c2x, qfixed c2y,
                             qfixed ex, qfixed ey,
                             void *data)
{
    ((QFTOutlineMapper *) data)->curveTo(QPointF(qt_fixed_to_real(c1x), qt_fixed_to_real(c1y)),
                                         QPointF(qt_fixed_to_real(c2x), qt_fixed_to_real(c2y)),
                                         QPointF(qt_fixed_to_real(ex), qt_fixed_to_real(ey)));
}


#if !defined(QT_NO_DEBUG) && 0
static void qt_debug_path(const QPainterPath &path)
{
    const char *names[] = {
        "MoveTo     ",
        "LineTo     ",
        "CurveTo    ",
        "CurveToData"
    };

    printf("\nQPainterPath: elementCount=%d\n", path.elementCount());
    for (int i=0; i<path.elementCount(); ++i) {
        const QPainterPath::Element &e = path.elementAt(i);
        Q_ASSERT(e.type >= 0 && e.type <= QPainterPath::CurveToDataElement);
        printf(" - %3d:: %s, (%.2f, %.2f)\n", i, names[e.type], e.x, e.y);
    }
}
#endif

QRasterPaintEngine::QRasterPaintEngine()
    : QPaintEngine(*(new QRasterPaintEnginePrivate),
                   QPaintEngine::PaintEngineFeatures(AllFeatures)
        )
{
    init();
}

QRasterPaintEngine::QRasterPaintEngine(QRasterPaintEnginePrivate &dd)
    : QPaintEngine(dd, QPaintEngine::PaintEngineFeatures(AllFeatures))
{
    init();
}

void QRasterPaintEngine::init()
{
    Q_D(QRasterPaintEngine);

    d->rasterBuffer = new QRasterBuffer();
#ifdef Q_WS_WIN
    d->fontRasterBuffer = new QRasterBuffer();
#endif
    d->outlineMapper = new QFTOutlineMapper;
    if (!qt_gray_raster || !qt_black_raster) {
        qt_initialize_ft();
    };

    d->dashStroker = 0;

    d->flushOnEnd = true;

    d->basicStroker.setMoveToHook(qt_ft_outline_move_to);
    d->basicStroker.setLineToHook(qt_ft_outline_line_to);
    d->basicStroker.setCubicToHook(qt_ft_outline_cubic_to);
    d->dashStroker = 0;
}


QRasterPaintEngine::~QRasterPaintEngine()
{
    Q_D(QRasterPaintEngine);

    delete d->rasterBuffer;
    delete d->outlineMapper;
#ifdef Q_WS_WIN
    delete d->fontRasterBuffer;
#endif

    delete d->dashStroker;
}


bool QRasterPaintEngine::begin(QPaintDevice *device)
{
    Q_D(QRasterPaintEngine);

    qInitDrawhelperAsm();
    d->deviceDepth = device->depth();
    d->clipEnabled = false;
    d->antialiased = false;
    d->bilinear = false;
    d->opaqueBackground = false;
    d->bgBrush = Qt::white;
    d->mono_surface = false;
    d->has_pen = true;
    d->has_brush = false;
    d->fast_pen = true;
    d->int_xform = true;

    d->stroker = &d->basicStroker;

    d->compositionMode = QPainter::CompositionMode_SourceOver;

    d->deviceRect = QRect(0, 0, device->width(), device->height());

    DrawHelper::Layout layout = DrawHelper::Layout_RGB32;

    gccaps &= ~PorterDuff;

    // reset paintevent clip
    d->baseClip = QPainterPath();
    if (device->devType() == QInternal::Widget) {
        QRegion sysClip = systemClip();
        if (!sysClip.isEmpty()) {
            d->baseClip.addRegion(sysClip);
            d->deviceRect = sysClip.boundingRect();
            // Shift the baseclip to absolute
            d->baseClip = d->baseClip * QMatrix(1, 0, 0, 1,
                                                -d->deviceRect.x(),
                                                -d->deviceRect.y());
        }
    }
#if defined(Q_WS_QWS)
    else if (device->devType() == QInternal::Pixmap) {
        // Only embedded uses system clipping on pixmaps
        QRegion sysClip = systemClip();
        if (!sysClip.isEmpty())
            d->baseClip.addRegion(sysClip);
    }
#endif
    else {
        QRegion sysClip = systemClip();
        if (!sysClip.isEmpty()) {
            d->baseClip.addRegion(sysClip);
        }
    }

    bool isBitmap = false;
#if defined(Q_WS_WIN) || defined(Q_WS_QWS)
    if (device->devType() == QInternal::Pixmap) {
        QPixmap *pixmap = static_cast<QPixmap *>(device);
        if (pixmap->isNull()) {
            qWarning("Cannot paint on a null pixmap");
            return false;
        }
        QPixmapData *data = static_cast<QPixmap *>(device)->data;
        device = &data->image;
        isBitmap = pixmap->depth() == 1;
    }
#endif

    if (device->devType() == QInternal::Image) {
        QImage *image = static_cast<QImage *>(device);
        int format = image->format();
        d->flushOnEnd = (format != QImage::Format_ARGB32_Premultiplied
                         && format != QImage::Format_RGB32
                         && !isBitmap);

        d->rasterBuffer->prepare(image);
#ifdef Q_WS_QWS
        isBitmap = image->depth() == 1; //### temporary until we have QScreenPaintDevice
#endif
        if (format == QImage::Format_MonoLSB && isBitmap) {
            d->mono_surface = true;
            layout = DrawHelper::Layout_MonoLSB;
        } else if (format == QImage::Format_RGB32) {
            layout = DrawHelper::Layout_RGB32;
        } else if (format == QImage::Format_ARGB32_Premultiplied) {
            layout = DrawHelper::Layout_ARGB;
            gccaps |= PorterDuff;
#ifdef Q_WS_QWS
        } else if (format == QImage::Format_RGB16) {
            layout = DrawHelper::Layout_RGB16;
        } else if (format == QImage::Format_Grayscale4LSB) {
            layout = DrawHelper::Layout_Gray4LSB;
#endif
        } else {
            qWarning("QRasterPaintEngine::begin(), unsupported image format (%d)\n"
                     "Supported image formats: Format_RGB32 and Format_ARGB32_Premultiplied",
                     format);
            return false;
        }
    } else {
        d->rasterBuffer->prepare(d->deviceRect.width(), d->deviceRect.height());
    }

    d->rasterBuffer->resetClip();

    d->drawHelper = qDrawHelper + layout;

    d->matrix = QMatrix();
    d->txop = QPainterPrivate::TxNone;
    d->outlineMapper->setMatrix(d->matrix, d->txop);

    if (device->depth() == 1) {
        d->pen = QPen(Qt::color1);
        d->brush = QBrush(Qt::color0);
    } else {
        d->pen = QPen(Qt::black);
        d->brush = QBrush(Qt::NoBrush);
    }

    updateClipPath(QPainterPath(), Qt::NoClip);

    setActive(true);
    return true;
}


bool QRasterPaintEngine::end()
{
    Q_D(QRasterPaintEngine);

    if (d->flushOnEnd)
        flush(d->pdev, QPoint());

    d->clipEnabled = false;

    setActive(false);

    return true;
}

void QRasterPaintEngine::releaseBuffer()
{
    Q_D(QRasterPaintEngine);
    delete d->rasterBuffer;
    d->rasterBuffer = new QRasterBuffer;
}

QSize QRasterPaintEngine::size() const
{
    Q_D(const QRasterPaintEngine);
    return QSize(d->rasterBuffer->width(), d->rasterBuffer->height());
}

#ifndef QT_NO_DEBUG
void QRasterPaintEngine::saveBuffer(const QString &s) const
{
    Q_D(const QRasterPaintEngine);
    d->rasterBuffer->bufferImage().save(s, "PNG");
}
#endif

void QRasterPaintEngine::setFlushOnEnd(bool flushOnEnd)
{
    Q_D(QRasterPaintEngine);

    d->flushOnEnd = flushOnEnd;
}


/*!
  Force the contents of the buffer out on the underlying device.
*/
void QRasterPaintEngine::flush(QPaintDevice *device, const QPoint &offset)
{
    Q_D(QRasterPaintEngine);
    Q_ASSERT(device);

#if defined(Q_WS_WIN)
    if (device->devType() == QInternal::Widget) {
        HDC hdc = device->getDC();

        Q_ASSERT(d->rasterBuffer->hdc());

        bool tmp_hdc = false;
        if (!hdc) {
            tmp_hdc = true;
            hdc = GetDC(((QWidget*) device)->winId());
        }

        QRegion sysClip = systemClip();
        if (sysClip.isEmpty()) {
            BitBlt(hdc, d->deviceRect.x() + offset.x(), d->deviceRect.y() + offset.y(),
                   d->deviceRect.width(), d->deviceRect.height(),
                   d->rasterBuffer->hdc(), 0, 0, SRCCOPY);
        } else {
            QVector<QRect> rects = sysClip.rects();
            for (int i=0; i<rects.size(); ++i) {
                QRect r = rects.at(i);
                BitBlt(hdc,
                       r.x() + offset.x(), r.y() + offset.y(), r.width(), r.height(),
                       d->rasterBuffer->hdc(), r.x() - d->deviceRect.x(), r.y() - d->deviceRect.y(),
                       SRCCOPY);
            }
        }

        if (tmp_hdc) {
            ReleaseDC(((QWidget *) device)->winId(), hdc);
        } else {
            device->releaseDC(hdc);
        }
    } else {

        QImage *target = 0;

        if (device->devType() == QInternal::Pixmap) {
            target = &static_cast<QPixmap *>(device)->data->image;
        } else if (device->devType() == QInternal::Image) {
            target = static_cast<QImage *>(device);
        }

        Q_ASSERT(target);
        Q_ASSERT(target->format() != QImage::Format_RGB32 &&
                 target->format() != QImage::Format_ARGB32_Premultiplied);

        switch (target->format()) {

        case QImage::Format_ARGB32:
            d->rasterBuffer->flushToARGBImage(target);
            break;

        default:
            qWarning("QRasterPaintEngine::flush(), unhandled case: %d", target->format());
            break;
        }
    }

#elif defined(Q_WS_MAC)
#  ifdef QMAC_NO_COREGRAPHICS
#    warning "unhandled"
#  else
    extern CGContextRef qt_macCreateCGHandle(const QPaintDevice *); //qpaintdevice_mac.cpp
    extern CGContextRef qt_mac_cg_context(const QPaintDevice *); //qpaintdevice_mac.cpp

    if(CGContextRef ctx = qt_mac_cg_context(device)) {
        CGRect rect = CGRectMake(d->deviceRect.x(), d->deviceRect.y(),
                                 d->deviceRect.width(), d->deviceRect.height());
        HIViewDrawCGImage(ctx, &rect, d->rasterBuffer->m_data); //top left
        CGContextRelease(ctx);
    }
#  endif
    Q_UNUSED(offset);
#else
    Q_UNUSED(d);
    Q_UNUSED(offset);
    Q_UNUSED(device);
#endif
}

void QRasterPaintEngine::updateMatrix(const QMatrix &matrix)
{
    Q_D(QRasterPaintEngine);

    d->matrix = matrix;
    d->int_xform = false;
    if (d->matrix.m12() != 0 || d->matrix.m21() != 0) {
        d->txop = QPainterPrivate::TxRotShear;
    } else if (d->matrix.m11() != 1 || d->matrix.m22() != 1) {
        d->txop = QPainterPrivate::TxScale;
        d->int_xform = qreal(int(d->matrix.dx())) == d->matrix.dx()
                            && qreal(int(d->matrix.dy())) == d->matrix.dy()
                            && qreal(int(d->matrix.m11())) == d->matrix.m11()
                            && qreal(int(d->matrix.m22())) == d->matrix.m22();
    } else if (d->matrix.dx() != 0 || d->matrix.dy() != 0) {
        d->txop = QPainterPrivate::TxTranslate;
        d->int_xform = qreal(int(d->matrix.dx())) == d->matrix.dx()
                            && qreal(int(d->matrix.dy())) == d->matrix.dy();
    } else {
        d->txop = QPainterPrivate::TxNone;
        d->int_xform = true;
    }
    d->outlineMapper->setMatrix(d->matrix, d->txop);
}

void QRasterPaintEngine::updateState(const QPaintEngineState &state)
{
    Q_D(QRasterPaintEngine);

    QPaintEngine::DirtyFlags flags = state.state();

    bool update_fast_pen = false;

    if (flags & DirtyTransform) {
        update_fast_pen = true;
        updateMatrix(state.matrix());
    }

    if (flags & DirtyPen) {
        update_fast_pen = true;
        d->pen = state.pen();
        Qt::PenStyle pen_style = d->pen.style();
        d->has_pen = pen_style != Qt::NoPen;
        d->basicStroker.setJoinStyle(d->pen.joinStyle());
        d->basicStroker.setCapStyle(d->pen.capStyle());
        qreal penWidth = d->pen.widthF();
        if (penWidth == 0)
            d->basicStroker.setStrokeWidth(1);
        else
            d->basicStroker.setStrokeWidth(penWidth);

        if(pen_style == Qt::SolidLine) {
            d->stroker = &d->basicStroker;
        } else if (d->has_pen) {
            if (!d->dashStroker)
                d->dashStroker = new QDashStroker(&d->basicStroker);
            d->dashStroker->setDashPattern(QDashStroker::patternForStyle(pen_style));
            d->stroker = d->dashStroker;
        } else {
            d->stroker = 0;
        }

    }

    if ((flags & DirtyBrush) || (flags & DirtyBrushOrigin)) {
        QBrush brush = state.brush();
        QPointF offset = state.brushOrigin();
        d->brush = brush;
        d->brushOffset = offset;
        d->has_brush = brush.style() != Qt::NoBrush;
    }

    if (flags & DirtyBackgroundMode) {
        d->opaqueBackground = (state.backgroundMode() == Qt::OpaqueMode);
    }

    if (flags & DirtyBackground) {
        d->bgBrush = state.backgroundBrush();
    }

    if (flags & DirtyClipPath) {
        updateClipPath(state.clipPath(), state.clipOperation());
    }

    if (flags & DirtyClipRegion) {
        updateClipRegion(state.clipRegion(), state.clipOperation());
    }

    if (!d->mono_surface) {
        if (flags & DirtyHints) {
            update_fast_pen = true;
            d->antialiased = bool(state.renderHints() & QPainter::Antialiasing);
            d->bilinear = bool(state.renderHints() & QPainter::SmoothPixmapTransform);
        }

        if (flags & DirtyCompositionMode) {
            d->compositionMode = state.compositionMode();
        }
    }

    if (update_fast_pen) {
        d->fast_pen = d->pen.style() == Qt::SolidLine && !d->antialiased
                      && (d->pen.widthF() == 0
                          || d->pen.widthF() <= 1 && d->txop <= QPainterPrivate::TxTranslate);
    }
}


void QRasterPaintEngine::updateClipRegion(const QRegion &r, Qt::ClipOperation op)
{
#ifdef QT_DEBUG_DRAW
    qDebug() << " - QRasterPaintEngine::updateClipRegion() op=" << op << r;
#endif
    QPainterPath p;
    p.addRegion(r);
    updateClipPath(p, op);
}


void QRasterPaintEngine::updateClipPath(const QPainterPath &path, Qt::ClipOperation op)
{
    Q_D(QRasterPaintEngine);
#ifdef QT_DEBUG_DRAW
    qDebug() << " - QRasterPaintEngine::updateClipPath(), op="
             << op
             << path.boundingRect();
#endif
    d->updateClip_helper(path, op);

    // Reset if baseClip if the operation it.
    if (!d->baseClip.isEmpty()) {
        switch (op) {
        case Qt::UniteClip:
        case Qt::ReplaceClip:
        case Qt::NoClip:
            d->outlineMapper->setMatrix(QMatrix(), QPainterPrivate::TxNone);
            d->updateClip_helper(d->baseClip, Qt::IntersectClip);
            d->outlineMapper->setMatrix(d->matrix, d->txop);
            break;
        default:
            break;
        }
    }
}


static QImage qt_map_to_32bit(const QPixmap &pixmap)
{
    QImage image = pixmap.toImage();
    return image.convertToFormat(image.hasAlphaChannel()
                                 ? QImage::Format_ARGB32_Premultiplied
                                 : QImage::Format_RGB32);
}

void QRasterPaintEngine::fillPath(const QPainterPath &path, QSpanFillData *fillData)
{
#ifdef QT_DEBUG_DRAW
    qDebug() << " --- fillPath, bounds=" << path.boundingRect();
#endif

    if (!fillData->blend)
        return;

    Q_D(QRasterPaintEngine);
    qt_scanconvert(d->outlineMapper->convertPath(path), fillData->blend, fillData, d);
}



void QRasterPaintEngine::drawRects(const QRect *rects, int rectCount)
{
#ifdef QT_DEBUG_DRAW
    qDebug(" - QRasterPaintEngine::drawRect(), rectCount=%d", rectCount);
#endif
    Q_D(QRasterPaintEngine);
    if (!d->antialiased && d->int_xform) {

        int offset_x = int(d->matrix.dx());
        int offset_y = int(d->matrix.dy());

        d->clippedFillForBrush(d->brush, &d->spanFillData);
        bool had_brush = d->has_brush;
        d->has_brush = false;

        const QRect *lastRect = rects + rectCount;

        while (rects < lastRect) {

            if (had_brush) {
                QRect rect = rects->normalized();
                rect.translate(offset_x, offset_y);
                int x1 = qMax(rect.x(), 0);
                int x2 = qMin(rect.width() + rect.x(), d->rasterBuffer->width());
                int y1 = qMax(rect.y(), 0);
                int y2 = qMin(rect.height() + rect.y(), d->rasterBuffer->height());;

                int len = x2 - x1;

                if (len > 0) {
                    QT_FT_Span span;

                    Q_ASSERT(d->spanFillData.blend);
                    span.x = x1;
                    span.len = x2 - x1;
                    span.coverage = 255;

                    // draw the fill
                    for (int y = y1; y < y2; ++y) {
                        span.y = y;
                        d->spanFillData.blend(1, &span, &d->spanFillData);
                    }
                }
            }

            if (d->has_pen) {
                int left = rects->x();
                int right = rects->x() + rects->width();
                int top = rects->y();
                int bottom = rects->y() + rects->height();
                QPoint pts[] = { QPoint(left, top),
                                 QPoint(right, top),
                                 QPoint(right, bottom),
                                 QPoint(left, bottom) };
                QRasterPaintEngine::drawPolygon(pts, 4, WindingMode);
            }

            ++rects;
        }
        d->has_brush = had_brush;
    } else {
        QPaintEngine::drawRects(rects, rectCount);
    }
}

void QRasterPaintEngine::drawRects(const QRectF *rects, int rectCount)
{
#ifdef QT_DEBUG_DRAW
    qDebug(" - QRasterPaintEngine::drawRect(), rectCount=%d", rectCount);
#endif
    for (int i=0; i<rectCount; ++i) {
        const QRectF &rf = rects[i];
        QPointF pts[4] = { QPointF(rf.x(), rf.y()),
                           QPointF(rf.x() + rf.width(), rf.y()),
                           QPointF(rf.x() + rf.width(), rf.y() + rf.height()),
                           QPointF(rf.x(), rf.y() + rf.height()) };
        drawPolygon(pts, 4, ConvexMode);
    }
}

void QRasterPaintEngine::drawPath(const QPainterPath &path)
{
#ifdef QT_DEBUG_DRAW
    QRectF bounds = path.boundingRect();
    qDebug(" - QRasterPaintEngine::drawPath(), [%.2f, %.2f, %.2f, %.2f]",
           bounds.x(), bounds.y(), bounds.width(), bounds.height());
#endif
    if (path.isEmpty())
        return;

    Q_D(QRasterPaintEngine);

    if (d->has_brush) {
        d->outlineMapper->setMatrix(d->matrix, d->txop);
        d->clippedFillForBrush(d->brush, &d->spanFillData);
        fillPath(path, &d->spanFillData);
    }

    if (d->has_pen) {
        Q_ASSERT(d->stroker);
        qreal width = d->pen.widthF();
        d->outlineMapper->beginOutline(Qt::WindingFill);
        if (width == 0) {
            d->outlineMapper->setMatrix(QMatrix(), QPainterPrivate::TxNone);
            d->stroker->strokePath(path, d->outlineMapper, d->matrix);
        } else {
            d->outlineMapper->setMatrix(d->matrix, d->txop);
            d->stroker->strokePath(path, d->outlineMapper, QMatrix());
        }
        d->clippedFillForBrush(QBrush(d->pen.brush()), &d->spanFillData);
        d->outlineMapper->endOutline();

        qt_scanconvert(&d->outlineMapper->m_outline, d->spanFillData.blend, &d->spanFillData, d);
        d->outlineMapper->setMatrix(d->matrix, d->txop);
    }

}


void QRasterPaintEngine::drawPolygon(const QPointF *points, int pointCount, PolygonDrawMode mode)
{
    Q_D(QRasterPaintEngine);
#ifdef QT_DEBUG_DRAW
    qDebug(" - QRasterPaintEngine::drawPolygon(), pointCount=%d", pointCount);
    for (int i=0; i<pointCount; ++i)
        qDebug() << "   - " << points[i];
#endif
    Q_ASSERT(pointCount >= 2);

    // Do the fill
    if (d->has_brush && mode != PolylineMode) {

        // Compose polygon fill..,
        d->outlineMapper->beginOutline(mode == WindingMode ? Qt::WindingFill : Qt::OddEvenFill);
        d->outlineMapper->moveTo(*points);
        const QPointF *p = points;
        const QPointF *ep = points + pointCount - 1;
        do {
            d->outlineMapper->lineTo(*(++p));
        } while (p < ep);
        d->outlineMapper->endOutline();

        // scanconvert.
        d->clippedFillForBrush(d->brush, &d->spanFillData);
        qt_scanconvert(&d->outlineMapper->m_outline, d->spanFillData.blend, &d->spanFillData, d);
    }

    // Do the outline...
    if (d->has_pen) {

        bool needs_closing = mode != PolylineMode && points[0] != points[pointCount-1];

        if (d->fast_pen) {
            // Use fast path for 0 width /  trivial pens.

            d->clippedFillForBrush(d->pen.brush(), &d->spanFillData);
            QRect devRect(0, 0, d->deviceRect.width(), d->deviceRect.height());

            LineDrawMode mode_for_last = (d->pen.capStyle() != Qt::FlatCap
                                          ? LineDrawIncludeLastPixel
                                          : LineDrawNormal);

            // Draw the all the line segments.
            for (int i=1; i<pointCount; ++i) {
                QPointF lp1 = points[i-1] * d->matrix;
                QPointF lp2 = points[i] * d->matrix;
                drawLine_midpoint_i(QLine(qFloor(lp1.x()), qFloor(lp1.y()),
                                           qFloor(lp2.x()), qFloor(lp2.y())),
                                    d->spanFillData.blend, &d->spanFillData,
                                    i == pointCount - 1 ? mode_for_last : LineDrawIncludeLastPixel,
                                    devRect);
            }

            // Polygons are implicitly closed.
            if (needs_closing) {
                QPointF lp1 = points[pointCount-1] * d->matrix;
                QPointF lp2 = points[0] * d->matrix;
                drawLine_midpoint_i(QLine(qFloor(lp1.x()), qFloor(lp1.y()),
                                          qFloor(lp2.x()), qFloor(lp2.y())),
                                    d->spanFillData.blend, &d->spanFillData, LineDrawIncludeLastPixel,
                                    devRect);
            }

        } else {
            // fallback case for complex or transformed pens.
            qreal width = d->pen.widthF();
            d->outlineMapper->beginOutline(Qt::WindingFill);
            if (width == 0) {
                d->basicStroker.setStrokeWidth(1);
                d->outlineMapper->setMatrix(QMatrix(), QPainterPrivate::TxNone);
                d->stroker->strokePolygon(points, pointCount, needs_closing,
                                          d->outlineMapper, d->matrix);
            } else {
                d->basicStroker.setStrokeWidth(width);
                d->outlineMapper->setMatrix(d->matrix, d->txop);
                d->stroker->strokePolygon(points, pointCount, needs_closing,
                                          d->outlineMapper, QMatrix());
            }
            d->clippedFillForBrush(QBrush(d->pen.brush()), &d->spanFillData);
            d->outlineMapper->endOutline();

            qt_scanconvert(&d->outlineMapper->m_outline, d->spanFillData.blend, &d->spanFillData, d);

            d->outlineMapper->setMatrix(d->matrix, d->txop);
        }
    }

}


void QRasterPaintEngine::drawPixmap(const QRectF &r, const QPixmap &pixmap, const QRectF &sr)
{
#ifdef QT_DEBUG_DRAW
    qDebug() << " - QRasterPaintEngine::drawPixmap(), r=" << r << " sr=" << sr << " pixmap=" << pixmap.size() << "depth=" << pixmap.depth();
#endif

    Q_D(QRasterPaintEngine);

    QImage image;
    if (pixmap.depth() == 1) {
        if (d->txop <= QPainterPrivate::TxTranslate
            && !d->opaqueBackground
            && r.size() == sr.size()
            && r.size() == pixmap.size()) {
            d->clippedFillForBrush(QBrush(d->pen.color()), &d->spanFillData);
            d->drawBitmap(r.topLeft() + QPointF(d->matrix.dx(), d->matrix.dy()), pixmap, &d->spanFillData);
            return;
        } else {
            image = d->colorizeBitmap(pixmap.toImage(), d->pen.color());
        }
    } else {
        image = qt_map_to_32bit(pixmap);
    }
    drawImage(r, image, sr);
}

void QRasterPaintEngine::drawImage(const QRectF &r, const QImage &img, const QRectF &sr,
                                   Qt::ImageConversionFlags)
{
#ifdef QT_DEBUG_DRAW
    qDebug() << " - QRasterPaintEngine::drawImage(), r=" << r << " sr=" << sr << " image=" << img.size() << "depth=" << img.depth();
#endif

    const QImage image = img.format() == QImage::Format_RGB32
                         ? img
                         : img.convertToFormat(QImage::Format_ARGB32_Premultiplied);

    Q_D(QRasterPaintEngine);
    QSpanFillData textureData;
    textureData.rasterBuffer = d->rasterBuffer;
    textureData.compositionMode = d->compositionMode;
    textureData.initTexture(&image);

    bool stretch_sr = r.width() != sr.width() || r.height() != sr.height();

    if (d->txop > QPainterPrivate::TxTranslate || stretch_sr) {
        textureData.blend = d->bilinear ? d->drawHelper->blendTransformedBilinear : d->drawHelper->blendTransformed;
        QMatrix copy = d->matrix;
        copy.translate(r.x(), r.y());
        if (stretch_sr)
            copy.scale(r.width() / sr.width(), r.height() / sr.height());
        copy.translate(-sr.x(), -sr.y());
        QMatrix inv = copy.inverted();
        textureData.m11 = inv.m11();
        textureData.m12 = inv.m12();
        textureData.m21 = inv.m21();
        textureData.m22 = inv.m22();
        textureData.dx = inv.dx();
        textureData.dy = inv.dy();
    } else {
        textureData.blend = d->drawHelper->blend;
        textureData.dx = -(r.x() + d->matrix.dx()) + sr.x();
        textureData.dy = -(r.y() + d->matrix.dy()) + sr.y();
    }

    QPainterPath path;
    path.addRect(r);

    d->addClip(&textureData);

    bool wasAntialiased = d->antialiased;
    d->antialiased = d->bilinear;

    fillPath(path, &textureData);

    d->antialiased = wasAntialiased;
}

void QRasterPaintEngine::drawTiledPixmap(const QRectF &r, const QPixmap &pixmap, const QPointF &sr)
{
#ifdef QT_DEBUG_DRAW
    qDebug() << " - QRasterPaintEngine::drawTiledPixmap(), r=" << r << "pixmap=" << pixmap.size();
#endif
    Q_D(QRasterPaintEngine);

    QPainterPath path;
    path.addRect(r);

    QImage image;
    if (pixmap.depth() == 1)
        image = d->colorizeBitmap(pixmap.toImage(), d->pen.color());
    else
        image = qt_map_to_32bit(pixmap);

    QSpanFillData textureData;
    textureData.rasterBuffer = d->rasterBuffer;
    textureData.compositionMode = d->compositionMode;
    textureData.initTexture(&image);

    if (d->txop > QPainterPrivate::TxTranslate) {
        textureData.blend = d->bilinear
                            ? d->drawHelper->blendTransformedBilinearTiled : d->drawHelper->blendTransformedTiled;
        QMatrix copy = d->matrix;
        copy.translate(r.x(), r.y());
        copy.translate(-sr.x(), -sr.y());
        QMatrix inv = copy.inverted();
        textureData.m11 = inv.m11();
        textureData.m12 = inv.m12();
        textureData.m21 = inv.m21();
        textureData.m22 = inv.m22();
        textureData.dx = inv.dx();
        textureData.dy = inv.dy();
    } else {
        textureData.blend = d->drawHelper->blendTiled;
        textureData.dx = -(r.x() + d->matrix.dx()) + sr.x();
        textureData.dy = -(r.y() + d->matrix.dy()) + sr.y();
    }

    d->addClip(&textureData);
    fillPath(path, &textureData);
}


#ifdef Q_WS_QWS
//QWS hack
static inline bool monoVal(const uchar* s, int x)
{
    return  (s[x>>3] << (x&7)) & 0x80;
}
void QRasterPaintEngine::alphaPenBlt(const void* src, int bpl, bool mono, int rx,int ry,int w,int h)
{
    Q_D(QRasterPaintEngine);

    // Decide on which span func to use
    d->clippedFillForBrush(d->pen.brush(), &d->spanFillData);

    if (!d->spanFillData.blend)
        return;

    int y0 = (ry < 0) ? -ry : 0;
    int x0 = (rx < 0) ? -rx : 0;

    QRasterBuffer *rb = d->rasterBuffer;

    w = qMin(w, rb->width() - rx);
    h = qMin(h, rb->height() - ry);

    const int NSPANS = 256;
    QSpan spans[NSPANS];
    int current = 0;

    for (int y=y0; y < h; ++y) {
        const uchar * const scanline = static_cast<const uchar *>(src) + y*bpl;

        if (mono) {
            for (int x = x0; x < w; ) {

                // Skip those with 0 coverage
                while (x < w && !monoVal(scanline,x))
                    ++x;
                if (x >= w) break;

                QT_FT_Span span = { x + rx, 0, y + ry, 255 };

                // extend span until we find a different one.
                while (x < w && monoVal(scanline,x))
                    ++x;
                span.len = x +rx - span.x;

                if (current == NSPANS) {
                    d->spanFillData.blend(current, spans, &d->spanFillData);
                    current = 0;
                }
                spans[current++] = span;
            }
        } else {
            for (int x = x0; x < w; ) {
                // Skip those with 0 coverage
                while (x < w && scanline[x] == 0)
                    ++x;
                if (x >= w) break;

                int prev = scanline[x];
                QT_FT_Span span = { x + rx, 0, y + ry, scanline[x] };

                // extend span until we find a different one.
                while (x < w && scanline[x] == prev)
                    ++x;
                span.len = x +rx - span.x;

                if (current == NSPANS) {
                    d->spanFillData.blend(current, spans, &d->spanFillData);
                    current = 0;
                }
                spans[current++] = span;
            }
        }
    }
//     qDebug() << "alphaPenBlt: num spans=" << current
//              << "span:" << spans->x << spans->y << spans->len << spans->coverage;
        // Call span func for current set of spans.
    if (current != 0) 
        d->spanFillData.blend(current, spans, &d->spanFillData);
}

void QRasterPaintEngine::qwsFillRect(int x, int y, int w, int h, const QBrush &brush)
{
    Q_D(QRasterPaintEngine);
    d->clippedFillForBrush(brush, &d->spanFillData);
    int x1 = qMax(x,0);
    int x2 = qMin(x+w, d->rasterBuffer->width());
    int y1 = qMax(y, 0);
    int y2 = qMin(y+h, d->rasterBuffer->height());;

    int len = x2 - x1;

    if (d->spanFillData.blend && len > 0) {
        QT_FT_Span span;
        span.x = x1;
        span.len = x2 - x1;
        span.y = y;
        span.coverage = 255;

        // draw the fill
        for (int y=y1; y<y2; ++y) {
            d->spanFillData.blend(1, &span, &d->spanFillData);
        }
    }
}
#endif

#ifdef Q_WS_X11
void QRasterPaintEngine::drawTextItem(const QPointF &p, const QTextItem &textItem)
{
    QPaintEngine::drawTextItem(p, textItem);
    return;

    const QTextItemInt &ti = static_cast<const QTextItemInt &>(textItem);

#ifdef QT_DEBUG_DRAW
    printf(" - QRasterPaintEngine::drawTextItem(), (%.2f,%.2f), string=%s\n",
           p.x(), p.y(), QString::fromRawData(ti.chars, ti.num_chars).toLatin1().data());
#endif
    Q_D(QRasterPaintEngine);

    switch(ti.fontEngine->type()) {
    case QFontEngine::Multi:
        d->drawMulti(p, ti);
        break;
    case QFontEngine::XLFD:
        d->drawXLFD(p, ti);
        break;
    case QFontEngine::Box:
        d->drawBox(p, ti);
        break;
#ifndef QT_NO_FONTCONFIG
    case QFontEngine::Freetype: {
            bool aa = d->antialiased;
            d->antialiased = !d->mono_surface;
            QPaintEngine::drawTextItem(p, ti);
            d->antialiased = aa;
        }
        break;
#endif
    default:
        Q_ASSERT(false);
    }
}

void QRasterPaintEnginePrivate::drawMulti(const QPointF &p, const QTextItem &textItem)
{
    Q_Q(QRasterPaintEngine);
    const QTextItemInt &ti = static_cast<const QTextItemInt &>(textItem);
    QFontEngineMulti *multi = static_cast<QFontEngineMulti *>(ti.fontEngine);
    QGlyphLayout *glyphs = ti.glyphs;
    int which = glyphs[0].glyph >> 24;

    qreal x = p.x();
    qreal y = p.y();

    int start = 0;
    int end, i;
    for (end = 0; end < ti.num_glyphs; ++end) {
        const int e = glyphs[end].glyph >> 24;
        if (e == which)
            continue;

        // set the high byte to zero
        for (i = start; i < end; ++i)
            glyphs[i].glyph = glyphs[i].glyph & 0xffffff;

        // draw the text
        QTextItemInt ti2 = ti;
        ti2.glyphs = ti.glyphs + start;
        ti2.num_glyphs = end - start;
        ti2.fontEngine = multi->engine(which);
        ti2.f = ti.f;
        q->drawTextItem(QPointF(x, y), ti2);

        // reset the high byte for all glyphs and advance to the next sub-string
        const int hi = which << 24;
        for (i = start; i < end; ++i) {
            glyphs[i].glyph = hi | glyphs[i].glyph;
            x += glyphs[i].advance.x();
        }

        // change engine
        start = end;
        which = e;
    }

    // set the high byte to zero
    for (i = start; i < end; ++i)
        glyphs[i].glyph = glyphs[i].glyph & 0xffffff;

    // draw the text
    QTextItemInt ti2 = ti;
    ti2.glyphs = ti.glyphs + start;
    ti2.num_glyphs = end - start;
    ti2.fontEngine = multi->engine(which);
    ti2.f = ti.f;
    q->drawTextItem(QPointF(x,y), ti2);

    // reset the high byte for all glyphs
    const int hi = which << 24;
    for (i = start; i < end; ++i)
        glyphs[i].glyph = hi | glyphs[i].glyph;
}

void QRasterPaintEnginePrivate::drawXLFD(const QPointF &p, const QTextItem &textItem)
{
    const QTextItemInt &ti = static_cast<const QTextItemInt &>(textItem);

    // xlfd: draw into bitmap, convert to image and rasterize that

    // Decide on which span func to use
    clippedFillForBrush(pen.brush(), &spanFillData);
    if (!spanFillData.blend)
        return;

    QRectF logRect(p.x(), p.y() - ti.ascent, ti.width, ti.ascent + ti.descent);
    QRect devRect = matrix.mapRect(logRect).toRect();

    if(devRect.width() == 0 || devRect.height() == 0)
        return;

    int w = qRound(ti.width);
    int h = qRound(ti.ascent + ti.descent + 1);
    QBitmap bm(w, h);
    {
        QPainter painter(&bm);
        painter.fillRect(0, 0, w, h, Qt::color0);
        painter.setPen(Qt::color1);

        QTextItemInt item;
        item.flags = 0;
        item.descent = ti.descent;
        item.ascent = ti.ascent;
        item.width = ti.width;
        item.chars = 0;
        item.num_chars = 0;
        item.glyphs = ti.glyphs;
        item.num_glyphs = ti.num_glyphs;
        item.fontEngine = ti.fontEngine;
        item.f = 0;

        painter.drawTextItem(QPointF(0, ti.ascent), item);
    }

    drawBitmap(devRect.topLeft(), bm, &spanFillData);
}

void QRasterPaintEnginePrivate::drawBox(const QPointF &, const QTextItem &)
{
    // nothing for now
}

#else

void QRasterPaintEngine::drawTextItem(const QPointF &p, const QTextItem &textItem)
{
    const QTextItemInt &ti = static_cast<const QTextItemInt &>(textItem);

#ifdef QT_DEBUG_DRAW
    printf(" - QRasterPaintEngine::drawTextItem(), (%.2f,%.2f), string=%s\n",
           p.x(), p.y(), QString::fromRawData(ti.chars, ti.num_chars).toLatin1().data());
#endif
    Q_D(QRasterPaintEngine);
#if defined(Q_WS_WIN)

    // Decide on which span func to use
    d->clippedFillForBrush(d->pen.brush(), &d->spanFillData);
    if (!d->spanFillData.blend)
        return;

    if (d->txop >= QPainterPrivate::TxScale) {
        bool antialiased = d->antialiased;
        d->antialiased = true;
        QPaintEngine::drawTextItem(p, textItem);
        d->antialiased = antialiased;
        return;
    }

    int x_buffering = ti.ascent;
    QRectF logRect(p.x(), p.y() - ti.ascent, ti.width + x_buffering, ti.ascent + ti.descent);
    QRect devRect = d->matrix.mapRect(logRect).toRect();

    if(devRect.width() == 0 || devRect.height() == 0)
        return;

    d->fontRasterBuffer->prepare(devRect.width(), devRect.height());

    if (d->mono_surface) {
        // Some extra work to get proper rasterization of text on monochrome targets

        HBITMAP bitmap = CreateBitmap(devRect.width(), devRect.height(), 1, 1, 0);
        HDC hdc = CreateCompatibleDC(qt_win_display_dc());
        HGDIOBJ null_bitmap = SelectObject(hdc, bitmap);
        SelectObject(hdc, GetStockObject(WHITE_BRUSH));
        SelectObject(hdc, GetStockObject(NULL_PEN));
        Rectangle(hdc, 0, 0, devRect.width() + 1, devRect.height() + 1);

        // Fill buffer with stuff
        qt_draw_text_item(QPoint(0, ti.ascent), ti, hdc, d);

        BitBlt(d->fontRasterBuffer->hdc(), 0, 0, devRect.width(), devRect.height(),
               hdc, 0, 0, SRCCOPY);

        SelectObject(hdc, null_bitmap);
        DeleteObject(bitmap);
        DeleteDC(hdc);
    } else {
        d->fontRasterBuffer->resetBuffer(255);

        // Fill buffer with stuff
        qt_draw_text_item(QPoint(0, ti.ascent), ti, d->fontRasterBuffer->hdc(), d);
    }

    // Boundaries
    int ymax = qMin(devRect.y() + devRect.height(), d->rasterBuffer->height());
    int ymin = qMax(devRect.y(), 0);
    int xmax = qMin(devRect.x() + devRect.width(), d->rasterBuffer->width());
    int xmin = qMax(devRect.x(), 0);

    const int NSPANS = 256;
    QSpan spans[NSPANS];
    int current = 0;

    if (d->mono_surface) {
        for (int y=ymin; y<ymax; ++y) {
            QRgb *scanline = (QRgb *) d->fontRasterBuffer->scanLine(y - devRect.y()) - devRect.x();
            // Generate spans for this y coord
            for (int x = xmin; x<xmax; ) {
                // Skip those with 0 coverage (black on white so inverted)
                while (x < xmax && qBlue(scanline[x]) > 0x80) ++x;
                if (x >= xmax) break;

                QT_FT_Span span = { x, 0, y, 255 };

                // extend span until we find a different one.
                while (x < xmax && qBlue(scanline[x]) < 0x80) ++x;
                span.len = x - span.x;
                if (current == NSPANS) {
                    d->spanFillData.blend(current, spans, &d->spanFillData);
                    current = 0;
                }
                spans[current++] = span;
            }
        }
    } else {
        for (int y=ymin; y<ymax; ++y) {
            QRgb *scanline = (QRgb *) d->fontRasterBuffer->scanLine(y - devRect.y()) - devRect.x();
            // Generate spans for this y coord
            for (int x = xmin; x<xmax; ) {
                // Skip those with 0 coverage (black on white so inverted)
                while (x < xmax && qGray(scanline[x]) == 255) ++x;
                if (x >= xmax) break;

                int prev = qGray(scanline[x]);
                QT_FT_Span span = { x, 0, y, 255 - prev };

                // extend span until we find a different one.
                while (x < xmax && qGray(scanline[x]) == prev) ++x;
                span.len = x - span.x;

                if (current == NSPANS) {
                    d->spanFillData.blend(current, spans, &d->spanFillData);
                    current = 0;
                }
                spans[current++] = span;
            }
        }
    }

    if (current != 0) 
        d->spanFillData.blend(current, spans, &d->spanFillData);

    return;

#elif defined Q_WS_QWS
    bool useFontEngine = true;
    QMatrix matrix = d->matrix;
    bool simple = matrix.m11() == 1 && matrix.m12() == 0 && matrix.m21() == 0 && matrix.m22() == 1;
    if (!simple) {
        useFontEngine = false;
        QFontEngine *fe = ti.fontEngine;
        QFontEngine::FECaps fecaps = fe->capabilites();
        useFontEngine = (fecaps == QFontEngine::FullTransformations);
        if (!useFontEngine
            && matrix.m11() == matrix.m22()
            && matrix.m12() == -matrix.m21())
            useFontEngine = (fecaps & QFontEngine::RotScale) == QFontEngine::RotScale;
    }
    if (useFontEngine) {
        ti.fontEngine->draw(this, qRound(p.x()), qRound(p.y()), ti);
        return;
    }
#endif // Q_WS_WIN

    // Fallthrough for embedded and default for mac.
    bool aa = d->antialiased;
    d->antialiased = true;
    QPaintEngine::drawTextItem(p, ti);
    d->antialiased = aa;
    return;
}
#endif


void QRasterPaintEngine::drawPoints(const QPointF *points, int pointCount)
{
    Q_D(QRasterPaintEngine);

    double pw = d->pen.widthF();

    if (d->txop > QPainterPrivate::TxTranslate || pw > 1) {
        QBrush oldBrush = d->brush;
        d->brush = Qt::NoBrush;

        const QPointF *end = points + pointCount;
        while (points < end) {
            QPainterPath path;
            path.moveTo(*points);
            path.lineTo(points->x() + 0.001, points->y());
            drawPath(path);
            ++points;
        }

        d->brush = oldBrush;

    } else {
        d->clippedFillForBrush(d->pen.brush(), &d->spanFillData);
        if (!d->spanFillData.blend)
            return;

        QT_FT_Span span = { 0, 1, 0, 255 };
        qreal dx = d->matrix.dx();
        qreal dy = d->matrix.dy();
        const QPointF *end = points + pointCount;
        int x, y;
        int left = 0;
        int right = d->deviceRect.width();
        int top = 0;
        int bottom = d->deviceRect.height();
        while (points < end) {
            x = qRound(points->x() + dx);
            y = qRound(points->y() + dy);
            if (x >= left && x < right && y >= top && y < bottom) {
                span.x = x;
                span.y = y;
                d->spanFillData.blend(1, &span, &d->spanFillData);
            }
            ++points;
        }
    }
}


void QRasterPaintEngine::drawLines(const QLine *lines, int lineCount)
{
#ifdef QT_DEBUG_DRAW
    qDebug() << " - QRasterPaintEngine::drawLine()";
#endif
    Q_D(QRasterPaintEngine);
    if (d->fast_pen) {
        QRect bounds(0, 0, d->deviceRect.width(), d->deviceRect.height());
        LineDrawMode mode = d->pen.capStyle() == Qt::FlatCap
                            ? LineDrawNormal
                            : LineDrawIncludeLastPixel;
        d->clippedFillForBrush(QBrush(d->pen.brush()), &d->spanFillData);

        for (int i=0; i<lineCount; ++i) {
            if (d->int_xform) {
                QLine l = lines[i];
                l = QLine(l.x1() * int(d->matrix.m11()) + int(d->matrix.dx()),
                          l.y1() * int(d->matrix.m22()) + int(d->matrix.dy()),
                          l.x2() * int(d->matrix.m11()) + int(d->matrix.dx()),
                          l.y2() * int(d->matrix.m22()) + int(d->matrix.dy()));

                drawLine_midpoint_i(l, d->spanFillData.blend, &d->spanFillData, mode, bounds);
            } else {
                QLineF line = lines[i] * d->matrix;
                drawLine_midpoint_i(line.toLine(), d->spanFillData.blend, &d->spanFillData, mode, bounds);
            }
        }
    } else {
        QPaintEngine::drawLines(lines, lineCount);
    }
}

void QRasterPaintEngine::drawLines(const QLineF *lines, int lineCount)
{
#ifdef QT_DEBUG_DRAW
    qDebug() << " - QRasterPaintEngine::drawLine()";
#endif
    Q_D(QRasterPaintEngine);
    if (d->fast_pen) {
        QRect bounds(0, 0, d->deviceRect.width(), d->deviceRect.height());
        LineDrawMode mode = d->pen.capStyle() == Qt::FlatCap
                            ? LineDrawNormal
                            : LineDrawIncludeLastPixel;
        for (int i=0; i<lineCount; ++i) {
            QLineF line = lines[i] * d->matrix;
            d->clippedFillForBrush(QBrush(d->pen.brush()), &d->spanFillData);
            drawLine_midpoint_i(line.toLine(), d->spanFillData.blend, &d->spanFillData, mode, bounds);
        }
    } else {
        QPaintEngine::drawLines(lines, lineCount);
    }
}

void QRasterPaintEngine::drawEllipse(const QRectF &rect)
{
    Q_D(QRasterPaintEngine);
    if (d->has_brush) {
        QPointF controlPoints[12];
        int point_count = 0;
        QPointF start = qt_curves_for_arc(rect, 0, 360, controlPoints, &point_count);

        Q_ASSERT(point_count == 12); // a perfect circle...

        d->outlineMapper->beginOutline(Qt::WindingFill);
        d->outlineMapper->moveTo(start);
        for (int i=0; i<point_count; i+=3) {
            d->outlineMapper->curveTo(controlPoints[i], controlPoints[i+1], controlPoints[i+2]);
        }
        d->outlineMapper->endOutline();

        d->clippedFillForBrush(d->brush, &d->spanFillData);
        qt_scanconvert(&d->outlineMapper->m_outline, d->spanFillData.blend, &d->spanFillData, d);
    }

    if (d->has_pen) {
        qreal width = d->pen.widthF();
        d->outlineMapper->beginOutline(Qt::WindingFill);
        if (width == 0) {
            d->outlineMapper->setMatrix(QMatrix(), QPainterPrivate::TxNone);
            d->stroker->strokeEllipse(rect, d->outlineMapper, d->matrix);
        } else {
            d->outlineMapper->setMatrix(d->matrix, d->txop);
            d->stroker->strokeEllipse(rect, d->outlineMapper, QMatrix());
        }
        d->clippedFillForBrush(QBrush(d->pen.brush()), &d->spanFillData);
        d->outlineMapper->endOutline();

        qt_scanconvert(&d->outlineMapper->m_outline, d->spanFillData.blend, &d->spanFillData, d);

        d->outlineMapper->setMatrix(d->matrix, d->txop);
    }
}

#ifdef Q_WS_WIN
HDC QRasterPaintEngine::getDC() const
{
    Q_D(const QRasterPaintEngine);
    return d->rasterBuffer->hdc();
}

void QRasterPaintEngine::releaseDC(HDC) const
{
}
#endif


QPoint QRasterPaintEngine::coordinateOffset() const
{
    Q_D(const QRasterPaintEngine);
    return QPoint(d->deviceRect.x(), d->deviceRect.y());
}

void QRasterPaintEnginePrivate::drawBitmap(const QPointF &pos, const QPixmap &pm, QSpanFillData *fg)
{
    Q_ASSERT(fg);
    if (!fg->blend)
        return;

    const QImage image = pm.toImage();
    Q_ASSERT(image.depth() == 1);

    const int spanCount = 256;
    QT_FT_Span spans[spanCount];
    int n = 0;

    // Boundaries
    int w = pm.width();
    int h = pm.height();
    int ymax = qMin(qRound(pos.y() + h), rasterBuffer->height());
    int ymin = qMax(qRound(pos.y()), 0);
    int xmax = qMin(qRound(pos.x() + w), rasterBuffer->width());
    int xmin = qMax(qRound(pos.x()), 0);

    int x_offset = xmin - qRound(pos.x());

#if defined (BITMAPS_ARE_MSB)
    QImage::Format format = image.format();
#endif
    for (int y = ymin; y < ymax; ++y) {
        const uchar *src = image.scanLine(y - qRound(pos.y()));
#if defined (BITMAPS_ARE_MSB)
        if (format == QImage::Format_MonoLSB) {
#endif
            for (int x = 0; x < xmax - xmin; ++x) {
                int src_x = x + x_offset;
                uchar pixel = src[src_x >> 3];
                if (!pixel) {
                    x += 7;
                    continue;
                }
                if (pixel & (0x1 << (src_x & 7))) {
                    QT_FT_Span span = { xmin + x, 1, y, 255 };
                    while (src_x < w-1 && src[(src_x+1) >> 3] & (0x1 << ((src_x+1) & 7))) {
                        ++src_x;
                        ++span.len;
                    }
                    x += span.len;
                    spans[n] = span;
                    ++n;
                }
                if (n == spanCount) {
                    fg->blend(n, spans, fg);
                    n = 0;
                }
            }
#if defined (BITMAPS_ARE_MSB)
        } else {
            for (int x = 0; x < xmax - xmin; ++x) {
                bool set = src[x >> 3] & (0x80 >> (x & 7));
                if (set) {
                    QT_FT_Span span = { xmin + x, 1, y, 255 };
                    while (x < w-1 && src[(x+1) >> 3] & (0x80 >> ((x+1) & 7))) {
                        ++x;
                        ++span.len;
                    }

                    spans[n] = span;
                    ++n;
                }
                if (n == spanCount) {
                    fg->blend(n, spans, fg);
                    n = 0;
                }
            }
        }
#endif
        if (n) {
            fg->blend(n, spans, fg);
            n = 0;
        }
    }
}

/* Sets up potential clipping for this FillData object.
 * Note that the data object must be valid throughout the lifetime of
 * the return value.
 */
void QRasterPaintEnginePrivate::clippedFillForBrush(const QBrush &brush, QSpanFillData *data)
{
    fillForBrush(brush, data);
    addClip(data);
}

void QRasterPaintEnginePrivate::addClip(QSpanFillData *data)
{
    if (clipEnabled && data->blend) {
        data->unclipped_blend = data->blend;
        data->blend = data->rasterBuffer->clip ? qt_span_fill_clipped : 0;
    }
}


void QRasterPaintEnginePrivate::fillForBrush(const QBrush &brush, QSpanFillData *data)
{
    Q_ASSERT(data);

    data->rasterBuffer = rasterBuffer;
    data->compositionMode = compositionMode;

    Qt::BrushStyle brushStyle = brush.style();
    switch (brushStyle) {
    case Qt::SolidPattern:
        data->solid.color = PREMUL(brush.color().rgba());
        data->blend = drawHelper->blendColor;
        break;

    case Qt::LinearGradientPattern:
        {
            const QLinearGradient *g = static_cast<const QLinearGradient *>(brush.gradient());
            data->blend = drawHelper->blendLinearGradient;
            data->gradient.alphaColor = !brush.isOpaque();
            data->initGradient(g);
            data->initMatrix(brushMatrix());

            data->gradient.linear.origin.x = g->start().x();
            data->gradient.linear.origin.y = g->start().y();
            data->gradient.linear.end.x = g->finalStop().x();
            data->gradient.linear.end.y = g->finalStop().y();
            break;
        }

    case Qt::RadialGradientPattern:
        {
            const QRadialGradient *g = static_cast<const QRadialGradient *>(brush.gradient());
            data->blend = drawHelper->blendRadialGradient;
            data->gradient.alphaColor = !brush.isOpaque();
            data->initGradient(g);
            data->initMatrix(brushMatrix());

            QPointF center = g->center();
            data->gradient.radial.center.x = center.x();
            data->gradient.radial.center.y = center.y();
            QPointF focal = g->focalPoint();
            data->gradient.radial.focal.x = focal.x();
            data->gradient.radial.focal.y = focal.y();
            data->gradient.radial.radius = g->radius();
        }
        break;

    case Qt::ConicalGradientPattern:
        {
            const QConicalGradient *g = static_cast<const QConicalGradient *>(brush.gradient());
            data->blend = drawHelper->blendConicalGradient;
            data->gradient.alphaColor = !brush.isOpaque();
            data->initGradient(g);
            data->gradient.spread = QGradient::RepeatSpread;
            data->initMatrix(brushMatrix());

            QPointF center = g->center();
            data->gradient.conical.center.x = center.x();
            data->gradient.conical.center.y = center.y();
            data->gradient.conical.angle = g->angle() * 2 * Q_PI / 360.0;
        }
        break;

    case Qt::Dense1Pattern:
    case Qt::Dense2Pattern:
    case Qt::Dense3Pattern:
    case Qt::Dense4Pattern:
    case Qt::Dense5Pattern:
    case Qt::Dense6Pattern:
    case Qt::Dense7Pattern:
    case Qt::HorPattern:
    case Qt::VerPattern:
    case Qt::CrossPattern:
    case Qt::BDiagPattern:
    case Qt::FDiagPattern:
    case Qt::DiagCrossPattern:
    case Qt::TexturePattern:
        {
            extern QPixmap qt_pixmapForBrush(int brushStyle, bool invert);
            QPixmap texture = brushStyle == Qt::TexturePattern
                              ? brush.texture() : qt_pixmapForBrush(brushStyle, true);
            if (texture.depth() == 1) {
                tempImage = colorizeBitmap(texture.toImage(), brush.color());
            } else {
                tempImage = qt_map_to_32bit(brush.texture());
            }
            data->initMatrix(brushMatrix());
            data->initTexture(&tempImage);
            if (txop > QPainterPrivate::TxTranslate) {
                data->blend = bilinear
                                     ? drawHelper->blendTransformedBilinearTiled
                                     : drawHelper->blendTransformedTiled;
            } else {
                data->blend = drawHelper->blendTiled;
            }
        }
        break;

    case Qt::NoBrush:
    default:
        data->blend = 0;
        break;
    }
}


static void qt_merge_clip(const QClipData *c1, const QClipData *c2, QClipData *result)
{
    Q_ASSERT(c1->clipSpanHeight == c2->clipSpanHeight && c1->clipSpanHeight == result->clipSpanHeight);
    
    // ### buffer overflow possible
    const int BUFFER_SIZE = 4096;
    int buffer[BUFFER_SIZE];
    int *b = buffer;
    int bsize = BUFFER_SIZE;

    for (int y = 0; y < c1->clipSpanHeight; ++y) {
        const QSpan *c1_spans = c1->clipLines[y].spans;
        int c1_count = c1->clipLines[y].count;
        const QSpan *c2_spans = c2->clipLines[y].spans;
        int c2_count = c2->clipLines[y].count;

        if (c1_count == 0 && c2_count == 0)
            continue;
        if (c1_count == 0) {
            result->appendSpans(c2_spans, c2_count);
            continue;
        } else if (c2_count == 0) {
            result->appendSpans(c1_spans, c1_count);
            continue;
        }

        // we need to merge the two

        // find required length
        int max = qMax(c1_spans[c1_count - 1].x + c1_spans[c1_count - 1].len,
                       c2_spans[c2_count - 1].x + c2_spans[c2_count - 1].len);
        if (max > bsize) {
            b = (int *)realloc(bsize == BUFFER_SIZE ? 0 : b, max*sizeof(int));
            bsize = max;
        }
        memset(buffer, 0, BUFFER_SIZE * sizeof(int));
            
        // Fill with old spans.
        for (int i = 0; i < c1_count; ++i) {
            const QSpan *cs = c1_spans + i;
            for (int j=cs->x; j<cs->x + cs->len; ++j)
                buffer[j] = cs->coverage;
        }
            
        // Fill with new spans
        for (int i = 0; i < c2_count; ++i) {
            const QSpan *cs = c2_spans + i;
            for (int j = cs->x; j < cs->x + cs->len; ++j) {
                buffer[j] += cs->coverage;
                if (buffer[j] > 255)
                    buffer[j] = 255;
            }
        }

        int x = 0;
        while (x<max) {

            // Skip to next span
            while (x < max && buffer[x] == 0) ++x;
            if (x >= max) break;

            int sx = x;
            int coverage = buffer[x];
            
            // Find length of span
            while (x < max && buffer[x] == coverage)
                ++x;

            result->appendSpan(sx, x - sx, y, coverage);
        }
    }
    if (b != buffer)
        free(b);
    result->fixup();
}


void QRasterPaintEnginePrivate::updateClip_helper(const QPainterPath &path, Qt::ClipOperation op)
{
#ifdef QT_DEBUG_DRAW
    QRectF bounds = path.boundingRect();
    qDebug() << " --- updateClip_helper(), op=" << op << ", bounds=" << bounds;
#endif
    if (op == Qt::IntersectClip && !clipEnabled)
        op = Qt::ReplaceClip;

    clipEnabled = true;

    if (op == Qt::NoClip) {
        rasterBuffer->resetClip();
        clipEnabled = false;
        return;
    }
    if (op == Qt::ReplaceClip || (op == Qt::IntersectClip && path.isEmpty())) {
        rasterBuffer->resetClip();
    }

    if (path.isEmpty())
        return;

    QClipData *newClip = new QClipData(rasterBuffer->height());
    ClipData clipData = { rasterBuffer->clip, newClip, op };
    qt_scanconvert(outlineMapper->convertPath(path), qt_span_clip, &clipData, this);
    newClip->fixup();

    if (op == Qt::UniteClip) {
        // merge clips
        QClipData *result = new QClipData(rasterBuffer->height());
        qt_merge_clip(rasterBuffer->clip, newClip, result);
        delete newClip;
        newClip = result;
    }
    
    delete rasterBuffer->clip;
    rasterBuffer->clip = newClip;
}

QImage QRasterPaintEnginePrivate::colorizeBitmap(const QImage &image, const QColor &color)
{
    Q_ASSERT(image.depth() == 1);

    QImage sourceImage = image.convertToFormat(QImage::Format_MonoLSB);
    QImage dest = QImage(sourceImage.size(), QImage::Format_ARGB32_Premultiplied);

    QRgb fg = PREMUL(color.rgba());
    QRgb bg = opaqueBackground ? PREMUL(bgBrush.color().rgba()) : 0;

    int height = sourceImage.height();
    int width = sourceImage.width();
    for (int y=0; y<height; ++y) {
        uchar *source = sourceImage.scanLine(y);
        QRgb *target = reinterpret_cast<QRgb *>(dest.scanLine(y));
        for (int x=0; x < width; ++x)
            target[x] = (source[x>>3] >> (x&7)) & 1 ? fg : bg;
    }
    return dest;
}

QRasterBuffer::~QRasterBuffer()
{
    delete clip;
    
#if defined (Q_WS_WIN)
    if (m_bitmap || m_hdc) {
        Q_ASSERT(m_hdc);
        Q_ASSERT(m_bitmap);
        DeleteObject(m_hdc);
        DeleteObject(m_bitmap);
    }
#endif
}

void QRasterBuffer::init()
{
    clip = 0;
}


void QRasterBuffer::prepare(int w, int h)
{
    if (w<=m_width && h<=m_height)
        return;

    prepareBuffer(w, h);

    m_width = w;
    m_height = h;
    bytes_per_line = 4*m_width;
}


void QRasterBuffer::prepare(QImage *image)
{
    int depth = image->depth();
    m_buffer = (uchar *)image->bits();
    m_width = image->width();
    m_height = image->height();
    bytes_per_line = 4*(depth == 32 ? m_width : (m_width*depth + 31)/32);
}

void QRasterBuffer::resetBuffer(int val)
{
    memset(m_buffer, val, m_width*m_height*sizeof(uint));
}

#if defined(Q_WS_WIN)
void QRasterBuffer::prepareBuffer(int width, int height)
{
    BITMAPINFO bmi;
    memset(&bmi, 0, sizeof(bmi));
    bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth       = width;
    bmi.bmiHeader.biHeight      = -height;
    bmi.bmiHeader.biPlanes      = 1;
    bmi.bmiHeader.biBitCount    = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biSizeImage   = 0;

    HDC displayDC = GetDC(0);

    // a little bit of cleanup...
    if (m_bitmap || m_hdc) {
        Q_ASSERT(m_hdc);
        Q_ASSERT(m_bitmap);
        DeleteObject(m_hdc);
        DeleteObject(m_bitmap);
    }

    m_hdc = CreateCompatibleDC(displayDC);
    Q_ASSERT(m_hdc);

    m_buffer = 0;
    m_bitmap = CreateDIBSection(m_hdc, &bmi, DIB_RGB_COLORS, (void**) &m_buffer, 0, 0);
    Q_ASSERT(m_bitmap);
    Q_ASSERT(m_buffer);
    GdiFlush();

    SelectObject(m_hdc, m_bitmap);
    ReleaseDC(0, displayDC);
}
#elif defined(Q_WS_X11)
void QRasterBuffer::prepareBuffer(int width, int height)
{
    delete[] m_buffer;
    m_buffer = new uchar[width*height];
    memset(m_buffer, 255, width*height*sizeof(uint));
}
#elif defined(Q_WS_MAC)
static void qt_mac_raster_data_free(void *memory, const void *, size_t)
{
    free(memory);
}

void QRasterBuffer::prepareBuffer(int width, int height)
{
    m_buffer = new uchar[width*height*sizeof(uint)];
    memset(m_buffer, 255, width*height*sizeof(uint));

#ifdef QMAC_NO_COREGRAPHICS
# warning "Unhandled!!"
#else
    if (m_data)
        CGImageRelease(m_data);
    CGColorSpaceRef colorspace = CGColorSpaceCreateDeviceRGB();
    CGDataProviderRef provider = CGDataProviderCreateWithData(m_buffer, m_buffer, width*height,
                                                              qt_mac_raster_data_free);
    m_data = CGImageCreate(width, height, 8, 32, width, colorspace,
                           kCGImageAlphaFirst, provider, 0, 0, kCGRenderingIntentDefault);
    CGColorSpaceRelease(colorspace);
    CGDataProviderRelease(provider);
#endif

}
#elif defined(Q_WS_QWS)
void QRasterBuffer::prepareBuffer(int /*width*/, int /*height*/)
{
    qFatal("QRasterBuffer::prepareBuffer not implemented on embedded");
    m_buffer = 0;
}
#endif


QClipData::QClipData(int height)
{
    clipSpanHeight = height;
    clipLines = (ClipLine *)calloc(sizeof(ClipLine), height);

    allocated = height;
    spans = (QSpan *)malloc(height*sizeof(QSpan));
    count = 0;
}

QClipData::~QClipData()
{
    free(clipLines);
    free(spans);
}

void QClipData::fixup()
{
//     qDebug("QClipData::fixup:");
    int y = -1;
    for (int i = 0; i < count; ++i) {
//         qDebug() << "    " << spans[i].x << spans[i].y << spans[i].len << spans[i].coverage;
        if (spans[i].y != y) {
            y = spans[i].y;
            clipLines[y].spans = spans+i;
//             qDebug() << "        new line: y=" << y;
        }
        ++clipLines[y].count;
    }
}


static int qt_intersect_spans(const QClipData *clip,
                              const QSpan *spans, int spanCount,
                              QSpan **outSpans, int available)
{
    QSpan *out = *outSpans;
    
    const QSpan *clipSpans;
    int clipSpanCount;
    int clipSpanIndex = 0;

    int spanIndex = 0;
    int y = -1;
    while (available && spanIndex < spanCount) {
        if (spans[spanIndex].y != y) {
            y = spans[spanIndex].y;
            clipSpans = clip->clipLines[y].spans;
            clipSpanCount = clip->clipLines[y].count;
            clipSpanIndex = 0;
        }
        if (clipSpanIndex >= clipSpanCount) {
            ++spanIndex;
            continue;
        }
        
        int sx1 = spans[spanIndex].x;
        int sx2 = sx1 + spans[spanIndex].len;
        int cx1 = clipSpans[clipSpanIndex].x;
        int cx2 = cx1 + clipSpans[clipSpanIndex].len;

        if (cx1 < sx1 && cx2 < sx1) {
            ++clipSpanIndex;
            continue;
        } else if (sx1 < cx1 && sx2 < cx1) {
            ++spanIndex;
            continue;
        }
        int x = qMax(sx1, cx1);
        int len = qMin(sx2, cx2) - x;
        if (len) {
            out->x = qMax(sx1, cx1);
            out->len = qMin(sx2, cx2) - out->x;
            out->y = y;
            out->coverage = qt_div_255(spans[spanIndex].coverage * clipSpans[clipSpanIndex].coverage);
            ++out;
            --available;
        }
        if (sx2 < cx2) {
            ++spanIndex;
        } else {
            ++clipSpanIndex;
        }
    }

    *outSpans = out;
    return spanIndex;
}

static void qt_span_fill_clipped(int spanCount, const QSpan *spans, void *userData)
{
//     qDebug() << "qt_span_fill_clipped";
    QSpanFillData *fillData = reinterpret_cast<QSpanFillData *>(userData);

    Q_ASSERT(fillData->blend && fillData->unclipped_blend);

    QRasterBuffer *rb = fillData->rasterBuffer;
    Q_ASSERT(rb->clip);

    const int NSPANS = 256;
    QSpan cspans[NSPANS];
    while (spanCount) {
        QSpan *clipped = cspans;
        int processed = qt_intersect_spans(rb->clip, spans, spanCount, &clipped, NSPANS);
//         qDebug() << "processed " << processed << "clipped" << clipped-cspans
//                  << "span:" << cspans->x << cspans->y << cspans->len << spans->coverage;

        fillData->unclipped_blend(clipped - cspans, cspans, fillData);
        spanCount -= processed;
    }
}

static void qt_span_clip(int count, const QSpan *spans, void *userData)
{
    ClipData *clipData = reinterpret_cast<ClipData *>(userData);

    switch (clipData->operation) {

    case Qt::IntersectClip:
        {
            QClipData *newClip = clipData->newClip;
            while (count) {
                QSpan *newspans = newClip->spans + newClip->count;
                int processed = qt_intersect_spans(clipData->oldClip, spans, count,
                                                   &newspans, newClip->allocated - newClip->count);
                newClip->count += newspans - (newClip->spans + newClip->count);
                count -= processed;
                if (count) {
                    newClip->allocated *= 2;
                    newClip->spans = (QSpan *)realloc(newClip->spans, newClip->allocated*sizeof(QSpan));
                }
            }
        }
        break;

    case Qt::UniteClip:
    case Qt::ReplaceClip:
        clipData->newClip->appendSpans(spans, count);
        break;
    case Qt::NoClip:
        break;
    }
}

static void qt_scanconvert(QT_FT_Outline *outline, ProcessSpans callback, void *userData,
                           QRasterPaintEnginePrivate *d)
{
    if (!callback)
        return;
    
    void *data = userData;

    QT_FT_BBox clip_box = { 0, 0, d->deviceRect.width(), d->deviceRect.height() };

    QT_FT_Raster_Params rasterParams;
    rasterParams.target = 0;
    rasterParams.source = outline;
    rasterParams.flags = QT_FT_RASTER_FLAG_CLIP;
    rasterParams.gray_spans = 0;
    rasterParams.black_spans = 0;
    rasterParams.bit_test = 0;
    rasterParams.bit_set = 0;
    rasterParams.user = data;
    rasterParams.clip_box = clip_box;

    if (d->antialiased) {
        rasterParams.flags |= (QT_FT_RASTER_FLAG_AA | QT_FT_RASTER_FLAG_DIRECT);
        rasterParams.gray_spans = callback;
        int error = qt_ft_grays_raster.raster_render(qt_gray_raster, &rasterParams);
        if (error) {
            printf("qt_scanconvert(), gray raster failed...: %d\n", error);
        }
    } else {
        rasterParams.flags |= QT_FT_RASTER_FLAG_DIRECT;
        rasterParams.black_spans = callback;
        int error = qt_ft_standard_raster.raster_render(qt_black_raster, &rasterParams);
        if (error) {
            qWarning("black raster failed to render, code=%d", error);
        }
    }

}

#ifndef QT_NO_DEBUG
QImage QRasterBuffer::clipImage() const
{
    QImage image(m_width, m_height, QImage::Format_ARGB32_Premultiplied);
    image.fill(qRgb(0, 0, 0));

    for (int y = 0; y < m_height; ++y) {
        QSpan *spans = clip->clipLines[y].spans;
        int count = clip->clipLines[y].count;

        while (count--) {
            for (int x=spans->x; x<spans->x + spans->len; ++x) {
                QRgb pixel = image.pixel(x, y);
                image.setPixel(x, y, qRgb(spans->coverage, qGreen(pixel) + 10, 0));
            }
            ++spans;
        }
    }
    return image;
}

QImage QRasterBuffer::bufferImage() const
{
    QImage image(m_width, m_height, QImage::Format_ARGB32_Premultiplied);

    for (int y = 0; y < m_height; ++y) {
        uint *span = (uint *)const_cast<QRasterBuffer *>(this)->scanLine(y);

        for (int x=0; x<m_width; ++x) {
            uint argb = span[x];
            image.setPixel(x, y, argb);
        }
    }
    return image;
}
#endif


void QRasterBuffer::flushToARGBImage(QImage *target) const
{
    int w = qMin(m_width, target->width());
    int h = qMin(m_height, target->height());

    for (int y=0; y<h; ++y) {
        uint *sourceLine = (uint *)const_cast<QRasterBuffer *>(this)->scanLine(y);
        QRgb *dest = (QRgb *) target->scanLine(y);
        for (int x=0; x<w; ++x) {
            QRgb pixel = sourceLine[x];
            int alpha = qAlpha(pixel);
            if (!alpha) {
                dest[x] = 0;
            } else {
                dest[x] = (alpha << 24)
                        | ((255*qRed(pixel)/alpha) << 16)
                        | ((255*qGreen(pixel)/alpha) << 8)
                        | ((255*qBlue(pixel)/alpha) << 0);
            }
        }
    }
}

void QSpanFillData::initMatrix(const QMatrix &matrix)
{
    QMatrix inv = matrix.inverted();
    m11 = inv.m11();
    m12 = inv.m12();
    m21 = inv.m21();
    m22 = inv.m22();
    dx = inv.dx();
    dy = inv.dy();
}

void QSpanFillData::initTexture(const QImage *image)
{
    texture.imageData = image->bits();
    texture.width = image->width();
    texture.height = image->height();
    texture.hasAlpha = image->format() != QImage::Format_RGB32;
}

void QSpanFillData::initGradient(const QGradient *g)
{
    const QGradientStops stops = g->stops();
    int stopCount = stops.count();
    Q_ASSERT(stopCount > 0);

    // The position where the gradient begins and ends
    int begin_pos = int(stops[0].first * GRADIENT_STOPTABLE_SIZE);
    int end_pos = int(stops[stopCount-1].first * GRADIENT_STOPTABLE_SIZE);

    int pos = 0; // The position in the color table.

    // Up to first point
    while (pos<=begin_pos) {
        gradient.colorTable[pos] = PREMUL(stops[0].second.rgba());
        ++pos;
    }

    qreal incr = 1 / qreal(GRADIENT_STOPTABLE_SIZE); // the double increment.
    qreal dpos = incr * pos; // The position in terms of 0-1.

    int current_stop = 0; // We always interpolate between current and current + 1.

    // Gradient area
    while (pos < end_pos) {

        Q_ASSERT(current_stop < stopCount);

        uint current_color = PREMUL(stops[current_stop].second.rgba());
        uint next_color = PREMUL(stops[current_stop+1].second.rgba());

        int dist = (int)(256*(dpos - stops[current_stop].first)
                         / (stops[current_stop+1].first - stops[current_stop].first));
        int idist = 256 - dist;

        gradient.colorTable[pos] = INTERPOLATE_PIXEL_256(current_color, idist, next_color, dist);

        ++pos;
        dpos += incr;

        if (dpos > stops[current_stop+1].first) {
            ++current_stop;
        }
    }

    // After last point
    while (pos < GRADIENT_STOPTABLE_SIZE) {
        gradient.colorTable[pos] = PREMUL(stops[stopCount-1].second.rgba());
        ++pos;
    }

    gradient.spread = g->spread();
}

#ifdef Q_WS_WIN
static void draw_text_item_win(const QPointF &pos, const QTextItemInt &ti, HDC hdc,
                               QRasterPaintEnginePrivate *d)
{
    QPointF p = pos;

    if (d->txop > QPainterPrivate::TxTranslate) {
        XFORM m;
        m.eM11 = d->matrix.m11();
        m.eM12 = d->matrix.m12();
        m.eM21 = d->matrix.m21();
        m.eM22 = d->matrix.m22();
        // Don't include the translation since it is done when we write the HDC
        // Back to the screen.
        m.eDx  = 0;
        m.eDy  = 0;
        if (!SetGraphicsMode(hdc, GM_ADVANCED))
            qErrnoWarning("QWin32PaintEngine::setNativeMatrix(), SetGraphicsMode failed");
        if (!SetWorldTransform(hdc, &m))
            qErrnoWarning("QWin32PaintEngine::setNativeMatrix(), SetWorldTransformation failed");
    }

    QFontEngine *fe = ti.fontEngine;

    SetTextAlign(hdc, TA_BASELINE);
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(0, 0, 0));

    double scale = 1.;
    int angle = 0;
    bool transform = false;
    bool has_kerning = ti.f && ti.f->kerning();

    qreal x = p.x();
    qreal y = p.y();

    if (d->txop >= QPainterPrivate::TxScale
        && !(QSysInfo::WindowsVersion & QSysInfo::WV_NT_based)) {
        // Draw rotated and sheared text on Windows 98

        // All versions can draw rotated text natively. Scaling can be done with window/viewport transformations.
        // Shearing transformations are done by QPainter.

        // rotation + scale + translation
        scale = sqrt(d->matrix.m11()*d->matrix.m22()
                      - d->matrix.m12()*d->matrix.m21());
        angle = qRound(1800*acos(d->matrix.m11()/scale)/Q_PI);
        if (d->matrix.m12() < 0)
            angle = 3600 - angle;

        transform = true;
    }

    if (ti.flags & (QTextItem::Underline|QTextItem::StrikeOut) || scale != 1. || angle) {
        LOGFONT lf = fe->logfont;
        lf.lfUnderline = (ti.flags & QTextItem::Underline);
        lf.lfStrikeOut = (ti.flags & QTextItem::StrikeOut);
        if (angle) {
            lf.lfOrientation = -angle;
            lf.lfEscapement = -angle;
        }
        if (scale != 1.) {
            lf.lfHeight = (int) (lf.lfHeight*scale);
            lf.lfWidth = (int) (lf.lfWidth*scale);
        }
        HFONT hf = QT_WA_INLINE(CreateFontIndirectW(&lf), CreateFontIndirectA((LOGFONTA*)&lf));
        SelectObject(hdc, hf);
    } else {
        SelectObject(hdc, fe->hfont);
    }

    unsigned int options =  fe->ttf ? ETO_GLYPH_INDEX : 0;

    QGlyphLayout *glyphs = ti.glyphs;

    int xo = qRound(x);

    if (!(ti.flags & QTextItem::RightToLeft)) {
        // hack to get symbol fonts working on Win95. See also QFontEngine constructor
        if (fe->useTextOutA) {
            // can only happen if !ttf
            for(int i = 0; i < ti.num_glyphs; i++) {
                QString str(QChar(glyphs->glyph));
                QByteArray cstr = str.toLocal8Bit();
                TextOutA(hdc, qRound(x + glyphs->offset.x()), qRound(y + glyphs->offset.y()),
                         cstr.data(), cstr.length());
                x += qRound(glyphs->advance.x());
                glyphs++;
            }
        } else {
            bool haveOffsets = false;
            qreal w = 0;
            for(int i = 0; i < ti.num_glyphs; i++) {
                if (glyphs[i].offset.x() != 0 || glyphs[i].offset.y() != 0 || glyphs[i].space_18d6 != 0) {
                    haveOffsets = true;
                    break;
                }
                w += glyphs[i].advance.x();
            }

            if (haveOffsets || transform || has_kerning) {
                for(int i = 0; i < ti.num_glyphs; i++) {
                    wchar_t chr = glyphs->glyph;
                    qreal xp = x + glyphs->offset.x();
                    qreal yp = y + glyphs->offset.y();
                    if (transform)
                        d->matrix.map(xp, yp, &xp, &yp);
                    ExtTextOutW(hdc, qRound(xp), qRound(yp), options, 0, &chr, 1, 0);
                    x += glyphs->advance.x() + ((qreal)glyphs->space_18d6) / 64.;
                    y += glyphs->advance.y();
                    glyphs++;
                }
            } else {
                // fast path
                QVarLengthArray<wchar_t> g(ti.num_glyphs);
                for (int i = 0; i < ti.num_glyphs; ++i)
                    g[i] = glyphs[i].glyph;
                // fast path
                ExtTextOutW(hdc,
                            qRound(x + glyphs->offset.x()),
                            qRound(y + glyphs->offset.y()),
                            options, 0, g.data(), ti.num_glyphs, 0);
                x += w;
            }
        }
    } else {
        int i = ti.num_glyphs;
        while(i--) {
            x += glyphs[i].advance.x() + ((qreal)glyphs[i].space_18d6) / 64.;
            y += glyphs[i].advance.y();
        }
        i = 0;
        while(i < ti.num_glyphs) {
            x -= glyphs[i].advance.x();
            y -= glyphs[i].advance.y();

            int xp = qRound(x+glyphs[i].offset.x());
            int yp = qRound(y+glyphs[i].offset.y());
            ExtTextOutW(hdc, xp, yp, options, 0, reinterpret_cast<wchar_t *>(&glyphs[i].glyph), 1, 0);

            if (glyphs[i].nKashidas) {
                QChar ch(0x640); // Kashida character
                QGlyphLayout g[8];
                int nglyphs = 7;
                ti.fontEngine->stringToCMap(&ch, 1, g, &nglyphs, 0);
                for (uint k = 0; k < glyphs[i].nKashidas; ++k) {
                    x -= g[0].advance.x();
                    y -= g[0].advance.y();

                    int xp = qRound(x+g[0].offset.x());
                    int yp = qRound(y+g[0].offset.y());
                    ExtTextOutW(hdc, xp, yp, options, 0, reinterpret_cast<wchar_t *>(&g[0].glyph), 1, 0);
                }
            } else {
                x -= ((qreal)glyphs[i].space_18d6) / 64;
            }
            ++i;
        }
    }

    if (ti.flags & (QTextItem::Underline|QTextItem::StrikeOut) || scale != 1. || angle)
        DeleteObject(SelectObject(hdc, fe->hfont));

    if (ti.flags & (QTextItem::Overline)) {
        int lw = qRound(fe->lineThickness());
        int yp = qRound(y - fe->ascent() - 1);
        Rectangle(hdc, xo, yp, qRound(x), yp + lw);

    }

    if (d->txop > QPainterPrivate::TxTranslate) {
        XFORM m;
        m.eM11 = m.eM22 = 1;
        m.eDx = m.eDy = m.eM12 = m.eM21 = 0;
        if (!SetWorldTransform(hdc, &m))
            qErrnoWarning("SetWorldTransformation failed");
        if (!SetGraphicsMode(hdc, GM_COMPATIBLE))
            qErrnoWarning("SetGraphicsMode failed");
    }
}

static void draw_text_item_multi(const QPointF &p, const QTextItemInt &ti, HDC hdc,
                       QRasterPaintEnginePrivate *d)
{
    QFontEngineMulti *multi = static_cast<QFontEngineMulti *>(ti.fontEngine);
    QGlyphLayout *glyphs = ti.glyphs;
    int which = glyphs[0].glyph >> 24;

    qreal x = p.x();
    qreal y = p.y();

    int start = 0;
    int end, i;
    for (end = 0; end < ti.num_glyphs; ++end) {
        const int e = glyphs[end].glyph >> 24;
        if (e == which)
            continue;

        // set the high byte to zero
        for (i = start; i < end; ++i)
            glyphs[i].glyph = glyphs[i].glyph & 0xffffff;

        // draw the text
        QTextItemInt ti2 = ti;
        ti2.glyphs = ti.glyphs + start;
        ti2.num_glyphs = end - start;
        ti2.fontEngine = multi->engine(which);
        ti2.f = ti.f;
        draw_text_item_win(QPointF(x, y), ti2, hdc, d);

        // reset the high byte for all glyphs and advance to the next sub-string
        const int hi = which << 24;
        for (i = start; i < end; ++i) {
            glyphs[i].glyph = hi | glyphs[i].glyph;
            x += glyphs[i].advance.x();
        }

        // change engine
        start = end;
        which = e;
    }

    // set the high byte to zero
    for (i = start; i < end; ++i)
        glyphs[i].glyph = glyphs[i].glyph & 0xffffff;

    // draw the text
    QTextItemInt ti2 = ti;
    ti2.glyphs = ti.glyphs + start;
    ti2.num_glyphs = end - start;
    ti2.fontEngine = multi->engine(which);
    ti2.f = ti.f;
    draw_text_item_win(QPointF(x, y), ti2, hdc, d);

    // reset the high byte for all glyphs
    const int hi = which << 24;
    for (i = start; i < end; ++i)
        glyphs[i].glyph = hi | glyphs[i].glyph;
}

static void qt_draw_text_item(const QPointF &pos, const QTextItemInt &ti, HDC hdc,
                       QRasterPaintEnginePrivate *d)
{
    if (!ti.num_glyphs)
        return;

    switch(ti.fontEngine->type()) {
    case QFontEngine::Multi:
        draw_text_item_multi(pos, ti, hdc, d);
        break;
    case QFontEngine::Win:
    default:
        draw_text_item_win(pos, ti, hdc, d);
        break;
    }
}



#endif


/*!
    \internal

    Draws a line using the floating point midpoint algorithm. The line
    \a line is already in device coords at this point.
*/

static void drawLine_midpoint_i(const QLine &line, ProcessSpans span_func, void *data,
                                LineDrawMode style, const QRect &devRect)
{
#ifdef QT_DEBUG_DRAW
    qDebug() << "   - drawLine_midpoint_i" << line;
#endif

    int x, y;
    int dx, dy, d, incrE, incrNE;

    int x1 = line.x1();
    int x2 = line.x2();
    int y1 = line.y1();
    int y2 = line.y2();

    QT_FT_Span span = { 0, 1, 0, 255 };

    dx = x2 - x1;
    dy = y2 - y1;

    // specialcase horizontal lines
    if (dy == 0) {
        if (y1 >= 0 && y1 < devRect.height()) {
            int start = qMax(0, qMin(x1, x2));
            int stop = qMax(x1, x2) + 1;
            int stop_clipped = qMin(devRect.width(), stop);
            int len = stop_clipped - start;
            if (len > 0) {
                if (style == LineDrawNormal && stop == stop_clipped)
                    len--;
                span.x = ushort(start);
                span.len = ushort(len);
                span.y = y1;
                span_func(1, &span, data);
            }
        }
        return;
    } else if (dx == 0) {
        if (x1 >= 0 && x1 < devRect.width()) {
            int start = qMax(0, qMin(y1, y2));
            int stop = qMax(y1, y2) + 1;
            int stop_clipped = qMin(devRect.height(), stop);
            if (style == LineDrawNormal && stop == stop_clipped)
                --stop;
            span.x = ushort(x1);
            span.len = 1;
            for (int i=start; i<stop_clipped; ++i) {
                span.y = i;
                span_func(1, &span, data);
            }
        }
        return;
    }


    if (qAbs(dx) >= qAbs(dy)) {       /* if x is the major axis: */

        if (x2 < x1) {  /* if coordinates are out of order */
            qt_swap_int(x1, x2);
            dx = -dx;

            qt_swap_int(y1, y2);
            dy = -dy;
        }

        if (style == LineDrawNormal)
            --x2;

        // In the loops below we increment before call the span function so
        // we need to stop one pixel before
        x2 = qMin(x2, devRect.width() - 1);

        // completly clipped, so abort
        if (x2 <= x1) {
            return;
        }

        int x = x1;
        int y = y1;

        if (x>=0 && y>=0 && y < devRect.height()) {
            Q_ASSERT(x >= 0 && y >= 0 && x < devRect.width() && y < devRect.height());
            span.x = x;
            span.y = y;
            span_func(1, &span, data);
        }

        if (y2 > y1) { // 315 -> 360 and 135 -> 180 (unit circle degrees)
            y2 = qMin(y2, devRect.height() - 1);

            incrE = dy * 2;
            d = incrE - dx;
            incrNE = (dy - dx) * 2;

            if (y > y2)
                return;

            while (x < x2) {
                if (d > 0) {
                    ++y;
                    d += incrNE;
                    if (y > y2)
                        return;
                } else {
                    d += incrE;
                }
                ++x;

                if (x < 0 || y < 0)
                    continue;

                Q_ASSERT(x<devRect.width());
                Q_ASSERT(y<devRect.height());
                span.x = x;
                span.y = y;
                span_func(1, &span, data);
            }
        } else {  // 0-45 and 180->225 (unit circle degrees)

            y1 = qMin(y1, devRect.height() - 1);

            incrE = dy * 2;
            d = incrE + dx;
            incrNE = (dy + dx) * 2;

            if (y < 0)
                return;

            while (x < x2) {
                if (d < 0) {
                    --y;
                    d += incrNE;
                    if (y < 0)
                        return;
                } else {
                    d += incrE;
                }
                ++x;

                if (x < 0 || y > y1)
                    continue;

                Q_ASSERT(x<devRect.width() && y<devRect.height());
                span.x = x;
                span.y = y;
                span_func(1, &span, data);
            }
        }

    } else {

        // if y is the major axis:

        if (y2 < y1) {      /* if coordinates are out of order */
            qt_swap_int(y1, y2);
            dy = -dy;

            qt_swap_int(x1, x2);
            dx = -dx;
        }

        if (style == LineDrawNormal)
            --y2;

        // In the loops below we increment before call the span function so
        // we need to stop one pixel before
        y2 = qMin(y2, devRect.height() - 1);

        // completly clipped, so abort
        if (y2 <= y1) {
            return;
        }

        x = x1;
        y = y1;

        if (x>=0 && y>=0 && x < devRect.width()) {
            Q_ASSERT(x >= 0 && y >= 0 && x < devRect.width() && y < devRect.height());
            span.x = x;
            span.y = y;
            span_func(1, &span, data);
        }

        if (x2 > x1) { // 90 -> 135 and 270 -> 315 (unit circle degrees)
            x2 = qMin(x2, devRect.width() - 1);
            incrE = dx * 2;
            d = incrE - dy;
            incrNE = (dx - dy) * 2;

            if (x > x2)
                return;

            while (y < y2) {
                if (d > 0) {
                    ++x;
                    d += incrNE;
                    if (x > x2)
                        return;
                } else {
                    d += incrE;
                }
                ++y;
                if (x < 0 || y < 0)
                    continue;
                Q_ASSERT(x<devRect.width() && y<devRect.height());
                span.x = x;
                span.y = y;
                span_func(1, &span, data);
            }
        } else { // 45 -> 90 and 225 -> 270 (unit circle degrees)
            x1 = qMin(x1, devRect.width() - 1);
            incrE = dx * 2;
            d = incrE + dy;
            incrNE = (dx + dy) * 2;

            if (x < 0)
                return;

            while (y < y2) {
                if (d < 0) {
                    --x;
                    d += incrNE;
                    if (x < 0)
                        return;;
                } else {
                    d += incrE;
                }
                ++y;
                if (y < 0 || x > x1)
                    continue;
                Q_ASSERT(x>=0 && x<devRect.width() && y>=0 && y<devRect.height());
                span.x = x;
                span.y = y;
                span_func(1, &span, data);
            }
        }
    }
}
