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
#endif

#if defined(Q_WS_WIN64)
#  include <malloc.h>
#endif


#define qreal_to_fixed(f) (int(f * 64))
#define qt_swap(x, y) { int tmp = (x); (x) = (y); (y) = tmp; }

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
void qt_draw_text_item(const QPointF &point, const QTextItemInt &ti, HDC hdc,
                       QRasterPaintEnginePrivate *d);
#endif

// #define QT_DEBUG_DRAW
// #define QT_DEBUG_CONVERT

/********************************************************************************
 * Span functions
 */
typedef void (*qt_span_func)(int y, int count, QT_FT_Span *spans, void *userData);

void qt_span_fill_clipped(int y, int count, QT_FT_Span *spans, void *userData);
void qt_span_solidfill(int y, int count, QT_FT_Span *spans, void *userData);
void qt_span_texturefill(int y, int count, QT_FT_Span *spans, void *userData);
void qt_span_texturefill_xform(int y, int count, QT_FT_Span *spans, void *userData);
void qt_span_linear_gradient(int y, int count, QT_FT_Span *spans, void *userData);
void qt_span_radial_gradient(int y, int count, QT_FT_Span *spans, void *userData);
void qt_span_conical_gradient(int y, int count, QT_FT_Span *spans, void *userData);
void qt_span_clip(int y, int count, QT_FT_Span *spans, void *userData);

struct SolidFillData
{
    QRasterBuffer *rasterBuffer;
    uint color;
    BlendColor blendColor;
    QPainter::CompositionMode compositionMode;
};

struct TextureFillData
{
    QRasterBuffer *rasterBuffer;
    const void *imageData;
    int width, height;
    bool hasAlpha;
    qreal m11, m12, m21, m22, dx, dy;   // inverse xform matrix

    Blend blend;
    BlendTransformed blendFunc;

    QPainter::CompositionMode compositionMode;

    void init(QRasterBuffer *rasterBuffer, const QImage *image, const QMatrix &matrix,
              Blend b, BlendTransformed func);
};

struct FillData
{
    QRasterBuffer *rasterBuffer;
    qt_span_func callback;
    void *data;
};

struct ClipData
{
    QRasterBuffer *rasterBuffer;
    Qt::ClipOperation operation;
    int lastIntersected;
};

void qt_scanconvert(QT_FT_Outline *outline, qt_span_func callback, void *userData, QT_FT_BBox *bounds, QRasterPaintEnginePrivate *d);


/*******************************************************************************
 * Path iterators
 */
struct PathItData
{
    const QPainterPath *path;
    qreal m11, m12, m21, m22, dx, dy;
};

typedef const QPainterPath::Element &(*qt_path_iterator)(int index, PathItData *iteratorData);

const QPainterPath::Element &qt_path_iterator_xform(int index, PathItData *d)
{
    static QPainterPath::Element static_element;
    const QPainterPath::Element &e = d->path->elementAt(index);
    static_element.x = d->m11*e.x + d->m21*e.y + d->dx;
    static_element.y = d->m12*e.x + d->m22*e.y + d->dy;
    static_element.type = e.type;
    return static_element;
}

const QPainterPath::Element &qt_path_iterator_translate(int index, PathItData *d)
{
    static QPainterPath::Element static_element;
    const QPainterPath::Element &e = d->path->elementAt(index);
    static_element.x = e.x + d->dx;
    static_element.y = e.y + d->dy;
    static_element.type = e.type;
    return static_element;
}

const QPainterPath::Element &qt_path_iterator_noop(int index, PathItData *d)
{
    return d->path->elementAt(index);
}


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
      Resets the current outline.
    */
    void reset()
    {
        m_points.reset();
        m_tags.reset();
        m_contours.reset();
    }

    /*!
      Sets up the matrix to be used for conversion. This also
      sets up the qt_path_iterator function that is used as a callback
      to get points.
    */
    void setMatrix(const QMatrix &m, uint txop)
    {
        switch (txop) {
        case QPainterPrivate::TxNone:
            m_iterator = qt_path_iterator_noop;
            break;
        case QPainterPrivate::TxTranslate:
            m_iterator = qt_path_iterator_translate;
            break;
        default:
            m_iterator = qt_path_iterator_xform;
            break;
        }

        m_iterator_data.m11 = m.m11();
        m_iterator_data.m12 = m.m12();
        m_iterator_data.m21 = m.m21();
        m_iterator_data.m22 = m.m22();
        m_iterator_data.dx = m.dx();
        m_iterator_data.dy = m.dy();
    }

    QT_FT_Outline *convert(const QPainterPath &path)
    {
        QT_FT_Vector pt;
        QT_FT_Vector last = {0, 0};

        Q_ASSERT(!path.isEmpty());
        reset();

        m_iterator_data.path = &path;

        const QPainterPath::Element &startPt = m_iterator(0, &m_iterator_data);
        pt.x = qreal_to_fixed(startPt.x);
        pt.y = qreal_to_fixed(startPt.y);

        QT_FT_Vector start = { pt.x, pt.y };

#ifdef QT_DEBUG_CONVERT
        printf("moveto: %.2f, %.2f\n",
               start.x / 64.0, start.y / 64.0);
#endif

        m_points.add(pt);
        m_tags.add(QT_FT_CURVE_TAG_ON);

        int elmCount = path.elementCount();

        for (int index=1; index<elmCount; ++index) {
            const QPainterPath::Element &elm = m_iterator(index, &m_iterator_data);

            switch (elm.type) {

            case QPainterPath::MoveToElement:

                // If the path ends with a move to we can skip it...
                if (index == elmCount - 1)
                    continue;

                pt.x = qreal_to_fixed(elm.x);
                pt.y = qreal_to_fixed(elm.y);

#ifdef QT_DEBUG_CONVERT
                printf("moveto: %.2f, %.2f --  %.2f, %.2f\n",
                       pt.x / 64.0, pt.y / 64.0, start.x / 64.0, start.y / 64.0);
#endif

                // implicitly close the path
                if (start.x != last.x || start.y != last.y) {
                    m_points.add(start);
                    m_tags.add(QT_FT_CURVE_TAG_ON);
                }

                m_contours.add(m_points.size() - 1);
                m_points.add(pt);
                m_tags.add(QT_FT_CURVE_TAG_ON);
                start = pt;
                break;

            case QPainterPath::LineToElement:
                pt.x = qreal_to_fixed(elm.x);
                pt.y = qreal_to_fixed(elm.y);

#ifdef QT_DEBUG_CONVERT
                printf("lineto: %.2f, %.2f\n", pt.x  / 64.0, pt.y / 64.0);
#endif

                m_points.add(pt);
                m_tags.add(QT_FT_CURVE_TAG_ON);
                break;

            case QPainterPath::CurveToElement:
                pt.x = qreal_to_fixed(elm.x);
                pt.y = qreal_to_fixed(elm.y);
                m_points.add(pt);

                ++index;
                pt.x = qreal_to_fixed(m_iterator(index, &m_iterator_data).x);
                pt.y = qreal_to_fixed(m_iterator(index, &m_iterator_data).y);
                m_points.add(pt);

                ++index;
                pt.x = qreal_to_fixed(m_iterator(index, &m_iterator_data).x);
                pt.y = qreal_to_fixed(m_iterator(index, &m_iterator_data).y);
                m_points.add(pt);

#ifdef QT_DEBUG_CONVERT
                printf("curveto (end): %.2f, %.2f\n", pt.x / 64.0, pt.y / 64.0);
#endif

                m_tags.add(QT_FT_CURVE_TAG_CUBIC);         // Control point 1
                m_tags.add(QT_FT_CURVE_TAG_CUBIC);         // Control point 2
                m_tags.add(QT_FT_CURVE_TAG_ON);            // End point
                break;

            default:
                break; // This will never hit..
            }
            last = pt;
        }

        // close last element
        if (start.x != last.x || start.y != last.y) {
            m_points.add(start);
            m_tags.add(QT_FT_CURVE_TAG_ON);
        }
        // There will always be that last contour...
        m_contours.add(m_points.size() - 1);

        m_outline.n_contours = m_contours.size();
        m_outline.n_points = m_points.size();

        m_outline.points = m_points.data();
        m_outline.tags = m_tags.data();
        m_outline.contours = m_contours.data();

        m_outline.flags = path.fillRule() == Qt::WindingFill
                          ? QT_FT_OUTLINE_NONE
                          : QT_FT_OUTLINE_EVEN_ODD_FILL;


#ifdef QT_DEBUG_CONVERT
        printf("path has: %d\n", path.elementCount());

        printf("contours: %d\n", m_outline.n_contours);
        for (int i=0; i<m_outline.n_contours; ++i) {
            printf(" - %d\n", m_outline.contours[i]);
        }

        printf("points: %d\n", m_outline.n_points);
        for (int i=0; i<m_outline.n_points; ++i) {
            printf(" - %d -- %.2f, %.2f, (%d, %d)\n", i,
                   m_outline.points[i].x / 64.0,
                   m_outline.points[i].y / 64.0,
                   m_outline.points[i], m_outline.points[i]);
        }
#endif
        return &m_outline;
    }
public:
    QDataBuffer<QT_FT_Vector> m_points;
    QDataBuffer<char> m_tags;
    QDataBuffer<short> m_contours;
    QT_FT_Outline m_outline;
    PathItData m_iterator_data;
    qt_path_iterator m_iterator;
};


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

QHash<int, QImage> qt_raster_image_cache;

QRasterPaintEngine::QRasterPaintEngine()
    : QPaintEngine(*(new QRasterPaintEnginePrivate),
                   QPaintEngine::PaintEngineFeatures(AllFeatures)
        )
{
    Q_D(QRasterPaintEngine);

    d->rasterBuffer = new QRasterBuffer();
    d->fontRasterBuffer = new QRasterBuffer();
    d->outlineMapper = new QFTOutlineMapper;
    if (!qt_gray_raster || !qt_black_raster) {
        qt_initialize_ft();
    };

    d->fillData = new FillData;
    d->solidFillData = new SolidFillData;
    d->textureFillData = new TextureFillData;
    d->linearGradientData = new LinearGradientData;
    d->radialGradientData = new RadialGradientData;
    d->conicalGradientData = new ConicalGradientData;

    d->flushOnEnd = true;
}

QRasterPaintEngine::~QRasterPaintEngine()
{
    Q_D(QRasterPaintEngine);

    delete d->rasterBuffer;
    delete d->outlineMapper;
    delete d->fontRasterBuffer;

    delete d->fillData;
    delete d->solidFillData;
    delete d->textureFillData;
    delete d->linearGradientData;
    delete d->radialGradientData;
    delete d->conicalGradientData;
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

        if (format == QImage::Format_MonoLSB && isBitmap) {
            d->mono_surface = true;
            layout = DrawHelper::Layout_MonoLSB;
        } else if (format == QImage::Format_RGB32) {
            layout = DrawHelper::Layout_RGB32;
        } else if (format == QImage::Format_ARGB32_Premultiplied) {
            layout = DrawHelper::Layout_ARGB;
            gccaps |= PorterDuff;
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
        flush(d->pdev);

    d->clipEnabled = false;

    setActive(false);

    return true;
}


void QRasterPaintEngine::setFlushOnEnd(bool flushOnEnd)
{
    Q_D(QRasterPaintEngine);

    d->flushOnEnd = flushOnEnd;
}


/*!
  Force the contents of the buffer out on the underlying device.
*/
void QRasterPaintEngine::flush(QPaintDevice *device)
{
    Q_D(QRasterPaintEngine);
    Q_ASSERT(device);

#if defined(Q_WS_WIN)
    if (device->devType() == QInternal::Widget) {
        HDC hdc = device->getDC();
        Q_ASSERT(hdc);

        QRegion sysClip = systemClip();
        if (sysClip.isEmpty()) {
            BitBlt(hdc, d->deviceRect.x(), d->deviceRect.y(),
                   d->deviceRect.width(), d->deviceRect.height(),
                   d->rasterBuffer->hdc(), 0, 0, SRCCOPY);
        } else {
            QVector<QRect> rects = sysClip.rects();
            for (int i=0; i<rects.size(); ++i) {
                QRect r = rects.at(i);
                BitBlt(hdc,
                       r.x(), r.y(), r.width(), r.height(),
                       d->rasterBuffer->hdc(), r.x() - d->deviceRect.x(), r.y() - d->deviceRect.y(),
                       SRCCOPY);
            }
        }
        device->releaseDC(hdc);
        return;
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
#endif
#else
    Q_UNUSED(d);
#endif
}


void QRasterPaintEngine::updateState(const QPaintEngineState &state)
{
    Q_D(QRasterPaintEngine);

    QPaintEngine::DirtyFlags flags = state.state();

    if (flags & DirtyTransform) {
        d->matrix = state.matrix();
        if (d->matrix.m12() != 0 || d->matrix.m21() != 0)
            d->txop = QPainterPrivate::TxRotShear;
        else if (d->matrix.m11() != 1 || d->matrix.m22() != 1)
            d->txop = QPainterPrivate::TxScale;
        else if (d->matrix.dx() != 0 || d->matrix.dy() != 0)
            d->txop = QPainterPrivate::TxTranslate;
        else
            d->txop = QPainterPrivate::TxNone;
        d->outlineMapper->setMatrix(d->matrix, d->txop);
    }

    if (flags & DirtyPen) {
        d->pen = state.pen();
    }

    if ((flags & DirtyBrush) || (flags & DirtyBrushOrigin)) {
        QBrush brush = state.brush();
        QPointF offset = state.brushOrigin();
        d->brush = brush;
        d->brushOffset = offset;
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
            d->antialiased = bool(state.renderHints() & QPainter::Antialiasing);
            d->bilinear = bool(state.renderHints() & QPainter::SmoothPixmapTransform);
        }

        if (flags & DirtyCompositionMode) {
            d->compositionMode = state.compositionMode();
        }
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


QImage qt_map_to_32bit(const QPixmap &pixmap)
{
    QImage image = pixmap.toImage();
    return image.convertToFormat(image.hasAlphaChannel()
                                 ? QImage::Format_ARGB32_Premultiplied
                                 : QImage::Format_RGB32);
}

void QRasterPaintEngine::fillPath(const QPainterPath &path, FillData *fillData)
{
#ifdef QT_DEBUG_DRAW
    qDebug() << " --- fillPath, bounds=" << path.boundingRect();
#endif

    if (!fillData->callback)
        return;

    Q_D(QRasterPaintEngine);

    QT_FT_BBox clipBox = { 0, 0, d->deviceRect.width(), d->deviceRect.height() };

    Q_ASSERT(d->deviceRect.width() <= d->rasterBuffer->width());
    Q_ASSERT(d->deviceRect.height() <= d->rasterBuffer->height());

    qt_scanconvert(d->outlineMapper->convert(path), fillData->callback, fillData->data, &clipBox, d);
}


void QRasterPaintEngine::drawRects(const QRectF *rects, int rectCount)
{
#ifdef QT_DEBUG_DRAW
    qDebug(" - QRasterPaintEngine::drawRect(), x=%.2f, y=%.2f, width=%.2f, height=%.2f",
           r.x(), r.y(), r.width(), r.height());
#endif
    Q_D(QRasterPaintEngine);
    if (!d->antialiased
        && d->txop <= QPainterPrivate::TxTranslate) {

        bool hasPen = d->pen.style() != Qt::NoPen;
        qreal offset_x = d->matrix.dx();
        qreal offset_y = d->matrix.dy();

        QBrush oldBrush = d->brush;
        d->brush = QBrush();

        for (int i=0; i<rectCount; ++i) {
            QRectF rect = rects[i].normalized();
            rect.translate(offset_x, offset_y);

            FillData fillData = d->fillForBrush(oldBrush);
            int x1 = qMax(qRound(rect.x()), 0);
            int x2 = qMin(qRound(rect.width() + rect.x()), d->rasterBuffer->width());
            int y1 = qMax(qRound(rect.y()), 0);
            int y2 = qMin(qRound(rect.height() + rect.y()), d->rasterBuffer->height());;

            int len = x2 - x1;

            if (fillData.callback && len > 0) {
                QT_FT_Span span;
                span.x = x1;
                span.len = x2 - x1;
                span.coverage = 255;

                // draw the fill
                for (int y=y1; y<y2; ++y) {
                    fillData.callback(y, 1, &span, fillData.data);
                }
            }

            if (hasPen)
                QPaintEngine::drawRects(&rects[i], 1);
        }

        d->brush = oldBrush;
    } else {
        QPaintEngine::drawRects(rects, rectCount);
    }
}

void QRasterPaintEngine::drawPath(const QPainterPath &path)
{
#ifdef QT_DEBUG_DRAW
    QRectF bounds = path.boundingRect();
    printf(" - QRasterPaintEngine::drawPath(), [%.2f, %.2f, %.2f, %.2f]\n",
           bounds.x(), bounds.y(), bounds.width(), bounds.height());
#endif
    if (path.isEmpty())
        return;

    Q_D(QRasterPaintEngine);

    if (d->brush.style() != Qt::NoBrush) {
        d->outlineMapper->setMatrix(d->matrix, d->txop);
        FillData fillData = d->fillForBrush(d->brush);
        fillPath(path, &fillData);
    }

    if (d->pen.style() != Qt::NoPen) {
        QPainterPathStroker stroker;
        stroker.setDashPattern(d->pen.style());
        stroker.setCapStyle(d->pen.capStyle());
        stroker.setJoinStyle(d->pen.joinStyle());
        QPainterPath stroke;

        qreal width = d->pen.widthF();
        if (width == 0) {
            stroker.setWidth(1);
            d->outlineMapper->setMatrix(QMatrix(), QPainterPrivate::TxNone);
            stroke = stroker.createStroke(path * d->matrix);
            if (stroke.isEmpty())
                return;
        } else {
            stroker.setWidth(width);
            stroker.setCurveThreshold(1 / (10 * d->matrix.m11() * d->matrix.m22()));
            stroke = stroker.createStroke(path);
            d->outlineMapper->setMatrix(d->matrix, d->txop);
            if (stroke.isEmpty())
                return;
        }
        FillData fillData = d->fillForBrush(QBrush(d->pen.brush()));
        fillPath(stroke, &fillData);
    }

    d->outlineMapper->setMatrix(d->matrix, d->txop);
}


void QRasterPaintEngine::drawPolygon(const QPointF *points, int pointCount, PolygonDrawMode mode)
{
    Q_D(QRasterPaintEngine);
    QBrush oldBrush = d->brush;
    QPainterPath path(points[0]);
    for (int i=1; i<pointCount; ++i)
        path.lineTo(points[i]);
    if (mode == PolylineMode) {
        d->brush = QBrush();
    } else {
        path.setFillRule(mode == WindingMode ? Qt::WindingFill : Qt::OddEvenFill);
        path.closeSubpath();
    }
    drawPath(path);
    d->brush = oldBrush;
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
            && r.size() == sr.size()) {
            FillData fill = d->fillForBrush(QBrush(d->pen.color()));
            d->drawBitmap(r.topLeft() + QPointF(d->matrix.dx(), d->matrix.dy()), pixmap, &fill);
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
    TextureFillData textureData = {
        d->rasterBuffer,
        image.bits(), image.width(), image.height(), image.format() != QImage::Format_RGB32,
        0., 0., 0., 0., 0., 0.,
        d->drawHelper->blend,
        d->bilinear ? d->drawHelper->blendTransformedBilinear : d->drawHelper->blendTransformed,
        d->compositionMode
    };
    FillData fillData = { d->rasterBuffer, 0, &textureData };

    bool stretch_sr = r.width() != sr.width() || r.height() != sr.height();

    if (d->txop > QPainterPrivate::TxTranslate || stretch_sr) {
        fillData.callback = qt_span_texturefill_xform;
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
        fillData.callback = qt_span_texturefill;
        textureData.dx = -(r.x() + d->matrix.dx()) + sr.x();
        textureData.dy = -(r.y() + d->matrix.dy()) + sr.y();
    }

    QPainterPath path;
    path.addRect(r);

    FillData clippedFill = d->clipForFill(&fillData);

    bool wasAntialiased = d->antialiased;
    d->antialiased = d->bilinear;

    fillPath(path, &clippedFill);

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

    TextureFillData textureData = {
        d->rasterBuffer,
        ((const QImage &)(image)).bits(), image.width(), image.height(), image.format() != QImage::Format_RGB32,
        0., 0., 0., 0., 0., 0.,
        d->drawHelper->blendTiled,
        d->bilinear ? d->drawHelper->blendTransformedBilinearTiled : d->drawHelper->blendTransformedTiled,
        d->compositionMode
    };
    FillData fillData = { d->rasterBuffer, 0, &textureData };

    if (d->txop > QPainterPrivate::TxTranslate) {
        fillData.callback = qt_span_texturefill_xform;
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
        fillData.callback = qt_span_texturefill;
        textureData.dx = -( r.x() + d->matrix.dx()) + sr.x();
        textureData.dy = -( r.y() + d->matrix.dy()) + sr.y();
    }

    FillData clippedFill = d->clipForFill(&fillData);
    fillPath(path, &clippedFill);
}


#ifdef Q_WS_QWS
//QWS hack
static inline uchar monoVal(const uchar* s, int x)
{
    return  (s[x>>3] << (x&7)) & 0x80;
}
void QRasterPaintEngine::alphaPenBlt(const void* src, int bpl, bool mono, int rx,int ry,int w,int h)
{
    Q_D(QRasterPaintEngine);

    // Decide on which span func to use
    FillData fillData = d->fillForBrush(d->pen.brush());

    if (!fillData.callback)
        return;

    int y0 = (ry < 0) ? -ry : 0;
    int x0 = (rx < 0) ? -rx : 0;

    QRasterBuffer *rb = d->rasterBuffer;

    w = qMin(w, rb->width() - rx);
    h = qMin(h, rb->height() - ry);

    static QDataBuffer<QT_FT_Span> spans;

    for (int y=y0; y < h; ++y) {
        const uchar *scanline = static_cast<const uchar *>(src) + y*bpl;
        // Generate spans for this y coord
        spans.reset();

        if (mono) {
            for (int x = x0; x < w; ) {

                // Skip those with 0 coverage
                while (x < w && monoVal(scanline,x) == 0)
                    ++x;
                if (x >= w) break;

                int prev = monoVal(scanline,x);
                QT_FT_Span span = { x + rx, 0, prev*255 };

                // extend span until we find a different one.
                while (x < w && monoVal(scanline,x) == prev)
                    ++x;
                span.len = x +rx - span.x;

                spans.add(span);
            }
            // Call span func for current set of spans.
            fillData.callback(y + ry, spans.size(), spans.data(), fillData.data);

        } else {
            for (int x = x0; x < w; ) {
                // Skip those with 0 coverage
                while (x < w && scanline[x] == 0)
                    ++x;
                if (x >= w) break;

                int prev = scanline[x];
                QT_FT_Span span = { x + rx, 0, scanline[x] };

                // extend span until we find a different one.
                while (x < w && scanline[x] == prev)
                    ++x;
                span.len = x +rx - span.x;

                spans.add(span);
            }
        }
        // Call span func for current set of spans.
        fillData.callback(y + ry, spans.size(), spans.data(), fillData.data);
    }
}

void QRasterPaintEngine::qwsFillRect(int x, int y, int w, int h, const QBrush &brush)
{
    Q_D(QRasterPaintEngine);
    FillData fillData = d->fillForBrush(brush);
    int x1 = qMax(x,0);
    int x2 = qMin(x+w, d->rasterBuffer->width());
    int y1 = qMax(y, 0);
    int y2 = qMin(y+h, d->rasterBuffer->height());;

    int len = x2 - x1;

    if (fillData.callback && len > 0) {
        QT_FT_Span span;
        span.x = x1;
        span.len = x2 - x1;
        span.coverage = 255;

        // draw the fill
        for (int y=y1; y<y2; ++y) {
            fillData.callback(y, 1, &span, fillData.data);
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
    FillData fillData = fillForBrush(pen.brush());
    if (!fillData.callback)
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

    drawBitmap(devRect.topLeft(), bm, &fillData);
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
    d->fontRasterBuffer->resetBuffer(255);

    // Fill buffer with stuff
    qt_draw_text_item(QPoint(0, ti.ascent), ti, d->fontRasterBuffer->hdc(), d);

    // Decide on which span func to use
    FillData fillData = d->fillForBrush(d->pen.brush());

    if (!fillData.callback)
        return;

    // Boundaries
    int ymax = qMin(devRect.y() + devRect.height(), d->rasterBuffer->height());
    int ymin = qMax(devRect.y(), 0);
    int xmax = qMin(devRect.x() + devRect.width(), d->rasterBuffer->width());
    int xmin = qMax(devRect.x(), 0);

    static QDataBuffer<QT_FT_Span> spans;

    if (d->mono_surface) {
        for (int y=ymin; y<ymax; ++y) {
            QRgb *scanline = (QRgb *) d->fontRasterBuffer->scanLine(y - devRect.y()) - devRect.x();
            // Generate spans for this y coord
            spans.reset();
            for (int x = xmin; x<xmax; ) {
                // Skip those with 0 coverage (black on white so inverted)
                while (x < xmax && qGray(scanline[x]) > 0x80) ++x;
                if (x >= xmax) break;

                QT_FT_Span span = { x, 0, 255 };

                // extend span until we find a different one.
                while (x < xmax && qGray(scanline[x]) < 0x80) ++x;
                span.len = x - span.x;

                spans.add(span);
            }

            // Call span func for current set of spans.
            fillData.callback(y, spans.size(), spans.data(), fillData.data);
        }

    } else {
        for (int y=ymin; y<ymax; ++y) {
            QRgb *scanline = (QRgb *) d->fontRasterBuffer->scanLine(y - devRect.y()) - devRect.x();
            // Generate spans for this y coord
            spans.reset();
            for (int x = xmin; x<xmax; ) {
                // Skip those with 0 coverage (black on white so inverted)
                while (x < xmax && qGray(scanline[x]) == 255) ++x;
                if (x >= xmax) break;

                int prev = qGray(scanline[x]);
                QT_FT_Span span = { x, 0, 255 - prev };

                // extend span until we find a different one.
                while (x < xmax && qGray(scanline[x]) == prev) ++x;
                span.len = x - span.x;

                spans.add(span);
            }

            // Call span func for current set of spans.
            fillData.callback(y, spans.size(), spans.data(), fillData.data);
        }
    }

    return;

#elif defined Q_WS_QWS
    bool useFontEngine = true;
    QMatrix matrix = d->matrix();
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

enum LineDrawMode {
    LineDrawClipped,
    LineDrawNormal,
    LineDrawIncludeLastPixel
};

static inline void clipped_to_device(const QRect &deviceRect,
                              int y, QT_FT_Span *spans, qt_span_func func, void *userData)
{
    if (y >= 0 && y <= deviceRect.bottom()) {
        int span_right = spans->x + spans->len;
        int dev_right = deviceRect.right() + 1;
        int sleft = qMax(int(spans->x), 0);
        int sright = qMin(span_right, dev_right); // <=
        int len = sright - sleft;

        if (len > 0) {
            QT_FT_Span span = { sleft, len, 255 };
            func(y, 1, &span, userData);
        }
    }
}


// Bresenham algorithm from Graphics Gems
static void drawLine_bresenham(const QLineF &line, qt_span_func span_func, void *data, LineDrawMode style)
{
#ifdef QT_DEBUG_DRAW
    qDebug("drawLine_bresenham, x1=%.2f, y1=%.2f, x2=%.2f, y2=%.2f",
           line.x1(), line.y1(), line.x2(), line.y2());
#endif

    qreal x1 = line.x1();
    qreal x2 = line.x2();
    qreal y1 = line.y1();
    qreal y2 = line.y2();

    QRasterBuffer *rb = ((FillData *)data)->rasterBuffer;
    QRect deviceRect(0, 0, rb->width(), rb->height());

    int ax = int(qAbs(x2-x1)*256);
    int ay = int(qAbs(y2-y1)*256);
    int sx = x2 > x1 ? 1 : -1;
    int sy = y2 > y1 ? 1 : -1;
    int x = int(x1*256.);
    int dx = x & 0xff;
    x  = x >> 8;
    int y = int(y1*256.);
    int dy = y & 0xff;
    y = y >> 8;
    int xe = int(x2 + 0.5);
    int ye = int(y2 + 0.5);
    QT_FT_Span span;
    span.coverage = 255;
    span.len = 1;

    int d = (sy*ax*dy + sx*ay*dx) >> 8;
    if(ax > ay) {
        d += ay - (ax >> 1);
        if (dy >= 0x80) {
            ++y;
            d -= sy*ax;
        }
        if (dx >= 0x80)
            ++x;
        span.x = x;
        if (sx > 0) {
            while(x != xe) {
                if(d >= 0) {
                    span.len = x - span.x + 1;
                    clipped_to_device(deviceRect, y, &span, span_func, data);
                    span.x = x + 1;
                    y += sy;
                    d -= ax;
                }
                ++x;
                d += ay;
            }
            span.len = x - span.x;
            if (style == LineDrawIncludeLastPixel)
                ++span.len;
            clipped_to_device(deviceRect, y, &span, span_func, data);
        } else {
            while(x != xe) {
                if(d >= 0) {
                    span.len = span.x - x + 1;
                    span.x = x;
                    clipped_to_device(deviceRect, y, &span, span_func, data);
                    span.x = x - 1;
                    y += sy;
                    d -= ax;
                }
                --x;
                d += ay;
            }
            span.len = span.x - x;
            span.x = x;
            if (style != LineDrawIncludeLastPixel)
                ++span.x;
            else
                ++span.len;

            clipped_to_device(deviceRect, y, &span, span_func, data);
        }
    } else {
        d += (ax - (ay >> 1));
        if (dx >= 0x80) {
            ++x;
            d -= sx*ay;
        }
        if (dy >= 0x80)
            ++y;
        while(y != ye) {
            // y is dominant so we can't optimise the spans
            span.x = x;
            clipped_to_device(deviceRect, y, &span, span_func, data);
            if(d > 0) {
                x += sx;
                d -= ay;
            }
            y += sy;
            d += ax;
        }
        if (style == LineDrawIncludeLastPixel) {
            span.x = x;
            clipped_to_device(deviceRect, y, &span, span_func, data);
        }
    }
}

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
        FillData fillData = d->fillForBrush(d->pen.brush());
        if (!fillData.callback)
            return;

        QT_FT_Span span = { 0, 1, 255 };
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
                fillData.callback(y, 1, &span, fillData.data);
            }
            ++points;
        }
    }
}

void QRasterPaintEngine::drawLines(const QLineF *lines, int lineCount)
{
#ifdef QT_DEBUG_DRAW
    qDebug("\n - QRasterPaintEngine::drawLine(), x1=%.2f, y1=%.2f, x2=%.2f, y2=%.2f",
           l.x1(), l.y1(), l.x2(), l.y2());
#endif

    Q_D(QRasterPaintEngine);
    if (!d->antialiased
        && d->pen.style() == Qt::SolidLine
        && (d->pen.widthF() == 0
            || (d->pen.widthF() <= 1 && d->txop <= QPainterPrivate::TxTranslate))) {

        for (int i=0; i<lineCount; ++i) {
            QLineF line = lines[i] * d->matrix;
            LineDrawMode mode = LineDrawNormal;

            if (d->pen.capStyle() != Qt::FlatCap)
                mode = LineDrawIncludeLastPixel;

            FillData fillData = d->fillForBrush(QBrush(d->pen.brush()));
            drawLine_bresenham(line, fillData.callback, fillData.data, mode);
        }
        return;
    }
    QPaintEngine::drawLines(lines, lineCount);
}

void QRasterPaintEngine::drawEllipse(const QRectF &rect)
{
    Q_D(QRasterPaintEngine);
    if (!d->antialiased && d->pen.style() == Qt::NoPen && d->txop <= QPainterPrivate::TxTranslate) {
        QPen oldPen = d->pen;
        d->pen = QPen(d->brush, 0);
        QPaintEngine::drawEllipse(rect.adjusted(0, 0, -1, -1));
        d->pen = oldPen;
    } else {
        QPaintEngine::drawEllipse(rect);
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

void QRasterPaintEnginePrivate::drawBitmap(const QPointF &pos, const QPixmap &pm, FillData *fg)
{
    Q_ASSERT(fg);
    Q_ASSERT(fg->callback);

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

#if BITMAPS_ARE_MSB
    QImage::Format format = image.format();
#endif
    for (int y = ymin; y < ymax; ++y) {
        const uchar *src = image.scanLine(y - qRound(pos.y()));
#if BITMAPS_ARE_MSB
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
                    QT_FT_Span span = { xmin + x, 1, 255 };
                    while (src_x < w-1 && src[(src_x+1) >> 3] & (0x1 << ((src_x+1) & 7))) {
                        ++src_x;
                        ++span.len;
                    }
                    x += span.len;
                    spans[n] = span;
                    ++n;
                }
                if (n == spanCount) {
                    fg->callback(y, n, spans, fg->data);
                    n = 0;
                }
            }
#if BITMAPS_ARE_MSB
        } else {
            for (int x = 0; x < xmax - xmin; ++x) {
                bool set = src[x >> 3] & (0x80 >> (x & 7));
                if (set) {
                    QT_FT_Span span = { xmin + x, 1, 255 };
                    while (x < w-1 && src[(x+1) >> 3] & (0x80 >> ((x+1) & 7))) {
                        ++x;
                        ++span.len;
                    }

                    spans[n] = span;
                    ++n;
                }
                if (n == spanCount) {
                    fg->callback(y, n, spans, fg->data);
                    n = 0;
                }
            }
        }
#endif
        if (n) {
            fg->callback(y, n, spans, fg->data);
            n = 0;
        }
    }
}

/* Sets up potential clipping for this FillData object.
 * Note that the data object must be valid throughout the lifetime of
 * the return value.
 */
FillData QRasterPaintEnginePrivate::clipForFill(FillData *data)
{
    if (clipEnabled && data->callback) {
        FillData clipFillData = {
            data->rasterBuffer,
            qt_span_fill_clipped,
            data
        };
        return clipFillData;
    } else {
        return *data;
    }
}


FillData QRasterPaintEnginePrivate::fillForBrush(const QBrush &brush)
{
    Q_ASSERT(fillData);

    fillData->rasterBuffer = rasterBuffer;
    fillData->callback = 0;
    fillData->data = 0;

    switch (brush.style()) {

    case Qt::NoBrush:
        break;

    case Qt::SolidPattern:
        fillData->callback = qt_span_solidfill;
        fillData->data = solidFillData;
        solidFillData->color = PREMUL(brush.color().rgba());
        solidFillData->rasterBuffer = fillData->rasterBuffer;
        solidFillData->blendColor = drawHelper->blendColor;
        solidFillData->compositionMode = compositionMode;
        break;

    case Qt::TexturePattern:
        {
            tempImage = qt_map_to_32bit(brush.texture());
            fillData->data = textureFillData;
            fillData->callback = txop > QPainterPrivate::TxTranslate
                                 ? qt_span_texturefill_xform
                                 : qt_span_texturefill;
            textureFillData->compositionMode = compositionMode;
            textureFillData->init(rasterBuffer, &tempImage, brushMatrix(),
                                  drawHelper->blendTiled,
                                  bilinear
                                  ? drawHelper->blendTransformedBilinearTiled
                                  : drawHelper->blendTransformedTiled);
        }
        break;

    case Qt::LinearGradientPattern:
        {
            linearGradientData->rasterBuffer = fillData->rasterBuffer;
            linearGradientData->spread = brush.gradient()->spread();
            linearGradientData->stopCount = brush.gradient()->stops().size();
            linearGradientData->stopPoints = gradientStopPoints(brush.gradient());
            linearGradientData->stopColors = gradientStopColors(brush.gradient());
            const QLinearGradient *lg = static_cast<const QLinearGradient *>(brush.gradient());
            linearGradientData->origin = lg->start();
            linearGradientData->end = lg->finalStop();

            linearGradientData->brushMatrix = brushMatrix();
            linearGradientData->alphaColor = !brush.isOpaque();
            linearGradientData->init();
            linearGradientData->initColorTable();
            linearGradientData->blendFunc = drawHelper->blendLinearGradient;
            linearGradientData->compositionMode = compositionMode;
            fillData->callback = qt_span_linear_gradient;
            fillData->data = linearGradientData;
            break;
        }

    case Qt::RadialGradientPattern:
        {
            radialGradientData->rasterBuffer = fillData->rasterBuffer;
            radialGradientData->spread = brush.gradient()->spread();
            radialGradientData->stopCount = brush.gradient()->stops().size();
            radialGradientData->stopPoints = gradientStopPoints(brush.gradient());
            radialGradientData->stopColors = gradientStopColors(brush.gradient());
            radialGradientData->center =
                static_cast<const QRadialGradient *>(brush.gradient())->center();
            radialGradientData->radius =
                static_cast<const QRadialGradient *>(brush.gradient())->radius();
            radialGradientData->focal =
                static_cast<const QRadialGradient *>(brush.gradient())->focalPoint();
            radialGradientData->alphaColor = !brush.isOpaque();
            radialGradientData->initColorTable();
            radialGradientData->imatrix = brushMatrix().inverted();
            radialGradientData->blendFunc = drawHelper->blendRadialGradient;
            radialGradientData->compositionMode = compositionMode;

            fillData->data = radialGradientData;
            fillData->callback = qt_span_radial_gradient;
        }
        break;

    case Qt::ConicalGradientPattern:
        {
            conicalGradientData->rasterBuffer = fillData->rasterBuffer;
            conicalGradientData->spread = QGradient::RepeatSpread; // don't support any anyway
            conicalGradientData->stopCount = brush.gradient()->stops().size();
            conicalGradientData->stopPoints = gradientStopPoints(brush.gradient());
            conicalGradientData->stopColors = gradientStopColors(brush.gradient());
            conicalGradientData->alphaColor = !brush.isOpaque();
            conicalGradientData->compositionMode = compositionMode;
            conicalGradientData->blendFunc = drawHelper->blendConicalGradient;
            const QConicalGradient *cg = static_cast<const QConicalGradient *>(brush.gradient());
            conicalGradientData->init(cg->center(), cg->angle(), brushMatrix());
            fillData->data = conicalGradientData;
            fillData->callback = qt_span_conical_gradient;
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
        {
            extern QPixmap qt_pixmapForBrush(int brushStyle, bool invert);
            QPixmap pixmap = qt_pixmapForBrush(brush.style(), true);

            Q_ASSERT(!pixmap.isNull());
            Q_ASSERT(pixmap.depth() == 1);

            tempImage = colorizeBitmap(pixmap.toImage(), brush.color());
            fillData->data = textureFillData;
            fillData->callback = txop > QPainterPrivate::TxTranslate
                                 ? qt_span_texturefill_xform
                                 : qt_span_texturefill;
            textureFillData->compositionMode = compositionMode;
            textureFillData->init(rasterBuffer, &tempImage, brushMatrix(),
                                  drawHelper->blendTiled,
                                  bilinear
                                  ? drawHelper->blendTransformedBilinearTiled
                                  : drawHelper->blendTransformedTiled);
        }
        break;


    default:
        break;
    }

    return clipForFill(fillData);
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
    ClipData clipData = { rasterBuffer, op, 0 };

    switch (op) {
    case Qt::NoClip:
        rasterBuffer->resetClip();
        clipEnabled = false;
        return;
    case Qt::ReplaceClip:
        rasterBuffer->resetClip();
        break;
    case Qt::UniteClip:
        break;
    case Qt::IntersectClip:
        if (path.isEmpty())
            rasterBuffer->resetClip();
        clipData.lastIntersected = -1;
        break;
    }

    if (path.isEmpty())
        return;

    QT_FT_BBox clipBox = { 0, 0, rasterBuffer->width(), rasterBuffer->height() };
    qt_scanconvert(outlineMapper->convert(path), qt_span_clip, &clipData, &clipBox, this);

    // Need to reset the clipspans that where not touched during scan conversion.
    if (op == Qt::IntersectClip) {
        int start = clipData.lastIntersected + 1;
        rasterBuffer->resetClipSpans(start, rasterBuffer->height() - start);
    }
}

qreal *QRasterPaintEnginePrivate::gradientStopPoints(const QGradient *gradient)
{
    stopPoints.reset();
    QGradientStops stops = gradient->stops();
    for (int i=0; i<stops.size(); ++i) {
        Q_ASSERT(stops.at(i).first >= 0 && stops.at(i).first <= 1);
        stopPoints.add(stops.at(i).first);
    }
    return stopPoints.data();
}

uint *QRasterPaintEnginePrivate::gradientStopColors(const QGradient *gradient)
{
    stopColors.reset();
    QGradientStops stops = gradient->stops();
    for (int i=0; i<stops.size(); ++i)
        stopColors.add(PREMUL(stops.at(i).second.rgba()));
    return stopColors.data();
}


QImage QRasterPaintEnginePrivate::colorizeBitmap(const QImage &image, const QColor &color)
{
    Q_ASSERT(image.depth() == 1);

    QImage sourceImage = image.convertToFormat(QImage::Format_MonoLSB);
    QImage dest = QImage(sourceImage.size(), QImage::Format_ARGB32_Premultiplied);

    QRgb fg = PREMUL(color.rgba());
    QRgb bg = opaqueBackground ? PREMUL(bgBrush.color().rgba()) : 0;

    for (int y=0; y<sourceImage.height(); ++y) {
        uchar *source = sourceImage.scanLine(y);
        QRgb *target = reinterpret_cast<QRgb *>(dest.scanLine(y));
        for (int x=0; x < sourceImage.width(); ++x)
            target[x] = (source[x>>3] >> (x&7)) & 1 ? fg : bg;
    }
    return dest;
}

QRasterBuffer::~QRasterBuffer()
{
    if (m_clipSpanCount || m_clipSpanCapacity || m_clipSpans) {
        Q_ASSERT(m_clipSpanCount);
        qFree(m_clipSpanCount);

        Q_ASSERT(m_clipSpanCapacity);
        qFree(m_clipSpanCapacity);

        Q_ASSERT(m_clipSpans);
        for (int y=0; y<m_height; ++y)
            qFree((QT_FT_Span *)m_clipSpans[y]);
        qFree(m_clipSpans);
    }

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
    m_clipSpanCount = 0;
    m_clipSpanCapacity = 0;
    m_clipSpans = 0;
    m_clipSpanHeight = 0;
}


void QRasterBuffer::prepare(int w, int h)
{
    if (w<=m_width && h<=m_height)
        return;

    prepareBuffer(w, h);
    prepareClip(w, h);

    m_width = w;
    m_height = h;
    bytes_per_line = 4*m_width;
}


void QRasterBuffer::prepare(QImage *image)
{

    int depth = image->depth();
    if (depth == 32) {
        prepareClip(image->width(), image->height());
        m_buffer = (uchar *)image->bits();
    } else if (depth == 1) {
        prepareClip(image->width(), image->height());
        m_buffer = (uchar *)image->bits();
    } else {
        qWarning("QRasterBuffer::prepare() cannot prepare from image of depth=%d", depth);
    }
    m_width = image->width();
    m_height = image->height();
    bytes_per_line = 4*(depth == 32 ? m_width : (m_width + 31)/32);
}

void QRasterBuffer::prepareClip(int /*width*/, int height)
{
    if (height <= m_clipSpanHeight) {
        resetClipSpans(0, height);
    } else {

        m_clipSpanHeight = height;

        // clean up.. Should reuse old_height first elements for improved reallocs.
        if (m_clipSpanCount || m_clipSpanCapacity || m_clipSpans) {
            Q_ASSERT(m_clipSpanCount);
            qFree(m_clipSpanCount);

            Q_ASSERT(m_clipSpanCapacity);
            qFree(m_clipSpanCapacity);

            Q_ASSERT(m_clipSpans);
            for (int y=0; y<m_height; ++y)
                qFree((QT_FT_Span *)m_clipSpans[y]);
            qFree(m_clipSpans);
        }

        m_clipSpanCount = (int *) qMalloc(height * sizeof(int));
        m_clipSpanCapacity = (int *) qMalloc(height * sizeof(int));
        m_clipSpans = (QSpan **) qMalloc(height * sizeof(QT_FT_Span *));
        for (int y=0; y<height; ++y) {
            m_clipSpanCapacity[y] = 4;
            m_clipSpanCount[y] = 0;
            m_clipSpans[y] = (QSpan *) qMalloc(m_clipSpanCapacity[y] * sizeof(QSpan));
        }
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

void QRasterBuffer::appendClipSpan(int x, int y, int len, int coverage)
{
//     printf("QRasterBuffer::apendClipSpan(x=%d, y=%d, len=%d, oldSize=%d\n", x, y, len,
//            m_clipSpanCount[y]);

    QSpan *span = 0;

    int clipSpanCount = m_clipSpanCount[y];

    if (clipSpanCount == m_clipSpanCapacity[y])
        resizeClipSpan(y, clipSpanCount << 1);

//     Uncomment for sanity checking
//     for (int i=0; i<m_clipSpanCount[y]; ++i) {
//         QSpan *s = m_clipSpans[y] + i;
//         if (x < s->x + s->len) {
//             printf("bad append clip for: x=%d, y=%d, len=%d, cov=%d\n", x, y, len, coverage);
//             Q_ASSERT(0);
//         }
//     }

    span = m_clipSpans[y] + clipSpanCount;

    span->x = x;
    span->len = len;
    span->coverage = coverage;
    m_clipSpanCount[y] += 1;
}

void QRasterBuffer::replaceClipSpans(int y, QSpan *spans, int spanCount)
{
    if (m_clipSpanCapacity[y] < spanCount)
        resizeClipSpan(y, spanCount);
    memcpy(m_clipSpans[y], spans, spanCount * sizeof(QSpan));
    m_clipSpanCount[y] = spanCount;
}

void QRasterBuffer::resetClipSpans(int y, int count)
{
    memset(m_clipSpanCount + y, 0, count * sizeof(int));
}

void QRasterBuffer::resetClip()
{
    memset(m_clipSpanCount, 0, m_height * sizeof(int));
}

void QRasterBuffer::resizeClipSpan(int y, int size)
{
    Q_ASSERT(size > m_clipSpanCount[y]);
    m_clipSpans[y] = (QSpan *) qRealloc(m_clipSpans[y], size * sizeof(QSpan));
    m_clipSpanCapacity[y] = size;
}

void qt_span_solidfill(int y, int count, QT_FT_Span *spans, void *userData)
{
    SolidFillData *data = reinterpret_cast<SolidFillData *>(userData);
    QRasterBuffer *rb = data->rasterBuffer;
    uchar *rasterBuffer = rb->scanLine(y);
//     fprintf(stdout, "qt_span_solidfill, y=%d, count=%d rb->width=%d rb->bytes_per_line=%d\n", y, count, rb->width(), rb->bytesPerLine());
//     fflush(stdout);

    Q_ASSERT(y >= 0);
    Q_ASSERT(y < rb->height());

    BlendColorData bd;
    bd.color = data->color;
    bd.y = y;

    for (int span=0; span<count; ++span) {
        Q_ASSERT(spans->x >= 0);
        Q_ASSERT(spans->len > 0);
        Q_ASSERT(spans->x + spans->len <= rb->width());
        data->blendColor(rasterBuffer, (const QSpan *)spans, data->compositionMode, &bd);
        ++spans;
    }
}


void qt_span_texturefill(int y, int count, QT_FT_Span *spans, void *userData)
{
    TextureFillData *data = reinterpret_cast<TextureFillData *>(userData);
    QRasterBuffer *rb = data->rasterBuffer;
    int image_width = data->width;
    int image_height = data->height;
    int xoff = qRound(data->dx) % image_width;
    int yoff = qRound(data->dy) % image_height;

    if (xoff < 0)
        xoff += image_width;
    if (yoff < 0)
        yoff += image_height;

    uchar *baseTarget = rb->scanLine(y);
    while (count--) {
        QPainter::CompositionMode mode = data->compositionMode;
        if (!data->hasAlpha && mode == QPainter::CompositionMode_SourceOver
            && spans->coverage == 255)
            mode = QPainter::CompositionMode_Source;
        data->blend(baseTarget, (const QSpan *)spans, (xoff + spans->x)%image_width,
                    ((y + yoff) % image_height), data->imageData, image_width, image_height,
                    mode);
        ++spans;
    }
}

void qt_span_texturefill_xform(int y, int count, QT_FT_Span *spans, void *userData)
{
    TextureFillData *data = reinterpret_cast<TextureFillData *>(userData);
    QRasterBuffer *rb = data->rasterBuffer;
    int image_width = data->width;
    int image_height = data->height;
    uchar *baseTarget = rb->scanLine(y);

    // Base point for the inversed transform
    qreal ix = data->m21 * y + data->dx;
    qreal iy = data->m22 * y + data->dy;

    // The increment pr x in the scanline
    qreal dx = data->m11;
    qreal dy = data->m12;

    while (count--) {
        data->blendFunc(baseTarget, (const QSpan *)spans,
                        ix, iy, dx, dy,
                        data->imageData, image_width, image_height,
                        data->compositionMode);
        ++spans;
    }
}


uint qt_gradient_pixel(const GradientData *data, double pos)
{
    int ipos = qRound(pos * GRADIENT_STOPTABLE_SIZE - 1);

  // calculate the actual offset.
    if (ipos < 0 || ipos >= GRADIENT_STOPTABLE_SIZE) {
        if (data->spread == QGradient::RepeatSpread) {
            ipos = ipos % GRADIENT_STOPTABLE_SIZE;
            ipos = ipos < 0 ? GRADIENT_STOPTABLE_SIZE + ipos : ipos;

        } else if (data->spread == QGradient::ReflectSpread) {
            const int limit = GRADIENT_STOPTABLE_SIZE * 2 - 1;
            ipos = ipos % limit;
            ipos = ipos < 0 ? limit + ipos : ipos;
            ipos = ipos >= GRADIENT_STOPTABLE_SIZE ? limit - ipos : ipos;

        } else {
            if (ipos < 0) ipos = 0;
            else if (ipos >= GRADIENT_STOPTABLE_SIZE) ipos = GRADIENT_STOPTABLE_SIZE-1;
        }
    }

    Q_ASSERT(ipos >= 0);
    Q_ASSERT(ipos < GRADIENT_STOPTABLE_SIZE);

    return data->colorTable[ipos];
}


void qt_span_linear_gradient(int y, int count, QT_FT_Span *spans, void *userData)
{
    LinearGradientData *data = reinterpret_cast<LinearGradientData *>(userData);
    uchar *baseTarget = data->rasterBuffer->scanLine(y);

    qreal ybase = (y - data->origin.y()) * data->yincr;

    while (count--) {
        data->blendFunc(baseTarget, (const QSpan *)spans, data, ybase, y, data->compositionMode);
        ++spans;
    }
}

void qt_span_radial_gradient(int y, int count, QT_FT_Span *spans, void *userData)
{
    RadialGradientData *data = reinterpret_cast<RadialGradientData *>(userData);
    uchar *baseTarget = data->rasterBuffer->scanLine(y);

    while (count--) {
        data->blendFunc(baseTarget, (const QSpan *)spans, data, y, data->compositionMode);
        ++spans;
    }
}

void qt_span_conical_gradient(int y, int count, QT_FT_Span *spans, void *userData)
{
    ConicalGradientData *data = reinterpret_cast<ConicalGradientData *>(userData);
    uchar *baseTarget = data->rasterBuffer->scanLine(y);

    while (count--) {
        data->blendFunc(baseTarget, (const QSpan *)spans, data, y, data->compositionMode);
        ++spans;
    }
}

void qt_intersect_spans(QSpan *clipSpans, int clipSpanCount,
                        QT_FT_Span *spans, int spanCount,
                        QSpan **outSpans, int *outCount)
{
    static QDataBuffer<QSpan> newSpans;
    newSpans.reset();

    int spanIndex = 0;
    int clipSpanIndex = 0;

    int sx1, sx2, cx1, cx2;

    while (spanIndex < spanCount && clipSpanIndex < clipSpanCount) {
        sx1 = spans[spanIndex].x;
        sx2 = sx1 + spans[spanIndex].len;
        cx1 = clipSpans[clipSpanIndex].x;
        cx2 = cx1 + clipSpans[clipSpanIndex].len;

        if (cx1 < sx1 && cx2 < sx1) {
            ++clipSpanIndex;
        } else if (sx1 < cx1 && sx2 < cx1) {
            ++spanIndex;
        } else {
            QSpan newClip;
            newClip.x = qMax(sx1, cx1);
            newClip.len = qMin(sx2, cx2) - newClip.x;
            newClip.coverage = spans[spanIndex].coverage * clipSpans[clipSpanIndex].coverage / 255;
            if (newClip.len>0)
                newSpans.add(newClip);
            if (sx2 < cx2) {
                ++spanIndex;
            } else {
                ++clipSpanIndex;
            }
        }
    }

    *outSpans = newSpans.data();
    *outCount = newSpans.size();
}

void qt_unite_spans(QSpan *clipSpans, int clipSpanCount,
                    QT_FT_Span *spans, int spanCount,
                    QSpan **outSpans, int *outCount)
{


    static QDataBuffer<QSpan> newSpans;
    newSpans.reset();


    // ### will leak for now... BTW, this is a horrible algorithm, but then again it works...
    const int BUFFER_SIZE = 4096;
    static int *buffer = (int*) malloc(BUFFER_SIZE * sizeof(int));
    memset(buffer, 0, BUFFER_SIZE * sizeof(int));

    // Fill with old spans.
    for (int i=0; i<clipSpanCount; ++i) {
        QSpan *cs = clipSpans + i;
        for (int j=cs->x; j<cs->x + cs->len; ++j)
            buffer[j] += cs->coverage;
    }

    // Fill with new spans
    for (int i=0; i<spanCount; ++i) {
        QT_FT_Span *s = spans + i;
        for (int j=s->x; j<s->x + s->len; ++j) {
            buffer[j] += s->coverage;
            if (buffer[j] > 255) buffer[j] = 255;
        }
    }


    int maxClip = clipSpanCount > 0
                  ? clipSpans[clipSpanCount-1].x + clipSpans[clipSpanCount-1].len
                  : -1;
    int maxSpan = spanCount > 0
                  ? spans[spanCount-1].x + spans[spanCount-1].len
                  : -1;

    int max = qMax(maxClip, maxSpan);

    int x = 0;
    while (x<max) {

        // Skip to next span
        while (x < max && buffer[x] == 0) ++x;
        if (x >= max) break;

        QSpan sp;
        sp.x = x;
        sp.coverage = buffer[x];

        // Find length of span
        while (x < max && buffer[x] == sp.coverage) ++x;
        sp.len = x - sp.x;

        newSpans.add(sp);
    }

    *outSpans = newSpans.data();
    *outCount = newSpans.size();
}


void qt_span_fill_clipped(int y, int spanCount, QT_FT_Span *spans, void *userData)
{
    FillData *fillData = reinterpret_cast<FillData *>(userData);

    Q_ASSERT(fillData->callback);

    QRasterBuffer *rb = fillData->rasterBuffer;

    QSpan *clippedSpans = 0;
    int clippedSpanCount = 0;

    qt_intersect_spans(rb->clipSpans(y), rb->clipSpanCount(y),
                       spans, spanCount,
                       &clippedSpans, &clippedSpanCount);

    fillData->callback(y, clippedSpanCount, (QT_FT_Span *) clippedSpans, fillData->data);
}

void qt_span_clip(int y, int count, QT_FT_Span *spans, void *userData)
{
    ClipData *clipData = reinterpret_cast<ClipData *>(userData);
    QRasterBuffer *rb = clipData->rasterBuffer;

    switch (clipData->operation) {

    case Qt::IntersectClip:
        {
            QSpan *newSpans;
            int newSpanCount = 0;
            qt_intersect_spans(rb->clipSpans(y), rb->clipSpanCount(y),
                               spans, count,
                               &newSpans, &newSpanCount);

            // Clear the spans between last y spanned and this.
            for (int i=clipData->lastIntersected+1; i<y; ++i)
                rb->replaceClipSpans(i, 0, 0);
            clipData->lastIntersected = y;

            // Replace this
            rb->replaceClipSpans(y, newSpans, newSpanCount);
        }
        break;

    case Qt::UniteClip:
        {
            QSpan *newSpans;
            int newSpanCount = 0;
            qt_unite_spans(rb->clipSpans(y), rb->clipSpanCount(y),
                           spans, count,
                           &newSpans, &newSpanCount);

            rb->replaceClipSpans(y, newSpans, newSpanCount);
        }
        break;

    case Qt::ReplaceClip:
        for (int i=0; i<count; ++i) {
            rb->appendClipSpan(spans->x, y, spans->len, spans->coverage);
            ++spans;
        }
        break;
    case Qt::NoClip:
        break;
    }
}

void qt_scanconvert(QT_FT_Outline *outline, qt_span_func callback, void *userData,
                    QT_FT_BBox *boundingBox, QRasterPaintEnginePrivate *d)
{
    qt_span_func func = callback;
    void *data = userData;

    QT_FT_Raster_Params rasterParams;
    rasterParams.target = 0;
    rasterParams.source = outline;
    rasterParams.flags = QT_FT_RASTER_FLAG_CLIP;
    rasterParams.gray_spans = 0;
    rasterParams.black_spans = 0;
    rasterParams.bit_test = 0;
    rasterParams.bit_set = 0;
    rasterParams.user = data;
    rasterParams.clip_box = *boundingBox;

    if (d->antialiased) {
        rasterParams.flags |= (QT_FT_RASTER_FLAG_AA | QT_FT_RASTER_FLAG_DIRECT);
        rasterParams.gray_spans = func;
        int error = qt_ft_grays_raster.raster_render(qt_gray_raster, &rasterParams);
        if (error) {
            printf("qt_scanconvert(), gray raster failed...: %d\n", error);
        }
    } else {
        rasterParams.flags |= QT_FT_RASTER_FLAG_DIRECT;
        rasterParams.black_spans = func;
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
        QSpan *spans = clipSpans(y);
        int count = clipSpanCount(y);

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

void TextureFillData::init(QRasterBuffer *raster, const QImage *image, const QMatrix &matrix,
                           Blend b, BlendTransformed func)
{
    rasterBuffer = raster;
    imageData = (uint*) image->bits();
    width = image->width();
    height = image->height();
    hasAlpha = image->format() != QImage::Format_RGB32;

    QMatrix inv = matrix.inverted();
    m11 = inv.m11();
    m12 = inv.m12();
    m21 = inv.m21();
    m22 = inv.m22();
    dx = inv.dx();
    dy = inv.dy();

    blend = b;
    blendFunc = func;
}

void GradientData::initColorTable()
{
    Q_ASSERT(stopCount > 0);

    // The position where the gradient begins and ends
    int begin_pos = int(stopPoints[0] * GRADIENT_STOPTABLE_SIZE);
    int end_pos = int(stopPoints[stopCount-1] * GRADIENT_STOPTABLE_SIZE);

    int pos = 0; // The position in the color table.

    // Up to first point
    while (pos<=begin_pos) {
        colorTable[pos] = stopColors[0];
        ++pos;
    }

    qreal incr = 1 / qreal(GRADIENT_STOPTABLE_SIZE); // the double increment.
    qreal dpos = incr * pos; // The position in terms of 0-1.

    int current_stop = 0; // We always interpolate between current and current + 1.

    // Gradient area
    while (pos < end_pos) {

        Q_ASSERT(current_stop < stopCount);

        uint current_color = stopColors[current_stop];
        uint next_color = stopColors[current_stop+1];

        int dist = (int)(256*(dpos - stopPoints[current_stop])
                         / (stopPoints[current_stop+1] - stopPoints[current_stop]));
        int idist = 256 - dist;

        colorTable[pos] = INTERPOLATE_PIXEL_256(current_color, idist, next_color, dist);

        ++pos;
        dpos += incr;

        if (dpos > stopPoints[current_stop+1]) {
            ++current_stop;
        }
    }

    // After last point
    while (pos < GRADIENT_STOPTABLE_SIZE) {
        colorTable[pos] = stopColors[stopCount-1];
        ++pos;
    }
}

/**
 * Initialzes the xincr and yincr delta values that is used to interpolate the Linear Gradient
 *
 * The deltas are found by projecting the gradientline down to a horizontal (xincr) or vertical (yincr)
 * line that covers the whole gradient (from 0 to 1.0).
 * Given that the gradient line is d, the transformed normal vector is n, we use this formula to
 * find the length of the side in the triangle is supposed to interpolate over the gradient:
 * _     _                _             _
 * d + a*n = [l,0],       where d = [dx, dy], n = [nx, ny], l is the length of the line
 *
 * rearranging, we get the length of line like this:
 *
 * l = dx - dy*nx/ny;  =>  xincr = 1.0/l
 *
 * We calculate yincr similarly:
 * l = dy - dx*ny/nx;  =>  yincr = 1.0/l
 *
 *
 * We then find the length of that line, and divides the length of the gradient (1.0) by the length
 * of the line (in pixels)
 *
 *
 */
void LinearGradientData::init()
{
    qreal x1 = origin.x();
    qreal y1 = origin.y();
    qreal x2 = end.x();
    qreal y2 = end.y();

#ifdef QT_DEBUG_DRAW
    qDebug("LinearGradientData::init(), x1=%f, y1=%f, x2=%f, y2=%f, spread=%d",
           x1, y1, x2, y2, spread);
    for (int i=0; i<stopCount; ++i) {
        qDebug(" - %d, pos=%f, color=%x", i, stopPoints[i], stopColors[i]);
    }
#endif

    // Calculate the normalvector and transform it.
    QLineF n = brushMatrix.map(QLineF(x1, y1, x2, y2).normalVector() );

    origin = brushMatrix.map(origin);
    end = brushMatrix.map(end);

    x1 = origin.x();
    y1 = origin.y();
    x2 = end.x();
    y2 = end.y();

    qreal dx = x2 - x1;
    qreal dy = y2 - y1;

    // qDebug() << "(" << x1 << "," << y1 << ")";
    // qDebug() << "(" << x2 << "," << y2 << ")";

    qreal nx = n.dx();
    qreal ny = n.dy();

    // Find the length of the projection
    qreal l = dx - dy*nx/ny;

    // qDebug() << "b: " << b << "dx: " << dx << "dy: " << dy << "nx: " << nx;
    xincr = 1.0/l;

    l = dy - dx*ny/nx;
    yincr = 1.0/l;

    // qDebug() << "inc: " << xincr << "," << yincr;

}


void ConicalGradientData::init(const QPointF &pt, qreal a, const QMatrix &matrix)
{
    center = pt;
    angle = a * 2 * Q_PI / 360.0;
    imatrix = matrix.inverted();

    initColorTable();
};


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
    qreal x = p.x();
    qreal y = p.y();

    if (d->txop >= QPainterPrivate::TxScale
        && !(QSysInfo::WindowsVersion & QSysInfo::WV_NT_based)) {
        // Draw rotated and sheared text on Windows 95, 98

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

            if (haveOffsets || transform) {
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

void qt_draw_text_item(const QPointF &pos, const QTextItemInt &ti, HDC hdc,
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
