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

#define DISABLE_DEBUG_ONCE

#ifdef DISABLE_DEBUG_ONCE
#define DEBUG_ONCE_STR(str) ;
#define DEBUG_ONCE if (0)
#else
#define DEBUG_ONCE_STR(str) for (static bool DEBUG_ONCE_FLAG = false; !DEBUG_ONCE_FLAG; DEBUG_ONCE_FLAG = true) qDebug() << (str);
#define DEBUG_ONCE for (static bool DEBUG_ONCE_FLAG = false; !DEBUG_ONCE_FLAG; DEBUG_ONCE_FLAG = true)
#endif

static inline void qt_glColor4ubv(unsigned char *col)
{
#ifdef Q_WS_QWS
        glColor4f(col[0]/255.0, col[1]/255.0, col[2]/255.0, col[3]/255.0);
#else
        glColor4ubv(col);
#endif
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

class QOpenGLPaintEnginePrivate : public QPaintEnginePrivate {
    Q_DECLARE_PUBLIC(QOpenGLPaintEngine)
public:
    QOpenGLPaintEnginePrivate()
        : opacity(1)
        , has_fast_pen(false)
        , has_fast_brush(false)
        , txop(QTransform::TxNone)
        , inverseScale(1)
        , moveToCount(0)
        , shader_ctx(0)
        , grad_palette(0)
        , has_glsl(false)
        , use_stencil_method(false)
        , has_stencil_face_ext(false)
        , has_ellipse_program(false)
        , use_antialiasing(false)
        , offscreenFbo(0)
        , has_valid_offscreen_fbo(false)
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
    void fillPolygon_dev(const QPointF *polygonPoints, int pointCount,
                         const QRectF &bounds, Qt::FillRule fill);

    QPen cpen;
    QBrush cbrush;
    QRegion crgn;
    Qt::BrushStyle brush_style;
    Qt::BrushStyle pen_brush_style;
    qreal opacity;

    uint has_clipping : 1;
    uint has_pen : 1;
    uint has_brush : 1;
    uint has_fast_pen : 1;
    uint has_fast_brush : 1;

    QTransform matrix;
    GLubyte pen_color[4];
    GLubyte brush_color[4];
    QTransform::TransformationCodes txop;
    QGLDrawable drawable;

    qreal inverseScale;

    int moveToCount;
    QPointF path_start;

    void activateEllipseProgram();
    void deactivateEllipseProgram();

    void drawFastEllipse(float *vertexArray, float *texCoordArray);
    void drawOffscreenEllipse(const QRectF &rect, float *vertexArray, float *texCoordArray);
    void drawStencilEllipse(float *vertexArray, float *texCoordArray);
    void drawEllipsePen(const QRectF &rect);

    QGLContext *shader_ctx;
    GLuint grad_palette;

    GLuint radial_frag_program;
    GLuint conical_frag_program;
    GLuint ellipse_frag_program;
    GLuint ellipse_aa_frag_program;
    GLuint ellipse_aa_radial_frag_program;

    bool has_glsl;
    bool use_stencil_method;
    bool has_stencil_face_ext;
    bool has_ellipse_program;
    bool use_antialiasing;

    QGLFramebufferObject *offscreenFbo;
    QSize offscreenSize;
    QSizeF invOffscreenSize;
    bool has_valid_offscreen_fbo;

    GLuint radial_glsl_prog;
    GLuint radial_glsl_shader;

    GLuint conical_glsl_prog;
    GLuint conical_glsl_shader;

    GLuint ellipse_glsl_prog;
    GLuint ellipse_glsl_shader;

    GLuint ellipse_aa_glsl_prog;
    GLuint ellipse_aa_glsl_shader;

    GLuint radial_inv_location;
    GLuint radial_inv_mat_offset_location;
    GLuint radial_fmp_location;
    GLuint radial_fmp2_m_radius_location;
    GLuint radial_tex_location;

    GLuint conical_inv_location;
    GLuint conical_inv_mat_offset_location;
    GLuint conical_angle_location;
    GLuint conical_tex_location;

    GLuint ellipse_solid_color_location;
    GLuint ellipse_aa_solid_color_location;

    qreal max_x;
    qreal max_y;
    qreal min_x;
    qreal min_y;

    QDataBuffer<QPointF> tess_points;
    QVector<int> tess_points_stops;
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
    if (width == 0) {
        stroker.setWidth(1);
    } else
        stroker.setWidth(width);

    QPainterPath stroke = stroker.createStroke(path);
    stroke.setFillRule(Qt::WindingFill);
    return stroke;
}


#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE  0x809D
#endif

#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif

#ifndef GL_IBM_texture_mirrored_repeat
#define GL_MIRRORED_REPEAT_IBM            0x8370
#endif

#ifndef GL_SGIS_generate_mipmap
#define GL_GENERATE_MIPMAP_SGIS           0x8191
#define GL_GENERATE_MIPMAP_HINT_SGIS      0x8192
#endif

// ARB_fragment_program extension protos
#ifndef GL_FRAGMENT_PROGRAM_ARB
#define GL_FRAGMENT_PROGRAM_ARB           0x8804
#define GL_PROGRAM_FORMAT_ASCII_ARB       0x8875
#endif

// GLSL defines
#ifndef GL_FRAGMENT_SHADER
#define GL_FRAGMENT_SHADER 0x8B30
#endif
#ifndef GL_LINK_STATUS
#define GL_LINK_STATUS 0x8B82
#endif

// Stencil wrap and two-side defines
#ifndef GL_STENCIL_TEST_TWO_SIDE_EXT
#define GL_STENCIL_TEST_TWO_SIDE_EXT 0x8910
#endif
#ifndef GL_INCR_WRAP_EXT
#define GL_INCR_WRAP_EXT 0x8507
#endif
#ifndef GL_DECR_WRAP_EXT
#define GL_DECR_WRAP_EXT 0x8508
#endif
extern QGLContextPrivate *qt_glctx_get_dptr(QGLContext *);

#ifdef Q_WS_WIN
#define glProgramStringARB qt_glctx_get_dptr(ctx)->qt_glProgramStringARB
#define glBindProgramARB qt_glctx_get_dptr(ctx)->qt_glBindProgramARB
#define glDeleteProgramsARB qt_glctx_get_dptr(ctx)->qt_glDeleteProgramsARB
#define glGenProgramsARB qt_glctx_get_dptr(ctx)->qt_glGenProgramsARB
#define glProgramLocalParameter4fvARB qt_glctx_get_dptr(ctx)->qt_glProgramLocalParameter4fvARB
// GLSL definitions
#define glCreateShader qt_glctx_get_dptr(ctx)->qt_glCreateShader
#define glShaderSource qt_glctx_get_dptr(ctx)->qt_glShaderSource
#define glCompileShader qt_glctx_get_dptr(ctx)->qt_glCompileShader
#define glDeleteShader qt_glctx_get_dptr(ctx)->qt_glDeleteShader

#define glCreateProgram qt_glctx_get_dptr(ctx)->qt_glCreateProgram
#define glAttachShader qt_glctx_get_dptr(ctx)->qt_glAttachShader
#define glDetachShader qt_glctx_get_dptr(ctx)->qt_glDetachShader
#define glLinkProgram qt_glctx_get_dptr(ctx)->qt_glLinkProgram
#define glUseProgram qt_glctx_get_dptr(ctx)->qt_glUseProgram
#define glDeleteProgram qt_glctx_get_dptr(ctx)->qt_glDeleteProgram

#define glGetShaderInfoLog qt_glctx_get_dptr(ctx)->qt_glGetShaderInfoLog
#define glGetProgramiv qt_glctx_get_dptr(ctx)->qt_glGetProgramiv

#define glGetUniformLocation qt_glctx_get_dptr(ctx)->qt_glGetUniformLocation
#define glUniform4fv qt_glctx_get_dptr(ctx)->qt_glUniform4fv
#define glUniform3fv qt_glctx_get_dptr(ctx)->qt_glUniform3fv
#define glUniform2fv qt_glctx_get_dptr(ctx)->qt_glUniform2fv
#define glUniform1fv qt_glctx_get_dptr(ctx)->qt_glUniform1fv
#define glUniform1i qt_glctx_get_dptr(ctx)->qt_glUniform1i

#define glActiveStencilFaceEXT qt_glctx_get_dptr(ctx)->qt_glActiveStencilFaceEXT

#else
static _glProgramStringARB qt_glProgramStringARB = 0;
static _glBindProgramARB qt_glBindProgramARB = 0;
static _glDeleteProgramsARB qt_glDeleteProgramsARB = 0;
static _glGenProgramsARB qt_glGenProgramsARB = 0;
static _glProgramLocalParameter4fvARB qt_glProgramLocalParameter4fvARB = 0;
// GLSL definitions
static _glCreateShader qt_glCreateShader = 0;
static _glShaderSource qt_glShaderSource = 0;
static _glCompileShader qt_glCompileShader = 0;
static _glDeleteShader qt_glDeleteShader = 0;

static _glCreateProgram qt_glCreateProgram = 0;
static _glAttachShader qt_glAttachShader = 0;
static _glDetachShader qt_glDetachShader = 0;
static _glLinkProgram qt_glLinkProgram = 0;
static _glUseProgram qt_glUseProgram = 0;
static _glDeleteProgram qt_glDeleteProgram = 0;

static _glGetShaderInfoLog qt_glGetShaderInfoLog = 0;
static _glGetProgramiv qt_glGetProgramiv = 0;

static _glGetUniformLocation qt_glGetUniformLocation = 0;
static _glUniform4fv qt_glUniform4fv = 0;
static _glUniform3fv qt_glUniform3fv = 0;
static _glUniform2fv qt_glUniform2fv = 0;
static _glUniform1fv qt_glUniform1fv = 0;
static _glUniform1i qt_glUniform1i = 0;

static _glActiveStencilFaceEXT qt_glActiveStencilFaceEXT = 0;

#define glProgramStringARB qt_glProgramStringARB
#define glBindProgramARB qt_glBindProgramARB
#define glDeleteProgramsARB qt_glDeleteProgramsARB
#define glGenProgramsARB qt_glGenProgramsARB
#define glProgramLocalParameter4fvARB qt_glProgramLocalParameter4fvARB

// GLSL definitions
#define glCreateShader qt_glCreateShader
#define glShaderSource qt_glShaderSource
#define glCompileShader qt_glCompileShader
#define glDeleteShader qt_glDeleteShader

#define glCreateProgram qt_glCreateProgram
#define glAttachShader qt_glAttachShader
#define glDetachShader qt_glDetachShader
#define glLinkProgram qt_glLinkProgram
#define glUseProgram qt_glUseProgram
#define glDeleteProgram qt_glDeleteProgram

#define glGetShaderInfoLog qt_glGetShaderInfoLog
#define glGetProgramiv qt_glGetProgramiv

#define glGetUniformLocation qt_glGetUniformLocation
#define glUniform4fv qt_glUniform4fv
#define glUniform3fv qt_glUniform3fv
#define glUniform2fv qt_glUniform2fv
#define glUniform1fv qt_glUniform1fv
#define glUniform1i qt_glUniform1i

#define glActiveStencilFaceEXT qt_glActiveStencilFaceEXT

#endif // Q_WS_WIN

/*  radial fragment program
    parameter: 0 = inv_matrix
               1 = inv_matrix_offset
               2 = fmp
               4 = radius

               This program is included from radial.frag, see also src/opengl/util/README-GLSL
*/
static const char *const radial_program =
#include "util/radial.frag"

static const char *const radial_glsl_program =
#include "util/radial.glsl_quoted"

/*  conical fragment program
    parameter: 0 = inv_matrix
               1 = inv_matrix_offset
               4 = angle

               This program is included from conical.frag, see also src/opengl/util/README-GLSL
*/
static const char *const conical_program =
#include "util/conical.frag"

static const char *const conical_glsl_program =
#include "util/conical.glsl_quoted"

/*  ellipse fragment program
    parameter: 0 = solid_color

               These programs are included from ellipse.frag, see also src/opengl/util/README-GLSL
*/
static const char *const ellipse_program =
#include "util/ellipse.frag"

static const char *const ellipse_glsl_program =
#include "util/ellipse.glsl_quoted"

/*  antialiased ellipse fragment program
    parameter: 1 = solid_color

               These programs are included from ellipse_aa.frag, see also src/opengl/util/README-GLSL
*/
static const char *const ellipse_aa_program =
#include "util/ellipse_aa.frag"

static const char *const ellipse_aa_glsl_program =
#include "util/ellipse_aa.glsl_quoted"

static const char *const ellipse_aa_radial_program =
#include "util/ellipse_aa_radial.frag"

bool qt_resolve_stencil_face_extension(QGLContext *ctx)
{
    if (glActiveStencilFaceEXT != 0)
        return true;

    QGLContext cx(QGLFormat::defaultFormat());
    glActiveStencilFaceEXT = (_glActiveStencilFaceEXT) ctx->getProcAddress(QLatin1String("glActiveStencilFaceEXT"));

    return glActiveStencilFaceEXT;
}

static bool qt_resolve_frag_program_extensions(QGLContext *ctx)
{
    if (glProgramStringARB != 0)
        return true;

    // ARB_fragment_program
    glProgramStringARB = (_glProgramStringARB) ctx->getProcAddress(QLatin1String("glProgramStringARB"));
    glBindProgramARB = (_glBindProgramARB) ctx->getProcAddress(QLatin1String("glBindProgramARB"));
    glDeleteProgramsARB = (_glDeleteProgramsARB) ctx->getProcAddress(QLatin1String("glDeleteProgramsARB"));
    glGenProgramsARB = (_glGenProgramsARB) ctx->getProcAddress(QLatin1String("glGenProgramsARB"));
    glProgramLocalParameter4fvARB = (_glProgramLocalParameter4fvARB) ctx->getProcAddress(QLatin1String("glProgramLocalParameter4fvARB"));

    return glProgramStringARB
        && glBindProgramARB
        && glDeleteProgramsARB
        && glGenProgramsARB
        && glProgramLocalParameter4fvARB;
}

static bool qt_resolve_GLSL_functions(QGLContext *ctx)
{
    if (glCreateShader != 0)
        return true;

    // GLSL
    glCreateShader = (_glCreateShader) ctx->getProcAddress(QLatin1String("glCreateShader"));
    glShaderSource = (_glShaderSource) ctx->getProcAddress(QLatin1String("glShaderSource"));
    glCompileShader = (_glCompileShader) ctx->getProcAddress(QLatin1String("glCompileShader"));
    glDeleteShader = (_glDeleteShader) ctx->getProcAddress(QLatin1String("glDeleteShader"));

    glCreateProgram = (_glCreateProgram) ctx->getProcAddress(QLatin1String("glCreateProgram"));
    glAttachShader = (_glAttachShader) ctx->getProcAddress(QLatin1String("glAttachShader"));
    glDetachShader = (_glDetachShader) ctx->getProcAddress(QLatin1String("glDetachShader"));
    glLinkProgram = (_glLinkProgram) ctx->getProcAddress(QLatin1String("glLinkProgram"));
    glUseProgram = (_glUseProgram) ctx->getProcAddress(QLatin1String("glUseProgram"));
    glDeleteProgram = (_glDeleteProgram) ctx->getProcAddress(QLatin1String("glDeleteProgram"));

    glGetShaderInfoLog = (_glGetShaderInfoLog) ctx->getProcAddress(QLatin1String("glGetShaderInfoLog"));
    glGetProgramiv = (_glGetProgramiv) ctx->getProcAddress(QLatin1String("glGetProgramiv"));

    glGetUniformLocation = (_glGetUniformLocation) ctx->getProcAddress(QLatin1String("glGetUniformLocation"));
    glUniform4fv = (_glUniform4fv) ctx->getProcAddress(QLatin1String("glUniform4fv"));
    glUniform3fv = (_glUniform3fv) ctx->getProcAddress(QLatin1String("glUniform3fv"));
    glUniform2fv = (_glUniform2fv) ctx->getProcAddress(QLatin1String("glUniform2fv"));
    glUniform1fv = (_glUniform1fv) ctx->getProcAddress(QLatin1String("glUniform1fv"));
    glUniform1i = (_glUniform1i) ctx->getProcAddress(QLatin1String("glUniform1i"));

    return glCreateShader
        && glShaderSource
        && glCompileShader
        && glDeleteShader
        && glCreateProgram
        && glAttachShader
        && glDetachShader
        && glLinkProgram
        && glUseProgram
        && glDeleteProgram
        && glGetShaderInfoLog
        && glGetProgramiv
        && glGetUniformLocation
        && glUniform4fv
        && glUniform3fv
        && glUniform2fv
        && glUniform1fv
        && glUniform1i;
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

    if (!(QGLExtensions::glExtensions & QGLExtensions::GenerateMipmap))
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    else if (g.type() == QGradient::ConicalGradient || g.spread() == QGradient::PadSpread) {
        // disable mipmaps for pad gradients and conical gradients
        glTexParameteri(GL_TEXTURE_1D, GL_GENERATE_MIPMAP_SGIS, GL_FALSE);
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    } else {
        glTexParameteri(GL_TEXTURE_1D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    }
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, qt_opengl_gradient_cache()->paletteSize(),
                 0, GL_BGRA, GL_UNSIGNED_BYTE, palbuf);
#endif
}


inline void QOpenGLPaintEnginePrivate::setGradientOps(Qt::BrushStyle style)
{
#ifndef Q_WS_QWS //###
    QGL_FUNC_CONTEXT;
    if (style == Qt::LinearGradientPattern) {
        if (has_glsl)
            glUseProgram(0);
        else
            glDisable(GL_FRAGMENT_PROGRAM_ARB);
        glEnable(GL_TEXTURE_GEN_S);
        glEnable(GL_TEXTURE_1D);
    } else {
        glDisable(GL_TEXTURE_GEN_S);
        glDisable(GL_TEXTURE_1D);

        if (style == Qt::RadialGradientPattern) {
            if (has_glsl)
                glUseProgram(radial_glsl_prog);
            else {
                glEnable(GL_FRAGMENT_PROGRAM_ARB);
                glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, radial_frag_program);
            }
        } else if (style == Qt::ConicalGradientPattern) {
            if (has_glsl)
                glUseProgram(conical_glsl_prog);
            else {
                glEnable(GL_FRAGMENT_PROGRAM_ARB);
                glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, conical_frag_program);
            }
        } else {
            if (has_glsl)
                glUseProgram(0);
            else
                glDisable(GL_FRAGMENT_PROGRAM_ARB);
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

bool qt_createGLSLProgram(QGLContext *ctx, GLuint &program, const char *shader_src, GLuint &shader)
{
#ifndef Q_WS_WIN
    Q_UNUSED(ctx);
#endif
    program = glCreateProgram();
    shader = glCreateShader(GL_FRAGMENT_SHADER);
    const char *shader_prog = shader_src;
    glShaderSource(shader, 1, &shader_prog, NULL);
    glCompileShader(shader);
    glAttachShader(program, shader);
    glLinkProgram(program);
    GLint status_ok;
    glGetProgramiv(program, GL_LINK_STATUS, &status_ok);
    return status_ok;
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
    d->has_clipping = false;
    d->has_fast_pen = false;
    d->has_fast_brush = false;
    d->inverseScale = 1;
    d->opacity = 1;
    d->drawable.makeCurrent();

    QGLContext *ctx = const_cast<QGLContext *>(d->drawable.context());
    if (QGLExtensions::glExtensions & QGLExtensions::FragmentProgram)
        qt_resolve_frag_program_extensions(ctx);

    d->use_stencil_method = d->drawable.format().stencil() &&
                            QGLExtensions::glExtensions & QGLExtensions::StencilWrap;
    if (d->use_stencil_method && QGLExtensions::glExtensions & QGLExtensions::StencilTwoSide)
        d->has_stencil_face_ext = qt_resolve_stencil_face_extension(ctx);

    // disable GLSL usage for now, since it seems there are bugs in
    // some implementation regarding detection of the proper extensions
    if (0 && (QGLFormat::openGLVersionFlags() & QGLFormat::OpenGL_Version_2_0 ||
              QGLFormat::openGLVersionFlags() & QGLFormat::OpenGL_ES_Version_2_0))
        d->has_glsl = qt_resolve_GLSL_functions(ctx);

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
    glShadeModel(GL_FLAT);
    if (d->use_stencil_method) {
        glStencilFunc(GL_ALWAYS, 0, ~0);
        glClearStencil(0);
    }
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    const QColor &c = d->drawable.backgroundColor();
    glClearColor(c.redF(), c.greenF(), c.blueF(), 1.0);
    if (d->drawable.autoFillBackground()) {
        GLbitfield clearBits = GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT;
#ifndef Q_WS_QWS
        clearBits |= GL_ACCUM_BUFFER_BIT;
#endif
        glClear(clearBits);
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
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    if (QGLExtensions::glExtensions & QGLExtensions::FramebufferObject) {
#ifndef Q_WS_QWS
        if (!d->offscreenFbo || d->offscreenSize != sz || d->drawable.context() != d->shader_ctx) {
            delete d->offscreenFbo;
            d->offscreenFbo = new QGLFramebufferObject(sz.width(), sz.height());

            if (!d->offscreenFbo->isValid())
                DEBUG_ONCE qDebug() << "QOpenGLPaintEngine: Invalid fbo," << "old size was" << d->offscreenSize << ", new size is" << sz;

            d->offscreenSize = sz;
            d->invOffscreenSize = QSizeF(1.0 / sz.width(), 1.0 / sz.height());
        }

        d->has_valid_offscreen_fbo = d->offscreenFbo->isValid();

        if (!d->has_valid_offscreen_fbo)
            qWarning() << "QOpenGLPaintEngine: Unable to create valid framebuffer object.";
#endif
    }

    if ((QGLExtensions::glExtensions & QGLExtensions::MirroredRepeat)
        && d->drawable.context() != d->shader_ctx
        && !qgl_share_reg()->checkSharing(d->drawable.context(), d->shader_ctx))
    {
#ifndef Q_WS_QWS
        if (d->shader_ctx) {
            glBindTexture(GL_TEXTURE_1D, 0);
            glDeleteTextures(1, &d->grad_palette);

            if (d->has_glsl) {
                glDeleteShader(d->radial_glsl_shader);
                glDeleteProgram(d->radial_glsl_prog);
                glDeleteShader(d->conical_glsl_shader);
                glDeleteProgram(d->conical_glsl_prog);
                glDeleteShader(d->ellipse_glsl_shader);
                glDeleteProgram(d->ellipse_glsl_prog);
                glDeleteShader(d->ellipse_aa_glsl_shader);
                glDeleteProgram(d->ellipse_aa_glsl_prog);
            } else if (QGLExtensions::glExtensions & QGLExtensions::FragmentProgram) {
                glDeleteProgramsARB(1, &d->radial_frag_program);
                glDeleteProgramsARB(1, &d->conical_frag_program);
                glDeleteProgramsARB(1, &d->ellipse_frag_program);
                glDeleteProgramsARB(1, &d->ellipse_aa_frag_program);
                glDeleteProgramsARB(1, &d->ellipse_aa_radial_frag_program);
            }

            d->has_ellipse_program = false;
        }
        d->shader_ctx = d->drawable.context();
        gccaps |= LinearGradientFill;
        glGenTextures(1, &d->grad_palette);

        if (d->has_glsl) {
            if (qt_createGLSLProgram(ctx, d->radial_glsl_prog, radial_glsl_program, d->radial_glsl_shader))
                gccaps |= RadialGradientFill;
            else
                qWarning() << "QOpenGLPaintEngine: Unable to use radial gradient GLSL fragment shader.";

            GLuint prog = d->radial_glsl_prog;
            glUseProgram(prog);
            d->radial_inv_location = glGetUniformLocation(prog, "inv_matrix");
            d->radial_inv_mat_offset_location = glGetUniformLocation(prog, "inv_matrix_offset");
            d->radial_fmp_location = glGetUniformLocation(prog, "fmp");
            d->radial_fmp2_m_radius_location = glGetUniformLocation(prog, "fmp2_m_radius2");
            d->radial_tex_location = glGetUniformLocation(prog, "palette");
            glUseProgram(0);

            if (qt_createGLSLProgram(ctx, d->conical_glsl_prog, conical_glsl_program, d->conical_glsl_shader))
                gccaps |= ConicalGradientFill;
            else
                qWarning() << "QOpenGLPaintEngine: Unable to use conical gradient GLSL fragment shader.";

            prog = d->conical_glsl_prog;
            glUseProgram(prog);
            d->conical_inv_location = glGetUniformLocation(prog, "inv_matrix");
            d->conical_inv_mat_offset_location = glGetUniformLocation(prog, "inv_matrix_offset");
            d->conical_angle_location = glGetUniformLocation(prog, "angle");
            d->conical_tex_location = glGetUniformLocation(prog, "palette");
            glUseProgram(0);

            if (  qt_createGLSLProgram(ctx, d->ellipse_glsl_prog, ellipse_glsl_program, d->ellipse_glsl_shader)
               && qt_createGLSLProgram(ctx, d->ellipse_aa_glsl_prog, ellipse_aa_glsl_program, d->ellipse_aa_glsl_shader))
                d->has_ellipse_program = true;
            else
                qWarning() << "QOpenGLPaintEngine: Unable to use ellipse GLSL fragment shader.";

            prog = d->ellipse_glsl_prog;
            glUseProgram(prog);
            d->ellipse_solid_color_location = glGetUniformLocation(prog, "solid_color");
            glUseProgram(0);

            prog = d->ellipse_aa_glsl_prog;
            glUseProgram(prog);
            d->ellipse_aa_solid_color_location = glGetUniformLocation(prog, "solid_color");
            glUseProgram(0);

        } else if (QGLExtensions::glExtensions & QGLExtensions::FragmentProgram) {
            glGenProgramsARB(1, &d->radial_frag_program);
            glGenProgramsARB(1, &d->conical_frag_program);

            while (glGetError() != GL_NO_ERROR) {} // reset the error state
            glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, d->radial_frag_program);
            glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB,
                               strlen(radial_program), (const GLbyte *) radial_program);
            if (glGetError() == GL_NO_ERROR)
                gccaps |= RadialGradientFill;
            else
                qWarning() << "QOpenGLPaintEngine: Unable to use radial gradient fragment shader.";

            glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, d->conical_frag_program);
            glProgramStringARB(GL_FRAGMENT_PROGRAM_ARB, GL_PROGRAM_FORMAT_ASCII_ARB,
                               strlen(conical_program), (const GLbyte *) conical_program);
            if (glGetError() == GL_NO_ERROR)
                gccaps |= ConicalGradientFill;
            else
                qWarning() << "QOpenGLPaintEngine: Unable to use conical gradient fragment shader.";

            if (  qt_createFragmentProgram(ctx, d->ellipse_frag_program, ellipse_program)
               && qt_createFragmentProgram(ctx, d->ellipse_aa_frag_program, ellipse_aa_program)
               && qt_createFragmentProgram(ctx, d->ellipse_aa_radial_frag_program, ellipse_aa_radial_program))
                d->has_ellipse_program = true;
            else
                qWarning() << "QOpenGLPaintEngine: Unable to use ellipse fragment shader.";
        }
#endif
    }

    setDirty(QPaintEngine::DirtyPen);
    setDirty(QPaintEngine::DirtyBrush);
    return true;
}

bool QOpenGLPaintEngine::end()
{
    Q_D(QOpenGLPaintEngine);
    QGL_D_FUNC_CONTEXT;
#ifndef Q_WS_QWS
    glPopAttrib();
#endif
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    if (d->has_glsl)
        glUseProgram(0); // GLSL program state is not part of GL_ALL_ATTRIB_BITS
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
    bool update_fast_brush = false;

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
    }

    if (flags & DirtyPen) {
        update_fast_pen = true;
        updatePen(state.pen());
    }

    if (flags & DirtyBrush) {
        updateBrush(state.brush(), state.brushOrigin());
        update_fast_brush = true;
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
            (pen_width == 0 || (pen_width == 1 && d->txop <= QTransform::TxTranslate))
            && d->cpen.style() == Qt::SolidLine
            && d->cpen.isSolid();
    }

    if (update_fast_brush) {
        Q_D(QOpenGLPaintEngine);
        d->has_fast_brush =
            (d->has_brush
            && (d->brush_style == Qt::SolidPattern
               || d->brush_style == Qt::RadialGradientPattern && d->use_antialiasing));
    }
}

void QOpenGLPaintEnginePrivate::updateGradient(const QBrush &brush)
{
#ifndef Q_WS_QWS
    bool has_mirrored_repeat = QGLExtensions::glExtensions & QGLExtensions::MirroredRepeat;
    bool has_frag_program = QGLExtensions::glExtensions & QGLExtensions::FragmentProgram;
    Qt::BrushStyle style = brush.style();
    QGL_FUNC_CONTEXT;

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

        glBindTexture(GL_TEXTURE_1D, grad_palette);
        createGradientPaletteTexture(*brush.gradient());
    } else if (has_glsl || has_frag_program) {
        if (style == Qt::RadialGradientPattern) {
            const QRadialGradient *g = static_cast<const QRadialGradient *>(brush.gradient());
            QTransform translate(1, 0, 0, 1, -g->focalPoint().x(), -g->focalPoint().y());
            QTransform gl_to_qt(1, 0, 0, -1, 0, pdev->height());
            QTransform inv_matrix = gl_to_qt * matrix.inverted() * brush.transform().inverted() * translate;

            float pt[4] = {inv_matrix.dx(), inv_matrix.dy(), 0.f, 0.f};
            float inv[4] = {inv_matrix.m11(), inv_matrix.m21(),
                            inv_matrix.m12(), inv_matrix.m22()};
            float pt1[4] = {g->center().x() - g->focalPoint().x(),
                             g->center().y() - g->focalPoint().y(), 0.f, 0.f};
            float f[4] = {-pt1[0]*pt1[0] - pt1[1]*pt1[1] + g->radius()*g->radius(), 0.f, 0.f, 0.f};

            if (has_glsl) {
                glUseProgram(radial_glsl_prog);
                glUniform4fv(radial_inv_location, 1, inv);
                glUniform2fv(radial_inv_mat_offset_location, 1, pt);
                glUniform2fv(radial_fmp_location, 1, pt1);
                glUniform1fv(radial_fmp2_m_radius_location, 1, f);
                glUniform1i(radial_tex_location, 0);
            } else {
                glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, radial_frag_program);
                glProgramLocalParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, 0, inv); // inv_matrix
                glProgramLocalParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, 1, pt); // inv_matrix_offset
                glProgramLocalParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, 2, pt1); // fmp
                glProgramLocalParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, 4, f); // fmp2_m_radius2

                if (has_ellipse_program) {
                    glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, ellipse_aa_radial_frag_program);
                    glProgramLocalParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, 0, inv); // inv_matrix
                    glProgramLocalParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, 1, pt); // inv_matrix_offset
                    glProgramLocalParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, 2, pt1); // fmp
                    glProgramLocalParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, 4, f); // fmp2_m_radius2
                }
            }
            glBindTexture(GL_TEXTURE_1D, grad_palette);
            createGradientPaletteTexture(*brush.gradient());
        } else if (style == Qt::ConicalGradientPattern) {
            const QConicalGradient *g = static_cast<const QConicalGradient *>(brush.gradient());
            QTransform translate(1, 0, 0, 1, -g->center().x(), -g->center().y());
            QTransform gl_to_qt(1, 0, 0, -1, 0, pdev->height());
            QTransform inv_matrix = gl_to_qt * matrix.inverted() * brush.transform().inverted() * translate;


            float pt[4] = {inv_matrix.dx(), inv_matrix.dy(), 0.f, 0.f};
            float inv[4] = {inv_matrix.m11(), inv_matrix.m21(),
                            inv_matrix.m12(), inv_matrix.m22()};
            float angle[4] = {-(g->angle() * 2 * Q_PI) / 360.0, 0.f, 0.f, 0.f};

            if (has_glsl) {
                glUseProgram(conical_glsl_prog);
                glUniform4fv(conical_inv_location, 1, inv);
                glUniform2fv(conical_inv_mat_offset_location, 1, pt);
                glUniform1fv(conical_angle_location, 1, angle);
                glUniform1i(radial_tex_location, 0);
            } else {
                glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, conical_frag_program);
                glProgramLocalParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, 0, inv); // inv_matrix
                glProgramLocalParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, 1, pt); // inv_matrix_offset
                glProgramLocalParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, 4, angle); // angle
            }
            glBindTexture(GL_TEXTURE_1D, grad_palette);
            createGradientPaletteTexture(*brush.gradient());
        }
    }

    glDisable(GL_TEXTURE_1D);
    glDisable(GL_TEXTURE_GEN_S);
    if (has_glsl)
        glUseProgram(0);
    else
        glDisable(GL_FRAGMENT_PROGRAM_ARB);
#endif
}


class QOpenGLTessellator : public QTessellator
{
public:
    QOpenGLTessellator() : vertices(0), allocated(0), size(0) {}
    ~QOpenGLTessellator() { free(vertices); }
    float *vertices;
    int allocated;
    int size;
    void addTrap(const Trapezoid &trap);
    QRect tessellate(const QPointF *points, int nPoints, bool winding) {
        size = 0;
        setWinding(winding);
        return QTessellator::tessellate(points, nPoints).toRect();
    }
    void done() {
        if (allocated > 512) {
            free(vertices);
            vertices = 0;
            allocated = 0;
        }
    }
};

void QOpenGLTessellator::addTrap(const Trapezoid &trap)
{
    if (size > allocated - 12) {
        allocated = qMax(2*allocated, 512);
        vertices = (float *)realloc(vertices, allocated * sizeof(float));
    }

    float top = Q27Dot5ToDouble(trap.top);
    float bottom = Q27Dot5ToDouble(trap.bottom);

    Q27Dot5 y = trap.topLeft->y - trap.bottomLeft->y;
    if (!y)
        return;
    qreal m = (-Q27Dot5ToDouble(trap.topLeft->x) + Q27Dot5ToDouble(trap.bottomLeft->x)) / Q27Dot5ToDouble(y);
    qreal tx = Q27Dot5ToDouble(trap.topLeft->x);
    qreal topLeftX
        = tx + m * (Q27Dot5ToDouble(trap.topLeft->y) - Q27Dot5ToDouble(trap.top));
    qreal bottomLeftX
        = tx + m * (Q27Dot5ToDouble(trap.topLeft->y) - Q27Dot5ToDouble(trap.bottom));
//     qDebug() << "trap: top=" << Q27Dot5ToDouble(trap.top)
//              << "bottom=" << Q27Dot5ToDouble(trap.bottom);
//     qDebug() << "      topLeft=" << Q27Dot5ToDouble(trap.topLeft->x) << Q27Dot5ToDouble(trap.topLeft->y);
//     qDebug() << "      bottomLeft=" << Q27Dot5ToDouble(trap.bottomLeft->x) << Q27Dot5ToDouble(trap.bottomLeft->y);

//     qDebug() << " -> m=" << m << "tx=" << tx;
//     qDebug() << " -> topLeftX" << topLeftX << "bottomLeftX" << bottomLeftX;

    y = trap.topRight->y - trap.bottomRight->y;
    if (!y)
        return;
    m = (Q27Dot5ToDouble(-trap.topRight->x) + Q27Dot5ToDouble(trap.bottomRight->x)) / Q27Dot5ToDouble(y);
    tx = Q27Dot5ToDouble(trap.topRight->x);
    qreal topRightX
        = tx + m * (Q27Dot5ToDouble(trap.topRight->y) - Q27Dot5ToDouble(trap.top));
    qreal bottomRightX
        = tx + m * (Q27Dot5ToDouble(trap.topRight->y) - Q27Dot5ToDouble(trap.bottom));

    vertices[size++] = topLeftX;
    vertices[size++] = top;
    vertices[size++] = topRightX;
    vertices[size++] = top;
    vertices[size++] = bottomLeftX;
    vertices[size++] = bottom;

    vertices[size++] = bottomLeftX;
    vertices[size++] = bottom;
    vertices[size++] = topRightX;
    vertices[size++] = top;
    vertices[size++] = bottomRightX;
    vertices[size++] = bottom;
}

void QOpenGLPaintEnginePrivate::fillPolygon_dev(const QPointF *polygonPoints, int pointCount,
                                                const QRectF &bounds, Qt::FillRule fill)
{
    Q_UNUSED(bounds);
    QOpenGLTessellator tessellator;
    tessellator.tessellate(polygonPoints, pointCount, fill == Qt::WindingFill);

    glVertexPointer(2, GL_FLOAT, 0, tessellator.vertices);
    glEnableClientState(GL_VERTEX_ARRAY);
    glDrawArrays(GL_TRIANGLES, 0, tessellator.size/2);
    glDisableClientState(GL_VERTEX_ARRAY);
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
            glClear(GL_STENCIL_BUFFER_BIT);
            drawVertexArrays();

            glCullFace(GL_FRONT);
            glStencilOp(GL_KEEP, GL_KEEP, GL_DECR_WRAP_EXT);
            drawVertexArrays();

            glDisable(GL_CULL_FACE);
        }
    }

    // Enable stencil func.
    glStencilFunc(GL_NOTEQUAL, 0, stencilMask);

    // Enable color writes.
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glStencilMask(0);

    glBegin(GL_QUADS);
    glVertex2f(min_x, min_y);
    glVertex2f(max_x, min_y);
    glVertex2f(max_x, max_y);
    glVertex2f(min_x, max_y);
    glEnd();

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

    if (use_stencil_method) {
        pathToVertexArrays(path);
        fillVertexArray(path.fillRule());
        return;
    }

    QList<QPolygonF> polys = path.toFillPolygons(matrix);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDepthMask(GL_TRUE);
    glStencilMask(~0);

    glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    for (int i = 0; i < polys.size(); ++i) {
        const QPolygonF &poly = polys.at(i);
        fillPolygon_dev(poly.data(), poly.count(),
                        poly.boundingRect(),
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

    if (d->pen_brush_style >= Qt::LinearGradientPattern
        && d->pen_brush_style <= Qt::ConicalGradientPattern) {
        d->updateGradient(pen.brush());
        d->setGLPen(Qt::white);
    } else {
        d->setGLPen(pen.color());
        qt_glColor4ubv(d->pen_color);
    }
}

void QOpenGLPaintEngine::updateBrush(const QBrush &brush, const QPointF &)
{
    Q_D(QOpenGLPaintEngine);
    d->cbrush = brush;
    d->brush_style = brush.style();
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
    if (!(QGLExtensions::glExtensions & QGLExtensions::SampleBuffers))
        return;

    Q_D(QOpenGLPaintEngine);

    d->use_antialiasing = hints & QPainter::Antialiasing;

    if (d->use_antialiasing)
        glEnable(GL_MULTISAMPLE);
    else
        glDisable(GL_MULTISAMPLE);
}

void QOpenGLPaintEngine::updateCompositionMode(QPainter::CompositionMode composition_mode)
{
    switch(composition_mode) {
    case QPainter::CompositionMode_DestinationOver:
        glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_ONE);
        break;
    case QPainter::CompositionMode_Clear:
        glBlendFunc(GL_ZERO, GL_ZERO);
        break;
    case QPainter::CompositionMode_Source:
        glBlendFunc(GL_ONE, GL_ZERO);
        break;
    case QPainter::CompositionMode_Destination:
        glBlendFunc(GL_ZERO, GL_ONE);
        break;
    case QPainter::CompositionMode_SourceIn:
        glBlendFunc(GL_DST_ALPHA, GL_ZERO);
        break;
    case QPainter::CompositionMode_DestinationIn:
        glBlendFunc(GL_ZERO, GL_SRC_ALPHA);
        break;
    case QPainter::CompositionMode_SourceOut:
        glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_ZERO);
        break;
    case QPainter::CompositionMode_DestinationOut:
        glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);
        break;
    case QPainter::CompositionMode_SourceAtop:
        glBlendFunc(GL_DST_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        break;
    case QPainter::CompositionMode_DestinationAtop:
        glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_SRC_ALPHA);
        break;
    case QPainter::CompositionMode_Xor:
        glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        break;
    case QPainter::CompositionMode_SourceOver:
    default:
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    }
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

void QOpenGLPaintEngine::drawRects(const QRectF *rects, int rectCount)
{
    Q_D(QOpenGLPaintEngine);

    for (int i=0; i<rectCount; ++i) {
        QRectF r = rects[i];

        qreal left = r.left();
        qreal right = r.right();
        qreal top = r.top();
        qreal bottom = r.bottom();

        float vertexArray[10];
        qt_add_rect_to_array(r, vertexArray);

        if (d->has_brush) {
            d->setGradientOps(d->brush_style);
            qt_glColor4ubv(d->brush_color);
            glVertexPointer(2, GL_FLOAT, 0, vertexArray);
            glEnableClientState(GL_VERTEX_ARRAY);
            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
            glDisableClientState(GL_VERTEX_ARRAY);
        }

        if (d->has_pen) {
            d->setGradientOps(d->pen_brush_style);
            qt_glColor4ubv(d->pen_color);
            if (d->has_fast_pen) {
                vertexArray[8] = vertexArray[0];
                vertexArray[9] = vertexArray[1];

                glVertexPointer(2, GL_FLOAT, 0, vertexArray);
                glEnableClientState(GL_VERTEX_ARRAY);
                glDrawArrays(GL_LINE_STRIP, 0, 5);
                glDisableClientState(GL_VERTEX_ARRAY);
            } else {
                if (d->use_stencil_method)
                {
                    QPainterPath path; path.setFillRule(Qt::WindingFill);
                    path.moveTo(left, top);
                    path.lineTo(right, top);
                    path.lineTo(right, bottom);
                    path.lineTo(left, bottom);
                    path.lineTo(left, top);
                    QPainterPath stroke = qt_opengl_stroke_cache()->getStrokedPath(path, d->cpen);
                    d->fillPath(stroke);
                } else {
                QPainterPath path; path.setFillRule(Qt::WindingFill);
                path.moveTo(left, top);
                path.lineTo(right, top);
                path.lineTo(right, bottom);
                path.lineTo(left, bottom);
                path.lineTo(left, top);
                QPainterPath stroke = strokeForPath(path, d->cpen);
                d->fillPath(stroke);
                }
            }
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
        if (d->has_fast_pen) {
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
                if (d->use_stencil_method) {
                    QPainterPath path; path.setFillRule(Qt::WindingFill);
                    path.moveTo(l.x1(), l.y1());
                    path.lineTo(l.x2(), l.y2());
                    QPainterPath stroke = strokeForPath(path, d->cpen);
                    d->fillPath(stroke);
                } else {
                QPainterPath path; path.setFillRule(Qt::WindingFill);
                path.moveTo(l.x1(), l.y1());
                path.lineTo(l.x2(), l.y2());
                QPainterPath stroke = strokeForPath(path, d->cpen);
                d->fillPath(stroke);
                }
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
        if (mode == ConvexMode) {

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
            if (d->use_stencil_method) {
                QPainterPath path;
                path.setFillRule(mode == WindingMode ? Qt::WindingFill : Qt::OddEvenFill);
                path.moveTo(points[0]);

                for (int i=1; i<pointCount; ++i)
                    path.lineTo(points[i]);
                //path.closeSubpath();
                d->fillPath(path);
            } else {
            QPainterPath path;
            path.setFillRule(mode == WindingMode ? Qt::WindingFill : Qt::OddEvenFill);
            path.moveTo(points[0]);
            for (int i=1; i<pointCount; ++i)
                path.lineTo(points[i]);
            //path.closeSubpath();
            d->fillPath(path);
            }
        }
    }

    if (d->has_pen) {
        d->setGradientOps(d->pen_brush_style);
        qt_glColor4ubv(d->pen_color);
        if (d->has_fast_pen) {
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
            if (d->use_stencil_method) {
                QPainterPath path(points[0]);
                for (int i = 1; i < pointCount; ++i)
                    path.lineTo(points[i]);
                QPainterPath stroke = qt_opengl_stroke_cache()->getStrokedPath(path, d->cpen);
                d->fillPath(stroke);
            } else {
            QPainterPath path(points[0]);
            for (int i = 1; i < pointCount; ++i)
                path.lineTo(points[i]);

            QPainterPath stroke = strokeForPath(path, d->cpen);
            if (stroke.isEmpty())
                return;
            d->fillPath(stroke);
            }
        }
    }
}

void QOpenGLPaintEngine::drawPath(const QPainterPath &path)
{
    Q_D(QOpenGLPaintEngine);

    if (path.isEmpty())
        return;

    if (d->has_brush) {
        d->setGradientOps(d->brush_style);
        qt_glColor4ubv(d->brush_color);
            d->fillPath(path);
    }
    if (d->has_pen) {
        qt_glColor4ubv(d->pen_color);
        d->setGradientOps(d->pen_brush_style);
        if (d->has_fast_pen) {
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

                    qreal inverseScaleHalf = d->inverseScale / 2;
                    beziers[0] = QBezier::fromPoints(sp, e, cp2, ep);
                    QBezier *b = beziers;
                    while (b >= beziers) {
                        // check if we can pop the top bezier curve from the stack
                        qreal l = qAbs(b->x4 - b->x1) + qAbs(b->y4 - b->y1);
                        qreal d;
                        if (l > d_func()->inverseScale) {
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
        } else {
            if (d->use_stencil_method) {
                QPainterPath stroke = qt_opengl_stroke_cache()->getStrokedPath(path, d->cpen);
                d->fillPath(stroke);
            } else {
                QPainterPath npath = strokeForPath(path, d->cpen);
                d->fillPath(npath);
            }
        }
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
//                  << hex << "tex id:" << font_tex->texture << "key:" << font_key;
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


void QOpenGLPaintEnginePrivate::activateEllipseProgram()
{
#ifndef Q_WS_QWS
    QGL_FUNC_CONTEXT;
    static const float inv = 1.0f / 255.0f;

    if (brush_style == Qt::SolidPattern || !has_fast_brush) {
        float solid_color[4];

        if (has_fast_brush) {
            const GLubyte *col = brush_color;

            for (int i = 0; i < 4; ++i)
                solid_color[i] = col[i] * inv;
        } else {
            for (int i = 0; i < 4; ++i)
                solid_color[i] = 1.0f;
        }

        if (use_antialiasing) {
            if (has_glsl) {
                glUseProgram(ellipse_aa_glsl_prog);
                glUniform4fv(ellipse_aa_solid_color_location, 1, solid_color);
            } else {
                glEnable(GL_FRAGMENT_PROGRAM_ARB);
                glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, ellipse_aa_frag_program);
                glProgramLocalParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, 1, solid_color);
            }
        } else {
            if (has_glsl) {
                glUseProgram(ellipse_glsl_prog);
                glUniform4fv(ellipse_solid_color_location, 1, solid_color);
            } else {
                glEnable(GL_FRAGMENT_PROGRAM_ARB);
                glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, ellipse_frag_program);
                glProgramLocalParameter4fvARB(GL_FRAGMENT_PROGRAM_ARB, 0, solid_color);
            }
        }
    } else if (brush_style == Qt::RadialGradientPattern) {
        glEnable(GL_FRAGMENT_PROGRAM_ARB);
        glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, ellipse_aa_radial_frag_program);

        DEBUG_ONCE_STR("QOpenGLPaintEnginePrivate: Using fast radial program");
    } else {
        Q_ASSERT(false);
    }
#endif
}


void QOpenGLPaintEnginePrivate::deactivateEllipseProgram()
{
    setGradientOps(Qt::NoBrush);
}


void QOpenGLPaintEnginePrivate::drawFastEllipse(float *vertexArray, float *texCoordArray)
{
#ifndef Q_WS_QWS
    Q_ASSERT(has_ellipse_program && has_brush && has_fast_brush);

    if (use_antialiasing) {
        DEBUG_ONCE_STR("QOpenGLPainterPrivate::drawFastEllipse(): Drawing fast antialiased ellipse");

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    } else
        DEBUG_ONCE_STR("QOpenGLPainterPrivate::drawFastEllipse(): Drawing fast aliased ellipse");

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glVertexPointer(2, GL_FLOAT, 0, vertexArray);
    glTexCoordPointer(2, GL_FLOAT, 0, texCoordArray);

    activateEllipseProgram();
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    deactivateEllipseProgram();

    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);

    if (use_antialiasing)
        glDisable(GL_BLEND);
#endif
}


void QOpenGLPaintEnginePrivate::drawOffscreenEllipse(const QRectF &rect, float *vertexArray, float *texCoordArray)
{
#ifndef Q_WS_QWS
    Q_ASSERT(has_ellipse_program && has_brush);

    DEBUG_ONCE_STR("QOpenGLPainter: Drawing ellipse using offscreen buffer");

    glVertexPointer(2, GL_FLOAT, 0, vertexArray);
    glTexCoordPointer(2, GL_FLOAT, 0, texCoordArray);

    glEnableClientState(GL_VERTEX_ARRAY);

    setGradientOps(brush_style);
    qt_glColor4ubv(brush_color);

    offscreenFbo->bind();
    // draw the brush to the offscreen
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glEnable(GL_BLEND);
    glBlendFunc(GL_ZERO, GL_SRC_ALPHA);

    setGradientOps(Qt::NoBrush);

    activateEllipseProgram();
    // mask out the coverage in the offscreen
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    deactivateEllipseProgram();

    offscreenFbo->release();

    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    QRectF slimmed = rect.adjusted(1, 1, -1, -1);

    qt_add_rect_to_array(slimmed, vertexArray);

    for (int i = 1; i < 8; i += 2) {
        QPointF mapped = matrix.map(QPointF(vertexArray[i & ~1], vertexArray[i]));

        mapped.setX(mapped.x() * invOffscreenSize.width());
        mapped.setY(1 - mapped.y() * invOffscreenSize.height());

        texCoordArray[i & ~1] = mapped.x();
        texCoordArray[i] = mapped.y();
    }

    glColor4f(1, 1, 1, 1);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, offscreenFbo->texture());
    // draw the result to the screen
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glDisable(GL_TEXTURE_2D);

    glDisable(GL_BLEND);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
#endif
}


void QOpenGLPaintEnginePrivate::drawStencilEllipse(float *vertexArray, float *texCoordArray)
{
#ifndef Q_WS_QWS
    Q_ASSERT(has_ellipse_program && has_brush);

    DEBUG_ONCE_STR("QOpenGLPainter: Drawing ellipse using stencil buffer");

    // initialize the stencil buffer
    glDepthMask(GL_FALSE);

    glEnable(GL_STENCIL_TEST);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glStencilMask(1);
    glStencilOp(GL_KEEP, GL_KEEP, GL_INVERT);
    glStencilFunc(GL_ALWAYS, 0, ~0);

    // draw the coverage to the stencil buffer
    glVertexPointer(2, GL_FLOAT, 0, vertexArray);
    glTexCoordPointer(2, GL_FLOAT, 0, texCoordArray);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    activateEllipseProgram();
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    deactivateEllipseProgram();

    glDisable(GL_TEXTURE_COORD_ARRAY);

    // now draw the brush using the stencil buffer as mask
    glStencilFunc(GL_NOTEQUAL, 0, 1);
    glStencilMask(0);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

    glDepthMask(GL_TRUE);

    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    setGradientOps(brush_style);
    qt_glColor4ubv(brush_color);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    // finally, clear the stencil buffer
    glStencilMask(~0);
    glStencilFunc(GL_ALWAYS, 0, 0);
    glStencilOp(GL_ZERO, GL_ZERO, GL_ZERO);

    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    glDisable(GL_STENCIL_TEST);
    glDisableClientState(GL_VERTEX_ARRAY);
#endif
}


void QOpenGLPaintEnginePrivate::drawEllipsePen(const QRectF &rect)
{
#ifndef Q_WS_QWS
    Q_ASSERT(has_pen);

    setGradientOps(pen_brush_style);
    qt_glColor4ubv(pen_color);

    QPainterPath path;
    path.addEllipse(rect);

    QPolygonF polygon = path.toFillPolygon();
    const int pointCount = polygon.size();

    const QPointF *points = polygon.data();

    if (has_fast_pen) {
        QVarLengthArray<float> vertexArray((pointCount + 1) * 2);
        glVertexPointer(2, GL_FLOAT, 0, vertexArray.data());

        for (int i = 0; i < pointCount; ++i) {
            vertexArray[i * 2] = points[i].x();
            vertexArray[i * 2 + 1] = points[i].y();
        }

        vertexArray[pointCount * 2] = points[0].x();
        vertexArray[pointCount * 2 + 1] = points[0].y();

        glEnableClientState(GL_VERTEX_ARRAY);
        glDrawArrays(GL_LINE_STRIP, 0, pointCount + 1);
        glDisableClientState(GL_VERTEX_ARRAY);
    } else {
            QPainterPath path(points[0]);

            for (int i = 1; i < pointCount; ++i)
                path.lineTo(points[i]);

        QPainterPath stroke = strokeForPath(path, cpen);

            if (stroke.isEmpty())
                return;

            fillPath(stroke);
    }
#endif
}


void QOpenGLPaintEngine::drawEllipse(const QRectF &rect)
{
#ifndef Q_WS_QWS
    Q_D(QOpenGLPaintEngine);

    DEBUG_ONCE_STR(d->has_glsl ? "QOpenGLPainter: Using GLSL" : "QOpenGLPainter: Using fragment programs");

    bool can_draw =
        (  d->has_ellipse_program
        && d->has_brush
        && (  d->has_fast_brush
           || d->has_valid_offscreen_fbo
           || !d->use_antialiasing));

    if (can_draw) {
        int grow = d->use_antialiasing ? 4 : 0;

        float vertexArray[4 * 2];
        float texCoordArray[4 * 2];

        QRectF boundingRect = grow ? rect.adjusted(-grow, -grow, grow, grow) : rect;
        qt_add_rect_to_array(boundingRect, vertexArray);

        if (grow) {
            float wfactor = 2 * grow / float(rect.width());
            float hfactor = 2 * grow / float(rect.height());

            qt_add_texcoords_to_array(-1.0 - wfactor, -1.0 - hfactor, 1.0 + wfactor, 1.0 + hfactor, texCoordArray);
        } else {
            qt_add_texcoords_to_array(-1.0, -1.0, 1.0, 1.0, texCoordArray);
        }

        if (d->has_fast_brush)
            d->drawFastEllipse(vertexArray, texCoordArray);
        else if (d->use_antialiasing)
            d->drawOffscreenEllipse(boundingRect, vertexArray, texCoordArray);
        else
            d->drawStencilEllipse(vertexArray, texCoordArray);

        if (d->has_pen)
            d->drawEllipsePen(rect);
    } else {
        DEBUG_ONCE_STR("QOpenGLPaintEngine: Falling back to QPaintEngine::drawEllipse()");

        QPaintEngine::drawEllipse(rect);
    }
#else
    QPaintEngine::drawEllipse(rect);
#endif
}

#include "qpaintengine_opengl.moc"
