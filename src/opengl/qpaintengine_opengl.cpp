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
#include "qpen.h"
#include "qvarlengtharray.h"
#include <private/qpainter_p.h>
#include <qglpixelbuffer.h>
#include <private/qglpixelbuffer_p.h>
#include <private/qstroker_p.h>
#include <private/qbezier_p.h>
#include <qglframebufferobject.h>

#ifdef Q_WS_WIN
#define QGL_FUNC_CONTEXT QGLContext *ctx = const_cast<QGLContext *>(drawable.context());
#else
#define QGL_FUNC_CONTEXT
#endif

#ifdef Q_OS_MAC
# include <OpenGL/glu.h>
#else
# include <GL/glu.h>
#endif
#include <stdlib.h>

struct QSubpath
{
    int start, end;
    qreal left, right, top, bottom;
    bool processed;
    bool added;


    inline void unite(qreal x, qreal y) {
        if (x < left) left = x;
        else if (x > right) right = x;
        if (y > bottom) bottom = y;
        else if (y < top) top = y;
    }

    inline bool intersects(const QSubpath &s) const
    {
        return (qMax(left, s.left) <= qMin(right, s.right) &&
                qMax(top, s.top) <= qMin(bottom, s.bottom));
    }
};

QDebug operator<<(QDebug s, const QSubpath &p)
{
    s << "Subpath [" << p.start << "-" << p.end << "]"
      << "left:" << p.left
      << "right:" << p.right
      << "top:" << p.top
      << "bottom:" << p.bottom
      << p.processed << p.added;
    return s;
}

#define QREAL_MAX 9e100
#define QREAL_MIN -9e100

void qt_painterpath_split(const QPainterPath &path, QDataBuffer<int> *paths, QDataBuffer<QSubpath> *subpaths)
{
//     qDebug() << "\nqt_painterpath_split";

    // reset the old buffers...
    paths->reset();
    subpaths->reset();

    // Create the set of current subpaths...
    {
        QSubpath *current = 0;
        for (int i=0; i<path.elementCount(); ++i) {
            const QPainterPath::Element &e = path.elementAt(i);
            switch (e.type) {
            case QPainterPath::MoveToElement: {
                if (current)
                    current->end = i-1;
                QSubpath sp = { i, 0, QREAL_MAX, QREAL_MIN, QREAL_MAX, QREAL_MIN, false, false };
                sp.unite(e.x, e.y);
                subpaths->add(sp);
                current = &subpaths->data()[subpaths->size() - 1];
                break;
            }
            case QPainterPath::LineToElement:
                Q_ASSERT(current);
                current->unite(e.x, e.y);
                break;
            case QPainterPath::CurveToElement: {
                const QPainterPath::Element &cp2 = path.elementAt(i+1);
                const QPainterPath::Element &ep = path.elementAt(i+2);
                Q_ASSERT(current);
                current->unite(e.x, e.y);
                current->unite(cp2.x, cp2.y);
                current->unite(ep.x, ep.y);
                i+=2;
                break;
            }
            default:
                break;
            }
        }
        if (current)
            current->end = path.elementCount() - 1;
    }

//     for (int i=0; i<subpaths->size(); ++i) {
//         qDebug() << subpaths->at(i);
//     }

    // Check which intersect and merge in paths variable
    {
        for (int spi=0; spi<subpaths->size(); ++spi) {
            if (subpaths->at(spi).processed)
                continue;

            paths->add(spi);
            (subpaths->data() + spi)->added = true;

//             qDebug() << " - matching: " << spi << subpaths->at(spi) << paths->size();

            for (int i=paths->size() - 1; i<paths->size(); ++i) {
                QSubpath *s1 = subpaths->data() + paths->at(i);
                s1->processed = true;
//                 qDebug() << "   - checking" << paths->at(i) << *(subpaths->data() + paths->at(i));

                for (int j=spi+1; j<subpaths->size(); ++j) {
                    QSubpath *s2 = subpaths->data() + j;
                    if (s2->processed || s2->added)
                        continue;
//                     qDebug() << "      - against" << j << *s2;
                    if (s1->intersects(*s2)) {
//                         qDebug() << "        - intersects...";
                        s2->added = true;
                        paths->add(j);

                    }
                }
            }

            paths->add(-1);
//             qDebug() << "paths...";
//             for (int i=0; i<paths->size(); ++i)
//                 qDebug() << "  " << paths->at(i);
        }
    }
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
    inline GLuint bindTexture(const QImage &image, GLenum target = GL_TEXTURE_2D, GLint format = GL_RGBA8);
    inline GLuint bindTexture(const QPixmap &pixmap, GLenum target = GL_TEXTURE_2D, GLint format = GL_RGBA8);
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
        return widget->palette().brush(QPalette::Background).color();
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

#ifndef CALLBACK // for Windows
#define CALLBACK
#endif

// Need to allocate space for new vertices on intersecting lines and
// they need to be alive until gluTessEndPolygon() has returned
static QList<GLdouble *> vertexStorage;
static void CALLBACK qgl_tess_combine(GLdouble coords[3],
                                      GLdouble *[4],
                                      GLfloat [4], GLdouble **dataOut)
{
    GLdouble *vertex = (GLdouble *) malloc(3 * sizeof(GLdouble));
    vertex[0] = coords[0];
    vertex[1] = coords[1];
    vertex[2] = coords[2];
    *dataOut = vertex;
    vertexStorage.append(vertex);
}

static void CALLBACK qgl_tess_error(GLenum errorCode)
{
    qWarning("QOpenGLPaintEngine: tessellation error: %s", gluErrorString(errorCode));
}

struct QGLUTesselatorCleanupHandler
{
    inline QGLUTesselatorCleanupHandler() { qgl_tess = gluNewTess(); }
    inline ~QGLUTesselatorCleanupHandler() { gluDeleteTess(qgl_tess); }
    GLUtesselator *qgl_tess;
};

Q_GLOBAL_STATIC(QGLUTesselatorCleanupHandler, tessHandler)

class QOpenGLPaintEnginePrivate : public QPaintEnginePrivate {
    Q_DECLARE_PUBLIC(QOpenGLPaintEngine)
public:
    QOpenGLPaintEnginePrivate()
        : opacity(1)
        , has_fast_pen(false)
        , txop(QPainterPrivate::TxNone)
        , inverseScale(1)
        , moveToCount(0)
        , dashStroker(0)
        , stroker(0)
        , tessVector(20000)
        , shader_dev(0)
        , grad_palette(0)
        , has_glsl(false)
        {}

    inline void setGLPen(const QColor &c) {
        pen_color[0] = c.red();
        pen_color[1] = c.green();
        pen_color[2] = c.blue();
        pen_color[3] = qRound(c.alpha() * opacity);
    }

    inline void setGLBrush(const QColor &c) {
        brush_color[0] = c.red();
        brush_color[1] = c.green();
        brush_color[2] = c.blue();
        brush_color[3] = qRound(c.alpha() * opacity);
    }

    inline void setGradientOps(Qt::BrushStyle style);
    void generateGradientColorTable(const QGradientStops& s,
                                    unsigned int *colorTable, int size);
    void createGradientPaletteTexture(const QGradient& g);

    void updateGradient(const QBrush &brush);
    void beginPath(QPaintEngine::PolygonDrawMode mode);
    void endPath();
    inline void moveTo(const QPointF &p);
    inline void lineTo(const QPointF &p);
    inline void curveTo(const QPointF &cp1, const QPointF &cp2, const QPointF &ep);

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

    QMatrix matrix;
    GLubyte pen_color[4];
    GLubyte brush_color[4];
    QPainterPrivate::TransformationCodes txop;
    QGLDrawable drawable;

    qreal inverseScale;

    int moveToCount;
    QStroker basicStroker;
    QDashStroker *dashStroker;
    QStrokerOps *stroker;
    QPointF path_start;

    QDataBuffer<QSubpath> subpath_buffer;
    QDataBuffer<int> int_buffer;
    QDataBuffer<GLdouble> tessVector;

    QPaintDevice *shader_dev;
    GLuint grad_palette;

    GLuint radial_frag_program;
    GLuint conical_frag_program;

    bool has_glsl;
    GLuint radial_glsl_prog;
    GLuint radial_glsl_shader;

    GLuint conical_glsl_prog;
    GLuint conical_glsl_shader;

    GLuint radial_inv_location;
    GLuint radial_inv_mat_offset_location;
    GLuint radial_fmp_location;
    GLuint radial_fmp2_m_radius_location;
    GLuint radial_tex_location;

    GLuint conical_inv_location;
    GLuint conical_inv_mat_offset_location;
    GLuint conical_angle_location;
    GLuint conical_tex_location;
};

void QOpenGLPaintEnginePrivate::beginPath(QPaintEngine::PolygonDrawMode mode)
{
    Q_ASSERT(mode != QPaintEngine::PolylineMode);

    tessVector.reset();
    moveToCount = 0;

    GLUtesselator *qgl_tess = tessHandler()->qgl_tess;
    gluTessProperty(qgl_tess, GLU_TESS_WINDING_RULE,
                    mode == QPaintEngine::WindingMode
                    ? GLU_TESS_WINDING_NONZERO
                    : GLU_TESS_WINDING_ODD);
    gluTessBeginPolygon(qgl_tess, NULL);
    gluTessBeginContour(qgl_tess);
}

void QOpenGLPaintEnginePrivate::endPath()
{
    // rewind to first position...
    lineTo(path_start);
//     printf("endPath() -> tessVector = %d, vertexStorage = %d\n", tessVector.size(), vertexStorage.size());

    GLUtesselator *qgl_tess = tessHandler()->qgl_tess;
    gluTessEndContour(qgl_tess);
    gluTessEndPolygon(qgl_tess);

    for (int i=0; i<vertexStorage.size(); ++i)
        free(vertexStorage.at(i));
    vertexStorage.clear();
}

inline void QOpenGLPaintEnginePrivate::moveTo(const QPointF &p)
{
    if (moveToCount == 0) {
        // store rewind position
        path_start = p;
    } else if (moveToCount >= 2) {
        // rewind to first position...
        lineTo(path_start);
    }

    ++moveToCount;
    lineTo(p);
}

inline void QOpenGLPaintEnginePrivate::lineTo(const QPointF &p)
{
    GLUtesselator *qgl_tess = tessHandler()->qgl_tess;
    // ### temp crash fix - the GLU tesselator can't handle the
    // ### realloc being done after this
    if (tessVector.size() + 3 > 20000)
        return;
    tessVector.add(p.x());
    tessVector.add(p.y());
    tessVector.add(0);
    gluTessVertex(qgl_tess, tessVector.data() + tessVector.size() - 3, tessVector.data() + tessVector.size() - 3);
}

inline void QOpenGLPaintEnginePrivate::curveTo(const QPointF &cp1, const QPointF &cp2, const QPointF &ep)
{
    qreal x1 = tessVector.at(tessVector.size() - 3);
    qreal y1 = tessVector.at(tessVector.size() - 2);

    qreal inverseScaleHalf = inverseScale / 2;

    QBezier beziers[32];
    beziers[0] = QBezier::fromPoints(QPointF(x1, y1), cp1, cp2, ep);
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
            lineTo(QPointF(b->x4, b->y4));
            --b;
        } else {
            // split, second half of the polygon goes lower into the stack
            b->split(b+1, b);
            ++b;
        }
    }
}


static void strokeMoveTo(qfixed x, qfixed y, void *data)
{
    ((QOpenGLPaintEnginePrivate *) data)->moveTo(QPointF(qt_fixed_to_real(x),
                                                         qt_fixed_to_real(y)));
}


static void strokeLineTo(qfixed x, qfixed y, void *data)
{
    ((QOpenGLPaintEnginePrivate *) data)->lineTo(QPointF(qt_fixed_to_real(x),
                                                         qt_fixed_to_real(y)));
}


static void strokeCurveTo(qfixed c1x, qfixed c1y,
                   qfixed c2x, qfixed c2y,
                   qfixed ex, qfixed ey,
                   void *data)
{
    ((QOpenGLPaintEnginePrivate *) data)->curveTo(QPointF(qt_fixed_to_real(c1x),
                                                          qt_fixed_to_real(c1y)),
                                                  QPointF(qt_fixed_to_real(c2x),
                                                          qt_fixed_to_real(c2y)),
                                                  QPointF(qt_fixed_to_real(ex),
                                                          qt_fixed_to_real(ey)));
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

#ifdef Q_WS_WIN
#define glProgramStringARB ctx->d_ptr->qt_glProgramStringARB
#define glBindProgramARB ctx->d_ptr->qt_glBindProgramARB
#define glDeleteProgramsARB ctx->d_ptr->qt_glDeleteProgramsARB
#define glGenProgramsARB ctx->d_ptr->qt_glGenProgramsARB
#define glProgramLocalParameter4fvARB ctx->d_ptr->qt_glProgramLocalParameter4fvARB
// GLSL definitions
#define glCreateShader ctx->d_ptr->qt_glCreateShader
#define glShaderSource ctx->d_ptr->qt_glShaderSource
#define glCompileShader ctx->d_ptr->qt_glCompileShader
#define glDeleteShader ctx->d_ptr->qt_glDeleteShader

#define glCreateProgram ctx->d_ptr->qt_glCreateProgram
#define glAttachShader ctx->d_ptr->qt_glAttachShader
#define glDetachShader ctx->d_ptr->qt_glDetachShader
#define glLinkProgram ctx->d_ptr->qt_glLinkProgram
#define glUseProgram ctx->d_ptr->qt_glUseProgram
#define glDeleteProgram ctx->d_ptr->qt_glDeleteProgram

#define glGetShaderInfoLog ctx->d_ptr->qt_glGetShaderInfoLog
#define glGetProgramiv ctx->d_ptr->qt_glGetProgramiv

#define glGetUniformLocation ctx->d_ptr->qt_glGetUniformLocation
#define glUniform4fv ctx->d_ptr->qt_glUniform4fv
#define glUniform3fv ctx->d_ptr->qt_glUniform3fv
#define glUniform2fv ctx->d_ptr->qt_glUniform2fv
#define glUniform1fv ctx->d_ptr->qt_glUniform1fv
#define glUniform1i ctx->d_ptr->qt_glUniform1i

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

#ifdef Q_WS_WIN
bool qt_resolve_frag_program_extensions(QGLContext *ctx)
{
    if (glProgramStringARB != 0)
        return true;

    // ARB_fragment_program
    glProgramStringARB = (_glProgramStringARB) wglGetProcAddress("glProgramStringARB");
    glBindProgramARB = (_glBindProgramARB) wglGetProcAddress("glBindProgramARB");
    glDeleteProgramsARB = (_glDeleteProgramsARB) wglGetProcAddress("glDeleteProgramsARB");
    glGenProgramsARB = (_glGenProgramsARB) wglGetProcAddress("glGenProgramsARB");
    glProgramLocalParameter4fvARB = (_glProgramLocalParameter4fvARB) wglGetProcAddress("glProgramLocalParameter4fvARB");

    return glProgramStringARB
        && glBindProgramARB
        && glDeleteProgramsARB
        && glGenProgramsARB
        && glProgramLocalParameter4fvARB;
}
#else
static bool qt_resolve_frag_program_extensions(QGLContext *)
{
    static int resolved = false;

    if (!resolved) {

        QGLContext cx(QGLFormat::defaultFormat());

        // ARB_fragment_program
        qt_glProgramStringARB = (_glProgramStringARB) cx.getProcAddress(QLatin1String("glProgramStringARB"));
        qt_glBindProgramARB = (_glBindProgramARB) cx.getProcAddress(QLatin1String("glBindProgramARB"));
        qt_glDeleteProgramsARB = (_glDeleteProgramsARB) cx.getProcAddress(QLatin1String("glDeleteProgramsARB"));
        qt_glGenProgramsARB = (_glGenProgramsARB) cx.getProcAddress(QLatin1String("glGenProgramsARB"));
        qt_glProgramLocalParameter4fvARB = (_glProgramLocalParameter4fvARB) cx.getProcAddress(QLatin1String("glProgramLocalParameter4fvARB"));

        resolved = true;
    }

    return qt_glProgramStringARB
        && qt_glBindProgramARB
        && qt_glDeleteProgramsARB
        && qt_glGenProgramsARB
        && qt_glProgramLocalParameter4fvARB;
}
#endif

#ifdef Q_WS_WIN
bool qt_resolve_GLSL_functions(QGLContext *ctx)
{
    if (glCreateShader != 0)
        return true;

    // GLSL
    glCreateShader = (_glCreateShader) wglGetProcAddress("glCreateShader");
    glShaderSource = (_glShaderSource) wglGetProcAddress("glShaderSource");
    glCompileShader = (_glCompileShader) wglGetProcAddress("glCompileShader");
    glDeleteShader = (_glDeleteShader) wglGetProcAddress("glDeleteShader");

    glCreateProgram = (_glCreateProgram) wglGetProcAddress("glCreateProgram");
    glAttachShader = (_glAttachShader) wglGetProcAddress("glAttachShader");
    glDetachShader = (_glDetachShader) wglGetProcAddress("glDetachShader");
    glLinkProgram = (_glLinkProgram) wglGetProcAddress("glLinkProgram");
    glUseProgram = (_glUseProgram) wglGetProcAddress("glUseProgram");
    glDeleteProgram = (_glDeleteProgram) wglGetProcAddress("glDeleteProgram");

    glGetShaderInfoLog = (_glGetShaderInfoLog) wglGetProcAddress("glGetShaderInfoLog");
    glGetProgramiv = (_glGetProgramiv) wglGetProcAddress("glGetProgramiv");

    glGetUniformLocation =  (_glGetUniformLocation) wglGetProcAddress("glGetUniformLocation");
    glUniform4fv = (_glUniform4fv) wglGetProcAddress("glUniform4fv");
    glUniform3fv = (_glUniform3fv) wglGetProcAddress("glUniform3fv");
    glUniform2fv = (_glUniform2fv) wglGetProcAddress("glUniform2fv");
    glUniform1fv = (_glUniform1fv) wglGetProcAddress("glUniform1fv");
    glUniform1i = (_glUniform1i) wglGetProcAddress("glUniform1i");

    return glCreateShader
        && glShaderSource
        && glCompileShader
        && glDeleteShader
        && glCreateProgram
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
#else
static bool qt_resolve_GLSL_functions(QGLContext *)
{
    static int resolved = false;

    if (!resolved) {
        QGLContext cx(QGLFormat::defaultFormat());

        // GLSL
        qt_glCreateShader = (_glCreateShader) cx.getProcAddress(QLatin1String("glCreateShader"));
        qt_glShaderSource = (_glShaderSource) cx.getProcAddress(QLatin1String("glShaderSource"));
        qt_glCompileShader = (_glCompileShader) cx.getProcAddress(QLatin1String("glCompileShader"));
        qt_glDeleteShader = (_glDeleteShader) cx.getProcAddress(QLatin1String("glDeleteShader"));
        qt_glCreateProgram = (_glCreateProgram) cx.getProcAddress(QLatin1String("glCreateProgram"));

        qt_glCreateProgram = (_glCreateProgram) cx.getProcAddress(QLatin1String("glCreateProgram"));
        qt_glAttachShader = (_glAttachShader) cx.getProcAddress(QLatin1String("glAttachShader"));
        qt_glDetachShader = (_glDetachShader) cx.getProcAddress(QLatin1String("glDetachShader"));
        qt_glLinkProgram = (_glLinkProgram) cx.getProcAddress(QLatin1String("glLinkProgram"));
        qt_glUseProgram = (_glUseProgram) cx.getProcAddress(QLatin1String("glUseProgram"));
        qt_glDeleteProgram = (_glDeleteProgram) cx.getProcAddress(QLatin1String("glDeleteProgram"));

        qt_glGetShaderInfoLog = (_glGetShaderInfoLog) cx.getProcAddress(QLatin1String("glGetShaderInfoLog"));
        qt_glGetProgramiv = (_glGetProgramiv) cx.getProcAddress(QLatin1String("glGetProgramiv"));

        qt_glGetUniformLocation = (_glGetUniformLocation) cx.getProcAddress(QLatin1String("glGetUniformLocation"));
        qt_glUniform4fv = (_glUniform4fv) cx.getProcAddress(QLatin1String("glUniform4fv"));
        qt_glUniform3fv = (_glUniform3fv) cx.getProcAddress(QLatin1String("glUniform3fv"));
        qt_glUniform2fv = (_glUniform2fv) cx.getProcAddress(QLatin1String("glUniform2fv"));
        qt_glUniform1fv = (_glUniform1fv) cx.getProcAddress(QLatin1String("glUniform1fv"));
        qt_glUniform1i = (_glUniform1i) cx.getProcAddress(QLatin1String("glUniform1i"));

        resolved = true;
    }

    return qt_glCreateShader
        && qt_glShaderSource
        && qt_glCompileShader
        && qt_glDeleteShader
        && qt_glCreateProgram
        && qt_glCreateProgram
        && qt_glDetachShader
        && qt_glLinkProgram
        && qt_glUseProgram
        && qt_glDeleteProgram
        && qt_glGetShaderInfoLog
        && qt_glGetProgramiv
        && qt_glGetUniformLocation
        && qt_glUniform4fv
        && qt_glUniform3fv
        && qt_glUniform2fv
        && qt_glUniform1fv
        && qt_glUniform1i;
}
#endif

void QOpenGLPaintEnginePrivate::generateGradientColorTable(const QGradientStops& s, unsigned int *colorTable, int size)
{
    int pos = 0;
    qreal fpos = 0.0;
    qreal incr = 1.0 / qreal(size);
    QVector<unsigned int> colors(s.size());

    for (int i = 0; i < s.size(); ++i)
        colors[i] = s[i].second.rgba();

    while (fpos < s.first().first) {
        colorTable[pos] = colors[0];
        pos++;
        fpos += incr;
    }

    for (int i = 0; i < s.size() - 1; ++i) {
        qreal delta = 1/(s[i+1].first - s[i].first);
        while (fpos < s[i+1].first && pos < size) {
            int dist = int(255 * ((fpos - s[i].first) * delta));
            int idist = 255 - dist;
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
            colorTable[pos] = INTERPOLATE_PIXEL_256(colors[i], idist, colors[i+1], dist);
#else
            uint c = INTERPOLATE_PIXEL_256(colors[i], idist, colors[i+1], dist);
            colorTable[pos] = ((c << 24) & 0xff000000)
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

void QOpenGLPaintEnginePrivate::createGradientPaletteTexture(const QGradient& g)
{
    const int PAL_SIZE = 1024;
    unsigned int palbuf[PAL_SIZE];
    generateGradientColorTable(g.stops(), palbuf, PAL_SIZE);

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
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, PAL_SIZE, 0, GL_BGRA, GL_UNSIGNED_BYTE, palbuf);
}


inline void QOpenGLPaintEnginePrivate::setGradientOps(Qt::BrushStyle style)
{
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
            else
                glEnable(GL_FRAGMENT_PROGRAM_ARB);
        } else if (style == Qt::ConicalGradientPattern) {
            if (has_glsl)
                glUseProgram(conical_glsl_prog);
            else
                glEnable(GL_FRAGMENT_PROGRAM_ARB);
        } else {
            if (has_glsl)
                glUseProgram(0);
            else
                glDisable(GL_FRAGMENT_PROGRAM_ARB);
        }
    }
}

QOpenGLPaintEngine::QOpenGLPaintEngine()
    : QPaintEngine(*(new QOpenGLPaintEnginePrivate),
                   PaintEngineFeatures(AllFeatures
                                       & ~(LinearGradientFill
                                           | RadialGradientFill
                                           | ConicalGradientFill
                                           | PatternBrush)))
{
    Q_D(QOpenGLPaintEngine);
    d->basicStroker.setMoveToHook(strokeMoveTo);
    d->basicStroker.setLineToHook(strokeLineTo);
    d->basicStroker.setCubicToHook(strokeCurveTo);

    GLUtesselator *qgl_tess = tessHandler()->qgl_tess;

// This removes warnings on OS X 10.4 and below.
#if defined(Q_OS_MAC) && !defined(Q_CC_INTEL) && (MAC_OS_X_VERSION_MAX_ALLOWED < MAC_OS_X_VERSION_10_5) 
    gluTessCallback(qgl_tess, GLU_TESS_BEGIN, reinterpret_cast<GLvoid (CALLBACK *)(...)>(&glBegin));
    gluTessCallback(qgl_tess, GLU_TESS_VERTEX,
                    reinterpret_cast<GLvoid (CALLBACK *)(...)>(&glVertex3dv));
    gluTessCallback(qgl_tess, GLU_TESS_END, reinterpret_cast<GLvoid (CALLBACK *)(...)>(&glEnd));
    gluTessCallback(qgl_tess, GLU_TESS_COMBINE,
                    reinterpret_cast<GLvoid (CALLBACK *)(...)>(&qgl_tess_combine));
    gluTessCallback(qgl_tess, GLU_TESS_ERROR,
                    reinterpret_cast<GLvoid (CALLBACK *)(...)>(&qgl_tess_error));
#else
    gluTessCallback(qgl_tess, GLU_TESS_BEGIN, reinterpret_cast<GLvoid (CALLBACK *)()>(&glBegin));
    gluTessCallback(qgl_tess, GLU_TESS_VERTEX,
                    reinterpret_cast<GLvoid (CALLBACK *)()>(&glVertex3dv));
    gluTessCallback(qgl_tess, GLU_TESS_END, reinterpret_cast<GLvoid (CALLBACK *)()>(&glEnd));
    gluTessCallback(qgl_tess, GLU_TESS_COMBINE,
                    reinterpret_cast<GLvoid (CALLBACK *)()>(&qgl_tess_combine));
    gluTessCallback(qgl_tess, GLU_TESS_ERROR,
                    reinterpret_cast<GLvoid (CALLBACK *) ()>(&qgl_tess_error));
#endif
}

QOpenGLPaintEngine::~QOpenGLPaintEngine()
{
    Q_D(QOpenGLPaintEngine);
    if (d->dashStroker)
        delete d->dashStroker;
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

bool QOpenGLPaintEngine::begin(QPaintDevice *pdev)
{
    Q_D(QOpenGLPaintEngine);
    d->drawable.setDevice(pdev);
    d->has_clipping = false;
    d->has_fast_pen = false;
    d->inverseScale = 1;
    d->opacity = 1;
    d->drawable.makeCurrent();

    QGLContext *ctx = const_cast<QGLContext *>(d->drawable.context());
    if (QGLExtensions::glExtensions & QGLExtensions::FragmentProgram)
        qt_resolve_frag_program_extensions(ctx);

    if (QGLFormat::openGLVersionFlags() & QGLFormat::OpenGL_Version_2_0 ||
        QGLFormat::openGLVersionFlags() & QGLFormat::OpenGL_ES_Version_2_0)
        d->has_glsl = qt_resolve_GLSL_functions(ctx);

    glPushAttrib(GL_ALL_ATTRIB_BITS);
    if (QGLExtensions::glExtensions & QGLExtensions::SampleBuffers)
        glDisable(GL_MULTISAMPLE);
    glDisable(GL_TEXTURE_1D);
    glDisable(GL_TEXTURE_2D);
    if (QGLExtensions::glExtensions & QGLExtensions::TextureRectangle)
        glDisable(GL_TEXTURE_RECTANGLE_NV);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_CULL_FACE);
    glShadeModel(GL_FLAT);

    const QColor &c = d->drawable.backgroundColor();
    glClearColor(c.redF(), c.greenF(), c.blueF(), 1.0);
    if (d->drawable.autoFillBackground())
        glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_ACCUM_BUFFER_BIT);
    QSize sz(d->drawable.size());
    glViewport(0, 0, sz.width(), sz.height());
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, sz.width(), sz.height(), 0, -999999, 999999);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);


    if ((QGLExtensions::glExtensions & QGLExtensions::MirroredRepeat)
        && (pdev != d->shader_dev))
    {
        if (d->shader_dev) {
            glBindTexture(GL_TEXTURE_1D, 0);
            glDeleteTextures(1, &d->grad_palette);

            if (d->has_glsl) {
                glDeleteShader(d->radial_glsl_shader);
                glDeleteProgram(d->radial_glsl_prog);
                glDeleteShader(d->conical_glsl_shader);
                glDeleteProgram(d->conical_glsl_prog);
            } else if (QGLExtensions::glExtensions & QGLExtensions::FragmentProgram) {
                glDeleteProgramsARB(1, &d->radial_frag_program);
                glDeleteProgramsARB(1, &d->conical_frag_program);
            }
        }
        d->shader_dev = pdev;
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

        } else if (QGLExtensions::glExtensions & QGLExtensions::FragmentProgram) {
            glGenProgramsARB(1, &d->radial_frag_program);
            glGenProgramsARB(1, &d->conical_frag_program);

            glGetError(); // reset the error state
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
        }
    }

    setDirty(QPaintEngine::DirtyPen);
    setDirty(QPaintEngine::DirtyBrush);
    return true;
}

bool QOpenGLPaintEngine::end()
{
    Q_D(QOpenGLPaintEngine);
    glPopAttrib();
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
        updateMatrix(state.matrix());
    }

    if (flags & DirtyPen) {
        update_fast_pen = true;
        updatePen(state.pen());
    }

    if (flags & DirtyBrush) {
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

    if (update_fast_pen) {
        Q_D(QOpenGLPaintEngine);
        qreal pen_width = d->cpen.widthF();
        d->has_fast_pen =
            (pen_width == 0 || (pen_width == 1 && d->txop <= QPainterPrivate::TxTranslate))
            && d->cpen.style() == Qt::SolidLine
            && d->cpen.isSolid();
    }
}

void QOpenGLPaintEnginePrivate::updateGradient(const QBrush &brush)
{
    bool has_mirrored_repeat = QGLExtensions::glExtensions & QGLExtensions::MirroredRepeat;
    bool has_frag_program = QGLExtensions::glExtensions & QGLExtensions::FragmentProgram;
    Qt::BrushStyle style = brush.style();
    QGL_FUNC_CONTEXT;

    if (has_mirrored_repeat && style == Qt::LinearGradientPattern) {
        const QLinearGradient *g = static_cast<const QLinearGradient *>(brush.gradient());
        QPointF start = brush.matrix().map(g->start());
        QPointF stop = brush.matrix().map(g->finalStop());

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
            QMatrix translate(1, 0, 0, 1, -g->focalPoint().x(), -g->focalPoint().y());
            QMatrix gl_to_qt(1, 0, 0, -1, 0, pdev->height());
            QMatrix inv_matrix = gl_to_qt * matrix.inverted() * brush.matrix().inverted() * translate;

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
            }
            glBindTexture(GL_TEXTURE_1D, grad_palette);
            createGradientPaletteTexture(*brush.gradient());
        } else if (style == Qt::ConicalGradientPattern) {
            const QConicalGradient *g = static_cast<const QConicalGradient *>(brush.gradient());
            QMatrix translate(1, 0, 0, 1, -g->center().x(), -g->center().y());
            QMatrix gl_to_qt(1, 0, 0, -1, 0, pdev->height());
            QMatrix inv_matrix = gl_to_qt * matrix.inverted() * brush.matrix().inverted() * translate;


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
    glDisable(GL_FRAGMENT_PROGRAM_ARB);
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
        glColor4ubv(d->pen_color);
    }

    d->basicStroker.setJoinStyle(pen.joinStyle());
    d->basicStroker.setCapStyle(pen.capStyle());
    d->basicStroker.setMiterLimit(pen.miterLimit());
    qreal penWidth = pen.widthF();
    if (penWidth == 0)
        d->basicStroker.setStrokeWidth(1);
    else
        d->basicStroker.setStrokeWidth(penWidth);

    if (pen_style == Qt::SolidLine) {
        d->stroker = &d->basicStroker;
    } else if (pen_style != Qt::NoPen) {
        if (!d->dashStroker)
            d->dashStroker = new QDashStroker(&d->basicStroker);

        QRectF deviceRect(0, 0, d->pdev->width(), d->pdev->height());
        if (penWidth == 0) {
            d->dashStroker->setClipRect(deviceRect);
        } else {
            QRectF clipRect = d->matrix.inverted().mapRect(deviceRect);
            d->dashStroker->setClipRect(clipRect);
        }

        d->dashStroker->setDashPattern(pen.dashPattern());
        d->stroker = d->dashStroker;
    } else {
        d->stroker = 0;
    }
}

void QOpenGLPaintEngine::updateBrush(const QBrush &brush, const QPointF &)
{
    Q_D(QOpenGLPaintEngine);
    d->cbrush = brush;
    d->brush_style = brush.style();
    d->has_brush = (d->brush_style != Qt::NoBrush);

    if (d->brush_style >= Qt::LinearGradientPattern
        && d->brush_style <= Qt::ConicalGradientPattern) {
        d->updateGradient(brush);
    } else {
        d->setGLBrush(brush.color());
        glColor4ubv(d->brush_color);
    }
}

void QOpenGLPaintEngine::updateFont(const QFont &)
{
}

void QOpenGLPaintEngine::updateMatrix(const QMatrix &mtx)
{
    Q_D(QOpenGLPaintEngine);

    d->matrix = mtx;
    GLdouble mat[4][4];

    mat[0][0] = mtx.m11();
    mat[0][1] = mtx.m12();
    mat[0][2] = 0;
    mat[0][3] = 0;

    mat[1][0] = mtx.m21();
    mat[1][1] = mtx.m22();
    mat[1][2] = 0;
    mat[1][3] = 0;

    mat[2][0] = 0;
    mat[2][1] = 0;
    mat[2][2] = 1;
    mat[2][3] = 0;

    mat[3][0] = mtx.dx();
    mat[3][1] = mtx.dy();
    mat[3][2] = 0;
    mat[3][3] = 1;

    if (mtx.m12() != 0 || mtx.m21() != 0)
        d->txop = QPainterPrivate::TxRotShear;
    else if (mtx.m11() != 1 || mtx.m22() != 1)
        d->txop = QPainterPrivate::TxScale;
    else if (mtx.dx() != 0 || mtx.dy() != 0)
        d->txop = QPainterPrivate::TxTranslate;
    else
        d->txop = QPainterPrivate::TxNone;

    // 1/10000 == 0.0001, so we have good enough res to cover curves
    // that span the entire widget...
    d->inverseScale = qMax(1 / qMax( qMax(qAbs(mtx.m11()), qAbs(mtx.m22())),
                                     qMax(qAbs(mtx.m12()), qAbs(mtx.m21())) ),
                           0.0001);

    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixd(&mat[0][0]);
}

void QOpenGLPaintEngine::updateClipRegion(const QRegion &clipRegion, Qt::ClipOperation op)
{
    Q_D(QOpenGLPaintEngine);
    QGLFormat f = d->drawable.format();
    bool useStencilBuffer = f.stencil();
    bool useDepthBuffer = f.depth() && !useStencilBuffer;

    // clipping is only supported when a stencil or depth buffer is
    // available
    if (!useStencilBuffer && !useDepthBuffer)
        return;

    if (op == Qt::NoClip) {
        d->has_clipping = false;
        d->crgn = QRegion();
        glDisable(useStencilBuffer ? GL_STENCIL_TEST : GL_DEPTH_TEST);
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

    if (useStencilBuffer) {
        glClearStencil(0x0);
        glClear(GL_STENCIL_BUFFER_BIT);
        glClearStencil(0x1);
    } else {
        glClearDepth(0x0);
        glClear(GL_DEPTH_BUFFER_BIT);
        glDepthMask(true);
        glClearDepth(0x1);
    }

    const QVector<QRect> rects = d->crgn.rects();
    glEnable(GL_SCISSOR_TEST);
    for (int i = 0; i < rects.size(); ++i) {
        glScissor(rects.at(i).left(), d->drawable.size().height() - rects.at(i).bottom(),
                  rects.at(i).width(), rects.at(i).height());
        glClear(useStencilBuffer ? GL_STENCIL_BUFFER_BIT : GL_DEPTH_BUFFER_BIT);
    }
    glDisable(GL_SCISSOR_TEST);

    if (useStencilBuffer) {
        glStencilFunc(GL_EQUAL, 0x1, 0x1);
        glEnable(GL_STENCIL_TEST);
    } else {
        glDepthFunc(GL_LEQUAL);
        glEnable(GL_DEPTH_TEST);
    }
    d->has_clipping = true;
}

void QOpenGLPaintEngine::updateRenderHints(QPainter::RenderHints hints)
{
    if (!(QGLExtensions::glExtensions & QGLExtensions::SampleBuffers))
        return;
    if (hints & QPainter::Antialiasing)
        glEnable(GL_MULTISAMPLE);
    else
        glDisable(GL_MULTISAMPLE);
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
            glColor4ubv(d->brush_color);
            
            glVertexPointer(2, GL_FLOAT, 0, vertexArray);
            glEnableClientState(GL_VERTEX_ARRAY);
            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
            glDisableClientState(GL_VERTEX_ARRAY);
        }

        if (d->has_pen) {
            d->setGradientOps(d->pen_brush_style);
            glColor4ubv(d->pen_color);
            if (d->has_fast_pen) {
                glColor4ubv(d->pen_color);
                
                vertexArray[8] = vertexArray[0];
                vertexArray[9] = vertexArray[1];

                glVertexPointer(2, GL_FLOAT, 0, vertexArray);
                glEnableClientState(GL_VERTEX_ARRAY);
                glDrawArrays(GL_LINE_STRIP, 0, 5);
                glDisableClientState(GL_VERTEX_ARRAY);
            } else {
                d->beginPath(QPaintEngine::WindingMode);
                d->stroker->begin(d);
                d->stroker->moveTo(qt_real_to_fixed(left), qt_real_to_fixed(top));
                d->stroker->lineTo(qt_real_to_fixed(right), qt_real_to_fixed(top));
                d->stroker->lineTo(qt_real_to_fixed(right), qt_real_to_fixed(bottom));
                d->stroker->lineTo(qt_real_to_fixed(left), qt_real_to_fixed(bottom));
                d->stroker->lineTo(qt_real_to_fixed(left), qt_real_to_fixed(top));
                d->stroker->end();
                d->endPath();
            }
        }
    }
}

void QOpenGLPaintEngine::drawPoints(const QPointF *points, int pointCount)
{
    Q_D(QOpenGLPaintEngine);
    d->setGradientOps(d->pen_brush_style);

    GLfloat pen_width = d->cpen.widthF();
    if (pen_width > 1 || (pen_width > 0 && d->txop > QPainterPrivate::TxTranslate)) {
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
            glColor4ubv(d->pen_color);
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
            glColor4ubv(d->pen_color);
            for (int i=0; i<lineCount; ++i) {
                const QLineF &l = lines[i];
                d->beginPath(QPaintEngine::WindingMode);
                d->stroker->begin(d);
                d->stroker->moveTo(qt_real_to_fixed(l.x1()), qt_real_to_fixed(l.y1()));
                d->stroker->lineTo(qt_real_to_fixed(l.x2()), qt_real_to_fixed(l.y2()));
                d->stroker->end();
                d->endPath();
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
            glColor4ubv(d->brush_color);
            d->beginPath(mode);
            d->moveTo(points[0]);
            for (int i=1; i<pointCount; ++i)
                d->lineTo(points[i]);
            d->endPath();
        }
    }

    if (d->has_pen) {
        d->setGradientOps(d->pen_brush_style);
        glColor4ubv(d->pen_color);
        if (d->has_fast_pen) {
            QVarLengthArray<float> vertexArray(pointCount*2);
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
            }
            else {
                glDrawArrays(GL_LINE_STRIP, 0, pointCount);
            }
            glDisableClientState(GL_VERTEX_ARRAY);
        } else {
            d->beginPath(WindingMode);
            d->stroker->strokePolygon(points, pointCount, mode != PolylineMode, d, QMatrix());
            d->endPath();
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
        // Don't split "simple" paths...
        if (path.elementCount() > 32) {
            qt_painterpath_split(path, &d->int_buffer, &d->subpath_buffer);
            glColor4ubv(d->brush_color);
            for (int i=0; i<d->int_buffer.size(); ++i) {
                d->beginPath(path.fillRule() == Qt::WindingFill
                             ? QPaintEngine::WindingMode
                             : QPaintEngine::OddEvenMode);
                for (; d->int_buffer.at(i) != -1; ++i) {
                    const QSubpath &sp = d->subpath_buffer.at(d->int_buffer.at(i));
                    for (int j=sp.start; j<=sp.end; ++j) {
                        const QPainterPath::Element &e = path.elementAt(j);
                        switch (e.type) {
                        case QPainterPath::MoveToElement:
                            d->moveTo(e);
                            break;
                        case QPainterPath::LineToElement:
                            d->lineTo(e);
                            break;
                        case QPainterPath::CurveToElement:
                            d->curveTo(e, path.elementAt(j+1), path.elementAt(j+2));
                            j+=2;
                            break;
                        default:
                            break;
                        }
                    }
                }
                d->endPath();
            }
        } else {
            glColor4ubv(d->brush_color);
            d->beginPath(path.fillRule() == Qt::WindingFill
                         ? QPaintEngine::WindingMode
                         : QPaintEngine::OddEvenMode);
            for (int i=0; i<path.elementCount(); ++i) {
                const QPainterPath::Element &e = path.elementAt(i);
                switch (e.type) {
                case QPainterPath::MoveToElement:
                    d->moveTo(e);
                    break;
                case QPainterPath::LineToElement:
                    d->lineTo(e);
                    break;
                case QPainterPath::CurveToElement:
                    d->curveTo(e, path.elementAt(i+1), path.elementAt(i+2));
                    i+=2;
                    break;
                default:
                    break;
                }
            }
            d->endPath();
        }
    }

    if (d->has_pen) {
        glColor4ubv(d->pen_color);
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
            d->beginPath(WindingMode);
            d->stroker->strokePath(path, d, QMatrix());
            d->endPath();
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

    glPushAttrib(GL_CURRENT_BIT);
    glColor4f(1.0, 1.0, 1.0, d->opacity);
    glEnable(GL_TEXTURE_2D);

    GLdouble tc_w = r.width()/pm.width();
    GLdouble tc_h = r.height()/pm.height();

    // Rotate the texture so that it is aligned correctly and the
    // wrapping is done correctly
    glMatrixMode(GL_TEXTURE);
    glPushMatrix();
    glRotated(180.0, 0.0, 1.0, 0.0);
    glRotated(180.0, 0.0, 0.0, 1.0);

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
    glPopAttrib();
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
    glPushAttrib(GL_CURRENT_BIT);
    glColor4f(1.0, 1.0, 1.0, d->opacity);
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
    glPopAttrib();
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
            connect(widget, SIGNAL(destroyed(QObject *)), SLOT(widgetDestroyed(QObject *)));
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
        connect(ti.fontEngine, SIGNAL(destroyed(QObject *)), SLOT(fontEngineDestroyed(QObject *)));
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
        int tex_height = nearest_gl_texture_size(qRound(ti.ascent.toReal() + ti.descent.toReal())+2);
        int tex_width = nearest_gl_texture_size(tex_height*30); // ###
        if (tex_width > 2048)
            tex_width = 2048;

        uchar *tex_data = (uchar *) malloc(tex_width*tex_height*2);
        memset(tex_data, 0, tex_width*tex_height*2);
        glBindTexture(GL_TEXTURE_2D, font_texture);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE8_ALPHA8,
                     tex_width, tex_height, 0,
                     GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, tex_data);
        free(tex_data);
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
                font_tex->x_offset = x_margin;
                font_tex->y_offset += glyph_height + y_margin;
            }

            const int scale = 1;
            QGLGlyphCoord *qgl_glyph = new QGLGlyphCoord;
            qgl_glyph->x = float(font_tex->x_offset) / font_tex->width;
            qgl_glyph->y = float(font_tex->y_offset) / font_tex->height;
            qgl_glyph->width = float(glyph_width/scale) / font_tex->width;
            qgl_glyph->height = float(glyph_height/scale) / font_tex->height;
            qgl_glyph->log_width = qgl_glyph->width * font_tex->width;
            qgl_glyph->log_height = qgl_glyph->height * font_tex->height;
            qgl_glyph->x_offset = -metrics.x;
            qgl_glyph->y_offset = metrics.y;

            QImage glyph_im(ti.fontEngine->alphaMapForGlyph(glyphs[i]).convertToFormat(QImage::Format_Indexed8));
            int padded_width = glyph_im.width();
            if (padded_width%2 != 0)
                ++padded_width;

            int idx = 0;
            uchar *tex_data = (uchar *) malloc(padded_width*glyph_im.height()*2);
            memset(tex_data, 0, padded_width*glyph_im.height()*2);

            for (int y=0; y<glyph_im.height(); ++y) {
                uchar *s = (uchar *) glyph_im.scanLine(y);
                for (int x=0; x<glyph_im.width(); ++x) {
                    tex_data[idx] = 0xff;
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
    return current_cache->find(g).value(); // ### careful here
}

Q_GLOBAL_STATIC(QGLGlyphCache, qt_glyph_cache)

void QOpenGLPaintEngine::drawTextItem(const QPointF &p, const QTextItem &textItem)
{
    Q_D(QOpenGLPaintEngine);

    // fall back to drawing a polygon if the scale factor is large, or
    // we use a gradient pen
    if ((d->matrix.det() > 2) || (d->pen_brush_style >= Qt::LinearGradientPattern
                                  && d->pen_brush_style <= Qt::ConicalGradientPattern)) {
        QPaintEngine::drawTextItem(p, textItem);
        return;
    }

    // add the glyphs used to the glyph texture cache
    const QTextItemInt &ti = static_cast<const QTextItemInt &>(textItem);
    QVarLengthArray<QFixedPoint> positions;
    QVarLengthArray<glyph_t> glyphs;
    QMatrix matrix;
    matrix.translate(qRound(p.x()), qRound(p.y()));
    ti.fontEngine->getGlyphPositions(ti.glyphs, ti.num_glyphs, matrix, ti.flags, glyphs, positions);

    // make sure the glyphs we want to draw are in the cache
    qt_glyph_cache()->cacheGlyphs(d->drawable.context(), ti, glyphs);

    d->setGradientOps(Qt::SolidPattern); // turns off gradient ops
    glColor4ubv(d->pen_color);
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

#include "qpaintengine_opengl.moc"
