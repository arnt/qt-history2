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
    unsigned char *poolBase = (unsigned char *) malloc(poolSize);

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

QImage qt_draw_radial_gradient_image( const QRect &rect, RadialGradientData *rdata );
QImage qt_draw_conical_gradient_image(const QRect &rect, ConicalGradientData *cdata);

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
        QT_FT_Vector pt, last;

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
                   QPaintEngine::PaintEngineFeatures(AllFeatures))
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
    d->compositionMode = QPainter::CompositionMode_SourceOver;

    d->deviceRect = QRect(0, 0, device->width(), device->height());

    DrawHelper::Layout layout = DrawHelper::Layout_RGB32;

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

#ifdef Q_WS_WIN
    if (device->devType() == QInternal::Pixmap) {
        QPixmap *pixmap = static_cast<QPixmap *>(device);
        if (pixmap->isNull()) {
            qWarning("Cannot paint on a null pixmap");
            return false;
        }
        QPixmapData *data = static_cast<QPixmap *>(device)->data;
        device = &data->image;
    }
#endif

    if (device->devType() == QInternal::Image) {
        QImage *image = static_cast<QImage *>(device);
        d->flushOnEnd = image->format() != QImage::Format_ARGB32_Premultiplied;
        d->rasterBuffer->prepare(image);
        if (image->format() != QImage::Format_RGB32)
            layout = DrawHelper::Layout_ARGB;
#ifdef Q_WS_QWS
    } else if (device->devType() == QInternal::Pixmap) {
        QPixmap *pix = static_cast<QPixmap *>(device);
        if (pix->depth() != 32) {
            qWarning("QRasterPaintEngine::begin(), only 32 bit pixmaps are supported at this time");
            return false;
        }
        // is this the right place to do clipping ???
        //### shouldn't this clipping be platform independent ???
        QRegion sysClip = systemClip();
        if (!sysClip.isEmpty()) {
            d->baseClip.addRegion(sysClip);
            // qDebug() << "adding to clip:" << sysClip;
        }
        d->flushOnEnd = false; // Direct access so no flush.
        d->rasterBuffer->prepare(pix);
        if (pix->hasAlphaChannel())
            layout =  DrawHelper::Layout_ARGB;
#endif
    } else {
        d->rasterBuffer->prepare(d->deviceRect.width(), d->deviceRect.height());
    }

    d->rasterBuffer->resetClip();

    d->drawHelper = qDrawHelper + layout;

    d->matrix = QMatrix();
    d->txop = QPainterPrivate::TxNone;
    d->brushMatrix = QMatrix();
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
        switch (target->format()) {
        case QImage::Format_Mono:
        case QImage::Format_MonoLSB:
            d->rasterBuffer->flushTo1BitImage(target);
            break;

        case QImage::Format_RGB32:
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
        d->penMatrix = d->matrix;
    }

    if ((flags & DirtyBrush) || (flags & DirtyBrushOrigin)) {
        QBrush brush = state.brush();
        QPointF offset = state.brushOrigin();
        d->brush = brush;
        d->brushMatrix = d->matrix;
        d->brushOffset = offset;

        // Offset the brush matrix with offset.
        d->brushMatrix.translate(offset.x(), offset.y());
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

    if (flags & DirtyHints) {
        d->antialiased = bool(state.renderHints() & QPainter::Antialiasing);
        d->bilinear = bool(state.renderHints() & QPainter::SmoothPixmapTransform);
    }

    if (flags & DirtyCompositionMode) {
        d->compositionMode = state.compositionMode();
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
        FillData fillData = d->fillForBrush(d->brush, &path);
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
        FillData fillData = d->fillForBrush(QBrush(d->pen.brush()), &stroke);
        fillPath(stroke, &fillData);
    }

    d->outlineMapper->setMatrix(d->matrix, d->txop);
}


void QRasterPaintEngine::drawPolygon(const QPointF *points, int pointCount, PolygonDrawMode mode)
{
    Q_D(QRasterPaintEngine);
    QBrush oldBrush = d->brush;
    QMatrix oldMatrix = d->brushMatrix;
    QPainterPath path(points[0]);
    for (int i=1; i<pointCount; ++i)
        path.lineTo(points[i]);
    if (mode == PolylineMode) {
        d->brush = QBrush();
        d->brushMatrix = d->matrix;
    } else {
        path.setFillRule(mode == WindingMode ? Qt::WindingFill : Qt::OddEvenFill);
        path.closeSubpath();
    }
    drawPath(path);
    d->brush = oldBrush;
    d->brushMatrix = oldMatrix;
}


void QRasterPaintEngine::drawPixmap(const QRectF &r, const QPixmap &pixmap, const QRectF &sr)
{
#ifdef QT_DEBUG_DRAW
    qDebug() << " - QRasterPaintEngine::drawPixmap(), r=" << r << " sr=" << sr << " pixmap=" << pixmap.size() << "depth=" << pixmap.depth();
#endif

    Q_D(QRasterPaintEngine);

    QImage image;
    if (pixmap.depth() == 1)
        image = d->colorizeBitmap(pixmap.toImage(), d->pen.color());
    else
        image = qt_map_to_32bit(pixmap);
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
    fillPath(path, &clippedFill);
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
        textureData.dx = -( + d->matrix.dx()) + sr.x();
        textureData.dy = -( + d->matrix.dy()) + sr.y();
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
    FillData fillData = d->fillForBrush(d->pen.brush(), 0);

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
#endif

#ifdef Q_WS_X11
void QRasterPaintEngine::drawTextItem(const QPointF &p, const QTextItem &textItem)
{
    const QTextItemInt &ti = static_cast<const QTextItemInt &>(textItem);

#ifdef QT_DEBUG_DRAW
    printf(" - QRasterPaintEngine::drawTextItem(), (%.2f,%.2f), string=%s\n",
           p.x(), p.y(), QString::fromRawData(ti.chars, ti.num_chars).toLatin1().data());
#endif
    Q_D(QRasterPaintEngine);

    if (ti.fontEngine->type() == QFontEngine::Freetype) {
        bool aa = d->antialiased;
        d->antialiased = true;
        QPaintEngine::drawTextItem(p, ti);
        d->antialiased = aa;
        return;
    }

    // xlfd: draw into bitmap, convert to image and rasterize that

    // Decide on which span func to use
    FillData fillData = d->fillForBrush(d->pen.brush(), 0);
    if (!fillData.callback)
        return;

    QRectF logRect(p.x(), p.y() - ti.ascent, ti.width, ti.ascent + ti.descent);
    QRect devRect = d->matrix.mapRect(logRect).toRect();

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
//        qDebug() << "w" << w << "h" << h << "y" << metrics.y;

        painter.drawTextItem(QPointF(0, ti.ascent), item);
        if (d->txop > QPainterPrivate::TxTranslate)
            bm = bm.transformed(QImage::trueMatrix(d->matrix, w, h));
    }

    QImage image = bm.toImage();
    Q_ASSERT(image.depth() == 1);

    const int spanCount = 256;
    QT_FT_Span spans[spanCount];
    int n = 0;

    // Boundaries
    int ymax = qMin(devRect.y() + devRect.height(), d->rasterBuffer->height());
    int ymin = qMax(devRect.y(), 0);
    int xmax = qMin(devRect.x() + devRect.width(), d->rasterBuffer->width());
    int xmin = qMax(devRect.x(), 0);

    QImage::Format format = image.format();
    for (int y = ymin; y < ymax; ++y) {
        uchar *src = image.scanLine(y - devRect.y());
        if (format == QImage::Format_MonoLSB) {
            for (int x = 0; x < xmax - xmin; ++x) {
                bool set = src[x >> 3] & (0x1 << (x & 7));
                if (set) {
                    QT_FT_Span span = { xmin + x, 1, 255 };
                    while (x < w-1 && src[(x+1) >> 3] & (0x1 << ((x+1) & 7))) {
                        ++x;
                        ++span.len;
                    }

                    spans[n] = span;
                    ++n;
                }
                if (n == spanCount) {
                    fillData.callback(y, n, spans, fillData.data);
                    n = 0;
                }
            }
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
                    fillData.callback(y, n, spans, fillData.data);
                    n = 0;
                }
            }
        }
        if (n) {
            fillData.callback(y, n, spans, fillData.data);
            n = 0;
        }
    }

}


#else

void QRasterPaintEngine::drawTextItem(const QPointF &p, const QTextItem &textItem)
{
    const QTextItemInt &ti = static_cast<const QTextItemInt &>(textItem);

#if defined(Q_WS_MAC)
    return;
#endif

#ifdef QT_DEBUG_DRAW
    printf(" - QRasterPaintEngine::drawTextItem(), (%.2f,%.2f), string=%s\n",
           p.x(), p.y(), QString::fromRawData(ti.chars, ti.num_chars).toLatin1().data());
#endif
    Q_D(QRasterPaintEngine);
#if defined(Q_WS_WIN)

    QRectF logRect(p.x(), p.y() - ti.ascent, ti.width, ti.ascent + ti.descent);
    QRect devRect = d->matrix.mapRect(logRect).toRect();

    if(devRect.width() == 0 || devRect.height() == 0)
        return;

    d->fontRasterBuffer->prepare(devRect.width(), devRect.height());
    d->fontRasterBuffer->resetBuffer(255);

    // Fill buffer with stuff
    qt_draw_text_item(QPoint(0, ti.ascent), ti, d->fontRasterBuffer->hdc(), d);

    // Decide on which span func to use
    FillData fillData = d->fillForBrush(d->pen.brush(), 0);

    if (!fillData.callback)
        return;

    // Boundaries
    int ymax = qMin(devRect.y() + devRect.height(), d->rasterBuffer->height());
    int ymin = qMax(devRect.y(), 0);
    int xmax = qMin(devRect.x() + devRect.width(), d->rasterBuffer->width());
    int xmin = qMax(devRect.x(), 0);

    static QDataBuffer<QT_FT_Span> spans;
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

#else
    bool aa = d->antialiased;
    d->antialiased = true;
    QPaintEngine::drawTextItem(p, ti);
    d->antialiased = aa;
    return;
#endif // Q_WS_WIN
}
#endif

enum LineDrawMode {
    LineDrawClipped,
    LineDrawNormal,
    LineDrawIncludeLastPixel
};

static LineDrawMode clipLine(QLineF *line, const QRect &rect)
{
    LineDrawMode mode = LineDrawNormal;

    qreal x1 = line->x1();
    qreal x2 = line->x2();
    qreal y1 = line->y1();
    qreal y2 = line->y2();

    qreal left = rect.x();
    qreal right = rect.x() + rect.width() - 1;
    qreal top = rect.y();
    qreal bottom = rect.y() + rect.height() - 1;

    enum { Left, Right, Top, Bottom };
    // clip the lines, after cohen-sutherland, see e.g. http://www.nondot.org/~sabre/graphpro/line6.html
    int p1 = ((x1 < left) << Left)
             | ((x1 > right) << Right)
             | ((y1 < top) << Top)
             | ((y1 > bottom) << Bottom);
    int p2 = ((x2 < left) << Left)
             | ((x2 > right) << Right)
             | ((y2 < top) << Top)
             | ((y2 > bottom) << Bottom);

    if (p1 & p2)
        // completely outside
        return LineDrawClipped;

    if (p1 | p2) {
        qreal dx = x2 - x1;
        qreal dy = y2 - y1;

        // clip x coordinates
        if (x1 < left) {
            y1 += dy/dx * (left - x1);
            x1 = left;
        } else if (x1 > right) {
            y1 -= dy/dx * (x1 - right);
            x1 = right;
        }
        if (x2 < left) {
            y2 += dy/dx * (left - x2);
            x2 = left;
            mode = LineDrawIncludeLastPixel;
        } else if (x2 > right) {
            y2 -= dy/dx * (x2 - right);
            x2 = right;
            mode = LineDrawIncludeLastPixel;
        }
        p1 = ((y1 < top) << Top)
             | ((y1 > bottom) << Bottom);
        p2 = ((y2 < top) << Top)
             | ((y2 > bottom) << Bottom);
        if (p1 & p2)
            return LineDrawClipped;
        // clip y coordinates
        if (y1 < top) {
            x1 += dx/dy * (top - y1);
            y1 = top;
        } else if (y1 > bottom) {
            x1 -= dx/dy * (y1 - bottom);
            y1 = bottom;
        }
        if (y2 < top) {
            x2 += dx/dy * (top - y2);
            y2 = top
                 ;
            mode = LineDrawIncludeLastPixel;
        } else if (y2 > bottom) {
            x2 -= dx/dy * (y2 - bottom);
            y2 = bottom;
            mode = LineDrawIncludeLastPixel;
        }
        *line = QLineF(QPointF(x1, y1), QPointF(x2, y2));
    }
    return mode;
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

    Q_ASSERT(x1 >= 0);
    Q_ASSERT(x1 < rb->width());
    Q_ASSERT(x2 >= 0);
    Q_ASSERT(x2 < rb->width());
    Q_ASSERT(y1 >= 0);
    Q_ASSERT(y1 < rb->height());
    Q_ASSERT(y2 >= 0);
    Q_ASSERT(y2 < rb->height());

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
                    span_func(y, 1, &span, data);
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
            if (span.len > 0 && span.x >= 0 && span.x + span.len <= rb->width() && y >= 0 && y < rb->height())
                span_func(y, 1, &span, data);
        } else {
            while(x != xe) {
                if(d >= 0) {
                    span.len = span.x - x + 1;
                    span.x = x;
                    span_func(y, 1, &span, data);
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

            if (span.x >= 0 && span.x + span.len < rb->width() && y >= 0 && y < rb->height())
                span_func(y, 1, &span, data);
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
            span_func(y, 1, &span, data);
            if(d > 0) {
                x += sx;
                d -= ay;
            }
            y += sy;
            d += ax;
        }
        if (style == LineDrawIncludeLastPixel) {
            if (x >= 0 && x < rb->width() && y >= 0 && y < rb->height()) {
                span.x = x;
                span_func(y, 1, &span, data);
            }
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
            LineDrawMode mode = clipLine(&line, QRect(QPoint(0, 0), d->deviceRect.size()));

            if (mode == LineDrawClipped)
                continue;

            if (mode == LineDrawNormal && d->pen.capStyle() != Qt::FlatCap)
                mode = LineDrawIncludeLastPixel;

            FillData fillData = d->fillForBrush(QBrush(d->pen.brush()), 0);
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


FillData QRasterPaintEnginePrivate::fillForBrush(const QBrush &brush, const QPainterPath *path)
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
            textureFillData->init(rasterBuffer, &tempImage, brushMatrix,
                                  drawHelper->blendTiled,
                                  bilinear
                                  ? drawHelper->blendTransformedBilinearTiled
                                  : drawHelper->blendTransformedTiled);
        }
        break;

    case Qt::LinearGradientPattern:
        linearGradientData->rasterBuffer = fillData->rasterBuffer;
        linearGradientData->spread = brush.gradient()->spread();
        linearGradientData->stopCount = brush.gradient()->stops().size();
        linearGradientData->stopPoints = gradientStopPoints(brush.gradient());
        linearGradientData->stopColors = gradientStopColors(brush.gradient());
        linearGradientData->origin =
            static_cast<const QLinearGradient *>(brush.gradient())->start() * brushMatrix;
        linearGradientData->end =
            static_cast<const QLinearGradient *>(brush.gradient())->finalStop() * brushMatrix;
        linearGradientData->alphaColor = !brush.isOpaque();
        linearGradientData->init();
        linearGradientData->initColorTable();
        linearGradientData->blendFunc = drawHelper->blendLinearGradient;
        linearGradientData->compositionMode = compositionMode;
        fillData->callback = qt_span_linear_gradient;
        fillData->data = linearGradientData;
        break;

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
            QRectF bounds = path ? path->boundingRect() : QRectF(deviceRect);
            tempImage = qt_draw_radial_gradient_image(bounds.toRect(), radialGradientData);

            fillData->data = textureFillData;
            fillData->callback = txop > QPainterPrivate::TxTranslate
                                 ? qt_span_texturefill_xform
                                 : qt_span_texturefill;
            textureFillData->compositionMode = compositionMode;
            textureFillData->init(rasterBuffer, &tempImage, matrix,
                                  drawHelper->blendTiled,
                                  drawHelper->blendTransformedBilinearTiled);
        }
        break;

    case Qt::ConicalGradientPattern:
        {
            conicalGradientData->rasterBuffer = fillData->rasterBuffer;
            conicalGradientData->spread = QGradient::RepeatSpread; // don't support any anyway
            conicalGradientData->stopCount = brush.gradient()->stops().size();
            conicalGradientData->stopPoints = gradientStopPoints(brush.gradient());
            conicalGradientData->stopColors = gradientStopColors(brush.gradient());
            conicalGradientData->center =
                static_cast<const QConicalGradient *>(brush.gradient())->center();
            conicalGradientData->angle =
                static_cast<const QConicalGradient *>(brush.gradient())->angle();
            conicalGradientData->alphaColor = !brush.isOpaque();
            conicalGradientData->initColorTable();
            QRectF bounds = path ? path->boundingRect() : QRectF(deviceRect);
            tempImage = qt_draw_conical_gradient_image(bounds.toRect(), conicalGradientData);

            fillData->data = textureFillData;
            fillData->callback = txop > QPainterPrivate::TxTranslate
                                 ? qt_span_texturefill_xform
                                 : qt_span_texturefill;
            textureFillData->compositionMode = compositionMode;
            textureFillData->init(rasterBuffer, &tempImage, matrix,
                                  drawHelper->blendTiled,
                                  drawHelper->blendTransformedBilinearTiled);
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
            textureFillData->init(rasterBuffer, &tempImage, brushMatrix,
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
}

void QRasterBuffer::init()
{
    m_clipSpanCount = 0;
    m_clipSpanCapacity = 0;
    m_clipSpans = 0;
}


void QRasterBuffer::prepare(int w, int h)
{
    if (w<=m_width && h<=m_height)
        return;

    prepareBuffer(w, h);
    prepareClip(w, h);

    m_width = w;
    m_height = h;
}


void QRasterBuffer::prepare(QImage *image)
{

    int depth = image->depth();
    if (depth == 32) {
        prepareClip(image->width(), image->height());
        m_buffer = (uint *)image->bits();
    } else if (depth == 1) {
        prepare(image->width(), image->height());
        uint table[2] = { image->color(0), image->color(1) };
        for (int y=0; y<image->height(); ++y) {
            uint *bscan = scanLine(y);
            // ### use image scanlines directly
            for (int x=0; x<image->width(); ++x) {
                bscan[x] = table[image->pixelIndex(x, y)];
            }
        }
    } else {
        qWarning("QRasterBuffer::prepare() cannot prepare from image of depth=%d", depth);
    }
    m_width = image->width();
    m_height = image->height();
}

#ifdef Q_WS_QWS
void QRasterBuffer::prepare(QPixmap *pixmap)
{
    prepareClip(pixmap->width(), pixmap->height());

    m_buffer = (uint *)pixmap->qwsScanLine(0);

    m_width = pixmap->width();
    m_height = pixmap->height();
}
#endif

void QRasterBuffer::prepareClip(int /*width*/, int height)
{
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
    m_buffer = new uint[width*height];
    memset(m_buffer, 255, width*height*sizeof(uint));
}
#elif defined(Q_WS_MAC)
static void qt_mac_raster_data_free(void *, const void *data, size_t)
{
    free(const_cast<void *>(data));
}

void QRasterBuffer::prepareBuffer(int width, int height)
{
    m_buffer = new uint[width*height*sizeof(uint)];
    memset(m_buffer, 255, width*height*sizeof(uint));

#ifdef QMAC_NO_COREGRAPHICS
# warning "Unhandled!!"
#else
    if (m_data)
        CGImageRelease(m_data);
    CGColorSpaceRef colorspace = CGColorSpaceCreateDeviceRGB();
    CGDataProviderRef provider = CGDataProviderCreateWithData(0, m_buffer, width*height,
                                                              qt_mac_raster_data_free);
    m_data = CGImageCreate(width, height, 8, 32, width, colorspace,
                           kCGImageAlphaFirst, provider, 0, 0, kCGRenderingIntentDefault);
    CGColorSpaceRelease(colorspace);
    CGDataProviderRelease(provider);
#endif

}
#elif defined(Q_WS_QWS)
void QRasterBuffer::prepareBuffer(int width, int height)
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

    if (m_clipSpanCount[y] == m_clipSpanCapacity[y])
        resizeClipSpan(y, m_clipSpanCapacity[y] * 2);

#ifdef QT_DEBUG
    for (int i=0; i<m_clipSpanCount[y]; ++i) {
        QSpan *s = m_clipSpans[y] + i;
        if (x < s->x + s->len) {
            printf("bad append clip for: x=%d, y=%d, len=%d, cov=%d\n", x, y, len, coverage);
            Q_ASSERT(0);
        }
    }
#endif

    span = m_clipSpans[y] + m_clipSpanCount[y];

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
//     fprintf(stdout, "qt_span_solidfill, y=%d, count=%d\n", y, count);
//     fflush(stdout);
    uint color = data->color;
    QRasterBuffer *rb = data->rasterBuffer;
    uint *rasterBuffer = rb->scanLine(y);

    Q_ASSERT(y >= 0);
    Q_ASSERT(y < rb->height());

    for (int span=0; span<count; ++span) {
        Q_ASSERT(spans->x >= 0);
        Q_ASSERT(spans->len > 0);
        Q_ASSERT(spans->x + spans->len <= rb->width());
        uint *target = rasterBuffer + (int) spans->x;
        data->blendColor(target, (const QSpan *)spans, color, data->compositionMode);
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

    const uint *scanline = (const uint *)data->imageData + ((y+yoff) % image_height) * image_width;
    uint *baseTarget = rb->scanLine(y);
    bool opaque = !data->hasAlpha || data->compositionMode == QPainter::CompositionMode_Source;
    while (count--) {
        uint *target = baseTarget + spans->x;
        if (opaque && spans->coverage == 255) {
            int span_x = spans->x;
            int span_len = spans->len;
            while (span_len > 0) {
                int image_x = (span_x + xoff) % image_width;
                int len = qMin(image_width - image_x, span_len);
                Q_ASSERT(image_x >= 0);
                Q_ASSERT(image_x + len <= image_width); // inclusive since it is used as upper bound.
                Q_ASSERT(span_x + len <= rb->width());
                memcpy(target, scanline + image_x, len * sizeof(uint));
                span_x += len;
                span_len -= len;
                target += len;
            }
        } else {
            data->blend(target, (const QSpan *)spans, (xoff + spans->x)%image_width,
                        ((y + yoff) % image_height), data->imageData, image_width, image_height,
                        data->compositionMode);
        }
        ++spans;
    }
}

void qt_span_texturefill_xform(int y, int count, QT_FT_Span *spans, void *userData)
{
    TextureFillData *data = reinterpret_cast<TextureFillData *>(userData);
    QRasterBuffer *rb = data->rasterBuffer;
    int image_width = data->width;
    int image_height = data->height;
    uint *baseTarget = rb->scanLine(y);

    qreal ix = data->m21 * y + data->dx;
    qreal iy = data->m22 * y + data->dy;

    qreal dx = data->m11;
    qreal dy = data->m12;

    while (count--) {
        data->blendFunc(baseTarget + spans->x, (const QSpan *)spans,
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
    uint *baseTarget = data->rasterBuffer->scanLine(y);

    qreal ybase = (y - data->origin.y()) * data->yincr;

    while (count--) {
        uint *target = baseTarget + spans->x;
        data->blendFunc(target, (const QSpan *)spans, data, ybase,
                        data->compositionMode);
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

    fflush(stdout);
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
        uint *span = const_cast<QRasterBuffer *>(this)->scanLine(y);

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
        uint *sourceLine = const_cast<QRasterBuffer *>(this)->scanLine(y);
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

void QRasterBuffer::flushTo1BitImage(QImage *target) const
{
    int w = qMin(m_width, target->width());
    int h = qMin(m_height, target->height());

    // ### Direct scanline access
    for (int y=0; y<h; ++y) {
        uint *sourceLine = const_cast<QRasterBuffer *>(this)->scanLine(y);
        int y_mod_16 = y & 15;
        for (int x=0; x<w; ++x) {
            uint p = sourceLine[x];
            target->setPixel(x, y, qGray(p) >= int(qt_bayer_matrix[y_mod_16][x&15]) ? 0 : 1);
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

    qreal dx = x2 - x1;
    qreal dy = y2 - y1;
    qreal len = sqrt(dx * dx + dy * dy);

    dx /= (len * len);
    dy /= (len * len);

    xincr = dx;
    yincr = dy;
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
            bool haveOffsets = true;
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

QImage qt_draw_radial_gradient_image( const QRect &rect, RadialGradientData *rdata )
{
    int width = rect.width();
    int height = rect.height();

    QImage image(width, height, rdata->alphaColor ? QImage::Format_ARGB32_Premultiplied : QImage::Format_RGB32);
    if ( rdata->stopCount == 0 ) {
        image.fill( 0 );
        return image;
    }

    if ( rdata->radius <= 0. ) {
        image.fill(qt_gradient_pixel(rdata, 0));
        return image;
    }

    qreal r, x0, y0, fx, fy, sw, sh, a, b, c, dc, d2c, dba, ba, rad, dx, dy, drad, d2rad, p, d2y;
    int i, j;
    uint *line;

    r = rdata->radius;
    x0 = ( rect.left() - rdata->center.x() ) / r;
    y0 = -( rect.top() - rdata->center.y() ) / r;
    fx = ( rdata->focal.x() - rdata->center.x() ) / r;
    fy = -( rdata->focal.y() - rdata->center.y() ) / r;
    sw = width / r;
    sh = height / r;

    a = 1 - fx * fx - fy * fy;
    if ( a <= 0 ) {
        qreal f = sqrt( fx * fx + fy * fy );
        fx = 0.999 * fx / f;
        fy = 0.999 * fy / f;
        a = 1 - fx * fx - fy * fy;
    }

    dx = x0 - fx;
    dy = y0 - fy;
    dc = 2 * dx / r + 1 / ( r * r );
    d2c = 2 * 1 / ( r * r );
    dba = fx / r / a;

    d2y = -1. / r;

    for ( i = 0; i < height; i++ ) {
        line = (uint *) image.scanLine( i );
        b = dx * fx + dy * fy;
        c = dx * dx + dy * dy;
        ba = b / a;
        rad = ba * ba + c / a;
        drad = 2 * ba * dba + dba * dba + dc / a;
        d2rad = 2 * dba * dba + d2c / a;

        for ( j = 0; j < width; j++ ) {
            p = ba + ( rad < 0 ? 0 : sqrt( rad ) );
            line[j] = qt_gradient_pixel( rdata, p );
            ba += dba;
            rad += drad;
            drad += d2rad;
        }
        dy += d2y;
    }

    return image;
} // qt_draw_gradient_pixmap

static const double cg_tan[ 5 ] = { -1., -.5880025, 0., .5880025, 1. };
static const double cg_po0[ 5 ] = { 3./8., 2./6., 1./4., 1./6., 1./8. };
static const double cg_po1[ 5 ] = { 3./8., 5./12., 2./4., 7./12., 5./8. };
static const double cg_po2[ 5 ] = { 5./8., 4./6., 3./4., 5./6., 7./8. };
static const double cg_po3[ 5 ] = { 1./8., 1./12., 1., 11./12., 7./8. };
static const double cg_dpr[ 5 ] = { .0955, .1405, .1405, .0955, .0 };

QImage qt_draw_conical_gradient_image(const QRect &rect, ConicalGradientData *cdata)
{
    int width = rect.width();
    int height = rect.height();

    QImage image(width, height, cdata->alphaColor ? QImage::Format_ARGB32_Premultiplied : QImage::Format_RGB32);

    if ( cdata->stopCount == 0 ) {
        image.fill( 0 );
        return image;
    }

    double dx0, dy0, da, dy, dx, dny, dnx, nx, ny, p, dp, rp;
    int i, j, si;
    bool mdp;
    uint *line;

    p = 0;
    dp = 0;

    dx0 = rect.x() - cdata->center.x();
    dy0 = rect.y() - cdata->center.y();
    da = cdata->angle / 2. / Q_PI;

    dy = dy0;

    if ( floor( dx0 ) == dx0 && floor( dy0 ) == dy0 )
        image.setPixel(-(int)dx0, -(int)dy0, qt_gradient_pixel( cdata, 0. ));

    for ( i = 0; i < height; i++ ) {
        if ( dy == 0. ) {
            dy += 1;
            continue;
        }
        line = (uint *) image.scanLine( i );
        dnx = fabs( 1. / dy );
        nx = dx0 * dnx;

        j = 0;
        while ( nx < -1.000001 && j < width ) {
            nx += dnx;
            j++;
        }
        mdp = true;
        si = 0;
        while ( nx < 1.000001 && j < width ) {
            while ( nx >= cg_tan[ si + 1 ] && nx < 1. ) {
                si++;
                mdp = true;
            }
            if ( mdp ) {
                dp = cg_dpr[ si ] / dy;
                p = ( dy < 0. ? cg_po0[ si ] : cg_po2[ si ] ) + ( dy > 0. ? 1. : -1. ) * ( nx - cg_tan[ si ] ) * cg_dpr[ si ];
                mdp = false;
            }
            rp = p - da;
            if ( rp < 0.001 && rp > -0.001 )
                rp = 0.;
            line[ j ] = qt_gradient_pixel( cdata, rp );
            p += dp;
            nx += dnx;
            j++;
        }
        dy += 1;
    }
    dx = dx0;
    for ( i = 0; i < width; i++ ) {
        if ( dx == 0. ) {
            dx += 1;
            continue;
        }
        line = (uint *) image.scanLine( i );
        dny = fabs( 1. / dx );
        ny = dy0 * dny;
        si = 0;

        j = 0;
        while ( ny < -1.000001 && j < height ) {
            ny += dny;
            j++;
        }
        mdp = true;
        si = 0;
        while ( ny < 1.000001 && j < height ) {
            while ( ny >= cg_tan[ si + 1 ] && ny < 1. ) {
                si++;
                mdp = true;
            }
            if ( mdp ) {
                dp = -cg_dpr[ si ] / dx;
                p = ( dx < 0. ? cg_po1[ si ] : cg_po3[ si ] ) + ( dx < 0. ? 1. : -1. ) * ( ny - cg_tan[ si ] ) * cg_dpr[ si ];
                mdp = false;
            }
            rp = p - da;
            if ( rp < 0.001 && rp > -0.001 )
                rp = 0.;
            image.setPixel(i, j, qt_gradient_pixel( cdata, rp ));
            p += dp;
            ny += dny;
            j++;
        }
        dx += 1;
    }

    return image;
}

