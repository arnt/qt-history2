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

#include <private/qmath_p.h>
#include <private/qdrawhelper_p.h>
#include <qdebug.h>
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
#include <qglpbuffer.h>
#include <private/qglpbuffer_p.h>
#include <private/qstroker_p.h>
#include <private/qbezier_p.h>

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
    bool marked;

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
      << p.marked;
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
                QSubpath sp = { i, 0, QREAL_MAX, QREAL_MIN, QREAL_MAX, QREAL_MIN, false };
                sp.unite(e.x, e.y);
                subpaths->add(sp);
                current = &subpaths->data()[subpaths->size() - 1];
                break;
            }
            case QPainterPath::LineToElement:
                current->unite(e.x, e.y);
                break;
            case QPainterPath::CurveToElement: {
                const QPainterPath::Element &cp2 = path.elementAt(i+1);
                const QPainterPath::Element &ep = path.elementAt(i+2);
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
            if (subpaths->at(spi).marked)
                continue;

            paths->add(spi);

//             qDebug() << " - matching: " << spi << subpaths->at(spi) << paths->size();

            for (int i=paths->size() - 1; i<paths->size(); ++i) {
                Q_ASSERT(paths->at(i) >= 0);
                QSubpath *s1 = subpaths->data() + paths->at(i);
                s1->marked = true;
//                 qDebug() << "   - marking" << paths->at(i) << *(subpaths->data() + paths->at(i));

                for (int j=spi+1; j<subpaths->size(); ++j) {
                    const QSubpath &s2 = subpaths->at(j);
                    if (s2.marked)
                        continue;
//                     qDebug() << "      - " << j << s2;
                    if (s1->intersects(s2)) {
//                         qDebug() << "        - intersects...";
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


#ifndef CALLBACK // for Windows
#define CALLBACK
#endif

#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE  0x809D
#endif

// define QT_GL_NO_CONCAVE_POLYGONS to remove support for drawing
// concave polygons (much faster)

//#define QT_GL_NO_CONCAVE_POLYGONS

class QGLDrawable {
public:
    QGLDrawable() : widget(0), buffer(0) {}
    inline void setDevice(QPaintDevice *pdev);
    inline void setAutoBufferSwap(bool);
    inline bool autoBufferSwap() const;
    inline void swapBuffers();
    inline void makeCurrent();
    inline QSize size() const;
    inline QGLFormat format() const;
    inline GLuint bindTexture(const QImage &image, GLenum target = GL_TEXTURE_2D, GLint format = GL_RGBA8);
    inline GLuint bindTexture(const QPixmap &pixmap, GLenum target = GL_TEXTURE_2D, GLint format = GL_RGBA8);
    inline QColor backgroundColor() const;

private:
    QGLWidget *widget;
    QGLPbuffer *buffer;
};

void QGLDrawable::setDevice(QPaintDevice *pdev)
{
    if (pdev->devType() == QInternal::Widget)
        widget = static_cast<QGLWidget *>(pdev);
    else if (pdev->devType() == QInternal::Pbuffer)
        buffer = static_cast<QGLPbuffer *>(pdev);
}

inline void QGLDrawable::setAutoBufferSwap(bool enable)
{
    if (widget)
        widget->setAutoBufferSwap(enable);
}

inline bool QGLDrawable::autoBufferSwap() const
{
    return widget && widget->autoBufferSwap();
}

inline void QGLDrawable::swapBuffers()
{
    if (widget)
        widget->swapBuffers();
}

inline void QGLDrawable::makeCurrent()
{
    if (widget)
        widget->makeCurrent();
    else if (buffer)
        buffer->makeCurrent();
}

inline QSize QGLDrawable::size() const
{
    if (widget)
        return widget->size();
    else if (buffer)
        return buffer->size();
    return QSize();
}

inline QGLFormat QGLDrawable::format() const
{
    if (widget)
        return widget->format();
    else if (buffer)
        return buffer->format();
    return QGLFormat();
}

inline GLuint QGLDrawable::bindTexture(const QImage &image, GLenum target, GLint format)
{
    if (widget)
        return widget->d_func()->glcx->d_func()->bindTexture(image, target, format, true);
    else if (buffer)
        return buffer->d_func()->qctx->d_func()->bindTexture(image, target, format, true);
    return 0;
}

inline GLuint QGLDrawable::bindTexture(const QPixmap &pixmap, GLenum target, GLint format)
{
    if (widget)
        return widget->d_func()->glcx->d_func()->bindTexture(pixmap, target, format, true);
    else if (buffer)
        return buffer->d_func()->qctx->d_func()->bindTexture(pixmap, target, format, true);
    return 0;
}

inline QColor QGLDrawable::backgroundColor() const
{
    if (widget)
        return widget->palette().brush(QPalette::Background).color();
    return QColor();
}

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

#ifndef GL_ARB_shader_objects
typedef char GLcharARB;
typedef unsigned int GLhandleARB;
#endif

class QOpenGLPaintEnginePrivate : public QPaintEnginePrivate {
    Q_DECLARE_PUBLIC(QOpenGLPaintEngine)
public:
    QOpenGLPaintEnginePrivate()
        : bgmode(Qt::TransparentMode)
        , has_fast_pen(false)
        , has_grad_brush(false)
        , txop(QPainterPrivate::TxNone)
        , inverseScale(1)
        , moveToCount(0)
        , dashStroker(0)
        , stroker(0)
        {}

    inline void setGLPen(const QColor &c) {
        pen_color[0] = c.red();
        pen_color[1] = c.green();
        pen_color[2] = c.blue();
        pen_color[3] = c.alpha();
    }

    inline void setGLBrush(const QColor &c) {
        brush_color[0] = c.red();
        brush_color[1] = c.green();
        brush_color[2] = c.blue();
        brush_color[3] = c.alpha();
    }

    inline void startGradientOps();
    inline void endGradientOps();
    GLhandleARB loadShader(const char *);
    void generateGradientColorTable(const QGradientStops& s,
                                    unsigned int *colorTable, int size);
    void createGradientPaletteTexture(const QGradient& g);

    void beginPath(QPaintEngine::PolygonDrawMode mode);
    void endPath();
    inline void moveTo(const QPointF &p);
    inline void lineTo(const QPointF &p);
    inline void curveTo(const QPointF &cp1, const QPointF &cp2, const QPointF &ep);

    QPen cpen;
    QBrush cbrush;
    QBrush bgbrush;
    Qt::BGMode bgmode;
    QRegion crgn;
    uint has_clipping : 1;
    uint has_pen : 1;
    uint has_brush : 1;
    uint has_autoswap : 1;
    uint has_fast_pen : 1;
    uint has_grad_brush : 1;

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

    GLuint grad_palette;
    GLhandleARB grad_radial;
    GLuint grad_radial_palette_loc;
    GLuint grad_radial_fmp_loc;
    GLuint grad_radial_fmp2_m_radius2_loc;
    GLuint grad_radial_inv_matrix_loc;
    GLuint grad_radial_inv_matrix_offset_loc;
    GLhandleARB grad_conical;
    GLuint grad_conical_palette_loc;
    GLuint grad_conical_angle_loc;
    GLuint grad_conical_inv_matrix_loc;
    GLuint grad_conical_inv_matrix_offset_loc;
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

//     for (int i=0; i<10000; ++i) {
//         tessVector.add(0);
//         vertexStorage.add(0);
//     }

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

#ifndef GL_IBM_texture_mirrored_repeat
#define GL_MIRRORED_REPEAT_IBM            0x8370
#endif

#ifndef GL_SGIS_generate_mipmap
#define GL_GENERATE_MIPMAP_SGIS           0x8191
#define GL_GENERATE_MIPMAP_HINT_SGIS      0x8192
#endif

#ifndef GL_ARB_fragment_shader
#define GL_FRAGMENT_SHADER_ARB 0x8B30
#define GL_MAX_FRAGMENT_UNIFORM_COMPONENTS_ARB 0x8B49
#define GL_FRAGMENT_SHADER_DERIVATIVE_HINT_ARB 0x8B8B
#endif

#ifndef GL_ARB_shader_objects
#define GL_PROGRAM_OBJECT_ARB             0x8B40
#define GL_SHADER_OBJECT_ARB              0x8B48
#define GL_OBJECT_TYPE_ARB                0x8B4E
#define GL_OBJECT_SUBTYPE_ARB             0x8B4F
#define GL_FLOAT_VEC2_ARB                 0x8B50
#define GL_FLOAT_VEC3_ARB                 0x8B51
#define GL_FLOAT_VEC4_ARB                 0x8B52
#define GL_INT_VEC2_ARB                   0x8B53
#define GL_INT_VEC3_ARB                   0x8B54
#define GL_INT_VEC4_ARB                   0x8B55
#define GL_BOOL_ARB                       0x8B56
#define GL_BOOL_VEC2_ARB                  0x8B57
#define GL_BOOL_VEC3_ARB                  0x8B58
#define GL_BOOL_VEC4_ARB                  0x8B59
#define GL_FLOAT_MAT2_ARB                 0x8B5A
#define GL_FLOAT_MAT3_ARB                 0x8B5B
#define GL_FLOAT_MAT4_ARB                 0x8B5C
#define GL_SAMPLER_1D_ARB                 0x8B5D
#define GL_SAMPLER_2D_ARB                 0x8B5E
#define GL_SAMPLER_3D_ARB                 0x8B5F
#define GL_SAMPLER_CUBE_ARB               0x8B60
#define GL_SAMPLER_1D_SHADOW_ARB          0x8B61
#define GL_SAMPLER_2D_SHADOW_ARB          0x8B62
#define GL_SAMPLER_2D_RECT_ARB            0x8B63
#define GL_SAMPLER_2D_RECT_SHADOW_ARB     0x8B64
#define GL_OBJECT_DELETE_STATUS_ARB       0x8B80
#define GL_OBJECT_COMPILE_STATUS_ARB      0x8B81
#define GL_OBJECT_LINK_STATUS_ARB         0x8B82
#define GL_OBJECT_VALIDATE_STATUS_ARB     0x8B83
#define GL_OBJECT_INFO_LOG_LENGTH_ARB     0x8B84
#define GL_OBJECT_ATTACHED_OBJECTS_ARB    0x8B85
#define GL_OBJECT_ACTIVE_UNIFORMS_ARB     0x8B86
#define GL_OBJECT_ACTIVE_UNIFORM_MAX_LENGTH_ARB 0x8B87
#define GL_OBJECT_SHADER_SOURCE_LENGTH_ARB 0x8B88
#endif

#ifndef APIENTRY
# define APIENTRY
#endif

typedef void (APIENTRY *pfn_glDeleteObjectARB) (GLhandleARB);
typedef GLhandleARB (APIENTRY *pfn_glGetHandleARB) (GLenum);
typedef void (APIENTRY *pfn_glDetachObjectARB) (GLhandleARB, GLhandleARB);
typedef GLhandleARB (APIENTRY *pfn_glCreateShaderObjectARB) (GLenum);
typedef void (APIENTRY *pfn_glShaderSourceARB) (GLhandleARB, GLsizei, const GLcharARB* *, const GLint *);
typedef void (APIENTRY *pfn_glCompileShaderARB) (GLhandleARB);
typedef GLhandleARB (APIENTRY *pfn_glCreateProgramObjectARB) (void);
typedef void (APIENTRY *pfn_glAttachObjectARB) (GLhandleARB, GLhandleARB);
typedef void (APIENTRY *pfn_glLinkProgramARB) (GLhandleARB);
typedef void (APIENTRY *pfn_glUseProgramObjectARB) (GLhandleARB);
typedef void (APIENTRY *pfn_glValidateProgramARB) (GLhandleARB);
typedef void (APIENTRY *pfn_glUniform1fARB) (GLint, GLfloat);
typedef void (APIENTRY *pfn_glUniform2fARB) (GLint, GLfloat, GLfloat);
typedef void (APIENTRY *pfn_glUniform3fARB) (GLint, GLfloat, GLfloat, GLfloat);
typedef void (APIENTRY *pfn_glUniform4fARB) (GLint, GLfloat, GLfloat, GLfloat, GLfloat);
typedef void (APIENTRY *pfn_glUniform1iARB) (GLint, GLint);
typedef void (APIENTRY *pfn_glUniform2iARB) (GLint, GLint, GLint);
typedef void (APIENTRY *pfn_glUniform3iARB) (GLint, GLint, GLint, GLint);
typedef void (APIENTRY *pfn_glUniform4iARB) (GLint, GLint, GLint, GLint, GLint);
typedef void (APIENTRY *pfn_glUniform1fvARB) (GLint, GLsizei, const GLfloat *);
typedef void (APIENTRY *pfn_glUniform2fvARB) (GLint, GLsizei, const GLfloat *);
typedef void (APIENTRY *pfn_glUniform3fvARB) (GLint, GLsizei, const GLfloat *);
typedef void (APIENTRY *pfn_glUniform4fvARB) (GLint, GLsizei, const GLfloat *);
typedef void (APIENTRY *pfn_glUniform1ivARB) (GLint, GLsizei, const GLint *);
typedef void (APIENTRY *pfn_glUniform2ivARB) (GLint, GLsizei, const GLint *);
typedef void (APIENTRY *pfn_glUniform3ivARB) (GLint, GLsizei, const GLint *);
typedef void (APIENTRY *pfn_glUniform4ivARB) (GLint, GLsizei, const GLint *);
typedef void (APIENTRY *pfn_glUniformMatrix2fvARB) (GLint, GLsizei, GLboolean, const GLfloat *);
typedef void (APIENTRY *pfn_glUniformMatrix3fvARB) (GLint, GLsizei, GLboolean, const GLfloat *);
typedef void (APIENTRY *pfn_glUniformMatrix4fvARB) (GLint, GLsizei, GLboolean, const GLfloat *);
typedef void (APIENTRY *pfn_glGetObjectParameterfvARB) (GLhandleARB, GLenum, GLfloat *);
typedef void (APIENTRY *pfn_glGetObjectParameterivARB) (GLhandleARB, GLenum, GLint *);
typedef void (APIENTRY *pfn_glGetInfoLogARB) (GLhandleARB, GLsizei, GLsizei *, GLcharARB *);
typedef void (APIENTRY *pfn_glGetAttachedObjectsARB) (GLhandleARB, GLsizei, GLsizei *, GLhandleARB *);
typedef GLint (APIENTRY *pfn_glGetUniformLocationARB) (GLhandleARB, const GLcharARB *);
typedef void (APIENTRY *pfn_glGetActiveUniformARB) (GLhandleARB, GLuint, GLsizei, GLsizei *, GLint *, GLenum *, GLcharARB *);
typedef void (APIENTRY *pfn_glGetUniformfvARB) (GLhandleARB, GLint, GLfloat *);
typedef void (APIENTRY *pfn_glGetUniformivARB) (GLhandleARB, GLint, GLint *);
typedef void (APIENTRY *pfn_glGetShaderSourceARB) (GLhandleARB, GLsizei, GLsizei *, GLcharARB *);

static pfn_glDeleteObjectARB qt_glDeleteObjectARB = 0;
static pfn_glGetHandleARB qt_glGetHandleARB = 0;
static pfn_glDetachObjectARB qt_glDetachObjectARB = 0;
static pfn_glCreateShaderObjectARB qt_glCreateShaderObjectARB = 0;
static pfn_glShaderSourceARB qt_glShaderSourceARB = 0;
static pfn_glCompileShaderARB qt_glCompileShaderARB = 0;
static pfn_glCreateProgramObjectARB qt_glCreateProgramObjectARB = 0;
static pfn_glAttachObjectARB qt_glAttachObjectARB = 0;
static pfn_glLinkProgramARB qt_glLinkProgramARB = 0;
static pfn_glUseProgramObjectARB qt_glUseProgramObjectARB = 0;
static pfn_glValidateProgramARB qt_glValidateProgramARB = 0;
static pfn_glUniform1fARB qt_glUniform1fARB = 0;
static pfn_glUniform2fARB qt_glUniform2fARB = 0;
static pfn_glUniform3fARB qt_glUniform3fARB = 0;
static pfn_glUniform4fARB qt_glUniform4fARB = 0;
static pfn_glUniform1iARB qt_glUniform1iARB = 0;
static pfn_glUniform2iARB qt_glUniform2iARB = 0;
static pfn_glUniform3iARB qt_glUniform3iARB = 0;
static pfn_glUniform4iARB qt_glUniform4iARB = 0;
static pfn_glUniform1fvARB qt_glUniform1fvARB = 0;
static pfn_glUniform2fvARB qt_glUniform2fvARB = 0;
static pfn_glUniform3fvARB qt_glUniform3fvARB = 0;
static pfn_glUniform4fvARB qt_glUniform4fvARB = 0;
static pfn_glUniform1ivARB qt_glUniform1ivARB = 0;
static pfn_glUniform2ivARB qt_glUniform2ivARB = 0;
static pfn_glUniform3ivARB qt_glUniform3ivARB = 0;
static pfn_glUniform4ivARB qt_glUniform4ivARB = 0;
static pfn_glUniformMatrix2fvARB qt_glUniformMatrix2fvARB = 0;
static pfn_glUniformMatrix3fvARB qt_glUniformMatrix3fvARB = 0;
static pfn_glUniformMatrix4fvARB qt_glUniformMatrix4fvARB = 0;
static pfn_glGetObjectParameterfvARB qt_glGetObjectParameterfvARB = 0;
static pfn_glGetObjectParameterivARB qt_glGetObjectParameterivARB = 0;
static pfn_glGetInfoLogARB qt_glGetInfoLogARB = 0;
static pfn_glGetAttachedObjectsARB qt_glGetAttachedObjectsARB = 0;
static pfn_glGetUniformLocationARB qt_glGetUniformLocationARB = 0;
static pfn_glGetActiveUniformARB qt_glGetActiveUniformARB = 0;
static pfn_glGetUniformfvARB qt_glGetUniformfvARB = 0;
static pfn_glGetUniformivARB qt_glGetUniformivARB = 0;
static pfn_glGetShaderSourceARB qt_glGetShaderSourceARB = 0;

#define glDeleteObjectARB qt_glDeleteObjectARB
#define glGetHandleARB qt_glGetHandleARB
#define glDetachObjectARB qt_glDetachObjectARB
#define glCreateShaderObjectARB qt_glCreateShaderObjectARB
#define glShaderSourceARB qt_glShaderSourceARB
#define glCompileShaderARB qt_glCompileShaderARB
#define glCreateProgramObjectARB qt_glCreateProgramObjectARB
#define glAttachObjectARB qt_glAttachObjectARB
#define glLinkProgramARB qt_glLinkProgramARB
#define glUseProgramObjectARB qt_glUseProgramObjectARB
#define glValidateProgramARB qt_glValidateProgramARB
#define glUniform1fARB qt_glUniform1fARB
#define glUniform2fARB qt_glUniform2fARB
#define glUniform3fARB qt_glUniform3fARB
#define glUniform4fARB qt_glUniform4fARB
#define glUniform1iARB qt_glUniform1iARB
#define glUniform2iARB qt_glUniform2iARB
#define glUniform3iARB qt_glUniform3iARB
#define glUniform4iARB qt_glUniform4iARB
#define glUniform1fvARB qt_glUniform1fvARB
#define glUniform2fvARB qt_glUniform2fvARB
#define glUniform3fvARB qt_glUniform3fvARB
#define glUniform4fvARB qt_glUniform4fvARB
#define glUniform1ivARB qt_glUniform1ivARB
#define glUniform2ivARB qt_glUniform2ivARB
#define glUniform3ivARB qt_glUniform3ivARB
#define glUniform4ivARB qt_glUniform4ivARB
#define glUniformMatrix2fvARB qt_glUniformMatrix2fvARB
#define glUniformMatrix3fvARB qt_glUniformMatrix3fvARB
#define glUniformMatrix4fvARB qt_glUniformMatrix4fvARB
#define glGetObjectParameterfvARB qt_glGetObjectParameterfvARB
#define glGetObjectParameterivARB qt_glGetObjectParameterivARB
#define glGetInfoLogARB qt_glGetInfoLogARB
#define glGetAttachedObjectsARB qt_glGetAttachedObjectsARB
#define glGetUniformLocationARB qt_glGetUniformLocationARB
#define glGetActiveUniformARB qt_glGetActiveUniformARB
#define glGetUniformfvARB qt_glGetUniformfvARB
#define glGetUniformivARB qt_glGetUniformivARB
#define glGetShaderSourceARB qt_glGetShaderSourceARB

static bool qt_resolve_frag_shader_extensions()
{
    static int resolved = false;

    if (resolved && qt_glGetHandleARB)
        return true;
    else if (resolved)
        return false;

    QGLContext cx(QGLFormat::defaultFormat());

    qt_glDeleteObjectARB = (pfn_glDeleteObjectARB) cx.getProcAddress("glDeleteObjectARB");
    qt_glGetHandleARB = (pfn_glGetHandleARB) cx.getProcAddress("glGetHandleARB");
    qt_glDetachObjectARB = (pfn_glDetachObjectARB) cx.getProcAddress("glDetachObjectARB");
    qt_glCreateShaderObjectARB = (pfn_glCreateShaderObjectARB) cx.getProcAddress("glCreateShaderObjectARB");
    qt_glShaderSourceARB = (pfn_glShaderSourceARB) cx.getProcAddress("glShaderSourceARB");
    qt_glCompileShaderARB = (pfn_glCompileShaderARB) cx.getProcAddress("glCompileShaderARB");
    qt_glCreateProgramObjectARB = (pfn_glCreateProgramObjectARB) cx.getProcAddress("glCreateProgramObjectARB");
    qt_glAttachObjectARB = (pfn_glAttachObjectARB) cx.getProcAddress("glAttachObjectARB");
    qt_glLinkProgramARB = (pfn_glLinkProgramARB) cx.getProcAddress("glLinkProgramARB");
    qt_glUseProgramObjectARB = (pfn_glUseProgramObjectARB) cx.getProcAddress("glUseProgramObjectARB");
    qt_glValidateProgramARB = (pfn_glValidateProgramARB) cx.getProcAddress("glValidateProgramARB");
    qt_glUniform1fARB = (pfn_glUniform1fARB) cx.getProcAddress("glUniform1fARB");
    qt_glUniform2fARB = (pfn_glUniform2fARB) cx.getProcAddress("glUniform2fARB");
    qt_glUniform3fARB = (pfn_glUniform3fARB) cx.getProcAddress("glUniform3fARB");
    qt_glUniform4fARB = (pfn_glUniform4fARB) cx.getProcAddress("glUniform4fARB");
    qt_glUniform1iARB = (pfn_glUniform1iARB) cx.getProcAddress("glUniform1iARB");
    qt_glUniform2iARB = (pfn_glUniform2iARB) cx.getProcAddress("glUniform2iARB");
    qt_glUniform3iARB = (pfn_glUniform3iARB) cx.getProcAddress("glUniform3iARB");
    qt_glUniform4iARB = (pfn_glUniform4iARB) cx.getProcAddress("glUniform4iARB");
    qt_glUniform1fvARB = (pfn_glUniform1fvARB) cx.getProcAddress("glUniform1fvARB");
    qt_glUniform2fvARB = (pfn_glUniform2fvARB) cx.getProcAddress("glUniform2fvARB");
    qt_glUniform3fvARB = (pfn_glUniform3fvARB) cx.getProcAddress("glUniform3fvARB");
    qt_glUniform4fvARB = (pfn_glUniform4fvARB) cx.getProcAddress("glUniform4fvARB");
    qt_glUniform1ivARB = (pfn_glUniform1ivARB) cx.getProcAddress("glUniform1ivARB");
    qt_glUniform2ivARB = (pfn_glUniform2ivARB) cx.getProcAddress("glUniform2ivARB");
    qt_glUniform3ivARB = (pfn_glUniform3ivARB) cx.getProcAddress("glUniform3ivARB");
    qt_glUniform4ivARB = (pfn_glUniform4ivARB) cx.getProcAddress("glUniform4ivARB");
    qt_glUniformMatrix2fvARB = (pfn_glUniformMatrix2fvARB) cx.getProcAddress("glUniformMatrix2fvARB");
    qt_glUniformMatrix3fvARB = (pfn_glUniformMatrix3fvARB) cx.getProcAddress("glUniformMatrix3fvARB");
    qt_glUniformMatrix4fvARB = (pfn_glUniformMatrix4fvARB) cx.getProcAddress("glUniformMatrix4fvARB");
    qt_glGetObjectParameterfvARB = (pfn_glGetObjectParameterfvARB) cx.getProcAddress("glGetObjectParameterfvARB");
    qt_glGetObjectParameterivARB = (pfn_glGetObjectParameterivARB) cx.getProcAddress("glGetObjectParameterivARB");
    qt_glGetInfoLogARB = (pfn_glGetInfoLogARB) cx.getProcAddress("glGetInfoLogARB");
    qt_glGetAttachedObjectsARB = (pfn_glGetAttachedObjectsARB) cx.getProcAddress("glGetAttachedObjectsARB");
    qt_glGetUniformLocationARB = (pfn_glGetUniformLocationARB) cx.getProcAddress("glGetUniformLocationARB");
    qt_glGetActiveUniformARB = (pfn_glGetActiveUniformARB) cx.getProcAddress("glGetActiveUniformARB");
    qt_glGetUniformfvARB = (pfn_glGetUniformfvARB) cx.getProcAddress("glGetUniformfvARB");
    qt_glGetUniformivARB = (pfn_glGetUniformivARB) cx.getProcAddress("glGetUniformivARB");
    qt_glGetShaderSourceARB = (pfn_glGetShaderSourceARB) cx.getProcAddress("glGetShaderSourceARB");

    resolved = qt_glGetShaderSourceARB ? true : false;
    return resolved;
}

GLhandleARB QOpenGLPaintEnginePrivate::loadShader(const char *sh)
{

    GLint status;
    GLhandleARB ps, fs;

    fs = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
    glShaderSourceARB(fs, 1, &sh, 0);
    glCompileShaderARB(fs);
    glGetObjectParameterivARB(fs, GL_OBJECT_COMPILE_STATUS_ARB, &status);
    if (!status) {
        qWarning("QOpenGLPaintEngine: Unable to compile fragment shader.");
        return 0;
    }
    ps = glCreateProgramObjectARB();
    glAttachObjectARB(ps, fs);
    glDeleteObjectARB(fs);
    glLinkProgramARB(ps);
    glGetObjectParameterivARB(ps, GL_OBJECT_LINK_STATUS_ARB, &status);
    if (!status) {
        qWarning("QOpenGLPaintEngine: Unable to link fragment shader.");
        return 0;
    }
    glValidateProgramARB(ps);
    glGetObjectParameterivARB(ps, GL_OBJECT_VALIDATE_STATUS_ARB, &status);
    if (!status) {
        qWarning("QOpenGLPaintEngine: Unable to validate shader object.");
        return 0;
    }
    return ps;
}

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
            colorTable[pos] = INTERPOLATE_PIXEL_256(colors[i], idist, colors[i+1], dist);
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
    else if (QGLExtensions::glExtensions & QGLExtensions::ClampToBorder)
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    else
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

    // mipmaps do not work well with conical gradients
    if (!(QGLExtensions::glExtensions & QGLExtensions::GenerateMipmap))
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    else if ((g.type() == QGradient::ConicalGradient)
             || ((g.spread() == QGradient::PadSpread)
                 && !(QGLExtensions::glExtensions & QGLExtensions::ClampToBorder)))
    {
        // if we are using CLAMP_TO_EDGE (in place of CLAMP_TO_BORDER)
        // disable mipmaps for pad gradients, or else they will look weird
        glTexParameteri(GL_TEXTURE_1D, GL_GENERATE_MIPMAP_SGIS, GL_FALSE);
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    } else {
        glTexParameteri(GL_TEXTURE_1D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    }
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, PAL_SIZE, 0, GL_BGRA, GL_UNSIGNED_BYTE, palbuf);
}


inline void QOpenGLPaintEnginePrivate::startGradientOps()
{
    if (cbrush.style() == Qt::RadialGradientPattern) {
        glDisable(GL_TEXTURE_GEN_S);
        glDisable(GL_TEXTURE_1D);
        glUseProgramObjectARB(grad_radial);
    } else if (cbrush.style() == Qt::ConicalGradientPattern) {
        glDisable(GL_TEXTURE_GEN_S);
        glDisable(GL_TEXTURE_1D);
        glUseProgramObjectARB(grad_conical);
    } else if (cbrush.style() == Qt::LinearGradientPattern) {
        glEnable(GL_TEXTURE_GEN_S);
        glEnable(GL_TEXTURE_1D);
    }
}

inline void QOpenGLPaintEnginePrivate::endGradientOps()
{
    glUseProgramObjectARB(0);
    glDisable(GL_TEXTURE_GEN_S);
    glDisable(GL_TEXTURE_1D);
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

#ifdef Q_WS_MAC  // This removes warnings.
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

    if ((QGLExtensions::glExtensions & QGLExtensions::MirroredRepeat)
        && (QGLExtensions::glExtensions &
            QGLExtensions::Extension(QGLExtensions::ClampToEdge || QGLExtensions::ClampToBorder)))
    {
        gccaps |= LinearGradientFill;
        glGenTextures(1, &d->grad_palette);

        if (QGLExtensions::glExtensions & QGLExtensions::FragmentShader) {
            qt_resolve_frag_shader_extensions();

            static char radial_shader[] =
                "uniform sampler1D palette;\n"
                "uniform vec2 fmp;\n"
                "uniform float fmp2_m_radius2;\n"
                "uniform mat2 inv_matrix;\n"
                "uniform vec2 inv_matrix_offset;\n"
                "void main(){\n"
                "       vec2 A = inv_matrix * gl_FragCoord.xy + inv_matrix_offset;\n"
                "       vec2 B = fmp;\n"
                "       float a = fmp2_m_radius2;\n"
                "       float b = 2.0*dot(A, B);\n"
                "       float c = -dot(A, A);\n"
                "       float val = (-b + sqrt(b*b - 4.0*a*c)) / (2.0*a);\n"
                "       gl_FragColor = texture1D(palette, val);\n"
                "}\n";
            static char conical_shader[] =
                "#define M_PI  3.14159265358979323846\n"
                "uniform sampler1D palette;\n"
                "uniform float angle;\n"
                "uniform mat2 inv_matrix;\n"
                "uniform vec2 inv_matrix_offset;\n"
                "void main(){\n"
                "       vec2 A = inv_matrix * gl_FragCoord.xy + inv_matrix_offset;\n"
                "       float val = mod((atan(-A.y, A.x) + angle)/(2.0 * M_PI), 1);\n"
                "       gl_FragColor = texture1D(palette, val);\n"
                "}\n";

            gccaps |= (RadialGradientFill | ConicalGradientFill);

            d->grad_radial = d->loadShader(radial_shader);
            d->grad_radial_palette_loc = glGetUniformLocationARB(d->grad_radial ,"palette");
            d->grad_radial_fmp_loc = glGetUniformLocationARB(d->grad_radial ,"fmp");
            d->grad_radial_fmp2_m_radius2_loc = glGetUniformLocationARB(d->grad_radial, "fmp2_m_radius2");
            d->grad_radial_inv_matrix_loc = glGetUniformLocationARB(d->grad_radial, "inv_matrix");
            d->grad_radial_inv_matrix_offset_loc = glGetUniformLocationARB(d->grad_radial, "inv_matrix_offset");

            d->grad_conical = d->loadShader(conical_shader);
            d->grad_conical_palette_loc = glGetUniformLocationARB(d->grad_conical, "palette");
            d->grad_conical_angle_loc = glGetUniformLocationARB(d->grad_conical, "angle");
            d->grad_conical_inv_matrix_loc = glGetUniformLocationARB(d->grad_conical, "inv_matrix");
            d->grad_conical_inv_matrix_offset_loc = glGetUniformLocationARB(d->grad_conical, "inv_matrix_offset");
        }
    }
}


QOpenGLPaintEngine::~QOpenGLPaintEngine()
{
    Q_D(QOpenGLPaintEngine);
    if (d->dashStroker)
        delete d->dashStroker;

    if(QGLExtensions::glExtensions & QGLExtensions::FragmentShader) {
        glUseProgramObjectARB(0);
        glBindTexture(GL_TEXTURE_1D, 0);

        glDeleteObjectARB(d->grad_radial);
        glDeleteObjectARB(d->grad_conical);
        glDeleteTextures(1, &d->grad_palette);
    }
}

bool QOpenGLPaintEngine::begin(QPaintDevice *pdev)
{
    Q_D(QOpenGLPaintEngine);
    d->drawable.setDevice(pdev);
    d->has_clipping = false;
    d->has_fast_pen = false;
    d->has_autoswap = d->drawable.autoBufferSwap();
    d->drawable.setAutoBufferSwap(false);
    d->inverseScale = 1;
    setActive(true);
    d->drawable.makeCurrent();
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    const QColor &c = d->drawable.backgroundColor();
    glClearColor(c.redF(), c.greenF(), c.blueF(), 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glShadeModel(GL_FLAT);
    QSize sz(d->drawable.size());
    glViewport(0, 0, sz.width(), sz.height());
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, sz.width(), sz.height(), 0, -999999, 999999);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    setDirty(QPaintEngine::DirtyPen);
    setDirty(QPaintEngine::DirtyBrush);
    glDisable(GL_MULTISAMPLE);
    glDisable(GL_TEXTURE_1D);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);
    return true;
}

bool QOpenGLPaintEngine::end()
{
    Q_D(QOpenGLPaintEngine);
    glPopAttrib();
    glFlush();
    d->drawable.swapBuffers();
    d->drawable.setAutoBufferSwap(d->has_autoswap);
    setActive(false);
    return true;
}

void QOpenGLPaintEngine::updateState(const QPaintEngineState &state)
{
    QPaintEngine::DirtyFlags flags = state.state();

    bool update_fast_pen = false;

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

    if (flags & DirtyBackground) {
        updateBackground(state.backgroundMode(), state.backgroundBrush());
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

void QOpenGLPaintEngine::updatePen(const QPen &pen)
{
    Q_D(QOpenGLPaintEngine);

    d->cpen = pen;
    d->has_pen = (pen.style() != Qt::NoPen);
    d->setGLPen(pen.color());

    glColor4ubv(d->pen_color);

    d->basicStroker.setJoinStyle(pen.joinStyle());
    d->basicStroker.setCapStyle(pen.capStyle());
    d->basicStroker.setMiterLimit(pen.miterLimit());
    qreal penWidth = pen.widthF();
    if (penWidth == 0)
        d->basicStroker.setStrokeWidth(1);
    else
        d->basicStroker.setStrokeWidth(penWidth);

    Qt::PenStyle pen_style = pen.style();
    if(pen_style == Qt::SolidLine) {
        d->stroker = &d->basicStroker;
    } else if (pen_style != Qt::NoPen) {
        if (!d->dashStroker)
            d->dashStroker = new QDashStroker(&d->basicStroker);
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
    d->has_brush = (brush.style() != Qt::NoBrush);
    d->setGLBrush(brush.color());
    glColor4ubv(d->brush_color);

    if (QGLExtensions::glExtensions & QGLExtensions::FragmentShader) {
        d->has_grad_brush = brush.style() >= Qt::LinearGradientPattern
                            && brush.style() <= Qt::ConicalGradientPattern;
        switch (brush.style()) {
        case Qt::LinearGradientPattern:
        {
            const QLinearGradient *g = static_cast<const QLinearGradient *>(brush.gradient());
            float tr[4], f;
            tr[0] = g->finalStop().x() - g->start().x();
            tr[1] = g->finalStop().y() - g->start().y();
            f = 1.0/(tr[0]*tr[0] + tr[1]*tr[1]);
            tr[0] *= f;
            tr[1] *= f;
            tr[2] = 0;
            tr[3] = -(g->start().x()*tr[0] + g->start().y()*tr[1]);
            glUseProgramObjectARB(0);
            d->setGLBrush(Qt::white);
            glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
            glTexGenfv(GL_S, GL_OBJECT_PLANE, tr);
            glEnable(GL_TEXTURE_GEN_S);
            glEnable(GL_TEXTURE_1D);

            glBindTexture(GL_TEXTURE_1D, d->grad_palette);
            d->createGradientPaletteTexture(*brush.gradient());
            glFlush();
            break;
        }
        case Qt::RadialGradientPattern:
        {
            const QRadialGradient *g = static_cast<const QRadialGradient *>(brush.gradient());
            float pt[2];
            float f;
            QMatrix translate(1, 0, 0, 1, -g->focalPoint().x(), -g->focalPoint().y());
            QMatrix gl_to_qt(1, 0, 0, -1, 0, d->pdev->height());
            QMatrix inv_matrix = gl_to_qt * d->matrix.invert() * translate;

            glUseProgramObjectARB(d->grad_radial);
            pt[0] = inv_matrix.dx();
            pt[1] = inv_matrix.dy();
            GLfloat inv[4];
            inv[0] = inv_matrix.m11();
            inv[1] = inv_matrix.m12();
            inv[2] = inv_matrix.m21();
            inv[3] = inv_matrix.m22();
            glUniform2fvARB(d->grad_radial_inv_matrix_offset_loc, 1, pt);
            glUniformMatrix2fvARB(d->grad_radial_inv_matrix_loc, 1, false, inv);
            pt[0] = g->center().x() - g->focalPoint().x();
            pt[1] = g->center().y() - g->focalPoint().y();
            glUniform2fvARB(d->grad_radial_fmp_loc, 1, pt);
            f = -pt[0]*pt[0] - pt[1]*pt[1] + g->radius()*g->radius();
            glUniform1fARB(d->grad_radial_fmp2_m_radius2_loc, f);
            glUniform1iARB(d->grad_radial_palette_loc, 0);

            glBindTexture(GL_TEXTURE_1D, d->grad_palette);
            d->createGradientPaletteTexture(*brush.gradient());
            glDisable(GL_TEXTURE_GEN_S);
            glDisable(GL_TEXTURE_1D);
            break;
        }
        case Qt::ConicalGradientPattern:
        {
            const QConicalGradient *g = static_cast<const QConicalGradient *>(brush.gradient());
            float pt[2];
            glUseProgramObjectARB(d->grad_conical);
            QMatrix translate(1, 0, 0, 1, -g->center().x(), -g->center().y());
            QMatrix gl_to_qt(1, 0, 0, -1, 0, d->pdev->height());
            QMatrix inv_matrix = gl_to_qt * d->matrix.invert() * translate;
            pt[0] = inv_matrix.dx();
            pt[1] = inv_matrix.dy();
            GLfloat inv[4];
            inv[0] = inv_matrix.m11();
            inv[1] = inv_matrix.m12();
            inv[2] = inv_matrix.m21();
            inv[3] = inv_matrix.m22();
            glUniform2fvARB(d->grad_conical_inv_matrix_offset_loc, 1, pt);
            glUniformMatrix2fvARB(d->grad_conical_inv_matrix_loc, 1, false, inv);
            glUniform1fARB(d->grad_conical_angle_loc, -(g->angle()) * 2*Q_PI / 360.0);
            glUniform1iARB(d->grad_conical_palette_loc, 0);

            glBindTexture(GL_TEXTURE_1D, d->grad_palette);
            d->createGradientPaletteTexture(*brush.gradient());
            glDisable(GL_TEXTURE_GEN_S);
            glDisable(GL_TEXTURE_1D);
            break;
        }
        default:
            glUseProgramObjectARB(0);
            glBindTexture(GL_TEXTURE_1D, 0);
            glDisable(GL_TEXTURE_GEN_S);
            glDisable(GL_TEXTURE_1D);
            break;
        }
    }
}

void QOpenGLPaintEngine::updateFont(const QFont &)
{
}

void QOpenGLPaintEngine::updateBackground(Qt::BGMode bgMode, const QBrush &bgBrush)
{
    Q_D(QOpenGLPaintEngine);
    const QColor &c = bgBrush.color();
    glClearColor(c.redF(), c.greenF(), c.blueF(), 1.0);
    d->bgmode = bgMode;
    d->bgbrush = bgBrush;
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

    d->inverseScale = qMax(1 / qMax(mtx.m11(), mtx.m22()), 0.01);

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
    if (hints & QPainter::Antialiasing)
        glEnable(GL_MULTISAMPLE);
    else
        glDisable(GL_MULTISAMPLE);
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

        if (d->has_brush) {
            if (d->has_grad_brush)
                d->startGradientOps();
            glColor4ubv(d->brush_color);
            glRectd(left, top, right, bottom);
            if (d->has_grad_brush)
                d->endGradientOps();
        }

        if (d->has_pen) {
            glColor4ubv(d->pen_color);
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

void QOpenGLPaintEngine::drawPoints(const QPointF *points, int pointCount)
{
    Q_D(QOpenGLPaintEngine);
    GLfloat pen_width = d->cpen.widthF();
    if (pen_width > 1 || (pen_width > 0 && d->txop > QPainterPrivate::TxTranslate)) {
        QPaintEngine::drawPoints(points, pointCount);
        return;
    }

    glBegin(GL_POINTS);
    {
        for (int i=0; i<pointCount; ++i)
            glVertex2d(points[i].x(), points[i].y());
    }
    glEnd();
}

void QOpenGLPaintEngine::drawLines(const QLineF *lines, int lineCount)
{
    Q_D(QOpenGLPaintEngine);
    if (d->has_pen) {
        if (d->has_fast_pen) {
            glColor4ubv(d->pen_color);
            glBegin(GL_LINES);
            {
                for (int i = 0; i < lineCount; ++i) {
                    glVertex2d(lines[i].x1(), lines[i].y1());
                    glVertex2d(lines[i].x2(), lines[i].y2());
                }
            }
            glEnd();
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
    if(pointCount < 3)
        return;

    if (d->has_brush && mode != PolylineMode) {
        if (d->has_grad_brush)
            d->startGradientOps();
        if (mode == ConvexMode) {
            glBegin(GL_TRIANGLE_FAN); {
                for (int i=0; i<pointCount; ++i)
                    glVertex2d(points[i].x(), points[i].y());
            }
            glEnd();
        } else {
            glColor4ubv(d->brush_color);
            d->beginPath(mode);
            d->moveTo(points[0]);
            for (int i=1; i<pointCount; ++i)
                d->lineTo(points[i]);
            d->endPath();
        }
        if (d->has_grad_brush)
            d->endGradientOps();
    }

    if (d->has_pen) {
        glColor4ubv(d->pen_color);
        if (d->has_fast_pen) {
            glBegin(GL_LINE_STRIP); {
                for (int i=0; i<pointCount; ++i)
                    glVertex2d(points[i].x(), points[i].y());
                if (mode != PolylineMode)
                    glVertex2d(points[0].x(), points[0].y());
            }
            glEnd();
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
        if (d->has_grad_brush)
            d->startGradientOps();
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
        if (d->has_grad_brush)
            d->endGradientOps();
    }

    if (d->has_pen) {
        glColor4ubv(d->pen_color);
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
        tpx.fill(d->bgbrush.color());
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
    glColor4f(1.0, 1.0, 1.0, 1.0);
    glEnable(GL_TEXTURE_2D);

    GLdouble tc_w = r.width()/pm.width();
    GLdouble tc_h = r.height()/pm.height();

    // Rotate the texture so that it is aligned correctly and the
    // wrapping is done correctly
    glMatrixMode(GL_TEXTURE);
    glPushMatrix();
    glRotated(180.0, 0.0, 1.0, 0.0);
    glRotated(180.0, 0.0, 0.0, 1.0);
    glBegin(GL_QUADS);
    {
        glTexCoord2d(0.0, 0.0);
        glVertex2d(r.x(), r.y());

        glTexCoord2d(tc_w, 0.0);
        glVertex2d(r.x()+r.width(), r.y());

        glTexCoord2d(tc_w, tc_h);
        glVertex2d(r.x()+r.width(), r.y()+r.height());

        glTexCoord2d(0.0, tc_h);
        glVertex2d(r.x(), r.y()+r.height());
    }
    glEnd();
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
    glPushAttrib(GL_CURRENT_BIT);
    glColor4f(1.0, 1.0, 1.0, 1.0);
    glEnable(target);

    glBegin(GL_QUADS);
    {
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

        glTexCoord2d(x1, y2);
        glVertex2d(r.x(), r.y());

        glTexCoord2d(x2, y2);
        glVertex2d(r.x()+r.width(), r.y());

        glTexCoord2d(x2, y1);
        glVertex2d(r.x()+r.width(), r.y()+r.height());

        glTexCoord2d(x1, y1);
        glVertex2d(r.x(), r.y()+r.height());
    }
    glEnd();

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

void QOpenGLPaintEngine::drawTextItem(const QPointF &p, const QTextItem &textItem)
{
#ifdef Q_WS_WIN
    Q_D(QOpenGLPaintEngine);
    // the image fallback is both faster and nicer looking on windows...
    if (d->txop <= QPainterPrivate::TxTranslate) {
        QImage img((int)textItem.width(),
                   (int)(textItem.ascent() + textItem.descent()),
                   QImage::Format_ARGB32_Premultiplied);
        img.fill(0);
        QPainter painter(&img);
        painter.setPen(d->cpen);
        painter.setBrush(d->cbrush);
        painter.drawTextItem(QPoint(0, (int)(textItem.ascent())), textItem);
        painter.end();
        drawImage(QRectF(p.x(), p.y()-(textItem.ascent()), img.width(), img.height()),
                  img,
                  QRectF(0, 0, img.width(), img.height()),
                  Qt::AutoColor);
    } else
#endif
    {
        QPaintEngine::drawTextItem(p, textItem);
    }
}
