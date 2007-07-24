/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <private/qtextengine_p.h>
#include <qdebug.h>
#include <private/qfontengine_p.h>
#include <private/qmath_p.h>
#include <private/qdrawhelper_p.h>
#include <private/qpaintengine_p.h>
#include "qapplication.h"
#include "qbrush.h"
#include "qgl.h"
#include <private/qgl_p.h>
#include "qmap.h"
#include <private/qpaintengine_opengl_p.h>
#include <private/qdatabuffer_p.h>
#include "qpen.h"
#include "qvarlengtharray.h"
#include <private/qpainter_p.h>
#include <qglpixelbuffer.h>
#include <private/qglpixelbuffer_p.h>
#include <private/qbezier_p.h>
#include <qglframebufferobject.h>

#include "private/qtessellator_p.h"

#include "util/fragmentprograms_p.h"

#ifdef Q_WS_QWS
#include "private/qglpaintdevice_qws_p.h"
#include "private/qglwindowsurface_qws_p.h"
#include "qwsmanager_qws.h"
#include "private/qwsmanager_p.h"
#endif

extern QImage qt_imageForBrush(int brushStyle, bool invert); //in qbrush.cpp

#define QGL_FUNC_CONTEXT QGLContext *ctx = const_cast<QGLContext *>(drawable.context());
#define QGL_D_FUNC_CONTEXT QGLContext *ctx = const_cast<QGLContext *>(d->drawable.context());

#include <stdlib.h>
#include "qpaintengine_opengl_p.h"

#define QREAL_MAX 9e100
#define QREAL_MIN -9e100

extern QGLContextPrivate *qt_glctx_get_dptr(QGLContext *);
extern int qt_next_power_of_two(int v);

#define DISABLE_DEBUG_ONCE

//#define DEBUG_DISPLAY_MASK_TEXTURE

#ifdef DISABLE_DEBUG_ONCE
#define DEBUG_OVERRIDE(state) ;
#define DEBUG_ONCE_STR(str) ;
#define DEBUG_ONCE if (0)
#else
static int DEBUG_OVERRIDE_FLAG = 0;
static bool DEBUG_TEMP_FLAG;
#define DEBUG_OVERRIDE(state) { state ? ++DEBUG_OVERRIDE_FLAG : --DEBUG_OVERRIDE_FLAG; }
#define DEBUG_ONCE if ((DEBUG_TEMP_FLAG = DEBUG_OVERRIDE_FLAG) && 0) ; else for (static int DEBUG_ONCE_FLAG = false; !DEBUG_ONCE_FLAG || DEBUG_TEMP_FLAG; DEBUG_ONCE_FLAG = true, DEBUG_TEMP_FLAG = false)
#define DEBUG_ONCE_STR(str) DEBUG_ONCE qDebug() << (str);
#endif

static inline void qt_glColor4ubv(unsigned char *col)
{
#ifdef Q_WS_QWS
        glColor4f(col[0]/255.0, col[1]/255.0, col[2]/255.0, col[3]/255.0);
#else
        glColor4ubv(col);
#endif
}

static void qt_add_rect_to_array(const QRectF &r, float *array)
{
    qreal left = r.left();
    qreal right = r.right();
    qreal top = r.top();
    qreal bottom = r.bottom();

    array[0] = left;
    array[1] = top;
    array[2] = right;
    array[3] = top;
    array[4] = right;
    array[5] = bottom;
    array[6] = left;
    array[7] = bottom;
}

static void qt_add_texcoords_to_array(qreal x1, qreal y1, qreal x2, qreal y2, float *array)
{
    array[0] = x1;
    array[1] = y1;
    array[2] = x2;
    array[3] = y1;
    array[4] = x2;
    array[5] = y2;
    array[6] = x1;
    array[7] = y2;
}

struct QGLTrapezoid
{
    QGLTrapezoid()
    {}

    QGLTrapezoid(qreal top_, qreal bottom_, qreal topLeftX_, qreal topRightX_, qreal bottomLeftX_, qreal bottomRightX_)
        : top(top_),
          bottom(bottom_),
          topLeftX(topLeftX_),
          topRightX(topRightX_),
          bottomLeftX(bottomLeftX_),
          bottomRightX(bottomRightX_)
    {}

    const QGLTrapezoid translated(const QPointF &delta) const;

    qreal top;
    qreal bottom;
    qreal topLeftX;
    qreal topRightX;
    qreal bottomLeftX;
    qreal bottomRightX;
};

const QGLTrapezoid QGLTrapezoid::translated(const QPointF &delta) const
{
    QGLTrapezoid trap(*this);
    trap.top += delta.y();
    trap.bottom += delta.y();
    trap.topLeftX += delta.x();
    trap.topRightX += delta.x();
    trap.bottomLeftX += delta.x();
    trap.bottomRightX += delta.x();
    return trap;
}

class QGLDrawable {
public:
    QGLDrawable() : widget(0), buffer(0), fbo(0)
#ifdef Q_WS_QWS
                  , wsurf(0)
#endif
        {}
    inline void setDevice(QPaintDevice *pdev);
    inline void swapBuffers();
    inline void makeCurrent();
    inline void doneCurrent();
    inline QSize size() const;
    inline QGLFormat format() const;
    inline GLuint bindTexture(const QImage &image, GLenum target = GL_TEXTURE_2D, GLint format = GL_RGBA);
    inline GLuint bindTexture(const QPixmap &pixmap, GLenum target = GL_TEXTURE_2D, GLint format = GL_RGBA);
    inline QColor backgroundColor() const;
    inline QGLContext *context() const;
    inline bool autoFillBackground() const;

private:
    QGLWidget *widget;
    QGLPixelBuffer *buffer;
    QGLFramebufferObject *fbo;
#ifdef Q_WS_QWS
    QWSGLWindowSurface *wsurf;
#endif
};

void QGLDrawable::setDevice(QPaintDevice *pdev)
{
    widget = 0;
    buffer = 0;
    fbo = 0;
#ifdef Q_WS_QWS
    wsurf = 0;
#endif
    if (pdev->devType() == QInternal::Widget)
        widget = static_cast<QGLWidget *>(pdev);
    else if (pdev->devType() == QInternal::Pbuffer)
        buffer = static_cast<QGLPixelBuffer *>(pdev);
    else if (pdev->devType() == QInternal::FramebufferObject)
        fbo = static_cast<QGLFramebufferObject *>(pdev);
#ifdef Q_WS_QWS
    else if (pdev->devType() == QInternal::UnknownDevice)
        wsurf = static_cast<QWSGLPaintDevice*>(pdev)->windowSurface();
#endif
}

inline void QGLDrawable::swapBuffers()
{
    if (widget) {
        if (widget->autoBufferSwap())
            widget->swapBuffers();
    } else {
        glFlush();
    }
}

inline void QGLDrawable::makeCurrent()
{
    if (widget)
        widget->makeCurrent();
    else if (buffer)
        buffer->makeCurrent();
    else if (fbo)
        fbo->bind();
}

inline void QGLDrawable::doneCurrent()
{
    if (fbo)
        fbo->release();
}

inline QSize QGLDrawable::size() const
{
    if (widget)
        return widget->size();
    else if (buffer)
        return buffer->size();
    else if (fbo)
        return fbo->size();
#ifdef Q_WS_QWS
    else if (wsurf)
        return wsurf->window()->frameSize();
#endif
    return QSize();
}

inline QGLFormat QGLDrawable::format() const
{
    if (widget)
        return widget->format();
    else if (buffer)
        return buffer->format();
    else if (fbo && QGLContext::currentContext()) {
        QGLFormat fmt = QGLContext::currentContext()->format();
        fmt.setStencil(fbo->attachment() == QGLFramebufferObject::CombinedDepthStencil);
        fmt.setDepth(fbo->attachment() != QGLFramebufferObject::NoAttachment);
        return fmt;
    }
#ifdef Q_WS_QWS
    else if (wsurf)
        return wsurf->context()->format();
#endif
    return QGLFormat();
}

inline GLuint QGLDrawable::bindTexture(const QImage &image, GLenum target, GLint format)
{
    if (widget)
        return widget->d_func()->glcx->d_func()->bindTexture(image, target, format, true);
    else if (buffer)
        return buffer->d_func()->qctx->d_func()->bindTexture(image, target, format, true);
    else if (fbo && QGLContext::currentContext())
        return const_cast<QGLContext *>(QGLContext::currentContext())->d_func()->bindTexture(image, target, format, true);
#ifdef Q_WS_QWS
    else if (wsurf)
        return wsurf->context()->d_func()->bindTexture(image, target, format, true);
#endif
    return 0;
}

inline GLuint QGLDrawable::bindTexture(const QPixmap &pixmap, GLenum target, GLint format)
{
    if (widget)
        return widget->d_func()->glcx->d_func()->bindTexture(pixmap, target, format, true);
    else if (buffer)
        return buffer->d_func()->qctx->d_func()->bindTexture(pixmap, target, format, true);
    else if (fbo && QGLContext::currentContext())
        return const_cast<QGLContext *>(QGLContext::currentContext())->d_func()->bindTexture(pixmap, target, format, true);
#ifdef Q_WS_QWS
    else if (wsurf)
        return wsurf->context()->d_func()->bindTexture(pixmap, target, format, true);
#endif
    return 0;
}

inline QColor QGLDrawable::backgroundColor() const
{
    if (widget)
        return widget->palette().brush(widget->backgroundRole()).color();
    return QApplication::palette().brush(QPalette::Background).color();
}

inline QGLContext *QGLDrawable::context() const
{
    if (widget)
        return widget->d_func()->glcx;
    else if (buffer)
        return buffer->d_func()->qctx;
    else if (fbo)
        return const_cast<QGLContext *>(QGLContext::currentContext());
#ifdef Q_WS_QWS
    else if (wsurf)
        return wsurf->context();
#endif
    return 0;
}

inline bool QGLDrawable::autoFillBackground() const
{
    if (widget)
        return widget->autoFillBackground();
    else
        return false;
}


class QOpenGLImmediateModeTessellator;
class QGLMaskGenerator;
class QGLOffscreen;

class QGLMaskTextureCache
{
public:
    void setOffscreenSize(const QSize &offscreenSize);
    void setDrawableSize(const QSize &drawableSize);

    struct CacheLocation {
        QRect rect;
        int channel;

        QRect screen_rect;
    };

    struct CacheInfo {
        inline CacheInfo(const QPainterPath &p, const QTransform &m, qreal w = -1) :
            path(p), matrix(m), stroke_width(w), age(0) {}

        QPainterPath path;
        QTransform matrix;
        qreal stroke_width;

        CacheLocation loc;

        int age;
    };

    struct QuadTreeNode {
        quint64 key;

        int largest_available_block;
        int largest_used_block;
    };

    CacheLocation getMask(QGLMaskGenerator &maskGenerator, QOpenGLPaintEnginePrivate *engine);

    typedef QMultiHash<quint64, CacheInfo> QGLTextureCacheHash;

    enum {block_size = 64};

    // throw out keys that are too old
    void maintainCache();
    void clearCache();

private:
    quint64 hash(const QPainterPath &p, const QTransform &m, qreal w);

    void createMask(quint64 key, CacheInfo &info, QGLMaskGenerator &maskGenerator);

    QSize offscreenSize;
    QSize drawableSize;

    QGLTextureCacheHash cache;

    QVector<QuadTreeNode> occupied_quadtree[4];

    void quadtreeUpdate(int channel, int node, int current_block_size);
    void quadtreeAllocate(quint64 key, const QSize &size, QRect *rect, int *channel);

    bool quadtreeFindAvailableLocation(const QSize &size, QRect *rect, int *channel);
    void quadtreeFindExistingLocation(const QSize &size, QRect *rect, int *channel);

    void quadtreeInsert(int channel, quint64 key, const QRect &rect, int node = 0);
    void quadtreeClear(int channel, const QRect &rect, int node = 0);

    int quadtreeBlocksize(int node);
    QPoint quadtreeLocation(int node);

    QOpenGLPaintEnginePrivate *engine;
};

Q_GLOBAL_STATIC(QGLMaskTextureCache, qt_mask_texture_cache)

class QGLOffscreen : public QObject
{
    Q_OBJECT
public:
    QGLOffscreen()
        : QObject(),
          offscreen(0),
          ctx(0),
          mask_dim(0),
          activated(false),
          bound(false)
    {
        connect(QGLProxy::signalProxy(),
                SIGNAL(aboutToDestroyContext(const QGLContext *)),
                SLOT(cleanupGLContextRefs(const QGLContext *)));
    }

    inline void setDevice(QPaintDevice *pdev);

    void begin();
    void end();

    inline void bind();
    inline void release();

    inline bool isBound() const;

    inline QSize drawableSize() const;
    inline QSize offscreenSize() const;

    inline GLuint offscreenTexture() const;

    QGLContext *context() const;

    static bool isSupported();

    inline void initialize();

    inline bool isValid() const;

public Q_SLOTS:
    void cleanupGLContextRefs(const QGLContext *context) {
        if (context == ctx)
            ctx = 0;
    }

private:
    QGLDrawable drawable;

    QGLFramebufferObject *offscreen;
    QGLContext *ctx;

    // dimensions of mask texture (square)
    int mask_dim;
    QSize last_failed_size;

    bool drawable_fbo;

    bool activated;
    bool initialized;

    bool bound;
};

inline void QGLOffscreen::setDevice(QPaintDevice *pdev)
{
    drawable.setDevice(pdev);

    drawable_fbo = (pdev->devType() == QInternal::FramebufferObject);
}

void QGLOffscreen::begin()
{
#ifndef Q_WS_QWS
    initialized = false;

    if (activated)
        initialize();
#endif
}

void QGLOffscreen::initialize()
{
#ifndef Q_WS_QWS
    if (initialized)
        return;

    activated = true;
    initialized = true;

    int dim = qMax(2048, static_cast<int>(qt_next_power_of_two(qMax(drawable.size().width(), drawable.size().height()))));

    bool shared_context = qgl_share_reg()->checkSharing(drawable.context(), ctx);
    bool would_fail = last_failed_size.isValid() &&
                      (drawable.size().width() >= last_failed_size.width() ||
                       drawable.size().height() >= last_failed_size.height());
    bool needs_refresh = dim > mask_dim || !shared_context;

    if (needs_refresh && !would_fail) {
        DEBUG_ONCE qDebug() << "QGLOffscreen::initialize(): creating offscreen of size" << dim;
        delete offscreen;
        offscreen = new QGLFramebufferObject(dim, dim);
        mask_dim = dim;

        if (!offscreen->isValid()) {
            qWarning("QGLOffscreen: Invalid offscreen fbo (size %dx%d)", mask_dim, mask_dim);
            delete offscreen;
            offscreen = 0;
            mask_dim = 0;
            last_failed_size = drawable.size();
        }
    }

    qt_mask_texture_cache()->setOffscreenSize(offscreenSize());
    qt_mask_texture_cache()->setDrawableSize(drawable.size());
    ctx = drawable.context();
#endif
}

inline bool QGLOffscreen::isValid() const
{
    return offscreen;
}

void QGLOffscreen::end()
{
    if (bound)
        release();
#ifdef DEBUG_DISPLAY_MASK_TEXTURE
    glDisable(GL_FRAGMENT_PROGRAM_ARB);
    glReadBuffer(GL_BACK);
    glDrawBuffer(GL_BACK);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glColor4f(1, 1, 1, 1);
    glDisable(GL_DEPTH_TEST);
    glBlendFunc(GL_ONE, GL_ZERO);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, offscreen->texture());

    glBegin(GL_QUADS);
    glTexCoord2f(0.0, 1.0); glVertex2f(0.0, 0.0);
    glTexCoord2f(1.0, 1.0); glVertex2f(drawable.size().width(), 0.0);
    glTexCoord2f(1.0, 0.0); glVertex2f(drawable.size().width(), drawable.size().height());
    glTexCoord2f(0.0, 0.0); glVertex2f(0.0, drawable.size().height());
    glEnd();

    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
#endif
}

inline void QGLOffscreen::bind()
{
#ifndef Q_WS_QWS
    Q_ASSERT(initialized);

    if (!offscreen || bound)
        return;

    DEBUG_ONCE qDebug() << "QGLOffscreen: binding offscreen";
    offscreen->bind();

    bound = true;

    glViewport(0, 0, offscreenSize().width(), offscreenSize().height());

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, offscreenSize().width(), offscreenSize().height(), 0, -999999, 999999);
    glMatrixMode(GL_MODELVIEW);
#endif
}

inline void QGLOffscreen::release()
{
#ifndef Q_WS_QWS
    if (!offscreen || !bound)
        return;

#ifdef Q_WS_X11
    // workaround for bug in nvidia driver versions 9x.xx
    if (QGLExtensions::nvidiaFboNeedsFinish)
        glFinish();
#endif

    DEBUG_ONCE_STR("QGLOffscreen: releasing offscreen");

    if (drawable_fbo)
        drawable.makeCurrent();
    else
        offscreen->release();

    QSize sz(drawable.size());
    glViewport(0, 0, sz.width(), sz.height());

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, sz.width(), sz.height(), 0, -999999, 999999);
    glMatrixMode(GL_MODELVIEW);

    bound = false;
#endif
}

inline bool QGLOffscreen::isBound() const
{
    return bound;
}

inline QSize QGLOffscreen::drawableSize() const
{
    return drawable.size();
}

inline QSize QGLOffscreen::offscreenSize() const
{
    return QSize(mask_dim, mask_dim);
}

inline GLuint QGLOffscreen::offscreenTexture() const
{
    return offscreen ? offscreen->texture() : 0;
}

inline QGLContext *QGLOffscreen::context() const
{
    return ctx;
}

bool QGLOffscreen::isSupported()
{
    return (QGLExtensions::glExtensions & QGLExtensions::FramebufferObject) // for fbo
        && (QGLFormat::openGLVersionFlags() & QGLFormat::OpenGL_Version_2_0); // non-power-of-two textures
}

struct QDrawQueueItem
{
    QDrawQueueItem(qreal _opacity,
                   QBrush _brush,
                   const QPointF &_brush_origion,
                   QPainter::CompositionMode _composition_mode,
                   const QTransform &_matrix,
                   QGLMaskTextureCache::CacheLocation _location)
        : opacity(_opacity),
          brush(_brush),
          brush_origin(_brush_origion),
          composition_mode(_composition_mode),
          matrix(_matrix),
          location(_location) {}
    qreal opacity;
    QBrush brush;
    QPointF brush_origin;
    QPainter::CompositionMode composition_mode;

    QTransform matrix;
    QGLMaskTextureCache::CacheLocation location;
};

class QOpenGLPaintEnginePrivate;
class QGLPrivateCleanup : public QObject
{
    Q_OBJECT
public:
    QGLPrivateCleanup(QOpenGLPaintEnginePrivate *priv)
        : p(priv)
    {
        connect(QGLProxy::signalProxy(),
                SIGNAL(aboutToDestroyContext(const QGLContext *)),
                SLOT(cleanupGLContextRefs(const QGLContext *)));
    }

public Q_SLOTS:
    void cleanupGLContextRefs(const QGLContext *context);

private:
    QOpenGLPaintEnginePrivate *p;
};

class QOpenGLPaintEnginePrivate : public QPaintEnginePrivate
{
    Q_DECLARE_PUBLIC(QOpenGLPaintEngine)
public:
    QOpenGLPaintEnginePrivate()
        : opacity(1)
        , composition_mode(QPainter::CompositionMode_SourceOver)
        , has_fast_pen(false)
        , txop(QTransform::TxNone)
        , inverseScale(1)
        , moveToCount(0)
        , shader_ctx(0)
        , grad_palette(0)
        , use_stencil_method(false)
        , has_stencil_face_ext(false)
        , use_fragment_programs(false)
        , high_quality_antialiasing(false)
        , drawable_texture(0)
        , ref_cleaner(this)
        {}

    inline void setGLPen(const QColor &c) {
        uint alpha = qRound(c.alpha() * opacity);
        pen_color[0] = (c.red() * alpha + 128) >> 8;
        pen_color[1] = (c.green() * alpha + 128) >> 8;
        pen_color[2] = (c.blue() * alpha + 128) >> 8;
        pen_color[3] = alpha;
    }

    inline void setGLBrush(const QColor &c) {
        uint alpha = qRound(c.alpha() * opacity);
        brush_color[0] = (c.red() * alpha + 128) >> 8;
        brush_color[1] = (c.green() * alpha + 128) >> 8;
        brush_color[2] = (c.blue() * alpha + 128) >> 8;
        brush_color[3] = alpha;
    }

    inline void setGradientOps(const QBrush &brush, const QRectF &bounds);
    void createGradientPaletteTexture(const QGradient& g);

    void updateGradient(const QBrush &brush, const QRectF &bounds);

    inline void lineToStencil(qreal x, qreal y);
    inline void curveToStencil(const QPointF &cp1, const QPointF &cp2, const QPointF &ep);
    void pathToVertexArrays(const QPainterPath &path);
    void fillVertexArray(Qt::FillRule fillRule);
    void drawVertexArrays();
    void fillPath(const QPainterPath &path);
    void fillPolygon_dev(const QPointF *polygonPoints, int pointCount,
                         Qt::FillRule fill);

    void drawFastRect(const QRectF &rect);
    void strokePath(const QPainterPath &path, bool use_cache);
    void strokePathFastPen(const QPainterPath &path, bool needsResolving);
    void strokeLines(const QPainterPath &path);

    void cleanupGLContextRefs(const QGLContext *context) {
        if (context == shader_ctx)
            shader_ctx = 0;
    }

    QPen cpen;
    QBrush cbrush;
    QRegion crgn;
    Qt::BrushStyle brush_style;
    QPointF brush_origin;
    Qt::BrushStyle pen_brush_style;
    qreal opacity;
    QPainter::CompositionMode composition_mode;

    Qt::BrushStyle current_style;

    uint has_clipping : 1;
    uint has_pen : 1;
    uint has_brush : 1;
    uint has_fast_pen : 1;

    QTransform matrix;
    GLubyte pen_color[4];
    GLubyte brush_color[4];
    QTransform::TransformationType txop;
    QGLDrawable drawable;
    QGLOffscreen offscreen;

    qreal inverseScale;

    int moveToCount;
    QPointF path_start;

    bool isFastRect(const QRectF &r);

    void drawImageAsPath(const QRectF &r, const QImage &img, const QRectF &sr);
    void drawTiledImageAsPath(const QRectF &r, const QImage &img);

    void drawOffscreenPath(const QPainterPath &path);

    void composite(const QRectF &rect, const QPoint &maskOffset = QPoint());
    void composite(GLuint primitive, const float *vertexArray, int vertexCount, const QPoint &maskOffset = QPoint());

    bool createFragmentPrograms();
    void deleteFragmentPrograms();
    void updateFragmentProgramData(int locations[]);

    void cacheItemErased(int channel, const QRect &rect);

    void addItem(const QGLMaskTextureCache::CacheLocation &location);
    void drawItem(const QDrawQueueItem &item);
    void flushDrawQueue();

    void copyDrawable(const QRectF &rect);

    QGLContext *shader_ctx;
    GLuint grad_palette;

    GLuint painter_fragment_programs[num_fragment_brushes][num_fragment_composition_modes];
    GLuint mask_fragment_programs[num_fragment_masks];

    bool use_stencil_method;
    bool has_stencil_face_ext;
    bool use_fragment_programs;
    bool high_quality_antialiasing;

    float inv_matrix_data[3][4];
    float fmp_data[4];
    float fmp2_m_radius2_data[4];
    float angle_data[4];
    float linear_data[4];

    float porterduff_ab_data[4];
    float porterduff_xyz_data[4];

    float mask_offset_data[4];
    float mask_channel_data[4];

    FragmentBrushType fragment_brush;
    FragmentCompositionModeType fragment_composition_mode;

    bool has_fast_composition_mode;

    void setPorterDuffData(float a, float b, float x, float y, float z);
    void setInvMatrixData(const QTransform &inv_matrix);

    qreal max_x;
    qreal max_y;
    qreal min_x;
    qreal min_y;

    QDataBuffer<QPointF> tess_points;
    QVector<int> tess_points_stops;

    QImage pattern_image;
    GLdouble projection_matrix[4][4];

    QList<QDrawQueueItem> drawQueue;

    GLuint drawable_texture;
    QSize drawable_texture_size;

    QGLPrivateCleanup ref_cleaner;
    friend class QGLMaskTextureCache;

};

void QGLPrivateCleanup::cleanupGLContextRefs(const QGLContext *context)
{
    p->cleanupGLContextRefs(context);
}


static inline QPainterPath strokeForPath(const QPainterPath &path, const QPen &cpen) {
    QPainterPathStroker stroker;
    if (cpen.style() == Qt::CustomDashLine)
        stroker.setDashPattern(cpen.dashPattern());
    else
        stroker.setDashPattern(cpen.style());

    stroker.setCapStyle(cpen.capStyle());
    stroker.setJoinStyle(cpen.joinStyle());
    stroker.setMiterLimit(cpen.miterLimit());

    qreal width = cpen.widthF();
    if (width == 0)
        stroker.setWidth(1);
    else
        stroker.setWidth(width);

    QPainterPath stroke = stroker.createStroke(path);
    stroke.setFillRule(Qt::WindingFill);
    return stroke;
}

class QGLStrokeCache
{
    struct CacheInfo
    {
        inline CacheInfo(QPainterPath p, QPainterPath sp, QPen stroke_pen) :
            path(p), stroked_path(sp), pen(stroke_pen) {}
        QPainterPath path;
        QPainterPath stroked_path;
        QPen pen;
    };

    typedef QMultiHash<quint64, CacheInfo> QGLStrokeTableHash;

public:
    inline QPainterPath getStrokedPath(const QPainterPath &path, const QPen &pen) {
        quint64 hash_val = 0;

        for (int i = 0; i < path.elementCount() && i <= 2; i++) {
            hash_val += quint64(path.elementAt(i).x);
            hash_val += quint64(path.elementAt(i).y);
        }

        QGLStrokeTableHash::const_iterator it = cache.constFind(hash_val);

        if (it == cache.constEnd())
            return addCacheElement(hash_val, path, pen);
        else {
            do {
                const CacheInfo &cache_info = it.value();
                if (cache_info.path == path && cache_info.pen == pen)
                    return cache_info.stroked_path;
                ++it;
            } while (it != cache.constEnd() && it.key() == hash_val);
            // an exact match for this path was not found, create new cache element
            return addCacheElement(hash_val, path, pen);
        }
    }

protected:
    inline int maxCacheSize() const { return 500; }
    QPainterPath addCacheElement(quint64 hash_val, QPainterPath path, const QPen &pen) {
        if (cache.size() == maxCacheSize()) {
            int elem_to_remove = qrand() % maxCacheSize();
            cache.remove(cache.keys()[elem_to_remove]); // may remove more than 1, but OK
        }
        QPainterPath stroke = strokeForPath(path, pen);
        CacheInfo cache_entry(path, stroke, pen);
        return cache.insert(hash_val, cache_entry).value().stroked_path;
    }

    QGLStrokeTableHash cache;
};

Q_GLOBAL_STATIC(QGLStrokeCache, qt_opengl_stroke_cache)

class QGLGradientCache : public QObject
{
    Q_OBJECT
    struct CacheInfo
    {
        inline CacheInfo(QGradientStops s, qreal op) :
            stops(s), opacity(op) {}

        GLuint texId;
        QGradientStops stops;
        qreal opacity;
    };

    typedef QMultiHash<quint64, CacheInfo> QGLGradientColorTableHash;

public:
    QGLGradientCache() : QObject(), buffer_ctx(0)
    {
        connect(QGLProxy::signalProxy(),
                SIGNAL(aboutToDestroyContext(const QGLContext *)),
                SLOT(cleanupGLContextRefs(const QGLContext *)));
    }

    inline GLuint getBuffer(const QGradientStops &stops, qreal opacity, QGLContext *ctx) {
        if (buffer_ctx && !qgl_share_reg()->checkSharing(buffer_ctx, ctx))
            cleanCache();

        buffer_ctx = ctx;

        quint64 hash_val = 0;

        for (int i = 0; i < stops.size() && i <= 2; i++)
            hash_val += stops[i].second.rgba();

        QGLGradientColorTableHash::const_iterator it = cache.constFind(hash_val);

        if (it == cache.constEnd())
            return addCacheElement(hash_val, stops, opacity);
        else {
            do {
                const CacheInfo &cache_info = it.value();
                if (cache_info.stops == stops && cache_info.opacity == opacity) {
                    return cache_info.texId;
                }
                ++it;
            } while (it != cache.constEnd() && it.key() == hash_val);
            // an exact match for these stops and opacity was not found, create new cache
            return addCacheElement(hash_val, stops, opacity);
        }
    }

    inline int paletteSize() const { return 1024; }

protected:
    inline int maxCacheSize() const { return 60; }
    inline void generateGradientColorTable(const QGradientStops& s,
                                           uint *colorTable,
                                           int size, qreal opacity) const;
    GLuint addCacheElement(quint64 hash_val, const QGradientStops &stops, qreal opacity) {
        if (cache.size() == maxCacheSize()) {
            int elem_to_remove = qrand() % maxCacheSize();
            quint64 key = cache.keys()[elem_to_remove];

            // need to call glDeleteTextures on each removed cache entry:
            QGLGradientColorTableHash::const_iterator it = cache.constFind(key);
            do {
                glDeleteTextures(1, &it.value().texId);
            } while (++it != cache.constEnd() && it.key() == key);
            cache.remove(key); // may remove more than 1, but OK
        }
        CacheInfo cache_entry(stops, opacity);
        uint buffer[1024];
        generateGradientColorTable(stops, buffer, paletteSize(), opacity);
        glGenTextures(1, &cache_entry.texId);
#ifndef Q_WS_QWS
        glBindTexture(GL_TEXTURE_1D, cache_entry.texId);
        glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, paletteSize(),
                     0, GL_BGRA, GL_UNSIGNED_BYTE, buffer);
#else
        // create 2D one-line texture instead. This requires an impl of manual GL_TEXGEN for all primitives
#endif
        return cache.insert(hash_val, cache_entry).value().texId;
    }

    void cleanCache() {
        QGLGradientColorTableHash::const_iterator it = cache.constBegin();
        for (; it != cache.constEnd(); ++it) {
            const CacheInfo &cache_info = it.value();
            glDeleteTextures(1, &cache_info.texId);
        }
        cache.clear();
    }

    QGLGradientColorTableHash cache;

    QGLContext *buffer_ctx;

public Q_SLOTS:
    void cleanupGLContextRefs(const QGLContext *context) {
        if (context == buffer_ctx) {
            cleanCache();
            buffer_ctx = 0;
        }
    }
};

void QGLGradientCache::generateGradientColorTable(const QGradientStops& s, uint *colorTable, int size, qreal opacity) const
{
    int pos = 0;
    qreal fpos = 0.0;
    qreal incr = 1.0 / qreal(size);
    QVector<uint> colors(s.size());

    for (int i = 0; i < s.size(); ++i)
        colors[i] = s[i].second.rgba();

    uint alpha = qRound(opacity * 256);
    while (fpos < s.first().first) {
        colorTable[pos] = PREMUL(ARGB_COMBINE_ALPHA(colors[0], alpha));
        pos++;
        fpos += incr;
    }

    for (int i = 0; i < s.size() - 1; ++i) {
        qreal delta = 1/(s[i+1].first - s[i].first);
        while (fpos < s[i+1].first && pos < size) {
            int dist = int(256 * ((fpos - s[i].first) * delta));
            int idist = 256 - dist;
            uint current_color = PREMUL(ARGB_COMBINE_ALPHA(colors[i], alpha));
            uint next_color = PREMUL(ARGB_COMBINE_ALPHA(colors[i+1], alpha));
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
            colorTable[pos] = INTERPOLATE_PIXEL_256(current_color, idist, next_color, dist);
#else
            uint c = INTERPOLATE_PIXEL_256(current_color, idist, next_color, dist);
            colorTable[pos] = ( (c << 24) & 0xff000000)
                              | ((c >> 24) & 0x000000ff)
                              | ((c << 8) & 0x00ff0000)
                              | ((c >> 8) & 0x0000ff00);
#endif // Q_BYTE_ORDER
            ++pos;
            fpos += incr;
        }
    }
    for (;pos < size; ++pos)
        colorTable[pos] = colors[s.size() - 1];
}

#ifndef Q_WS_QWS
Q_GLOBAL_STATIC(QGLGradientCache, qt_opengl_gradient_cache)
#endif

void QOpenGLPaintEnginePrivate::createGradientPaletteTexture(const QGradient& g)
{
#ifdef Q_WS_QWS //###
    Q_UNUSED(g);
#else
    GLuint texId = qt_opengl_gradient_cache()->getBuffer(g.stops(), opacity, drawable.context());
    glBindTexture(GL_TEXTURE_1D, texId);
    grad_palette = texId;
    if (g.spread() == QGradient::RepeatSpread || g.type() == QGradient::ConicalGradient)
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    else if (g.spread() == QGradient::ReflectSpread)
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT_IBM);
    else
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

#endif
}


inline void QOpenGLPaintEnginePrivate::setGradientOps(const QBrush &brush, const QRectF &bounds)
{
    current_style = brush.style();

    if (current_style < Qt::LinearGradientPattern || current_style > Qt::ConicalGradientPattern) {
        setGLBrush(brush.color());
        qt_glColor4ubv(brush_color);
    }

    updateGradient(brush, bounds);

#ifndef Q_WS_QWS //### GLES does not have GL_TEXTURE_GEN_ so we are falling back for gradients
    glDisable(GL_TEXTURE_GEN_S);
    glDisable(GL_TEXTURE_1D);

    if (current_style == Qt::LinearGradientPattern) {
        if (high_quality_antialiasing || !has_fast_composition_mode) {
            fragment_brush = FRAGMENT_PROGRAM_BRUSH_LINEAR;
        } else {
            glEnable(GL_TEXTURE_GEN_S);
            glEnable(GL_TEXTURE_1D);
        }
    } else {
        if (use_fragment_programs) {
            if (current_style == Qt::RadialGradientPattern)
                fragment_brush = FRAGMENT_PROGRAM_BRUSH_RADIAL;
            else if (current_style == Qt::ConicalGradientPattern)
                fragment_brush = FRAGMENT_PROGRAM_BRUSH_CONICAL;
            else if (current_style == Qt::SolidPattern)
                fragment_brush = FRAGMENT_PROGRAM_BRUSH_SOLID;
            else if (current_style == Qt::TexturePattern)
                fragment_brush = FRAGMENT_PROGRAM_BRUSH_TEXTURE;
            else
                fragment_brush = FRAGMENT_PROGRAM_BRUSH_PATTERN;
        }
    }
#endif
}

QOpenGLPaintEngine::QOpenGLPaintEngine()
    : QPaintEngine(*(new QOpenGLPaintEnginePrivate),
                   PaintEngineFeatures(AllFeatures
                                       & ~(LinearGradientFill
                                           | RadialGradientFill
                                           | ConicalGradientFill
                                           | PatternBrush
                                           | BlendModes)))
{
}

QOpenGLPaintEngine::~QOpenGLPaintEngine()
{
}

#ifndef Q_WS_QWS
static bool qt_createFragmentProgram(QGLContext *ctx, GLuint &program, const char *shader_src)
{
#ifndef Q_WS_WIN
    Q_UNUSED(ctx);
#endif
    glGenProgramsARB(1, &program);
    glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, program);
    const GLbyte *gl_shader_src = reinterpret_cast<const GLbyte *>(shader_src); /* MSVC.NET 2002 */
    glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB,
                       int(strlen(shader_src)), gl_shader_src);

    return glGetError() == GL_NO_ERROR;
}
#endif // !Q_WS_QWS

bool QOpenGLPaintEngine::begin(QPaintDevice *pdev)
{
    Q_D(QOpenGLPaintEngine);

    d->drawable.setDevice(pdev);
    d->offscreen.setDevice(pdev);
    d->has_clipping = false;
    d->has_fast_pen = false;
    d->inverseScale = 1;
    d->opacity = 1;
    d->drawable.makeCurrent();
    d->matrix = QTransform();

    bool has_frag_program = (QGLExtensions::glExtensions & QGLExtensions::FragmentProgram)
                            && (pdev->devType() != QInternal::Pixmap);

    QGLContext *ctx = const_cast<QGLContext *>(d->drawable.context());
    if (has_frag_program)
        has_frag_program = qt_resolve_frag_program_extensions(ctx);

    d->use_stencil_method = d->drawable.format().stencil() &&
                            QGLExtensions::glExtensions & QGLExtensions::StencilWrap;
    if (d->use_stencil_method && QGLExtensions::glExtensions & QGLExtensions::StencilTwoSide)
        d->has_stencil_face_ext = qt_resolve_stencil_face_extension(ctx);

    if (QGLFormat::openGLVersionFlags() & QGLFormat::OpenGL_Version_1_3)
        qt_resolve_version_1_3_functions(ctx);

#ifndef Q_WS_QWS
    glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glGetDoublev(GL_PROJECTION_MATRIX, &d->projection_matrix[0][0]);
#endif
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glMatrixMode(GL_TEXTURE);
    glPushMatrix();
    glLoadIdentity();

    if (QGLExtensions::glExtensions & QGLExtensions::SampleBuffers)
        glDisable(GL_MULTISAMPLE);
#ifndef Q_WS_QWS
    glDisable(GL_TEXTURE_1D);
#endif
    glDisable(GL_TEXTURE_2D);
    if (QGLExtensions::glExtensions & QGLExtensions::TextureRectangle)
        glDisable(GL_TEXTURE_RECTANGLE_NV);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_LIGHTING);
    glShadeModel(GL_FLAT);
    if (d->use_stencil_method) {
        glStencilFunc(GL_ALWAYS, 0, ~0);
        glClearStencil(0);
    }
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    d->offscreen.begin();

    const QColor &c = d->drawable.backgroundColor();
    glClearColor(c.redF(), c.greenF(), c.blueF(), 1.0);
    if (d->drawable.context()->d_func()->clear_on_painter_begin && d->drawable.autoFillBackground()) {
        GLbitfield clearBits = GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT;
#ifndef Q_WS_QWS
        clearBits |= GL_ACCUM_BUFFER_BIT;
#endif
        glClear(clearBits);
    } else if (d->use_stencil_method) {
        glClear(GL_STENCIL_BUFFER_BIT);
    }

    QSize sz(d->drawable.size());
    glViewport(0, 0, sz.width(), sz.height()); // XXX (Embedded): We need a solution for GLWidgets that draw in a part or a bigger surface...
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
#ifdef Q_WS_QWS
    glOrthof(0, sz.width(), sz.height(), 0, -999999, 999999);
#else
    glOrtho(0, sz.width(), sz.height(), 0, -999999, 999999);
#endif
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glEnable(GL_BLEND);
    d->composition_mode = QPainter::CompositionMode_SourceOver;

#ifndef Q_WS_QWS
    bool shared_ctx = qgl_share_reg()->checkSharing(d->drawable.context(), d->shader_ctx);

    if (!shared_ctx) {
        if (d->shader_ctx) {
            d->shader_ctx->makeCurrent();
            glBindTexture(GL_TEXTURE_1D, 0);
            glDeleteTextures(1, &d->grad_palette);

            if (has_frag_program && d->use_fragment_programs) {
                glDeleteTextures(1, &d->drawable_texture);
                d->deleteFragmentPrograms();
            }

            d->drawable.context()->makeCurrent();
        }
        d->shader_ctx = d->drawable.context();
        glGenTextures(1, &d->grad_palette);

        qt_mask_texture_cache()->clearCache();

        if (has_frag_program) {
            d->use_fragment_programs = d->createFragmentPrograms();

            if (!d->use_fragment_programs) {
                d->deleteFragmentPrograms();
                qWarning() << "QOpenGLPaintEngine: Failed to create fragment programs.";
            }
        }

        gccaps &= ~(RadialGradientFill | ConicalGradientFill | LinearGradientFill | PatternBrush | BlendModes);

        if (d->use_fragment_programs)
            gccaps |= (RadialGradientFill | ConicalGradientFill | LinearGradientFill | PatternBrush | BlendModes);
        else if (QGLExtensions::glExtensions & QGLExtensions::MirroredRepeat)
            gccaps |= LinearGradientFill;
    }

    if (d->use_fragment_programs && (!shared_ctx || sz.width() > d->drawable_texture_size.width()
                                                 || sz.height() > d->drawable_texture_size.height()))
    {
        // delete old texture if size has increased, otherwise it was deleted earlier
        if (shared_ctx)
            glDeleteTextures(1, &d->drawable_texture);

        glGenTextures(1, &d->drawable_texture);
        glBindTexture(GL_TEXTURE_2D, d->drawable_texture);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, sz.width(), sz.height(), 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, NULL);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        d->drawable_texture_size = sz;
    }
#endif

    setDirty(QPaintEngine::DirtyPen);
    setDirty(QPaintEngine::DirtyBrush);
    setDirty(QPaintEngine::DirtyCompositionMode);

#ifdef Q_WS_QWS
    // XXX: needed only if painting on a widget?
    updateClipRegion(QRegion(), Qt::NoClip);
#endif

    return true;
}

bool QOpenGLPaintEngine::end()
{
    Q_D(QOpenGLPaintEngine);
    d->flushDrawQueue();
    d->offscreen.end();
    glMatrixMode(GL_TEXTURE);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
#ifndef Q_WS_QWS
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixd(&d->projection_matrix[0][0]);
    glPopAttrib();
    glPopClientAttrib();
#endif

#ifndef Q_WS_QWS
    d->drawable.swapBuffers();
#endif
    d->drawable.doneCurrent();
    qt_mask_texture_cache()->maintainCache();

    return true;
}

void QOpenGLPaintEngine::updateState(const QPaintEngineState &state)
{
    Q_D(QOpenGLPaintEngine);
    QPaintEngine::DirtyFlags flags = state.state();

    bool update_fast_pen = false;

    if (flags & DirtyOpacity) {
        update_fast_pen = true;
        d->opacity = state.opacity();
        if (d->opacity > 1.0f)
            d->opacity = 1.0f;
        if (d->opacity < 0.f)
            d->opacity = 0.f;
        // force update
        flags |= DirtyPen;
        flags |= DirtyBrush;
    }

    if (flags & DirtyTransform) {
        update_fast_pen = true;
        updateMatrix(state.transform());
        // brush setup depends on transform state
        if (state.brush().style() != Qt::NoBrush)
            flags |= DirtyBrush;
    }

    if (flags & DirtyPen) {
        update_fast_pen = true;
        updatePen(state.pen());
    }

    if (flags & (DirtyBrush | DirtyBrushOrigin)) {
        updateBrush(state.brush(), state.brushOrigin());
    }

    if (flags & DirtyFont) {
        updateFont(state.font());
    }

    if (state.state() & DirtyClipEnabled) {
        if (state.isClipEnabled())
            updateClipRegion(painter()->clipRegion(), Qt::ReplaceClip);
        else
            updateClipRegion(QRegion(), Qt::NoClip);
    }

    if (flags & DirtyClipPath) {
        updateClipRegion(QRegion(state.clipPath().toFillPolygon().toPolygon(),
                                 state.clipPath().fillRule()),
                         state.clipOperation());
    }

    if (flags & DirtyClipRegion) {
        updateClipRegion(state.clipRegion(), state.clipOperation());
    }

    if (flags & DirtyHints) {
        updateRenderHints(state.renderHints());
    }

    if (flags & DirtyCompositionMode) {
        updateCompositionMode(state.compositionMode());
    }

    if (update_fast_pen) {
        Q_D(QOpenGLPaintEngine);
        qreal pen_width = d->cpen.widthF();
        d->has_fast_pen =
            ((pen_width == 0 || (pen_width <= 1 && d->txop <= QTransform::TxTranslate))
             || d->cpen.isCosmetic())
            && d->cpen.style() == Qt::SolidLine
            && d->cpen.isSolid();
    }
}


void QOpenGLPaintEnginePrivate::setInvMatrixData(const QTransform &inv_matrix)
{
    inv_matrix_data[0][0] = inv_matrix.m11();
    inv_matrix_data[1][0] = inv_matrix.m21();
    inv_matrix_data[2][0] = inv_matrix.m31();

    inv_matrix_data[0][1] = inv_matrix.m12();
    inv_matrix_data[1][1] = inv_matrix.m22();
    inv_matrix_data[2][1] = inv_matrix.m32();

    inv_matrix_data[0][2] = inv_matrix.m13();
    inv_matrix_data[1][2] = inv_matrix.m23();
    inv_matrix_data[2][2] = inv_matrix.m33();
}


void QOpenGLPaintEnginePrivate::updateGradient(const QBrush &brush, const QRectF &bounds)
{
#ifdef Q_WS_QWS
    Q_UNUSED(brush);
    Q_UNUSED(bounds);
#else
    bool has_mirrored_repeat = QGLExtensions::glExtensions & QGLExtensions::MirroredRepeat;
    Qt::BrushStyle style = brush.style();

    if (has_mirrored_repeat && style == Qt::LinearGradientPattern) {
        const QLinearGradient *g = static_cast<const QLinearGradient *>(brush.gradient());
        QTransform m = brush.transform();
        QPointF realStart = g->start();
        QPointF realFinal = g->finalStop();
        if (g->coordinateMode() == QGradient::ObjectBoundingMode) {
            qreal sx = bounds.x() + realStart.x() * bounds.width();
            qreal sy = bounds.y() + realStart.y() * bounds.height();
            qreal fx = bounds.x() + realFinal.x() * bounds.width();
            qreal fy = bounds.y() + realFinal.y() * bounds.height();
            realStart = QPointF(sx, sy);
            realFinal = QPointF(fx, fy);
        }
        QPointF start = m.map(realStart);
        QPointF stop;

        if (qFuzzyCompare(m.m11(), m.m22()) && m.m12() == 0.0 && m.m21() == 0.0) {
            // It is a simple uniform scale and/or translation
            stop = m.map(realFinal);
        } else {
            // It is not enough to just transform the endpoints.
            // We have to make sure the _pattern_ is transformed correctly.

            qreal odx = realFinal.x() - realStart.x();
            qreal ody = realFinal.y() - realStart.y();

            // nx, ny and dx, dy are normal and gradient direction after transform:
            qreal nx = m.m11()*ody - m.m21()*odx;
            qreal ny = m.m12()*ody - m.m22()*odx;

            qreal dx = m.m11()*odx + m.m21()*ody;
            qreal dy = m.m12()*odx + m.m22()*ody;

            qreal lx = 1.0/(dx - dy*nx/ny);
            qreal ly = 1.0/(dy - dx*ny/nx);
            qreal l = 1.0/sqrt(lx*lx+ly*ly);

            stop = start + QPointF(-ny, nx) * l/sqrt(nx*nx+ny*ny);
        }

        float tr[4], f;
        tr[0] = stop.x() - start.x();
        tr[1] = stop.y() - start.y();
        f = 1.0 / (tr[0]*tr[0] + tr[1]*tr[1]);
        tr[0] *= f;
        tr[1] *= f;
        tr[2] = 0;
        tr[3] = -(start.x()*tr[0] + start.y()*tr[1]);
        setGLBrush(Qt::white);
        qt_glColor4ubv(brush_color);
        glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
        glTexGenfv(GL_S, GL_OBJECT_PLANE, tr);
    }

    if (use_fragment_programs) {
        if (style == Qt::RadialGradientPattern) {
            const QRadialGradient *g = static_cast<const QRadialGradient *>(brush.gradient());
            QPointF realCenter = g->center();
            QPointF realFocal  = g->focalPoint();
            qreal   realRadius = g->radius();
            if (g->coordinateMode() == QGradient::ObjectBoundingMode) {
                qreal x = bounds.x() + realCenter.x() * bounds.width();
                qreal y = bounds.y() + realCenter.y() * bounds.height();
                realCenter = QPointF(x, y);

                x = bounds.x() + realFocal.x() * bounds.width();
                y = bounds.y() + realFocal.y() * bounds.height();
                realFocal = QPointF(x, y);

                realRadius = qMin(bounds.x() + bounds.width() * realRadius,
                                  bounds.y() + bounds.height() * realRadius);
            }
            QTransform translate(1, 0, 0, 1, -realFocal.x(), -realFocal.y());
            QTransform gl_to_qt(1, 0, 0, -1, 0, pdev->height());
            QTransform inv_matrix = gl_to_qt * matrix.inverted() * brush.transform().inverted() * translate;

            setInvMatrixData(inv_matrix);

            fmp_data[0] = realCenter.x() - realFocal.x();
            fmp_data[1] = realCenter.y() - realFocal.y();

            fmp2_m_radius2_data[0] = -fmp_data[0] * fmp_data[0] - fmp_data[1] * fmp_data[1] + realRadius * realRadius;
        } else if (style == Qt::ConicalGradientPattern) {
            const QConicalGradient *g = static_cast<const QConicalGradient *>(brush.gradient());
            QPointF realCenter = g->center();
            if (g->coordinateMode() == QGradient::ObjectBoundingMode) {
                qreal x = bounds.x() + bounds.width() * realCenter.x();
                qreal y = bounds.y() + bounds.height() * realCenter.y();
                realCenter = QPointF(x, y);
            }
            QTransform translate(1, 0, 0, 1, -realCenter.x(), -realCenter.y());
            QTransform gl_to_qt(1, 0, 0, -1, 0, pdev->height());
            QTransform inv_matrix = gl_to_qt * matrix.inverted() * brush.transform().inverted() * translate;

            setInvMatrixData(inv_matrix);

            angle_data[0] = -(g->angle() * 2 * Q_PI) / 360.0;
        } else if (style == Qt::LinearGradientPattern) {
            const QLinearGradient *g = static_cast<const QLinearGradient *>(brush.gradient());

            QPointF realStart = g->start();
            QPointF realFinal = g->finalStop();
            if (g->coordinateMode() == QGradient::ObjectBoundingMode) {
                qreal sx = bounds.x() + realStart.x() * bounds.width();
                qreal sy = bounds.y() + realStart.y() * bounds.height();
                qreal fx = bounds.x() + realFinal.x() * bounds.width();
                qreal fy = bounds.y() + realFinal.y() * bounds.height();
                realStart = QPointF(sx, sy);
                realFinal = QPointF(fx, fy);
            }

            QTransform translate(1, 0, 0, 1, -realStart.x(), -realStart.y());
            QTransform gl_to_qt(1, 0, 0, -1, 0, pdev->height());

            QTransform inv_matrix = gl_to_qt * matrix.inverted() * brush.transform().inverted() * translate;

            setInvMatrixData(inv_matrix);

            QPointF l = realFinal - realStart;

            linear_data[0] = l.x();
            linear_data[1] = l.y();

            linear_data[2] = 1.0f / (l.x() * l.x() + l.y() * l.y());
        } else if (style != Qt::SolidPattern) {
            QTransform translate(1, 0, 0, 1, brush_origin.x(), brush_origin.y());
            QTransform gl_to_qt(1, 0, 0, -1, 0, pdev->height());

            QTransform inv_matrix = gl_to_qt * matrix.inverted() * brush.transform().inverted() * translate;

            setInvMatrixData(inv_matrix);
        }
    }

    if (style >= Qt::LinearGradientPattern && style <= Qt::ConicalGradientPattern) {
        createGradientPaletteTexture(*brush.gradient());
    }
#endif
}


class QOpenGLTessellator : public QTessellator
{
public:
    QOpenGLTessellator() {}
    ~QOpenGLTessellator() { }
    QGLTrapezoid toGLTrapezoid(const Trapezoid &trap);
};

QGLTrapezoid QOpenGLTessellator::toGLTrapezoid(const Trapezoid &trap)
{
    QGLTrapezoid t;

    t.top = Q27Dot5ToDouble(trap.top);
    t.bottom = Q27Dot5ToDouble(trap.bottom);

    Q27Dot5 y = trap.topLeft->y - trap.bottomLeft->y;

    qreal topLeftY = Q27Dot5ToDouble(trap.topLeft->y);

    qreal tx = Q27Dot5ToDouble(trap.topLeft->x);
    qreal m = (-tx + Q27Dot5ToDouble(trap.bottomLeft->x)) / Q27Dot5ToDouble(y);
    t.topLeftX = tx + m * (topLeftY - t.top);
    t.bottomLeftX = tx + m * (topLeftY - t.bottom);

    y = trap.topRight->y - trap.bottomRight->y;

    qreal topRightY = Q27Dot5ToDouble(trap.topRight->y);

    tx = Q27Dot5ToDouble(trap.topRight->x);
    m = (-tx + Q27Dot5ToDouble(trap.bottomRight->x)) / Q27Dot5ToDouble(y);

    t.topRightX = tx + m * (topRightY - Q27Dot5ToDouble(trap.top));
    t.bottomRightX = tx + m * (topRightY - Q27Dot5ToDouble(trap.bottom));

    return t;
}

class QOpenGLImmediateModeTessellator : public QOpenGLTessellator
{
public:
    void addTrap(const Trapezoid &trap);
    void tessellate(const QPointF *points, int nPoints, bool winding) {
        trapezoids.reserve(trapezoids.size() + nPoints);
        setWinding(winding);
        QTessellator::tessellate(points, nPoints);
    }

    QVector<QGLTrapezoid> trapezoids;
};

void QOpenGLImmediateModeTessellator::addTrap(const Trapezoid &trap)
{
    trapezoids.append(toGLTrapezoid(trap));
}

#ifndef Q_WS_QWS
static void drawTrapezoid(const QGLTrapezoid &trap, const qreal offscreenHeight, QGLContext *ctx)
{
    qreal minX = qMin(trap.topLeftX, trap.bottomLeftX);
    qreal maxX = qMax(trap.topRightX, trap.bottomRightX);

    if (qFuzzyCompare(trap.top, trap.bottom) || qFuzzyCompare(minX, maxX) ||
        qFuzzyCompare(trap.topLeftX, trap.topRightX) && qFuzzyCompare(trap.bottomLeftX, trap.bottomRightX))
        return;

    const qreal xpadding = 1.0;
    const qreal ypadding = 1.0;

    qreal topDist = offscreenHeight - trap.top;
    qreal bottomDist = offscreenHeight - trap.bottom;

    qreal reciprocal = bottomDist / (bottomDist - topDist);

    qreal leftB = trap.bottomLeftX + (trap.topLeftX - trap.bottomLeftX) * reciprocal;
    qreal rightB = trap.bottomRightX + (trap.topRightX - trap.bottomRightX) * reciprocal;

    const bool topZero = qFuzzyCompare(topDist, 0);

    reciprocal = topZero ? 1.0 / bottomDist : 1.0 / topDist;

    qreal leftA = topZero ? (trap.bottomLeftX - leftB) * reciprocal : (trap.topLeftX - leftB) * reciprocal;
    qreal rightA = topZero ? (trap.bottomRightX - rightB) * reciprocal : (trap.topRightX - rightB) * reciprocal;

    qreal invLeftA = qFuzzyCompare(leftA, qreal(0.0)) ? 0.0 : 1.0 / leftA;
    qreal invRightA = qFuzzyCompare(rightA, qreal(0.0)) ? 0.0 : 1.0 / rightA;

    // fragment program needs the negative of invRightA as it mirrors the line
    glTexCoord4f(topDist, bottomDist, invLeftA, -invRightA);
    glMultiTexCoord4f(GL_TEXTURE1, leftA, leftB, rightA, rightB);

    qreal topY = trap.top - ypadding;
    qreal bottomY = trap.bottom + ypadding;

    qreal bounds_bottomLeftX = leftA * (offscreenHeight - bottomY) + leftB;
    qreal bounds_bottomRightX = rightA * (offscreenHeight - bottomY) + rightB;
    qreal bounds_topLeftX = leftA * (offscreenHeight - topY) + leftB;
    qreal bounds_topRightX = rightA * (offscreenHeight - topY) + rightB;

    QPointF leftNormal(1, -leftA);
    leftNormal /= sqrt(leftNormal.x() * leftNormal.x() + leftNormal.y() * leftNormal.y());
    QPointF rightNormal(1, -rightA);
    rightNormal /= sqrt(rightNormal.x() * rightNormal.x() + rightNormal.y() * rightNormal.y());

    qreal left_padding = xpadding / qAbs(leftNormal.x());
    qreal right_padding = xpadding / qAbs(rightNormal.x());

    glVertex2d(bounds_topLeftX - left_padding, topY);
    glVertex2d(bounds_topRightX + right_padding, topY);
    glVertex2d(bounds_bottomRightX + right_padding, bottomY);
    glVertex2d(bounds_bottomLeftX - left_padding, bottomY);

    glTexCoord4f(0.0f, 0.0f, 0.0f, 1.0f);
}
#endif // !Q_WS_QWS

class QOpenGLTrapezoidToArrayTessellator : public QOpenGLTessellator
{
public:
    QOpenGLTrapezoidToArrayTessellator() : vertices(0), allocated(0), size(0) {}
    ~QOpenGLTrapezoidToArrayTessellator() { free(vertices); }
    float *vertices;
    int allocated;
    int size;
    QRectF bounds;
    void addTrap(const Trapezoid &trap);
    void tessellate(const QPointF *points, int nPoints, bool winding) {
        size = 0;
        setWinding(winding);
        bounds = QTessellator::tessellate(points, nPoints);
    }
};

void QOpenGLTrapezoidToArrayTessellator::addTrap(const Trapezoid &trap)
{
#ifndef Q_WS_QWS
    if (size > allocated - 8) {
#else
    if (size > allocated - 12) {
#endif
        allocated = qMax(2*allocated, 512);
        vertices = (float *)realloc(vertices, allocated * sizeof(float));
    }

    QGLTrapezoid t = toGLTrapezoid(trap);

#ifndef Q_WS_QWS
    vertices[size++] = t.topLeftX;
    vertices[size++] = t.top;
    vertices[size++] = t.topRightX;
    vertices[size++] = t.top;
    vertices[size++] = t.bottomRightX;
    vertices[size++] = t.bottom;
    vertices[size++] = t.bottomLeftX;
    vertices[size++] = t.bottom;
#else
    vertices[size++] = t.topLeftX;
    vertices[size++] = t.top;
    vertices[size++] = t.topRightX;
    vertices[size++] = t.top;
    vertices[size++] = t.bottomRightX;
    vertices[size++] = t.bottom;

    vertices[size++] = t.bottomLeftX;
    vertices[size++] = t.bottom;
    vertices[size++] = t.topLeftX;
    vertices[size++] = t.top;
    vertices[size++] = t.bottomRightX;
    vertices[size++] = t.bottom;
#endif
}


void QOpenGLPaintEnginePrivate::fillPolygon_dev(const QPointF *polygonPoints, int pointCount,
                                                Qt::FillRule fill)
{
    QOpenGLTrapezoidToArrayTessellator tessellator;
    tessellator.tessellate(polygonPoints, pointCount, fill == Qt::WindingFill);

    DEBUG_ONCE qDebug() << "QOpenGLPaintEnginePrivate: Drawing polygon with" << pointCount << "points using fillPolygon_dev";

    setGradientOps(cbrush, tessellator.bounds);

    bool fast_style = current_style == Qt::LinearGradientPattern
                      || current_style == Qt::SolidPattern;

#ifndef Q_WS_QWS
    GLenum geometry_mode = GL_QUADS;
#else
    GLenum geometry_mode = GL_TRIANGLES;
#endif

    if (use_fragment_programs && !(fast_style && has_fast_composition_mode)) {
        composite(geometry_mode, tessellator.vertices, tessellator.size / 2);
    } else {
        glVertexPointer(2, GL_FLOAT, 0, tessellator.vertices);
        glEnableClientState(GL_VERTEX_ARRAY);
        glDrawArrays(geometry_mode, 0, tessellator.size/2);
        glDisableClientState(GL_VERTEX_ARRAY);
    }
}


inline void QOpenGLPaintEnginePrivate::lineToStencil(qreal x, qreal y)
{
    tess_points.add(QPointF(x, y));

    if (x > max_x)
        max_x = x;
    else if (x < min_x)
        min_x = x;
    if (y > max_y)
        max_y = y;
    else if (y < min_y)
        min_y = y;
}

inline void QOpenGLPaintEnginePrivate::curveToStencil(const QPointF &cp1, const QPointF &cp2, const QPointF &ep)
{
    qreal inverseScaleHalf = inverseScale / 2;

    QBezier beziers[32];
    beziers[0] = QBezier::fromPoints(tess_points.last(), cp1, cp2, ep);
    QBezier *b = beziers;
    while (b >= beziers) {
        // check if we can pop the top bezier curve from the stack
        qreal l = qAbs(b->x4 - b->x1) + qAbs(b->y4 - b->y1);
        qreal d;
        if (l > inverseScale) {
            d = qAbs( (b->x4 - b->x1)*(b->y1 - b->y2) - (b->y4 - b->y1)*(b->x1 - b->x2) )
                + qAbs( (b->x4 - b->x1)*(b->y1 - b->y3) - (b->y4 - b->y1)*(b->x1 - b->x3) );
            d /= l;
        } else {
            d = qAbs(b->x1 - b->x2) + qAbs(b->y1 - b->y2) +
                qAbs(b->x1 - b->x3) + qAbs(b->y1 - b->y3);
        }
        if (d < inverseScaleHalf || b == beziers + 31) {
            // good enough, we pop it off and add the endpoint
            lineToStencil(b->x4, b->y4);
            --b;
        } else {
            // split, second half of the polygon goes lower into the stack
            b->split(b+1, b);
           ++b;
        }
    }
}


void QOpenGLPaintEnginePrivate::pathToVertexArrays(const QPainterPath &path)
{
    const QPainterPath::Element &first = path.elementAt(0);
    min_x = max_x = first.x;
    min_y = max_y = first.y;

    tess_points.reset();
    tess_points_stops.clear();
    lineToStencil(first.x, first.y);

    for (int i=1; i<path.elementCount(); ++i) {
        const QPainterPath::Element &e = path.elementAt(i);
        switch (e.type) {
        case QPainterPath::MoveToElement:
            tess_points_stops.append(tess_points.size());
            lineToStencil(e.x, e.y);
            break;
        case QPainterPath::LineToElement:
            lineToStencil(e.x, e.y);
            break;
        case QPainterPath::CurveToElement:
            curveToStencil(e, path.elementAt(i+1), path.elementAt(i+2));
            i+=2;
            break;
        default:
            break;
        }
    }
    lineToStencil(first.x, first.y);
    tess_points_stops.append(tess_points.size());
}


void QOpenGLPaintEnginePrivate::drawVertexArrays()
{
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_DOUBLE, 0, tess_points.data());
    int previous_stop = 0;
    foreach(int stop, tess_points_stops) {
        glDrawArrays(GL_TRIANGLE_FAN, previous_stop, stop-previous_stop);
        previous_stop = stop;
    }
    glDisableClientState(GL_VERTEX_ARRAY);
}

void QOpenGLPaintEnginePrivate::fillVertexArray(Qt::FillRule fillRule)
{
    glStencilMask(~0);

    // Enable stencil.
    glEnable(GL_STENCIL_TEST);

    // Disable color writes.
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

    GLuint stencilMask = 0;

    if (fillRule == Qt::OddEvenFill) {
        stencilMask = 1;

        // Enable stencil writes.
        glStencilMask(stencilMask);

        // Set stencil xor mode.
        glStencilOp(GL_KEEP, GL_KEEP, GL_INVERT);

        // Disable stencil func.
        glStencilFunc(GL_ALWAYS, 0, ~0);

        drawVertexArrays();
    } else if (fillRule == Qt::WindingFill) {
        stencilMask = ~0;

        if (has_stencil_face_ext) {
            QGL_FUNC_CONTEXT;
            glEnable(GL_STENCIL_TEST_TWO_SIDE_EXT);

            glActiveStencilFaceEXT(GL_BACK);
            glStencilOp(GL_KEEP, GL_KEEP, GL_DECR_WRAP_EXT);
            glStencilFunc(GL_ALWAYS, 0, ~0);

            glActiveStencilFaceEXT(GL_FRONT);
            glStencilOp(GL_KEEP, GL_KEEP, GL_INCR_WRAP_EXT);
            glStencilFunc(GL_ALWAYS, 0, ~0);

            drawVertexArrays();

            glDisable(GL_STENCIL_TEST_TWO_SIDE_EXT);
        } else {
            glStencilFunc(GL_ALWAYS, 0, ~0);
            glEnable(GL_CULL_FACE);

            glCullFace(GL_BACK);
            glStencilOp(GL_KEEP, GL_KEEP, GL_INCR_WRAP_EXT);
            drawVertexArrays();

            glCullFace(GL_FRONT);
            glStencilOp(GL_KEEP, GL_KEEP, GL_DECR_WRAP_EXT);
            drawVertexArrays();

            glDisable(GL_CULL_FACE);
        }
    }

    // Enable color writes.
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glStencilMask(0);

    setGradientOps(cbrush, QRectF(QPointF(min_x, min_y), QSizeF(max_x - min_x, max_y - min_y)));

    bool fast_fill = has_fast_composition_mode && (current_style == Qt::LinearGradientPattern || current_style == Qt::SolidPattern);

    if (use_fragment_programs && !fast_fill) {
        DEBUG_ONCE qDebug() << "QOpenGLPaintEnginePrivate: Drawing polygon using stencil method (fragment programs)";
        QRectF rect(QPointF(min_x, min_y), QSizeF(max_x - min_x, max_y - min_y));

        // Enable stencil func.
        glStencilFunc(GL_NOTEQUAL, 0, stencilMask);
        composite(rect);
    } else {
        DEBUG_ONCE qDebug() << "QOpenGLPaintEnginePrivate: Drawing polygon using stencil method (no fragment programs)";

        // Enable stencil func.
        glStencilFunc(GL_NOTEQUAL, 0, stencilMask);
#ifndef Q_WS_QWS
        glBegin(GL_QUADS);
        glVertex2f(min_x, min_y);
        glVertex2f(max_x, min_y);
        glVertex2f(max_x, max_y);
        glVertex2f(min_x, max_y);
        glEnd();
#endif
    }

    glStencilMask(~0);
    glStencilFunc(GL_ALWAYS, 0, 0);
    glStencilOp(GL_ZERO, GL_ZERO, GL_ZERO);

    // clear all stencil values to 0
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

#ifndef Q_WS_QWS
    glBegin(GL_QUADS);
    glVertex2f(min_x, min_y);
    glVertex2f(max_x, min_y);
    glVertex2f(max_x, max_y);
    glVertex2f(min_x, max_y);
    glEnd();
#endif

    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    // Disable stencil writes.
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    glStencilMask(0);
    glDisable(GL_STENCIL_TEST);
}

void QOpenGLPaintEnginePrivate::fillPath(const QPainterPath &path)
{
    if (path.isEmpty())
        return;

    if (use_stencil_method && !high_quality_antialiasing) {
        pathToVertexArrays(path);
        fillVertexArray(path.fillRule());
        return;
    }

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    if (high_quality_antialiasing)
        drawOffscreenPath(path);
    else {
        QPolygonF poly = path.toFillPolygon(matrix);
        fillPolygon_dev(poly.data(), poly.count(),
                        path.fillRule());
    }

    glMatrixMode(GL_MODELVIEW);

#ifndef Q_WS_QWS
    GLdouble mat[4][4];
#else
    GLfloat mat[4][4];
#endif
    QTransform mtx = matrix;
    mat[0][0] = mtx.m11();
    mat[0][1] = mtx.m12();
    mat[0][2] = 0;
    mat[0][3] = mtx.m13();

    mat[1][0] = mtx.m21();
    mat[1][1] = mtx.m22();
    mat[1][2] = 0;
    mat[1][3] = mtx.m23();

    mat[2][0] = 0;
    mat[2][1] = 0;
    mat[2][2] = 1;
    mat[2][3] = 0;

    mat[3][0] = mtx.dx();
    mat[3][1] = mtx.dy();
    mat[3][2] = 0;
    mat[3][3] = 1;

#ifndef Q_WS_QWS
    glLoadMatrixd(&mat[0][0]);
#else
    glLoadMatrixf(&mat[0][0]);
#endif
}

void QOpenGLPaintEngine::updatePen(const QPen &pen)
{
    Q_D(QOpenGLPaintEngine);
    Qt::PenStyle pen_style = pen.style();
    d->pen_brush_style = pen.brush().style();
    d->cpen = pen;
    d->has_pen = (pen_style != Qt::NoPen);

    if (pen.isCosmetic()) {
        GLfloat width = pen.widthF() == 0.0f ? 1.0f : pen.widthF();
        glLineWidth(width);
        glPointSize(width);
    }

    if (d->pen_brush_style >= Qt::LinearGradientPattern
        && d->pen_brush_style <= Qt::ConicalGradientPattern)
    {
        d->setGLPen(Qt::white);
    } else {
        d->setGLPen(pen.color());
    }
}

void QOpenGLPaintEngine::updateBrush(const QBrush &brush, const QPointF &origin)
{
    Q_D(QOpenGLPaintEngine);
    d->cbrush = brush;
    d->brush_style = brush.style();
    d->brush_origin = origin;
    d->has_brush = (d->brush_style != Qt::NoBrush);
}

void QOpenGLPaintEngine::updateFont(const QFont &)
{
}

void QOpenGLPaintEngine::updateMatrix(const QTransform &mtx)
{
    Q_D(QOpenGLPaintEngine);

    d->matrix = mtx;
#ifndef Q_WS_QWS
    GLdouble mat[4][4];
#else
    GLfloat mat[4][4];
#endif

    mat[0][0] = mtx.m11();
    mat[0][1] = mtx.m12();
    mat[0][2] = 0;
    mat[0][3] = mtx.m13();

    mat[1][0] = mtx.m21();
    mat[1][1] = mtx.m22();
    mat[1][2] = 0;
    mat[1][3] = mtx.m23();

    mat[2][0] = 0;
    mat[2][1] = 0;
    mat[2][2] = 1;
    mat[2][3] = 0;

    mat[3][0] = mtx.dx();
    mat[3][1] = mtx.dy();
    mat[3][2] = 0;
    mat[3][3] = 1;

    d->txop = mtx.type();

    // 1/10000 == 0.0001, so we have good enough res to cover curves
    // that span the entire widget...
    d->inverseScale = qMax(1 / qMax( qMax(qAbs(mtx.m11()), qAbs(mtx.m22())),
                                     qMax(qAbs(mtx.m12()), qAbs(mtx.m21())) ),
                           qreal(0.0001));

    glMatrixMode(GL_MODELVIEW);
#ifndef Q_WS_QWS
    glLoadMatrixd(&mat[0][0]);
#else
    glLoadMatrixf(&mat[0][0]);
#endif
}

void QOpenGLPaintEngine::updateClipRegion(const QRegion &clipRegion, Qt::ClipOperation op)
{
    Q_D(QOpenGLPaintEngine);
    QGLFormat f = d->drawable.format();
    bool useDepthBuffer = f.depth();

    // clipping is only supported when a stencil or depth buffer is
    // available
    if (!useDepthBuffer)
        return;

    d->flushDrawQueue();

#ifdef Q_WS_QWS
    QRegion sysClip = systemClip();
    if (sysClip.isEmpty() && op == Qt::NoClip) {
#else
    if (op == Qt::NoClip) {
#endif
        d->has_clipping = false;
        d->crgn = QRegion();
        glDisable(GL_DEPTH_TEST);
        return;
    }

    bool isScreenClip = false;
    QVector<QRect> untransformedRects = clipRegion.rects();

    if (untransformedRects.size() == 1) {
        QPainterPath path;
        path.addRect(untransformedRects[0]);
        path = d->matrix.map(path);

        if (path.contains(QRectF(QPointF(), d->drawable.size())))
            isScreenClip = true;
    }

    QRegion region = isScreenClip ? QRegion() : clipRegion * d->matrix;
    switch (op) {
#ifdef Q_WS_QWS
    case Qt::NoClip:
        d->has_clipping = false;
        d->crgn = sysClip;
        break;
#endif
    case Qt::IntersectClip:
        if (isScreenClip)
            return;
        if (d->has_clipping) {
            d->crgn &= region;
            break;
        }
        // fall through
    case Qt::ReplaceClip:
#ifdef Q_WS_QWS
        if (!sysClip.isEmpty())
            d->crgn = region.intersected(sysClip);
        else
#endif
            d->crgn = region;
        break;
    case Qt::UniteClip:
        d->crgn |= region;
#ifdef Q_WS_QWS
        if (!sysClip.isEmpty())
            d->crgn = d->crgn.intersected(sysClip);
#endif
        break;
    default:
        break;
    }

    if (isScreenClip) {
        d->has_clipping = false;
        d->crgn = QRegion();
        glDisable(GL_DEPTH_TEST);
        return;
    }

#ifndef Q_WS_QWS
    glClearDepth(0x0);
#else
    glClearDepthf(0x0);
#endif
    glClear(GL_DEPTH_BUFFER_BIT);
    glDepthMask(true);
#ifndef Q_WS_QWS
    glClearDepth(0x1);
#else
    glClearDepthf(0x1);
#endif

    const QVector<QRect> rects = d->crgn.rects();
    glEnable(GL_SCISSOR_TEST);
    for (int i = 0; i < rects.size(); ++i) {
        glScissor(rects.at(i).left(), d->drawable.size().height() - rects.at(i).bottom(),
                  rects.at(i).width(), rects.at(i).height());
        glClear(GL_DEPTH_BUFFER_BIT);
    }
    glDisable(GL_SCISSOR_TEST);

    glDepthFunc(GL_LEQUAL);
    glEnable(GL_DEPTH_TEST);
    d->has_clipping = true;
}

void QOpenGLPaintEngine::updateRenderHints(QPainter::RenderHints hints)
{
    Q_D(QOpenGLPaintEngine);

    d->flushDrawQueue();

    if (hints & QPainter::Antialiasing) {
        if (d->use_fragment_programs && QGLOffscreen::isSupported() && hints & QPainter::HighQualityAntialiasing)
            d->high_quality_antialiasing = true;
        else {
            d->high_quality_antialiasing = false;
            if (QGLExtensions::glExtensions & QGLExtensions::SampleBuffers)
                glEnable(GL_MULTISAMPLE);
        }
    } else {
        d->high_quality_antialiasing = false;
        if (QGLExtensions::glExtensions & QGLExtensions::SampleBuffers)
            glDisable(GL_MULTISAMPLE);
    }

    if (d->high_quality_antialiasing) {
        d->offscreen.initialize();

        if (!d->offscreen.isValid()) {
            DEBUG_ONCE_STR("Unable to initialize offscreen, disabling high quality antialiasing");
            d->high_quality_antialiasing = false;
            if (QGLExtensions::glExtensions & QGLExtensions::SampleBuffers)
                glEnable(GL_MULTISAMPLE);
        }
    }
}


void QOpenGLPaintEnginePrivate::setPorterDuffData(float a, float b, float x, float y, float z)
{
    porterduff_ab_data[0] = a;
    porterduff_ab_data[1] = b;

    porterduff_xyz_data[0] = x;
    porterduff_xyz_data[1] = y;
    porterduff_xyz_data[2] = z;
}


void QOpenGLPaintEngine::updateCompositionMode(QPainter::CompositionMode composition_mode)
{
    Q_D(QOpenGLPaintEngine);
    d->composition_mode = composition_mode;

    d->has_fast_composition_mode = !d->high_quality_antialiasing
                                   && composition_mode <= QPainter::CompositionMode_Plus
                                   || composition_mode == QPainter::CompositionMode_SourceOver
                                   || composition_mode == QPainter::CompositionMode_Destination
                                   || composition_mode == QPainter::CompositionMode_DestinationOver
                                   || composition_mode == QPainter::CompositionMode_DestinationOut
                                   || composition_mode == QPainter::CompositionMode_SourceAtop
                                   || composition_mode == QPainter::CompositionMode_Xor
                                   || composition_mode == QPainter::CompositionMode_Plus;

    if (d->has_fast_composition_mode)
        d->fragment_composition_mode = d->high_quality_antialiasing ? COMPOSITION_MODE_BLEND_MODE_MASK : COMPOSITION_MODE_BLEND_MODE_NOMASK;
    else if (composition_mode <= QPainter::CompositionMode_Plus)
        d->fragment_composition_mode = d->high_quality_antialiasing ? COMPOSITION_MODES_SIMPLE_PORTER_DUFF : COMPOSITION_MODES_SIMPLE_PORTER_DUFF_NOMASK;
    else
        switch (composition_mode) {
        case QPainter::CompositionMode_Multiply:
            d->fragment_composition_mode = d->high_quality_antialiasing ? COMPOSITION_MODES_MULTIPLY : COMPOSITION_MODES_MULTIPLY_NOMASK;
            break;
        case QPainter::CompositionMode_Screen:
            d->fragment_composition_mode = d->high_quality_antialiasing ? COMPOSITION_MODES_SCREEN : COMPOSITION_MODES_SCREEN_NOMASK;
            break;
        case QPainter::CompositionMode_Overlay:
            d->fragment_composition_mode = d->high_quality_antialiasing ? COMPOSITION_MODES_OVERLAY : COMPOSITION_MODES_OVERLAY_NOMASK;
            break;
        case QPainter::CompositionMode_Darken:
            d->fragment_composition_mode = d->high_quality_antialiasing ? COMPOSITION_MODES_DARKEN : COMPOSITION_MODES_DARKEN_NOMASK;
            break;
        case QPainter::CompositionMode_Lighten:
            d->fragment_composition_mode = d->high_quality_antialiasing ? COMPOSITION_MODES_LIGHTEN : COMPOSITION_MODES_LIGHTEN_NOMASK;
            break;
        case QPainter::CompositionMode_ColorDodge:
            d->fragment_composition_mode = d->high_quality_antialiasing ? COMPOSITION_MODES_COLORDODGE : COMPOSITION_MODES_COLORDODGE_NOMASK;
            break;
        case QPainter::CompositionMode_ColorBurn:
            d->fragment_composition_mode = d->high_quality_antialiasing ? COMPOSITION_MODES_COLORBURN : COMPOSITION_MODES_COLORBURN_NOMASK;
            break;
        case QPainter::CompositionMode_HardLight:
            d->fragment_composition_mode = d->high_quality_antialiasing ? COMPOSITION_MODES_HARDLIGHT : COMPOSITION_MODES_HARDLIGHT_NOMASK;
            break;
        case QPainter::CompositionMode_SoftLight:
            d->fragment_composition_mode = d->high_quality_antialiasing ? COMPOSITION_MODES_SOFTLIGHT : COMPOSITION_MODES_SOFTLIGHT_NOMASK;
            break;
        case QPainter::CompositionMode_Difference:
            d->fragment_composition_mode = d->high_quality_antialiasing ? COMPOSITION_MODES_DIFFERENCE : COMPOSITION_MODES_DIFFERENCE_NOMASK;
            break;
        case QPainter::CompositionMode_Exclusion:
            d->fragment_composition_mode = d->high_quality_antialiasing ? COMPOSITION_MODES_EXCLUSION : COMPOSITION_MODES_EXCLUSION_NOMASK;
            break;
        default:
            Q_ASSERT(false);
        }

    switch(composition_mode) {
    case QPainter::CompositionMode_DestinationOver:
        glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_ONE);
        d->setPorterDuffData(0, 1, 1, 1, 1);
        break;
    case QPainter::CompositionMode_Clear:
        glBlendFunc(GL_ZERO, GL_ZERO);
        d->setPorterDuffData(0, 0, 0, 0, 0);
        break;
    case QPainter::CompositionMode_Source:
        glBlendFunc(GL_ONE, GL_ZERO);
        d->setPorterDuffData(1, 0, 1, 1, 0);
        break;
    case QPainter::CompositionMode_Destination:
        glBlendFunc(GL_ZERO, GL_ONE);
        d->setPorterDuffData(0, 1, 1, 0, 1);
        break;
    case QPainter::CompositionMode_SourceIn:
        glBlendFunc(GL_DST_ALPHA, GL_ZERO);
        d->setPorterDuffData(1, 0, 1, 0, 0);
        break;
    case QPainter::CompositionMode_DestinationIn:
        glBlendFunc(GL_ZERO, GL_SRC_ALPHA);
        d->setPorterDuffData(0, 1, 1, 0, 0);
        break;
    case QPainter::CompositionMode_SourceOut:
        glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_ZERO);
        d->setPorterDuffData(0, 0, 0, 1, 0);
        break;
    case QPainter::CompositionMode_DestinationOut:
        glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);
        d->setPorterDuffData(0, 0, 0, 0, 1);
        break;
    case QPainter::CompositionMode_SourceAtop:
        glBlendFunc(GL_DST_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        d->setPorterDuffData(1, 0, 1, 0, 1);
        break;
    case QPainter::CompositionMode_DestinationAtop:
        glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_SRC_ALPHA);
        d->setPorterDuffData(0, 1, 1, 1, 0);
        break;
    case QPainter::CompositionMode_Xor:
        glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        d->setPorterDuffData(0, 0, 0, 1, 1);
        break;
    case QPainter::CompositionMode_SourceOver:
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        d->setPorterDuffData(1, 0, 1, 1, 1);
        break;
    case QPainter::CompositionMode_Plus:
        glBlendFunc(GL_ONE, GL_ONE);
        d->setPorterDuffData(1, 1, 1, 1, 1);
        break;
    default:
        break;
    }
}

class QGLMaskGenerator
{
public:
    QGLMaskGenerator(const QPainterPath &path, const QTransform &matrix, qreal stroke_width = -1)
        : p(path),
          m(matrix),
          w(stroke_width)
    {
    }

    virtual QRect screenRect() = 0;
    virtual void drawMask(const QRect &rect) = 0;

    QPainterPath path() const { return p; }
    QTransform matrix() const { return m; }
    qreal strokeWidth() const { return w; }

    virtual ~QGLMaskGenerator() {}

private:
    QPainterPath p;
    QTransform m;
    qreal w;
};

void QGLMaskTextureCache::setOffscreenSize(const QSize &sz)
{
    Q_ASSERT(sz.width() == sz.height());

    if (offscreenSize != sz) {
        offscreenSize = sz;
        clearCache();
    }
}

void QGLMaskTextureCache::clearCache()
{
    cache.clear();

    int quad_tree_size = 1;

    for (int i = block_size; i < offscreenSize.width(); i *= 2)
        quad_tree_size += quad_tree_size * 4;

    for (int i = 0; i < 4; ++i) {
        occupied_quadtree[i].resize(quad_tree_size);

        occupied_quadtree[i][0].key = 0;
        occupied_quadtree[i][0].largest_available_block = offscreenSize.width();
        occupied_quadtree[i][0].largest_used_block = 0;

        DEBUG_ONCE qDebug() << "QGLMaskTextureCache:: created quad tree of size" << quad_tree_size;
    }
}

void QGLMaskTextureCache::setDrawableSize(const QSize &sz)
{
    drawableSize = sz;
}

void QGLMaskTextureCache::maintainCache()
{
    QGLTextureCacheHash::iterator it = cache.begin();
    QGLTextureCacheHash::iterator end = cache.end();

    while (it != end) {
        CacheInfo &cache_info = it.value();
        ++cache_info.age;

        if (cache_info.age > 1) {
            quadtreeInsert(cache_info.loc.channel, 0, cache_info.loc.rect);
            it = cache.erase(it);
        } else {
            ++it;
        }
    }
}

//#define DISABLE_MASK_CACHE

QGLMaskTextureCache::CacheLocation QGLMaskTextureCache::getMask(QGLMaskGenerator &maskGenerator, QOpenGLPaintEnginePrivate *e)
{
#ifndef DISABLE_MASK_CACHE
    engine = e;

    quint64 key = hash(maskGenerator.path(), maskGenerator.matrix(), maskGenerator.strokeWidth());

    if (key == 0)
        key = 1;

    CacheInfo info(maskGenerator.path(), maskGenerator.matrix(), maskGenerator.strokeWidth());

    QGLTextureCacheHash::iterator it = cache.find(key);

    while (it != cache.end() && it.key() == key) {
        CacheInfo &cache_info = it.value();
        if (info.stroke_width == cache_info.stroke_width && info.matrix == cache_info.matrix && info.path == cache_info.path) {
            DEBUG_ONCE_STR("QGLMaskTextureCache::getMask(): Using cached mask");

            cache_info.age = 0;
            return cache_info.loc;
        }
        ++it;
    }

    // mask was not found, create new mask

    DEBUG_ONCE_STR("QGLMaskTextureCache::getMask(): Creating new mask...");

    createMask(key, info, maskGenerator);

    cache.insert(key, info);

    return info.loc;
#else
    CacheInfo info(maskGenerator.path(), maskGenerator.matrix());
    createMask(0, info, maskGenerator);
    return info.loc;
#endif
}

#ifndef FloatToQuint64
#define FloatToQuint64(i) (quint64)((i) * 32)
#endif

quint64 QGLMaskTextureCache::hash(const QPainterPath &p, const QTransform &m, qreal w)
{
    Q_ASSERT(sizeof(quint64) == 8);

    quint64 h = 0;

    for (int i = 0; i < p.elementCount(); ++i) {
        h += FloatToQuint64(p.elementAt(i).x) << 32;
        h += FloatToQuint64(p.elementAt(i).y);
        h += p.elementAt(i).type;
    }

    h += FloatToQuint64(m.m11());
    h += FloatToQuint64(m.m12()) << 4;
    h += FloatToQuint64(m.m13()) << 8;
    h += FloatToQuint64(m.m21()) << 12;
    h += FloatToQuint64(m.m22()) << 16;
    h += FloatToQuint64(m.m23()) << 20;
    h += FloatToQuint64(m.m31()) << 24;
    h += FloatToQuint64(m.m32()) << 28;
    h += FloatToQuint64(m.m33()) << 32;

    h += FloatToQuint64(w);

    return h;
}

void QGLMaskTextureCache::createMask(quint64 key, CacheInfo &info, QGLMaskGenerator &maskGenerator)
{
    info.loc.screen_rect = maskGenerator.screenRect();

    if (info.loc.screen_rect.isEmpty()) {
        info.loc.channel = 0;
        info.loc.rect = QRect();
        return;
    }

    quadtreeAllocate(key, info.loc.screen_rect.size(), &info.loc.rect, &info.loc.channel);

    int ch = info.loc.channel;
    glColorMask(ch == 0, ch == 1, ch == 2, ch == 3);

    maskGenerator.drawMask(info.loc.rect);

    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
}

int QGLMaskTextureCache::quadtreeBlocksize(int node)
{
    DEBUG_ONCE qDebug() << "Offscreen size:" << offscreenSize.width();

    int blocksize = offscreenSize.width();

    while (node) {
        node = (node - 1) / 4;
        blocksize /= 2;
    }

    return blocksize;
}

QPoint QGLMaskTextureCache::quadtreeLocation(int node)
{
    QPoint location;
    int blocksize = quadtreeBlocksize(node);

    while (node) {
        --node;

        if (node & 1)
            location.setX(location.x() + blocksize);

        if (node & 2)
            location.setY(location.y() + blocksize);

        node /= 4;
        blocksize *= 2;
    }

    return location;
}

void QGLMaskTextureCache::quadtreeUpdate(int channel, int node, int current_block_size)
{
    while (node) {
        node = (node - 1) / 4;

        int first_child = node * 4 + 1;

        int largest_available = 0;
        int largest_used = 0;

        bool all_empty = true;

        for (int i = 0; i < 4; ++i) {
            largest_available = qMax(largest_available, occupied_quadtree[channel][first_child + i].largest_available_block);
            largest_used = qMax(largest_used, occupied_quadtree[channel][first_child + i].largest_used_block);

            if (occupied_quadtree[channel][first_child + i].largest_available_block < current_block_size)
                all_empty = false;
        }

        current_block_size *= 2;

        if (all_empty) {
            occupied_quadtree[channel][node].largest_available_block = current_block_size;
            occupied_quadtree[channel][node].largest_used_block = 0;
        } else {
            occupied_quadtree[channel][node].largest_available_block = largest_available;
            occupied_quadtree[channel][node].largest_used_block = largest_used;
        }
    }
}

void QGLMaskTextureCache::quadtreeInsert(int channel, quint64 key, const QRect &rect, int node)
{
    int current_block_size = quadtreeBlocksize(node);
    QPoint location = quadtreeLocation(node);
    QRect relative = rect.translated(-location);

    if (relative.left() >= current_block_size || relative.top() >= current_block_size
        || relative.right() < 0 || relative.bottom() < 0)
        return;

    if (current_block_size == block_size // no more refining possible
        || relative.top() < block_size && relative.bottom() >= (current_block_size - block_size)
           && relative.left() < block_size && relative.right() >= (current_block_size - block_size))
    {
        if (key != 0) {
            occupied_quadtree[channel][node].largest_available_block = 0;
            occupied_quadtree[channel][node].largest_used_block = rect.width() * rect.height();
        } else {
            occupied_quadtree[channel][node].largest_available_block = current_block_size;
            occupied_quadtree[channel][node].largest_used_block = 0;
        }

        occupied_quadtree[channel][node].key = key;

        quadtreeUpdate(channel, node, current_block_size);
    } else {
        if (key && occupied_quadtree[channel][node].largest_available_block == current_block_size) {
            // refining the quad tree, initialize child nodes
            int half_block_size = current_block_size / 2;

            int temp = node * 4 + 1;
            for (int sibling = 0; sibling < 4; ++sibling) {
                occupied_quadtree[channel][temp + sibling].largest_available_block = half_block_size;
                occupied_quadtree[channel][temp + sibling].largest_used_block = 0;
                occupied_quadtree[channel][temp + sibling].key = 0;
            }
        }

        node = node * 4 + 1;

        for (int sibling = 0; sibling < 4; ++sibling)
            quadtreeInsert(channel, key, rect, node + sibling);
    }
}

void QGLMaskTextureCache::quadtreeClear(int channel, const QRect &rect, int node)
{
    const quint64 &key = occupied_quadtree[channel][node].key;

    int current_block_size = quadtreeBlocksize(node);
    QPoint location = quadtreeLocation(node);

    QRect relative = rect.translated(-location);

    if (relative.left() >= current_block_size || relative.top() >= current_block_size
        || relative.right() < 0 || relative.bottom() < 0)
        return;

    if (key != 0) {
        QGLTextureCacheHash::iterator it = cache.find(key);

        Q_ASSERT(it != cache.end());

        while (it != cache.end() && it.key() == key) {
            const CacheInfo &cache_info = it.value();

            if (cache_info.loc.channel == channel
                && cache_info.loc.rect.left() <= location.x()
                && cache_info.loc.rect.top() <= location.y()
                && cache_info.loc.rect.right() >= location.x()
                && cache_info.loc.rect.bottom() >= location.y())
            {
                quadtreeInsert(channel, 0, cache_info.loc.rect);
                engine->cacheItemErased(channel, cache_info.loc.rect);
                cache.erase(it);
                goto found;
            } else {
                ++it;
            }
        }

        // if we don't find the key there's an error in the quadtree
        Q_ASSERT(false);
found:
        Q_ASSERT(occupied_quadtree[channel][node].key == 0);
    } else if (occupied_quadtree[channel][node].largest_available_block < current_block_size) {
        Q_ASSERT(current_block_size >= block_size);

        node = node * 4 + 1;

        for (int sibling = 0; sibling < 4; ++sibling)
            quadtreeClear(channel, rect, node + sibling);
    }
}

bool QGLMaskTextureCache::quadtreeFindAvailableLocation(const QSize &size, QRect *rect, int *channel)
{
    int needed_block_size = qMax(1, qMax(size.width(), size.height()));

    for (int i = 0; i < 4; ++i) {
        int current_block_size = offscreenSize.width();

        if (occupied_quadtree[i][0].largest_available_block >= needed_block_size) {
            int node = 0;

            while (current_block_size != occupied_quadtree[i][node].largest_available_block) {
                Q_ASSERT(current_block_size > block_size);
                Q_ASSERT(current_block_size > occupied_quadtree[i][node].largest_available_block);

                node = node * 4 + 1;
                current_block_size /= 2;

                int sibling = 0;

                while (occupied_quadtree[i][node + sibling].largest_available_block < needed_block_size)
                    ++sibling;

                Q_ASSERT(sibling < 4);
                node += sibling;
            }

            *channel = i;
            *rect = QRect(quadtreeLocation(node), size);

            return true;
        }
    }

    return false;
}

void QGLMaskTextureCache::quadtreeFindExistingLocation(const QSize &size, QRect *rect, int *channel)
{
    // try to pick small masks to throw out, as large masks are more expensive to recompute
    *channel = qrand() % 4;
    for (int i = 0; i < 4; ++i)
        if (occupied_quadtree[i][0].largest_used_block < occupied_quadtree[*channel][0].largest_used_block)
            *channel = i;

    int needed_block_size = qt_next_power_of_two(qMax(1, qMax(size.width(), size.height())));

    int node = 0;
    int current_block_size = offscreenSize.width();

    while (current_block_size > block_size
           && current_block_size >= needed_block_size * 2
           && occupied_quadtree[*channel][node].key == 0)
    {
        node = node * 4 + 1;

        int sibling = 0;

        for (int i = 1; i < 4; ++i) {
            if (occupied_quadtree[*channel][node + i].largest_used_block
                <= occupied_quadtree[*channel][node + sibling].largest_used_block)
            {
                sibling = i;
            }
        }

        node += sibling;
        current_block_size /= 2;
    }

    *rect = QRect(quadtreeLocation(node), size);
}

void QGLMaskTextureCache::quadtreeAllocate(quint64 key, const QSize &size, QRect *rect, int *channel)
{
#ifndef DISABLE_MASK_CACHE
    if (!quadtreeFindAvailableLocation(size, rect, channel)) {
        quadtreeFindExistingLocation(size, rect, channel);
        quadtreeClear(*channel, *rect);
    }

    quadtreeInsert(*channel, key, *rect);
#else
    *channel = 0;
    *rect = QRect(QPoint(), size);
#endif
}

class QGLTrapezoidMaskGenerator : public QGLMaskGenerator
{
public:
    QGLTrapezoidMaskGenerator(const QPainterPath &path, const QTransform &matrix, QGLOffscreen &offscreen, GLuint maskFragmentProgram, qreal strokeWidth = -1.0);

    QRect screenRect();
    void drawMask(const QRect &rect);

private:
    QRect screen_rect;
    bool has_screen_rect;

    QGLOffscreen *offscreen;

    GLuint maskFragmentProgram;

    virtual QVector<QGLTrapezoid> generateTrapezoids() = 0;
    virtual QRect computeScreenRect() = 0;
};

class QGLPathMaskGenerator : public QGLTrapezoidMaskGenerator
{
public:
    QGLPathMaskGenerator(const QPainterPath &path, const QTransform &matrix, QGLOffscreen &offscreen, GLuint maskFragmentProgram);

private:
    QVector<QGLTrapezoid> generateTrapezoids();
    QRect computeScreenRect();

    QPolygonF poly;
};

class QGLLineMaskGenerator : public QGLTrapezoidMaskGenerator
{
public:
    QGLLineMaskGenerator(const QPainterPath &path, const QTransform &matrix, qreal width, QGLOffscreen &offscreen, GLuint maskFragmentProgram);

private:
    QVector<QGLTrapezoid> generateTrapezoids();
    QRect computeScreenRect();

    QPainterPath transformedPath;
};

class QGLRectMaskGenerator : public QGLTrapezoidMaskGenerator
{
public:
    QGLRectMaskGenerator(const QPainterPath &path, const QTransform &matrix, QGLOffscreen &offscreen, GLuint maskFragmentProgram);

private:
    QVector<QGLTrapezoid> generateTrapezoids();
    QRect computeScreenRect();

    QPainterPath transformedPath;
};

class QGLEllipseMaskGenerator : public QGLMaskGenerator
{
public:
    QGLEllipseMaskGenerator(const QRectF &rect, const QTransform &matrix, QGLOffscreen &offscreen, GLuint maskFragmentProgram, int *maskVariableLocations);

    QRect screenRect();
    void drawMask(const QRect &rect);

private:
    QRect screen_rect;

    QRectF ellipseRect;

    QGLOffscreen *offscreen;

    GLuint maskFragmentProgram;

    int *maskVariableLocations;

    float vertexArray[4 * 2];
};

QGLTrapezoidMaskGenerator::QGLTrapezoidMaskGenerator(const QPainterPath &path, const QTransform &matrix, QGLOffscreen &offs, GLuint program, qreal stroke_width)
    : QGLMaskGenerator(path, matrix, stroke_width)
    ,  has_screen_rect(false)
    ,  offscreen(&offs)
    ,  maskFragmentProgram(program)
{
}

void QGLTrapezoidMaskGenerator::drawMask(const QRect &rect)
{
#ifdef Q_WS_QWS
    Q_UNUSED(rect);
#else
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    QGLContext *ctx = offscreen->context();
    offscreen->bind();

    glDisable(GL_TEXTURE_GEN_S);
    glDisable(GL_TEXTURE_1D);

    GLfloat vertexArray[4 * 2];
    qt_add_rect_to_array(rect, vertexArray);

    bool needs_scissor = rect != screen_rect;

    if (needs_scissor) {
        glEnable(GL_SCISSOR_TEST);
        glScissor(rect.left(), offscreen->offscreenSize().height() - rect.bottom() - 1, rect.width(), rect.height());
    }

    QVector<QGLTrapezoid> trapezoids = generateTrapezoids();

    // clear mask
    glBlendFunc(GL_ZERO, GL_ZERO); // clear
    glVertexPointer(2, GL_FLOAT, 0, vertexArray);
    glEnableClientState(GL_VERTEX_ARRAY);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glDisableClientState(GL_VERTEX_ARRAY);

    glBlendFunc(GL_ONE, GL_ONE); // add mask
    glEnable(GL_FRAGMENT_PROGRAM_ARB);
    glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, maskFragmentProgram);

    QPoint delta = rect.topLeft() - screen_rect.topLeft();
    glBegin(GL_QUADS);
    for (int i = 0; i < trapezoids.size(); ++i)
        drawTrapezoid(trapezoids[i].translated(delta), offscreen->offscreenSize().height(), ctx);
    glEnd();

    if (needs_scissor)
        glDisable(GL_SCISSOR_TEST);

    glDisable(GL_FRAGMENT_PROGRAM_ARB);

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
#endif
}

QRect QGLTrapezoidMaskGenerator::screenRect()
{
    if (!has_screen_rect) {
        screen_rect = computeScreenRect();
        has_screen_rect = true;
    }

    screen_rect = screen_rect.intersected(QRect(QPoint(), offscreen->drawableSize()));

    return screen_rect;
}

QGLPathMaskGenerator::QGLPathMaskGenerator(const QPainterPath &path, const QTransform &matrix, QGLOffscreen &offs, GLuint program)
    : QGLTrapezoidMaskGenerator(path, matrix, offs, program)
{
}

QRect QGLPathMaskGenerator::computeScreenRect()
{
    poly = path().toFillPolygon(matrix());
    return poly.boundingRect().toAlignedRect();
}

QVector<QGLTrapezoid> QGLPathMaskGenerator::generateTrapezoids()
{
    QOpenGLImmediateModeTessellator tessellator;
    tessellator.tessellate(poly.data(), poly.count(), path().fillRule() == Qt::WindingFill);
    return tessellator.trapezoids;
}

QGLRectMaskGenerator::QGLRectMaskGenerator(const QPainterPath &path, const QTransform &matrix, QGLOffscreen &offs, GLuint program)
    : QGLTrapezoidMaskGenerator(path, matrix, offs, program)
{
}

QGLLineMaskGenerator::QGLLineMaskGenerator(const QPainterPath &path, const QTransform &matrix, qreal width, QGLOffscreen &offs, GLuint program)
    : QGLTrapezoidMaskGenerator(path, matrix, offs, program, width)
{
}

QRect QGLRectMaskGenerator::computeScreenRect()
{
    transformedPath = matrix().map(path());

    return transformedPath.controlPointRect().adjusted(-1, -1, 1, 1).toAlignedRect();
}

QRect QGLLineMaskGenerator::computeScreenRect()
{
    transformedPath = matrix().map(path());

    return transformedPath.controlPointRect().adjusted(-1, -1, 1, 1).toAlignedRect();
}

QVector<QGLTrapezoid> QGLLineMaskGenerator::generateTrapezoids()
{
    QOpenGLImmediateModeTessellator tessellator;
    QPointF last;
    for (int i = 0; i < transformedPath.elementCount(); ++i) {
        QPainterPath::Element element = transformedPath.elementAt(i);

        Q_ASSERT(!element.isCurveTo());

        if (element.isLineTo())
            tessellator.tessellateRect(last, element, strokeWidth());

        last = element;
    }

    return tessellator.trapezoids;
}

QVector<QGLTrapezoid> QGLRectMaskGenerator::generateTrapezoids()
{
    Q_ASSERT(transformedPath.elementCount() == 5);

    QOpenGLImmediateModeTessellator tessellator;
    if (matrix().type() <= QTransform::TxScale) {
        QPointF a = transformedPath.elementAt(0);
        QPointF b = transformedPath.elementAt(1);
        QPointF c = transformedPath.elementAt(2);
        QPointF d = transformedPath.elementAt(3);

        QPointF first = (a + d) * 0.5;
        QPointF last = (b + c) * 0.5;

        QPointF delta = a - d;

        // manhattan distance (no rotation)
        qreal width = qAbs(delta.x()) + qAbs(delta.y());

        Q_ASSERT(qFuzzyCompare(delta.x(), static_cast<qreal>(0))
                 || qFuzzyCompare(delta.y(), static_cast<qreal>(0)));

        tessellator.tessellateRect(first, last, width);
    } else {
        QPointF points[5];

        for (int i = 0; i < 5; ++i)
            points[i] = transformedPath.elementAt(i);

        tessellator.tessellateConvex(points, 5);
    }
    return tessellator.trapezoids;
}

static QPainterPath ellipseRectToPath(const QRectF &rect)
{
    QPainterPath path;
    path.addEllipse(rect);
    return path;
}

QGLEllipseMaskGenerator::QGLEllipseMaskGenerator(const QRectF &rect, const QTransform &matrix, QGLOffscreen &offs, GLuint program, int *locations)
    : QGLMaskGenerator(ellipseRectToPath(rect), matrix),
      ellipseRect(rect),
      offscreen(&offs),
      maskFragmentProgram(program),
      maskVariableLocations(locations)
{
}

QRect QGLEllipseMaskGenerator::screenRect()
{
    QPointF center = ellipseRect.center();

    QPointF points[] = {
        QPointF(ellipseRect.left(), center.y()),
        QPointF(ellipseRect.right(), center.y()),
        QPointF(center.x(), ellipseRect.top()),
        QPointF(center.x(), ellipseRect.bottom())
    };

    qreal min_screen_delta_len = QREAL_MAX;

    for (int i = 0; i < 4; ++i) {
        QPointF delta = points[i] - center;

        // normalize
        delta /= sqrt(delta.x() * delta.x() + delta.y() * delta.y());

        QPointF screen_delta(matrix().m11() * delta.x() + matrix().m21() * delta.y(),
                             matrix().m12() * delta.x() + matrix().m22() * delta.y());

        min_screen_delta_len = qMin(min_screen_delta_len,
                                    qreal(sqrt(screen_delta.x() * screen_delta.x() + screen_delta.y() * screen_delta.y())));
    }

    const qreal padding = 2.0f;

    qreal grow = padding / min_screen_delta_len;

    QRectF boundingRect = ellipseRect.adjusted(-grow, -grow, grow, grow);

    boundingRect = matrix().mapRect(boundingRect);

    QPointF p(0.5, 0.5);

    screen_rect = QRect((boundingRect.topLeft() - p).toPoint(),
                        (boundingRect.bottomRight() + p).toPoint());

    return screen_rect;
}

void QGLEllipseMaskGenerator::drawMask(const QRect &rect)
{
#ifdef Q_WS_QWS
    Q_UNUSED(rect);
#else
    QGLContext *ctx = offscreen->context();
    offscreen->bind();

    glDisable(GL_TEXTURE_GEN_S);
    glDisable(GL_TEXTURE_1D);

    // fragment program needs the inverse radii of the ellipse
    glTexCoord2f(1.0f / (ellipseRect.width() * 0.5f),
                 1.0f / (ellipseRect.height() * 0.5f));

    QTransform translate(1, 0, 0, 1, -ellipseRect.center().x(), -ellipseRect.center().y());
    QTransform gl_to_qt(1, 0, 0, -1, 0, offscreen->drawableSize().height());
    QTransform inv_matrix = gl_to_qt * matrix().inverted() * translate;

    float m[3][4] = { { inv_matrix.m11(), inv_matrix.m12(), inv_matrix.m13() },
                      { inv_matrix.m21(), inv_matrix.m22(), inv_matrix.m23() },
                      { inv_matrix.m31(), inv_matrix.m32(), inv_matrix.m33() } };

    QPoint offs(screen_rect.left() - rect.left(), (offscreen->drawableSize().height() - screen_rect.top())
                                                - (offscreen->offscreenSize().height() - rect.top()));

    // last component needs to be 1.0f to avoid Nvidia bug on linux
    float ellipse_offset[4] = { offs.x(), offs.y(), 0.0f, 1.0f };

    GLfloat vertexArray[4 * 2];
    qt_add_rect_to_array(rect, vertexArray);

    glBlendFunc(GL_ONE, GL_ZERO); // set mask
    glEnable(GL_FRAGMENT_PROGRAM_ARB);
    glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, maskFragmentProgram);

    glProgramLocalParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, maskVariableLocations[VAR_INV_MATRIX_M0], m[0]);
    glProgramLocalParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, maskVariableLocations[VAR_INV_MATRIX_M1], m[1]);
    glProgramLocalParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, maskVariableLocations[VAR_INV_MATRIX_M2], m[2]);

    glProgramLocalParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, maskVariableLocations[VAR_ELLIPSE_OFFSET], ellipse_offset);

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, vertexArray);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glDisableClientState(GL_VERTEX_ARRAY);
#endif
}

void QOpenGLPaintEnginePrivate::drawOffscreenPath(const QPainterPath &path)
{
#ifdef Q_WS_QWS
    Q_UNUSED(path);
#else
    DEBUG_ONCE_STR("QOpenGLPaintEnginePrivate::drawOffscreenPath()");

    if (has_clipping)
        glDisable(GL_DEPTH_TEST);

    QGLPathMaskGenerator maskGenerator(path, matrix, offscreen, mask_fragment_programs[FRAGMENT_PROGRAM_MASK_TRAPEZOID_AA]);

    addItem(qt_mask_texture_cache()->getMask(maskGenerator, this));

    if (has_clipping)
        glEnable(GL_DEPTH_TEST);
#endif
}

void QOpenGLPaintEnginePrivate::drawFastRect(const QRectF &r)
{
    Q_Q(QOpenGLPaintEngine);
    DEBUG_ONCE_STR("QOpenGLPaintEngine::drawRects(): drawing fast rect");

    float vertexArray[10];
    qt_add_rect_to_array(r, vertexArray);

    if (has_brush) {
        flushDrawQueue();

        bool temp = high_quality_antialiasing;
        high_quality_antialiasing = false;

        q->updateCompositionMode(composition_mode);

        setGradientOps(cbrush, r);

        bool fast_style = current_style == Qt::LinearGradientPattern
                          || current_style == Qt::SolidPattern;

        if (fast_style && has_fast_composition_mode) {
            glEnableClientState(GL_VERTEX_ARRAY);
            glVertexPointer(2, GL_FLOAT, 0, vertexArray);
            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
            glDisableClientState(GL_VERTEX_ARRAY);
        } else {
            composite(r);
        }

        high_quality_antialiasing = temp;

        q->updateCompositionMode(composition_mode);
    }

    if (has_pen) {
        if (has_fast_pen && !high_quality_antialiasing) {
            setGradientOps(cpen.brush(), r);

            vertexArray[8] = vertexArray[0];
            vertexArray[9] = vertexArray[1];

            glVertexPointer(2, GL_FLOAT, 0, vertexArray);
            glEnableClientState(GL_VERTEX_ARRAY);
            glDrawArrays(GL_LINE_STRIP, 0, 5);
            glDisableClientState(GL_VERTEX_ARRAY);
        } else {
            QPainterPath path;
            path.setFillRule(Qt::WindingFill);

            qreal left = r.left();
            qreal right = r.right();
            qreal top = r.top();
            qreal bottom = r.bottom();

            path.moveTo(left, top);
            path.lineTo(right, top);
            path.lineTo(right, bottom);
            path.lineTo(left, bottom);
            path.lineTo(left, top);

            strokePath(path, false);
        }
    }
}

bool QOpenGLPaintEnginePrivate::isFastRect(const QRectF &rect)
{
    if (matrix.type() < QTransform::TxRotate) {
        QRectF r = matrix.mapRect(rect);
        return r.topLeft().toPoint() == r.topLeft()
            && r.bottomRight().toPoint() == r.bottomRight();
    }

    return false;
}

void QOpenGLPaintEngine::drawRects(const QRectF *rects, int rectCount)
{
    Q_D(QOpenGLPaintEngine);

    for (int i=0; i<rectCount; ++i) {
        const QRectF &r = rects[i];

        // optimization for rects which can be drawn aliased
        if (!d->high_quality_antialiasing || d->isFastRect(r)) {
            d->drawFastRect(r);
        } else {
            QPainterPath path;
            path.addRect(r);

            if (d->has_brush) {
                if (d->has_clipping)
                    glDisable(GL_DEPTH_TEST);

                QGLRectMaskGenerator maskGenerator(path, d->matrix, d->offscreen, d->mask_fragment_programs[FRAGMENT_PROGRAM_MASK_TRAPEZOID_AA]);

                d->addItem(qt_mask_texture_cache()->getMask(maskGenerator, d));

                if (d->has_clipping)
                    glEnable(GL_DEPTH_TEST);
            }

            if (d->has_pen) {
                if (d->has_fast_pen)
                    d->strokeLines(path);
                else
                    d->strokePath(path, false);
            }
        }
    }
}

void QOpenGLPaintEngine::drawPoints(const QPointF *points, int pointCount)
{
    Q_D(QOpenGLPaintEngine);
    d->setGradientOps(d->cpen.brush(), QRectF());

    if (!d->cpen.isCosmetic() || d->high_quality_antialiasing) {
        QPaintEngine::drawPoints(points, pointCount);
        return;
    }

    d->flushDrawQueue();

    const qreal *vertexArray = reinterpret_cast<const qreal*>(&points[0]);

    if (sizeof(qreal) == sizeof(double)) {
        Q_ASSERT(sizeof(QPointF) == 16);
        glVertexPointer(2, GL_DOUBLE, 0, vertexArray);
    }
    else {
        Q_ASSERT(sizeof(QPointF) == 8);
        glVertexPointer(2, GL_FLOAT, 0, vertexArray);
    }

    glEnableClientState(GL_VERTEX_ARRAY);
    glDrawArrays(GL_POINTS, 0, pointCount);
    glDisableClientState(GL_VERTEX_ARRAY);
}

void QOpenGLPaintEngine::drawLines(const QLineF *lines, int lineCount)
{
    Q_D(QOpenGLPaintEngine);
    if (d->has_pen) {
        if (d->has_fast_pen && !d->high_quality_antialiasing) {
            //### gradient resolving on lines isn't correct
            d->setGradientOps(d->cpen.brush(), QRectF());
            const qreal *vertexArray = reinterpret_cast<const qreal*>(&lines[0]);

            if (sizeof(qreal) == sizeof(double)) {
                Q_ASSERT(sizeof(QLineF) == 32);
                glVertexPointer(2, GL_DOUBLE, 0, vertexArray);
            }
            else {
                Q_ASSERT(sizeof(QLineF) == 16);
                glVertexPointer(2, GL_FLOAT, 0, vertexArray);
            }

            glEnableClientState(GL_VERTEX_ARRAY);
            glDrawArrays(GL_LINES, 0, lineCount*2);
            glDisableClientState(GL_VERTEX_ARRAY);
        } else {
            QPainterPath path;
            path.setFillRule(Qt::WindingFill);
            for (int i=0; i<lineCount; ++i) {
                const QLineF &l = lines[i];

                if (l.p1() == l.p2()) {
                    if (d->cpen.capStyle() != Qt::FlatCap) {
                        QPointF p = l.p1();
                        drawPoints(&p, 1);
                    }
                    continue;
                }

                path.moveTo(l.x1(), l.y1());
                path.lineTo(l.x2(), l.y2());
            }

            if (d->has_fast_pen && d->high_quality_antialiasing)
                d->strokeLines(path);
            else
                d->strokePath(path, false);
        }
    }
}


void QOpenGLPaintEngine::drawPolygon(const QPointF *points, int pointCount, PolygonDrawMode mode)
{
    Q_D(QOpenGLPaintEngine);
    if(pointCount < 2)
        return;

    QRectF bounds;
    if ((mode == ConvexMode && !d->high_quality_antialiasing && state->brushNeedsResolving()) ||
        (d->has_fast_pen && !d->high_quality_antialiasing) && state->penNeedsResolving()) {
        qreal minx = points[0].x(), miny = points[0].y(),
              maxx = points[0].x(), maxy = points[0].y();
        for (int i = 1; i < pointCount; ++i) {
            const QPointF &pt = points[i];
            if (minx > pt.x())
                minx = pt.x();
            if (miny > pt.y())
                miny = pt.y();
            if (maxx < pt.x())
                maxx = pt.x();
            if (maxy < pt.y())
                maxy = pt.y();
        }
        bounds = QRectF(minx, maxx, maxx-minx, maxy-miny);
    }

    if (d->has_brush && mode != PolylineMode) {
        if (mode == ConvexMode && !d->high_quality_antialiasing) {
            //### resolving on polygon from points isn't correct
            d->setGradientOps(d->cbrush, bounds);

            const qreal *vertexArray = reinterpret_cast<const qreal*>(&points[0]);

            if (sizeof(qreal) == sizeof(double)) {
                Q_ASSERT(sizeof(QPointF) == 16);
                glVertexPointer(2, GL_DOUBLE, 0, vertexArray);
            }
            else {
                Q_ASSERT(sizeof(QPointF) == 8);
                glVertexPointer(2, GL_FLOAT, 0, vertexArray);
            }

            glEnableClientState(GL_VERTEX_ARRAY);
            glDrawArrays(GL_TRIANGLE_FAN, 0, pointCount);
            glDisableClientState(GL_VERTEX_ARRAY);
        } else {
            QPainterPath path;
            path.setFillRule(mode == WindingMode ? Qt::WindingFill : Qt::OddEvenFill);
            path.moveTo(points[0]);
            for (int i=1; i<pointCount; ++i)
                path.lineTo(points[i]);
            d->fillPath(path);
        }
    }

    if (d->has_pen) {
        if (d->has_fast_pen && !d->high_quality_antialiasing) {
            d->setGradientOps(d->cpen.brush(), bounds);
            QVarLengthArray<float> vertexArray(pointCount*2 + 2);
            glVertexPointer(2, GL_FLOAT, 0, vertexArray.data());
            int i;
            for (i=0; i<pointCount; ++i) {
                vertexArray[i*2] = points[i].x();
                vertexArray[i*2+1] = points[i].y();
            }

            glEnableClientState(GL_VERTEX_ARRAY);
            if (mode != PolylineMode) {
                vertexArray[i*2] = points[0].x();
                vertexArray[i*2+1] = points[0].y();
                glDrawArrays(GL_LINE_STRIP, 0, pointCount+1);
            } else {
                glDrawArrays(GL_LINE_STRIP, 0, pointCount);
            }
            glDisableClientState(GL_VERTEX_ARRAY);
        } else {
            QPainterPath path(points[0]);
            for (int i = 1; i < pointCount; ++i)
                path.lineTo(points[i]);
            if (mode != PolylineMode)
                path.lineTo(points[0]);

            if (d->has_fast_pen)
                d->strokeLines(path);
            else
                d->strokePath(path, true);
        }
    }
}

void QOpenGLPaintEnginePrivate::strokeLines(const QPainterPath &path)
{
    DEBUG_ONCE_STR("QOpenGLPaintEnginePrivate::strokeLines()");

    qreal penWidth = cpen.widthF();

    QGLLineMaskGenerator maskGenerator(path, matrix, penWidth == 0 ? 1.0 : penWidth,
                                       offscreen, mask_fragment_programs[FRAGMENT_PROGRAM_MASK_TRAPEZOID_AA]);

    if (has_clipping)
        glDisable(GL_DEPTH_TEST);

    QBrush temp = cbrush;
    QPointF origin = brush_origin;

    cbrush = cpen.brush();
    brush_origin = QPointF();

    addItem(qt_mask_texture_cache()->getMask(maskGenerator, this));

    cbrush = temp;
    brush_origin = origin;

    if (has_clipping)
        glEnable(GL_DEPTH_TEST);
}

void QOpenGLPaintEnginePrivate::strokePath(const QPainterPath &path, bool use_cache)
{
    QBrush old_brush = cbrush;
    cbrush = cpen.brush();

    if (cpen.isCosmetic()) {
        QTransform temp = matrix;
        matrix = QTransform();
        glPushMatrix();
        glLoadIdentity();

        if (use_cache)
            fillPath(qt_opengl_stroke_cache()->getStrokedPath(temp.map(path), cpen));
        else
            fillPath(strokeForPath(temp.map(path), cpen));

        glPopMatrix();
        matrix = temp;
    } else if (use_cache) {
        fillPath(qt_opengl_stroke_cache()->getStrokedPath(path, cpen));
    } else {
        fillPath(strokeForPath(path, cpen));
    }

    cbrush = old_brush;
}

void QOpenGLPaintEnginePrivate::strokePathFastPen(const QPainterPath &path, bool needsResolving)
{
#ifndef Q_WS_QWS
    QRectF bounds;
    if (needsResolving)
        bounds = path.controlPointRect();
    setGradientOps(cpen.brush(), bounds);

    QBezier beziers[32];
    for (int i=0; i<path.elementCount(); ++i) {
        const QPainterPath::Element &e = path.elementAt(i);
        switch (e.type) {
        case QPainterPath::MoveToElement:
            if (i != 0)
                glEnd(); // GL_LINE_STRIP
            glBegin(GL_LINE_STRIP);
            glVertex2d(e.x, e.y);

            break;
        case QPainterPath::LineToElement:
            glVertex2d(e.x, e.y);
            break;

        case QPainterPath::CurveToElement:
        {
            QPointF sp = path.elementAt(i-1);
            QPointF cp2 = path.elementAt(i+1);
            QPointF ep = path.elementAt(i+2);
            i+=2;

            qreal inverseScaleHalf = inverseScale / 2;
            beziers[0] = QBezier::fromPoints(sp, e, cp2, ep);
            QBezier *b = beziers;
            while (b >= beziers) {
                // check if we can pop the top bezier curve from the stack
                qreal l = qAbs(b->x4 - b->x1) + qAbs(b->y4 - b->y1);
                qreal d;
                if (l > inverseScale) {
                    d = qAbs( (b->x4 - b->x1)*(b->y1 - b->y2)
                              - (b->y4 - b->y1)*(b->x1 - b->x2) )
                        + qAbs( (b->x4 - b->x1)*(b->y1 - b->y3)
                                - (b->y4 - b->y1)*(b->x1 - b->x3) );
                    d /= l;
                } else {
                    d = qAbs(b->x1 - b->x2) + qAbs(b->y1 - b->y2) +
                        qAbs(b->x1 - b->x3) + qAbs(b->y1 - b->y3);
                }
                if (d < inverseScaleHalf || b == beziers + 31) {
                    // good enough, we pop it off and add the endpoint
                    glVertex2d(b->x4, b->y4);
                    --b;
                } else {
                    // split, second half of the polygon goes lower into the stack
                    b->split(b+1, b);
                    ++b;
                }
            }
        } // case CurveToElement
        default:
            break;
        } // end of switch
    }
    glEnd(); // GL_LINE_STRIP
#else
    // have to use vertex arrays on embedded
    QRectF bounds;
    if (needsResolving)
        bounds = path.controlPointRect();
    setGradientOps(cpen.brush(), bounds);

    glEnableClientState(GL_VERTEX_ARRAY);
    tess_points.reset();
    QBezier beziers[32];
    for (int i=0; i<path.elementCount(); ++i) {
        const QPainterPath::Element &e = path.elementAt(i);
        switch (e.type) {
        case QPainterPath::MoveToElement:
            if (i != 0) {
                glVertexPointer(2, GL_FLOAT, 0, tess_points.data());
                glDrawArrays(GL_LINE_STRIP, 0, tess_points.size());
                tess_points.reset();
            }
            tess_points.add(QPointF(e.x, e.y));

            break;
        case QPainterPath::LineToElement:
            tess_points.add(QPointF(e.x, e.y));
            break;

        case QPainterPath::CurveToElement:
        {
            QPointF sp = path.elementAt(i-1);
            QPointF cp2 = path.elementAt(i+1);
            QPointF ep = path.elementAt(i+2);
            i+=2;

            qreal inverseScaleHalf = inverseScale / 2;
            beziers[0] = QBezier::fromPoints(sp, e, cp2, ep);
            QBezier *b = beziers;
            while (b >= beziers) {
                // check if we can pop the top bezier curve from the stack
                qreal l = qAbs(b->x4 - b->x1) + qAbs(b->y4 - b->y1);
                qreal d;
                if (l > inverseScale) {
                    d = qAbs( (b->x4 - b->x1)*(b->y1 - b->y2)
                              - (b->y4 - b->y1)*(b->x1 - b->x2) )
                        + qAbs( (b->x4 - b->x1)*(b->y1 - b->y3)
                                - (b->y4 - b->y1)*(b->x1 - b->x3) );
                    d /= l;
                } else {
                    d = qAbs(b->x1 - b->x2) + qAbs(b->y1 - b->y2) +
                        qAbs(b->x1 - b->x3) + qAbs(b->y1 - b->y3);
                }
                if (d < inverseScaleHalf || b == beziers + 31) {
                    // good enough, we pop it off and add the endpoint
                    tess_points.add(QPointF(b->x4, b->y4));
                    --b;
                } else {
                    // split, second half of the polygon goes lower into the stack
                    b->split(b+1, b);
                    ++b;
                }
            }
        } // case CurveToElement
        default:
            break;
        } // end of switch
    }
    glVertexPointer(2, GL_FLOAT, 0, tess_points.data());
    glDrawArrays(GL_LINE_STRIP, 0, tess_points.size());
    glDisableClientState(GL_VERTEX_ARRAY);
#endif
}

static bool pathClosed(const QPainterPath &path)
{
    QPointF lastMoveTo = path.elementAt(0);
    QPointF lastPoint = lastMoveTo;

    for (int i = 1; i < path.elementCount(); ++i) {
        const QPainterPath::Element &e = path.elementAt(i);
        switch (e.type) {
        case QPainterPath::MoveToElement:
            if (lastMoveTo != lastPoint)
                return false;
            lastMoveTo = lastPoint = e;
            break;
        case QPainterPath::LineToElement:
            lastPoint = e;
            break;
        case QPainterPath::CurveToElement:
            lastPoint = path.elementAt(i + 2);
            i+=2;
            break;
        default:
            break;
        }
    }

    return lastMoveTo == lastPoint;
}

void QOpenGLPaintEngine::drawPath(const QPainterPath &path)
{
    Q_D(QOpenGLPaintEngine);

    if (path.isEmpty())
        return;

    if (d->has_brush) {
        bool path_closed = pathClosed(path);

        bool has_thick_pen =
            path_closed
            && d->has_pen
            && d->cpen.style() == Qt::SolidLine
            && d->cpen.isSolid()
            && d->cpen.color().alpha() == 255
            && d->txop < QTransform::TxProject
            && d->cpen.widthF() >= 2.0 / sqrt(qMin(d->matrix.m11() * d->matrix.m11()
                                                   + d->matrix.m21() * d->matrix.m21(),
                                                   d->matrix.m12() * d->matrix.m12()
                                                   + d->matrix.m22() * d->matrix.m22()));

        if (has_thick_pen) {
            DEBUG_ONCE qDebug() << "QOpenGLPaintEngine::drawPath(): Using thick pen optimization, style:" << d->cbrush.style();

            d->flushDrawQueue();

            bool temp = d->high_quality_antialiasing;
            d->high_quality_antialiasing = false;

            updateCompositionMode(d->composition_mode);

            d->fillPath(path);

            d->high_quality_antialiasing = temp;
            updateCompositionMode(d->composition_mode);
        } else {
            d->fillPath(path);
        }
    }

    if (d->has_pen) {
        if (d->has_fast_pen && !d->high_quality_antialiasing)
            d->strokePathFastPen(path, state->penNeedsResolving());
        else
            d->strokePath(path, true);
    }
}

void QOpenGLPaintEnginePrivate::drawImageAsPath(const QRectF &r, const QImage &img, const QRectF &sr)
{
    QBrush old_brush = cbrush;
    QPointF old_brush_origin = brush_origin;

    qreal scaleX = r.width() / sr.width();
    qreal scaleY = r.height() / sr.height();

    QTransform brush_matrix;
    brush_matrix.translate(r.left(), r.top());
    brush_matrix.scale(scaleX, scaleY);
    brush_matrix.translate(-sr.left(), -sr.top());

    cbrush = QBrush(img);
    cbrush.setTransform(brush_matrix);
    brush_origin = QPointF();

    QPainterPath p;
    p.addRect(r);
    fillPath(p);

    cbrush = old_brush;
    brush_origin = old_brush_origin;
}

void QOpenGLPaintEnginePrivate::drawTiledImageAsPath(const QRectF &r, const QImage &img)
{
    QBrush old_brush = cbrush;
    QPointF old_brush_origin = brush_origin;

    QTransform brush_matrix;
    brush_matrix.translate(r.left(), r.top());

    cbrush = QBrush(img);
    cbrush.setTransform(brush_matrix);
    brush_origin = QPointF();

    QPainterPath p;
    p.addRect(r);
    fillPath(p);

    cbrush = old_brush;
    brush_origin = old_brush_origin;
}

void QOpenGLPaintEngine::drawPixmap(const QRectF &r, const QPixmap &pm, const QRectF &sr)
{
    Q_D(QOpenGLPaintEngine);
    if (pm.depth() == 1) {
        QPixmap tpx(pm.size());
        tpx.fill(Qt::transparent);
        QPainter p(&tpx);
        p.setPen(d->cpen);
        p.drawPixmap(0, 0, pm);
        p.end();
        drawPixmap(r, tpx, sr);
        return;
    }
    if (d->composition_mode > QPainter::CompositionMode_Plus || d->high_quality_antialiasing && !d->isFastRect(r))
        d->drawImageAsPath(r, pm.toImage(), sr);
    else {
        GLenum target = (QGLExtensions::glExtensions & QGLExtensions::TextureRectangle)
                        ? GL_TEXTURE_RECTANGLE_NV
                        : GL_TEXTURE_2D;
        if (r.size() != pm.size())
            target = GL_TEXTURE_2D;
        d->flushDrawQueue();
        d->drawable.bindTexture(pm, target);

        drawTextureRect(pm.width(), pm.height(), r, sr, target);
    }
}

void QOpenGLPaintEngine::drawTiledPixmap(const QRectF &r, const QPixmap &pm, const QPointF &)
{
    Q_D(QOpenGLPaintEngine);
    if (d->composition_mode > QPainter::CompositionMode_Plus || d->high_quality_antialiasing && !d->isFastRect(r))
        d->drawTiledImageAsPath(r, pm.toImage());
    else {
        d->flushDrawQueue();
        d->drawable.bindTexture(pm);

        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

#ifndef Q_WS_QWS
        glPushAttrib(GL_CURRENT_BIT);
#endif
        glColor4f(d->opacity, d->opacity, d->opacity, d->opacity);
        glEnable(GL_TEXTURE_2D);

        GLdouble tc_w = r.width()/pm.width();
        GLdouble tc_h = r.height()/pm.height();

        // Rotate the texture so that it is aligned correctly and the
        // wrapping is done correctly
        glMatrixMode(GL_TEXTURE);
        glPushMatrix();
        glRotatef(180.0, 0.0, 1.0, 0.0);
        glRotatef(180.0, 0.0, 0.0, 1.0);

        float vertexArray[4*2];
        float texCoordArray[4*2];

        qt_add_rect_to_array(r, vertexArray);
        qt_add_texcoords_to_array(0, 0, tc_w, tc_h, texCoordArray);

        glVertexPointer(2, GL_FLOAT, 0, vertexArray);
        glTexCoordPointer(2, GL_FLOAT, 0, texCoordArray);

        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        glDisableClientState(GL_VERTEX_ARRAY);
        glPopMatrix();

        glDisable(GL_TEXTURE_2D);
#ifndef Q_WS_QWS
        glPopAttrib();
#endif
    }
}

void QOpenGLPaintEngine::drawImage(const QRectF &r, const QImage &image, const QRectF &sr,
                                   Qt::ImageConversionFlags)
{
    Q_D(QOpenGLPaintEngine);
    if (d->composition_mode > QPainter::CompositionMode_Plus || d->high_quality_antialiasing && !d->isFastRect(r))
        d->drawImageAsPath(r, image, sr);
    else {
        GLenum target = (QGLExtensions::glExtensions & QGLExtensions::TextureRectangle)
                        ? GL_TEXTURE_RECTANGLE_NV
                        : GL_TEXTURE_2D;
        if (r.size() != image.size())
            target = GL_TEXTURE_2D;
        d->flushDrawQueue();
        d->drawable.bindTexture(image, target);
        drawTextureRect(image.width(), image.height(), r, sr, target);
    }
}

void QOpenGLPaintEngine::drawTextureRect(int tx_width, int tx_height, const QRectF &r,
                                         const QRectF &sr, GLenum target)
{
    Q_D(QOpenGLPaintEngine);
#ifndef Q_WS_QWS
    glPushAttrib(GL_CURRENT_BIT);
#endif
    glColor4f(d->opacity, d->opacity, d->opacity, d->opacity);
    glEnable(target);

    qreal x1, x2, y1, y2;
    if (target == GL_TEXTURE_2D) {
        x1 = sr.x() / tx_width;
        x2 = x1 + sr.width() / tx_width;
        y1 = 1.0 - ((sr.y() / tx_height) + (sr.height() / tx_height));
        y2 = 1.0 - (sr.y() / tx_height);
    } else {
        x1 = sr.x();
        x2 = sr.width();
        y1 = sr.y();
        y2 = sr.height();
    }

    float vertexArray[4*2];
    float texCoordArray[4*2];

    qt_add_rect_to_array(r, vertexArray);
    qt_add_texcoords_to_array(x1, y2, x2, y1, texCoordArray);

    glVertexPointer(2, GL_FLOAT, 0, vertexArray);
    glTexCoordPointer(2, GL_FLOAT, 0, texCoordArray);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);

    glDisable(target);
#ifndef Q_WS_QWS
    glPopAttrib();
#endif
}

#ifdef Q_WS_WIN
HDC
#else
Qt::HANDLE
#endif
QOpenGLPaintEngine::handle() const
{
    return 0;
}

static const int x_margin = 1;
static const int y_margin = 0;

struct QGLGlyphCoord {
    // stores the offset and size of a glyph texture
    qreal x;
    qreal y;
    qreal width;
    qreal height;
    qreal log_width;
    qreal log_height;
    QFixed x_offset;
    QFixed y_offset;
};

struct QGLFontTexture {
    int x_offset; // glyph offset within the
    int y_offset;
    GLuint texture;
    int width;
    int height;
};

typedef QHash<glyph_t, QGLGlyphCoord*>  QGLGlyphHash;
typedef QHash<QFontEngine*, QGLGlyphHash*> QGLFontGlyphHash;
typedef QHash<quint64, QGLFontTexture*> QGLFontTexHash;
typedef QHash<QGLContext*, QGLFontGlyphHash*> QGLContextHash;

class QGLGlyphCache : public QObject
{
    Q_OBJECT
public:
    QGLGlyphCache() : QObject(0) { current_cache = 0; }
    ~QGLGlyphCache();
    QGLGlyphCoord *lookup(QFontEngine *, glyph_t);
    void cacheGlyphs(QGLContext *, const QTextItemInt &, const QVarLengthArray<glyph_t> &);
    void cleanCache();
    void allocTexture(int width, int height, GLuint texture);
    void cleanupContext(QGLContext *);

public slots:
    void fontEngineDestroyed(QObject *);
    void widgetDestroyed(QObject *);

protected:
    QGLGlyphHash *current_cache;
    QGLFontTexHash qt_font_textures;
    QGLContextHash qt_context_cache;
};

QGLGlyphCache::~QGLGlyphCache()
{
//     qDebug() << "cleaning out the QGLGlyphCache";
    cleanCache();
}

void QGLGlyphCache::fontEngineDestroyed(QObject *o)
{
//     qDebug() << "fontEngineDestroyed()";
    QFontEngine *fe = static_cast<QFontEngine *>(o); // safe, since only the type is used
    QList<QGLContext *> keys = qt_context_cache.keys();
    QGLContext *ctx = 0;

    for (int i=0; i < keys.size(); ++i) {
        QGLFontGlyphHash *font_cache = qt_context_cache.value(keys.at(i));
        if (font_cache->find(fe) != font_cache->end()) {
            ctx = keys.at(i);
            QGLGlyphHash *cache = font_cache->take(fe);
            delete cache;
            break;
        }
    }

    quint64 font_key = (reinterpret_cast<quint64>(ctx) << 32) | reinterpret_cast<quint64>(fe);
    QGLFontTexture *tex = qt_font_textures.take(font_key);
    if (tex) {
#ifdef Q_WS_MAC
        if (aglGetCurrentContext() == 0)
#endif
            glDeleteTextures(1, &tex->texture);
        delete tex;
    }
}

void QGLGlyphCache::widgetDestroyed(QObject *)
{
//     qDebug() << "widget destroyed";
    cleanCache(); // ###
}

void QGLGlyphCache::cleanupContext(QGLContext *ctx)
{
//     qDebug() << "==> cleaning for: " << hex << ctx;
    QGLFontGlyphHash *font_cache = qt_context_cache.take(ctx);

    if (font_cache) {
        QList<QFontEngine *> keys = font_cache->keys();
        for (int i=0; i < keys.size(); ++i) {
            QFontEngine *fe = keys.at(i);
            delete font_cache->take(fe);
            quint64 font_key = (reinterpret_cast<quint64>(ctx) << 32) | reinterpret_cast<quint64>(fe);
            QGLFontTexture *font_tex = qt_font_textures.take(font_key);
            if (font_tex) {
#ifdef Q_WS_MAC
                if (aglGetCurrentContext() != 0)
#endif
                    glDeleteTextures(1, &font_tex->texture);
                delete font_tex;
            }
        }
        delete font_cache;
    }
//    qDebug() << "<=== done cleaning, num tex:" << qt_font_textures.size() << "num ctx:" << qt_context_cache.size();
}

void QGLGlyphCache::cleanCache()
{
    QGLFontTexHash::const_iterator it = qt_font_textures.constBegin();
    while (it != qt_font_textures.constEnd()) {
#ifdef Q_WS_MAC
        if (aglGetCurrentContext() == 0)
            break;
 #endif
        glDeleteTextures(1, &it.value()->texture);
        ++it;
    }
    qDeleteAll(qt_font_textures);
    qt_font_textures.clear();

    QList<QGLContext *> keys = qt_context_cache.keys();
    for (int i=0; i < keys.size(); ++i) {
        QGLFontGlyphHash *font_cache = qt_context_cache.value(keys.at(i));
        qDeleteAll(*font_cache);
        font_cache->clear();
    }
    qDeleteAll(qt_context_cache);
    qt_context_cache.clear();
}

void QGLGlyphCache::allocTexture(int width, int height, GLuint texture)
{
    uchar *tex_data = (uchar *) malloc(width*height*2);
    memset(tex_data, 0, width*height*2);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
#ifndef Q_WS_QWS
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE8_ALPHA8,
                 width, height, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, tex_data);
#else
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA,
                 width, height, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, tex_data);
#endif
    free(tex_data);
}

#if 0
// useful for debugging the glyph cache
static QImage getCurrentTexture(const QColor &color, QGLFontTexture *font_tex)
{
    ushort *old_tex_data = (ushort *) malloc(font_tex->width*font_tex->height*2);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, old_tex_data);
    QImage im(font_tex->width, font_tex->height, QImage::Format_ARGB32);
    for (int y=0; y<font_tex->height; ++y) {
        for (int x=0; x<font_tex->width; ++x) {
            im.setPixel(x, y, ((*(old_tex_data+x+y*font_tex->width)) << 24) | (0x00ffffff & color.rgb()));
        }
    }
    delete old_tex_data;
    return im;
}
#endif

void QGLGlyphCache::cacheGlyphs(QGLContext *context, const QTextItemInt &ti,
                                const QVarLengthArray<glyph_t> &glyphs)
{
    QGLContextHash::const_iterator dev_it = qt_context_cache.constFind(context);
    QGLFontGlyphHash *font_cache = 0;
    QGLContext *context_key = 0;

    if (dev_it == qt_context_cache.constEnd()) {
        // check for shared contexts
        QList<QGLContext *> contexts = qt_context_cache.keys();
        for (int i=0; i<contexts.size(); ++i) {
            QGLContext *ctx = contexts.at(i);
            if (ctx != context && qgl_share_reg()->checkSharing(context, ctx)) {
                context_key = ctx;
                dev_it = qt_context_cache.constFind(context_key);
                break;
            }
        }
    }

    if (dev_it == qt_context_cache.constEnd()) {
        // no shared contexts either - create a new entry
        font_cache = new QGLFontGlyphHash;
//         qDebug() << "new context" << context << font_cache;
        qt_context_cache.insert(context, font_cache);
        if (context->isValid() && context->device()->devType() == QInternal::Widget) {
            QWidget *widget = static_cast<QWidget *>(context->device());
            connect(widget, SIGNAL(destroyed(QObject*)), SLOT(widgetDestroyed(QObject*)));
        }
    } else {
        font_cache = dev_it.value();
    }
    Q_ASSERT(font_cache != 0);

    QGLFontGlyphHash::const_iterator cache_it = font_cache->constFind(ti.fontEngine);
    QGLGlyphHash *cache = 0;
    if (cache_it == font_cache->constEnd()) {
        cache = new QGLGlyphHash;
        font_cache->insert(ti.fontEngine, cache);
        connect(ti.fontEngine, SIGNAL(destroyed(QObject*)), SLOT(fontEngineDestroyed(QObject*)));
    } else {
        cache = cache_it.value();
    }
    current_cache = cache;

    quint64 font_key = (reinterpret_cast<quint64>(context_key ? context_key : context) << 32)
                       | reinterpret_cast<quint64>(ti.fontEngine);
    QGLFontTexHash::const_iterator it = qt_font_textures.constFind(font_key);
    QGLFontTexture *font_tex;
    if (it == qt_font_textures.constEnd()) {
        GLuint font_texture;
        glGenTextures(1, &font_texture);
        GLint tex_height = qt_next_power_of_two(qRound(ti.ascent.toReal() + ti.descent.toReal())+2);
        GLint tex_width = qt_next_power_of_two(tex_height*30); // ###
        GLint max_tex_size;
        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_tex_size);
        if (tex_width > max_tex_size)
            tex_width = max_tex_size;
        allocTexture(tex_width, tex_height, font_texture);
        font_tex = new QGLFontTexture;
        font_tex->texture = font_texture;
        font_tex->x_offset = x_margin;
        font_tex->y_offset = y_margin;
        font_tex->width = tex_width;
        font_tex->height = tex_height;
//         qDebug() << "new font tex - width:" << tex_width << "height:"<< tex_height
//                  << hex << "tex id:" << font_tex->texture << "key:" << font_key << "num cached:" << qt_font_textures.size();
        qt_font_textures.insert(font_key, font_tex);
    } else {
        font_tex = it.value();
        glBindTexture(GL_TEXTURE_2D, font_tex->texture);
    }

    for (int i=0; i< glyphs.size(); ++i) {
        QGLGlyphHash::const_iterator it = cache->constFind(glyphs[i]);
        if (it == cache->constEnd()) {
            // render new glyph and put it in the cache
            glyph_metrics_t metrics = ti.fontEngine->boundingBox(glyphs[i]);
            int glyph_width = qRound(metrics.width.toReal())+2;
            int glyph_height = qRound(ti.ascent.toReal() + ti.descent.toReal())+2;

            if (font_tex->x_offset + glyph_width + x_margin > font_tex->width) {
                int strip_height = qt_next_power_of_two(qRound(ti.ascent.toReal() + ti.descent.toReal())+2);
                font_tex->x_offset = x_margin;
                font_tex->y_offset += strip_height;
                if (font_tex->y_offset >= font_tex->height) {
                    // get hold of the old font texture
                    uchar *old_tex_data = (uchar *) malloc(font_tex->width*font_tex->height*2);
                    int old_tex_height = font_tex->height;
#ifndef Q_WS_QWS
                    glGetTexImage(GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, old_tex_data);
#endif

                    // realloc a larger texture
                    glDeleteTextures(1, &font_tex->texture);
                    glGenTextures(1, &font_tex->texture);
                    font_tex->height = qt_next_power_of_two(font_tex->height + strip_height);
                    allocTexture(font_tex->width, font_tex->height, font_tex->texture);

                    // write back the old texture data
                    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, font_tex->width, old_tex_height,
                                    GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, old_tex_data);
                    free(old_tex_data);

                    // update the texture coords and the y offset for the existing glyphs in
                    // the cache, because of the texture size change
                    QGLGlyphHash::iterator it = cache->begin();
                    while (it != cache->end()) {
                        it.value()->height = (it.value()->height * old_tex_height) / font_tex->height;
                        it.value()->y = (it.value()->y * old_tex_height) / font_tex->height;
                        ++it;
                    }
                }
            }

            QImage glyph_im(ti.fontEngine->alphaMapForGlyph(glyphs[i]).convertToFormat(QImage::Format_Indexed8));
            glyph_width = glyph_im.width();
            // pad the glyph width to an even number
            if (glyph_width%2 != 0)
                ++glyph_width;

            QGLGlyphCoord *qgl_glyph = new QGLGlyphCoord;
            qgl_glyph->x = qreal(font_tex->x_offset) / font_tex->width;
            qgl_glyph->y = qreal(font_tex->y_offset) / font_tex->height;
            qgl_glyph->width = qreal(glyph_width) / font_tex->width;
            qgl_glyph->height = qreal(glyph_height) / font_tex->height;
            qgl_glyph->log_width = qreal(glyph_width);
            qgl_glyph->log_height = qgl_glyph->height * font_tex->height;
            qgl_glyph->x_offset = -metrics.x;
            qgl_glyph->y_offset = metrics.y;

            if (!glyph_im.isNull()) {

                int idx = 0;
                uchar *tex_data = (uchar *) malloc(glyph_width*glyph_im.height()*2);
                memset(tex_data, 0, glyph_width*glyph_im.height()*2);

                for (int y=0; y<glyph_im.height(); ++y) {
                    uchar *s = (uchar *) glyph_im.scanLine(y);
                    for (int x=0; x<glyph_im.width(); ++x) {
                        tex_data[idx] = *s;
                        tex_data[idx+1] = *s;
                        ++s;
                        idx += 2;
                    }
                    if (glyph_im.width()%2 != 0)
                        idx += 2;
                }
                glTexSubImage2D(GL_TEXTURE_2D, 0, font_tex->x_offset, font_tex->y_offset,
                                glyph_width, glyph_im.height(),
                                GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, tex_data);
                free(tex_data);
            }
            if (font_tex->x_offset + glyph_width + x_margin > font_tex->width) {
                font_tex->x_offset = x_margin;
                font_tex->y_offset += glyph_height + y_margin;
            } else {
                font_tex->x_offset += glyph_width + x_margin;
            }

            cache->insert(glyphs[i], qgl_glyph);
        }
    }
}

QGLGlyphCoord *QGLGlyphCache::lookup(QFontEngine *, glyph_t g)
{
    Q_ASSERT(current_cache != 0);
    // ### careful here
    QGLGlyphHash::const_iterator it = current_cache->constFind(g);
    if (it == current_cache->constEnd())
        return 0;
    else
        return it.value();
}

Q_GLOBAL_STATIC(QGLGlyphCache, qt_glyph_cache)

//
// assumption: the context that this is called for has to be the
// current context
//
void qgl_cleanup_glyph_cache(QGLContext *ctx)
{
    qt_glyph_cache()->cleanupContext(ctx);
}

void QOpenGLPaintEngine::drawTextItem(const QPointF &p, const QTextItem &textItem)
{
    Q_D(QOpenGLPaintEngine);

    // fall back to drawing a polygon if the scale factor is large, or
    // we use a gradient pen
    if ((d->matrix.det() > 1) || (d->pen_brush_style >= Qt::LinearGradientPattern
                                  && d->pen_brush_style <= Qt::ConicalGradientPattern)) {
        QPaintEngine::drawTextItem(p, textItem);
        return;
    }

    d->flushDrawQueue();

    // add the glyphs used to the glyph texture cache
    const QTextItemInt &ti = static_cast<const QTextItemInt &>(textItem);
    QVarLengthArray<QFixedPoint> positions;
    QVarLengthArray<glyph_t> glyphs;
    QTransform matrix;
    matrix.translate(qRound(p.x()), qRound(p.y()));
    ti.fontEngine->getGlyphPositions(ti.glyphs, ti.num_glyphs, matrix, ti.flags, glyphs, positions);

    // make sure the glyphs we want to draw are in the cache
    qt_glyph_cache()->cacheGlyphs(d->drawable.context(), ti, glyphs);

    d->setGradientOps(Qt::SolidPattern, QRectF()); // turns off gradient ops
    qt_glColor4ubv(d->pen_color);
    glEnable(GL_TEXTURE_2D);

#ifdef Q_WS_QWS
    // XXX: it is necessary to disable alpha writes on GLES/embedded because we don't want
    // text rendering to update the alpha in the window surface.
    // XXX: This may not be needed as this behavior does seem to be caused by driver bug
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);
#endif

    // do the actual drawing
    float vertexArray[4*2];
    float texCoordArray[4*2];

    glVertexPointer(2, GL_FLOAT, 0, vertexArray);
    glTexCoordPointer(2, GL_FLOAT, 0, texCoordArray);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    for (int i=0; i< glyphs.size(); ++i) {
        QGLGlyphCoord *g = qt_glyph_cache()->lookup(ti.fontEngine, glyphs[i]);

        // we don't cache glyphs with no width/height
        if (!g)
            continue;

        qreal x1, x2, y1, y2;
        x1 = g->x;
        y1 = g->y;
        x2 = x1 + g->width;
        y2 = y1 + g->height;

        QPointF logical_pos((positions[i].x - g->x_offset).toReal(),
                            (positions[i].y + g->y_offset).toReal());

        qt_add_rect_to_array(QRectF(logical_pos, QSizeF(g->log_width, g->log_height)), vertexArray);
        qt_add_texcoords_to_array(x1, y1, x2, y2, texCoordArray);

        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }

    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);

    glDisable(GL_TEXTURE_2D);

#ifdef Q_WS_QWS
    // XXX: This may not be needed as this behavior does seem to be caused by driver bug
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
#endif
}


void QOpenGLPaintEngine::drawEllipse(const QRectF &rect)
{
#ifndef Q_WS_QWS
    Q_D(QOpenGLPaintEngine);

    if (d->high_quality_antialiasing) {
        if (d->has_brush) {
            if (d->has_clipping)
                glDisable(GL_DEPTH_TEST);

            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();
            glLoadIdentity();

            QGLEllipseMaskGenerator maskGenerator(rect,
                                                  d->matrix,
                                                  d->offscreen,
                                                  d->mask_fragment_programs[FRAGMENT_PROGRAM_MASK_ELLIPSE_AA],
                                                  mask_variable_locations[FRAGMENT_PROGRAM_MASK_ELLIPSE_AA]);

            d->addItem(qt_mask_texture_cache()->getMask(maskGenerator, d));

            if (d->has_clipping)
                glEnable(GL_DEPTH_TEST);

            glMatrixMode(GL_MODELVIEW);
            glPopMatrix();
        }

        if (d->has_pen) {
            QPainterPath path;
            path.addEllipse(rect);

            d->strokePath(path, false);
        }
    } else {
        DEBUG_ONCE_STR("QOpenGLPaintEngine::drawEllipse(): falling back to drawPath()");

        QPainterPath path;
        path.addEllipse(rect);
        drawPath(path);
    }
#else
    QPaintEngine::drawEllipse(rect);
#endif
}


void QOpenGLPaintEnginePrivate::updateFragmentProgramData(int locations[])
{
#ifdef Q_WS_QWS
    Q_UNUSED(locations);
#else
    QGL_FUNC_CONTEXT;

    QSize sz = offscreen.offscreenSize();

    float inv_mask_size_data[4] = { 1.0f / sz.width(), 1.0f / sz.height(), 0.0f, 0.0f };

    sz = drawable_texture_size;

    float inv_dst_size_data[4] = { 1.0f / sz.width(), 1.0f / sz.height(), 0.0f, 0.0f };

    // default inv size 0.125f == 1.0f / 8.0f for pattern brushes
    float inv_brush_texture_size_data[4] = { 0.125f, 0.125f };

    // texture patterns have their own size
    if (current_style == Qt::TexturePattern) {
        QSize sz = cbrush.texture().size();

        inv_brush_texture_size_data[0] = 1.0f / sz.width();
        inv_brush_texture_size_data[1] = 1.0f / sz.height();
    }

    for (unsigned int i = 0; i < num_fragment_variables; ++i) {
        int location = locations[i];

        if (location < 0)
            continue;

        switch (i) {
        case VAR_ANGLE:
            glProgramLocalParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, location, angle_data);
            break;
        case VAR_LINEAR:
            glProgramLocalParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, location, linear_data);
            break;
        case VAR_FMP:
            glProgramLocalParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, location, fmp_data);
            break;
        case VAR_FMP2_M_RADIUS2:
            glProgramLocalParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, location, fmp2_m_radius2_data);
            break;
        case VAR_INV_MASK_SIZE:
            glProgramLocalParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, location, inv_mask_size_data);
            break;
        case VAR_INV_DST_SIZE:
            glProgramLocalParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, location, inv_dst_size_data);
            break;
        case VAR_INV_MATRIX_M0:
            glProgramLocalParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, location, inv_matrix_data[0]);
            break;
        case VAR_INV_MATRIX_M1:
            glProgramLocalParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, location, inv_matrix_data[1]);
            break;
        case VAR_INV_MATRIX_M2:
            glProgramLocalParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, location, inv_matrix_data[2]);
            break;
        case VAR_PORTERDUFF_AB:
            glProgramLocalParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, location, porterduff_ab_data);
            break;
        case VAR_PORTERDUFF_XYZ:
            glProgramLocalParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, location, porterduff_xyz_data);
            break;
        case VAR_INV_BRUSH_TEXTURE_SIZE:
            glProgramLocalParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, location, inv_brush_texture_size_data);
            break;
        case VAR_MASK_OFFSET:
            glProgramLocalParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, location, mask_offset_data);
            break;
        case VAR_MASK_CHANNEL:
            glProgramLocalParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, location, mask_channel_data);
            break;
        case VAR_DST_TEXTURE:
        case VAR_MASK_TEXTURE:
        case VAR_PALETTE:
        case VAR_BRUSH_TEXTURE:
            // texture variables, not handled here
            break;
        default:
            qDebug() << "QOpenGLPaintEnginePrivate: Unhandled fragment variable:" << i;
        }
    }
#endif
}


void QOpenGLPaintEnginePrivate::copyDrawable(const QRectF &rect)
{
#ifdef Q_WS_QWS
    Q_UNUSED(rect);
#else
    DEBUG_ONCE qDebug() << "Refreshing drawable_texture for rectangle" << rect;
    QRectF screen_rect = rect.adjusted(-1, -1, 1, 1);

    int left = qMax(0, static_cast<int>(screen_rect.left()));
    int width = qMin(drawable.size().width() - left, static_cast<int>(screen_rect.width()) + 1);

    int bottom = qMax(0, static_cast<int>(drawable.size().height() - screen_rect.bottom()));
    int height = qMin(drawable.size().height() - bottom, static_cast<int>(screen_rect.height()) + 1);

    glBindTexture(GL_TEXTURE_2D, drawable_texture);
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, left, bottom, left, bottom, width, height);
#endif
}


void QOpenGLPaintEnginePrivate::composite(const QRectF &rect, const QPoint &maskOffset)
{
#ifdef Q_WS_QWS
    Q_UNUSED(rect);
    Q_UNUSED(maskOffset);
#else
    float vertexArray[8];
    qt_add_rect_to_array(rect, vertexArray);

    composite(GL_TRIANGLE_FAN, vertexArray, 4, maskOffset);
#endif
}


void QOpenGLPaintEnginePrivate::composite(GLuint primitive, const float *vertexArray, int vertexCount, const QPoint &maskOffset)
{
#ifdef Q_WS_QWS
    Q_UNUSED(primitive);
    Q_UNUSED(vertexArray);
    Q_UNUSED(vertexCount);
    Q_UNUSED(maskOffset);
#else
    Q_Q(QOpenGLPaintEngine);
    QGL_FUNC_CONTEXT;

    DEBUG_ONCE qDebug() << "QOpenGLPaintEnginePrivate: Using compositing program: fragment_brush ="
                        << fragment_brush << ", fragment_composition_mode =" << fragment_composition_mode;

    if (has_fast_composition_mode)
        q->updateCompositionMode(composition_mode);
    else {
        qreal minX = 1e9, minY = 1e9, maxX = -1e9, maxY = -1e9;

        for (int i = 0; i < vertexCount; ++i) {
            qreal x = vertexArray[2 * i];
            qreal y = vertexArray[2 * i + 1];

            qreal tx, ty;
            matrix.map(x, y, &tx, &ty);

            minX = qMin(minX, tx);
            minY = qMin(minY, ty);
            maxX = qMax(maxX, tx);
            maxY = qMax(maxY, ty);
        }

        QRectF r(minX, minY, maxX - minX, maxY - minY);
        copyDrawable(r);

        glBlendFunc(GL_ONE, GL_ZERO);
    }

    int *locations = painter_variable_locations[fragment_brush][fragment_composition_mode];

    int texture_locations[] = { locations[VAR_DST_TEXTURE],
                                locations[VAR_MASK_TEXTURE],
                                locations[VAR_PALETTE] };

    int brush_texture_location = locations[VAR_BRUSH_TEXTURE];

    GLuint texture_targets[] = { GL_TEXTURE_2D,
                                 GL_TEXTURE_2D,
                                 GL_TEXTURE_1D };

    GLuint textures[] = { drawable_texture,
                          offscreen.offscreenTexture(),
                          grad_palette };

    const int num_textures = sizeof(textures) / sizeof(*textures);

    Q_ASSERT(num_textures == sizeof(texture_locations) / sizeof(*texture_locations));
    Q_ASSERT(num_textures == sizeof(texture_targets) / sizeof(*texture_targets));

    for (int i = 0; i < num_textures; ++i)
        if (texture_locations[i] >= 0) {
            glActiveTexture(GL_TEXTURE0 + texture_locations[i]);
            glBindTexture(texture_targets[i], textures[i]);
        }

    if (brush_texture_location >= 0) {
        glActiveTexture(GL_TEXTURE0 + brush_texture_location);

        if (current_style == Qt::TexturePattern) {
            drawable.bindTexture(cbrush.textureImage());
        } else {
            pattern_image = qt_imageForBrush(current_style, true);
            drawable.bindTexture(pattern_image);
        }

        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, vertexArray);
    glEnable(GL_FRAGMENT_PROGRAM_ARB);
    glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, painter_fragment_programs[fragment_brush][fragment_composition_mode]);

    mask_offset_data[0] = maskOffset.x();
    mask_offset_data[1] = -maskOffset.y();

    updateFragmentProgramData(locations);

    glDrawArrays(primitive, 0, vertexCount);

    glDisable(GL_FRAGMENT_PROGRAM_ARB);
    glDisableClientState(GL_VERTEX_ARRAY);

    for (int i = 0; i < num_textures; ++i)
        if (texture_locations[i] >= 0) {
            glActiveTexture(GL_TEXTURE0 + texture_locations[i]);
            glBindTexture(texture_targets[i], 0);
        }

    if (brush_texture_location >= 0) {
        glActiveTexture(GL_TEXTURE0 + brush_texture_location);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    glActiveTexture(GL_TEXTURE0);

    if (!has_fast_composition_mode)
        q->updateCompositionMode(composition_mode);
#endif
}

void QOpenGLPaintEnginePrivate::cacheItemErased(int channel, const QRect &rect)
{
    bool isInDrawQueue = false;

    foreach (const QDrawQueueItem &item, drawQueue) {
        if (item.location.channel == channel && item.location.rect == rect) {
            isInDrawQueue = true;
            break;
        }
    }

    if (isInDrawQueue)
        flushDrawQueue();
}

void QOpenGLPaintEnginePrivate::addItem(const QGLMaskTextureCache::CacheLocation &location)
{
    drawQueue << QDrawQueueItem(opacity, cbrush, brush_origin, composition_mode, matrix, location);
}

void QOpenGLPaintEnginePrivate::drawItem(const QDrawQueueItem &item)
{
    Q_Q(QOpenGLPaintEngine);

    opacity = item.opacity;
    brush_origin = item.brush_origin;
    q->updateCompositionMode(item.composition_mode);
    matrix = item.matrix;
    cbrush = item.brush;
    brush_style = item.brush.style();

    mask_channel_data[0] = item.location.channel == 0;
    mask_channel_data[1] = item.location.channel == 1;
    mask_channel_data[2] = item.location.channel == 2;
    mask_channel_data[3] = item.location.channel == 3;

    setGradientOps(item.brush, item.location.rect);

    composite(item.location.screen_rect, item.location.rect.topLeft() - item.location.screen_rect.topLeft()
                                         - QPoint(0, offscreen.offscreenSize().height() - drawable.size().height()));
}

void QOpenGLPaintEnginePrivate::flushDrawQueue()
{
#ifndef Q_WS_QWS
    Q_Q(QOpenGLPaintEngine);

    glPushMatrix();
    glLoadIdentity();

    offscreen.release();

    if (!drawQueue.isEmpty()) {
        DEBUG_ONCE qDebug() << "QOpenGLPaintEngine::flushDrawQueue():" << drawQueue.size() << "items";

        qreal old_opacity = opacity;
        QPointF old_brush_origin = brush_origin;
        QPainter::CompositionMode old_composition_mode = composition_mode;
        QTransform old_matrix = matrix;
        QBrush old_brush = cbrush;

        bool hqaa_old = high_quality_antialiasing;

        high_quality_antialiasing = true;

        foreach (const QDrawQueueItem &item, drawQueue)
            drawItem(item);

        opacity = old_opacity;
        brush_origin = old_brush_origin;
        q->updateCompositionMode(old_composition_mode);
        matrix = old_matrix;
        cbrush = old_brush;
        brush_style = old_brush.style();

        high_quality_antialiasing = hqaa_old;

        setGLBrush(old_brush.color());
        qt_glColor4ubv(brush_color);

        drawQueue.clear();
    }

    glPopMatrix();

    glDisable(GL_FRAGMENT_PROGRAM_ARB);
#endif
}

bool QOpenGLPaintEnginePrivate::createFragmentPrograms()
{
#ifndef Q_WS_QWS
    QGLContext *ctx = const_cast<QGLContext *>(drawable.context());

    DEBUG_ONCE_STR("QOpenGLPaintEnginePrivate: creating fragment programs");

    for (unsigned int i = 0; i < num_fragment_masks; ++i)
        if (!qt_createFragmentProgram(ctx, mask_fragment_programs[i], mask_fragment_program_sources[i])) {
            DEBUG_ONCE qDebug() << "Couldn't create mask" << i << "fragment program:\n" << mask_fragment_program_sources[i];
            return false;
        }

    for (unsigned int i = 0; i < num_fragment_brushes; ++i)
        for (unsigned int j = 0; j < num_fragment_composition_modes; ++j)
            if (!qt_createFragmentProgram(ctx, painter_fragment_programs[i][j], painter_fragment_program_sources[i][j])) {
                DEBUG_ONCE qDebug() << "Couldn't create painter" << i << j << "fragment program\n:" << painter_fragment_program_sources[i][j];
                return false;
            }
#endif

    return true;
}


void QOpenGLPaintEnginePrivate::deleteFragmentPrograms()
{
#ifndef Q_WS_QWS
    QGL_FUNC_CONTEXT;

    DEBUG_ONCE_STR("QOpenGLPaintEnginePrivate: deleting fragment programs");

    glDeleteProgramsARB(num_fragment_masks, mask_fragment_programs);
    glDeleteProgramsARB(num_fragment_brushes * num_fragment_composition_modes, painter_fragment_programs[0]);
#endif
}

#include "qpaintengine_opengl.moc"
