#define QT_FT_BEGIN_HEADER
#define QT_FT_END_HEADER
#include <private/qrasterdefs_p.h>
#include <private/qgrayraster_p.h>
#include <private/qblackraster_p.h>

#include <qpainterpath.h>
#include <qdebug.h>
#include <qhash.h>
#include <qlabel.h>

#include <math.h>

#include <private/qdatabuffer_p.h>
#include <private/qpainter_p.h>
#include <private/qtextengine_p.h>

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

#ifndef M_PI
#define M_PI 3.14159265358979323846
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
    ARGB color;
    QRasterPaintEnginePrivate::RasterOperation rop;
    BlendColor blendColor;
};

struct TextureFillData
{
    QRasterBuffer *rasterBuffer;
    ARGB *imageData;
    int width, height;
    bool hasAlpha;
    qreal m11, m12, m21, m22, dx, dy;   // inverse xform matrix

    Blend blend;
    BlendTransformed blendFunc;

    void init(QRasterBuffer *rasterBuffer, QImage *image, const QMatrix &matrix,
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

#define GRADIENT_STOPTABLE_SIZE 1024

struct GradientData
{
    QRasterBuffer *rasterBuffer;
    QGradient::Spread spread;

    int stopCount;
    qreal *stopPoints;
    ARGB *stopColors;

    ARGB colorTable[GRADIENT_STOPTABLE_SIZE];

    uint alphaColor : 1;

    void initColorTable();
};

struct LinearGradientData : public GradientData
{
    QPointF origin;
    QPointF end;

    void init();

    qreal xincr;
    qreal yincr;
};

struct RadialGradientData : public GradientData
{
    QPointF center;
    qreal radius;
    QPointF focal;
};

struct ConicalGradientData : public GradientData
{
    QPointF center;
    qreal angle;
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
                   QPaintEngine::PaintEngineFeatures(
                                                     LineAntialiasing
                                                     | FillAntialiasing
                                                     | PainterPaths
                                                     | AlphaFill
                                                     | AlphaStroke
                                                     | BrushStroke
                                                     | LinearGradientFill
                                                     | RadialGradientFill
                                                     | ConicalGradientFill
                                                     | ClipTransform
                                                     | CoordTransform
                                                     | PenWidthTransform
                                                     | PatternTransform
                                                     | PixmapTransform
                                                     | PixmapScale
                                                     | PatternBrush))
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
    d->rasterOperation = device->depth() == 1
                         ? QRasterPaintEnginePrivate::SourceCopy
                         : QRasterPaintEnginePrivate::SourceOverComposite;

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

    if (device->devType() == QInternal::Image) {
        if (device->depth() != 32) {
            qWarning("QRasterPaintEngine::begin(), only 32 bit images are supported at this time");
            return false;
        }
        d->flushOnEnd = false; // Direct access so no flush.
        d->rasterBuffer->prepare(static_cast<QImage *>(device));
        if (static_cast<QImage *>(device)->hasAlphaBuffer())
            layout =  DrawHelper::Layout_ARGB;
    } else {
        d->rasterBuffer->prepare(d->deviceRect.width(), d->deviceRect.height());
    }

    d->rasterBuffer->resetClip();

    // Copy contents of pixmap over to ourselves...
    if (device->devType() == QInternal::Pixmap) {
#ifdef Q_WS_WIN
        HDC pmhdc = device->getDC();
        BitBlt(d->rasterBuffer->hdc(), 0, 0, device->width(), device->height(),
               pmhdc, 0, 0, SRCCOPY);
        device->releaseDC(pmhdc);
#else
        Q_ASSERT_X(false, "QRasterPaintEngine::begin()", "Painting on pixmaps not supported for this platform");
#endif
        if (static_cast<QPixmap *>(device)->hasAlphaChannel())
            layout =  DrawHelper::Layout_ARGB;
    }
    d->drawHelper = qDrawHelper + layout;

    updateMatrix(QMatrix());

    if (device->depth() == 1) {
        updatePen(QPen(Qt::color1));
        updateBrush(QBrush(Qt::color0), QPointF());
    } else {
        updatePen(QPen(Qt::black));
        updateBrush(QBrush(Qt::NoBrush), QPointF());
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
    HDC hdc = device->getDC();
    Q_ASSERT(hdc);

    // blt using paint event clip bounding rect
    BitBlt(hdc,
           d->deviceRect.x(), d->deviceRect.y(), d->deviceRect.width(), d->deviceRect.height(),
           d->rasterBuffer->hdc(), 0, 0,
           SRCCOPY);
    device->releaseDC(hdc);
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

void QRasterPaintEngine::updatePen(const QPen &pen)
{
#ifdef QT_DEBUG_DRAW
    qDebug(" - QRasterPaintEngine::updatePen(), style=%d, color=%08x",
           pen.style(), pen.color().rgba());
#endif
    Q_D(QRasterPaintEngine);
    d->pen = pen;
    d->penMatrix = d->matrix;
}


void QRasterPaintEngine::updateFont(const QFont &)
{
}


void QRasterPaintEngine::updateBackground(Qt::BGMode bgMode, const QBrush &bgBrush)
{
    Q_D(QRasterPaintEngine);

    d->opaqueBackground = (bgMode == Qt::OpaqueMode);
    d->bgBrush = bgBrush;
}


void QRasterPaintEngine::updateMatrix(const QMatrix &m)
{
    Q_D(QRasterPaintEngine);

    d->matrix = m;
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


void QRasterPaintEngine::updateClipRegion(const QRegion &r, Qt::ClipOperation op)
{
#ifdef QT_DEBUG_DRAW
    qDebug() << " - QRasterPaintEngine::updateClipRegion() op=" << op << r;
#endif
    QPainterPath p;
    p.addRegion(r);
    updateClipPath(p, op);
}


void QRasterPaintEngine::updateRenderHints(QPainter::RenderHints hints)
{
    Q_D(QRasterPaintEngine);
    d->antialiased = bool(hints & QPainter::Antialiasing);
    d->bilinear = bool(hints & QPainter::SmoothPixmapTransform);
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


void QRasterPaintEngine::updateBrush(const QBrush &brush, const QPointF &offset)
{
#ifdef QT_DEBUG_DRAW
    qDebug() << " - QRasterPaintEngine::updateBrush()" << brush.style() << brush.color();
#endif
    Q_D(QRasterPaintEngine);
    d->brush = brush;
    d->brushMatrix = d->matrix;

    // Offset the brush matrix with offset.
    d->brushMatrix.translate(offset.x(), offset.y());
}


// ### Use a decent cache for this..
QImage *qt_image_for_pixmap(const QPixmap &pixmap)
{
    int serial = pixmap.serialNumber();
    if (!qt_raster_image_cache.contains(serial)) {
        if (qt_raster_image_cache.size() > 32)
            qt_raster_image_cache.clear();
        qt_raster_image_cache[serial] = pixmap.toImage().convertDepth(32);
    }
    Q_ASSERT(qt_raster_image_cache.contains(serial));
    return &qt_raster_image_cache[serial];
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

    if (fillData->callback) {
        if (d->clipEnabled) {
            qt_scanconvert(d->outlineMapper->convert(path), qt_span_fill_clipped, fillData,
                           &clipBox, d);
        } else {
            qt_scanconvert(d->outlineMapper->convert(path), fillData->callback, fillData->data,
                           &clipBox, d);
        }
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

    FillData fillData = { d->rasterBuffer, 0, 0 };

    if (d->brush.style() != Qt::NoBrush) {
        d->outlineMapper->setMatrix(d->matrix, d->txop);
        d->fillForBrush(d->brush, &fillData, &path);
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
        d->fillForBrush(QBrush(d->pen.brush()), &fillData, &stroke);
        fillPath(stroke, &fillData);
    }

    d->outlineMapper->setMatrix(d->matrix, d->txop);
}


void QRasterPaintEngine::drawPixmap(const QRectF &r, const QPixmap &pixmap, const QRectF &sr,
                                    Qt::PixmapDrawingMode)
{
#ifdef QT_DEBUG_DRAW
    qDebug() << " - QRasterPaintEngine::drawPixmap(), r=" << r << " sr=" << sr << " pixmap=" << pixmap.size() << "depth=" << pixmap.depth();
#endif

    Q_D(QRasterPaintEngine);

    QImage *image = qt_image_for_pixmap(pixmap);
    if (pixmap.depth() == 1)
        image = d->colorizeBitmap(image, d->pen.color());
    drawImage(r, *image, sr);
}

void QRasterPaintEngine::drawImage(const QRectF &r, const QImage &image, const QRectF &sr,
                                   Qt::ImageConversionFlags)
{
#ifdef QT_DEBUG_DRAW
    qDebug() << " - QRasterPaintEngine::drawImage(), r=" << r << " sr=" << sr << " image=" << image.size() << "depth=" << image.depth();
#endif

    Q_D(QRasterPaintEngine);
    TextureFillData textureData = {
        d->rasterBuffer,
        (ARGB*)image.bits(), image.width(), image.height(), image.hasAlphaBuffer(),
        0., 0., 0., 0., 0., 0.,
        d->drawHelper->blend,
        d->bilinear ? d->drawHelper->blendTransformedBilinear : d->drawHelper->blendTransformed
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

    fillPath(path, &fillData);
}

void QRasterPaintEngine::drawTiledPixmap(const QRectF &r, const QPixmap &pixmap, const QPointF &sr,
                                         Qt::PixmapDrawingMode /*mode*/)
{
#ifdef QT_DEBUG_DRAW
    qDebug() << " - QRasterPaintEngine::drawTiledPixmap(), r=" << r << "pixmap=" << pixmap.size();
#endif
    Q_D(QRasterPaintEngine);

    QPainterPath path;
    path.addRect(r);

    QImage *image = qt_image_for_pixmap(pixmap);

    if (pixmap.depth() == 1)
        image = d->colorizeBitmap(image, d->pen.color());

    TextureFillData textureData = {
        d->rasterBuffer,
        (ARGB*)image->bits(), image->width(), image->height(), image->hasAlphaBuffer(),
        0., 0., 0., 0., 0., 0.,
        d->drawHelper->blendTiled,
        d->bilinear ? d->drawHelper->blendTransformedBilinearTiled : d->drawHelper->blendTransformedTiled
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

    fillPath(path, &fillData);

}


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
    FillData fillData = { d->rasterBuffer, 0, 0 };
    d->fillForBrush(d->pen.brush(), &fillData, 0);

    qt_span_func func = fillData.callback;
    void *data = fillData.data;

    if (!func)
        return;

    FillData clipData = { d->rasterBuffer, fillData.callback, fillData.data };
    if (d->clipEnabled) {
        func = qt_span_fill_clipped;
        data = &clipData;
    }

    // Boundaries
    int ymax = qMin(devRect.y() + devRect.height(), d->rasterBuffer->height());
    int ymin = qMax(devRect.y(), 0);
    int xmax = qMin(devRect.x() + devRect.width(), d->rasterBuffer->width());
    int xmin = qMax(devRect.x(), 0);

    static QDataBuffer<QT_FT_Span> spans;
    for (int y=ymin; y<ymax; ++y) {
        ARGB *scanline = d->fontRasterBuffer->scanLine(y - devRect.y()) - devRect.x();
        // Generate spans for this y coord
        spans.reset();
        for (int x = xmin; x<xmax; ) {
            // Skip those with 0 coverage (black on white so inverted)
            while (x < xmax && scanline[x].b == 255) ++x;
            if (x >= xmax) break;

            int prev = scanline[x].b;
            QT_FT_Span span = { x, 0, 255 - scanline[x].b };

            // extend span until we find a different one.
            while (x < xmax && scanline[x].b == prev) ++x;
            span.len = x - span.x;

            spans.add(span);
        }

        // Call span func for current set of spans.
        func(y, spans.size(), spans.data(), data);
    }

#else
    bool aa = d->antialiased;
    d->antialiased = true;
    QPaintEngine::drawTextItem(p, ti);
    d->antialiased = aa;
    return;
#endif // Q_WS_WIN
}

enum LineDrawMode {
    LineDrawClipped,
    LineDrawNormal,
    LineDrawIncludeLastPixel
};

static LineDrawMode clipLine(QLineF &line, const QRect &rect)
{
    LineDrawMode mode = LineDrawNormal;

    qreal x1 = line.x1();
    qreal x2 = line.x2();
    qreal y1 = line.y1();
    qreal y2 = line.y2();

    enum { left, right, top, bottom };
    // clip the lines, after cohen-sutherland, see e.g. http://www.nondot.org/~sabre/graphpro/line6.html
    int p1 = ((x1 < rect.left()) << left)
             | ((x1 >= rect.right()) << right)
             | ((y1 < rect.top()) << top)
             | ((y1 >= rect.bottom()) << bottom);
    int p2 = ((x2 < rect.top()) << left)
             | ((x2 >= rect.bottom()) << right)
             | ((y2 < rect.top()) << top)
             | ((y2 >= rect.bottom()) << bottom);

    if (p1 & p2)
        // completely outside
        return LineDrawClipped;

    if (p1 | p2) {
        qreal dx = x2 - x1;
        qreal dy = y2 - y1;

        // clip x coordinates
        if (x1 < rect.left()) {
            y1 += dy/dx * (rect.left() - x1);
            x1 = rect.left();
        } else if (x1 > rect.right()) {
            y1 -= dy/dx * (x1 - rect.right());
            x1 = rect.right();
        }
        if (x2 < rect.left()) {
            y2 += dy/dx * (rect.left() - x2);
            x2 = rect.left();
            mode = LineDrawIncludeLastPixel;
        } else if (x2 > rect.right()) {
            y2 -= dy/dx * (x2 - rect.right());
            x2 = rect.right();
            mode = LineDrawIncludeLastPixel;
        }
        p1 = ((y1 < rect.top()) << top)
             | ((y1 >= rect.bottom()) << bottom);
        p2 = ((y2 < rect.top()) << top)
             | ((y2 >= rect.bottom()) << bottom);
        if (p1 & p2)
            return LineDrawClipped;
        // clip y coordinates
        if (y1 < rect.left()) {
            x1 += dx/dy * (rect.left() - y1);
            y1 = rect.left();
        } else if (y1 > rect.bottom()) {
            x1 -= dx/dy * (y1 - rect.bottom());
            y1 = rect.bottom();
        }
        if (y2 < rect.left()) {
            x2 += dx/dy * (rect.left() - y2);
            y2 = rect.left();
            mode = LineDrawIncludeLastPixel;
        } else if (y2 > rect.bottom()) {
            x2 -= dx/dy * (y2 - rect.bottom());
            y2 = rect.bottom();
            mode = LineDrawIncludeLastPixel;
        }
        line = QLineF(QPointF(x1, y1), QPointF(x2, y2));
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
            span.x = x;
            span_func(y, 1, &span, data);
        }
    }
}

void QRasterPaintEngine::drawLine(const QLineF &l)
{
#ifdef QT_DEBUG_DRAW
    qDebug(" - QRasterPaintEngine::drawLine(), x1=%.2f, y1=%.2f, x2=%.2f, y2=%.2f",
           l.x1(), l.y1(), l.x2(), l.y2());
#endif

    Q_D(QRasterPaintEngine);
    if (!d->antialiased
        && d->pen.style() == Qt::SolidLine
        && (d->pen.widthF() == 0
            || (d->pen.widthF() <= 1 && d->txop <= QPainterPrivate::TxTranslate))) {

        QLineF line = d->matrix.map(l);
        LineDrawMode mode = clipLine(line, d->deviceRect);
        if (mode == LineDrawClipped)
            return;

        if (mode == LineDrawNormal && d->pen.capStyle() != Qt::FlatCap)
            mode = LineDrawIncludeLastPixel;

        FillData fillData = { d->rasterBuffer, 0, 0 };
        d->fillForBrush(QBrush(d->pen.brush()), &fillData, 0);

        FillData clipData = { d->rasterBuffer, fillData.callback, fillData.data };
        void *data = d->clipEnabled ? (void *)&clipData : (void *) fillData.data;
        qt_span_func func = d->clipEnabled ? qt_span_fill_clipped : fillData.callback;

        drawLine_bresenham(line, func, data, mode);
        return;
    }
    QPaintEngine::drawLine(l);
}


void QRasterPaintEngine::drawRect(const QRectF &r)
{
#ifdef QT_DEBUG_DRAW
    qDebug(" - QRasterPaintEngine::drawRect(), x=%.2f, y=%.2f, width=%.2f, height=%.2f",
           r.x(), r.y(), r.width(), r.height());
#endif
#if 0
    Q_D(QRasterPaintEngine);
    if (0 &&
        !d->antialiased
        && d->txop <= QPainterPrivate::TxTranslate) {

        QRectF rect(r);
        rect.translate(d->matrix.dx(), d->matrix.dy());

        FillData fillData = { d->rasterBuffer, 0, 0 };
        d->fillForBrush(d->brush, &fillData);

        FillData clipData = { d->rasterBuffer, fillData.callback, fillData.data };
        void *data = d->clipEnabled ? (void *)&clipData : (void *) fillData.data;
        qt_span_func func = d->clipEnabled ? qt_span_fill_clipped : fillData.callback;

        int x1 = qMax(qRound(rect.x()), 0);
        int x2 = qMin(qRound(rect.width() + rect.x()), d->rasterBuffer->width());

        QT_FT_Span span;
        span.x = x1;
        span.len = x2 - x1;
        span.coverage = 255;

        int y1 = qMax(qRound(rect.y()), 0);
        int y2 = qMin(qRound(rect.height() + rect.y()), d->rasterBuffer->height());;

        // draw the fill
        if (fillData.callback) {
            for (int y=y1; y<y2; ++y) {
                func(y, 1, &span, data);
            }
        }

        // draw the outline...
        if (d->pen.style() != Qt::NoPen) {

            QMatrix oldMatrix = d->matrix;
            d->matrix = QMatrix();

            drawLine(QLineF(x1, y1, x2, y1));
            drawLine(QLineF(x1, y2, x2, y2));
            // To avoid duplicate pixels.
            ++y1;
            --y2;
            drawLine(QLineF(x1, y1, x1, y2));
            drawLine(QLineF(x2, y1, x2, y2));

            d->matrix = oldMatrix;
        }
    } else
#endif
        {
        QPaintEngine::drawRect(r);
    }
}

void QRasterPaintEngine::drawEllipse(const QRectF &rect)
{
    Q_D(QRasterPaintEngine);
    if (!d->antialiased && d->pen.style() == Qt::NoPen && d->txop <= QPainterPrivate::TxTranslate) {
        QPen oldPen = d->pen;
        updatePen(QPen(d->brush, 0));
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


void QRasterPaintEnginePrivate::fillForBrush(const QBrush &brush, FillData *fillData,
                                             const QPainterPath *path)
{
    Q_ASSERT(fillData);
    Q_ASSERT(fillData->rasterBuffer);

    switch (brush.style()) {

    case Qt::NoBrush:
        fillData->callback = 0;
        fillData->data = 0;
        break;

    case Qt::SolidPattern:
        fillData->callback = qt_span_solidfill;
        fillData->data = solidFillData;
        solidFillData->color = mapColor(brush.color());
        solidFillData->rop = rasterOperation;
        solidFillData->rasterBuffer = fillData->rasterBuffer;
        solidFillData->blendColor = drawHelper->blendColor;
        break;

    case Qt::TexturePattern:
        {
            QImage *image = qt_image_for_pixmap(brush.texture());
            fillData->data = textureFillData;
            fillData->callback = txop > QPainterPrivate::TxTranslate
                                 ? qt_span_texturefill_xform
                                 : qt_span_texturefill;
            textureFillData->init(rasterBuffer, image, brushMatrix,
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
            QImage *image = qt_image_for_pixmap(pixmap);
            image = colorizeBitmap(image, brush.color());
            fillData->data = textureFillData;
            fillData->callback = txop > QPainterPrivate::TxTranslate
                                 ? qt_span_texturefill_xform
                                 : qt_span_texturefill;
            textureFillData->init(rasterBuffer, image, brushMatrix,
                                  drawHelper->blendTiled,
                                  bilinear
                                  ? drawHelper->blendTransformedBilinearTiled
                                  : drawHelper->blendTransformedTiled);
        }
        break;


    default:
        break;
    }
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

ARGB *QRasterPaintEnginePrivate::gradientStopColors(const QGradient *gradient)
{
    stopColors.reset();
    QGradientStops stops = gradient->stops();
    for (int i=0; i<stops.size(); ++i)
        stopColors.add(stops.at(i).second.rgba());
    return stopColors.data();
}


ARGB QRasterPaintEnginePrivate::mapColor(const QColor &c) const
{
#ifdef Q_WS_WIN
    if (deviceDepth == 1)
        return ARGB(0, c.red(), c.green(), c.blue());
#endif
    return c;
}

QImage *QRasterPaintEnginePrivate::colorizeBitmap(const QImage *image, const QColor &color)
{
    tempImage = QImage(image->size(), 32);
    Q_ASSERT(image->depth() == 32);
//     QRgb color0 = 0xffffffff;
    QRgb color1 = 0xff000000;
    QRgb fg = color.rgba();
    QRgb bg = opaqueBackground ? bgBrush.color().rgba() : 0;
    for (int y=0; y<image->height(); ++y) {
        const QRgb *source = reinterpret_cast<const QRgb *>(image->scanLine(y));
        uint *target = reinterpret_cast<QRgb *>(tempImage.scanLine(y));
        for (int x=0; x<image->width(); ++x)
            target[x] = source[x] == color1 ? fg : bg;
    }
    tempImage.setAlphaBuffer(true);
    return &tempImage;
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
    prepareClip(image->width(), image->height());

    m_buffer = (ARGB *)image->bits();

    m_width = image->width();
    m_height = image->height();
}

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
    memset(m_buffer, val, m_width*m_height*sizeof(ARGB));
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
    bmi.bmiHeader.biSizeImage   = width * height * sizeof(ARGB);

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
    m_bitmap = CreateDIBSection(m_hdc, &bmi, DIB_RGB_COLORS, (void**) &m_buffer, 0, 0);
    Q_ASSERT(m_bitmap);

    SelectObject(m_hdc, m_bitmap);

    SelectObject(m_hdc, GetStockObject(NULL_PEN));
    Rectangle(m_hdc, 0, 0, m_width, m_height);

    ReleaseDC(0, displayDC);
}
#elif defined(Q_WS_X11)
void QRasterBuffer::prepareBuffer(int width, int height)
{
    delete[] m_buffer;
    m_buffer = new ARGB[width*height];
    memset(m_buffer, 255, width*height*sizeof(ARGB));
}
#elif defined(Q_WS_MAC)
static void qt_mac_raster_data_free(void *, const void *data, size_t)
{
    free(const_cast<void *>(data));
}

void QRasterBuffer::prepareBuffer(int width, int height)
{
    m_buffer = new ARGB[width*height*sizeof(ARGB)];
    memset(m_buffer, 255, width*height*sizeof(ARGB));

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
    QRasterPaintEnginePrivate::RasterOperation rop = data->rop;
    ARGB color = data->color;
    QRasterBuffer *rb = data->rasterBuffer;
    ARGB *rasterBuffer = rb->buffer() + y * rb->width();

    Q_ASSERT(y >= 0);
    Q_ASSERT(y < rb->height());

    if (rop == QRasterPaintEnginePrivate::SourceOverComposite) {
        for (int span=0; span<count; ++span) {

            Q_ASSERT(spans->x >= 0);
            Q_ASSERT(spans->len > 0);
            Q_ASSERT(spans->x + spans->len <= rb->width());
            ARGB *target = rasterBuffer + spans->x;
            data->blendColor(target, (const QSpan *)spans, color);
            ++spans;
        }
    } else {
        for (int span=0; span<count; ++span) {

            Q_ASSERT(spans->x >= 0);
            Q_ASSERT(spans->len > 0);
            Q_ASSERT(spans->x + spans->len <= rb->width());
            ARGB *target = rasterBuffer + spans->x;
            for (int i=spans->len; i; --i) {
                *target++ = color;
            }
            ++spans;
        }

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

    ARGB *scanline = data->imageData + ((y+yoff) % image_height) * image_width;
    ARGB *baseTarget = rb->scanLine(y);
    bool opaque = !data->hasAlpha;
    while (count--) {
        ARGB *target = baseTarget + spans->x;
        if (opaque && spans->coverage == 255) {
            int span_x = spans->x;
            int span_len = spans->len;
            while (span_len > 0) {
                int image_x = (span_x + xoff) % image_width;
                int len = qMin(image_width - image_x, span_len);
                Q_ASSERT(image_x >= 0);
                Q_ASSERT(image_x + len <= image_width); // inclusive since it is used as upper bound.
                Q_ASSERT(span_x + len <= rb->width());
                memcpy(target, scanline + image_x, len * sizeof(ARGB));
                span_x += len;
                span_len -= len;
                target += len;
            }
        } else {
            data->blend(target, (const QSpan *)spans, (xoff + spans->x)%image_width,
                        ((y + yoff) % image_height), data->imageData, image_width, image_height);
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
    ARGB *baseTarget = rb->scanLine(y);

    qreal ix = data->m21 * y + data->dx;
    qreal iy = data->m22 * y + data->dy;

    qreal dx = data->m11;
    qreal dy = data->m12;

    while (count--) {
        data->blendFunc(baseTarget + spans->x, (const QSpan *)spans,
                        ix, iy, dx, dy,
                        data->imageData, image_width, image_height);
        ++spans;
    }
}


ARGB qt_gradient_pixel(const GradientData *data, double pos)
{
  // calculate the actual offset.
    if (pos <= 0 || pos >= 1) {
        if (data->spread == QGradient::RepeatSpread)
            pos = pos - floor(pos);
        else if (data->spread == QGradient::ReflectSpread) {
            pos = pos - 2 * floor (.5 * pos);
            pos = (pos > 1. ? 2. - pos : pos);
        } else {
            if (pos <= 0) return data->colorTable[0];
            else if (pos >= 1) return data->colorTable[GRADIENT_STOPTABLE_SIZE-1];
        }
    }

    return data->colorTable[int(pos * GRADIENT_STOPTABLE_SIZE)];
} // qt_gradient_pixel

void qt_span_linear_gradient(int y, int count, QT_FT_Span *spans, void *userData)
{
    LinearGradientData *data = reinterpret_cast<LinearGradientData *>(userData);

    ARGB *baseTarget = data->rasterBuffer->scanLine(y);

    qreal ybase = (y - data->origin.y()) * data->yincr;
    qreal x1 = data->origin.x();
    qreal t;

    while (count--) {
        ARGB *target = baseTarget + spans->x;
        t = ybase + data->xincr * (spans->x - x1);
        if (!data->alphaColor && spans->coverage == 255) {
            for (int x = spans->x; x<spans->x + spans->len; x++) {
                *target = qt_gradient_pixel(data, t);
                ++target;
                t += data->xincr;
            }
        } else {
            for (int x = spans->x; x<spans->x + spans->len; x++) {
                ARGB src = qt_gradient_pixel(data, t);
                qt_blend_pixel(src, target, spans->coverage);
                ++target;
                t += data->xincr;
            }
        }
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
    QImage image(m_width, m_height, 32);
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
#endif


void TextureFillData::init(QRasterBuffer *raster, QImage *image, const QMatrix &matrix,
                           Blend b, BlendTransformed func)
{
    rasterBuffer = raster;
    imageData = (ARGB*) image->bits();
    width = image->width();
    height = image->height();
    hasAlpha = image->hasAlphaBuffer();

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

        ARGB current_color = stopColors[current_stop];
        ARGB next_color = stopColors[current_stop+1];

        qreal dist = (dpos - stopPoints[current_stop])
                     / (stopPoints[current_stop+1] - stopPoints[current_stop]);
        qreal idist = 1 - dist;

        colorTable[pos] = ARGB(uchar(current_color.a * idist + next_color.a * dist),
                               uchar(current_color.r * idist + next_color.r * dist),
                               uchar(current_color.g * idist + next_color.g * dist),
                               uchar(current_color.b * idist + next_color.b * dist));

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
        angle = qRound(1800*acos(d->matrix.m11()/scale)/M_PI);
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

    QImage image(width, height, 32);
    if ( rdata->stopCount == 0 ) {
        image.fill( 0 );
        return image;
    }

    if ( rdata->radius <= 0. ) {
        image.fill( qt_gradient_pixel( rdata, 0 ).toRgba() );
        return image;
    }

    qreal r, x0, y0, fx, fy, sw, sh, a, b, c, dc, d2c, dba, ba, rad, dx, dy, drad, d2rad, p, d2y;
    int i, j;
    ARGB *line;

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
        line = (ARGB *) image.scanLine( i );
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

    image.setAlphaBuffer(rdata->alphaColor);

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

    QImage image(width, height, 32);

    if ( cdata->stopCount == 0 ) {
        image.fill( 0 );
        return image;
    }

    double dx0, dy0, da, dy, dx, dny, dnx, nx, ny, p, dp, rp;
    int i, j, si;
    bool mdp;
    ARGB *line;

    p = 0;
    dp = 0;

    dx0 = rect.x() - cdata->center.x();
    dy0 = rect.y() - cdata->center.y();
    da = cdata->angle / 2. / M_PI;

    dy = dy0;

    if ( floor( dx0 ) == dx0 && floor( dy0 ) == dy0 )
        image.setPixel( -(int)dx0, -(int)dy0, qt_gradient_pixel( cdata, 0. ).toRgba() );

    for ( i = 0; i < height; i++ ) {
        if ( dy == 0. ) {
            dy += 1;
            continue;
        }
        line = (ARGB *) image.scanLine( i );
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
        line = (ARGB *) image.scanLine( i );
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
            image.setPixel( i, j, qt_gradient_pixel( cdata, rp ).toRgba() );
            p += dp;
            ny += dny;
            j++;
        }
        dx += 1;
    }

    image.setAlphaBuffer(cdata->alphaColor);

    return image;
}

