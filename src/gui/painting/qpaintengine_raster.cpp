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
#include <private/qpolygonclipper_p.h>

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
#include <limits.h>


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


#ifdef Q_WS_WIN
void qt_draw_text_item(const QPointF &point, const QTextItemInt &ti, HDC hdc,
                       bool convertToText = false);
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

enum LineDrawMode {
    LineDrawClipped,
    LineDrawNormal,
    LineDrawIncludeLastPixel
};

static void drawLine_midpoint_i(int x1, int y1, int x2, int y2, ProcessSpans span_func, QSpanData *data,
                                LineDrawMode style, const QRect &devRect);
// static void drawLine_midpoint_f(const QLineF &line, qt_span_func span_func, void *data,
//                                 LineDrawMode style, const QRect &devRect);

const int QT_RASTER_COORD_LIMIT = 50000;

struct QRasterFloatPoint {
    qreal x;
    qreal y;
};

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
    void setMatrix(const QMatrix &m, uint txop)
    {
        m_m11 = m.m11();
        m_m12 = m.m12();
        m_m21 = m.m21();
        m_m22 = m.m22();
        m_dx = m.dx();
        m_dy = m.dy();
        m_txop = txop;
    }

    void beginOutline(Qt::FillRule fillRule)
    {
#ifdef QT_DEBUG_CONVERT
        printf("QFTOutlineMapper::beginOutline rule=%d\n", fillRule);
#endif
        m_elements.reset();
        m_elements_dev.reset();
        m_element_types.reset();
        m_points.reset();
        m_tags.reset();
        m_contours.reset();
        m_outline.flags = fillRule == Qt::WindingFill
                          ? QT_FT_OUTLINE_NONE
                          : QT_FT_OUTLINE_EVEN_ODD_FILL;
        m_subpath_start = 0;
    }

    void endOutline();

    void clipElements(const QPointF *points, const QPainterPath::ElementType *types, int count);

    void convertElements(const QPointF *points, const QPainterPath::ElementType *types, int count);

    inline void moveTo(const QPointF &pt) {
#ifdef QT_DEBUG_CONVERT
        printf("QFTOutlineMapper::moveTo() (%f, %f)\n", pt.x(), pt.y());
#endif
        closeSubpath();
        m_subpath_start = m_elements.size();
        m_elements << pt;
        m_element_types << QPainterPath::MoveToElement;
    }

    inline void lineTo(const QPointF &pt) {
#ifdef QT_DEBUG_CONVERT
        printf("QFTOutlineMapper::lineTo() (%f, %f)\n", pt.x(), pt.y());
#endif
        m_elements.add(pt);
        m_element_types << QPainterPath::LineToElement;
    }

    inline void curveTo(const QPointF &cp1, const QPointF &cp2, const QPointF &ep) {
#ifdef QT_DEBUG_CONVERT
        printf("QFTOutlineMapper::curveTo() (%f, %f)\n", ep.x(), ep.y());
#endif
        m_elements << cp1 << cp2 << ep;
        m_element_types << QPainterPath::CurveToElement
                        << QPainterPath::CurveToDataElement
                        << QPainterPath::CurveToDataElement;
    }

    inline void closeSubpath() {
        int element_count = m_elements.size();
        if (element_count > 0) {
            if (m_elements.at(element_count-1) != m_elements.at(m_subpath_start)) {
#ifdef QT_DEBUG_CONVERT
                printf(" - implicitly closing\n");
#endif
                lineTo(m_elements.at(m_subpath_start));
            }
        }
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
    QDataBuffer<QPainterPath::ElementType> m_element_types;
    QDataBuffer<QPointF> m_elements;
    QDataBuffer<QPointF> m_elements_dev;
    QDataBuffer<QT_FT_Vector> m_points;
    QDataBuffer<char> m_tags;
    QDataBuffer<short> m_contours;

    QPolygonClipper<QRasterFloatPoint, QRasterFloatPoint, qreal> m_clipper;
    QDataBuffer<QPointF> m_polygon_dev;

    QT_FT_Outline m_outline;
    uint m_txop;

    int m_subpath_start;

    // Matrix
    qreal m_m11;
    qreal m_m12;
    qreal m_m21;
    qreal m_m22;
    qreal m_dx;
    qreal m_dy;
};

void QFTOutlineMapper::endOutline()
{
    closeSubpath();

    int element_count = m_elements.size();
    const QPointF *elements;

    // Transform the outline
    if (m_txop == QPainterPrivate::TxNone) {
        elements = m_elements.data();
    } else {
        if (m_txop == QPainterPrivate::TxTranslate) {
            for (int i=0; i<m_elements.size(); ++i) {
                const QPointF &e = m_elements.at(i);
                m_elements_dev << QPointF(e.x() + m_dx, e.y() + m_dy);
            }
        } else if (m_txop == QPainterPrivate::TxScale) {
            for (int i=0; i<m_elements.size(); ++i) {
                const QPointF &e = m_elements.at(i);
                m_elements_dev << QPointF(m_m11 * e.x() + m_dx, m_m22 * e.y() + m_dy);
            }
        } else {
            for (int i=0; i<m_elements.size(); ++i) {
                const QPointF &e = m_elements.at(i);
                m_elements_dev << QPointF(m_m11 * e.x() + m_m21 * e.y() + m_dx,
                                          m_m22 * e.y() + m_m12 * e.x() + m_dy);
            }
        }
        elements = m_elements_dev.data();
    }

    // Check for out of dev bounds...
    const QPointF *last_element = elements + element_count;
    const QPointF *e = elements;
    bool do_clip = false;
    while (e < last_element) {
        if (e->x() < -QT_RASTER_COORD_LIMIT
            || e->x() > QT_RASTER_COORD_LIMIT
            || e->y() < -QT_RASTER_COORD_LIMIT
            || e->y() > QT_RASTER_COORD_LIMIT) {
            do_clip = true;
            break;
        }
        ++e;
    }

    if (do_clip) {
        clipElements(elements, m_element_types.data(), element_count);
    } else {
        convertElements(elements, m_element_types.data(), element_count);
    }
}

void QFTOutlineMapper::convertElements(const QPointF *elements,
                                       const QPainterPath::ElementType *types,
                                       int element_count)
{
    // Translate into FT coords
    const QPointF *e = elements;
    for (int i=0; i<element_count; ++i) {
        switch (*types) {
        case QPainterPath::MoveToElement:
            {
                QT_FT_Vector pt_fixed = { qreal_to_fixed_26_6(e->x()),
                                          qreal_to_fixed_26_6(e->y()) };
                if (i != 0)
                    m_contours << m_points.size() - 1;
                m_points << pt_fixed;
                m_tags <<  QT_FT_CURVE_TAG_ON;
            }
            break;

        case QPainterPath::LineToElement:
            {
                QT_FT_Vector pt_fixed = { qreal_to_fixed_26_6(e->x()),
                                          qreal_to_fixed_26_6(e->y()) };
                m_points << pt_fixed;
                m_tags << QT_FT_CURVE_TAG_ON;
            }
            break;

        case QPainterPath::CurveToElement:
            {
                QT_FT_Vector cp1_fixed = { qreal_to_fixed_26_6(e->x()),
                                           qreal_to_fixed_26_6(e->y()) };
                ++e;
                QT_FT_Vector cp2_fixed = { qreal_to_fixed_26_6((e)->x()),
                                           qreal_to_fixed_26_6((e)->y()) };
                ++e;
                QT_FT_Vector ep_fixed = { qreal_to_fixed_26_6((e)->x()),
                                          qreal_to_fixed_26_6((e)->y()) };

                m_points << cp1_fixed << cp2_fixed << ep_fixed;
                m_tags << QT_FT_CURVE_TAG_CUBIC
                       << QT_FT_CURVE_TAG_CUBIC
                       << QT_FT_CURVE_TAG_ON;

                types += 2;
                i += 2;
            }
            break;
        }
        ++types;
        ++e;
    }

    // close the very last subpath
    m_contours << m_points.size() - 1;

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

void QFTOutlineMapper::clipElements(const QPointF *elements,
                                    const QPainterPath::ElementType *types,
                                    int element_count)
{
    // We could save a bit of time by actually implementing them fully
    // instead of going through convenience functionallity, but since
    // this part of code hardly every used, it shouldn't matter.

    QPainterPath path;
    for (int i=0; i<element_count; ++i) {
        switch (types[i]) {
        case QPainterPath::MoveToElement:
            path.moveTo(elements[i]);
            break;

        case QPainterPath::LineToElement:
            path.lineTo(elements[i]);
            break;

        case QPainterPath::CurveToElement:
            path.cubicTo(elements[i], elements[i+1], elements[i+2]);
            i += 2;
            break;
        }
    }

    QPolygonF polygon = path.toFillPolygon();
    QPointF *clipped_points;
    int clipped_count;

    m_clipper.clipPolygon((QRasterFloatPoint *) polygon.constData(), polygon.size(),
                          ((QRasterFloatPoint **) &clipped_points), &clipped_count, true);

#ifdef QT_DEBUG_CONVERT
    printf(" - shape was clipped\n");
    for (int i=0; i<clipped_count; ++i) {
        printf("   - %.2f, -%.2f\n", clipped_points[i].x(), clipped_points[i].y());
    }
#endif

    if (!clipped_count)
        return;

    QPainterPath::ElementType *point_types = new QPainterPath::ElementType[clipped_count];
    point_types[0] = QPainterPath::MoveToElement;
    for (int i=0; i<clipped_count; ++i) point_types[i] = QPainterPath::LineToElement;
    convertElements(clipped_points, point_types, clipped_count);
}


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

    d->rasterPoolSize = 255 * 255;
    d->rasterPoolBase =
#if defined(Q_WS_WIN64)
        (unsigned char *) _aligned_malloc(d->rasterPoolSize, __alignof(void*));
#else
        (unsigned char *) malloc(d->rasterPoolSize);
#endif

    // The antialiasing raster.
    d->grayRaster = new QT_FT_Raster;
    qt_ft_grays_raster.raster_new(0, d->grayRaster);
    qt_ft_grays_raster.raster_reset(*d->grayRaster, d->rasterPoolBase, d->rasterPoolSize);

    // Initialize the standard raster.
    d->blackRaster = new QT_FT_Raster;
    qt_ft_standard_raster.raster_new(0, d->blackRaster);
    qt_ft_standard_raster.raster_reset(*d->blackRaster, d->rasterPoolBase, d->rasterPoolSize);

    d->rasterBuffer = new QRasterBuffer();
#ifdef Q_WS_WIN
    d->fontRasterBuffer = new QRasterBuffer();
#endif
    d->outlineMapper = new QFTOutlineMapper;

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

#if defined(Q_WS_WIN64)
    _aligned_free(d->rasterPoolBase);
#else
    free(d->rasterPoolBase);
#endif

    qt_ft_grays_raster.raster_done(*d->grayRaster);
    delete d->grayRaster;

    qt_ft_standard_raster.raster_done(*d->blackRaster);
    delete d->blackRaster;


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

    // ####### move to QApp
    qInitDrawhelperAsm();
    d->deviceDepth = device->depth();
    d->antialiased = false;
    d->bilinear = false;
    d->mono_surface = false;
    d->fast_pen = true;
    d->int_xform = true;

    d->rasterBuffer->init();

    d->deviceRect = QRect(0, 0, device->width(), device->height());


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
        if (format == QImage::Format_MonoLSB) {
            d->mono_surface = true;
        } else if (format == QImage::Format_Mono) {
            d->mono_surface = true;
        } else if (format == QImage::Format_RGB32) {
            ;
        } else if (format == QImage::Format_ARGB32_Premultiplied) {
            gccaps |= PorterDuff;
#ifdef Q_WS_QWS
        } else if (format == QImage::Format_RGB16) {
            ;
        } else if (format == QImage::Format_Grayscale4LSB) {
            ;
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

    d->matrix = QMatrix();
    d->txop = QPainterPrivate::TxNone;

    d->outlineMapper->setMatrix(d->matrix, d->txop);
    d->outlineMapper->m_clipper.setBoundingRect(d->deviceRect.adjusted(-10, -10, 10, 10));

    if (device->depth() == 1) {
        d->pen = QPen(Qt::color1);
        d->brush = QBrush(Qt::color0);
    } else {
        d->pen = QPen(Qt::black);
        d->brush = QBrush(Qt::NoBrush);
    }

    d->penData.init(d->rasterBuffer);
    d->penData.setup(d->pen.brush());
    d->stroker = &d->basicStroker;

    d->brushData.init(d->rasterBuffer);
    d->brushData.setup(d->brush);

    updateClipPath(QPainterPath(), Qt::NoClip);

    setDirty(DirtyBrushOrigin);

    setActive(true);
    return true;
}


bool QRasterPaintEngine::end()
{
    Q_D(QRasterPaintEngine);

    if (d->flushOnEnd)
        flush(d->pdev, QPoint());

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
    d->penData.setupMatrix(d->matrix, d->txop, d->bilinear);
    d->brushData.setupMatrix(d->brushMatrix(), d->txop, d->bilinear);
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
        d->basicStroker.setJoinStyle(d->pen.joinStyle());
        d->basicStroker.setCapStyle(d->pen.capStyle());
        d->basicStroker.setMiterLimit(d->pen.miterLimit());
        qreal penWidth = d->pen.widthF();
        if (penWidth == 0)
            d->basicStroker.setStrokeWidth(1);
        else
            d->basicStroker.setStrokeWidth(penWidth);

        Qt::PenStyle pen_style = d->pen.style();
        if(pen_style == Qt::SolidLine) {
            d->stroker = &d->basicStroker;
        } else if (pen_style != Qt::NoPen) {
            if (!d->dashStroker)
                d->dashStroker = new QDashStroker(&d->basicStroker);
            d->dashStroker->setDashPattern(d->pen.dashPattern());
            d->stroker = d->dashStroker;
        } else {
            d->stroker = 0;
        }
        d->penData.setup(pen_style == Qt::NoPen ? QBrush() : d->pen.brush());
    }

    if (flags & DirtyBrush) {
        QBrush brush = state.brush();
        d->brush = brush;
        d->brushData.setup(d->brush);
    }

    if (flags & DirtyBrushOrigin) {
        d->brushOffset = state.brushOrigin();
        d->brushData.setupMatrix(d->brushMatrix(), d->txop, d->bilinear);
    }

    if (flags & DirtyBackgroundMode) {
        d->rasterBuffer->opaqueBackground = (state.backgroundMode() == Qt::OpaqueMode);
    }

    if (flags & DirtyBackground) {
        d->rasterBuffer->bgBrush = state.backgroundBrush();
    }

    if (flags & DirtyClipEnabled) {
        d->rasterBuffer->clipEnabled = state.isClipEnabled();
        d->penData.adjustSpanMethods();
        d->brushData.adjustSpanMethods();
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
            // propegate state to data's
            d->brushData.bilinear = d->penData.bilinear = d->bilinear;
            d->penData.adjustSpanMethods();
            d->brushData.adjustSpanMethods();

        }

        if (flags & DirtyCompositionMode) {
            d->rasterBuffer->compositionMode = state.compositionMode();
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

    // Reset the baseClip if the operation requires it.
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

void QRasterPaintEngine::fillPath(const QPainterPath &path, QSpanData *fillData)
{
#ifdef QT_DEBUG_DRAW
    qDebug() << " --- fillPath, bounds=" << path.boundingRect();
#endif

    if (!fillData->blend)
        return;

    Q_D(QRasterPaintEngine);
    d->rasterize(d->outlineMapper->convertPath(path), fillData->blend, fillData, d->rasterBuffer);
}

static void fillRect(const QRect &r, QSpanData *data)
{
    QRect rect = r.normalized();
    int x1 = qMax(rect.x(), 0);
    int x2 = qMin(rect.width() + rect.x(), data->rasterBuffer->width());
    int y1 = qMax(rect.y(), 0);
    int y2 = qMin(rect.height() + rect.y(), data->rasterBuffer->height());
    QClipData *clip = data->rasterBuffer->clipEnabled ? data->rasterBuffer->clip : 0;
    if (clip) {
        x1 = qMax(x1, clip->xmin);
        x2 = qMin(x2, clip->xmax);
        y1 = qMax(y1, clip->ymin);
        y2 = qMin(y2, clip->ymax);
    }

    int len = x2 - x1;

    if (len > 0) {
        const int nspans = 256;
        QT_FT_Span spans[nspans];

        Q_ASSERT(data->blend);
        int y = y1;
        while (y < y2) {
            int n = qMin(nspans, y2 - y);
            int i = 0;
            while (i < n) {
                spans[i].x = x1;
                spans[i].len = len;
                spans[i].y = y + i;
                spans[i].coverage = 255;
                ++i;
            }
            data->blend(n, spans, data);
            y += n;
        }
    }
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

        const QRect *lastRect = rects + rectCount;

        while (rects < lastRect) {

            if (d->brushData.blend) {
                QRect r = *rects;
                r.translate(offset_x, offset_y);
                fillRect(r, &d->brushData);
            }

            if (d->penData.blend) {
                ProcessSpans brush_blend = d->brushData.blend;
                d->brushData.blend = 0;
                int left = rects->x();
                int right = rects->x() + rects->width();
                int top = rects->y();
                int bottom = rects->y() + rects->height();
                QPoint pts[] = { QPoint(left, top),
                                 QPoint(right, top),
                                 QPoint(right, bottom),
                                 QPoint(left, bottom) };
                QRasterPaintEngine::drawPolygon(pts, 4, WindingMode);
                d->brushData.blend = brush_blend;
            }

            ++rects;
        }
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

    if (d->brushData.blend) {
        d->outlineMapper->setMatrix(d->matrix, d->txop);
        fillPath(path, &d->brushData);
    }

    if (d->penData.blend) {
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
        d->outlineMapper->endOutline();

        d->rasterize(&d->outlineMapper->m_outline, d->penData.blend, &d->penData, d->rasterBuffer);
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
    if (d->brushData.blend && mode != PolylineMode) {

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
        d->rasterize(&d->outlineMapper->m_outline, d->brushData.blend, &d->brushData, d->rasterBuffer);
    }

    // Do the outline...
    if (d->penData.blend) {

        bool needs_closing = mode != PolylineMode && points[0] != points[pointCount-1];

        if (d->fast_pen) {
            // Use fast path for 0 width /  trivial pens.

            QRect devRect(0, 0, d->deviceRect.width(), d->deviceRect.height());

            LineDrawMode mode_for_last = (d->pen.capStyle() != Qt::FlatCap
                                          ? LineDrawIncludeLastPixel
                                          : LineDrawNormal);

            // Draw the all the line segments.
            for (int i=1; i<pointCount; ++i) {
                QPointF lp1 = points[i-1] * d->matrix;
                QPointF lp2 = points[i] * d->matrix;
                drawLine_midpoint_i(qFloor(lp1.x()), qFloor(lp1.y()),
                                    qFloor(lp2.x()), qFloor(lp2.y()),
                                    d->penData.blend, &d->penData,
                                    i == pointCount - 1 ? mode_for_last : LineDrawIncludeLastPixel,
                                    devRect);
            }

            // Polygons are implicitly closed.
            if (needs_closing) {
                QPointF lp1 = points[pointCount-1] * d->matrix;
                QPointF lp2 = points[0] * d->matrix;
                drawLine_midpoint_i(qFloor(lp1.x()), qFloor(lp1.y()),
                                    qFloor(lp2.x()), qFloor(lp2.y()),
                                    d->penData.blend, &d->penData, LineDrawIncludeLastPixel,
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
            d->outlineMapper->endOutline();

            d->rasterize(&d->outlineMapper->m_outline, d->penData.blend, &d->penData, d->rasterBuffer);

            d->outlineMapper->setMatrix(d->matrix, d->txop);
        }
    }

}

void QRasterPaintEngine::drawPolygon(const QPoint *points, int pointCount, PolygonDrawMode mode)
{
    Q_D(QRasterPaintEngine);
    if (1 || !(d->int_xform && d->fast_pen)) {
        // this calls the float version
        QPaintEngine::drawPolygon(points, pointCount, mode);
        return;
    }

#ifdef QT_DEBUG_DRAW
    qDebug(" - QRasterPaintEngine::drawPolygon(), pointCount=%d", pointCount);
    for (int i=0; i<pointCount; ++i)
        qDebug() << "   - " << points[i];
#endif
    Q_ASSERT(pointCount >= 2);

    // Do the fill
    if (d->brushData.blend && mode != PolylineMode) {

        // Compose polygon fill..,
        d->outlineMapper->beginOutline(mode == WindingMode ? Qt::WindingFill : Qt::OddEvenFill);
        d->outlineMapper->moveTo(*points);
        const QPoint *p = points;
        const QPoint *ep = points + pointCount - 1;
        do {
            d->outlineMapper->lineTo(*(++p));
        } while (p < ep);
        d->outlineMapper->endOutline();

        // scanconvert.
        d->rasterize(&d->outlineMapper->m_outline, d->brushData.blend, &d->brushData, d->rasterBuffer);
    }

    // Do the outline...
    if (d->penData.blend) {

        bool needs_closing = mode != PolylineMode && points[0] != points[pointCount-1];

        QRect devRect(0, 0, d->deviceRect.width(), d->deviceRect.height());

        LineDrawMode mode_for_last = (d->pen.capStyle() != Qt::FlatCap
                                      ? LineDrawIncludeLastPixel
                                      : LineDrawNormal);

        int dx = int(d->matrix.dx());
        int dy = int(d->matrix.dy());

        // Draw the all the line segments.
        for (int i=1; i<pointCount; ++i) {
            drawLine_midpoint_i(points[i-1].x() + dx, points[i-1].y() + dy,
                                points[i].x() + dx, points[i].y() + dy,
                                d->penData.blend, &d->penData,
                                i == pointCount - 1 ? mode_for_last : LineDrawIncludeLastPixel,
                                devRect);
        }

        // Polygons are implicitly closed.
        if (needs_closing) {
            drawLine_midpoint_i(points[pointCount-1].x() + dx, points[pointCount-1].y() + dy,
                                points[0].x() + dx, points[0].y() + dy,
                                d->penData.blend, &d->penData, LineDrawIncludeLastPixel,
                                devRect);
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
            && !d->rasterBuffer->opaqueBackground
            && r.size() == sr.size()
            && r.size() == pixmap.size()) {
            d->drawBitmap(r.topLeft() + QPointF(d->matrix.dx(), d->matrix.dy()), pixmap, &d->penData);
            return;
        } else {
            image = d->rasterBuffer->colorizeBitmap(pixmap.toImage(), d->pen.color());
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
    QSpanData textureData;
    textureData.init(d->rasterBuffer);
    textureData.type = QSpanData::Texture;
    textureData.initTexture(&image);

    bool stretch_sr = r.width() != sr.width() || r.height() != sr.height();

    if (d->txop > QPainterPrivate::TxTranslate || stretch_sr) {
        QMatrix copy = d->matrix;
        copy.translate(r.x(), r.y());
        if (stretch_sr)
            copy.scale(r.width() / sr.width(), r.height() / sr.height());
        copy.translate(-sr.x(), -sr.y());
        textureData.setupMatrix(copy, QPainterPrivate::TxRotShear, d->bilinear);

        bool wasAntialiased = d->antialiased;
        if (!d->antialiased)
            d->antialiased = d->bilinear;
        QPainterPath path;
        path.addRect(r);
        fillPath(path, &textureData);
        d->antialiased = wasAntialiased;
    } else {
        textureData.blend = d->rasterBuffer->drawHelper->blend;
        textureData.dx = -(r.x() + d->matrix.dx()) + sr.x();
        textureData.dy = -(r.y() + d->matrix.dy()) + sr.y();

        QRectF rr = r;
        rr.translate(d->matrix.dx(), d->matrix.dy());
        fillRect(rr.toRect(), &textureData);
    }
}

void QRasterPaintEngine::drawTiledPixmap(const QRectF &r, const QPixmap &pixmap, const QPointF &sr)
{
#ifdef QT_DEBUG_DRAW
    qDebug() << " - QRasterPaintEngine::drawTiledPixmap(), r=" << r << "pixmap=" << pixmap.size();
#endif
    Q_D(QRasterPaintEngine);

    QImage image;
    if (pixmap.depth() == 1)
        image = d->rasterBuffer->colorizeBitmap(pixmap.toImage(), d->pen.color());
    else
        image = qt_map_to_32bit(pixmap);

    QSpanData textureData;
    textureData.init(d->rasterBuffer);
    textureData.type = QSpanData::TiledTexture;
    textureData.initTexture(&image);

    if (d->txop > QPainterPrivate::TxTranslate) {
        QMatrix copy = d->matrix;
        copy.translate(r.x(), r.y());
        copy.translate(-sr.x(), -sr.y());
        textureData.setupMatrix(copy, QPainterPrivate::TxRotShear, d->bilinear);

        bool wasAntialiased = d->antialiased;
        if (!d->antialiased)
            d->antialiased = d->bilinear;
        QPainterPath path;
        path.addRect(r);
        fillPath(path, &textureData);
        d->antialiased = wasAntialiased;
    } else {
        textureData.blend = d->rasterBuffer->drawHelper->blendTiled;
        textureData.dx = -(r.x() + d->matrix.dx()) + sr.x();
        textureData.dy = -(r.y() + d->matrix.dy()) + sr.y();

        QRectF rr = r;
        rr.translate(d->matrix.dx(), d->matrix.dy());
        fillRect(rr.toRect(), &textureData);
    }

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

    if (!d->penData.blend)
        return;

    int y0 = (ry < 0) ? -ry : 0;
    int x0 = (rx < 0) ? -rx : 0;

    QRasterBuffer *rb = d->rasterBuffer;

    w = qMin(w, rb->width() - rx);
    h = qMin(h, rb->height() - ry);

    const int NSPANS = 256;
    QSpan spans[NSPANS];
    int current = 0;

    const uchar * scanline = static_cast<const uchar *>(src) + y0*bpl;
    if (mono) {
        for (int y=y0; y < h; ++y) {
            for (int x = x0; x < w; ) {
                if (!monoVal(scanline, x)) {
                    ++x;
                    continue;
                }

                if (current == NSPANS) {
                    d->penData.blend(current, spans, &d->penData);
                    current = 0;
                }
                spans[current].x = x + rx;
                spans[current].y = y + ry;
                spans[current].coverage = 255;
                int len = 1;
                ++x;
                // extend span until we find a different one.
                while (x < w && monoVal(scanline, x)) {
                    ++x;
                    ++len;
                }
                spans[current].len = len;
                ++current;
            }
            scanline += bpl;
        }
    } else {
        for (int y=y0; y < h; ++y) {
            for (int x = x0; x < w; ) {
                // Skip those with 0 coverage
                if (scanline[x] == 0) {
                    ++x;
                    continue;
                }

                if (current == NSPANS) {
                    d->penData.blend(current, spans, &d->penData);
                    current = 0;
                }
                int coverage = scanline[x];
                spans[current].x = x + rx;
                spans[current].y = y + ry;
                spans[current].coverage = coverage;
                int len = 1;
                ++x;

                // extend span until we find a different one.
                while (x < w && scanline[x] == coverage) {
                    ++x;
                    ++len;
                }
                spans[current].len = len;
                ++current;
            }
            scanline += bpl;
        }
    }
//     qDebug() << "alphaPenBlt: num spans=" << current
//              << "span:" << spans->x << spans->y << spans->len << spans->coverage;
        // Call span func for current set of spans.
    if (current != 0)
        d->penData.blend(current, spans, &d->penData);
}

/* Fills a rect with the current pen */
void QRasterPaintEngine::qwsFillRect(int x, int y, int w, int h)
{
    Q_D(QRasterPaintEngine);
    int x1 = qMax(x,0);
    int x2 = qMin(x+w, d->rasterBuffer->width());
    int y1 = qMax(y, 0);
    int y2 = qMin(y+h, d->rasterBuffer->height());;

    int len = x2 - x1;

    if (d->penData.blend && len > 0) {
        QT_FT_Span span;
        span.x = x1;
        span.len = x2 - x1;
        span.y = y;
        span.coverage = 255;

        // draw the fill
        for (int y=y1; y<y2; ++y) {
            d->penData.blend(1, &span, &d->penData);
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

        QFixed xadd;
        // reset the high byte for all glyphs and advance to the next sub-string
        const int hi = which << 24;
        for (i = start; i < end; ++i) {
            glyphs[i].glyph = hi | glyphs[i].glyph;
            xadd += glyphs[i].advance.x;
        }
        x += xadd.toReal();

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

    if (!penData.blend)
        return;

    QRectF logRect(p.x(), p.y() - ti.ascent.toReal(), ti.width.toReal(), (ti.ascent + ti.descent).toReal());
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

        painter.drawTextItem(QPointF(0, ti.ascent.toReal()), item);
    }

    drawBitmap(devRect.topLeft(), bm, &penData);
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

    if (!d->penData.blend)
        return;

    if (d->txop >= QPainterPrivate::TxScale) {
        bool antialiased = d->antialiased;
        d->antialiased = true;
        QPaintEngine::drawTextItem(p, textItem);
        d->antialiased = antialiased;
        return;
    }

    QFixed x_buffering = ti.ascent;
    QRectF logRect(p.x(), p.y() - ti.ascent.toReal(), (ti.width + x_buffering).toReal(),
		    (ti.ascent + ti.descent).toReal());
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
        qt_draw_text_item(QPoint(0, ti.ascent.toInt()), ti, hdc);

        BitBlt(d->fontRasterBuffer->hdc(), 0, 0, devRect.width(), devRect.height(),
               hdc, 0, 0, SRCCOPY);

        SelectObject(hdc, null_bitmap);
        DeleteObject(bitmap);
        DeleteDC(hdc);
    } else {
        d->fontRasterBuffer->resetBuffer(255);

        // Fill buffer with stuff
        qt_draw_text_item(QPoint(0, ti.ascent.toInt()), ti, d->fontRasterBuffer->hdc());
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
                    d->penData.blend(current, spans, &d->penData);
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
                    d->penData.blend(current, spans, &d->penData);
                    current = 0;
                }
                spans[current++] = span;
            }
        }
    }

    if (current != 0)
        d->penData.blend(current, spans, &d->penData);

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
        if (!d->penData.blend)
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
                d->penData.blend(1, &span, &d->penData);
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

        int m11 = int(d->matrix.m11());
        int m22 = int(d->matrix.m22());
        int dx = int(d->matrix.dx());
        int dy = int(d->matrix.dy());
        for (int i=0; i<lineCount; ++i) {
            if (d->int_xform) {
                const QLine &l = lines[i];
                int x1 = l.x1() * m11 + dx;
                int y1 = l.y1() * m22 + dy;
                int x2 = l.x2() * m11 + dx;
                int y2 = l.y2() * m22 + dy;

                drawLine_midpoint_i(x1, y1, x2, y2, d->penData.blend, &d->penData, mode, bounds);
            } else {
                QLineF line = lines[i] * d->matrix;
                drawLine_midpoint_i(qRound(line.x1()), qRound(line.y1()),
                                    qRound(line.x2()), qRound(line.y2()),
                                    d->penData.blend, &d->penData, mode, bounds);
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
            drawLine_midpoint_i(qRound(line.x1()), qRound(line.y1()),
                                qRound(line.x2()), qRound(line.y2()),
                                d->penData.blend, &d->penData, mode, bounds);
        }
    } else {
        QPaintEngine::drawLines(lines, lineCount);
    }
}

void QRasterPaintEngine::drawEllipse(const QRectF &rect)
{
    Q_D(QRasterPaintEngine);
    if (d->brushData.blend) {
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

        d->rasterize(&d->outlineMapper->m_outline, d->brushData.blend, &d->brushData, d->rasterBuffer);
    }

    if (d->penData.blend) {
        qreal width = d->pen.widthF();
        d->outlineMapper->beginOutline(Qt::WindingFill);
        if (width == 0) {
            d->outlineMapper->setMatrix(QMatrix(), QPainterPrivate::TxNone);
            d->stroker->strokeEllipse(rect, d->outlineMapper, d->matrix);
        } else {
            d->outlineMapper->setMatrix(d->matrix, d->txop);
            d->stroker->strokeEllipse(rect, d->outlineMapper, QMatrix());
        }
        d->outlineMapper->endOutline();

        d->rasterize(&d->outlineMapper->m_outline, d->penData.blend, &d->penData, d->rasterBuffer);

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

void QRasterPaintEnginePrivate::drawBitmap(const QPointF &pos, const QPixmap &pm, QSpanData *fg)
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
                    spans[n].x = xmin + x;
                    spans[n].y = y;
                    spans[n].coverage = 255;
                    int len = 1;
                    while (src_x < w-1 && src[(src_x+1) >> 3] & (0x1 << ((src_x+1) & 7))) {
                        ++src_x;
                        ++len;
                    }
                    spans[n].len = len;
                    x += len;
                    ++n;
                    if (n == spanCount) {
                        fg->blend(n, spans, fg);
                        n = 0;
                    }
                }
            }
#if defined (BITMAPS_ARE_MSB)
        } else {
            for (int x = 0; x < xmax - xmin; ++x) {
                int src_x = x + x_offset;
                uchar pixel = src[src_x >> 3];
                if (!pixel) {
                    x += 7;
                    continue;
                }
                if (pixel & (0x80 >> (x & 7))) {
                    spans[n].x = xmin + x;
                    spans[n].y = y;
                    spans[n].coverage = 255;
                    int len = 1;
                    while (src_x < w-1 && src[(src_x+1) >> 3] & (0x80 >> ((src_x+1) & 7))) {
                        ++src_x;
                        ++len;
                    }
                    spans[n].len = len;
                    x += len;
                    ++n;
                    if (n == spanCount) {
                        fg->blend(n, spans, fg);
                        n = 0;
                    }
                }
            }
        }
#endif
    }
    if (n) {
        fg->blend(n, spans, fg);
        n = 0;
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
}

void QRasterPaintEnginePrivate::rasterize(QT_FT_Outline *outline,
                                          ProcessSpans callback,
                                          void *userData, QRasterBuffer *rasterBuffer)
{
    if (!callback)
        return;

    void *data = userData;

    QT_FT_BBox clip_box = { 0, 0, deviceRect.width(), deviceRect.height() };
    if (rasterBuffer && rasterBuffer->clipEnabled && rasterBuffer->clip) {
        Q_ASSERT(((QSpanData *)userData)->rasterBuffer == rasterBuffer);
        clip_box.xMin = qMax((int)clip_box.xMin, rasterBuffer->clip->xmin);
        clip_box.xMax = qMin((int)clip_box.xMax, rasterBuffer->clip->xmax);
        if (antialiased)
            // ### Fixme: The black raster gives drawing errors when you try to
            // move ymin to something greater 0.
            clip_box.yMin = qMax((int)clip_box.yMin, rasterBuffer->clip->ymin);
        clip_box.yMax = qMin((int)clip_box.yMax, rasterBuffer->clip->ymax);
        Q_ASSERT(callback == qt_span_fill_clipped);
    }

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

    if (antialiased) {
        rasterParams.flags |= (QT_FT_RASTER_FLAG_AA | QT_FT_RASTER_FLAG_DIRECT);
        rasterParams.gray_spans = callback;
        int error = qt_ft_grays_raster.raster_render(*grayRaster, &rasterParams);
        if (error) {
            printf("QRasterPaintEnginePrivate::rasterize(), gray raster failed: %d\n", error);
        }
    } else {
        rasterParams.flags |= QT_FT_RASTER_FLAG_DIRECT;
        rasterParams.black_spans = callback;
        int error = qt_ft_standard_raster.raster_render(*blackRaster, &rasterParams);
        if (error) {
            qWarning("QRasterPaintEnginePrivate::rasterize(), black raster failed: %d", error);
        }
    }

}


void QRasterPaintEnginePrivate::updateClip_helper(const QPainterPath &path, Qt::ClipOperation op)
{
#ifdef QT_DEBUG_DRAW
    QRectF bounds = path.boundingRect();
    qDebug() << " --- updateClip_helper(), op=" << op << ", bounds=" << bounds
             << rasterBuffer->clipEnabled << rasterBuffer->clip;
#endif
    if (op == Qt::IntersectClip && !rasterBuffer->clipEnabled)
        op = Qt::ReplaceClip;

    if (op == Qt::NoClip) {
        rasterBuffer->resetClip();
        rasterBuffer->clipEnabled = false;
        goto end;
    } else if (op == Qt::ReplaceClip || (op == Qt::IntersectClip && path.isEmpty())) {
        rasterBuffer->resetClip();
    } else if (op == Qt::IntersectClip && !rasterBuffer->clip) {
        return;
    }

    rasterBuffer->clipEnabled = true;
    if (!path.isEmpty()) {
        QClipData *newClip = new QClipData(rasterBuffer->height());
        ClipData clipData = { rasterBuffer->clip, newClip, op };
        rasterize(outlineMapper->convertPath(path), qt_span_clip, &clipData, 0);
        newClip->fixup();

        if (op == Qt::UniteClip) {
            // merge clips
            QClipData *result = new QClipData(rasterBuffer->height());
            qt_merge_clip(rasterBuffer->clip, newClip, result);
            result->fixup();
            delete newClip;
            newClip = result;
        }

        delete rasterBuffer->clip;
        rasterBuffer->clip = newClip;
    }
end:
    penData.adjustSpanMethods();
    brushData.adjustSpanMethods();
}

QImage QRasterBuffer::colorizeBitmap(const QImage &image, const QColor &color)
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
    clipEnabled = false;
    opaqueBackground = false;

    compositionMode = QPainter::CompositionMode_SourceOver;
    delete clip;
    clip = 0;
    drawHelper = qDrawHelper + DrawHelper::Layout_ARGB;
    bgBrush = Qt::white;
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

    int format = image->format();
    if (format == QImage::Format_MonoLSB) {
        drawHelper = qDrawHelper + DrawHelper::Layout_MonoLSB;
    } else if (format == QImage::Format_Mono) {
        drawHelper = qDrawHelper + DrawHelper::Layout_Mono;
#ifdef Q_WS_QWS
    } else if (format == QImage::Format_RGB16) {
        drawHelper = qDrawHelper + DrawHelper::Layout_RGB16;
    } else if (format == QImage::Format_Grayscale4LSB) {
        drawHelper = qDrawHelper + DrawHelper::Layout_Gray4LSB;
#endif
    } else {
        drawHelper = qDrawHelper + DrawHelper::Layout_ARGB;
    }
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
//      qDebug("QClipData::fixup: count=%d",count);
    int y = -1;
    ymin = spans[0].y;
    ymax = spans[count-1].y + 1;
    xmin = INT_MAX;
    xmax = 0;
    for (int i = 0; i < count; ++i) {
//           qDebug() << "    " << spans[i].x << spans[i].y << spans[i].len << spans[i].coverage;
        if (spans[i].y != y) {
            y = spans[i].y;
            clipLines[y].spans = spans+i;
            clipLines[y].count = 0;
//              qDebug() << "        new line: y=" << y;
        }
        ++clipLines[y].count;
        xmin = qMin(xmin, (int)spans[i].x);
        xmax = qMax(xmax, (int)spans[i].x + spans[i].len);
    }
    ++xmax;
//     qDebug("xmin=%d,xmax=%d,ymin=%d,ymax=%d", xmin, xmax, ymin, ymax);
}


static const QSpan *qt_intersect_spans(const QClipData *clip, int *currentClip,
                                       const QSpan *spans, const QSpan *end,
                                       QSpan **outSpans, int available)
{
    QSpan *out = *outSpans;

    const QSpan *clipSpans = clip->spans + *currentClip;
    const QSpan *clipEnd = clip->spans + clip->count;

    while (available && spans < end ) {
        if (clipSpans >= clipEnd) {
            spans = end;
            break;
        }
        if (clipSpans->y > spans->y) {
            ++spans;
            continue;
        }
        if (spans->y != clipSpans->y) {
            if (clip->clipLines[spans->y].spans)
                clipSpans = clip->clipLines[spans->y].spans;
            else
                ++clipSpans;
            continue;
        }
        Q_ASSERT(spans->y == clipSpans->y);

        int sx1 = spans->x;
        int sx2 = sx1 + spans->len;
        int cx1 = clipSpans->x;
        int cx2 = cx1 + clipSpans->len;

        if (cx1 < sx1 && cx2 < sx1) {
            ++clipSpans;
            continue;
        } else if (sx1 < cx1 && sx2 < cx1) {
            ++spans;
            continue;
        }
        int x = qMax(sx1, cx1);
        int len = qMin(sx2, cx2) - x;
        if (len) {
            out->x = qMax(sx1, cx1);
            out->len = qMin(sx2, cx2) - out->x;
            out->y = spans->y;
            out->coverage = qt_div_255(spans->coverage * clipSpans->coverage);
            ++out;
            --available;
        }
        if (sx2 < cx2) {
            ++spans;
        } else {
            ++clipSpans;
        }
    }

    *outSpans = out;
    *currentClip = clipSpans - clip->spans;
    return spans;
}

static void qt_span_fill_clipped(int spanCount, const QSpan *spans, void *userData)
{
//     qDebug() << "qt_span_fill_clipped" << spanCount;
    QSpanData *fillData = reinterpret_cast<QSpanData *>(userData);

    Q_ASSERT(fillData->blend && fillData->unclipped_blend);

    QRasterBuffer *rb = fillData->rasterBuffer;
    Q_ASSERT(rb->clip);

    const int NSPANS = 256;
    QSpan cspans[NSPANS];
    int currentClip = 0;
    const QSpan *end = spans + spanCount;
    while (spans < end) {
        QSpan *clipped = cspans;
        spans = qt_intersect_spans(rb->clip, &currentClip, spans, end, &clipped, NSPANS);
//         qDebug() << "processed " << processed << "clipped" << clipped-cspans
//                  << "span:" << cspans->x << cspans->y << cspans->len << spans->coverage;

        if (clipped - cspans)
            fillData->unclipped_blend(clipped - cspans, cspans, fillData);
    }
}

static void qt_span_clip(int count, const QSpan *spans, void *userData)
{
    ClipData *clipData = reinterpret_cast<ClipData *>(userData);
//     qDebug() << " qt_span_clip: " << count << clipData->operation;
//      for (int i = 0; i < count; ++i) {
//           qDebug() << "    " << spans[i].x << spans[i].y << spans[i].len << spans[i].coverage;
//      }

    switch (clipData->operation) {

    case Qt::IntersectClip:
        {
            QClipData *newClip = clipData->newClip;
            int currentClip = 0;
            const QSpan *end = spans + count;
            while (spans < end) {
                QSpan *newspans = newClip->spans + newClip->count;
                spans = qt_intersect_spans(clipData->oldClip, &currentClip, spans, end,
                                           &newspans, newClip->allocated - newClip->count);
                newClip->count = newspans - newClip->spans;
                if (spans < end) {
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

void QSpanData::init(QRasterBuffer *rb)
{
    rasterBuffer = rb;
    type = None;
    txop = 0;
    bilinear = false;
    m11 = m22 = 1.;
    m12 = m21 = dx = dy = 0.;
}

void QSpanData::setup(const QBrush &brush)
{
    Qt::BrushStyle brushStyle = brush.style();
    switch (brushStyle) {
    case Qt::SolidPattern:
        type = Solid;
        solid.color = PREMUL(brush.color().rgba());
        break;

    case Qt::LinearGradientPattern:
        {
            type = LinearGradient;
            const QLinearGradient *g = static_cast<const QLinearGradient *>(brush.gradient());
            gradient.alphaColor = !brush.isOpaque();
            initGradient(g);

            gradient.linear.origin.x = g->start().x();
            gradient.linear.origin.y = g->start().y();
            gradient.linear.end.x = g->finalStop().x();
            gradient.linear.end.y = g->finalStop().y();
            break;
        }

    case Qt::RadialGradientPattern:
        {
            type = RadialGradient;
            const QRadialGradient *g = static_cast<const QRadialGradient *>(brush.gradient());
            gradient.alphaColor = !brush.isOpaque();
            initGradient(g);

            QPointF center = g->center();
            gradient.radial.center.x = center.x();
            gradient.radial.center.y = center.y();
            QPointF focal = g->focalPoint();
            gradient.radial.focal.x = focal.x();
            gradient.radial.focal.y = focal.y();
            gradient.radial.radius = g->radius();
        }
        break;

    case Qt::ConicalGradientPattern:
        {
            type = ConicalGradient;
            const QConicalGradient *g = static_cast<const QConicalGradient *>(brush.gradient());
            gradient.alphaColor = !brush.isOpaque();
            initGradient(g);
            gradient.spread = QGradient::RepeatSpread;

            QPointF center = g->center();
            gradient.conical.center.x = center.x();
            gradient.conical.center.y = center.y();
            gradient.conical.angle = g->angle() * 2 * Q_PI / 360.0;
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
            type = TiledTexture;
            extern QPixmap qt_pixmapForBrush(int brushStyle, bool invert);
            QPixmap texture = brushStyle == Qt::TexturePattern
                              ? brush.texture() : qt_pixmapForBrush(brushStyle, true);
            if (texture.depth() == 1) {
                rasterBuffer->tempImage = rasterBuffer->colorizeBitmap(texture.toImage(), brush.color());
            } else {
                rasterBuffer->tempImage = qt_map_to_32bit(texture);
            }
            initTexture(&rasterBuffer->tempImage);

        }
        break;

    case Qt::NoBrush:
    default:
        type = None;
        break;
    }
    adjustSpanMethods();
}

void QSpanData::adjustSpanMethods()
{
    switch(type) {
    case None:
        unclipped_blend = 0;
        break;
    case Solid:
        unclipped_blend = rasterBuffer->drawHelper->blendColor;
        break;
    case Texture:
        if (txop > QPainterPrivate::TxTranslate) {
            unclipped_blend = bilinear
                              ? rasterBuffer->drawHelper->blendTransformedBilinear
                              : rasterBuffer->drawHelper->blendTransformed;
        } else {
            unclipped_blend = rasterBuffer->drawHelper->blend;
        }
        break;
    case TiledTexture:
        if (txop > QPainterPrivate::TxTranslate) {
            unclipped_blend = bilinear
                              ? rasterBuffer->drawHelper->blendTransformedBilinearTiled
                              : rasterBuffer->drawHelper->blendTransformedTiled;
        } else {
            unclipped_blend = rasterBuffer->drawHelper->blendTiled;
        }
        break;
    case LinearGradient:
        unclipped_blend = rasterBuffer->drawHelper->blendLinearGradient;
        break;
    case RadialGradient:
        unclipped_blend = rasterBuffer->drawHelper->blendRadialGradient;
        break;
    case ConicalGradient:
        unclipped_blend = rasterBuffer->drawHelper->blendConicalGradient;
        break;
    }
    // setup clipping
    if (!unclipped_blend) {
        blend = 0;
    } else if (rasterBuffer->clipEnabled) {
        blend = rasterBuffer->clip ? qt_span_fill_clipped : 0;
    } else {
        blend = unclipped_blend;
    }
}

void QSpanData::setupMatrix(const QMatrix &matrix, int tx, int bilin)
{
    QMatrix inv = matrix.inverted();
    m11 = inv.m11();
    m12 = inv.m12();
    m21 = inv.m21();
    m22 = inv.m22();
    dx = inv.dx();
    dy = inv.dy();
    txop = tx;
    bilinear = bilin;
    adjustSpanMethods();
}

void QSpanData::initTexture(const QImage *image)
{
    texture.imageData = image->bits();
    texture.width = image->width();
    texture.height = image->height();
    texture.hasAlpha = image->format() != QImage::Format_RGB32;
    adjustSpanMethods();
}

void QSpanData::initGradient(const QGradient *g)
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


        int dist;
        qreal diff = (stops[current_stop+1].first - stops[current_stop].first);
        if (diff != 0.0)
            dist = (int)(256*(dpos - stops[current_stop].first) / diff);
        else
            dist = 0;
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
                               bool convertToText)
{
    QPointF p = pos;
    QFontEngine *fe = ti.fontEngine;

    SetTextAlign(hdc, TA_BASELINE);
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(0, 0, 0));

    bool has_kerning = ti.f && ti.f->kerning();

    qreal x = p.x();
    qreal y = p.y();

    if (ti.flags & (QTextItem::Underline|QTextItem::StrikeOut)) {
        LOGFONT lf = fe->logfont;
        lf.lfUnderline = (ti.flags & QTextItem::Underline);
        lf.lfStrikeOut = (ti.flags & QTextItem::StrikeOut);
        HFONT hf = QT_WA_INLINE(CreateFontIndirectW(&lf), CreateFontIndirectA((LOGFONTA*)&lf));
        SelectObject(hdc, hf);
    } else {
        SelectObject(hdc, fe->hfont);
    }

    unsigned int options =  fe->ttf && !convertToText ? ETO_GLYPH_INDEX : 0;

    wchar_t *convertedGlyphs = (wchar_t *)ti.chars;

    QGlyphLayout *glyphs = ti.glyphs;

    int xo = qRound(x);

    if (!(ti.flags & QTextItem::RightToLeft)) {
        // hack to get symbol fonts working on Win95. See also QFontEngine constructor
        if (fe->useTextOutA) {
            // can only happen if !ttf
            for(int i = 0; i < ti.num_glyphs; i++) {
                QString str(QChar(glyphs->glyph));
                QByteArray cstr = str.toLocal8Bit();
		TextOutA(hdc, qRound(x + glyphs->offset.x.toReal()), qRound(y + glyphs->offset.y.toReal()),
                         cstr.data(), cstr.length());
		x += glyphs->advance.x.toReal();
                glyphs++;
            }
        } else {
            bool haveOffsets = false;
            QFixed w = 0;
            for(int i = 0; i < ti.num_glyphs; i++) {
                if (glyphs[i].offset.x != 0 || glyphs[i].offset.y != 0 || glyphs[i].space_18d6 != 0) {
                    haveOffsets = true;
                    break;
                }
                w += glyphs[i].advance.x;
            }

            if (haveOffsets || has_kerning) {
                for(int i = 0; i < ti.num_glyphs; i++) {
                    wchar_t chr = glyphs->glyph;
		    qreal xp = x + glyphs->offset.x.toReal();
		    qreal yp = y + glyphs->offset.y.toReal();

            ExtTextOutW(hdc, qRound(xp), qRound(yp), options, 0,
                convertToText ? convertedGlyphs + i : &chr,
                1, 0);
		    x += (glyphs->advance.x + QFixed::fromFixed(glyphs->space_18d6)).toReal();
		    y += glyphs->advance.y.toReal();
                    glyphs++;
                }
            } else {
                // fast path
                QVarLengthArray<wchar_t> g(ti.num_glyphs);
                for (int i = 0; i < ti.num_glyphs; ++i)
                    g[i] = glyphs[i].glyph;
                // fast path
                    QString s = QString::fromRawData(ti.chars, ti.num_chars);
                ExtTextOutW(hdc,
		            qRound(x + glyphs->offset.x.toReal()),
		            qRound(y + glyphs->offset.y.toReal()),
                    options, 0, convertToText ? convertedGlyphs : g.data(), ti.num_glyphs, 0);
		x += w.toReal();
            }
        }
    } else {
        int i = ti.num_glyphs;
        while(i--) {
	    x += (glyphs[i].advance.x + QFixed::fromFixed(glyphs[i].space_18d6)).toReal();
	    y += glyphs[i].advance.y.toReal();
        }
        i = 0;
        while(i < ti.num_glyphs) {
	    x -= glyphs[i].advance.x.toReal();
	    y -= glyphs[i].advance.y.toReal();

	    int xp = qRound(x+glyphs[i].offset.x.toReal());
            int yp = qRound(y+glyphs[i].offset.y.toReal());
            QString s = QString::fromRawData(ti.chars, ti.num_chars);
            ExtTextOutW(hdc, xp, yp, options, 0,
                convertToText ?
                    convertedGlyphs + i
                    : reinterpret_cast<wchar_t *>(&glyphs[i].glyph),
                1, 0);

            if (glyphs[i].nKashidas) {
                QChar ch(0x640); // Kashida character
                QGlyphLayout g[8];
                int nglyphs = 7;
                ti.fontEngine->stringToCMap(&ch, 1, g, &nglyphs, 0);
                for (uint k = 0; k < glyphs[i].nKashidas; ++k) {
		    x -= g[0].advance.x.toReal();
		    y -= g[0].advance.y.toReal();

		    int xp = qRound(x+g[0].offset.x.toReal());
		    int yp = qRound(y+g[0].offset.y.toReal());
                    ExtTextOutW(hdc, xp, yp, options, 0,
                        convertToText
                            ? reinterpret_cast<wchar_t *>(&ch.unicode())
                            : reinterpret_cast<wchar_t *>(&g[0].glyph), 1, 0);
                }
            } else {
		x -= QFixed::fromFixed(glyphs[i].space_18d6).toReal();
            }
            ++i;
        }
    }

    if (ti.flags & (QTextItem::Underline|QTextItem::StrikeOut))
        DeleteObject(SelectObject(hdc, fe->hfont));

    if (ti.flags & (QTextItem::Overline)) {
	int lw = qRound(fe->lineThickness());
	int yp = qRound(y - fe->ascent().toReal() - 1);
        Rectangle(hdc, xo, yp, qRound(x), yp + lw);
    }
}

static void draw_text_item_multi(const QPointF &p, const QTextItemInt &ti, HDC hdc,
                                 bool convertToText)
{
    QFontEngineMulti *multi = static_cast<QFontEngineMulti *>(ti.fontEngine);

    if (!ti.num_glyphs) {
        if (ti.flags) {
            QTextItemInt ti2 = ti;
            ti2.fontEngine = multi->engine(0);
            ti2.f = ti.f;
            draw_text_item_win(p, ti2, hdc, convertToText);
        }
        return;
    }

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
        draw_text_item_win(QPointF(x, y), ti2, hdc, convertToText);

	QFixed x_add;
        // reset the high byte for all glyphs and advance to the next sub-string
        const int hi = which << 24;
        for (i = start; i < end; ++i) {
            glyphs[i].glyph = hi | glyphs[i].glyph;
            x_add += glyphs[i].advance.x;
        }
	x += x_add.toReal();

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
    draw_text_item_win(QPointF(x, y), ti2, hdc, convertToText);

    // reset the high byte for all glyphs
    const int hi = which << 24;
    for (i = start; i < end; ++i)
        glyphs[i].glyph = hi | glyphs[i].glyph;
}

void qt_draw_text_item(const QPointF &pos, const QTextItemInt &ti, HDC hdc,
                       bool convertToText)
{
    if (!ti.num_glyphs)
        return;

    switch(ti.fontEngine->type()) {
    case QFontEngine::Multi:
        draw_text_item_multi(pos, ti, hdc, convertToText);
        break;
    case QFontEngine::Win:
    default:
        draw_text_item_win(pos, ti, hdc, convertToText);
        break;
    }
}



#endif


/*!
    \internal

    Draws a line using the floating point midpoint algorithm. The line
    \a line is already in device coords at this point.
*/

static void drawLine_midpoint_i(int x1, int y1, int x2, int y2, ProcessSpans span_func, QSpanData *data,
                                LineDrawMode style, const QRect &devRect)
{
#ifdef QT_DEBUG_DRAW
    qDebug() << "   - drawLine_midpoint_i" << line;
#endif

    int x, y;
    int dx, dy, d, incrE, incrNE;

    QT_FT_Span span = { 0, 1, 0, 255 };

    dx = x2 - x1;
    dy = y2 - y1;

    if (dy == 0) {
        // specialcase horizontal lines
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
                span.coverage = 255;
                span_func(1, &span, data);
            }
        }
        return;
    } else if (dx == 0) {
        if (x1 >= 0 && x1 < devRect.width()) {
            int start = qMax(0, qMin(y1, y2));
            int stop = qMax(y1, y2) + 1;
            stop = qMin(devRect.height(), stop);
            fillRect(QRect(x1, start, 1, stop - start), data);
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
