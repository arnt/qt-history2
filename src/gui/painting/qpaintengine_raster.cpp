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


#if defined(Q_WS_WIN)
#  ifndef SPI_GETFONTSMOOTHINGTYPE
#    define SPI_GETFONTSMOOTHINGTYPE 0x200A
#  endif

#  ifndef FE_FONTSMOOTHINGCLEARTYPE
#    define FE_FONTSMOOTHINGCLEARTYPE 0x0002
#  endif
#endif

#define qreal_to_fixed_26_6(f) (int(f * 64))
#define qt_swap_int(x, y) { int tmp = (x); (x) = (y); (y) = tmp; }
#define qt_swap_qreal(x, y) { qreal tmp = (x); (x) = (y); (y) = tmp; }

#ifdef Q_WS_WIN
void qt_draw_text_item(const QPointF &point, const QTextItemInt &ti, HDC hdc,
                       bool convertToText, const QMatrix &xform, const QPointF &topLeft);
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

// This limitations comes from qgrayraster.c. Any higher and
// rasterization of shapes will produce incorrect results.
const int QT_RASTER_COORD_LIMIT = 16385;

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
        m_valid = true;
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
                // Put the object on the stack to avoid the odd case where
                // lineTo reallocs the databuffer and the QPointF & will
                // be invalidated.
                QPointF pt = m_elements.at(m_subpath_start);
                lineTo(pt);
            }
        }
    }

    QT_FT_Outline *outline() {
        if (m_valid)
            return &m_outline;
        return 0;
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
        return outline();
    }

public:
    QDataBuffer<QPainterPath::ElementType> m_element_types;
    QDataBuffer<QPointF> m_elements;
    QDataBuffer<QPointF> m_elements_dev;
    QDataBuffer<QT_FT_Vector> m_points;
    QDataBuffer<char> m_tags;
    QDataBuffer<int> m_contours;

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

    bool m_valid;
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
        default:
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
        default:
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

    if (!clipped_count) {
        m_valid = false;
        return;
    }

    QPainterPath::ElementType *point_types = new QPainterPath::ElementType[clipped_count];
    point_types[0] = QPainterPath::MoveToElement;
    for (int i=0; i<clipped_count; ++i) point_types[i] = QPainterPath::LineToElement;
    convertElements(clipped_points, point_types, clipped_count);
    delete[] point_types;
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

    d->rasterPoolSize = 4096;
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
    d->user_clip_enabled = false;
    d->opacity = 256;

#if defined(Q_WS_WIN)
    d->clear_type_text = false;
    QT_WA({
        UINT result;
        BOOL ok;
        ok = SystemParametersInfoW(SPI_GETFONTSMOOTHINGTYPE, 0, &result, 0);
        if (ok)
            d->clear_type_text = (result == FE_FONTSMOOTHINGCLEARTYPE);
    }, {
        UINT result;
        BOOL ok;
        ok = SystemParametersInfoA(SPI_GETFONTSMOOTHINGTYPE, 0, &result, 0);
        if (ok)
            d->clear_type_text = (result == FE_FONTSMOOTHINGCLEARTYPE);
    });
#endif

    d->rasterBuffer->init();

#if defined(Q_WS_WIN)
    d->fontRasterBuffer->setupHDC(d->clear_type_text);
#endif

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
        gccaps &= ~PaintOutsidePaintEvent;
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

#if defined(Q_WS_WIN) || defined(Q_WS_QWS)
    if (device->devType() == QInternal::Pixmap) {
        QPixmap *pixmap = static_cast<QPixmap *>(device);
        if (pixmap->isNull()) {
            qWarning("Cannot paint on a null pixmap");
            return false;
        }
        QPixmapData *data = static_cast<QPixmap *>(device)->data;
        device = &data->image;
    }
#elif defined(Q_WS_MAC)
    if (device->devType() == QInternal::Pixmap) {
        QPixmap *pixmap = static_cast<QPixmap *>(device);
        if (pixmap->isNull()) {
            qWarning("Cannot paint on a null pixmap");
            return false;
        }
        d->rasterBuffer->prepare(pixmap);
        isBitmap = pixmap->depth() == 1;
    }
#endif

    if (device->devType() == QInternal::Image) {
        QImage *image = static_cast<QImage *>(device);
        int format = image->format();
        d->flushOnEnd = false;

        d->rasterBuffer->prepare(image);
        if (format == QImage::Format_MonoLSB) {
            d->mono_surface = true;
        } else if (format == QImage::Format_Mono) {
            d->mono_surface = true;
        } else if (format == QImage::Format_RGB32) {
            ;
        } else if (format == QImage::Format_ARGB32_Premultiplied) {
            gccaps |= PorterDuff;
        } else if (format == QImage::Format_ARGB32) {
            gccaps |= PorterDuff;
#ifdef Q_WS_QWS
        } else if (format == QImage::Format_RGB16) {
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
    d->penData.setup(d->pen.brush(), d->opacity);
    d->stroker = &d->basicStroker;
    d->basicStroker.setClipRect(d->deviceRect);

    d->brushData.init(d->rasterBuffer);
    d->brushData.setup(d->brush, d->opacity);

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

    if (d->rasterBuffer->disabled_clip) {
        delete d->rasterBuffer->disabled_clip;
        d->rasterBuffer->disabled_clip = 0;
    }

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
    if (!d->rasterBuffer->hdc())
        return;

    if (device->devType() == QInternal::Widget) {
        HDC hdc = device->getDC();

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
    }
#elif defined(Q_WS_MAC)
#  ifdef QMAC_NO_COREGRAPHICS
#    warning "unhandled"
#  else
    extern CGContextRef qt_mac_cg_context(const QPaintDevice *); //qpaintdevice_mac.cpp
    extern void qt_mac_clip_cg(CGContextRef, const QRegion &, const QPoint *, CGAffineTransform *); //qpaintengine_mac.cpp
    if(CGContextRef ctx = qt_mac_cg_context(device)) {
        qt_mac_clip_cg(ctx, systemClip(), 0, 0);
        const CGRect source = CGRectMake(0, 0, d->deviceRect.width(), d->deviceRect.height());
        CGImageRef subimage = CGImageCreateWithImageInRect(d->rasterBuffer->m_data, source);

        const CGRect dest = CGRectMake(offset.x(), offset.y(),
                                       d->deviceRect.width(), d->deviceRect.height());
        HIViewDrawCGImage(ctx, &dest, subimage); //top left
        CGContextRelease(ctx);
        CGImageRelease(subimage);
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

    if (flags & DirtyOpacity) {
        update_fast_pen = true;
        d->opacity = qRound(state.opacity() * 256.0);
        if (d->opacity > 256)
            d->opacity = 256;
        if (d->opacity < 0)
            d->opacity = 0;

        // Force update pen/brush as to get proper alpha colors propagated
        flags |= DirtyPen;
        flags |= DirtyBrush;
    }

    if (flags & DirtyBackgroundMode) {
        d->rasterBuffer->opaqueBackground = (state.backgroundMode() == Qt::OpaqueMode);
    }

    if (flags & DirtyBackground) {
        d->rasterBuffer->bgBrush = state.backgroundBrush();
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
            d->dashStroker->setClipRect(d->deviceRect);
            d->dashStroker->setDashPattern(d->pen.dashPattern());
            d->stroker = d->dashStroker;
        } else {
            d->stroker = 0;
        }
        d->penData.setup(pen_style == Qt::NoPen ? QBrush() : d->pen.brush(), d->opacity);
    }

    if (flags & DirtyBrush) {
        QBrush brush = state.brush();
        d->brush = brush;
        d->brushData.setup(d->brush, d->opacity);
    }

    if (flags & DirtyBrushOrigin) {
        d->brushOffset = state.brushOrigin();
        d->brushData.setupMatrix(d->brushMatrix(), d->txop, d->bilinear);
    }

    if (flags & (DirtyClipPath | DirtyClipRegion)) {
        d->user_clip_enabled = true;
        // If we're setting a clip, we kill the old clip
        if (d->rasterBuffer->disabled_clip) {
            delete d->rasterBuffer->disabled_clip;
            d->rasterBuffer->disabled_clip = 0;
        }
    }

    if (flags & DirtyClipPath) {
        updateClipPath(state.clipPath(), state.clipOperation());

    } else if (flags & DirtyClipRegion) {
        updateClipRegion(state.clipRegion(), state.clipOperation());

    } else if (flags & DirtyClipEnabled) {

        if (state.isClipEnabled() != d->user_clip_enabled) {
            d->user_clip_enabled = state.isClipEnabled();

            // The tricky case... When we disable clipping we still do
            // system clip so we need to rasterize the system clip and
            // replace the current clip with it. Since people might
            // choose to set clipping to true later on we have to the
            // current one (in disabled_clip).
            if (!d->baseClip.isEmpty()) {
                if (!state.isClipEnabled()) { // save current clip for later
                    Q_ASSERT(!d->rasterBuffer->disabled_clip);
                    d->rasterBuffer->disabled_clip = d->rasterBuffer->clip;
                    d->rasterBuffer->clip = 0;
                    updateClipPath(QPainterPath(), Qt::NoClip);
                } else { // re-enable old clip
                    Q_ASSERT(d->rasterBuffer->disabled_clip);
                    d->rasterBuffer->resetClip();
                    d->rasterBuffer->clip = d->rasterBuffer->disabled_clip;
                    d->rasterBuffer->disabled_clip = 0;
                }
            }
            d->penData.adjustSpanMethods();
            d->brushData.adjustSpanMethods();
        }
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
        d->fast_pen = d->pen.style() == Qt::SolidLine
                      && !d->antialiased
                      && d->pen.brush().isOpaque()
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
    if (!d->antialiased && d->txop <= QPainterPrivate::TxTranslate) {
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

        d->rasterize(d->outlineMapper->outline(), d->penData.blend, &d->penData, d->rasterBuffer);
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
        d->rasterize(d->outlineMapper->outline(), d->brushData.blend, &d->brushData, d->rasterBuffer);
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

            d->rasterize(d->outlineMapper->outline(), d->penData.blend, &d->penData, d->rasterBuffer);

            d->outlineMapper->setMatrix(d->matrix, d->txop);
        }
    }

}

void QRasterPaintEngine::drawPolygon(const QPoint *points, int pointCount, PolygonDrawMode mode)
{
    Q_D(QRasterPaintEngine);
    if (!(d->int_xform && d->fast_pen)) {
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
        d->rasterize(d->outlineMapper->outline(), d->brushData.blend, &d->brushData, d->rasterBuffer);
    }

    // Do the outline...
    if (d->penData.blend) {

        bool needs_closing = mode != PolylineMode && points[0] != points[pointCount-1];

        QRect devRect(0, 0, d->deviceRect.width(), d->deviceRect.height());

        LineDrawMode mode_for_last = (d->pen.capStyle() != Qt::FlatCap
                                      ? LineDrawIncludeLastPixel
                                      : LineDrawNormal);

        int m11 = int(d->matrix.m11());
        int m22 = int(d->matrix.m22());
        int dx = int(d->matrix.dx());
        int dy = int(d->matrix.dy());

        // Draw the all the line segments.
        for (int i=1; i<pointCount; ++i) {
            drawLine_midpoint_i(points[i-1].x() * m11 + dx, points[i-1].y() * m22 + dy,
                                points[i].x() * m11 + dx, points[i].y() * m22 + dy,
                                d->penData.blend, &d->penData,
                                i == pointCount - 1 ? mode_for_last : LineDrawIncludeLastPixel,
                                devRect);
        }

        // Polygons are implicitly closed.
        if (needs_closing) {
            drawLine_midpoint_i(points[pointCount-1].x() * m11 + dx, points[pointCount-1].y() * m22 + dy,
                                points[0].x() * m11 + dx, points[0].y() * m22 + dy,
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

    if (pixmap.depth() == 1) {
        if (d->txop <= QPainterPrivate::TxTranslate
            && !d->rasterBuffer->opaqueBackground
            && r.size() == sr.size()
            && r.size() == pixmap.size()) {
            d->drawBitmap(r.topLeft() + QPointF(d->matrix.dx(), d->matrix.dy()), pixmap, &d->penData);
            return;
        } else {
            drawImage(r, d->rasterBuffer->colorizeBitmap(pixmap.toImage(), d->pen.color()), sr);
        }
    } else {
#if defined(Q_WS_MAC) && 0
        if(CGContextRef ctx = macCGContext()) {
            const CGRect source = CGRectMake(sr.x(), sr.y(), sr.width(), sr.height());
            CGImageRef subimage = CGImageCreateWithImageInRect(pixmap.data->cg_data, source);
            const CGRect dest = CGRectMake(r.x(), r.y(), r.width(), r.height());
            HIViewDrawCGImage(ctx, &dest, subimage); //top left
            CGImageRelease(subimage);
        } else
#endif
            drawImage(r, pixmap.toImage(), sr);
    }

}

void QRasterPaintEngine::drawImage(const QRectF &r, const QImage &img, const QRectF &sr,
                                   Qt::ImageConversionFlags)
{
#ifdef QT_DEBUG_DRAW
    qDebug() << " - QRasterPaintEngine::drawImage(), r=" << r << " sr=" << sr << " image=" << img.size() << "depth=" << img.depth();
#endif

    Q_D(QRasterPaintEngine);
    QSpanData textureData;
    textureData.init(d->rasterBuffer);
    textureData.type = QSpanData::Texture;
    textureData.initTexture(&img, d->opacity);

    bool stretch_sr = r.width() != sr.width() || r.height() != sr.height();

    if (d->txop > QPainterPrivate::TxTranslate || stretch_sr) {
        QMatrix copy = d->matrix;
        copy.translate(r.x(), r.y());
        if (stretch_sr)
            copy.scale(r.width() / sr.width(), r.height() / sr.height());
        copy.translate(-sr.x(), -sr.y());
        textureData.setupMatrix(copy, QPainterPrivate::TxRotShear, d->bilinear);
	textureData.adjustSpanMethods();

        bool wasAntialiased = d->antialiased;
        if (!d->antialiased)
            d->antialiased = d->bilinear;
        QPainterPath path;
        path.addRect(r);
        fillPath(path, &textureData);
        d->antialiased = wasAntialiased;
    } else {
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
        image = pixmap.toImage();

    QSpanData textureData;
    textureData.init(d->rasterBuffer);
    textureData.type = QSpanData::Texture;
    textureData.initTexture(&image, d->opacity, TextureData::Tiled);

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
    // #####
}

#else

#if defined(Q_WS_WIN)
bool QRasterPaintEngine::drawTextInFontBuffer(const QRect &devRect, int xmin, int ymin, int xmax,
                                              int ymax, const QTextItem &textItem, bool clearType,
                                              qreal leftBearingReserve, const QPointF &topLeft)
{
    Q_D(QRasterPaintEngine);
    const QTextItemInt &ti = static_cast<const QTextItemInt &>(textItem);

    if (d->mono_surface) {
        // Some extra work to get proper rasterization of text on monochrome targets
        HBITMAP bitmap = CreateBitmap(devRect.width(), devRect.height(), 1, 1, 0);
        HDC hdc = CreateCompatibleDC(qt_win_display_dc());
        HGDIOBJ null_bitmap = SelectObject(hdc, bitmap);
        SelectObject(hdc, GetStockObject(WHITE_BRUSH));
        SelectObject(hdc, GetStockObject(NULL_PEN));
        Rectangle(hdc, 0, 0, devRect.width() + 1, devRect.height() + 1);

        // Fill buffer with stuff
        SelectObject(hdc, GetStockObject(BLACK_BRUSH));
        SelectObject(hdc, GetStockObject(BLACK_PEN));
        SetTextColor(hdc, RGB(0,0,0));
        qt_draw_text_item(QPointF(leftBearingReserve, ti.ascent.toReal()), ti, hdc,
            false, QMatrix(d->matrix.m11(), d->matrix.m12(), d->matrix.m21(), 
            d->matrix.m22(), 0, 0), topLeft);

        BitBlt(d->fontRasterBuffer->hdc(), 0, 0, devRect.width(), devRect.height(),
               hdc, 0, 0, SRCCOPY);
        SelectObject(hdc, null_bitmap);
        DeleteObject(bitmap);
        DeleteDC(hdc);

        return false;
    } else {
        // Let Windows handle the composition of background and foreground for cleartype text
        QRgb penColor = 0;
        if (clearType) {
            penColor = d->penData.solid.color;

            // Copy background from raster buffer
            for (int y=ymin; y<ymax; ++y) {
                QRgb *sourceScanline = (QRgb *) d->rasterBuffer->scanLine(y);
                QRgb *destScanline = (QRgb *) d->fontRasterBuffer->scanLine(y - devRect.y());
                for (int x=xmin; x<xmax; ++x) {
                    // If the background is transparent, set it to completely opaque so we will
                    // recognize it after Windows screws up the alpha channel of font buffer.
                    // Otherwise, just copy the contents.
                    if (qAlpha(sourceScanline[x]) == 0x00)
                        destScanline[x - devRect.x()] |= 0xff000000;
                    else
                        destScanline[x - devRect.x()] = sourceScanline[x];
                }
            }
        } else {
            d->fontRasterBuffer->resetBuffer(255);
        }

        // Draw the text item
        if (clearType) {
            COLORREF cf = RGB(qRed(penColor), qGreen(penColor), qBlue(penColor));
            SelectObject(d->fontRasterBuffer->hdc(), CreateSolidBrush(cf));
            SelectObject(d->fontRasterBuffer->hdc(), CreatePen(PS_SOLID, 1, cf));
            SetTextColor(d->fontRasterBuffer->hdc(), cf);
        }


        qt_draw_text_item(QPointF(leftBearingReserve, ti.ascent.toReal()), ti,
                          d->fontRasterBuffer->hdc(), false, 
                          QMatrix(d->matrix.m11(), d->matrix.m12(), d->matrix.m21(), 
                          d->matrix.m22(), 0, 0), topLeft);


        if (clearType) {
            DeleteObject(SelectObject(d->fontRasterBuffer->hdc(),GetStockObject(NULL_BRUSH)));
            DeleteObject(SelectObject(d->fontRasterBuffer->hdc(),GetStockObject(BLACK_PEN)));
        }

        // Clean up alpha channel
        if (clearType) {
            for (int y=ymin; y<ymax; ++y) {
                QRgb *scanline = (QRgb *) d->fontRasterBuffer->scanLine(y - devRect.y());
                QRgb *rbScanline = (QRgb *) d->rasterBuffer->scanLine(y);
                for (int x=xmin; x<xmax; ++x) {
                    // If alpha is 0, then Windows has drawn text on top of the pixel, so set
                    // the pixel to opaque. Otherwise, Windows has not touched the pixel, so
                    // we can set it to transparent so the background shines through instead.
                    switch (qAlpha(scanline[x - devRect.x()])) {
                    case 0x0:
                        // Special case: If Windows has drawn on top of a transparent pixel, then
                        // we bail out. This is an attempt at avoiding the problem where Windows
                        // has no background to use for composition, but also minimizing the
                        // number of cases hit by the fall back.
                        // ### This is far from optimal.
                        if (qAlpha(rbScanline[x]) == 0) {
                            return drawTextInFontBuffer(devRect, xmin, ymin, xmax, ymax, textItem,
                                false, leftBearingReserve, topLeft);
                        }
                        scanline[x - devRect.x()] |= 0xff000000;
                        break ;
                    default: scanline[x - devRect.x()] = 0x0; break ;
                    };
                }
            }
        }
    }

    return clearType;
}
#endif // Q_WS_WIN


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

    if (QT_WA_INLINE(false, d->txop >= QPainterPrivate::TxScale)) {
        QPaintEngine::drawTextItem(p, textItem);
        return;
    }

    // Only support cleartype for solid pens, 32 bit target buffers and when the pen color is
    // opaque
    bool clearType = d->clear_type_text
                     && d->penData.type == QSpanData::Solid
                     && d->deviceDepth == 32
                     &&  qAlpha(d->penData.solid.color) == 255;


    QFixed x_buffering = ti.ascent;

    // Hack to reserve some space on the left side of the string in case
    // the character has a large negative bearing (e.g. it should be drawn on top
    // of the previous character)
    qreal leftBearingReserve = ti.fontEngine->maxCharWidth(); 
    qreal bufferWidth = (ti.width + x_buffering).toReal() + leftBearingReserve;
    qreal bufferHeight = (ti.ascent + ti.descent + 1).toReal();

    QMatrix m(d->matrix.m11(), d->matrix.m12(), d->matrix.m21(), d->matrix.m22(), 0, 0);
    QRectF logRect(0, 0, bufferWidth, bufferHeight);
    QPointF topLeft = m.mapRect(logRect).topLeft();

    logRect.moveTo(p.x() - leftBearingReserve, p.y() - ti.ascent.toReal());
    QRect devRect = d->matrix.mapRect(logRect).toRect();
    if(devRect.width() == 0 || devRect.height() == 0)
        return;

    d->fontRasterBuffer->prepare(devRect.width(), devRect.height());

    // Boundaries
    int ymax = qMin(devRect.y() + devRect.height(), d->rasterBuffer->height());
    int ymin = qMax(devRect.y(), 0);
    int xmax = qMin(devRect.x() + devRect.width(), d->rasterBuffer->width());
    int xmin = qMax(devRect.x(), 0);

    QClipData *clip = d->rasterBuffer->clipEnabled ? d->rasterBuffer->clip : 0;
    if (clip) {
        xmin = qMax(xmin, clip->xmin);
        xmax = qMin(xmax, clip->xmax);
        ymin = qMax(ymin, clip->ymin);
        ymax = qMin(ymax, clip->ymax);
    }

    if (xmax - xmin <= 0 || ymax - ymin <= 0)
        return;

    // Fill the font raster buffer with text
    clearType = drawTextInFontBuffer(devRect, xmin, ymin, xmax, ymax, textItem,
                                     clearType, leftBearingReserve, 
                                     topLeft);

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
    } else if (clearType) {
        QSpanData data;
        data.init(d->rasterBuffer);
        data.type = QSpanData::Texture;
        data.texture.imageData = d->fontRasterBuffer->buffer();
        data.texture.width = d->fontRasterBuffer->bytesPerLine() / 4;
        data.texture.height = d->fontRasterBuffer->height();
        data.texture.bytesPerLine = d->fontRasterBuffer->bytesPerLine();
        data.texture.hasAlpha = true;
        data.bilinear = false;
        data.texture.const_alpha = 255;
        data.texture.format = QImage::Format_ARGB32_Premultiplied;
        data.texture.colorTable = 0;

        data.dx = -devRect.x();
        data.dy = -devRect.y();
        data.adjustSpanMethods();
        fillRect(QRect(xmin, ymin, xmax - xmin, ymax - ymin), &data);
    } else if (d->clear_type_text) {
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
    } else {
        // For the noncleartype/grayscale text we can look at only one color component,
        // and save a bit of qGray effort...
        for (int y=ymin; y<ymax; ++y) {
            QRgb *scanline = (QRgb *) d->fontRasterBuffer->scanLine(y - devRect.y()) - devRect.x();
            // Generate spans for this y coord
            for (int x = xmin; x<xmax; ) {
                // Skip those with 0 coverage (black on white so inverted)
                while (x < xmax && qBlue(scanline[x]) == 255) ++x;
                if (x >= xmax) break;

                int prev = qBlue(scanline[x]);
                QT_FT_Span span = { x, 0, y, 255 - prev };

                // extend span until we find a different one.
                while (x < xmax && qBlue(scanline[x]) == prev) ++x;
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
    if (d->txop < QPainterPrivate::TxScale && !(ti.fontEngine->type() == QFontEngine::Freetype && static_cast<QFontEngineFT*>(ti.fontEngine)->drawAsOutline())) {
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

        bool flat_pen = d->pen.capStyle() == Qt::FlatCap;
        if (flat_pen)
            d->basicStroker.setCapStyle(Qt::SquareCap);

        const QPointF *end = points + pointCount;
        while (points < end) {
            QPainterPath path;
            path.moveTo(*points);
            path.lineTo(points->x() + 0.001, points->y());
            drawPath(path);
            ++points;
        }

        d->brush = oldBrush;

        if (flat_pen)
            d->basicStroker.setCapStyle(Qt::FlatCap);

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
            x = qFloor(points->x() + dx);
            y = qFloor(points->y() + dy);
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
    if (!d->penData.blend)
        return;
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
                drawLine_midpoint_i(qFloor(line.x1()), qFloor(line.y1()),
                                    qFloor(line.x2()), qFloor(line.y2()),
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
    if (!d->penData.blend)
        return;
    if (d->fast_pen) {
        QRect bounds(0, 0, d->deviceRect.width(), d->deviceRect.height());
        LineDrawMode mode = d->pen.capStyle() == Qt::FlatCap
                            ? LineDrawNormal
                            : LineDrawIncludeLastPixel;
        for (int i=0; i<lineCount; ++i) {
            QLineF line = lines[i] * d->matrix;
            drawLine_midpoint_i(qFloor(line.x1()), qFloor(line.y1()),
                                qFloor(line.x2()), qFloor(line.y2()),
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

        d->rasterize(d->outlineMapper->outline(), d->brushData.blend, &d->brushData, d->rasterBuffer);
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

        d->rasterize(d->outlineMapper->outline(), d->penData.blend, &d->penData, d->rasterBuffer);

        d->outlineMapper->setMatrix(d->matrix, d->txop);
    }
}

#ifdef Q_WS_MAC
CGContextRef
QRasterPaintEngine::macCGContext() const
{
    Q_D(const QRasterPaintEngine);
    return d->rasterBuffer->macCGContext();
}
#endif

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
                    x += 7 - (x%8);
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
                    x += 7 - (x%8);
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
    if (!callback || !outline)
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

    bool done = false;
    int error;

    while (!done) {

        if (antialiased) {
            rasterParams.flags |= (QT_FT_RASTER_FLAG_AA | QT_FT_RASTER_FLAG_DIRECT);
            rasterParams.gray_spans = callback;
            error = qt_ft_grays_raster.raster_render(*grayRaster, &rasterParams);
        } else {
            rasterParams.flags |= QT_FT_RASTER_FLAG_DIRECT;
            rasterParams.black_spans = callback;
            error = qt_ft_standard_raster.raster_render(*blackRaster, &rasterParams);
        }

        // Out of memory, reallocate some more and try again...
        if (error == -6) { // -6 is Result_err_OutOfMemory
            int new_size = rasterPoolSize * 2;
            if (new_size > 1024 * 1024) {
                qWarning("QPainter: Rasterization of primitive failed");
                return;
            }

#if defined(Q_WS_WIN64)
            _aligned_free(rasterPoolBase);
#else
            free(rasterPoolBase);
#endif

            rasterPoolSize = new_size;
            rasterPoolBase =
#if defined(Q_WS_WIN64)
                (unsigned char *) _aligned_malloc(rasterPoolSize, __alignof(void*));
#else
            (unsigned char *) malloc(rasterPoolSize);
#endif

            qt_ft_grays_raster.raster_new(0, grayRaster);
            qt_ft_grays_raster.raster_reset(*grayRaster, rasterPoolBase, rasterPoolSize);

            qt_ft_standard_raster.raster_new(0, blackRaster);
            qt_ft_standard_raster.raster_reset(*blackRaster, rasterPoolBase, rasterPoolSize);

        } else {
            done = true;
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

#if defined (Q_WS_MAC)
    if(m_ctx) {
        CGContextRelease(m_ctx);
        m_ctx = 0;
    }
    if(m_data) {
        CGImageRelease(m_data);
        m_data = 0;
    }
#endif

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
    clipEnabled = false;
    opaqueBackground = false;
    disabled_clip = 0;

    compositionMode = QPainter::CompositionMode_SourceOver;
    delete clip;
    clip = 0;
    format = QImage::Format_ARGB32_Premultiplied;
    drawHelper = qDrawHelper + QImage::Format_ARGB32_Premultiplied;
    bgBrush = Qt::white;
}


#if defined(Q_WS_MAC)
CGContextRef
QRasterBuffer::macCGContext() const
{
    if(!m_ctx && m_data) {
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4)
        uint flags = CGImageGetAlphaInfo(m_data);
        CGBitmapInfo (*CGImageGetBitmapInfo_ptr)(CGImageRef) = CGImageGetBitmapInfo;
        if(CGImageGetBitmapInfo_ptr)
            flags |= (*CGImageGetBitmapInfo_ptr)(m_data);
#else
        CGImageAlphaInfo flags = CGImageGetAlphaInfo(m_data);
#endif
        CGColorSpaceRef colorspace = CGColorSpaceCreateDeviceRGB();
        m_ctx = CGBitmapContextCreate(m_buffer, m_width, m_height, 8, m_width * 4, colorspace,
                                      flags);
        CGColorSpaceRelease(colorspace);
        if(!m_ctx)
            qWarning("QPaintDevice: Unable to create context for rasterbuffer (%d/%d)",
                     m_width, m_height);
        CGContextTranslateCTM(m_ctx, 0, m_height);
        CGContextScaleCTM(m_ctx, 1, -1);
    }
    return m_ctx;
}
#endif

#if defined(Q_WS_WIN)
void QRasterBuffer::setupHDC(bool clear_type)
{
    if (!clear_type) {
        SelectObject(m_hdc, GetStockObject(BLACK_BRUSH));
        SelectObject(m_hdc, GetStockObject(BLACK_PEN));
        SetTextColor(m_hdc, RGB(0, 0, 0));
    }
}
#endif

void QRasterBuffer::prepare(int w, int h)
{
    if (w<=m_width && h<=m_height) {
#ifdef Q_WS_MAC
        memset(m_buffer, 0, m_width*h*sizeof(uint));
#endif
        return;
    }

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

    format = image->format();
    drawHelper = qDrawHelper + format;
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
    m_buffer = new uchar[width*height*sizeof(uint)];
    memset(m_buffer, 255, width*height*sizeof(uint));
}
#elif defined(Q_WS_MAC)
static void qt_mac_raster_data_free(void *memory, const void *, size_t)
{
    free(memory);
}

void QRasterBuffer::prepare(QPixmap *pixmap)
{
    QPixmapData *data = pixmap->data;
    m_width = data->w;
    m_height = data->h;
    m_buffer = (uchar *)data->pixels;
    bytes_per_line = data->nbytes / data->h;
    m_data = data->cg_data;
    CGImageRetain(m_data);
}

void QRasterBuffer::prepareBuffer(int width, int height)
{
    m_buffer = new uchar[width*height*sizeof(uint)];
    memset(m_buffer, 0, width*height*sizeof(uint));

#ifdef QMAC_NO_COREGRAPHICS
# warning "Unhandled!!"
#else
    if (m_data) {
        CGImageRelease(m_data);
        m_data = 0;
    }
    if (m_ctx) {
        CGContextRelease(m_ctx);
        m_ctx = 0;
    }
    CGColorSpaceRef colorspace = CGColorSpaceCreateDeviceRGB();
    CGDataProviderRef provider = CGDataProviderCreateWithData(m_buffer, m_buffer, width*height*sizeof(uint),
                                                              qt_mac_raster_data_free);
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4)
    uint cgflags = kCGImageAlphaPremultipliedFirst;
#ifdef kCGBitmapByteOrder32Host //only needed because CGImage.h added symbols in the minor version
    if(QSysInfo::MacintoshVersion >= QSysInfo::MV_10_4)
        cgflags |= kCGBitmapByteOrder32Host;
#endif
#else
    CGImageAlphaInfo cgflags = kCGImageAlphaPremultipliedFirst;
#endif
    m_data = CGImageCreate(width, height, 8, 32, width*4, colorspace,
                           cgflags, provider, 0, 0, kCGRenderingIntentDefault);
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
    if (count == 0) {
        ymin = ymax = xmin = xmax = 0;
        return;
    }

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

void QSpanData::setup(const QBrush &brush, int alpha)
{
    Qt::BrushStyle brushStyle = brush.style();
    switch (brushStyle) {
    case Qt::SolidPattern:
        type = Solid;
        solid.color = PREMUL(ARGB_COMBINE_ALPHA(brush.color().rgba(), alpha));
        break;

    case Qt::LinearGradientPattern:
        {
            type = LinearGradient;
            const QLinearGradient *g = static_cast<const QLinearGradient *>(brush.gradient());
            gradient.alphaColor = !brush.isOpaque() || alpha != 256;
            initGradient(g, alpha);

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
            gradient.alphaColor = !brush.isOpaque() || alpha != 256;
            initGradient(g, alpha);

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
            gradient.alphaColor = !brush.isOpaque() || alpha != 256;
            initGradient(g, alpha);
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

            type = Texture;
            extern QPixmap qt_pixmapForBrush(int brushStyle, bool invert);
            QPixmap texture = brushStyle == Qt::TexturePattern
                              ? brush.texture() : qt_pixmapForBrush(brushStyle, true);
            if (texture.depth() == 1) {
                rasterBuffer->tempImage = rasterBuffer->colorizeBitmap(texture.toImage(),
                                                                       brush.color());
            } else {
                rasterBuffer->tempImage = texture.toImage();
            }
            initTexture(&rasterBuffer->tempImage, alpha, TextureData::Tiled);
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
    case LinearGradient:
    case RadialGradient:
    case ConicalGradient:
        unclipped_blend = rasterBuffer->drawHelper->blendGradient;
        break;
    case Texture:
        unclipped_blend = qBlendTexture;
        break;
    }
    // setup clipping
    if (!unclipped_blend) {
        blend = 0;
    } else if (rasterBuffer->clipEnabled) {
        blend = rasterBuffer->clip ? qt_span_fill_clipped : unclipped_blend;
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

extern const QVector<QRgb> *qt_image_colortable(const QImage &image);

void QSpanData::initTexture(const QImage *image, int alpha, TextureData::Type _type)
{
    texture.imageData = image->bits();
    texture.width = image->width();
    texture.height = image->height();
    texture.bytesPerLine = image->bytesPerLine();
    texture.format = image->format();
    texture.colorTable = qt_image_colortable(*image);
    texture.hasAlpha = image->format() != QImage::Format_RGB32 || alpha != 256;
    texture.const_alpha = alpha;
    texture.type = _type;

    adjustSpanMethods();
}

void QSpanData::initGradient(const QGradient *g, int alpha)
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
        gradient.colorTable[pos] = PREMUL(ARGB_COMBINE_ALPHA(stops[0].second.rgba(), alpha));
        ++pos;
    }

    qreal incr = 1 / qreal(GRADIENT_STOPTABLE_SIZE); // the double increment.
    qreal dpos = incr * pos; // The position in terms of 0-1.

    int current_stop = 0; // We always interpolate between current and current + 1.

    // Gradient area
    while (pos < end_pos) {

        Q_ASSERT(current_stop < stopCount);

        uint current_color = PREMUL(ARGB_COMBINE_ALPHA(stops[current_stop].second.rgba(), alpha));
        uint next_color = PREMUL(ARGB_COMBINE_ALPHA(stops[current_stop+1].second.rgba(), alpha));


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
        gradient.colorTable[pos] = PREMUL(ARGB_COMBINE_ALPHA(stops[stopCount-1].second.rgba(), alpha));
        ++pos;
    }

    gradient.spread = g->spread();
}

#ifdef Q_WS_WIN
static void draw_text_item_win(const QPointF &_pos, const QTextItemInt &ti, HDC hdc,
                               bool convertToText, const QMatrix &xform, const QPointF &topLeft)
{

    // Make sure we translate for systems that can't handle world transforms
    QPointF pos(QT_WA_INLINE(_pos, _pos + QPointF(xform.dx(), xform.dy())));
    QFontEngine *fe = ti.fontEngine;
    QPointF baseline_pos = xform.inverted().map(xform.map(pos) - topLeft);
  
    SetTextAlign(hdc, TA_BASELINE);
    SetBkMode(hdc, TRANSPARENT);

    bool has_kerning = ti.f && ti.f->kerning();

    SelectObject(hdc, fe->hfont);

    unsigned int options = (fe->ttf && !convertToText) ? ETO_GLYPH_INDEX : 0;

    wchar_t *convertedGlyphs = (wchar_t *)ti.chars;

    QGlyphLayout *glyphs = ti.glyphs;

    if (!(ti.flags & QTextItem::RightToLeft) && fe->useTextOutA) {
        qreal x = pos.x();
        qreal y = pos.y();

        // hack to get symbol fonts working on Win95. See also QFontEngine constructor
        // can only happen if !ttf
        for(int i = 0; i < ti.num_glyphs; i++) {
            QString str(QChar(glyphs->glyph));
            QByteArray cstr = str.toLocal8Bit();
            TextOutA(hdc, qRound(x + glyphs->offset.x.toReal()), 
                     qRound(y + glyphs->offset.y.toReal()),
                     cstr.data(), cstr.length());
            x += glyphs->advance.x.toReal();
            glyphs++;
        }
    } else {
        bool fast = !has_kerning;
        for(int i = 0; i < ti.num_glyphs; i++) {
            if (glyphs[i].offset.x != 0 || glyphs[i].offset.y != 0 || glyphs[i].space_18d6 != 0
                || glyphs[i].attributes.dontPrint) {
                fast = false;
                break;
            }            
        }

        // Scale, rotate and translate here. This is only valid for systems > Windows Me.
        // We should never get here on Windows Me or lower if the transformation specifies
        // scaling or rotation.        
        QT_WA({            
            XFORM win_xform;
            win_xform.eM11 = xform.m11();
            win_xform.eM12 = xform.m12();
            win_xform.eM21 = xform.m21();
            win_xform.eM22 = xform.m22();
            win_xform.eDx = xform.dx();
            win_xform.eDy = xform.dy();
            SetGraphicsMode(hdc, GM_ADVANCED);
            SetWorldTransform(hdc, &win_xform);
        }, {
            // nothing 
        });

        if (fast) {
            // fast path
            QVarLengthArray<wchar_t> g(ti.num_glyphs);
            for (int i = 0; i < ti.num_glyphs; ++i)
                g[i] = glyphs[i].glyph;            
            ExtTextOutW(hdc,
                        qRound(baseline_pos.x() + glyphs->offset.x.toReal()),
                        qRound(baseline_pos.y() + glyphs->offset.y.toReal()),
                        options, 0, convertToText ? convertedGlyphs : g.data(), ti.num_glyphs, 0);            
        } else {
            QVarLengthArray<QFixedPoint> positions;
            QVarLengthArray<glyph_t> _glyphs;

            QMatrix matrix;
            matrix.translate(baseline_pos.x(), baseline_pos.y());
            ti.fontEngine->getGlyphPositions(ti.glyphs, ti.num_glyphs, matrix, ti.flags, 
                _glyphs, positions);

            convertToText = convertToText && ti.num_glyphs == _glyphs.size();

            bool outputEntireItem = QT_WA_INLINE(_glyphs.size() > 0, false);

            if (outputEntireItem) {
                options |= ETO_PDY;
                QVarLengthArray<INT> glyphDistances(_glyphs.size() * 2);
                QVarLengthArray<wchar_t> g(_glyphs.size());
                for (int i=0; i<_glyphs.size() - 1; ++i) {
                    glyphDistances[i * 2] = qRound(positions[i + 1].x) - qRound(positions[i].x);
                    glyphDistances[i * 2 + 1] = qRound(positions[i + 1].y) - qRound(positions[i].y);
                    g[i] = _glyphs[i];
                }
                glyphDistances[(_glyphs.size() - 1) * 2] = 0;
                glyphDistances[(_glyphs.size() - 1) * 2 + 1] = 0;
                g[_glyphs.size() - 1] = _glyphs[_glyphs.size() - 1];
                ExtTextOutW(hdc, qRound(positions[0].x), qRound(positions[0].y), options, 0,
                            convertToText ? convertedGlyphs : g.data(), _glyphs.size(),
                            glyphDistances.data());
            } else {
                int i = 0;
                while(i < _glyphs.size()) {
                    wchar_t g = _glyphs[i];                    

                    ExtTextOutW(hdc, qRound(positions[i].x), 
                        qRound(positions[i].y), options, 0,
                                convertToText ? convertedGlyphs + i : &g, 1, 0);
                    ++i;
                }         
            }
        }    
        QT_WA({
            XFORM win_xform;            
            win_xform.eM11 = win_xform.eM22 = 1.0;
            win_xform.eM12 = win_xform.eM21 = win_xform.eDx = win_xform.eDy = 0.0;
            SetWorldTransform(hdc, &win_xform);
        }, {
            // nothing
        });
    }
}

void qt_draw_text_item(const QPointF &pos, const QTextItemInt &ti, HDC hdc,
                       bool convertToText, const QMatrix &xform, const QPointF &topLeft)
{
    Q_ASSERT(ti.fontEngine->type() != QFontEngine::Multi);
    draw_text_item_win(pos, ti, hdc, convertToText, xform, topLeft);
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
