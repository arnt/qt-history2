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

extern QImage qt_imageForBrush(int brushStyle, bool invert); //in qbrush.cpp

#ifdef Q_WS_WIN
#define QGL_FUNC_CONTEXT QGLContext *ctx = const_cast<QGLContext *>(drawable.context());
#define QGL_D_FUNC_CONTEXT QGLContext *ctx = const_cast<QGLContext *>(d->drawable.context());
#else
#define QGL_FUNC_CONTEXT
#define QGL_D_FUNC_CONTEXT
#endif

#include <stdlib.h>
#include "qpaintengine_opengl_p.h"

#define QREAL_MAX 9e100
#define QREAL_MIN -9e100

extern QGLContextPrivate *qt_glctx_get_dptr(QGLContext *);

#define DISABLE_DEBUG_ONCE

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

class QGLDrawable {
public:
    QGLDrawable() : widget(0), buffer(0), fbo(0) {}
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
};

void QGLDrawable::setDevice(QPaintDevice *pdev)
{
    if (pdev->devType() == QInternal::Widget)
        widget = static_cast<QGLWidget *>(pdev);
    else if (pdev->devType() == QInternal::Pbuffer)
        buffer = static_cast<QGLPixelBuffer *>(pdev);
    else if (pdev->devType() == QInternal::FramebufferObject)
        fbo = static_cast<QGLFramebufferObject *>(pdev);
}

inline void QGLDrawable::swapBuffers()
{
    if (widget && widget->autoBufferSwap())
        widget->swapBuffers();
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
    return QSize();
}

inline QGLFormat QGLDrawable::format() const
{
    if (widget)
        return widget->format();
    else if (buffer)
        return buffer->format();
    else if (fbo && QGLContext::currentContext())
        return QGLContext::currentContext()->format();
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
    return 0;
}

inline bool QGLDrawable::autoFillBackground() const
{
    if (widget)
        return widget->autoFillBackground();
    return false;
}

class QGLOffscreen {
public:
    QGLOffscreen()
        : offscreen(0),
          bound(false),
          context(0),
          copy_needed(false),
          use_fbo(QGLExtensions::glExtensions & QGLExtensions::FramebufferObject)
    {}

    ~QGLOffscreen() {
        glDeleteTextures(1, &drawable_texture);
        if (use_fbo)
            glDeleteTextures(1, &main_fbo_texture);
        else
            glDeleteTextures(1, &offscreen_texture);
    }

    inline void setDevice(QPaintDevice *pdev);

    inline void setDrawableCopyNeeded(bool drawable_copy_needed);
    inline bool isDrawableCopyNeeded() const;

    void begin();
    void end();

    inline void bind();
    inline void bind(const QRectF &rect);
    inline void release();

    inline QSize offscreenSize() const;
    inline QSize textureSize() const;

    inline GLuint offscreenTexture();
    inline GLuint drawableTexture();

private:
    QGLDrawable drawable;

    QGLFramebufferObject *offscreen;
    bool bound;
    QGLContext *context;

    // size of textures. next power of 2 from drawable size.
    QSize sz;

    // used as a fullscreen copy of the main window for rendering
    // and copied back to main buffer at end()
    GLuint main_fbo_texture;
    // used for offscreen rendering of masks
    GLuint offscreen_texture;
    // used to copy from the destination (main_fbo_texture)
    // when rendering composition modes that require destination data in the fragment
    // programs
    GLuint drawable_texture;
    bool drawable_fbo;

    QRectF active_rect;
    QRectF screen_rect;

    bool copy_needed;
    bool use_fbo;
};

inline void QGLOffscreen::setDevice(QPaintDevice *pdev)
{
    drawable.setDevice(pdev);

    drawable_fbo = (pdev->devType() == QInternal::FramebufferObject);
}

inline void QGLOffscreen::setDrawableCopyNeeded(bool drawable_copy_needed)
{
    copy_needed = drawable_copy_needed;
}

inline bool QGLOffscreen::isDrawableCopyNeeded() const
{
    return copy_needed;
}

inline void QGLOffscreen::bind()
{
    bind(QRectF(QPointF(0.0, 0.0), drawable.size()));
}

static uint nextPowerOfTwo(uint v)
{
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    ++v;
    return v;
}

void QGLOffscreen::begin()
{
    QSize needed_size(nextPowerOfTwo(drawable.size().width()), nextPowerOfTwo(drawable.size().height()));

    bool needs_refresh = needed_size.width() > sz.width()
                         || needed_size.height() > sz.height()
                         || (drawable.context() != context
                         && !qgl_share_reg()->checkSharing(drawable.context(), context));
    if (needs_refresh) {
        if (use_fbo) {
            if (!offscreen || needs_refresh) {

                // delete old FBO and texture in its context
                if (context) {
                    context->makeCurrent();
                    delete offscreen;
                    glDeleteTextures(1, &main_fbo_texture);
                    drawable.context()->makeCurrent();
                }

                offscreen = new QGLFramebufferObject(needed_size.width(), needed_size.height());

                if (offscreen->isValid()) {
                    offscreen_texture = offscreen->texture();
                    offscreen->bind();
                    // add one more texture as a color attachment to FBO
                    glGenTextures(1, &main_fbo_texture);
                    glBindTexture(GL_TEXTURE_2D, main_fbo_texture);

#ifndef Q_WS_QWS
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, needed_size.width(), needed_size.height(), 0,
                                 GL_RGBA, GL_UNSIGNED_BYTE, NULL);
#else
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, needed_size.width(), needed_size.height(), 0,
                                 GL_RGBA, GL_UNSIGNED_BYTE, NULL);
#endif
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#ifdef Q_WS_WIN
                    QGLContext *ctx = drawable.context(); // needed to call glFramebufferTexture2DEXT
#endif
                    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1_EXT,
                                              GL_TEXTURE_2D, main_fbo_texture, 0);
                    offscreen->release();

                    if (!offscreen->isValid())
                        glDeleteTextures(1, &main_fbo_texture);
                }

                if (!offscreen->isValid()) {
                    qWarning("QGLOffscreen: Invalid offscreen fbo (size %dx%d)", needed_size.width(), needed_size.height());
                    use_fbo = false;
                    delete offscreen;
                    offscreen = 0;
                }
            }
        }

        GLuint *textures[] = { &drawable_texture, &offscreen_texture };

        int gen_count = use_fbo ? 1 : 2;

        for (int i = 0; i < gen_count; ++i) {
            if (context) {
                context->makeCurrent();
                glDeleteTextures(1, textures[i]);
                drawable.context()->makeCurrent();
            }

            glGenTextures(1, textures[i]);
            glBindTexture(GL_TEXTURE_2D, *textures[i]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, needed_size.width(), needed_size.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }
        sz = needed_size;
    }

    context = drawable.context();
    bound = false;
}

void QGLOffscreen::end()
{
    if (use_fbo && bound) {
        offscreen->release();
        if (drawable_fbo)
            drawable.makeCurrent();
        bound = false;

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
        glBindTexture(GL_TEXTURE_2D, main_fbo_texture);
        // draw the result to the screen

        glBegin(GL_QUADS);
        glTexCoord2f(0.0, drawable.size().height()/qreal(sz.height()));
        glVertex2f(0.0, 0.0);

        glTexCoord2f(drawable.size().width()/qreal(sz.width()),
                     drawable.size().height()/qreal(sz.height()));
        glVertex2f(drawable.size().width(), 0.0);

        glTexCoord2f(drawable.size().width()/qreal(sz.width()), 0.0);
        glVertex2f(drawable.size().width(),
                   drawable.size().height());

        glTexCoord2f(0.0, 0.0);
        glVertex2f(0, drawable.size().height());
        glEnd();
        glBindTexture(GL_TEXTURE_2D, 0);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glColor4f(1, 1, 1, 1);
    }
}

inline void QGLOffscreen::bind(const QRectF &rect)
{
#ifndef Q_WS_QWS
    active_rect = rect;
    screen_rect = rect.adjusted(-1, -1, 1, 1);

    DEBUG_ONCE qDebug() << "QGLOffscreen: binding offscreen (use_fbo =" << use_fbo << ')';

    if (!use_fbo || copy_needed) {
        int left = qMax(0, static_cast<int>(screen_rect.left()));
        int width = qMin(drawable.size().width() - left, static_cast<int>(screen_rect.width()) + 1);

        int bottom = qMax(0, static_cast<int>(drawable.size().height() - screen_rect.bottom()));
        int height = qMin(drawable.size().height() - bottom, static_cast<int>(screen_rect.height()) + 1);

        glBindTexture(GL_TEXTURE_2D, drawable_texture);
        glCopyTexSubImage2D(GL_TEXTURE_2D, 0, left, bottom, left, bottom, width, height);
    }

    if (use_fbo) {

        if (!bound) {
            // Need to copy all of main buffer to color attachment1 of framebuffer object on the first bind()
            glBindTexture(GL_TEXTURE_2D, main_fbo_texture);
            glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, drawable.size().width(), drawable.size().height());
            offscreen->bind();

            glReadBuffer(GL_COLOR_ATTACHMENT1_EXT);

            bound = true;
        }

        glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
    }
#endif
}

inline void QGLOffscreen::release()
{
#ifndef Q_WS_QWS
    DEBUG_ONCE_STR("QGLOffscreen: releasing offscreen");
    if (use_fbo) {
        glDrawBuffer(GL_COLOR_ATTACHMENT1_EXT);
    } else {
        // copy buffer to offscreen
        int left = qMax(0, static_cast<int>(screen_rect.left()));
        int width = qMin(drawable.size().width() - left, static_cast<int>(screen_rect.width()) + 1);

        int bottom = qMax(0, static_cast<int>(drawable.size().height() - screen_rect.bottom()));
        int height = qMin(drawable.size().height() - bottom, static_cast<int>(screen_rect.height()) + 1);

        glBindTexture(GL_TEXTURE_2D, offscreen_texture);
        glCopyTexSubImage2D(GL_TEXTURE_2D, 0, left, bottom, left, bottom, width, height);

        // copy back active_rect
        glPushMatrix();
        glLoadIdentity();

        float inv_size_x = 1.0f / sz.width();
        float inv_size_y = 1.0f / sz.height();

        float x1 = active_rect.left() * inv_size_x;
        float x2 = active_rect.right() * inv_size_x;
        float y1 = (drawable.size().height() - active_rect.top()) * inv_size_y;
        float y2 = (drawable.size().height() - active_rect.bottom()) * inv_size_y;

        GLint src, dst;
        glGetIntegerv(GL_BLEND_SRC, &src);
        glGetIntegerv(GL_BLEND_DST, &dst);

        glBlendFunc(GL_ONE, GL_ZERO);

        float color[4];
        glGetFloatv(GL_CURRENT_COLOR, color);
        glColor4f(1, 1, 1, 1);

        float vertexArray[8];
        float texCoordArray[8];

        qt_add_rect_to_array(active_rect, vertexArray);
        qt_add_texcoords_to_array(x1, y1, x2, y2, texCoordArray);

        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(2, GL_FLOAT, 0, vertexArray);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glTexCoordPointer(2, GL_FLOAT, 0, texCoordArray);

        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, drawable_texture);

        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        glDisable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);

        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        glDisableClientState(GL_VERTEX_ARRAY);

        glPopMatrix();

        glColor4fv(color);

        glBlendFunc(src, dst);
    }
#endif
}

inline QSize QGLOffscreen::offscreenSize() const
{
    return use_fbo ? sz : drawable.size();
}

inline QSize QGLOffscreen::textureSize() const
{
    return sz;
}

inline GLuint QGLOffscreen::drawableTexture()
{
    return drawable_texture;
}

inline GLuint QGLOffscreen::offscreenTexture()
{
    return offscreen_texture;
}

class QOpenGLPaintEnginePrivate : public QPaintEnginePrivate {
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
        , use_antialiasing(false)
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

    inline void setGradientOps(Qt::BrushStyle style);
    void createGradientPaletteTexture(const QGradient& g);

    void updateGradient(const QBrush &brush);

    inline void lineToStencil(qreal x, qreal y);
    inline void curveToStencil(const QPointF &cp1, const QPointF &cp2, const QPointF &ep);
    void pathToVertexArrays(const QPainterPath &path);
    void fillVertexArray(Qt::FillRule fillRule);
    void drawVertexArrays();
    void fillPath(const QPainterPath &path);
    void fillPolygon_dev(const QRectF &boundingRect, const QPointF *polygonPoints, int pointCount,
                         Qt::FillRule fill);

    void drawFastRect(const QRectF &rect);
    void strokePath(const QPainterPath &path, bool use_cache);
    void strokePathFastPen(const QPainterPath &path);

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
    QTransform::TransformationCodes txop;
    QGLDrawable drawable;
    QGLOffscreen offscreen;

    qreal inverseScale;

    int moveToCount;
    QPointF path_start;

    void drawOffscreenPath(const QPainterPath &path);

    void composite(const QRectF &rect);
    void composite(GLuint primitive, const float *vertexArray, int vertexCount);

    bool createFragmentPrograms();
    void deleteFragmentPrograms();
    void updateFragmentProgramData(int locations[]);

    QGLContext *shader_ctx;
    GLuint grad_palette;

    GLuint painter_fragment_programs[num_fragment_brushes][num_fragment_composition_modes];
    GLuint mask_fragment_programs[num_fragment_masks];

    bool use_stencil_method;
    bool has_stencil_face_ext;
    bool use_fragment_programs;
    bool use_antialiasing;

    float inv_matrix_data[3][4];
    float fmp_data[4];
    float fmp2_m_radius2_data[4];
    float angle_data[4];
    float linear_data[4];

    float porterduff_ab_data[4];
    float porterduff_xyz_data[4];

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
};

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

class QGLGradientCache
{
    struct CacheInfo
    {
        inline CacheInfo(QGradientStops s, qreal op) :
            stops(s), opacity(op) {}
        uint buffer[1024];
        QGradientStops stops;
        qreal opacity;
    };

    typedef QMultiHash<quint64, CacheInfo> QGLGradientColorTableHash;

public:
    inline const uint *getBuffer(const QGradientStops &stops, qreal opacity) {
        quint64 hash_val = 0;

        for (int i = 0; i < stops.size() && i <= 2; i++)
            hash_val += stops[i].second.rgba();

        QGLGradientColorTableHash::const_iterator it = cache.constFind(hash_val);

        if (it == cache.constEnd())
            return addCacheElement(hash_val, stops, opacity);
        else {
            do {
                const CacheInfo &cache_info = it.value();
                if (cache_info.stops == stops && cache_info.opacity == opacity)
                    return cache_info.buffer;
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
    uint *addCacheElement(quint64 hash_val, const QGradientStops &stops, qreal opacity) {
        if (cache.size() == maxCacheSize()) {
            int elem_to_remove = qrand() % maxCacheSize();
            cache.remove(cache.keys()[elem_to_remove]); // may remove more than 1, but OK
        }
        CacheInfo cache_entry(stops, opacity);
        generateGradientColorTable(stops, cache_entry.buffer, paletteSize(), opacity);
        return cache.insert(hash_val, cache_entry).value().buffer;
    }

    QGLGradientColorTableHash cache;
};

void QGLGradientCache::generateGradientColorTable(const QGradientStops& s, uint *colorTable, int size, qreal opacity) const
{
    int pos = 0;
    qreal fpos = 0.0;
    qreal incr = 1.0 / qreal(size);
    QVector<uint> colors(s.size());

    for (int i = 0; i < s.size(); ++i)
        colors[i] = s[i].second.rgba();

    uint alpha = qRound(opacity * 255);
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


Q_GLOBAL_STATIC(QGLGradientCache, qt_opengl_gradient_cache)

void QOpenGLPaintEnginePrivate::createGradientPaletteTexture(const QGradient& g)
{
#ifndef Q_WS_QWS //###
    const uint *palbuf = qt_opengl_gradient_cache()->getBuffer(g.stops(), opacity);

    if (g.spread() == QGradient::RepeatSpread || g.type() == QGradient::ConicalGradient)
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    else if (g.spread() == QGradient::ReflectSpread)
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT_IBM);
    else
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, qt_opengl_gradient_cache()->paletteSize(),
                 0, GL_BGRA, GL_UNSIGNED_BYTE, palbuf);
#endif
}


inline void QOpenGLPaintEnginePrivate::setGradientOps(Qt::BrushStyle style)
{
#ifndef Q_WS_QWS //###
    current_style = style;

    glDisable(GL_TEXTURE_GEN_S);
    glDisable(GL_TEXTURE_1D);

    if (style == Qt::LinearGradientPattern) {
        if (use_fragment_programs && (use_antialiasing || !has_fast_composition_mode)) {
            fragment_brush = FRAGMENT_PROGRAM_BRUSH_LINEAR;
        } else {
            glEnable(GL_TEXTURE_GEN_S);
            glEnable(GL_TEXTURE_1D);
        }
    } else {
        if (use_fragment_programs) {
            if (style == Qt::RadialGradientPattern)
                fragment_brush = FRAGMENT_PROGRAM_BRUSH_RADIAL;
            else if (style == Qt::ConicalGradientPattern)
                fragment_brush = FRAGMENT_PROGRAM_BRUSH_CONICAL;
            else if (style == Qt::SolidPattern)
                fragment_brush = FRAGMENT_PROGRAM_BRUSH_SOLID;
            else if (style == Qt::TexturePattern)
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
                                           | PatternBrush)))
{
}

QOpenGLPaintEngine::~QOpenGLPaintEngine()
{
}

static bool qt_createFragmentProgram(QGLContext *ctx, GLuint &program, const char *shader_src)
{
#ifndef Q_WS_WIN
    Q_UNUSED(ctx);
#endif
    glGenProgramsARB(1, &program);
    glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, program);
    glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB,
                       strlen(shader_src), reinterpret_cast<const GLbyte *>(shader_src));

    return glGetError() == GL_NO_ERROR;
}

bool QOpenGLPaintEngine::begin(QPaintDevice *pdev)
{
    Q_D(QOpenGLPaintEngine);

    d->drawable.setDevice(pdev);
    d->offscreen.setDevice(pdev);
    d->offscreen.setDrawableCopyNeeded(true);
    d->has_clipping = false;
    d->has_fast_pen = false;
    d->inverseScale = 1;
    d->opacity = 1;
    d->drawable.makeCurrent();
    d->matrix = QTransform();

    QGLContext *ctx = const_cast<QGLContext *>(d->drawable.context());
    if (QGLExtensions::glExtensions & QGLExtensions::FragmentProgram)
        qt_resolve_frag_program_extensions(ctx);

    d->use_stencil_method = d->drawable.format().stencil() &&
                            QGLExtensions::glExtensions & QGLExtensions::StencilWrap;
    if (d->use_stencil_method && QGLExtensions::glExtensions & QGLExtensions::StencilTwoSide)
        d->has_stencil_face_ext = qt_resolve_stencil_face_extension(ctx);

    if (QGLFormat::openGLVersionFlags() & QGLFormat::OpenGL_Version_1_3)
        qt_resolve_version_1_3_functions(ctx);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

#ifndef Q_WS_QWS
    glPushAttrib(GL_ALL_ATTRIB_BITS);
#endif
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
    if (d->drawable.autoFillBackground()) {
        GLbitfield clearBits = GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT;
#ifndef Q_WS_QWS
        clearBits |= GL_ACCUM_BUFFER_BIT;
#endif
        glClear(clearBits);
    } else if (d->use_stencil_method) {
        glClear(GL_STENCIL_BUFFER_BIT);
    }
    QSize sz(d->drawable.size());
    glViewport(0, 0, sz.width(), sz.height());
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
    if (d->drawable.context() != d->shader_ctx
        && !qgl_share_reg()->checkSharing(d->drawable.context(), d->shader_ctx))
    {
        if (d->shader_ctx) {
            glBindTexture(GL_TEXTURE_1D, 0);
            glDeleteTextures(1, &d->grad_palette);

            if (d->use_fragment_programs)
                d->deleteFragmentPrograms();
        }
        d->shader_ctx = d->drawable.context();
        glGenTextures(1, &d->grad_palette);

        if (QGLExtensions::glExtensions & QGLExtensions::FragmentProgram) {
            d->use_fragment_programs = d->createFragmentPrograms();

            if (!d->use_fragment_programs)
                qWarning() << "QOpenGLPaintEngine: Failed to create fragment programs.";
        }

        gccaps &= ~(RadialGradientFill | ConicalGradientFill | LinearGradientFill | PatternBrush);

        if (d->use_fragment_programs)
            gccaps |= (RadialGradientFill | ConicalGradientFill | LinearGradientFill | PatternBrush);
        else if (QGLExtensions::glExtensions & QGLExtensions::MirroredRepeat)
            gccaps |= LinearGradientFill;
    }
#endif

    setDirty(QPaintEngine::DirtyPen);
    setDirty(QPaintEngine::DirtyBrush);
    setDirty(QPaintEngine::DirtyCompositionMode);
    return true;
}

bool QOpenGLPaintEngine::end()
{
    Q_D(QOpenGLPaintEngine);
    d->offscreen.end();
#ifndef Q_WS_QWS
    glPopAttrib();
#endif
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glFlush();
    d->drawable.swapBuffers();
    d->drawable.doneCurrent();

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


void QOpenGLPaintEnginePrivate::updateGradient(const QBrush &brush)
{
#ifndef Q_WS_QWS
    bool has_mirrored_repeat = QGLExtensions::glExtensions & QGLExtensions::MirroredRepeat;
    Qt::BrushStyle style = brush.style();

    if (has_mirrored_repeat && style == Qt::LinearGradientPattern) {
        const QLinearGradient *g = static_cast<const QLinearGradient *>(brush.gradient());
        QTransform m = brush.transform();
        QPointF start = m.map(g->start());
        QPointF stop;

        if (qFuzzyCompare(m.m11(), m.m22()) && m.m12() == 0.0 && m.m21() == 0.0) {
            // It is a simple uniform scale and/or translation
            stop = m.map(g->finalStop());
        } else {
            // It is not enough to just transform the endpoints.
            // We have to make sure the _pattern_ is transformed correctly.

            qreal odx = g->finalStop().x() - g->start().x();
            qreal ody = g->finalStop().y() - g->start().y();

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
        glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
        glTexGenfv(GL_S, GL_OBJECT_PLANE, tr);
    }

    if (use_fragment_programs) {
        if (style == Qt::RadialGradientPattern) {
            const QRadialGradient *g = static_cast<const QRadialGradient *>(brush.gradient());
            QTransform translate(1, 0, 0, 1, -g->focalPoint().x(), -g->focalPoint().y());
            QTransform gl_to_qt(1, 0, 0, -1, 0, pdev->height());
            QTransform inv_matrix = gl_to_qt * matrix.inverted() * brush.transform().inverted() * translate;

            setInvMatrixData(inv_matrix);

            fmp_data[0] = g->center().x() - g->focalPoint().x();
            fmp_data[1] = g->center().y() - g->focalPoint().y();

            fmp2_m_radius2_data[0] = -fmp_data[0] * fmp_data[0] - fmp_data[1] * fmp_data[1] + g->radius() * g->radius();
        } else if (style == Qt::ConicalGradientPattern) {
            const QConicalGradient *g = static_cast<const QConicalGradient *>(brush.gradient());
            QTransform translate(1, 0, 0, 1, -g->center().x(), -g->center().y());
            QTransform gl_to_qt(1, 0, 0, -1, 0, pdev->height());
            QTransform inv_matrix = gl_to_qt * matrix.inverted() * brush.transform().inverted() * translate;

            setInvMatrixData(inv_matrix);

            angle_data[0] = -(g->angle() * 2 * Q_PI) / 360.0;
        } else if (style == Qt::LinearGradientPattern) {
            const QLinearGradient *g = static_cast<const QLinearGradient *>(brush.gradient());

            QTransform translate(1, 0, 0, 1, -g->start().x(), -g->start().y());
            QTransform gl_to_qt(1, 0, 0, -1, 0, pdev->height());

            QTransform inv_matrix = gl_to_qt * matrix.inverted() * brush.transform().inverted() * translate;

            setInvMatrixData(inv_matrix);

            QPointF l = g->finalStop() - g->start();

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
        glBindTexture(GL_TEXTURE_1D, grad_palette);
        createGradientPaletteTexture(*brush.gradient());
    }
#endif
}


class QOpenGLTessellator : public QTessellator
{
public:
    QOpenGLTessellator() {}
    ~QOpenGLTessellator() { }
    inline void addTrapHelper(const Trapezoid &trap,
                              qreal &topLeftX,
                              qreal &topRightX,
                              qreal &bottomLeftX,
                              qreal &bottomRightX,
                              qreal &top,
                              qreal &bottom) {

        top = Q27Dot5ToDouble(trap.top);
        bottom = Q27Dot5ToDouble(trap.bottom);

        Q27Dot5 y = trap.topLeft->y - trap.bottomLeft->y;

        qreal topLeftY = Q27Dot5ToDouble(trap.topLeft->y);

        qreal tx = Q27Dot5ToDouble(trap.topLeft->x);
        qreal m = (-tx + Q27Dot5ToDouble(trap.bottomLeft->x)) / Q27Dot5ToDouble(y);
        topLeftX = tx + m * (topLeftY - top);
        bottomLeftX = tx + m * (topLeftY - bottom);

//     qDebug() << "trap: top=" << Q27Dot5ToDouble(trap.top)
//              << "bottom=" << Q27Dot5ToDouble(trap.bottom);
//     qDebug() << "      topLeft=" << Q27Dot5ToDouble(trap.topLeft->x) << Q27Dot5ToDouble(trap.topLeft->y);
//     qDebug() << "      bottomLeft=" << Q27Dot5ToDouble(trap.bottomLeft->x) << Q27Dot5ToDouble(trap.bottomLeft->y);

//     qDebug() << " -> m=" << m << "tx=" << tx;
//     qDebug() << " -> topLeftX" << topLeftX << "bottomLeftX" << bottomLeftX;

        y = trap.topRight->y - trap.bottomRight->y;

        qreal topRightY = Q27Dot5ToDouble(trap.topRight->y);

        tx = Q27Dot5ToDouble(trap.topRight->x);
        m = (-tx + Q27Dot5ToDouble(trap.bottomRight->x)) / Q27Dot5ToDouble(y);
        topRightX = tx + m * (topRightY - Q27Dot5ToDouble(trap.top));
        bottomRightX = tx + m * (topRightY - Q27Dot5ToDouble(trap.bottom));
    }
};

class QOpenGLImmediateModeTessellator : public QOpenGLTessellator
{
public:
    QOpenGLImmediateModeTessellator(qreal offscrHeight, QGLContext *cx) :
        offscreenHeight(offscrHeight), ctx(cx) {}
    ~QOpenGLImmediateModeTessellator() {}
    void addTrap(const Trapezoid &trap);
    void tessellate(const QPointF *points, int nPoints, bool winding) {
        setWinding(winding);
        QTessellator::tessellate(points, nPoints);
    }
    void done() {}

    qreal offscreenHeight;
    QGLContext *ctx;
};

void QOpenGLImmediateModeTessellator::addTrap(const Trapezoid &trap)
{
    qreal topLeftX;
    qreal topRightX;
    qreal bottomLeftX;
    qreal bottomRightX;
    qreal top;
    qreal bottom;

    addTrapHelper(trap, topLeftX, topRightX, bottomLeftX, bottomRightX,
                  top, bottom);

    qreal minX = qMin(topLeftX, bottomLeftX);
    qreal maxX = qMax(topRightX, bottomRightX);

    if (qFuzzyCompare(top, bottom) || qFuzzyCompare(minX, maxX) ||
        qFuzzyCompare(topLeftX, topRightX) && qFuzzyCompare(bottomLeftX, bottomRightX))
        return;

    const qreal xpadding = 1.0;
    const qreal ypadding = 1.0;

    qreal topDist = offscreenHeight - top;
    qreal bottomDist = offscreenHeight - bottom;

    qreal reciprocal = bottomDist / (bottomDist - topDist);

    qreal leftB = bottomLeftX + (topLeftX - bottomLeftX) * reciprocal;
    qreal rightB = bottomRightX + (topRightX - bottomRightX) * reciprocal;

    const bool topZero = qFuzzyCompare(topDist, 0);

    reciprocal = topZero ? 1.0 / bottomDist : 1.0 / topDist;

    qreal leftA = topZero ? (bottomLeftX - leftB) * reciprocal : (topLeftX - leftB) * reciprocal;
    qreal rightA = topZero ? (bottomRightX - rightB) * reciprocal : (topRightX - rightB) * reciprocal;

    qreal invLeftA = qFuzzyCompare(leftA, 0.0) ? 0.0 : 1.0 / leftA;
    qreal invRightA = qFuzzyCompare(rightA, 0.0) ? 0.0 : 1.0 / rightA;

    // fragment program needs the negative of invRightA as it mirrors the line
    glTexCoord4f(topDist, bottomDist, invLeftA, -invRightA);
    glMultiTexCoord4f(GL_TEXTURE1, leftA, leftB, rightA, rightB);

    qreal topY = top - ypadding;
    qreal bottomY = bottom + ypadding;

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

class QOpenGLTrapezoidToArrayTessellator : public QOpenGLTessellator
{
public:
    QOpenGLTrapezoidToArrayTessellator() : vertices(0), allocated(0), size(0) {}
    ~QOpenGLTrapezoidToArrayTessellator() { free(vertices); }
    float *vertices;
    int allocated;
    int size;
    void addTrap(const Trapezoid &trap);
    void tessellate(const QPointF *points, int nPoints, bool winding) {
        size = 0;
        setWinding(winding);
        QTessellator::tessellate(points, nPoints);
    }
    void done() {
        if (allocated > 512) {
            free(vertices);
            vertices = 0;
            allocated = 0;
        }
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

    qreal topLeftX;
    qreal topRightX;
    qreal bottomLeftX;
    qreal bottomRightX;
    qreal top;
    qreal bottom;

    addTrapHelper(trap, topLeftX, topRightX, bottomLeftX, bottomRightX,
                  top, bottom);

#ifndef Q_WS_QWS
    vertices[size++] = topLeftX;
    vertices[size++] = top;
    vertices[size++] = topRightX;
    vertices[size++] = top;
    vertices[size++] = bottomRightX;
    vertices[size++] = bottom;
    vertices[size++] = bottomLeftX;
    vertices[size++] = bottom;
#else
    vertices[size++] = topLeftX;
    vertices[size++] = top;
    vertices[size++] = topRightX;
    vertices[size++] = top;
    vertices[size++] = bottomRightX;
    vertices[size++] = bottom;

    vertices[size++] = bottomLeftX;
    vertices[size++] = bottom;
    vertices[size++] = topLeftX;
    vertices[size++] = top;
    vertices[size++] = bottomRightX;
    vertices[size++] = bottom;
#endif
}


void QOpenGLPaintEnginePrivate::fillPolygon_dev(const QRectF &rect, const QPointF *polygonPoints, int pointCount,
                                                Qt::FillRule fill)
{
    QOpenGLTrapezoidToArrayTessellator tessellator;
    tessellator.tessellate(polygonPoints, pointCount, fill == Qt::WindingFill);

    DEBUG_ONCE qDebug() << "QOpenGLPaintEnginePrivate: Drawing polygon with" << pointCount << "points using fillPolygon_dev";

    bool fast_style = current_style == Qt::LinearGradientPattern
                      || current_style == Qt::SolidPattern;

#ifndef Q_WS_QWS
    GLenum geometry_mode = GL_QUADS;
#else
    GLenum geometry_mode = GL_TRIANGLES;
#endif

    if (use_fragment_programs && !(fast_style && has_fast_composition_mode)) {
        const QRectF screen_rect = rect.adjusted(-1, -1, 1, 1);
        offscreen.bind(screen_rect);

        // fill mask
        float vertexArray[8];
        qt_add_rect_to_array(screen_rect, vertexArray);

        glBlendFunc(GL_ONE, GL_ZERO);

        float color[4];
        glGetFloatv(GL_CURRENT_COLOR, color);
        glColor4f(1, 1, 1, 1);

        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(2, GL_FLOAT, 0, vertexArray);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        glDisableClientState(GL_VERTEX_ARRAY);

        offscreen.release();

        glColor4fv(color);

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

    bool fast_style = current_style == Qt::LinearGradientPattern
                      || current_style == Qt::SolidPattern;

    if (use_fragment_programs && !(fast_style && has_fast_composition_mode)) {
        DEBUG_ONCE qDebug() << "QOpenGLPaintEnginePrivate: Drawing polygon using stencil method (fragment programs)";

        QRectF rect(QPointF(min_x, min_y), QSizeF(max_x - min_x, max_y - min_y));

        offscreen.bind(rect);

        glBlendFunc(GL_ONE, GL_ZERO);

        float color[4];
        glGetFloatv(GL_CURRENT_COLOR, color);
        glColor4f(1, 1, 1, 1);

        // Fill mask to 1
        glBegin(GL_QUADS);
        glVertex2f(min_x, min_y);
        glVertex2f(max_x, min_y);
        glVertex2f(max_x, max_y);
        glVertex2f(min_x, max_y);
        glEnd();

        offscreen.release();

        glColor4fv(color);

        // Enable stencil func.
        glStencilFunc(GL_NOTEQUAL, 0, stencilMask);
        composite(rect);
    } else {
        DEBUG_ONCE qDebug() << "QOpenGLPaintEnginePrivate: Drawing polygon using stencil method (no fragment programs)";

        // Enable stencil func.
        glStencilFunc(GL_NOTEQUAL, 0, stencilMask);
        glBegin(GL_QUADS);
        glVertex2f(min_x, min_y);
        glVertex2f(max_x, min_y);
        glVertex2f(max_x, max_y);
        glVertex2f(min_x, max_y);
        glEnd();
    }

    glStencilMask(~0);
    glStencilFunc(GL_ALWAYS, 0, 0);
    glStencilOp(GL_ZERO, GL_ZERO, GL_ZERO);

    // clear all stencil values to 0
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

    glBegin(GL_QUADS);
    glVertex2f(min_x, min_y);
    glVertex2f(max_x, min_y);
    glVertex2f(max_x, max_y);
    glVertex2f(min_x, max_y);
    glEnd();

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

    if (use_stencil_method && !(use_antialiasing && use_fragment_programs)) {
        pathToVertexArrays(path);
        fillVertexArray(path.fillRule());
        return;
    }

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    if (use_antialiasing && use_fragment_programs)
        drawOffscreenPath(path);
    else {
        QPolygonF poly = path.toFillPolygon(matrix);
        fillPolygon_dev(poly.boundingRect(), poly.data(), poly.count(),
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
        glLineWidth(pen.widthF());
    }

    if (d->pen_brush_style >= Qt::LinearGradientPattern
        && d->pen_brush_style <= Qt::ConicalGradientPattern) {
        d->updateGradient(pen.brush());
        d->setGLPen(Qt::white);
    } else {
        d->setGLPen(pen.color());
        qt_glColor4ubv(d->pen_color);
    }
}

void QOpenGLPaintEngine::updateBrush(const QBrush &brush, const QPointF &origin)
{
    Q_D(QOpenGLPaintEngine);
    d->cbrush = brush;
    d->brush_style = brush.style();
    d->brush_origin = origin;
    d->has_brush = (d->brush_style != Qt::NoBrush);

    // This is to update the gradient GL settings even when
    // the brush does not have a gradient (disable unwanted states etc)
    d->updateGradient(brush);

    if (!(d->brush_style >= Qt::LinearGradientPattern
        && d->brush_style <= Qt::ConicalGradientPattern)) {
        d->setGLBrush(brush.color());
        qt_glColor4ubv(d->brush_color);
    }
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

    d->txop = (QTransform::TransformationCodes)mtx.type();

    // 1/10000 == 0.0001, so we have good enough res to cover curves
    // that span the entire widget...
    d->inverseScale = qMax(1 / qMax( qMax(qAbs(mtx.m11()), qAbs(mtx.m22())),
                                     qMax(qAbs(mtx.m12()), qAbs(mtx.m21())) ),
                           0.0001);

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

    if (op == Qt::NoClip) {
        d->has_clipping = false;
        d->crgn = QRegion();
        glDisable(GL_DEPTH_TEST);
        return;
    }

    QRegion region = clipRegion * d->matrix;
    switch (op) {
    case Qt::IntersectClip:
        if (d->has_clipping) {
            d->crgn &= region;
            break;
        }
        // fall through
    case Qt::ReplaceClip:
        d->crgn = region;
        break;
    case Qt::UniteClip:
        d->crgn |= region;
        break;
    default:
        break;
    }

#ifndef Q_WS_QWS
    glClearDepth(0x0);
#endif
    glClear(GL_DEPTH_BUFFER_BIT);
    glDepthMask(true);
#ifndef Q_WS_QWS
    glClearDepth(0x1);
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

    d->use_antialiasing = hints & QPainter::Antialiasing;

    if (!(QGLExtensions::glExtensions & QGLExtensions::SampleBuffers))
        return;

    if (d->use_antialiasing)
        glEnable(GL_MULTISAMPLE);
    else
        glDisable(GL_MULTISAMPLE);
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

    d->has_fast_composition_mode = !d->use_antialiasing
                                   || composition_mode == QPainter::CompositionMode_SourceOver
                                   || composition_mode == QPainter::CompositionMode_Destination
                                   || composition_mode == QPainter::CompositionMode_DestinationOver
                                   || composition_mode == QPainter::CompositionMode_DestinationOut
                                   || composition_mode == QPainter::CompositionMode_SourceAtop
                                   || composition_mode == QPainter::CompositionMode_Xor;

    if (d->has_fast_composition_mode) {
        d->fragment_composition_mode = COMPOSITION_MODE_BLEND_MODE;
        d->offscreen.setDrawableCopyNeeded(false);

        DEBUG_ONCE_STR("QOpenGLPaintEngine::updateCompositionMode: using blend mode compositioning");
    } else {
        d->fragment_composition_mode = COMPOSITION_MODES_SIMPLE_PORTER_DUFF;
        d->offscreen.setDrawableCopyNeeded(true);

        DEBUG_ONCE_STR("QOpenGLPaintEngine::updateCompositionMode: using fragment program compositioning");
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
    default:
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        d->setPorterDuffData(1, 0, 1, 1, 1);
    }
}

void QOpenGLPaintEnginePrivate::drawOffscreenPath(const QPainterPath &path)
{
#ifndef Q_WS_QWS
    QGL_FUNC_CONTEXT;

    DEBUG_ONCE_STR("QOpenGLPaintEnginePrivate::drawOffscreenPath()");

    QList<QPolygonF> polys; // = path.toFillPolygons(matrix);

    polys << path.toFillPolygon(matrix);

    if (polys.isEmpty())
        return;

    if (has_clipping)
        glDisable(GL_DEPTH_TEST);

    QRectF boundingRect = polys.at(0).boundingRect();

    for (int i = 1; i < polys.size(); ++i)
        boundingRect = boundingRect.united(polys.at(i).boundingRect());

    boundingRect = boundingRect.adjusted(-2, -2, 2, 2);

    GLfloat vertexArray[4 * 2];
    qt_add_rect_to_array(boundingRect, vertexArray);

    offscreen.bind(boundingRect);

    // clear mask
    glBlendFunc(GL_ZERO, GL_ZERO); // clear
    glVertexPointer(2, GL_FLOAT, 0, vertexArray);
    glEnableClientState(GL_VERTEX_ARRAY);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glDisableClientState(GL_VERTEX_ARRAY);

    glBlendFunc(GL_ONE, GL_ONE); // add mask
    glEnable(GL_FRAGMENT_PROGRAM_ARB);
    glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, mask_fragment_programs[FRAGMENT_PROGRAM_MASK_TRAPEZOID_AA]);

    glBegin(GL_QUADS);
    for (int i = 0; i < polys.size(); ++i) {
        const QPolygonF &poly = polys.at(i);

        // draw mask to offscreen
        QOpenGLImmediateModeTessellator tessellator(drawable.size().height(), shader_ctx);
        tessellator.tessellate(poly.data(), poly.count(), path.fillRule() == Qt::WindingFill);
    }
    glEnd();

    glDisable(GL_FRAGMENT_PROGRAM_ARB);

    offscreen.release();

    if (has_clipping)
        glEnable(GL_DEPTH_TEST);

    composite(boundingRect.adjusted(1, 1, -1, -1));
#endif
}

void QOpenGLPaintEnginePrivate::drawFastRect(const QRectF &r)
{
    DEBUG_ONCE_STR("QOpenGLPaintEngine::drawRects(): drawing fast rect");

    float vertexArray[10];
    qt_add_rect_to_array(r, vertexArray);

    if (has_brush) {
        setGradientOps(brush_style);

        bool use_compositioning =
            use_fragment_programs
            && brush_style != Qt::SolidPattern
            && brush_style != Qt::LinearGradientPattern;

        if (use_compositioning) {
            offscreen.bind();
            glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

            glBlendFunc(GL_ONE, GL_ZERO);
        } else {
            qt_glColor4ubv(brush_color);

            if (brush_style == Qt::LinearGradientPattern) {
                glEnable(GL_TEXTURE_GEN_S);
                glEnable(GL_TEXTURE_1D);
            }
        }

        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(2, GL_FLOAT, 0, vertexArray);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        glDisableClientState(GL_VERTEX_ARRAY);

        if (use_compositioning) {
            offscreen.release();
            qt_glColor4ubv(brush_color);

            composite(GL_TRIANGLE_FAN, vertexArray, 4);
        } else if (brush_style == Qt::LinearGradientPattern) {
            glDisable(GL_TEXTURE_GEN_S);
            glDisable(GL_TEXTURE_1D);
        }
    }

    if (has_pen) {
        setGradientOps(pen_brush_style);
        qt_glColor4ubv(pen_color);

        if (has_fast_pen && !(use_fragment_programs && use_antialiasing)) {
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

void QOpenGLPaintEngine::drawRects(const QRectF *rects, int rectCount)
{
    Q_D(QOpenGLPaintEngine);

    for (int i=0; i<rectCount; ++i) {
        QRectF r = rects[i];

        bool fast_rect = false;

        // don't allow rotations
        if (d->use_antialiasing && d->matrix.type() < QTransform::TxRotShear) {
            QRectF screen_rect = d->matrix.mapRect(r);

            // pixel aligned rect?
            fast_rect =
                screen_rect.topLeft().toPoint() == screen_rect.topLeft()
                && screen_rect.bottomRight().toPoint() == screen_rect.bottomRight();
        }

        // optimization for rects which can be drawn aliased
        if (fast_rect || !d->use_antialiasing) {
            d->drawFastRect(r);
        } else {
            qreal left = r.left();
            qreal right = r.right();
            qreal top = r.top();
            qreal bottom = r.bottom();

            QPainterPath path;
            path.setFillRule(Qt::WindingFill);

            path.moveTo(left, top);
            path.lineTo(right, top);
            path.lineTo(right, bottom);
            path.lineTo(left, bottom);
            path.lineTo(left, top);

            drawPath(path);
        }
    }
}

void QOpenGLPaintEngine::drawPoints(const QPointF *points, int pointCount)
{
    Q_D(QOpenGLPaintEngine);
    d->setGradientOps(d->pen_brush_style);

    GLfloat pen_width = d->cpen.widthF();
    if (pen_width > 1 || (pen_width > 0 && d->txop > QTransform::TxTranslate)) {
        QPaintEngine::drawPoints(points, pointCount);
        return;
    }

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
        d->setGradientOps(d->pen_brush_style);
        if (d->has_fast_pen && !(d->use_fragment_programs && d->use_antialiasing)) {
            qt_glColor4ubv(d->pen_color);
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
            qt_glColor4ubv(d->pen_color);
            for (int i=0; i<lineCount; ++i) {
                const QLineF &l = lines[i];
                QPainterPath path;
                path.setFillRule(Qt::WindingFill);
                path.moveTo(l.x1(), l.y1());
                path.lineTo(l.x2(), l.y2());

                d->strokePath(path, false);
            }
        }
    }
}


void QOpenGLPaintEngine::drawPolygon(const QPointF *points, int pointCount, PolygonDrawMode mode)
{
    Q_D(QOpenGLPaintEngine);
    if(pointCount < 2)
        return;

    if (d->has_brush && mode != PolylineMode) {
        d->setGradientOps(d->brush_style);
        if (mode == ConvexMode && !(d->use_antialiasing && d->use_fragment_programs)) {

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
            qt_glColor4ubv(d->brush_color);
            QPainterPath path;
            path.setFillRule(mode == WindingMode ? Qt::WindingFill : Qt::OddEvenFill);
            path.moveTo(points[0]);
            for (int i=1; i<pointCount; ++i)
                path.lineTo(points[i]);
            d->fillPath(path);
        }
    }

    if (d->has_pen) {
        d->setGradientOps(d->pen_brush_style);
        qt_glColor4ubv(d->pen_color);
        if (d->has_fast_pen && !(d->use_fragment_programs && d->use_antialiasing)) {
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

            d->strokePath(path, true);
        }
    }
}

void QOpenGLPaintEnginePrivate::strokePath(const QPainterPath &path, bool use_cache)
{
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

}

void QOpenGLPaintEnginePrivate::strokePathFastPen(const QPainterPath &path)
{
#ifndef Q_WS_QWS
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
        qt_glColor4ubv(d->brush_color);

        bool path_closed = pathClosed(path);

        bool has_thick_pen =
            path_closed
            && d->has_pen
            && d->cpen.style() == Qt::SolidLine
            && d->cpen.isSolid()
            && d->cpen.color().alpha() == 255
            && d->txop <= QTransform::TxRotShear
            && d->cpen.widthF() >= 2.0 / sqrt(qMin(d->matrix.m11() * d->matrix.m11()
                                                   + d->matrix.m21() * d->matrix.m21(),
                                                   d->matrix.m12() * d->matrix.m12()
                                                   + d->matrix.m22() * d->matrix.m22()));

        if (has_thick_pen) {
            DEBUG_ONCE qDebug() << "QOpenGLPaintEngine::drawPath(): Using thick pen optimization";

            bool temp = d->use_antialiasing;
            d->use_antialiasing = false;

            updateCompositionMode(d->composition_mode);
            d->setGradientOps(d->brush_style);
            d->fillPath(path);

            d->use_antialiasing = temp;
            updateCompositionMode(d->composition_mode);
        } else {
            d->setGradientOps(d->brush_style);
            d->fillPath(path);
        }
    }
    if (d->has_pen) {
        qt_glColor4ubv(d->pen_color);
        d->setGradientOps(d->pen_brush_style);
        if (d->has_fast_pen && !(d->use_fragment_programs && d->use_antialiasing))
            d->strokePathFastPen(path);
        else
            d->strokePath(path, true);
    }
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
    GLenum target = QGLExtensions::glExtensions & QGLExtensions::TextureRectangle
                    ? GL_TEXTURE_RECTANGLE_NV
                    : GL_TEXTURE_2D;
    if (r.size() != pm.size())
        target = GL_TEXTURE_2D;
    d->drawable.bindTexture(pm, target);

    drawTextureRect(pm.width(), pm.height(), r, sr, target);
}

void QOpenGLPaintEngine::drawTiledPixmap(const QRectF &r, const QPixmap &pm, const QPointF &)
{
    Q_D(QOpenGLPaintEngine);
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

void QOpenGLPaintEngine::drawImage(const QRectF &r, const QImage &image, const QRectF &sr,
                                   Qt::ImageConversionFlags)
{
    Q_D(QOpenGLPaintEngine);
    GLenum target = QGLExtensions::glExtensions & QGLExtensions::TextureRectangle
                    ? GL_TEXTURE_RECTANGLE_NV
                    : GL_TEXTURE_2D;
    if (r.size() != image.size())
        target = GL_TEXTURE_2D;
    d->drawable.bindTexture(image, target);
    drawTextureRect(image.width(), image.height(), r, sr, target);
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
extern int nearest_gl_texture_size(int v);

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

void QGLGlyphCache::cacheGlyphs(QGLContext *context, const QTextItemInt &ti,
                                const QVarLengthArray<glyph_t> &glyphs)
{
#ifndef Q_WS_QWS //###
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
        GLint tex_height = nearest_gl_texture_size(qRound(ti.ascent.toReal() + ti.descent.toReal())+2);
        GLint tex_width = nearest_gl_texture_size(tex_height*30); // ###
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
                int strip_height = nearest_gl_texture_size(qRound(ti.ascent.toReal() + ti.descent.toReal())+2);
                font_tex->x_offset = x_margin;
                font_tex->y_offset += strip_height;
                if (font_tex->y_offset >= font_tex->height) {
                    // get hold of the old font texture
                    uchar *old_tex_data = (uchar *) malloc(font_tex->width*font_tex->height*2);
                    int old_tex_height = font_tex->height;
                    glGetTexImage(GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, old_tex_data);

                    // realloc a larger texture
                    glDeleteTextures(1, &font_tex->texture);
                    glGenTextures(1, &font_tex->texture);
                    font_tex->height = nearest_gl_texture_size(font_tex->height + strip_height);
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

            QGLGlyphCoord *qgl_glyph = new QGLGlyphCoord;
            qgl_glyph->x = qreal(font_tex->x_offset) / font_tex->width;
            qgl_glyph->y = qreal(font_tex->y_offset) / font_tex->height;
            qgl_glyph->width = qreal(glyph_width) / font_tex->width;
            qgl_glyph->height = qreal(glyph_height) / font_tex->height;
            qgl_glyph->log_width = qgl_glyph->width * font_tex->width;
            qgl_glyph->log_height = qgl_glyph->height * font_tex->height;
            qgl_glyph->x_offset = -metrics.x;
            qgl_glyph->y_offset = metrics.y;

            QImage glyph_im(ti.fontEngine->alphaMapForGlyph(glyphs[i]).convertToFormat(QImage::Format_Indexed8));

            if (!glyph_im.isNull()) {
                int padded_width = glyph_im.width();
                if (padded_width%2 != 0)
                    ++padded_width;

                int idx = 0;
                uchar *tex_data = (uchar *) malloc(padded_width*glyph_im.height()*2);
                memset(tex_data, 0, padded_width*glyph_im.height()*2);

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
                                padded_width, glyph_im.height(),
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
#endif
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

    // add the glyphs used to the glyph texture cache
    const QTextItemInt &ti = static_cast<const QTextItemInt &>(textItem);
    QVarLengthArray<QFixedPoint> positions;
    QVarLengthArray<glyph_t> glyphs;
    QTransform matrix;
    matrix.translate(qRound(p.x()), qRound(p.y()));
    ti.fontEngine->getGlyphPositions(ti.glyphs, ti.num_glyphs, matrix, ti.flags, glyphs, positions);

    // make sure the glyphs we want to draw are in the cache
    qt_glyph_cache()->cacheGlyphs(d->drawable.context(), ti, glyphs);

    d->setGradientOps(Qt::SolidPattern); // turns off gradient ops
    qt_glColor4ubv(d->pen_color);
    glEnable(GL_TEXTURE_2D);

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
}


void QOpenGLPaintEngine::drawEllipse(const QRectF &rect)
{
#ifndef Q_WS_QWS
    Q_D(QOpenGLPaintEngine);
    QGL_D_FUNC_CONTEXT

    if (d->use_antialiasing && d->use_fragment_programs) {
        if (d->has_brush) {
            QPointF center = rect.center();

            QPointF points[] = {
                QPointF(rect.left(), center.y()),
                QPointF(rect.right(), center.y()),
                QPointF(center.x(), rect.top()),
                QPointF(center.x(), rect.bottom())
            };

            qreal min_screen_delta_len = QREAL_MAX;

            for (int i = 0; i < 4; ++i) {
                QPointF delta = points[i] - center;

                // normalize
                delta /= sqrt(delta.x() * delta.x() + delta.y() * delta.y());

                QPointF screen_delta(d->matrix.m11() * delta.x() + d->matrix.m21() * delta.y(),
                                     d->matrix.m12() * delta.x() + d->matrix.m22() * delta.y());

                min_screen_delta_len = qMin(min_screen_delta_len,
                                            sqrt(screen_delta.x() * screen_delta.x() + screen_delta.y() * screen_delta.y()));
            }

            const qreal padding = 2.0f;

            qreal grow = padding / min_screen_delta_len;

            float vertexArray[4 * 2];

            QRectF boundingRect = rect.adjusted(-grow, -grow, grow, grow);
            qt_add_rect_to_array(boundingRect, vertexArray);

            // fragment program needs the inverse radii of the ellipse
            glTexCoord2f(1.0f / (rect.width() * 0.5f),
                         1.0f / (rect.height() * 0.5f));

            QTransform translate(1, 0, 0, 1, -center.x(), -center.y());
            QTransform gl_to_qt(1, 0, 0, -1, 0, d->pdev->height());
            QTransform inv_matrix = gl_to_qt * d->matrix.inverted() * translate;

            float m[3][4] = { { inv_matrix.m11(), inv_matrix.m12(), inv_matrix.m13() },
                              { inv_matrix.m21(), inv_matrix.m22(), inv_matrix.m23() },
                              { inv_matrix.m31(), inv_matrix.m32(), inv_matrix.m33() } };

            QRectF screenRect = d->matrix.mapRect(boundingRect);

            if (d->has_clipping)
                glDisable(GL_DEPTH_TEST);

            d->offscreen.bind(screenRect);
            glBlendFunc(GL_ONE, GL_ZERO); // set mask
            glEnable(GL_FRAGMENT_PROGRAM_ARB);
            glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, d->mask_fragment_programs[FRAGMENT_PROGRAM_MASK_ELLIPSE_AA]);

            int *locations = mask_variable_locations[FRAGMENT_PROGRAM_MASK_ELLIPSE_AA];

            glProgramLocalParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, locations[VAR_INV_MATRIX_M0], m[0]);
            glProgramLocalParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, locations[VAR_INV_MATRIX_M1], m[1]);
            glProgramLocalParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, locations[VAR_INV_MATRIX_M2], m[2]);

            glEnableClientState(GL_VERTEX_ARRAY);
            glVertexPointer(2, GL_FLOAT, 0, vertexArray);
            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
            glDisableClientState(GL_VERTEX_ARRAY);

            glDisable(GL_FRAGMENT_PROGRAM_ARB);

            d->offscreen.release();

            if (d->has_clipping)
                glEnable(GL_DEPTH_TEST);

            d->setGradientOps(d->brush_style);
            qt_glColor4ubv(d->brush_color);

            qreal shrink = grow * 0.5;

            d->composite(boundingRect.adjusted(shrink, shrink, -shrink, -shrink));
        }

        if (d->has_pen) {
            d->setGradientOps(d->pen_brush_style);
            qt_glColor4ubv(d->pen_color);

            QPainterPath path;
            path.addEllipse(rect);

            if (d->has_fast_pen && !(d->use_fragment_programs && d->use_antialiasing))
                d->strokePathFastPen(path);
            else
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
#ifndef Q_WS_QWS
    QGL_FUNC_CONTEXT;

    QSize sz = offscreen.textureSize();

    float inv_buffer_size_data[4] = { 1.0f / sz.width(), 1.0f / sz.height(), 0.0f, 0.0f };

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
        case VAR_INV_BUFFER_SIZE:
            glProgramLocalParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, location, inv_buffer_size_data);
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


void QOpenGLPaintEnginePrivate::composite(const QRectF &rect)
{
#ifndef Q_WS_QWS
    float vertexArray[8];
    qt_add_rect_to_array(rect, vertexArray);

    composite(GL_TRIANGLE_FAN, vertexArray, 4);
#endif
}


void QOpenGLPaintEnginePrivate::composite(GLuint primitive, const float *vertexArray, int vertexCount)
{
#ifndef Q_WS_QWS
    Q_Q(QOpenGLPaintEngine);
    QGL_FUNC_CONTEXT;

    DEBUG_ONCE_STR("QOpenGLPaintEnginePrivate: Using compositing program");

    if (has_fast_composition_mode)
        q->updateCompositionMode(composition_mode);
    else
        glBlendFunc(GL_ONE, GL_ZERO);

    int *locations = painter_variable_locations[fragment_brush][fragment_composition_mode];

    int texture_locations[] = { locations[VAR_DST_TEXTURE],
                                locations[VAR_MASK_TEXTURE],
                                locations[VAR_PALETTE] };

    int brush_texture_location = locations[VAR_BRUSH_TEXTURE];

    GLuint texture_targets[] = { GL_TEXTURE_2D,
                                 GL_TEXTURE_2D,
                                 GL_TEXTURE_1D };

    GLuint textures[] = { offscreen.drawableTexture(),
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
